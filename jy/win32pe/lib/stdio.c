// stdio.c

#include <math.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <errno.h>	   

#define __BWRITE 0x001
#define __BREAD  0x002
#define __NOTDEF 0x004

char __tmp_name[10000];
FILE *files[FOPEN_MAX];
FILE *stdin, *stdout,*stderr;

int __movep (FILE *stream,int flag );
void __init_buf(FILE *f,int fl);
int __convert_to_arg(int flagfile,const char **buf,const char **format,va_list *arglist, FILE *f,int *num);
double _pow_ten(int k);
 
void _stdio_start() 
{
	int i;
	for (i = 0; i < FOPEN_MAX; i++) files[i] = NULL;
	__tmp_name[0] = 0;
	stdin = malloc(sizeof(FILE));
	stdout = malloc(sizeof(FILE));
	stderr = malloc(sizeof(FILE));
	
	stdin->file_descr = _get_std_handle(0);
	stdout->file_descr = _get_std_handle(1); 	
	stderr->file_descr = _get_std_handle(2); 
	
	stdin->flag = _F_READ;
	stdout->flag = _F_WRITE;
	stderr->flag = _F_WRITE;
	
	setvbuf(stdin,0,0,BUFSIZ);
	setvbuf(stdout,0,0,BUFSIZ);
	setvbuf(stderr,0,0,BUFSIZ);
	
	stdin->flag &= ~_F_B_NOTDEF; 
	stdin->flag |= _F_B_READ;
	stdout->flag &= ~_F_B_NOTDEF;
	stdout->flag |= _F_B_WRITE | _F_APPEND;
	stderr->flag &= ~_F_B_NOTDEF;
	stderr->flag |= _F_B_WRITE | _F_APPEND;

	stdin->b_first = stdin->b_last = 0;
	stdout->b_first = stdout->b_last = 0;
	stderr->b_first = stderr->b_last = 0;
	
    asm "section .init { call __stdio_start; }";
}

void _stdio_end() 
{
	int i;
	for (i = 0; i < FOPEN_MAX; i++)
		if (files[i]) fclose(files[i]);		
	free(stdin);
	free(stdout);
	free(stderr);
	asm "section .finalize { call __stdio_end; }";
}

FILE * fopen( const char *name, const char *mode )
{
    
	int fl=0,t=0,s=0,i=0,wr=0;
	unsigned int error;
	DWORD d=1;
    FILE * f;

    if ( name == NULL || mode == NULL ) {
		errno = EINVAL;
		return NULL;
	}
    for ( ; i<2; i++, mode++ )  
        switch (*mode) {
            case 'w': 
                if (d==0) break; else d=0;
                fl |= _F_WRITE; 
                t  |= _O_CREAT | _O_TRUNC;
                if ( *(mode+1) == '+' ){
                    fl |= _F_READ | _F_MOVEP;
                    t  |= _O_RDWR;        
                    mode++;
                } else {
                    t  |= _O_WRONLY;
                    wr=1;
                }
                break;
            case 'r':
                if (d==0) break; else d=0;
                fl |= _F_READ;
                if ( *(mode+1) == '+' ) {
                    fl |= _F_WRITE | _F_MOVEP;
                    t  |= _O_RDWR ; 
                    mode++;
                } else {
                    t |= _O_RDONLY;
                    wr=2;
                }
                break;
            case 'a':
                
                if (d==0) break; else d=0;
                fl |= _F_WRITE | _F_APPEND; 
                t  |=_O_APPEND;
                if ( *(mode+1) == '+' ) {
                    fl |= _F_READ | _F_MOVEP;
                    t  |= _O_RDWR  | _O_CREAT;
                    mode++;
                } else {
                    t |= _O_WRONLY;
                    wr=1;
                }
                break;
            case 'b':  
                fl |= _F_BINARY; 
                break;
            case 't':  
                t  |= _O_TEXT;
                break;
        }
    
    if ( (fl & (_F_WRITE | _F_READ) ) == 0 ) {
		errno = EINVAL;
		return NULL;
	}
    
    s = (t == _O_RDONLY) ? 1 : 0;

    d=open(name,t,s);
    if (d==0) {
		if ((error =_get_last_error()) == ERROR_CANT_ACCESS_FILE)
			errno = EACCES;
		else {
			if ( error ==  ERROR_FILE_NOT_FOUND ) errno = ENFILE;
			else errno = EFAULT;
		}
		return NULL;  
	}
    f = (FILE *) malloc (sizeof (FILE));
    if (!f) {
		errno =  ENOMEM;
        close (d);
        return NULL; 
    }
    f->file_descr=d;
    f->flag=fl;   
    for (i = 0; ((i<FOPEN_MAX) && (files[i])) ; i++);
	if (i == FOPEN_MAX) {
		errno = EMFILE;
		free(f);
		close(f->file_descr);
		return NULL;
	}
	files[i] = f;	
    f->buf= malloc (BUFSIZ);
    if (f->buf) {
        f->bufsiz=BUFSIZ;
        f->flag |= _F_BUF|_F_BUFSYS;
        f->b_first=f->b_last=0;
        if (wr == 1) f->flag |=_F_B_WRITE;     
            else f->flag |= (wr == 2) ? _F_B_READ : _F_B_NOTDEF;                       
    }
    return f;  
} 
	

int fsetpos( FILE *stream, fpos_t *pos )
{
   int k;
   clearerr(stream);
   if (!stream) {
	   errno = EINVAL;
	   return 1;
   }
   k = fseek(stream,*pos,SEEK_SET);
   if (!k) {
		k = _get_last_error();
		if ((k == ERROR_ACCESS_DENIED) || (k == ERROR_INVALID_HANDLE)) errno = EBADF;		
		k = 0;
   } 
   return k;
}

long ftell( FILE *stream )
{
	if (!stream) {
		errno = EINVAL;
		return -1L;
	}
    if ((stream->flag & _F_BUF) && (!(stream->flag & _F_B_NOTDEF)))
        if (stream->flag & _F_B_WRITE) fflush(stream); 
        else  __movep(stream,__NOTDEF);
    stream->b_first = stream->b_last = 0;
	return tell(stream->file_descr);
}

int fgetpos( FILE *stream, fpos_t *pos )
{
    long l;
    if (!stream) {
		errno = EINVAL;
		return -1L;
	}
	l=ftell(stream);
    if (l<=-1L) {
		errno = EBADF;
		return -1;
	}
    *pos=l;
    return 0;
}

int feof( FILE *stream ) 
{	
	return (stream->flag & _F_EOF) ? 1 : 0;
}

void clearerr( FILE *stream )
{
	stream->flag &=~(_F_ERROR | _F_EOF);
}

void rewind( FILE *stream )
{
	clearerr(stream);
	if (stream->flag & _F_BUF) {
		if (stream->flag & _F_B_WRITE) fflush(stream);
		if (stream->flag & _F_B_READ) __movep(stream,0);
	}
	lseek( stream->file_descr,0,SEEK_SET);
}


int fseek( FILE *stream, long offset, int origin )
{
    clearerr(stream);
	if (!stream) {
		errno = EINVAL;
		return -1;
	}
    if ((stream->flag & _F_BUF) && (!(stream->flag & _F_B_NOTDEF))) 
        if (stream->flag & _F_B_WRITE) fflush(stream);
        else  __movep(stream,__NOTDEF);          
    if (lseek(stream->file_descr,offset,origin)!=-1)return 0;
    return -1;
}

int __get_next(FILE * f) 
{	
	int i;
	
	l:
	if (f->b_first >= f->b_last) {
		if (!(f->b_last = read(f->file_descr,f->buf,f->bufsiz))) {
			f->flag |= _F_EOF | _F_ERROR;
			return EOF;
		}
		f->b_first = 0;
	}
	if (f->buf[f->b_first]!='\r') 
		i = f->buf[f->b_first++];	
	else {
		f->b_first++;
		goto l;
	}
	return i;
}


