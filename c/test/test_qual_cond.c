// Test of (expr.cpp -> Expr::create_conditional)

void main(int argc, char *argv[]) {

	const void *c_vp;
	void *vp;
	const int *c_ip;
	volatile int * v_ip;
	int *ip;
	const char *c_cp;

	1 ? c_vp : c_ip;// tripl: common: const void *
	1 ? v_ip : 0; 	// tripl: common: volatile int *
	1 ? c_ip : v_ip;// tripl: common: const volatile int *
	1 ? vp : c_cp; 	// tripl: common: const void *
	1 ? ip : c_ip; 	// tripl: common: const int *
	1 ? vp : ip; 	// tripl: common: void *
}
