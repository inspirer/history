// expr.cpp

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "expr.h"
#include "linker.h"

// ======== Expr ========

Expr::Expr()
{
}

Expr::~Expr()
{
	switch( type ){
		case EXP_ID: case EXP_SECTION:
			delete id;
			break;
		case EXP_CONST: case '$': case '#':
			break;
		default:
			if( p[0] ) delete p[0];
			if( p[1] ) delete p[1];
			if( p[2] ) delete p[2];
			break;
	}
}

Expr * Expr::make_tree_expr(int type,Expr *left,Expr *right,Expr *condition)
{
	Expr * n = new Expr;
	n->type = type;
	n->p[0] = left;
	n->p[1] = right;
	n->p[2] = condition;
    if( left && left->type == EXP_CONST && (right == NULL || right->type == EXP_CONST) && (condition == NULL || condition->type == EXP_CONST) ){
    	ulong u = n->Evaluate(NULL);
        n->type = EXP_CONST;
        n->cnst = u;
		delete left;
		if( right ) delete right;
		if( condition ) delete condition;
    }
	return n;
}

ulong Expr::Evaluate(NamesHolder *nh)
{
	Name *n;
	switch( type ){
		case EXP_ID:
			n = nh->SearchName(id);
			if( !n ){ lnk.error(E_NOID,0,id);return 0;}
				else return n->value;
		case EXP_SECTION:
			n = lnk.sections->SearchName(id);
			if( lnk.status == 4 ) return n->used;
			else if( n && n->action ) return 1; else return 0;
		case '$':
			if( lnk.status < 3 ) lnk.error(E_NOSHARP,0,'$');
			return lnk.codegen.offset;
		case '#':
			if( lnk.status < 3 ) lnk.error(E_NOSHARP,0,'#');
			return lnk.codegen.fileoffset;
		case EXP_CONST:
			return cnst;
		case EXP_UNARY:
			return (ulong)(-(long)p[0]->Evaluate(nh));
		case EXP_CONDITIONAL:
			if( p[2]->Evaluate(nh) )
				return p[0]->Evaluate(nh);
			else 
				return p[1]->Evaluate(nh);
		case '!':
			return !p[0]->Evaluate(nh);
		case '~':
			return ~p[0]->Evaluate(nh);
		case '*':
			return p[0]->Evaluate(nh) * p[1]->Evaluate(nh);
		case '%':
			return p[0]->Evaluate(nh) % p[1]->Evaluate(nh);
		case '/':
			return p[0]->Evaluate(nh) / p[1]->Evaluate(nh);
		case '+':
			return p[0]->Evaluate(nh) + p[1]->Evaluate(nh);
		case '-':
			return p[0]->Evaluate(nh) - p[1]->Evaluate(nh);
		case Lshl:
			return p[0]->Evaluate(nh) << p[1]->Evaluate(nh);
		case Lshr:
			return p[0]->Evaluate(nh) >> p[1]->Evaluate(nh);
		case '<':
			return p[0]->Evaluate(nh) < p[1]->Evaluate(nh);
		case '>':
			return p[0]->Evaluate(nh) > p[1]->Evaluate(nh);
		case Lleq:
			return p[0]->Evaluate(nh) <= p[1]->Evaluate(nh);
		case Lgeq:
			return p[0]->Evaluate(nh) >= p[1]->Evaluate(nh);
		case Leq2:
			return p[0]->Evaluate(nh) == p[1]->Evaluate(nh);
		case Lnoteq:
			return p[0]->Evaluate(nh) != p[1]->Evaluate(nh);
		case '&':
			return p[0]->Evaluate(nh) & p[1]->Evaluate(nh);
		case '|':
			return p[0]->Evaluate(nh) | p[1]->Evaluate(nh);
		case '^':
			return p[0]->Evaluate(nh) ^ p[1]->Evaluate(nh);
		case Land2:
			return p[0]->Evaluate(nh) && p[1]->Evaluate(nh);
		case Lor2:
			return p[0]->Evaluate(nh) || p[1]->Evaluate(nh);
	}
	return 0;
}

#ifdef DEBUG_expr

