// linker.cpp

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "place.h"
#include "defs.h"
#include "linker.h"
#include "jyof.h"

Linker lnk;

char * errorlist[] = {
#define err(a,b,c) c,
#include "errlist.h"
#undef err
};

// ======== Node ========

void Node::DeleteList(Node *list)
{
	Node *i = list,*e;
	while(i){ e = i; i = i->next; delete e; }
}

Node::Node()
{ 
	next = NULL; exp = NULL; 
	type = size = 0; 
}

Node::~Node()
{
	switch( type ){
		case NODE_OPCODE:
			delete op;
			break;
		case NODE_ALIGN: case NODE_ORG:
			delete exp;
			break;
		case NODE_SECTION_CALL:
			delete exp;
			delete id;
			if( name && name->action){
				DeleteList(name->action->first);
				delete name->action;
				name->action = NULL;
			}
			break;
		case NODE_EXPRESSION:
			delete exp;
			delete id;
			break;
		case NODE_SECTION_DEF:
			delete id;
			if( section ){ 
				DeleteList(section->first);
				delete section;
			}
			break;
		case NODE_COMPOUND:
			DeleteList(compound);
			break;
		case NODE_IF:
			delete exp;
			DeleteList(p[0]);
			DeleteList(p[1]);
			break;
		case NODE_LABEL:
			delete id;
			break;
		case NODE_DATA:
			delete id;
			IA32data::DeleteList(data);
			break;
	}
}

// ======== Linker ========

void Linker::error(int type,int place,...)
{
	va_list arglist;

	va_start(arglist, place);
	if( place>=0 ){
		if( !status ){
			TokenPlace::where * q = info.get_place(place);
			if (q){
				char *name = info.get_index(q->index);
				if (name)
					fprintf(stderr,"%s(%i,col %i): ",name,q->line,q->col);
			}
		} else {
			fprintf(stderr,"%s(%i): ",cnode->file,cnode->line);
		}
	}
	switch(type&3){
		case E_WARN: fprintf(stderr,"warning(N%i): ",type >> 2);warnings++;break;
		case E_ERROR: fprintf(stderr,"error(N%i): ",type >> 2);errors++;break;
		case E_FATAL: fprintf(stderr,"fatal(N%i): ",type >> 2);fatals++;break;
	}
	vfprintf(stderr, errorlist[type>>2], arglist);
	fprintf(stderr,"\n");
}

void Linker::init()
{
	errors = warnings = fatals = 0;
	nholders_list = NULL;
	IA32::inithash();
}

void Linker::compile()
{
	status = 0;
	program = NULL;
	parse();
	info.clear();
}

void Linker::walk_sections(Node *w)
{
	while( w ){
		Name *n;
		cnode = w;
		switch( w->type ){
			case NODE_SECTION_CALL:
				n = sections->SearchName(w->id);
				if( !n ){
					n = sections->AddName(w->id);
					n->action = NULL; n->used = 0;
				}
				if( n->used & 1 ) error(E_TWOSEC_CALL,-1,w->id);
					else w->name = n, n->used |= 1;
				break;
			case NODE_SECTION_DEF:
				n = sections->SearchName(w->id);
				if( !n ){
					n = sections->AddName(w->id);
					n->action = NULL; n->used = 0;
				}
				delete w->id;
				if( w->section ){
					if( w->section->first ){
						walk_sections(w->section->first);
						if( n->action ) {
							n->action->last->next = new Node;
							n->action->last = n->action->last->next;
						} else {
							n->action = new NodeSeq;
							n->action->first = n->action->last = new Node;
						}
						n->action->last->type = NODE_COMPOUND;
						n->action->last->line = w->section->first->line;
						n->action->last->file = w->section->first->file;
						n->action->last->compound = w->section->first;
						n->action->last->holder = new NamesHolder;
						delete w->section;
						w->compound = n->action->last;
					} else {
						delete w->section;
						w->compound = NULL;
					}
				} else w->compound = NULL;
				w->type = NODE_SECTION_MOVED;
				break;
			case NODE_COMPOUND:
				walk_sections(w->compound);
				break;
			case NODE_IF:
				walk_sections(w->p[0]);
				walk_sections(w->p[1]);
				break;
		}
		w = w->next;
	}
}

