// res.h

#define sizeop(n) ((n)<<10)
#define specific(op) ((op)&0x3FF)
#define opindex(op) (((op)>>4)&0x3F)
#define opsize(op)  ((op)>>10)
#define optype(op)  ((op)&0xF)

typedef struct node *Node;

struct node {
	short op;
	short count;
 	Sym syms[3];
	Node kids[2];
	Node link;
};

enum { 	F, I, U, P, V, B };

enum {
	c = 1,
	s = 2,
	i = 4,
	l = 4
	h = 8,

	f = 4,
	d = 8,
	x = 8,

	p = 4
};
