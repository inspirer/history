// lex.cpp

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "lex.h"
#include "linker.h"
#include "ia32.h"

#define _issymbol(c) ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||c=='.'||c=='@')
#define _ishex(c) ((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))
#define _isoct(c) (c>='0'&&c<='7')
#define _isdec(c) (c>='0'&&c<='9')

struct reserved_word {
	int id;
	char *word;
	int upinfo;
	ulong hash;
};

static reserved_word list_of_rwords[] = {
	{ Ldata,	"db",		1,	0 },
	{ Ldata,	"dw",		2,	0 },
	{ Ldata,	"dd",		4,	0 },
	{ Ldata,	"dq",		8,	0 },
	{ Ldata,	"dt",		10,	0 },
	{ Lalign,	"align",	-1, 0 },
	{ Lsection,	"section",	-1, 0 },
	{ Lorg,		"org",		-1, 0 },
	{ Lif,		"if",		-1,	0 },
	{ Lelse,	"else",		-1,	0 },
	{ Lcode,	"code",		-1, 0 },
	{ 0, NULL, 0, 0 }
};

int Lex::escape(int c)
{
	switch(c){
	    case 'a': return 7;
	    case 'b': return '\b';
	    case 'f': return '\f';
	    case 'n': return '\n';
	    case 'r': return '\r';
	    case 't': return '\t';
		case 'v': return '\v';
	    case '"': case '\\': case '\?': case '\'': return c;
	}
	return 0;
}