size_t fread( void *buffer, size_t size, size_t count, FILE *stream )
{
	int i,j,n,h;
	char *buf=buffer;
	if ((!stream) || (!(stream->flag & _F_READ)) || (size<=0) || (count<=0)) {
		errno = EINVAL;
		return 0;
	}
	if ((stream->flag & _F_B_MUSTNEX) || ( (!(stream->flag & _F_BUF)) &&
		(!(setvbuf(stream,0,_IOFBF,BUFSIZ))) )){
		i = read(stream->file_descr,buffer,size * count);
		if ((i = i/size) != (int)count ) stream->flag |= _F_EOF | _F_ERROR;
		return i;
	}
    if (stream->flag & _F_B_NOTDEF) __init_buf(stream,0);
    if (stream->flag & _F_B_WRITE) { 
		fflush(stream);
		stream->flag &=~_F_B_WRITE;
		stream->flag |=_F_B_READ;
	}
	if (stream->flag & _F_B_READ) {
		i=0;			//уже прочитано
		n=size*count;	// должно быть прочитано
		if (!(stream->flag & _F_BINARY)) {
			for (i = 0; i<n; i++) if ((buf[i] = __get_next(stream)) == EOF) { 
				stream->flag |= _F_EOF | _F_ERROR;
				break;
			}
			return i/size;
		}
		while (i<n) {
			if ((stream->b_last - stream->b_first) >= (h=n-i)) { 
				//h - необходимо еще прочитать
				j=stream->b_first;
                for (;j<stream->b_first+h;j++,i++)                
                    buf[i]=stream->buf[j];
				stream->b_first+=h;
				return count;
			}
			for (j=stream->b_first;j<stream->b_last;j++,i++) buf[i]=stream->buf[j];
            stream->b_last=read(stream->file_descr,stream->buf,stream->bufsiz);
            stream->b_first=0;
            if (stream->b_last == 0) {
				stream->flag |= _F_EOF | _F_ERROR;
				return i/size;
			}
		}
	}
	return 0;
}

int fputs( const char *string, FILE *stream )
{
	unsigned int k,l;	  
    if ((!stream) || (!(stream->flag & _F_WRITE))){
		errno = EINVAL;
		return EOF;
	}
    if ((l =fwrite(string,1,(k=strlen(string)),stream))!=k) {
		errno = (l > 0) ? EACCES : ENOMEM;
		stream->flag |= _F_ERROR;
		return EOF;    
	}
    return (int)k;
}

int puts(const char *s) 
{
	int k = fputs(s,stdout);
	fflush(stdout);
	return k;
} 

char *fgets( char *string, int n, FILE *stream )
{
	int i=0;
	char *c=string;
	if ((!stream) ||(!(stream->flag & _F_READ))) {
		errno = EINVAL;
		return NULL;
	}
	if ( (stream->flag & _F_B_MUSTNEX) || 
        ((!(stream->flag & _F_BUF)) && (setvbuf(stream,0,0,0)== -1 )) ) {
        for (i=0; ((i<(n-1)) && (*c!='\n')); c++,i++) ;
		*(++c)=0;
		return string;		        
    }
	if (stream->flag & _F_B_NOTDEF) __init_buf(stream,0);
	if (stream->flag & _F_B_WRITE) { 
		fflush(stream);
		stream->flag &=~_F_B_WRITE;
		stream->flag |=_F_B_READ;
	}

	if (stream->flag & _F_BINARY) {
		while (i<(n-1)) {
			for (; ( (stream->b_first<stream->b_last) && (i<(n-1))&& 
				(stream->buf[stream->b_first]!='\n')); stream->b_first++, i++) 
					string[i]=stream->buf[stream->b_first];
			if (stream->buf[stream->b_first]=='\n') {
				string[i]  ='\n';
				if (stream->b_first<(n-1)) i++;
				stream->b_first++;
				string[i]=0;
				return string;
			}
			if (i==(n-1)) {
				string[i]=0;
				return string;
			}
			stream->b_last=read(stream->file_descr,stream->buf,stream->bufsiz);
	        stream->b_first=0;
			if (stream->b_last == 0) {
				string[i]=0;
				stream->flag |= _F_ERROR;
				return string;
			}
		}
		string[i]=0;
	} else {
		string[0] = __get_next(stream);
		for (c = string,i = 1; (*c!='\n' && *c!=EOF && i < n-1); 
			c++,i++) string[i] = __get_next(stream); 
		if (i < n-1) {
			string[i] = '\n';
			i++;
		}
		string[i] = 0;
	}
	return string; 
}




size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream )
{
    int i,n,h,s,j;
    const char *buf=buffer;
     
    if ( !stream || !(stream->flag & _F_WRITE) || size <= 0 ||
		count <= 0 || !buffer) {
		errno = EINVAL;
		return 0;
	}
    if (stream->flag & 	_F_B_MUSTNEX) {
		i=write(stream->file_descr,buffer,size*count);
		if ((i = i/size) != (int)count) stream->flag |= _F_ERROR;
		return i;
	}
    if (!(stream->flag & _F_BUF)) {
		if (setvbuf(stream,0,_IOFBF,BUFSIZ)) ;
		else {
			i=write(stream->file_descr,buffer,size*count);  
			if ((i = i/size) != (int)count) stream->flag |= _F_ERROR;
			return i;
		}
	}
    if (stream->flag & _F_B_NOTDEF) __init_buf(stream,1);
    if (stream->flag & _F_B_READ) __movep(stream,__BWRITE);
    if (stream->flag & _F_B_WRITE) {
        //n - должно быть записано
        //i - уже записано
        //h - осталось записать 
		n=size*count;
        i=0;
        while (i<n){
            if ( (stream->bufsiz - stream->b_last) >= (h=n-i) ){
                s=stream->b_last+h;
                for (j=stream->b_last; j<s; j++ , i++) 
                    stream->buf[j]=buf[i];
                stream->b_last = s;
                return count;
            }             
            for (j=stream->b_last; j<stream->bufsiz; j++ , i++) 
                stream->buf[j]=buf[i];
			stream->b_last = j;
			if (fflush(stream)){ 
				if ((i = i/size) != (int)count) stream->flag |= _F_ERROR;
				return i;				
            }
		}
    }
    return 0; //выполнение не должно доходить до этой точки
}

int fputc(int ch,FILE *stream) 
{
    int c=ch;   
	if ((!stream) || (!(stream->flag & _F_WRITE))) {
		errno = EINVAL;
		return EOF;
    }
	if ( (stream->flag & _F_B_MUSTNEX) || 
        ((!(stream->flag & _F_BUF)) && (setvbuf(stream,0,0,0)== -1 )) ) {
        if (write(stream->file_descr,&c,1)!=1) {
			stream->flag |= _F_ERROR;		
			return EOF;
		}
        return ch;
    }
    if (stream->flag & _F_B_NOTDEF) __init_buf(stream,1); 
    if (stream->flag & _F_B_READ) __movep(stream,__BWRITE);
    if (stream->flag & _F_B_WRITE) {
        if (stream->b_last == stream->bufsiz) 
            if (fflush(stream)) {
				stream->flag |= _F_ERROR;		
				return EOF;			
			}
        stream->buf[stream->b_last++]= c;
        return ch;
    }
    stream->flag |= _F_ERROR;		
	return EOF;//выполнение не должно доходить до этой точки
}

int putc(int ch) 
{
	int k = fputc(ch,stdout);
	fflush(stdout);
	return k;
} 

// если значение fl == 0 буфер далее будет использован для чтения  
// иначе для письма

void __init_buf(FILE *f,int fl) {
    f->flag &= ~_F_B_NOTDEF;
    f->flag |= fl ? _F_B_WRITE : _F_B_READ;
    f->b_first=f->b_last=0;   
}


int fgetc(FILE *f)
{
    int k;
    char c;
    
	if ((!f) || (!(f->flag & _F_READ))) {
		errno = EINVAL;
		return EOF;
	}
	if ( f->flag & _F_B_MUSTNEX || (!(f->flag & _F_BUF) && 
		(setvbuf(f,NULL,0,BUFSIZ) != -1))){
		read(f->file_descr,&c,1);
		if (c == EOF) f->flag |= _F_EOF | _F_ERROR;		
		return c;
	}
	if (f->flag & _F_BUF){
		if (f->flag & _F_B_WRITE) {
			fflush(f);
			f->flag &= ~_F_B_WRITE;
			f->flag |= _F_B_READ;
		}
        if (f->flag & _F_B_NOTDEF)__init_buf(f,0);
        if (f->flag & _F_B_READ) {
			if (f->flag & _F_BINARY) {
				if (f->b_first == f->b_last) {
					  k = read(f->file_descr,f->buf,f->bufsiz);
					  if (k==0) {
						  f->flag |= _F_EOF | _F_ERROR;
						  return EOF;
					  }
				      f->b_first=1;
					  f->b_last=k;
					  return (int) f->buf[0];
				}
				return f->buf[f->b_first++];
			} else {
				c = __get_next(f);
				return c;
			}
        }    
    } 	
	errno = EFAULT;
	f->flag |= _F_ERROR;
	return EOF;	 //не должно приходить в эту точку
}

