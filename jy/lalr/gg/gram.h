// gram.h (Eugeniy Gryaznov, 2002, inspirer@inbox.ru)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define LINEAR

#define EMPTY 1
#define TERM  2
#define MUTABLE 4

class set {
friend class tree;
private: 
    int hash_calculated, hash;
    int N;
    char *data;        
public:
    set(int n);
    int operator()(int i);
    void setbit(int n);
    void clearbit(int n);
    void calculate_hash();
    int compare(set& s2);
    int add(set * i);
    void clear();
    set * clone();
    set& operator=(set& o);
    int empty();
    int next(int current);
};

#ifdef LINEAR

class tree {
private:
    class te {
    public:
        te * next;
        set * data;
        int number;
        te() : next(NULL) {}
        ~te(){ delete data; }
    } *first;
    int total;
public:
    tree(){ first=NULL;total=0; }

    set *search(set * q);
    int  isearch(set * q);
    set *getn(int n);
    int add(set * q);
    void clear();
    int  count();
};

#else

struct chset {
    chset(){nextch=0; This=0;IdNum=0;};
    set* This;
    chset * nextch,*SetNext;
    int IdNum;
};
struct node {
    node(){ parent=0; right=0; left=0;hl=hr=hash=0; chs=0;};
    int hl,hr,hash;
    node * parent,*right,*left;
    chset* chs;
};
class tree {
private:
    chset * SetFirst,*SetLast;
    chset * find(chset * chs,set * q);
    int SetNum;
    node * pnode;
    
public:
    tree(){pnode = 0;SetNum=0;SetFirst=SetLast=0;pnode=0;};
    set *search(set * q);
    int  isearch(set * q);
    set *getn(int n);
    int add(set * q);
    void clear();
    int  count(){return SetNum;}; 
    
};

#endif

class info {
public:
    info();

	int mutables;
    int **rule,*length,*reduce;
    int rules,symbols,power,input,eoi;
    char **name;
    int *symbol;
    char *file_cpp,*file_h,*class_name;
    int priorities;
    struct priority {
        int assoc;
        int *place,count;
        set *s;
    } * prior;

    set **first,**follow;
    int *sRule,*sPos,*index;
    int situations;

    int **chstate,states;
    tree V;
    set ***avan;
    int *symtoterm,*termtosym,terminals;
    int **chnext;

    FILE *err,*debug;

    void First();
    void Follow();
    void enumerate();
    void extendSet(set& dyn, set& els);
    void buildV();
    int  extendAvan(set& dyn,set **s);
    void buildAvan();
    void buildLast();
    void outTables();
    void checkdebug();
    void checkerror();
};
