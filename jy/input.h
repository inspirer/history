// input.h

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "defs.h"

class stream {
public:
    virtual int read(void *where,int max)=0;
    virtual void close()=0;
    char *name;
};

class filestream : public stream {
public:
    int read(void *where,int max);
    int open(char *fname);
    void close();
    filestream();
private:
    FILE *p;
};

class consolestream : public stream {
public:
    int read(void *where,int max);
    void close(){};
};

#endif