// fold - если необходимо взять старый символ
// при этом если необходимо передвинуть указатель - то mp
// mp - передвинуть указатель

int __getc(int flagfile,FILE *f,const char**buf,int mp,int *num)
{	
	int k;
	
	if (flagfile){
		if ((f->flag & _F_BUF) && (!(f->flag & _F_B_MUSTNEX))) {
			l:
			if (f->b_first >= f->b_last) {
				if (!(f->b_last = read(f->file_descr,f->buf,f->bufsiz))){
					f->flag |= _F_EOF | _F_ERROR;
					return EOF;
				}
				f->b_first = 0;
			}
			if (f->flag & _F_BINARY) k = f->buf[f->b_first];
			else 
				if (f->buf[f->b_first] == '\r') {
					f->b_first++;
					goto l;
				} else k = f->buf[f->b_first]; 
			if (mp) {
				f->b_first++; 
				(*num)++;
			} else if (!(*num)) *num = 1;						
		} else 
			if (read(f->file_descr,&k,1)!=1){
				f->flag |= _F_EOF | _F_ERROR;
				return EOF;
			}
			else (*num)++;
	} else {				
		k = **buf;
		if (mp) {
			(*buf)++;
			(*num)++;
		} else if (!(*num)) *num = 1;		
	}
	return k;
}

int setvbuf(FILE *stream,char * buf,int mode,size_t size) 
{
    	
	if (!stream) {
		errno = EINVAL;
		return -1;
	}
    if ((stream->flag & _F_BUF) && (stream->flag & _F_BUFSYS)) {
        free(stream->buf);
        stream->flag &= ~ _F_BUFSYS;
    }
    if (mode & _IONBF){
        stream->flag |= _F_B_MUSTNEX;
        stream->flag &= ~ _F_BUF;
        errno = EFAULT;
		return 0;
    }
    if  (!buf){
        buf = malloc(size);
        if (!buf) return -1;
        stream->flag |= _F_BUFSYS;
    }
    stream->flag |= _F_BUF;
    stream->buf = buf;
    stream->bufsiz = size;
	stream->flag |= _F_B_NOTDEF;
    return 0;       
}

void setbuf( FILE *stream, char *buffer )
{
	if ((stream) && (buffer)) {
		if (stream->flag & _F_BUFSYS) {
			fflush(stream);
			free(stream->buf);
		}
		stream->buf=buffer;
		stream->bufsiz=BUFSIZ;
		stream->flag |= _F_B_NOTDEF;
	}
}

//when want to interrupt the reading buffer
int __movep (FILE *stream,int flag ) 
{
    long pos;	

	if ((!stream) || (!(stream->flag & _F_B_READ))) {
		errno = EINVAL;
		return -1;
	}
  	if (flag & __BWRITE) {
        stream->flag &=~_F_B_READ;
        stream->flag |= _F_B_WRITE;
        if (stream->flag & _F_APPEND) fseek(stream,0,SEEK_END); 
        else {
			pos = -stream->b_last+stream->b_first+tell(stream->file_descr);
            if (lseek(stream->file_descr,pos,SEEK_SET)==-1) return -1;  			
        }
    }
    if (flag & __NOTDEF) {
        pos=-stream->b_last + stream->b_first + tell(stream->file_descr);
        if ((pos = lseek(stream->file_descr,pos,SEEK_SET))==-1) return -1;
        stream->flag |= _F_B_NOTDEF;
        stream->flag &= ~ (_F_B_READ | _F_B_WRITE);
    }
    stream->b_first = stream->b_last=0;	 
    return 0;
}


    
int fflush (FILE *stream) 
{
    int i,k;
	char * ps,*pd,*p;
    
	if (!stream){
        for (i = 0; i<FOPEN_MAX; i++) 
			if (files[i]) fflush(files[i]);    
        return 0;
    }
    if ((!(stream->flag & _F_B_WRITE)) || (stream->b_first == stream->b_last)) return EOF;                     
    if (stream->flag & _F_BINARY) i=write(stream->file_descr,stream->buf,(k = stream->b_last));
	else {		
		for (ps = stream->buf, i=0; ps < &stream->buf[stream->b_last]; ps++) 
			if (*ps == '\n') i++;
		p = pd = malloc(stream->b_last+i);
		for (ps = stream->buf; ps < &stream->buf[stream->b_last]; ps++,pd++) 
			if (*ps == '\n') {
				*pd = '\r';
				pd++;
				*pd = '\n';
			} else *pd = *ps;
	    i = write(stream->file_descr,p,(k = stream->b_last + i));
		free(p);
	}	
    stream->b_last=stream->b_first=0;
    if (i!=k) {
		stream->flag |= _F_ERROR;
		return EOF;
	}
    return 0;
}	

int __get_int(const char *f,int *t) 
{
    int k=0;

    *t=0;
    while ((*f>='0') && (*f<='9')) {
        k*=10;
        k+=*f-'0';
        f++;
        (*t)++;
    }
    return k;
}

int __count_ch(int k)
{
    int i=0;
    
	if (k==0) return 1;
    if (k<0) k*=-1;
    for (; k>0; k/=10, i++);
    return i;
}

int __count_uch(unsigned int k) {
	int i=0;
	
	if (k==0) return 1;
    for (; k>0; k/=10, i++);
    return i;
}

int __int_print(long t,char **buf,int o)
{
    
	char *p=*buf;
    int j=0;
    
	if (t<0) t*=-1;
    if (t==0) {
        if (o==0) {
            **buf='0';
            (*buf)++;            
            return 1;
        }
        return 0;
    }
    j=__int_print(t/10,buf,1);
    **buf='0'+(t%10);
    (*buf)++;   
    return 0;
}

int __uint_print(unsigned long t,char **buf,int o)
{
    
	char *p=*buf;
    int j=0;

    if (t==0) {
        if (o==0) {
            **buf='0';
            (*buf)++;           
            return 1;
        }
        return 0;
    }
    j=__uint_print(t/10,buf,1);
    **buf='0'+(int)(t%10);
    (*buf)++;
    return 0;
}
 
#define __PLUS 1
#define __MINUS 2
#define __PROB 4
#define __SPEC 8
#define __ZIROS 0x10
#define __INFLAGS 0x20
#define __PRECISION 0x40
#define __LFLAG 0x80
#define __HFLAG 0x100

