// sample.c

#include <stdio.h>
#include <stdlib.h>

void main()
{
    double f = 1.789;
	FILE *p;
	p = fopen("rt","wt");
    fprintf(p,"%g\n",f);
    fclose(p);
    _ShowMemoryLeaks();
}
