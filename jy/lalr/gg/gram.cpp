// gram.cpp (Eugeniy Gryaznov/Elena Loschenko, 2002, inspirer@inbox.ru)

#include "gram.h"

info::info()
{
   rule=NULL;length=NULL;
   rules=symbols=power=input=eoi=0;
   name=NULL;
   symbol=NULL;
   file_cpp=class_name=NULL;
   first=follow=NULL;
   chstate=NULL;avan=NULL;states=0;
   sRule=sPos=index=NULL;
   err=debug=NULL;
}

/// set //////////////////////////////////////////////////////////////////////

#define init_hash 0x12b3
#define update_hash(prev,symb) prev=(prev<<1)+(prev>>31),prev^=0x989693*(symb);

set::set(int n)
{
    hash_calculated=0;N=(n+7)/8;
    data = new char[N];
    for (int i=0; i<N;i++) data[i]=0;
}

int set::operator()(int i)
{
    char p=*(data+(i/8));
    p &= (1<<(i%8));
    return (p!=0);
}

void set::setbit(int n)
{
    char* p=data+(n/8);
    (*p) |= 1<<(n%8);
    hash_calculated=0;
}

void set::clearbit(int n)
{
    char* p=data+(n/8);
    (*p)&=~(1<<(n%8));
    hash_calculated=0;
}

void set::calculate_hash()
{
    hash_calculated=1;
    hash=init_hash;
    for (int i=0;i<N;i++) 
        update_hash(hash,data[i]);
}

int set::compare(set& s2)
{
    if (hash_calculated&&s2.hash_calculated) {
        if (hash>s2.hash) return 1;
        if (hash<s2.hash) return -1;
    };
    char* p1=data,*p2=s2.data;
    if (N!=s2.N) return -2;
    for (int i=0;i<N;i++,p1++,p2++) if (*p1!=*p2) return -2;
    return 0;
}

int set::add(set * i)
{
    if (N != i->N) return 0;
    int e,added = 0;
    char *l1=data,*l2=i->data;

    for( e=0; e<N; e++,l1++,l2++){
        if ((*l1|*l2) != *l1) added = 1;
        *l1 |= *l2;
    }
    return added;
}

set * set::clone()
{
    return new set(N*8);
}

set& set::operator=(set& o)
{
    for( int i=0; i<N; i++ ) data[i]=o.data[i];
    return *this;
}

void set::clear()
{
    for( int i=0; i<N; i++ ) data[i]=0;
}

int set::empty()
{
    for (int i=0; i<N; i++ ) if (data[i]!=0) return 0;
    return 1;
}

int set::next(int current)
{
    int i=current/8,e;
    char p=*(data+i);

    for (e=current%8; e<8; e++) if (p & (1<<e)) return i*8 + e;
    for (i++; i<N; i++){
        p = *(data+i);
        for (e=0; e<8; e++) if (p & (1<<e)) return i*8 + e;
    }
    return -1;
}

/// tree /////////////////////////////////////////////////////////////////////

#ifdef LINEAR
set * tree::search(set * q)
{
    te * e;
    q->calculate_hash();
    for( e=first; e!=NULL; e=e->next ) if (q->compare(*e->data)==0) return e->data;
    return NULL;
}

int tree::isearch(set * q)
{
    te * e;
    q->calculate_hash();
    for( e=first; e!=NULL; e=e->next ) if (q->compare(*e->data)==0) return e->number;
    return -1;
}

set * tree::getn(int n)
{
    te *e;
    for( e=first; e!=NULL; e=e->next ) if (e->number == n) return e->data;
    return NULL;
}

int tree::add(set * q)
{
    int i = isearch(q);
    if (i>=0) return i;    
    te * e = new te;
    e->next = first; first = e;
    e->data = q->clone();
    *e->data = *q;
    e->data->calculate_hash();
    e->number = total++;
    return e->number;
}

void tree::clear()
{
    te *e,*i = first;
    while(i){
        e = i;
        i = i->next;
        delete e;
    }
    first=NULL;
}

int  tree::count()
{
    return total;
}

#else
// tree //////////////////////////////////////////////////////////////////////

chset * tree::find(chset * chs,set * q) {
    chset * p;
    for (p=chs;p;p=p->nextch) if (p->This->compare(*q)==0) return p;
    return 0;
};

set * tree::search(set * q) {
    int h;
    node * PN=pnode;
    q->calculate_hash();
    h=q->hash;
    while (PN!=0) {
        if ((PN->hash)==h) return find(PN->chs,q)->This;
        PN = ((PN->hash)>h) ? PN->left : PN->right;
    };
    return 0;
};

int tree::isearch(set * q){
    int h;
    chset * p;
    node * PN=pnode;
    q->calculate_hash();
    h=q->hash;
    while (PN!=0) {
        if ((PN->hash)==h){
            for (p=PN->chs;p!=0;p=p->nextch) 
                if (p->This->compare(*q)==0) return p->IdNum;
            return -1;
        };
        PN = ((PN->hash)>h) ? PN->left : PN->right;
    };
    return -1;
};

set *tree::getn(int n){
    if (n>=SetNum) return NULL;
    chset * p;
    for (p=SetFirst;p;p=p->SetNext) if (p->IdNum==n) return p->This;
    return NULL;
};

void tree::clear() {
    int DelSets=0;
    chset * t;
    node * pn=pnode;
    while (DelSets!=SetNum){
        if (pn->left) pn=pn->left; else 
            if (pn->right) pn=pn->right; else {
                while (pn->chs!=0) {
                    t=pn->chs->nextch;
                    delete pn->chs;
                    pn->chs=t;
                };
                delete pn;
                DelSets++;
            };
    };
};

int tree::add(set * q){
    int h;
    int b;
    chset *tch;
    node * newnode;
    chset * U;
    node * PN=pnode,*Ln;

    q->calculate_hash();
    h=q->hash;
    if (!PN) {
         pnode=new node();
         pnode->chs=new chset();
         pnode->chs->This=q->clone();
         *(pnode->chs->This) = *q;
         pnode->chs->This->calculate_hash();
         pnode->chs->IdNum=0;
         pnode->chs->SetNext=0;
         SetFirst=pnode->chs;
         pnode->hash=h;
         SetNum++;
         return 0;
     };
    while (1) {
       if ((PN->hash)==h) {
            U=find(PN->chs,q);
            if (U) return U->IdNum; else  {
                tch=new chset;
                tch->This=q->clone();
                *(tch->This) = *q;
                tch->This->calculate_hash();
                tch->IdNum=SetNum;
                tch->nextch=PN->chs;
                PN->chs=tch;
                tch->SetNext=SetFirst;
                SetFirst=tch;  
                SetNum++;
                return tch->IdNum;
            };
        };
        b=(PN->hash)>h;         
        if (b) Ln=PN->left; else Ln=PN->right;
        if (!Ln) {
            newnode=new node();
            newnode->parent=PN;
            newnode->chs=new chset();
            newnode->chs->This=q->clone();
            *(newnode->chs->This) = *q;
            newnode->chs->This->calculate_hash();
            newnode->chs->IdNum=SetNum;
            newnode->chs->SetNext=SetFirst;          
            SetFirst=newnode->chs;  
            newnode->hash=SetFirst->This->hash;
            if (b) PN->left=newnode; else PN->right=newnode;
            SetNum++;
            return SetNum-1;
        } else PN=Ln;
    };
    return -1;
};

#endif