int _convert( const char **format, char **buf, va_list *parg, char *bufhead)
{
   	
	int flag=0x0020,i,g,k,t; 
   unsigned long ut;
   int width=0,deg,wl;
   int prec=-1;
   int *ip;
   char c,ch,*pch, *pc,*pch1;
   int l,cnum;
   long double d,f;

   while (flag & __INFLAGS)
     switch (**format) {
        case '+':   flag |= __PLUS;
                    (*format)++;
                    break;
        case '-':   flag |= __MINUS;
                    (*format)++;
                    break;
        case ' ':   flag |= __PROB;
                    (*format)++;
                    break;
        case '#':   flag |= __SPEC;
                    (*format)++;
                    break;
        case '0':   flag |= __ZIROS;
                    (*format)++;
                    break;
        default:    flag &= ~__INFLAGS;
    }

   if (**format == '*') {
      width=va_arg(*parg,int);
      (*format)++;
   } else {
       width=__get_int(*format,&t);
       *format+=t;
   }


   if (**format == '.') {
      (*format)++;
      flag |=__PRECISION;
      if (**format == '*') {
          (*format)++;
          prec = va_arg(*parg,int);
      } else {
          prec = __get_int(*format,&t);
          *format+=t;
      }
   }

   if (**format=='h') {
       flag |=__HFLAG;
       (*format)++;
   } else {
       if (**format=='l'){
           flag |=__LFLAG;
           (*format)++;
       }
   }   

   switch (**format) {
   case '%':
       *(*buf)++ = '%';
       break;
   case 'c':       
       c = va_arg(*parg,char);
	   if ((width>1) && (!(flag & __MINUS))) 
		   for (; width>1; width--, (*buf)++) **buf = ' ';
	   **buf = c;
	   (*buf)++;
	   width--;
	   for (; width>0; width--,(*buf)++) **buf = ' ';      
       break;
   case 'n':
       ip=va_arg(*parg,int *);
       *ip=*buf - bufhead;      
       break;
   case 'd':
   case 'i':
       if (flag & __LFLAG) {
           t = va_arg(*parg,long);
       } else {
           if (flag & __HFLAG) {
               t=(long)va_arg(*parg,short);
           } else {
               t=(long)va_arg(*parg,int);
           }
       }
       i=__count_ch(t);
       if ((t<0) || (flag & __PLUS) || (flag & __PROB)){ 
           i++;//знак или пробел
           ut=1;
       } else ut=0;
       c = (flag & __ZIROS) ? '0' : ' ';
       if (ut && ( (c=='0') || (flag & __MINUS) || (width<=i)) ) {//знак 0 лев выравнивание
		    if (t<0) **buf='-';
		    else if (flag & __PROB) **buf = ' '; else **buf = '-';
            (*buf)++;            
            width--;
            i--;
            g=0;
       } else g=1;  
       if ((width<=i) || (flag & __MINUS)) {
           __int_print(t,buf,0);
           width-=i;
           i=0;
           c=' ';
       }
       for (; width>i; width--,(*buf)++) **buf=c;
       if (ut && g) {
            if (t<0) **buf='-';
		    else if (flag & __PROB) **buf = ' '; else **buf = '-';
            (*buf)++;          
            width--;
            i--;
            g=0;
       } 
       if (i>0) __int_print(t,buf,0); 
       break;
   case 'u':     
       if (flag & __LFLAG) {
           ut = va_arg(*parg,unsigned long);
       } else {
           if (flag & __HFLAG) {
               ut=(unsigned long)va_arg(*parg,unsigned short);
           } else {
               ut=(unsigned long)va_arg(*parg,unsigned int);
           }
       }
       i=__count_uch(ut);
       c = (flag & __ZIROS) ? '0' : ' ';
       if ((width<=i) || (flag & __MINUS)) {
		   __uint_print(ut,buf,0);
		   width-=i;
		   i=0;
		   c = ' ';
	   }
       for (; width>i; width--,(*buf)++) **buf=c;
       if (i>0) __uint_print(ut,buf,0);
       break;   
   case 'o':
       if (flag & __LFLAG) {
           ut = va_arg(*parg,unsigned long);
       } else {
           if (flag & __HFLAG) {
               ut=(unsigned long)va_arg(*parg,unsigned short);
           } else {
               ut=(unsigned long)va_arg(*parg,unsigned int);
           }
       }  
       if ((prec == 0) && (ut==0)) break; else width = (prec >= width) ? prec : width;
       if (prec!=-1) flag |= __ZIROS;
       pc = pch = malloc(11);    
       for (pch +=10;pch>=pc; pch--) {
           *pch=(char)(ut & 7)+'0';
           ut >>= 3;
       }

       for (pch = pc;((*pch=='0') && (pch<pc+11));pch++);

       k=11 +(int)(pc -pch);    //сколько символов в самом слове     
       c = (flag & __ZIROS) ? '0' : ' ';//в случае, когда width или prec > кол-ва цифр в 
       //восьмиричном представлении числа, этим символом будет заполнено оставшееся место

       if ((width<=k) || (flag & __MINUS)) {           
           if (flag & __SPEC) {
               **buf='0';
               (*buf)++; 		
           }           
           for ( ;pch<pc+11;pch++,(*buf)++)  **buf=*pch;
           width=0;
           k=0;																				         
       } else {    
            k++;            
            for ( ; width>k ; width--, (*buf)++ ) **buf=c;            
            **buf = (flag & __SPEC) ? '0' : c;
            (*buf)++;				        
            for ( ; pch<pc+11;  (*buf)++ ,pch++) **buf=*pch;
       }
	   free(pc);
	   break;
   case 'X': 
   case 'x':
   case 'p':
       if (flag & __LFLAG) {
           ut = va_arg(*parg,unsigned long);
       } else {
           if (flag & __HFLAG) {
               ut=(unsigned long)va_arg(*parg,unsigned short);
           } else {
               ut=(unsigned long)va_arg(*parg,unsigned int);
           }
       }
       if ((prec == 0) && (ut==0)) break; 
       if (!(flag & __PRECISION)) prec = 0;
       pc = pch = malloc(8);    
       for (pch+=7;pch>=pc;pch--) {
           if ((k=(ut & 15)) <= 9) *pch=k+'0'; 
		   else {
			  switch (k) {
				case 10: *pch='a';	break;
				case 11: *pch='b';	break;
				case 12: *pch='c';	break;
				case 13: *pch='d';	break;
				case 14: *pch='e';	break;
				case 15: *pch='f';	break;
			  }
			if (**format=='X') *pch+='A'-'a';		
		   }				
           ut >>= 4;
       }

       for (pch = pc;((*pch=='0') && (pch<pc+8));pch++);

       k=8-(int)(pch - pc); //сколько символов в самом слове
       c = (flag & __ZIROS) ? '0' : ' ';
	   
	   if (flag & __SPEC)  {			   
			if (!(flag & __MINUS)) {			   
				if (prec>k) {
					for (; width>(prec+2); width--, (*buf)++) **buf=c;
					**buf='0';
					(*buf)++;
					**buf= (**format!='X') ? 'x' : 'X';
					(*buf)++;
					for (; prec>k; prec--,(*buf)++) **buf='0';
				} else {
					for (; width>(k+2);(*buf)++) **buf=c;
					**buf='0';
					(*buf)++;
					**buf= (**format=='x') ? 'x' : 'X';
					(*buf)++;				
				}
				width = 0;				
			} else {				
				for (; prec>k+2; prec--, (*buf)++ ,width--) **buf='0';
				**buf='0';
				(*buf)++;
				**buf= (**format=='x') ? 'x' : 'X';				
				(*buf)++;
				width-=2;
			}				   
	   } else {
		   if (!(flag & __MINUS)){
			   if (prec>k) {
				   for (;width>prec;width--,(*buf)++) **buf=c;
				   for (width=prec;width>k;(*buf)++,width--)**buf='0';
			   } else for (; width>k ; width--, (*buf)++) **buf=c;
		   }
	   }	              	        
       
	   for ( ; pch<pc+8; pch++, (*buf)++ ) **buf=*pch;
	   width -=k;
       for ( ; width>0; width--, (*buf)++) **buf = ' ';
	   free(pc);

	   break;
   case 's':	   
	   pc=pch = va_arg(*parg , char *);
	   if (!pch) break;
	   i=0;
	   if (prec>0) for (;((*pc) && (i<prec)); pc++, i++) ;
	   else for (; *pc!=0; pc++,i++);

	   c = (flag & __ZIROS) ? '0' : ' ';		    
	   if (prec <= 0){		   
		   if (!(flag & __MINUS)) 
			   for (; width>i; width--, (*buf)++ ) **buf=c;
		   for (; pch<pc; pch++, (*buf)++ ) **buf=*pch;
		   width-=i;
		   for (; width>0; width--, (*buf)++ ) **buf=' ';	   	   
		} else {
		   if (prec>i) {	
			    if (!(flag & __MINUS))
					for (; width>i; width--, (*buf)++) **buf=c;
				if (width<i) width=0; else width-=i;
				for (k=i; k>0; k--, (*buf)++, pch++) **buf=*pch;				   
				**buf=0;
				(*buf)++;
				for (; width>0; width--, (*buf)++) **buf=' ';				
		   } else {
			   if (!(flag & __MINUS))
				   for (; width>prec; width--, (*buf)++) **buf=c;
			   if (width<prec) width=0; else width-=prec;
			   for (; prec>0; (*buf)++, pch++,prec--) **buf=*pch;
			   for (; width>0; width--, (*buf)++) **buf=' ';
		   }
		}			   	
	   break;
   case 'G':
   case 'g':
   case 'E':
   case 'e':
	   d = (flag & __LFLAG) ? va_arg(*parg,long double) :
		   (long double)va_arg(*parg,double);
	   if (!(flag & __PRECISION)) prec=6;
	   
	   if (!(flag & __LFLAG))	{
       		// g == mlo
		   // i == mhi
		   // k == exponent
		    g = (unsigned long) d;
        	i = *((unsigned long *)(&d)+1);           
            k = (i >> 20 & 0x7ff);
            i &= 0xfffff;
            if( k == 0x7ff){
            	if( g == 0 && i == 0 ) goto infinity;
                else goto nan;
            }            
	   } else {	   
        	g = (unsigned long)d;
        	i = *((unsigned long *)(&d)+1);
            k = *((unsigned short *)(&d)+4);            
            k &= 0x7fff;
            if( k == 0x7fff){
				if( g == 0 && i == 1<<31 ) {
					infinity:			
					if (!(flag & __MINUS)) for (; width>3; width--,(*buf)++) **buf = ' ';
					**buf = 'I';
					(*buf)++;
					**buf = 'N';
					(*buf)++;
					**buf = 'F';
					(*buf)++;
					for (;width>0; width--, (*buf)++) **buf = ' ';
					break;
				} else {
					nan:			
					if (!(flag & __MINUS)) for (; width>3; width--,(*buf)++) **buf = ' ';
					**buf = 'N';
					(*buf)++;
					**buf = 'a';
					(*buf)++;
					**buf = 'N';
					(*buf)++;
					for (;width>0; width--, (*buf)++) **buf = ' ';
					break;	
				}
			}            
	   }


	   if (d<0) {
		   c = '-'; 
		   d *=-1;
	   } else c = '+';
	   
	    pc = malloc(prec+3);
		if (pc == 0) return 0;

	    for (i=0,pch=pc; i<prec+3;i++, pch++) *pch = '0'; 
		if (d>=10) for (cnum=0, f=d; f >=1 ; cnum++, f/=10); else 
			cnum = (d<1) ? 0 : 1;
		
		if (cnum>0) {
			deg = cnum-1;
			if (cnum<=prec+2) {
				for (pch=&pc[cnum], f=d; pch>pc; f/=10, pch--) {
					f = floor(f);					
					*pch+=(int) (f-floor(f/10) * 10);
				}			 				
				for (i=cnum+1, pch = &pc[cnum+1], f=d; i<=prec+2; i++, pch++) {
					f = (f-floor(f))*10;
					*pch += (int)floor(f);
				}									
			} else {
				for(f=d,i=cnum;i>prec+2;i--,f/=10);
				for (pch=&pc[prec+2];pch>pc;f/=10,pch--) {
					f = floor(f);
		 			*pch+=(int) (f-floor(f/10) * 10);
				}
			}							
		} else {
			if (d != 0){
				for(cnum=0,f=d;(int)floor(f)==0;cnum--) f = (f-floor(f))*10;
				pc[1] += (int)floor(f);
				for(pch=&pc[2];pch<=(pc+(prec+2)); pch++){
					f = (f-floor(f))*10;
					*pch += (int) floor(f);
				}
			} else cnum = 0;
			deg = cnum;
		}	
		
	if (i=((**format == 'e') || (**format == 'E')) || (deg<-4 ) || (deg>=prec)) {

		if (((**format == 'g') || (**format == 'G')) && (prec > 0)) prec--; 	
		if (pc[prec+2] > '4'){
			for (pch=&pc[prec+1];*pch=='9'; pch--) *pch='0';
			if (pch == pc) deg++;			
			*pch+=1;
		}		
		if ((**format == 'g') || (**format == 'G')) {
			pch = (pc[0] == '0') ? (pc+prec+1) : (pc+prec+2);
			for (i=prec;((i>0) && (*pch == '0')); i--, pch--);
			prec = i;
		}	
		if (flag & __SPEC) wl = 13; else {
			if ((flag & __PRECISION) || (**format == 'g') || (**format == 'G')) {
				if (prec==0) wl = 6; else wl = 7+prec;
			} else wl = 7+prec;
		}			
		if (k = ((c == '-') || (flag & (__PLUS | __PROB)))) width--;
		if (k && ((width<wl) || (flag & (__ZIROS | __MINUS))) ) {
			if (c == '-') **buf = '-';
			else **buf = (flag & __PROB) ?  ' ' : '+'; 
			(*buf)++;
			k = 0;
		}
		ch = (flag & __ZIROS) ? '0' : ' ';
		if (!(flag & __MINUS))
			for (; width > (wl+1); width--, (*buf)++) **buf=ch;		
		if (k) {
			if (c == '-') **buf = '-';
			else **buf = (flag & __PROB) ?  ' ' : '+'; 
			(*buf)++;
		} else {
			if ((!(flag & __MINUS) && (width>=wl))) {
				**buf = ch; 
				width--;
				(*buf)++;
			}
		}
		width-=wl;
		pch = (pc[0] == '0') ? &pc[1] : pc;
		**buf = *pch;
		(*buf)++;
		pch++;		
		if (wl !=6 ) {
			**buf = '.';
			(*buf)++;
			for (i=0;i<prec;i++, (*buf)++,pch++) **buf = *pch;
		}
		**buf = ((**format == 'e') || (**format == 'g')) ? 'e' : 'E';	
		(*buf)++;
		if (deg >=0) **buf = '+'; else {
			**buf = '-';
			deg*=-1;
		}
		(*buf)++;
		i=**buf = (deg / 100);
		**buf+='0';
		(*buf)++;
		**buf = deg / 10 - i*10 + '0';
		(*buf)++;
		**buf = deg % 10 + '0';
		(*buf)++;
		for (; width>0; width--,(*buf)++) **buf = ' ';	
 		free(pc);
		break;
	}
	if (c == '-') d*=-1;
	free(pc);
   case 'f':
	   if (**format == 'f') {
		    if (flag & __LFLAG) {
            d = va_arg(*parg,long double);
			} else d=(double)va_arg(*parg,double);
			if (!(flag & __PRECISION)) prec=6; 
	   }

	   // g == mlo
	   // i == mhi
	   // k == exponent
	   g = (unsigned long) d;
	   i = *((unsigned long *)(&d)+1);
	   if (!(flag & __LFLAG))	{
       		k = (i >> 20 & 0x7ff);
            i &= 0xfffff;
            if( k == 0x7ff){
            	if( g == 0 && i == 0 ) goto infinity;
                else goto nan;
            }            
	   } else {	   
        	k = *((unsigned short *)(&d)+4);            
            k &= 0x7fff;
            if( k == 0x7fff){
				if( g == 0 && i == 1<<31 ) goto infinity;
				else goto nan;			
			}            
	   }
	   
	   f = (d>=0) ? d : -d;			   
	   for (i=2;f>=1;f/=10, i++); 
	   if (**format != 'f') 
		   if (i-2<=prec) prec-=(i-2); else prec=0;
	   pc = (char *) malloc(i+prec);
	   f = (d>=0) ? d : -d;
	   for (k=0;k<i+prec;k++) pc[k] = '0';
	   for (k=i-2;((f>=1) && (k>=0)) ;k--,f/=10) {
		   f = floor(f);
		   pc[k]=(char) (f-floor(f/10) * 10) + '0';
	   }

	   pc[i-1]='.';
	   f = (d>=0) ? d : -d;
	   g = (prec >15) ? 15 : prec;
	   for (k=0,l=i; k < g ; k++, l++) {
		   f = (f-floor(f))*10;
		   pc[l] = (int)floor(f) + '0';		   
	   }

	   	   
	   if ( floor((f - floor(f))*10) > 4 )  {		   	
			while (1) {			   
			   if (l == i) l-=2; else l--;
			   if (pc[l] != '9') {
				   pc[l]+=1; 
				   break;
			   } else pc[l]='0';			   
		   }
	   }

		if ((pc[0] == '0')&& (i>2)) {
			wl = i-2;
			pch = &pc[1];
		} else {
			if ((**format != 'f')&&(pc[0]!='0')) {
				pc[1] = (**format == 'g') ? 'e' : 'E';
				if (deg>0) pc[2]='+';
				else {
					deg*=-1;
					pc[2]='-';
				}
				deg++;
				wl=deg/100;
				pc[3]=wl+'0';
				pc[4]=deg/10-wl*10+'0';
				pc[5]=deg%10+'0';
				wl = 6;
				prec=0;
			} else wl = i-1;
			pch = pc;		
		}
		 
	    if (prec != 0) wl += (prec+1);
		
		if (!(flag & __SPEC)) {
			if (**format == 'f') {
				for (k=i,pch1=&pc[i]; ((k<i+prec)&&(*pch1 == '0')); k++,pch1++); 
				if (k == i+prec) wl -=prec+1;
			} else {
				for (k=prec ,pch1 = &pc[i+prec-1]; ((*pch1 == '0') && (k>0)); pch1--,k--);
				if ((k == 0)&&(pc[1]!='e')) wl -= (prec+1); else wl -=(prec-k);
				prec = k;
			}
		}
		
		if (k=((flag & __PLUS) || (d<0) || (flag & __PROB))) wl++;
		if (k && ((flag & __MINUS)|| (wl>=width) || (flag & __ZIROS))) {
			if (d<0) **buf = '-';
			else **buf = (flag & __PROB) ? ' ' : '+';
			(*buf)++;
			wl--;
			width--;
			g=0;
		} else g=1;

		c = (flag & __ZIROS) ? '0' : ' ';
		if (!(flag & __MINUS))
			for (; width>wl; width--,(*buf)++) **buf=c;
		if (g && k) {
			if (d<0) **buf = '-';
			else **buf = (flag & __PROB) ? ' ' : '+';
			(*buf)++;
			wl--;
			width--;
		}
		for (g=0; g<wl; g++, pch++, (*buf)++) **buf = *pch; 
		width -=wl;
		for (;width > 0; width--,(*buf)++) **buf = ' ';


	   free(pc);
	   break;
   }   	   
   (*format)++;    
   return 0;  
}  


