//  place.h

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef place_h_included
#define place_h_included

class TokenPlace {
public:
	struct where {
		int line,col;
		int index,used;
	};

private:
	struct index {
		int ind,used;
		char *text;
		index *next;
	} *fi;
	struct pool {
		where data[100];
		int used,frompos;
		pool *next;
	} *fp;
	int next100;
	int nextindex;

	void use_index(int y);
	void free_index(int y);

public:
	TokenPlace();
	~TokenPlace();

	int add_place(where& q);
	void use_place(int place,int num);
	void free_place(int place);
	where * get_place(int place);
	int add_index(const char *s);
	char * get_index(int number);
	void clear();
};

#endif
