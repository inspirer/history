//  errlist.h

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

//	 syntax:  err( E_<errorID>,  <E_FATAL|E_WARN|E_ERROR>, <error text> )

err( E_NOTIMPLEMENTED,	E_FATAL,	"not implemented: %s" )
err( E_READFATAL,		E_FATAL,	"reading error" )
err( E_FILENOTFOUND,	E_ERROR,	"file `%s' not found" )
err( E_FILENOTCREAT,	E_FATAL,	"file `%s' not created" )
err( E_INVALIDPARAM,	E_ERROR,	"invalid parameter: %s" )
err( E_SECONDPARAM,		E_ERROR,	"second use of %s" )
err( E_TOOMINPUT,		E_ERROR,	"64 input files max" )

err( E_UNKNWNSYM,		E_ERROR,	"unknown symbol `%c`" )
err( E_UNKNWNSYMCODE,	E_ERROR,	"unknown symbol 0x%02x" )
err( E_TOOLONGID,		E_ERROR,	"too long identifier" )
err( E_OVERFLOW,		E_ERROR,	"integer constant overflow" )
err( E_INVALIDHEX,		E_ERROR,	"empty hexadecimal constant" )
err( E_WRONGCONST,		E_ERROR,	"wrong constant" )
err( E_LONGSTR,			E_ERROR,	"too long string" )
err( E_NOTFIN,			E_ERROR,	"ending \" missing for string literal" )
err( E_NOTFIN2,			E_ERROR,	"ending \' missing for character literal" )
err( E_WRONGESC,		E_ERROR,	"wrong escape sequence" )

err( E_SYNTAX,			E_ERROR,	"syntax error" )
err( E_EOPARSE,			E_FATAL,	"parse error at end of input" )

err( E_SYMALREADYEX,	E_ERROR,	"symbol `%s` already defined" )
err( E_SYMNOTFOUND,		E_ERROR,	"symbol `%s' not found" )
err( E_NOTSCOPE,		E_ERROR,	"symbol `%s' does not represent nor section, nor scope" )
err( E_TOOMUCHPARAMS,	E_ERROR,	"too much parameters, skipped" )
err( E_CODETYPEUNKNOWN, E_ERROR,    "unknown code type, unchanged" )
err( E_SEG_ALR_PRES,	E_ERROR,	"segment register for this opcode already defined" )
err( E_UNKN_MEMMOD,		E_ERROR,	"unknown memory modifier `%s'" )
err( E_2_MEMMOD,		E_ERROR,	"two memory modifiers" )
err( E_2_INDEXREG,		E_ERROR,	"two index registers" )
err( E_WRONGINDEX,		E_ERROR,	"wrong register used as index: %s" )
err( E_WRONGBASE,		E_ERROR,	"wrong register used as base: %s" )
err( E_ALR_BASEANDIND,  E_ERROR,    "base and index registers already present" )
err( E_WRONGMEMADDR,	E_ERROR,	"composing 16 and 32 bit addressing is forbidden" )
err( E_WRONGMULT,		E_ERROR,	"wrong multiplier, only 1,2,4 and 8 supported" )
err( E_SECONDUSE,		E_ERROR,	"register %s was used twice" )

err( E_INVALIDOPC,		E_ERROR,	"invalid opcode: %s" )
err( E_INTERNAL,		E_ERROR,	"internal error" )
err( E_NULLREF,			E_ERROR,	"empty memory reference" )
err( E_ESPINDEX,		E_ERROR,	"esp cannot be used as index" )
err( E_CALCOUTRANGE,	E_ERROR,	"calculation-time value `%i' is out of range (0..31)" )

err( E_TWOSEC_CALL,		E_ERROR,	"section %s was called twice" )
err( E_SECT_LOOP,		E_FATAL,	"loop found in sections near %s" )
err( E_NOSEC_CALL,		E_ERROR,	"no section call for %s" )
err( E_NOID,			E_ERROR,	"identifier not found: %s" )
err( E_NOSHARP,			E_ERROR,	"%c cannot be used in global expressions" )
err( E_IMMDISPOUT,		E_ERROR,	"%s is out of range" )
