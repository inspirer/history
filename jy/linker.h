// linker.h

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef LINKER_H_INCLUDED
#define LINKER_H_INCLUDED

#include "defs.h"
#include "lex.h"
#include "place.h"
#include "parse.h"
#include "expr.h"
#include "ia32.h"

#define NODE_OPCODE			1
#define NODE_ALIGN			2
#define NODE_ORG			3
#define NODE_COMPOUND		4
#define NODE_IF				5
#define NODE_LABEL			6
#define NODE_DATA			7
#define NODE_EXPRESSION		8
#define NODE_SECTION_CALL	9
#define NODE_SECTION_DEF	10

#define NODE_SECTION_MOVED  11

struct Node
{
	ulong type,line;
	char *file;
	Node *next;
	union {
		ulong size;				// NODE_OPCODE, NODE_DATA, NODE_ALIGN
		NamesHolder *nh;		// NODE_EXPRESSION, NODE_IF
	};
	Expr *exp;				// NODE_ALIGN, NODE_ORG, NODE_SECTION_CALL, NODE_IF, NODE_EXPRESSION
	union {
		ulong offset;		// NODE_ORG
		IA32opcode *op;		// NODE_OPCODE
		struct {			// NODE_COMPOUND
			Node *compound;
			NamesHolder *holder;
		};
		Node *p[2];			// NODE_IF
		struct {
			char *id;		// NODE_SECTION_CALL, NODE_SECTION_DEF, NODE_LABEL, NODE_EXPRESSION, NODE_DATA
			union {
				Name *name;			// NODE_SECTION_CALL
				NodeSeq *section;	// NODE_SECTION_DEF
				int time;			// NODE_EXPRESSION
				Node *next_expr;
				IA32data *data;		// NODE_DATA
			};
		};
	};

	Node();
	~Node();
	static void DeleteList(Node *list);
};

CLASS_SEQ(Node)

class Linker {
public:
	NamesHolder *nholders_list;

private:
    static int gg_translate[256];
    static short gg_ract[],gg_rlen[],gg_rlex[];
    static char * gg_text[];
    gg_msymbol gg_m[1024];
    gg_symbol gg_next;
    int gg_head;
    void gg_apply_rule(int rule);
    void * gg_create_rule_nterm(int type);
    void gg_push_stack(gg_msymbol& s);
    void gg_push_stack_null(int lexem);
    void gg_pop_stack(int count);
    int  gg_update_state(int state,int lexem);
    int  gg_next_state(int state,int lexem);
    void gg_shift();
    void parse();

public:
    Lex lex;
	TokenPlace info;
	NamesHolder *names,*global,*sections;
	IA32struct iaop;
	IA32param *iapm;
	IA32 codegen;
	int datatype;
	NodeSeq *program;
	Node *expressions[33],*expr_ends[32];

	ulong errors,warnings,fatals;
	ulong status;
	Node *cnode;
	char *current_file,*output_file;
	FILE *out;
	union {
		struct {
			unsigned compile:1;
		} flags;
		int binary;
	};

	void init();
    void compile();
	void calc_expr_list(Node *w);
	void walk_expressions(Node *w,NamesHolder *holder);
	void walk_sections(Node *w);
	void walk_if(Node *w);
	ulong walk_program(Node *w,NamesHolder *curr);
	void link(NodeSeq *program);
	void walk_tree(Node *w);
	void outtree(Node *w);
	void error(int type,int place,...);
	void clear(NodeSeq *program);
};

extern Linker lnk;

enum {
#define err(a,b,c) y_##a,
#include "errlist.h"
#undef err
};

enum {
#define err(a,b,c) a = (y_##a<<2)+b,
#include "errlist.h"
#undef err
};

#endif