int __count_length(const char * format,va_list mark) 
{
	
	int k=0,t,width=0,prec,flag=0,i;
	long double d;
	unsigned int ui;
	char *pch;
	void *pv;
	
	while (*format) {
        if (*format =='%') {
            format++;
			i=1;
			while (i){
				switch (*format) {
				case '-':
				case '+':
				case ' ':
				case '0':
				case '#': format++; break;
				default: i=0; 
				}
			}
			if (*format == '*') {
				width=va_arg(mark,int);
				format++;
			} else {
				width=__get_int(format,&t);
				format+=t;
			}
	   	    if (*format == '.') {
			    format++;
			    flag |=__PRECISION;
			    if (*format == '*') {
					format++;
					prec = va_arg(mark,int);
				} else {
					prec = __get_int(format,&t);
					format+=t;
				}
			}

   		   if (*format=='h') {
			   flag |=__HFLAG;
			   format++;
		   } else {
               if (*format=='l') {
                   flag |=__LFLAG;
                   format++;
			   }
		   }
		   switch (*format) {
		   case '%':
		 	  k ++;
		 	  break;
		   case 'c':
			  i = va_arg(mark,char);
			  k += (width>=1) ? width : 1;
			  break;
		   case 'n' : 
			  pv = (void *) va_arg(mark,int *);
			  break;	  
		   case 's' :
			  pch = va_arg(mark,char *);
			  if (flag & __PRECISION) {
				  k += (prec>=width) ? prec : width;
				  break;
			  }
			  i = strlen(pch);
			  k += (width>=i) ? width : i;
			  break;
		   case 'i':
  		   case 'd':
           case 'u':
 		   case 'o':
 			  if (flag & __HFLAG)  i = (int)va_arg(mark,short int);
			  else i = va_arg(mark,int);
			  k += (width >= 12) ? width : 12;
			  break;
		   case 'p':
		   case 'x':
		   case 'X':
			  if (flag & __HFLAG) ui = va_arg(mark,unsigned short);
			  else ui = va_arg(mark,unsigned int);
			  k += (width>=(12+prec)) ? width : (12+prec);
			  break;
		   case 'e':
	 	   case 'E':
		   case 'g':
		   case 'G':
			  if (flag & __LFLAG) d = (double)va_arg(mark,long double );
			  else d = va_arg(mark,double);
			  if (!(flag & __PRECISION)) prec = 6;
			  k += (width>=(10+prec)) ? width : 10+prec;
			  break;
		   case 'f':
			  if (flag & __LFLAG) d = va_arg(mark,long double);
			  else d = va_arg(mark,double);
			  if (!(flag & __PRECISION)) prec = 6; 
			  if (d<0) d*=-1;
			  for (i=4;d>1;d/=10,i++);
			  k += (width>=(prec+i)) ? width : (prec+i);
			  break;			 		 			  		   
		   }
		   format++;
		} else {
            format++;
			k++;                          
        }
    }

	return k;
}

