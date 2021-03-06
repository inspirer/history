/*   types.h
 *
 *   C Compiler project (cc)
 *   Copyright (C) 2002-03  Eugeniy Gryaznov (gryaznov@front.ru)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef types_h_included
#define types_h_included

#define DEBUG_TREE

#define TYPE(tt) ((struct type *)(tt))
#define SYM(tt) ((struct sym *)(tt))

#define SUBSET(a,b) (((a)|(b))==(b))

#define T(tt) (tt)->specifier
#define Q(tt) (tt)->qualifier
#define M(ss) (ss)->ns_modifier


// this union represents any value

union Value {
	llong i;
	ullong u;
	long double d;
	void *p;
	void (*g)(void);
};


// type qualifiers
enum {
    tq_none     = 0,
    tq_const    = 1,
    tq_volatile = 2,
    tq_restrict = 4,
};

// function specifiers
enum {
    fs_inline   = 1,
};

/*
	EG: modifiers mark is used to store in one integer the type of modifier and
	the modifier itself. We have three types of modifiers 'storage class specifiers', 
	'type qualifiers' and 'function specifiers'. See type::add_modifier.
*/

// modifiers mark
enum {
    mod_storage = 0x100,
    mod_qual    = 0x200,
    mod_func    = 0x300,
};


// type specifiers
enum {
    // basic types
    t_void   = 1,      // void
    t_char   = 2,      // char
    t_schar  = 3,      // signed char
    t_uchar  = 4,      // unsigned char 
    t_short  = 5,      // short, signed short, short int, or signed short int
    t_ushort = 6,      // unsigned short, or unsigned short int
    t_int    = 7,      // int, signed, or signed int
    t_uint   = 8,      // unsigned, or unsigned int
    t_long   = 9,      // long, signed long, long int, or signed long int
    t_ulong  = 10,     // unsigned long, or unsigned long int
    t_llong  = 11,     // long long, signed long long, long long int, or signed long long int
    t_ullong = 12,     // unsigned long long, or unsigned long long int
    t_float  = 13,     // float
    t_double = 14,     // double
    t_ldouble= 15,     // long double
    t_bool   = 16,     // _Bool
    t_fcompl = 17,     // float _Complex
    t_dcompl = 18,     // double _Complex
    t_lcompl = 19,     // long double _Complex
    t_fimg   = 20,     // float _Imaginary
    t_dimg   = 21,     // double _Imaginary
    t_limg   = 22,     // long double _Imaginary
    t_basiccnt = 23,

#define BASICTYPE(tt) ( (tt)->specifier > 0 && (tt)->specifier < t_basiccnt )

	// 6.2.5.17 The type char, the signed and unsigned integer types, and the
	// enumerated types are collectively called integer types.

#define INTTYPE(tt) ((tt)->specifier >= t_char && (tt)->specifier <= t_ullong ||\
	(tt)->specifier == t_enum )

#define STRUCTTYPE(tt) ((tt)->specifier == t_struct || (tt)->specifier == t_union )
#define VOIDTYPE(tt) ((tt)->specifier == t_void )
#define PTRTYPE(tt) ((tt)->specifier == t_pointer )
#define FLOATTYPE(tt) ((tt)->specifier >= t_float && (tt)->specifier <= t_limg && (tt)->specifier!= t_bool )

	// 6.2.5.18 Integer and floating types are collectively called arithmetic
	// types. Each arithmetic type belongs to one type domain: the real type domain
	// comprises the real types, the complex type domain comprises the complex types.

#define ARITHMETIC(tt) (INTTYPE(tt) || FLOATTYPE(tt))

	// 6.2.5.21 Arithmetic types and pointer types are collectively called scalar
	// types. Array and structure types are collectively called aggregate types.

#define SCALAR(tt) ( PTRTYPE(tt) || ARITHMETIC(tt) )
#define AGGREGATE(tt) ( (tt)->specifier == t_struct || (tt)->specifier == t_union || \
	(tt)->specifier == t_array )

	// 6.2.5.22 An array type of unknown size is an incomplete type. It is 
	// completed, for an identifier of that type, by specifying the size in a 
	// later declaration (with internal or external linkage). A structure or 
	// union type of unknown content (as described in 6.7.2.3) is an incomplete
	// type. It is completed, for all declarations of that type, by declaring the
	// same structure or union tag with its defining content later in the same scope.

#define INCOMPLETE(tt) ( (tt)->specifier == t_array && (tt)->ar_size_val == 0 || \
	((tt)->specifier == t_union || (tt)->specifier == t_struct) && (tt)->params == NULL )

