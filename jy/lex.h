// lex.h

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include "input.h"
#include "defs.h"

#define init_hash 0x12b3
#define update_hash(prev,symb) prev=(prev<<1)+(prev>>31),prev^=0x989693*(symb);

enum {
_START = 255,
#define pp(a,b,c) a,
#include "terms.h"
#undef pp
};

class Lex {
private:
    stream *in;
    int size,line,column,index;
    uchar buffer[MAXTOKEN + 1 + BUFSIZE + 1],tb[4096],*cp,*limit;
    int fillb();

public:
	int wannanewline;
	int escape(int c);
	int gettoken(void **inf,int *where);
	int start(stream *in);
	void close();

	static inline ulong hash_value(char *s){
		if (!s) return 0;
		ulong l = init_hash;
		while (*s){
			l = update_hash(l,*s)
			s++;
		}
		return l;
	}
};

#endif // LEX_H_INCLUDED