int vfprintf( FILE *stream, const char *format, va_list mark )
{
	char *buf, *buf_f;
	int k;
	
	if ( !(stream && format) )  {
		errno = EINVAL;
		return -1;
	}
	if (!(stream->flag & _F_WRITE)) {
		errno = EINVAL;
		return -1;//файл открыт в режиме только для чтения  
	}
	k =  __count_length(format,mark);
	buf = buf_f = malloc(k);
	while (*format) {
        if (*format!='%') {
            *buf++=*format++;
        } else {
            format++;
            _convert(&format, &buf, &mark,buf_f);                          
        }
    }
	k =  fwrite(buf_f,1,(int)(buf-buf_f),stream);
	free(buf_f);
	return k;
}

//пишет в строку данные в соответствующем формате
int sprintf( char *buffer, const char *format ,... )
{
	char * buf_f = buffer;
	va_list m;
	va_start(m,format);
	while (*format) {
        if (*format!='%') {
            *buffer++=*format++;
        } else {
            format++;
            _convert(&format, &buffer, &m,buf_f);                          
        }
    }
	return (int)(buffer - buf_f);
}




int fprintf (FILE * stream, const char *format, ... ) 
{	
	va_list mark;	
	if ((!(stream)) || (!(stream->flag & _F_WRITE))) {
		errno = EINVAL;
		return -1;
	}
    va_start(mark,format);
	return vfprintf(stream,format,mark);               
}     

int printf( const char *format, ... )
{
	va_list mark;
	int num;
	
	va_start(mark,format);
	num = vfprintf(stdout,format,mark);
	fflush(stdout);	 
	return num;
}

int  fclose(FILE * f)
{		
	int i;
	
	if (!f) {
		errno = EINVAL;
		return EOF;
	}
	fflush(f);	
	for (i = 0; ((files[i]!=f) && (i<FOPEN_MAX)); i++);	
	if (i == FOPEN_MAX) {
		errno = EINVAL;
		return -1;
	}
	files[i] = NULL;
	close(f->file_descr);
	if (f->flag & _F_BUFSYS) free(f->buf);
    free(f);
	return 0;
}

int remove( const char *path )
{
	//выдает -1 если файл открыт
	//также если файл не найден или путь указывает на директорию то errno 
	
	// устанавливается значение ENOENT
	// если файл read only то errno устанавливается EACCES 
	
	DWORD error;	
	
	if (_delete_file(path))return 0;	
	else {
		error = _get_last_error();	
		if (error == ERROR_FILE_NOT_FOUND) errno = ENOENT;
		else if (error == ERROR_ACCESS_DENIED) errno = EACCES; 
		return -1;
	}	 
}

int rename( const char *oldname, const char *newname )
{
	unsigned long error;
	
	if (_move_file(oldname,newname))  return 0;
	else {
		error = _get_last_error();
		if (error == ERROR_FILE_NOT_FOUND) errno = ENOENT;
		else if (error == ERROR_ACCESS_DENIED) errno = EACCES;
			else if (error & ERROR_INVALID_NAME) errno = EINVAL;
		return -1;
	}
}

char *tmpnam( char *string )
{
	if (!string) string = __tmp_name; 	
	if (!_get_temp_file_name("\\",".tmp",0,string)) {
		errno = EFAULT;
		return NULL;
	}
	_delete_file(string);
	return string;
}

FILE *tmpfile( void )
{		
	int i;
	
	if (!tmpnam(NULL)) return NULL;
	for (i = 0; ((i<FOPEN_MAX) && (files[i])); i++);
	if (i == FOPEN_MAX) {
		errno = EMFILE;
		return NULL;
	}
	files[i] = malloc(sizeof(FILE));
	if (!files[i]) return NULL;
	files[i]->flag = _F_BINARY | _F_WRITE | _F_READ;
	files[i]->file_descr = _create_tmp_file(__tmp_name);
	if ((files[i]->file_descr == -1) || (!(files[i]->file_descr))) {	
		free(files[i]);
		errno = EFAULT;
		return NULL;
	}	
	setvbuf(files[i],0,0,BUFSIZ);
	files[i]->b_first = files[i]->b_last = 0;	
	return files[i];
}