void Linker::walk_expressions(Node *w,NamesHolder *holder)
{
	int t;

	while( w ){
		switch( w->type ){
			case NODE_EXPRESSION:
				w->nh = holder;
				t = w->time;
				if( t != 1){
					w->next_expr = NULL;
					if( expressions[t] ) expr_ends[t] = expr_ends[t]->next_expr = w;
						else expr_ends[t] = expressions[t] = w;
				}
				break;
			case NODE_SECTION_CALL:
				if( w->name ){ 
					w->name->used |= 2;
					if( w->name->action ) walk_expressions(w->name->action->first,holder);
				}
				break;
			case NODE_SECTION_MOVED:
				if( w->compound ){
					w->compound->holder->original = holder;
				}
				break;
			case NODE_COMPOUND:
				if( !w->holder) w->holder = new NamesHolder(holder);
					else w->holder->parent = holder;
				walk_expressions(w->compound,w->holder);
				break;
			case NODE_IF:
				w->nh = holder;
				walk_expressions(w->p[0],holder);
				walk_expressions(w->p[1],holder);
				break;
		}
		w = w->next;
	}
}

void Linker::calc_expr_list(Node *w)
{
	Name * n;
	while( w ){
		cnode = w;
		if( *w->id != '.' && *w->id !='_' ) n = w->nh->AddName(w->id);
			else n = global->AddName(w->id);
		n->value = w->exp->Evaluate(w->nh);
		w = w->next_expr;
	}
}

void Linker::walk_if(Node *w)
{
	Node *n;
	while( w ){
		cnode = w;
		if( w->type == NODE_IF){
			if( w->exp->Evaluate(w->nh) ){
				n = w->p[0]; Node::DeleteList(w->p[1]);
			} else {
				n = w->p[1]; Node::DeleteList(w->p[0]);
			}
			delete w->exp;
			w->type = NODE_COMPOUND;
			w->compound = n;
			w->holder = w->nh;
		} 
		if( w->type == NODE_SECTION_CALL){
			if( w->name && w->name->action ) walk_if(w->name->action->first);
		} else if( w->type == NODE_COMPOUND) walk_if(w->compound);

		w = w->next;
	}
}

ulong Linker::walk_program(Node *w,NamesHolder *curr)
{
	ulong i,e,lsize = 0;
	union {
		Name *n;
		IA32data *d;
	};

	while( w ){
		cnode = w;
		if( status == 3 && w->type!=NODE_EXPRESSION ) w->size = 0;
		switch( w->type ){
			case NODE_OPCODE:
				if( status == 3 ){
					w->size = w->op->size + w->op->disp_size + w->op->imm_size;
				} else {
					fwrite(w->op->code,1,w->op->size,out);					
					if( w->op->disp_size ){
						if( w->op->dispe )
							i = w->op->dispe->Evaluate(curr);
						else i = 0;
						if( w->op->relative ) i -= codegen.offset + w->size;
						switch(w->op->disp_size){
							case 2:
								if( i > 0x7fff && i < 0xffff8000 )
									error(E_IMMDISPOUT,0,"disp16");
								break;
							case 1:
								if( i > 0x7f && i < 0xffffff80 )
									error(E_IMMDISPOUT,0,"disp8");
								break;
						}
						fwrite(&i,w->op->disp_size,1,out);
					}
					if( w->op->imm_size ){
						if( w->op->imme )
							i = w->op->imme->Evaluate(curr);
						else i = 0;
						switch(w->op->imm_size){
							case 2:
								if( i > 0xffff )
									error(E_IMMDISPOUT,0,"imm16");
								break;
							case 1:
								if( i > 0xff )
									error(E_IMMDISPOUT,0,"imm8");
								break;
						}
						fwrite(&i,w->op->imm_size,1,out);
					}
				}
				break;
			case NODE_EXPRESSION:
				if( status == 3 && w->time == 1){
					if( *w->id != '.' && *w->id !='_' ) n = w->nh->AddName(w->id);
						else n = global->AddName(w->id);
					n->value = w->exp->Evaluate(w->nh);
				}
				break;
			case NODE_ALIGN:
				if( status == 3 ){
					i = w->exp->Evaluate(curr);
					w->size = i - codegen.offset%i;
					if( w->size == i ) w->size = 0;
				} else {
					for( i=0; i<w->size; i++) fputc(0,out);
				}
				break;
			case NODE_ORG:
				if( status == 3) w->offset = w->exp->Evaluate(curr);
				codegen.offset = w->offset;
				break;
			case NODE_COMPOUND:
				i = codegen.offset;
				e = walk_program(w->compound,w->holder);
				codegen.offset = i+e;
				lsize += e;
				break;
			case NODE_LABEL:
				if( status == 3 ){
					if( *w->id != '.' && *w->id !='_' ) n = curr->AddName(w->id);
						else n = global->AddName(w->id);
					n->value = codegen.offset;
				}
				break;
			case NODE_DATA:
				if( status == 3 && w->id ){
					if( *w->id != '.' && *w->id !='_' ) n = curr->AddName(w->id);
						else n = global->AddName(w->id);
					n->value = codegen.offset;
				}
				d = w->data;
				if( status == 3){
					while( d ){
						if( d->type == -1) w->size += strlen(d->string);
						else if( d->type & DT_ARRAY ) w->size += (d->type&~DT_ARRAY)*d->expr->Evaluate(curr);
							else w->size += d->type;
						d = d->next;
					}
				} else {
					while( d ){
						if( d->type == -1) fwrite(d->string,1,strlen(d->string),out);
						else if( d->type & DT_ARRAY ){
							i = d->type& ~DT_ARRAY;
							e = d->expr->Evaluate(curr);
							if( i == 1 )
								for( ; e; e--) fputc(d->fill,out);
							else if( i<=4)
								for( ; e; e--) fwrite(&d->fill,1,i,out);
							else error(E_NOTIMPLEMENTED,-1,"array of dq and dt");

						} else if( d->type <= 4){
							d->number = d->expr->Evaluate(curr);
							switch(d->type){
								case 2:
									if( d->number > 0xffff )
										error(E_IMMDISPOUT,0,"data16");
									break;
								case 1:
									if( d->number > 0xff )
										error(E_IMMDISPOUT,0,"data8");
									break;
							}
							fwrite(&d->number,1,d->type,out);
						} else error(E_NOTIMPLEMENTED,-1,"dq and dt");
						d = d->next;
					}
				}
				break;
			case NODE_SECTION_CALL:
				i = codegen.offset; e = 0;
				if( w->exp ) codegen.offset = w->exp->Evaluate(curr);
				if( w->name && w->name->action ) e = walk_program(w->name->action->first,curr);
				codegen.offset = i+e;
				if( w->name ) w->name->used = e;
				lsize += e;
				break;
		}
		if( w->type!=NODE_EXPRESSION){
			codegen.offset += w->size;
			codegen.fileoffset += w->size;
			lsize += w->size;
		}
		w = w->next;
	}
	return lsize;
}

