// input.cpp

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "input.h"
#include "defs.h"

filestream::filestream()
{
    p = NULL; name = NULL;
}

int filestream::read(void *where,int max)
{
    if( p ) return fread(where,1,max,p);
		else return 0;
}

int filestream::open(char *fname)
{
    if( p ) close();
    if((p=fopen(fname,"r"))){
        name = new char[strlen(fname)+1];
        strcpy(name,fname);
        return 1;
    }
    return 0;
}

void filestream::close()
{
    if( p ) fclose( p );
    if( name ) delete[] name;
    p = NULL;name = NULL;
}

int consolestream::read(void *where,int max)
{
	if( feof(stdin) ) return 0;
	return fread(where,1,max,stdin);
}