int __pro_scanf(int flagfile,const char *format,va_list arglist,const char * buf,FILE *f) 
{	
	//вывода ошибок нет
	int i=0,num = 0;
	char c;
	
	while (*format) {
		switch (*format) {
		case ' ':
		case '\n':
		case '\t':
			for (; ( ((c = __getc(flagfile,f,&buf,0,&num)) == ' ') || 
				(c == '\t') || 
				(c == '\n') );) __getc(flagfile,f,&buf,1,&num);
			format++;
			break;
		case '%':
			format++;		
			if(__convert_to_arg(flagfile,&buf,&format,&arglist,f,&num) == -1) return i;	
			i++;
			break;
		default:
			if (__getc(flagfile,f,&buf,1,&num)!=*format) return  i;
			format++;			
			break;
		}
	}	
	return i;	
}

//читает из строки данные в соответствующем формате
int sscanf( const char *buffer, const char *format, ... )
{
	//вывода ошибок нет
	va_list m;
	va_start(m,format);

	return vsprintf(buffer,format,m);
}

int vsprintf(const char *buffer, const char *format, va_list argptr )
{
	return __pro_scanf(0,format,argptr,buffer,0);
}

#define __IGNORED  1
#define __WIDTHDEF 2

int __get_sign(const char **buf, int *i,int *width,int flagfile,FILE *f,int flag,int *num) 
{
	//вывод ошибок есть

	if ((flag & __WIDTHDEF) && (*width<=0)) {
		errno = EINVAL;
		return -1;
	}
	if (__getc(flagfile,f,buf,0,num) == '-') {
		(*width)--;
		*i = -1;
		__getc(flagfile,f,buf,1,num);
	} else {
		if (__getc(flagfile,f,buf,0,num) == '+'){			
			(*width)--;
			__getc(flagfile,f,buf,1,num);
		}
		*i = 1;
	}
return 0;
}

int __get_spec_int (int flagfile,FILE *f,const char **buf, int flag,int *width,int *k,int *num) 
{
	//вывода ошибок нет
	char c;
	*k = 0;
	
	if (!( ((!(flag & __WIDTHDEF)) || (*width>0)) && (((c = __getc(flagfile,f,buf,0,num)) >='0') && 
		(c<='9')) ))
		return 1;
	while ( ((!(flag & __WIDTHDEF)) || (*width>0)) && (((c = __getc(flagfile,f,buf,0,num)) >='0') && (c<='9')) ) {
		*k = *k*10 + c - '0';		
		(*width)--;
		__getc(flagfile,f,buf,1,num);
	}
	return 0;
}


int fscanf( FILE *stream, const char *format , ... )
{
	//вывод ошибок eсть

	va_list mark;
	if ((!(stream) || (!(stream->flag & _F_READ)))) {
		errno = EINVAL;
		return -1;
	}
	if ((!(stream->flag & _F_BUF)) && (!(stream->flag & _F_B_MUSTNEX)))
		setvbuf(stream,0,0,0);
	if (stream->flag & _F_B_NOTDEF) 
		__init_buf(stream,0);
	if (stream->flag & _F_B_WRITE) { 
		fflush(stream);
		stream->flag &=~_F_B_WRITE;
		stream->flag |=_F_B_READ;
	}		
	va_start(mark,format);
	return __pro_scanf(1,format,mark,0,stream);
}

int scanf( const char *format ,... )
{
	va_list m;
	va_start(m,format);
	return __pro_scanf(1,format,m,0,stdin);
}