void Expr::Debug()
{
	switch( type ){
		case EXP_ID:
			printf("%s",id); break;
		case EXP_SECTION:
			printf("section(%s)",id); break;
		case '$':
			printf("$"); break;
		case '#':
			printf("#"); break;
		case EXP_CONST:
			printf("%u",cnst); break;
		case EXP_UNARY:
			printf("( -");p[0]->Debug(); printf(" )");break;
		case EXP_CONDITIONAL:
			printf("( ");p[2]->Debug();printf(" ? ");p[0]->Debug();printf(" : ");p[1]->Debug();printf(" )"); break;
		case '!':
			printf("( !");p[0]->Debug();printf(" )"); break;
		case '~':
			printf("( ~");p[0]->Debug();printf(" )"); break;
		case '*':
			printf("( ");p[0]->Debug();printf(" * ");p[1]->Debug();printf(" )"); break;
		case '%':
			printf("( ");p[0]->Debug();printf(" % ");p[1]->Debug();printf(" )"); break;
		case '/':
			printf("( ");p[0]->Debug();printf(" / ");p[1]->Debug();printf(" )"); break;
		case '+':
			printf("( ");p[0]->Debug();printf(" + ");p[1]->Debug();printf(" )"); break;
		case '-':
			printf("( ");p[0]->Debug();printf(" - ");p[1]->Debug();printf(" )"); break;
		case Lshl:
			printf("( ");p[0]->Debug();printf(" << ");p[1]->Debug();printf(" )"); break;
		case Lshr:
			printf("( ");p[0]->Debug();printf(" >> ");p[1]->Debug();printf(" )"); break;
		case '<':
			printf("( ");p[0]->Debug();printf(" < ");p[1]->Debug();printf(" )"); break;
		case '>':
			printf("( ");p[0]->Debug();printf(" > ");p[1]->Debug();printf(" )"); break;
		case Lleq:
			printf("( ");p[0]->Debug();printf(" <= ");p[1]->Debug();printf(" )"); break;
		case Lgeq:
			printf("( ");p[0]->Debug();printf(" >= ");p[1]->Debug();printf(" )"); break;
		case Leq2:
			printf("( ");p[0]->Debug();printf(" == ");p[1]->Debug();printf(" )"); break;
		case Lnoteq:
			printf("( ");p[0]->Debug();printf(" != ");p[1]->Debug();printf(" )"); break;
		case '&':
			printf("( ");p[0]->Debug();printf(" & ");p[1]->Debug();printf(" )"); break;
		case '|':
			printf("( ");p[0]->Debug();printf(" | ");p[1]->Debug();printf(" )"); break;
		case '^':
			printf("( ");p[0]->Debug();printf(" ^ ");p[1]->Debug();printf(" )"); break;
		case Land2:
			printf("( ");p[0]->Debug();printf(" && ");p[1]->Debug();printf(" )"); break;
		case Lor2:
			printf("( ");p[0]->Debug();printf(" || ");p[1]->Debug();printf(" )"); break;
	}
}

#endif

// ======== NamesHolder ========

NamesHolder::NamesHolder(NamesHolder *p,NamesHolder *o)
{
	first = NULL;
	original = o;
	parent = p;
	next = lnk.nholders_list;
	lnk.nholders_list = this;
}

NamesHolder::~NamesHolder()
{
	while( first ){
		Name * current = first;
		first = first->next;
		delete current->name;
		delete current;
	}
}

Name * NamesHolder::AddName(char *s)
{
	if( original ) return original->AddName(s);

	Name * n;
	if( (n=SearchName(s,0)) )
		lnk.error(E_SYMALREADYEX,0,s);

	n = new Name;
	n->name = new char[strlen(s)+1];
	strcpy(n->name,s);
    n->hash = Lex::hash_value(s);
	n->next = first;
	first = n;
	return n;
}

Name * NamesHolder::SearchName(char *s,int godown)
{
	Name *n;
	if( original ){ 
		n = original->SearchName(s,godown);
		if( n ) return n;
	} else {
		ulong hash = Lex::hash_value(s);
		for( n = first; n; n = n->next){
			if( n->hash == hash && strcmp(s,n->name)==0 )
				return n;
		}
	}
	if( godown && parent ) return parent->SearchName(s,1);
	return NULL;
}
