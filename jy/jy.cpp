// jy.cpp

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "lex.h"
#include "linker.h"

#include <crtdbg.h>

int main(int argc,char *argv[])
{
	stream *in;
	static char *infiles[65];
	int i,infilescount = 0;
	NodeSeq prg = { NULL, NULL };
	Node * module;

	lnk.output_file = NULL;
	lnk.binary = 0;

	for( i=1; i<argc; i++){
		if( argv[i][0] == '/' || argv[i][0] == '-' && argv[i][1] != 0){
			if( !strcmp(argv[i]+1,"c"))
				lnk.flags.compile = 1;
			else if( !strcmp(argv[i]+1,"o")){
				if( lnk.output_file ) lnk.error(E_SECONDPARAM,-1,argv[i]);
				else if( i+1<argc ) lnk.output_file = _strdup(argv[++i]);
				else lnk.error(E_INVALIDPARAM,-1,argv[i]);
			} else lnk.error(E_INVALIDPARAM,-1,argv[i]);
		}
		else infiles[infilescount++] = _strdup(argv[i]);
		if( infilescount == 65 ){ lnk.error(E_TOOMINPUT,-1);return 0; }
	}

	lnk.init();
	if( !lnk.output_file ){
		if( infilescount ){
			char *p,*l,*m;
			l = NULL; m = p = infiles[0];
			while( *p ) switch( *p++ ){
				case '\\': case '/': m = p; l = NULL; break;
				case '.': l = p-1; break;
			}
			if( !l ) l = p;
			i = l - m;
			if( i ){
				lnk.output_file = new char[i+5];
				strncpy(lnk.output_file,m,i);
				strcpy(lnk.output_file+i,(lnk.flags.compile)? ".of":".exe");
			} else lnk.output_file = _strdup((lnk.flags.compile)? "output.of":"output.exe");
		} else lnk.output_file = _strdup((lnk.flags.compile)? "output.of":"output.exe");
	}
	if( !infilescount ){ infilescount = 1; infiles[0] = NULL; }

	for( i=0; i<infilescount; i++){
		lnk.current_file = infiles[i];

		if( lnk.current_file && *lnk.current_file!='-' && *lnk.current_file!=0 ){
			filestream *fs = new filestream;
			if (!fs->open(lnk.current_file)){
				lnk.error(E_FILENOTFOUND,-1,lnk.current_file);
				return 0;
			}
			in = fs;

		} else {
			consolestream *cs = new consolestream;
			if (lnk.current_file) delete lnk.current_file;
			cs->name = lnk.current_file = infiles[i] = _strdup("stdin");
			in = cs;
		}

		lnk.lex.start(in);
		lnk.compile();
		lnk.lex.close();
		in->close();
		delete in;

		if( lnk.program ){
			module = new Node;
			module->type = NODE_COMPOUND;
			module->line = 0;
			module->file = lnk.current_file;
			module->compound = lnk.program->first;
			delete lnk.program;
			module->holder = NULL;

			if( prg.first )
				prg.last = prg.last->next = module;
			else
				prg.last = prg.first = module;
		}
	}

	if( !lnk.errors && !lnk.fatals && prg.first ) 
		if( lnk.flags.compile ) lnk.outtree(prg.first);
			else lnk.link(&prg);

	lnk.clear(&prg);
	for( i=0; i<infilescount; i++ ) if( infiles[i] ) delete infiles[i];
	delete lnk.output_file;

	_CrtDumpMemoryLeaks();
    return 0;
}
