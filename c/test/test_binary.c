// Test of (expr.cpp -> Expr::create_binary)
// Test of (expr.cpp -> Expr::get_type_size)

void main( register int a) {

	char c;
	short s;
	int i;
	float f;
	long double d;

	c + s;		// binary: 3: int (int,int)
	d + c;		// binary: 3: long double (long double,long double)
	sizeof(long long);	  // sizeof: 8
	sizeof(void(*)(int)); // sizeof: 4
}