#define VARSIZE(tt) ( (unsigned)(tt)->ar_size_val == -1 )

    /*
		EG: To simplify the compiler's logic we suppose that 'long _Imaginary' is the
		short form of 'long double _Imaginary' and 'long _Complex' is the short form
		of 'long double _Complex'. See validate_type func.
    */

    // aggregate and other types
    t_typename = 0x40,
    t_struct   = 0x41,
    t_union    = 0x42,
    t_array    = 0x43,
    t_func     = 0x44,
    t_pointer  = 0x45,
    t_enum     = 0x46,

    // namespace modifier, used to store labels in namespaces
    t_label    = 0x47,

	// bad symbol mark
	t_bad	   = 0x50,

    // type mask
    t_mask     = 0xff,

    /*
		EG: During syntax analysis it's easier to use word representation of type, but
		as soon as the type is collected, the type::fixup function transforms word-
		representation to normal-representation. t_mask constant allows to detect the
        model of representation. To work with word-representation there are type::add
        and validate_type functions.
    */

    // basic words
    ts_void   = 0x100,
    ts_char   = 0x200,
    ts_int    = 0x400,
    ts_float  = 0x800,
    ts_double   = 0x1000,
    ts_signed   = 0x2000,
    ts_unsigned = 0x4000,
    ts_short    = 0x8000,
    ts_long     = 0x10000,
    ts_llong    = 0x20000,
    ts__Bool    = 0x40000,
    ts__Complex = 0x80000,
    ts__Imaginary = 0x100000,
};


/* EG:
	size member declares the number of bytes needed to hold the variable
	of such type in memory. size = 0 for function and void types.

	ar_size_val = -1 for variable sizes: see ar_size, VARSIZE

	structure stores its members in 'params' list, but for fast access we
	use 'members' namespace (each structure has its own namespace).

	enumeration has a pointer to its members - enum_members

	! qualifier for functions stores function_specifier
*/


struct type {
    int    specifier, qualifier;	// see T(), Q()
	word   size, align;
	char   *tagname;				// t_struct, t_union, t_enum
	union {
		Type parent, return_value;
	};
	Type  cloned;

    // compiler's stages dependent vars
	union {
		struct {					// stage 1 (syntax analysis)
			int storage;
		} st1;
	};

	// specifier dependent vars
    union {
		struct {				// t_func, t_struct, t_union
			Sym params;			// for t_struct, t_union NULL means incompleted
			Namespace *members; // members structure is fast access to params
		};
		struct {				// t_array (ar_size_val == 0 means incompleted)
			Expr ar_size;		//						== -1 means variable, see ar_size
			int ar_size_val;
		};
		Namespace *enum_members; // t_enum
    };

	// toString operator uses two static buffers
	const char *toString() const;

	// type conversions
	int can_convert_to( Type t, Compiler *cc ) const;
	Type integer_promotion( Compiler *cc ) const;

	// utilities
	static Type compatible( Type t1, Type t2, int qualif, Compiler *cc );

	// constructors
    static type *create( int ts, Compiler *cc );
	type *clone( Compiler *cc ) const;

    static Type create_function( Sym paramlist, Compiler *cc );
    static Type create_struct( char *name, int agg_type, sym *members, Namespace *mm, Place loc, Compiler *cc );
    static Type create_enum( char *name, Namespace *ns, Place loc, Compiler *cc );
    static Type create_array( int tqualifiers, expr *expr, Compiler *cc );
	static Type get_basic_type( int spec, Compiler *cc );

    // grammar-based operations
    void add( int ts, Place loc, Compiler *cc );
    void add_modifier( int modifier, Place loc, Compiler *cc );
    void fixup( Compiler *cc );
    static int addqualifier( int list, int one, Place loc, Compiler *cc );

    int asOP() const;
};

//  Basic type information array declaration.

struct basic_type_descr {
	int size_name;			// _c _s _i _l _h _f _d _x _p
	int suffix;				// F I U P V B
    int rank;				// type rank
    int domain;				// only for floating types
    const char *name;
};

// (see 6.3.1.1 Boolean, characters, and integers)

// TODO improve enum_type
#define INTRANK 4
#define SBIT 0x1000
#define RANK(type) ((type)->specifier == t_enum ? INTRANK : tdescr[(type)->specifier].rank&0xff)
#define SIGNED(type) (tdescr[(type)->specifier].rank&SBIT)
#define F_DOMAIN(a) ((a) >> 4)
#define F_TYPE(a) ((a) & 15)