int Lex::gettoken(void **inf,int *where)
{
	ulong x32;
	next_gettoken:
	if (limit - cp < MAXTOKEN) fillb();
	while( (*cp == ' ' || *cp == '\r' || *cp == '\t' || *cp == '\n') && cp<=limit){
		column++;while( *cp == '\t' && column%4 !=1 ) column ++;
		if( *cp == '\n' ) column = 1, line++;
		if( wannanewline && *cp == '\n' ) break;
		cp++;if (limit - cp < MAXTOKEN) fillb();
	}

	TokenPlace::where q;
	q.index = index;
	q.line = line;
	q.col = column;
	*where = lnk.info.add_place(q);
	*inf = (void *)1;

	if( wannanewline && *cp == '\n' ){
		cp++; wannanewline = 0;
		return Lnewline;
	}

	if( cp<=limit ) if( _issymbol(*cp)){
		int size = 0,i;
		char *str = (char *)tb,*nstr;
		x32 = init_hash;

		while( _issymbol(*cp) || _isdec(*cp)){
			x32 = update_hash(x32,*cp);
			*str++ = *cp++;size++;column++;
			if(size > 128){
				lnk.error(E_TOOLONGID,*where);
				break;
			}
		}
		*str = 0;

		if( (i = IA32::isregisterorprefix((char *)tb,x32)) >= 0 ){
			*inf = (void *)new int(i);
			switch(x32){
				case 0: return Lseg;
				case 1: return Lreg;
				case 2: return Lprefix;
			}
		}

		for( i=0; list_of_rwords[i].id; i++){
			if( list_of_rwords[i].hash == x32)
				if( !strcmp((char *)tb,list_of_rwords[i].word) ){
					if( list_of_rwords[i].upinfo == -1)
						*inf = (void *)1;
					else
						*inf = new int(list_of_rwords[i].upinfo);
					return list_of_rwords[i].id;
				}
		}

		nstr = new char[size+1];strncpy(nstr,(char *)tb,size);
		nstr[size]=0;
		*inf = nstr;
		return Lid;
	} else switch(*cp){

		case '>': cp++,column++;
			if (*cp == '=') return cp++,column++, Lgeq;
			if (*cp == '>') return cp++,column++, Lshr;
			return '>';

		case '<': cp++,column++;
			if (*cp == '=') return cp++,column++, Lleq;
			if (*cp == '<') return cp++,column++, Lshl;
			return '<';

		case '|': cp++,column++;return (*cp == '|')? cp++,column++,Lor2  : '|';
		case '&': cp++,column++;return (*cp == '&')? cp++,column++,Land2 : '&';
		case '=': cp++,column++;return (*cp == '=')? cp++,column++,Leq2  : '=';
		case '!': cp++,column++;return (*cp == '=')? cp++,column++,Lnoteq: '!';

		case '/':
			cp++,column++;
			if( *cp == '/' ){
				while( *cp != '\n' ){
					cp++,column++;
					if (limit - cp < MAXTOKEN) fillb();
				}
				goto next_gettoken;
			}
			return '/';

		case '+': case '-': case '*': case '%': case '^': case '#':
		case '[': case ']': case '{': case '}': case '(': case ')':
		case ';': case ',': case ':': case '~': case '?': case '$': 
			return column++,*cp++;

		case '0':
			column++;cp++;
			x32 = 0;
			if( *cp == 'x' || *cp == 'X' ){
				cp++;column++;

				if( !_ishex(*cp) )
					lnk.error(E_INVALIDHEX,*where);

				while( _ishex(*cp)){
					if( x32>0xfffffff ){
						lnk.error(E_OVERFLOW,*where);
						while( _ishex(*cp)){
							cp++;column++;
							if (limit - cp < MAXTOKEN) fillb();
						}
						break;
					}
					if( *cp >='a' && *cp <='f' ) *cp -= 39;
					else if( *cp >='A' && *cp <='F' ) *cp -= 7;
					x32 <<= 4;x32 += *cp - '0';
					column++;cp++;
				}
			} else
				while( _isoct(*cp)){
					if( x32>03777777777 ){
						lnk.error(E_OVERFLOW,*where);
						while( _isoct(*cp)){
							cp++;column++;
							if (limit - cp < MAXTOKEN) fillb();
						}
						break;
					}
					x32 <<= 3;x32 += *cp - '0';
					column++;cp++;
				}

			if( _issymbol(*cp) || _isdec(*cp) )
				lnk.error(E_WRONGCONST,*where);
			*inf = (void *)new ulong(x32);
			return Licon;

		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			x32 = 0;
			while( _isdec(*cp)){
				if(x32>429496729 || (x32 == 429496729 && *cp>'5')){
					lnk.error(E_OVERFLOW,*where);
					while( _isdec(*cp)){
						cp++;column++;
						if (limit - cp < MAXTOKEN) fillb();
					}
					break;
				}
				x32 *= 10;x32 += *cp - '0';
				column++;cp++;
			}
			if( _issymbol(*cp) )
				lnk.error(E_WRONGCONST,*where);
			*inf = (void *)new ulong(x32);
			return Licon;

		case '\'':
			cp++; column++;
			if( *cp == '\\' ){
				cp++; column++;
				x32 = escape(*cp);
				if( x32 ){
					cp++; column++;
				} lnk.error(E_WRONGESC,*where);
			} else {
				x32 = *cp++; column++;
			}
			if( *cp == '\'' ){
				cp++; column++;
			} else lnk.error(E_NOTFIN2,*where);

			*inf = (void *)new ulong(x32);
			return Lccon;

		case '"': {
				int i = 0;
				char *s = (char *)tb;
				cp++; column++;
				while( *cp != '"' && *cp != '\n' && i<4094){
					if( *cp == '\\' ){
						cp++; column++;
						x32 = escape(*cp);
						if( x32 ){
							i++; cp++; column++;
							*s++ = (char)x32;
						} else {
							lnk.error(E_WRONGESC,*where);
							break;
						}
					} else { 					
						*s++ = *cp++; i++; column++;
					}
					if (limit - cp < MAXTOKEN) fillb();
				}
				*s = 0;
				if( i == 4095 ) lnk.error(E_LONGSTR,*where);
				if( *cp == '\n' ) lnk.error(E_NOTFIN,*where);
				if( *cp == '"' ) cp++,column++;
				s = new char[i+1];
				strcpy(s,(char *)tb);
				*inf = (void *)s;
				return Lscon;
			}

		default: 
			lnk.error(( *cp>=32 && *cp<127)?E_UNKNWNSYM:E_UNKNWNSYMCODE,*where,*cp);
			column++;cp++;
			goto next_gettoken;

	}
	return Leoi;
}

int Lex::start(stream *in)
{
	for( int i=0; list_of_rwords[i].id; i++)
		list_of_rwords[i].hash = hash_value(list_of_rwords[i].word);

	Lex::in = in;
	lnk.info.clear();
	index = lnk.info.add_index(in->name);
	line = column = 1;

	cp = limit = &buffer[MAXTOKEN+1];
	size = 1; wannanewline = 0;
	fillb();
	return (in!=NULL);
}

void Lex::close()
{
	lnk.info.clear();
	in = NULL;
}

int Lex::fillb()
{
	if( size == 0 || limit - cp >= MAXTOKEN) return 1;

	if( cp >= limit) cp = &buffer[MAXTOKEN+1];
	else {
		int n = limit-cp;
		uchar *s = &buffer[MAXTOKEN+1] - n;
		while( cp < limit) *s++ = *cp++;
		cp = &buffer[MAXTOKEN+1] - n;
	}

	size = in->read(&buffer[MAXTOKEN+1], BUFSIZE);
	if( size<0){
		lnk.error(E_READFATAL,-1);
		limit = &buffer[MAXTOKEN+1];
		return 0;
	}
	limit = &buffer[MAXTOKEN+1+size];
	*limit = '\n';
	return 1;
}
