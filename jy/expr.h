// expr.h

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef EXPR_H_INCLUDED
#define EXPR_H_INCLUDED

#include "defs.h"

struct Expr;
class NamesHolder;
struct NodeSeq;

struct Name 
{
	char *name;
	ulong hash;
	union {
		ulong value;
		struct {
			ulong used;
			NodeSeq *action;
		};
	};
	Name *next;
};

class NamesHolder {
public:
	NamesHolder *parent,*original,*next;
	Name *first;

	Name * AddName(char *s);
	Name * SearchName(char *s,int godown=1);
	NamesHolder(NamesHolder *p=NULL,NamesHolder *o=NULL);
	~NamesHolder();
};

#define EXP_CONST		1001
#define EXP_ID			1002
#define EXP_SECTION		1003
#define EXP_CONDITIONAL 1004
#define EXP_UNARY		1005

struct Expr
{
	int type;
	union {
		Expr *p[3];
		ulong cnst;		// EXP_CONST
		char *id;		// EXP_ID, EXP_SECTION
	};

	Expr();
	~Expr();
	static Expr * make_tree_expr(int type,Expr *left,Expr *right=NULL,Expr *condition=NULL);
	ulong Evaluate(NamesHolder *nh);

	#ifdef DEBUG_expr
		void Debug();
	#endif
};

#endif
