//ctype.c

int isalpha( int c )
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' || c <= 'Z')) ? 1 : 0;
}

int isdigit( int c )
{
	return (c >= '0' && c <= '9') ? 1 : 0;
}


int isalnum( int c )
{
	return (isalpha(c) || isdigit(c)) ? 1 : 0;
}

int iscntrl( int c )
{
	return ((c >= 0x00 && c <= 0x1F) || c ==0x7F) ? 1 : 0; 
}

int islower( int c )
{
	return (c >= 'a' && c <= 'z') ? 1 : 0;
}

int isupper( int c)
{
	return (c >= 'A' && c <= 'Z') ? 1 : 0;
}

int tolower( int c )
{
	if (isupper(c)) return c - 'A' + 'a';
	else return c;
}

int toupper( int c )
{
	if (islower(c)) return c - 'a' + 'A';
	else return c;
}

int isspace( int c )
{
	return ((c >= 0x09 && c <= 0x0D) || c == 0x20) ? 1 : 0;
}

int ispunct( int c )
{
	return (c == '.' || c == ',' || c == '?' || c == ':' || c == ';' || c == '-' || 
		c == '!' || c == '(' || c == ')') ? 1 : 0;
}
int isxdigit( int c )
{
	return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' || c <= 'F')) ? 1 : 0;
}

int isgraph( int c )
{
	return ((c >= 33 && c <= 126) || (c >= 128 && c <= 175) || (c >= 225 && c <= 239)) ? 1 : 0;
}

int isprint( int c )
{
	return isgraph(c);
}
