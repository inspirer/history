// sample2.c

#include <ctype.h>
#include <windows.h>
#include <stdio.h>

void main()
{
	int i;
    int a,b;
	char c;
	char s[10];
	float e1,e2;
	FILE * out = fopen("myf","w");
	fprintf(out,"the file was opened");	
	printf("enter integer number :");
	scanf("%d",&a);
	printf("enter another integer number :");
	scanf("%i",&b);
	printf("a * b = %d\n",a*b);
	printf("enter float number :");
	scanf("%e",&e1);
	printf("enter another float number :");
	scanf("%e",&e2);
	printf("%he * %he = %he",e1,e2,e1*e2);
	printf("\nenter string");
	scanf("%s",s);
	printf("here is your string :%s\nto exit press any button",s);
	
	scanf("%c",&c);
	fclose(out);	
}