void Linker::link(NodeSeq *program)
{
	int i;
	global = names = new NamesHolder;
	sections = new NamesHolder;

	status = 1;
	for( i = 0; i < 33; i++ ) expressions[i] = NULL;
	walk_sections(program->first);
	walk_expressions(program->first,names);

	for( Name * n = sections->first; n; n = n->next ){
		if( n->used != 3 ){
			NodeSeq * d = n->action;
			n->action = NULL;
			if( d ){
				Node::DeleteList(d->first);
				delete d;
			}
			if( !(n->used&1) ) error(E_NOSEC_CALL,-1,n->name);
			if( !(n->used&1) ){ error(E_SECT_LOOP,-1,n->name);return;}
		}
	}

	status = 2;
	calc_expr_list(expressions[0]);
	walk_if(program->first);

	status = 3;
	codegen.fileoffset = 0; codegen.offset = 0;
	walk_program(program->first,global);

	status = 4;
	for( i=2; i<33; i++ ) calc_expr_list(expressions[i]);
	codegen.fileoffset = 0; codegen.offset = 0;
	if( (out=fopen(output_file,"wb")) ){
		walk_program(program->first,global);
		fclose(out);
	} else error(E_FILENOTCREAT,-1,output_file);
}

void Linker::walk_tree(Node *w)
{
	while( w ){
		switch( w->type ){
			case NODE_OPCODE:
				fputc(_OF_OPCODE,out);
				fwrite(w->op,1,sizeof(IA32opcode),out);
				break;
			case NODE_ALIGN:
			case NODE_ORG:
			case NODE_COMPOUND:
			case NODE_IF:
			case NODE_LABEL:
			case NODE_DATA:
			case NODE_EXPRESSION:
			case NODE_SECTION_CALL:
			case NODE_SECTION_DEF:
				break;
		}
		w = w->next;
	}
	fputc(_OF_BLOCKEND,out);
}

void Linker::outtree(Node *w)
{
	ulong i;
	if( (out=fopen(output_file,"wb")) ){
		
		fwrite("JYOF\r\n",6,1,out);
		while( w ){
			fputc(_OF_NEWFILE,out);
			i = strlen(w->file)+1;
			fwrite(&i,1,4,out);
			fwrite(w->file,1,i,out);
			walk_tree(w->compound);
			w = w->next;
		}

		fclose(out);
	} else error(E_FILENOTCREAT,-1,output_file);
}

void Linker::clear(NodeSeq *program)
{
	if( program ) Node::DeleteList(program->first);
	while( nholders_list ){
		NamesHolder *current = nholders_list;
		nholders_list = nholders_list->next;
		delete current;
	}
}