extern const struct basic_type_descr tdescr[t_basiccnt];

/* EG:
	sym structure is used to store all symbolic names found in the source file.
	Be careful, we also store typenames/struct/enums here. ns_modifier represents the
	type of symbol.

	st1.type_tail member is used only during syntax analysis, the type of symbol
	is constructed step by step, adding the new information to the end of 
	'type list'. see sym::symbol_created and sym::addtype functions

	name member can be NULL, it means abstract declarator

	the structure is initialized in sym::create function

	type_tail is NULL by default, while next_tail point out the current Symbol (by default)
 */

/* EG: To combine several namespaces in one, we use ns_modifier member of sym structure.

	0 - usual, t_struct, t_union, t_enum, t_typename, t_label, t_bad

	 6.2.3 Name spaces of identifiers (ns_modifier)
	  1. ..., there are separate name spaces for various categories of identifiers,
	  as follows:
		 - label names (t_label)
	     - the tags of structures, unions, and enumerations (t_enum, t_struct, t_union)
	     - the members of structures or unions; each structure or union has a separate name
			space for its members (separate ns)
		 - all other identifiers, called ordinary identifiers (declared in ordinary
		   declarators or as enumeration constants). (ns_modifier=0)
*/


// storage class specifiers (see debug.cpp)
enum {
    scs_none     = 0,
    scs_extern   = 1, 
    scs_static   = 2, 
    scs_auto     = 3, 
    scs_register = 4, 
    scs_typedef  = 5,
	// extra
    scs_imm      = 7, 
	scs_bitfield = 8,
	scs_member   = 9,
	scs_arg      = 10,
	scs_arg_reg  = 11,
	scs_label	 = 12,
	scs_code	 = 13,
	scs_max      = 14,
};


/* 
    symbol_created is the end of Symbol creation mechanism, it accepts
    fix argument:
*/

enum {
	fix_none   = 0,
	fix_global = 1,
	fix_infunc = 2,
	fix_member = 3,
	fix_param  = 4,
	fix_func   = 5
};

struct sym {
    char *name;
    int storage, ns_modifier;
    Type type;
    Sym next, hashed;

	// storage dependent vars
    union {
		char *exportname;	// scs_extern
    	Init *init;			// scs_static, scs_global

    	Value val;			// scs_imm
    	Node label_node;	// scs_label
    	
    	struct {			// scs_arg, scs_arg_reg
			int offset;
			Value def;
    	} arg;

    	struct {			// scs_member, scs_bitfield
			Sym container;		
			int bitsize, bitstart;  // bitsize = -1 for usual types
			int offset;
		} member;

		Node body;			// scs_code
    } loc;

    // compiler's stages dependent vars
    union {
		struct {			// syntax analysis
			Type type_tail;
			Sym  next_tail;
		} st1;

		struct {			// code generation
			char *name;

		} x;
    };

	// constructors
    static sym *create( char *s, Compiler *cc );
    static sym *create_imm( char *s, Type t, ullong i, Compiler *cc );

	// registers sym in the outer namespace, returns INIT Nodes
    Node register_self( Place loc, Compiler *cc );
	void declare_function( Node statement, Namespace *outer, Place loc, Compiler *cc );

    // grammar-based operations
    void symbol_created( Type t, int fix, Place loc, Compiler *cc );
    void addtype( Type t );
    void addnext( sym *s );
};


#define BAD(sym) ( sym->ns_modifier == t_bad )
#define GOOD(sym) ( sym->ns_modifier != t_bad )
#define makebad(sym) sym->ns_modifier = t_bad;

/*
	EG: We have 'global' namespace and separate namespace for each function/structure/enum.
	Later I plan to improve namespaces.

	associated member points to Namespace's Node if the last one exists
*/

struct Namespace {
    Namespace *parent;
	//Node associated;
    sym **hash;

    Namespace();
    ~Namespace();
    void add_item( Sym i );
    Sym search_id( const char *name, int modifier, int go_deep );

    /*
		EG: entering to and exiting from namespaces (in slab)
    */
    void *operator new( unsigned int size, void *place ) { return place; }
    void newns( Compiler *cc );
    static void close_ns( Compiler *cc );
    void export_outer();
};


/*  
   EG:
		Here goes debug functions declarations. All functions printf
		debug information to stdout.
*/
#ifdef DEBUG_TREE

void debug_show_namespace( Namespace *ns, int deep );
void debug_show_sym( Sym s, int deep );
void debug_show_type( Type s, int deep );

#endif


#endif
