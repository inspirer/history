//  place.cpp

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "place.h"
#include "defs.h"

TokenPlace::TokenPlace()
{
	next100 = nextindex = 0;
	fi = NULL;fp = NULL;
}

TokenPlace::~TokenPlace()
{
	clear();
}

int TokenPlace::add_place(where& q)
{
	pool *p = fp;
	int i;

	use_index(q.index);
	while(p){
		if (p->used<100){
			for( i=0; i<100; i++) if (p->data[i].used == -1){
				p->data[i] = q; p->data[i].used = 1;
				p->used ++;
				return i+p->frompos;
			}
		}
		p=p->next;
	}
	p = new pool;
	p->next = fp;
	fp = p;
	p->used = 1;
	for( i=1; i<100; i++) p->data[i].used = -1;
	p->frompos = next100;
	next100 += 100;
	p->data[0] = q;
	p->data[0].used=1;
	return p->frompos;
}

TokenPlace::where * TokenPlace::get_place(int place)
{
	pool *p = fp;
	int hund = place - place%100;

	while(p){
		if( p->frompos == hund )
			if (p->data[place%100].used >=0)
				return &p->data[place%100];
		p = p->next;
	}
	return NULL;
}

void TokenPlace::use_place(int place,int num)
{
	pool *p = fp;
	int hund = place - place%100;

	while(p){
		if( p->frompos == hund )
			if (p->data[place%100].used >=0){
				p->data[place%100].used += num;
				return;
			}
		p = p->next;
	}
}

void TokenPlace::free_place(int place)
{
	pool *p = fp, **prev = &fp;
	int hund = place - place%100;

	while(p){
		if( p->frompos == hund )
			if (p->data[place%100].used >=0){
				p->data[place%100].used--;
				if (p->data[place%100].used == -1){
					free_index(p->data[place%100].index);
					p->used--;
					if (!p->used){
						*prev = p->next;
						delete p;
					}
				}
				return;
			}
		prev = &p->next;
		p = p->next;
	}
}

int TokenPlace::add_index(const char *s)
{
	index *i;

	i = new index;
	i->ind = nextindex ++;
	i->next = fi;
	fi = i;
	i->used = 0;
	i->text = new char[strlen(s) + 1];
	strcpy(i->text,s);
	return i->ind;
}

void TokenPlace::use_index(int y)
{
	index *i = fi;
	while(i){
		if (i->ind == y) i->used++;
		i = i->next;
	}
}

void TokenPlace::free_index(int y)
{
	index *i = fi;
	while(i){
		if (i->ind == y) i->used--;
		i = i->next;
	}
}

char * TokenPlace::get_index(int number)
{
	index *i = fi;
	while(i){
		if (i->ind == number){
			return i->text;
		}
		i = i->next;
	}
	return NULL;
}

void TokenPlace::clear()
{
	index *t;
	while(fi){
		t=fi;fi=fi->next;
		delete t->text;
		delete t;
	}
	pool *p;
	while(fp){
		p=fp;fp=fp->next;
		delete p;
	}
	next100 = 0;
	fi = NULL;fp = NULL;
}
