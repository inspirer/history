// Test of (types.cpp -> Type::create_struct, sym.cpp -> Sym::symbol_created)

union a { 
	unsigned :33;
	unsigned IOPL:3;
	struct yo { int i; } a:1;
	int yo;
    const char c;

    void f();
    int i[];
    struct a yo;
};

struct a;
union a { int q,b,c; };

struct aq {
	int i;
	struct {
		int b;
	} q;
	struct {
		int c;
	} n;
} i;


struct yoyo *q;

struct yoyo {
	int i;
};

void main() {

	struct aq yo;

	struct aq {
		int o;
	};

	struct aq yo2;

	yo.i;
	i.i;
	yo.q.b;
	yo2.o;

	q->i;
}