int __convert_to_arg(int flagfile,const char **buf,const char **format,va_list *arglist,
					 FILE *f,int *num)
{
	int flag = 0,width=0,k,j,*pi,i,a,i1;
	unsigned int *pui,ui;
	double d1,*pd;	
	char *pc;
	char c;
	if (**format == '*') {
		flag = __IGNORED;
		(*format)++;
	}
	k=0;
	while (((c = **format) >= '0') && (c<='9')) {
		width = width*10 + c - '0';
		k++;
		(*format)++;
	}
	if (k>0) flag |= __WIDTHDEF;		
	switch (**format) {
	case 'h': 
		flag |= __HFLAG;
		(*format)++;
		break;
	case 'l':
		flag |= __LFLAG;
		(*format)++;
		break;
	}	
	if ((**format!='c') && (**format!='n')) {		
			k=1;
			while ( ((!(flag & __WIDTHDEF)) || (width>0)) && (k) ) {
				switch (c = __getc(flagfile,f,buf,0,num)) {
				case  ' ':
				case '\n':
				case '\t': 
					__getc(flagfile,f,buf,1,num);
					width--;
					break;
				default: k=0;
				}
			}
	}
	switch (**format){
	case 'c':
		if (!(flag & __IGNORED)) {
			pc = va_arg(*arglist, char * );
			*pc = __getc(flagfile,f,buf,1,num);			
		}		
		break;
	case 'd':
		if (!(flag & __IGNORED)) 
		pi = (flag & __HFLAG) ? (int*)va_arg(*arglist,short int *) : va_arg(*arglist, int *);		
		__get_sign(buf,&i,&width,flagfile,f,flag,num);	
		if (__get_spec_int(flagfile,f,buf,flag,&width,&k,num)) return -1;	//переделать
		k *= i;
		if (!(flag & __IGNORED))  
			if (flag & __HFLAG)  *(short int *)pi = (short int) k;
			else *pi = k;
		break;
	case 'o':
		if (!(flag & __IGNORED))
			pi = (flag & __HFLAG) ? (int *)va_arg(*arglist,short int *) : va_arg(*arglist, int *);
		__get_sign(buf,&i,&width,flagfile,f,flag,num);
		k = 0;
		if (!( ((c = __getc(flagfile,f,buf,0,num))>='0') && (c <= '8') && 
			((!(flag & __WIDTHDEF)) || width>0) )) return -1;
		while ( ((c = __getc(flagfile,f,buf,0,num))>='0') && (c<='8') && 
			((!(flag & __WIDTHDEF)) || width>0) ) {
			k = k*8 + c - '0';
			__getc(flagfile,f,buf,1,num);
			width--;
		}
		k *=i;
		if (!(flag & __IGNORED)) 
			if (flag & __HFLAG) *(short int*)pi = (short int) k;
			else *pi = k;
		break;
	case 'u':
		if (!(flag & __IGNORED))
			pui = (flag & __HFLAG) ? (unsigned int *)va_arg(*arglist,unsigned short int *) : 
				va_arg(*arglist,unsigned int *);
		ui = 0;
		while (((c = __getc(flagfile,f,buf,0,num))>='0') && (c<='9') && ( (!(flag & __WIDTHDEF)) || 
			(width>0) ) ){
			ui = ui*10+c - '0';
			__getc(flagfile,f,buf,1,num);
			width--;
		}
		if (!(flag & __IGNORED)) 
			if (flag & __HFLAG) *(unsigned short int*)pui = (unsigned short int) ui;
			else *pui = ui;
		break;
	case 'x':
	case 'X':
		if (!(flag & __IGNORED))
			pi = (flag & __HFLAG) ? (int *)va_arg(*arglist,short int *) :
				va_arg(*arglist, int *);
		__get_sign(buf,&i,&width,flagfile,f,flag,num);
		if ((flag & __WIDTHDEF) && (width<=0)) return -1;		
		if (__getc(flagfile,f,buf,0,num) == '0') {
			__getc(flagfile,f,buf,1,num);
			if ((flag & __WIDTHDEF) && (width<=1)) {
				if (!(flag & __IGNORED)) 
					if (flag & __HFLAG) *(short int*)pi = (short int)0;
					else *pi = 0;
				break;
			} else width--;
			if (__getc(flagfile,f,buf,0,num) == **format) {
				if ((flag & __WIDTHDEF) && (width<=1)) {
					if (!(flag & __IGNORED)) 
						if (flag & __HFLAG) *(short int*)pi = (short int)0;
						else *pi = 0;
					break;
				} else {
					width--;
					__getc(flagfile,f,buf,1,num);
				}
			}
		}


		if (!( ((c = __getc(flagfile,f,buf,0,num))>='0') && (c <='9'))) {
			if (**format == 'x') {
				if (!( (c>='a') && (c<='f'))) return -1;
			} else if (!( (c>='A') && (c<='F'))) return -1;
		}
		if ((flag & __WIDTHDEF) && (width<=0)) return -1;
		while ( ( (((c = __getc(flagfile,f,buf,0,num))>='0') && (c<='9')) || 
			((**format == 'x') && (c>='a') && (c<='f')) || 
			((**format == 'X') && (c>='A') && (c<='F')) ) && 
			((!(flag & __WIDTHDEF)) || (width>0)) ) {
			if ((**format == 'x') && (c>='a') && (c<='f')) k = k*16 + c - 'a'+10;
			else if ((**format == 'X') && (c>='A') && (c<='F')) k = k*16 + c - 'A'+10;
			     else k = k*16 + c - '0';
			__getc(flagfile,f,buf,1,num);
			width--;
		}
		k*=i;
		if (!(flag & __IGNORED)) 
			if (flag & __HFLAG) *(short int*)pi = (short int)k;
			else *pi = k;
		break;
	case 'n':
		if (!(flag & __IGNORED)) {
			pi = (flag & __HFLAG) ? (int *)va_arg(*arglist,short int *) : va_arg(*arglist, int *);
			if (flag & __HFLAG)  *(short int*)pi = (short int)(*num-1);
			else *pi = (int)(*num-1);
		}
		break;
	case 'i':
		if (!(flag & __IGNORED))
			pi = (flag & __HFLAG) ? (int*)va_arg(*arglist,short int *) : va_arg(*arglist, int *);
		__get_sign(buf,&i,&width,flagfile,f,flag,num);
		k = 0;
		if (__getc(flagfile,f,buf,0,num) == '0'){ 
			if ((flag & __WIDTHDEF) && (width<=1)) {
				if (!(flag & __IGNORED)) 
					if (flag & __HFLAG) *(short int *)pi = (short int )k;
					else *pi = k;
				break; 
			} else width--;
			__getc(flagfile,f,buf,1,num);

			if (((a = __getc(flagfile,f,buf,0,num)) == 'x') || (a == 'X')) {
				if (flag & __WIDTHDEF) {
					if (width<=1) { 
						if (!(flag & __IGNORED)) 
							if (flag & __HFLAG) *(short int *)pi = (short int )k;
							else *pi = k;
						break;
					} else width--;
				}
				__getc(flagfile,f,buf,1,num);
				if (a == 'x') {
					while ( ( ((c = __getc(flagfile,f,buf,0,num))>='0') && (c <='9')) || 
					((c>='a') && (c<='f')) && ((!(flag & __WIDTHDEF)) || (width>0)) ) {
						__getc(flagfile,f,buf,1,num);
						k = k*16 + c;
						k +=  ((c>='a') &&(c<='f')) ? (- 'a' + 10) : (- '0');						
						width--;
					}
				} else {
					while ( ( ((c = __getc(flagfile,f,buf,0,num))>='0') && (c <='9')) || 
					((c>='A') && (c<='F')) && ((!(flag & __WIDTHDEF)) || (width>0)) ) {
						__getc(flagfile,f,buf,1,num);
						k = k*16 + c;
						k +=  ((c>='A') &&(c<='F')) ? (- 'A' + 10) : (- '0');						
						width--;
					}
				}

			} else {
				
				while (((c = __getc(flagfile,f,buf,0,num))>='0') && (c<='8') && 
					((!(flag & __WIDTHDEF)) || (width>0)) ) {
						k = k*8 + c - '0';
						__getc(flagfile,f,buf,1,num);
						width--;
					}
			}
		} else {
			while (((c =  __getc(flagfile,f,buf,0,num))>='0') && (c<='9') && 
					((!(flag & __WIDTHDEF)) || (width>0)) ) {
						k = k*10 + c - '0';
						__getc(flagfile,f,buf,1,num);
						width--;
					}
		}
		k *=i;
		if (!(flag & __IGNORED)) 
			if (flag & __HFLAG) *(short int *)pi = (short int )k;
			else *pi = k;
		break;
		 
	case 'e':
	case 'E':
	case 'f':
	case 'G':
	case 'g':
		i = i1 = 1;
	   if (!(flag & __IGNORED))
		   pd = (flag & __LFLAG) ? va_arg(*arglist,double *) :(double *)va_arg(*arglist, float *);
	   
	   __get_sign(buf,&i,&width,flagfile,f,flag,num);
	   
	   if ((flag & __WIDTHDEF) && (width<1)) return -1;
	   
	   if (((c = __getc(flagfile,f,buf,1,num)) >='0') && ( c <= '9')) 
		   d1 = (double) c - '0';	
	   else return -1;	 
	   width--;
	   for (;(((c = __getc(flagfile,f,buf,0,num))>='0') && (c <= '9') && ((!(flag & __WIDTHDEF)) 
		   || (width>0))); width--) {
		   d1 = d1*10 + (double)(c - '0');
		   __getc(flagfile,f,buf,1,num);
	   }
	   if ((flag & __WIDTHDEF) && (width<1)) {
		end :
		d1*=i;   
	   if (!(flag & __IGNORED)) 			   
				if (flag & __LFLAG) *pd = d1;
				else *(float *)pd = (float)d1;
			break;
	   }
	   if (__getc(flagfile,f,buf,0,num) == '.') {
		   __getc(flagfile,f,buf,1,num);
		   width--;
		   for (j = 1; ( ( (c = __getc(flagfile,f,buf,0,num)) >= '0') && (c <= '9') && 
			   ((!(flag & __WIDTHDEF)) || (width>0)) ); width--,j++) {
			   d1 += (double)(c - '0') * _pow_ten(-j);
			   __getc(flagfile,f,buf,1,num);
		   }		  
	   }
	   if (( (flag & __WIDTHDEF) && (width<2) ) || 
	    ( ((c = __getc(flagfile,f,buf,0,num)) != 'e') && (c != 'E') )) goto end;
	   else __getc(flagfile,f,buf,1,num);
	   __get_sign(buf,&i1,&width,flagfile,f,flag,num);
	   
	   if ((flag & __WIDTHDEF) && (width<1)) goto end;
	   for (k=0; ( ((c = __getc(flagfile,f,buf,0,num)) >= '0') 
		   && (c <= '9') && ((!(flag & __WIDTHDEF)) || (width>0))); width--) {
			__getc(flagfile,f,buf,1,num);
			k = k*10 + c - '0';
	   }
	   d1*=i;
	   k *=i1;
	   if (!(flag & __IGNORED)) 
		   if (flag & __LFLAG) *pd = d1 *_pow_ten(k);
		   else 
			   *(float *)pd = (float)(d1 * _pow_ten(k));
	   break;
	case 's':
	   if (!(flag & __IGNORED)) pc = va_arg(*arglist,char *);
	   for (; ((c = __getc(flagfile,f,buf,1,num)) && (c != '\t') && (c != '\n') && (c != ' ') &&
	   (c != 0) && (!(flag & __WIDTHDEF)) || (width>0));pc++,width--) if (!(flag & __IGNORED)) *pc = c;
	    *pc = 0;
		break;
	}
  (*format)++;
  return 0;
}

void perror( const char *string )
{
	if (errno < 10) printf("%s%s",string,_sys_errlist[errno]);		
}

int ferror( FILE *stream )
{
	if (!stream) {
		errno = EINVAL;
		return 1;
	}
	return (stream->flag & _F_ERROR || stream->flag & _F_EOF) ? 1 : 0;		
}

int ungetc( int c, FILE *stream )
{
	if ( (!(stream->flag & _F_READ)) || ((stream->flag & _F_BUF) && (!(stream->flag & _F_B_READ))) ) return	EOF;
	if (!tell(stream->file_descr)) {
		errno = EBADF;
		return EOF;
	}
	if (stream->b_first>0) {
		if (stream->buf[stream->b_first - 1]!=c) {
			errno = EINVAL;
			return EOF;
		}
		if ((stream->flag & _F_BINARY) || (c != '\n')){ 
			stream->b_first--;
			return c;
		}
		if (stream->b_first>1) {
			if (stream->buf[stream->b_first - 2] == '\r') stream->b_first-=2;
			else stream->b_first--;
			return c;
		}
	}
	return EOF;
		//не корректно, если после загрузки буфера ничего не прочитано	
		//или если чтение не буферизованное		
}

double _pow_ten(int k) 
{
    int i;
    double d = 1;
	
	if (k >= 0) for (i = 0; i < k; i++) d*=10;
	else for (i = 0; i > k; i--) d/=10;
	return d;
}
