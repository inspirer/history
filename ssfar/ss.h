// Eugeniy Gryaznov

#ifndef SS_H_INCLUDED
#define SS_H_INCLUDED

#include <windows.h>
#include <string.h>
#include "plugin.hpp"

#import "C:\MSVS\Common\VSS\win32\ssapi.dll" no_namespace

// debug
#define MSGBOX(msg) g_m[1] = msg;Info.Message( Info.ModuleNumber, 0, NULL, g_m, 4, 1 );
#define MSG(id) Info.GetMsg( Info.ModuleNumber, id )
extern const char *g_m[4];

#define heapNEW GetProcessHeap()

inline void *m_malloc(size_t size) {
	return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

inline char *m_strdup( const char * str ) {

	char *dest = (char *)HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, strlen(str)+1);
	strcpy( dest, str );
	return dest;
}


inline void m_free(void *block) {
	HeapFree(heapNEW,0,block);
}

inline void * cdecl operator new(size_t size) {
  return m_malloc(size);
}

inline void cdecl operator delete(void *block) {
  m_free(block);
}


extern struct PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern void InitDialogItems( const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber );

struct InitDialogItem {
	unsigned char Type;
	unsigned char X1,Y1,X2,Y2;
	unsigned char Focus;
	unsigned int Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	char *Data;
};


class SS {
private:
	char cdir[MAX_PATH];
	char tmp_dir[MAX_PATH], tmp_dir2[MAX_PATH];
	int try_to_change;

private:
	IVSSDatabase *db;

	int db_connect();
	void db_disconnect();


public:
	SS();
	char ssafe_ini[MAX_PATH];

	int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
	void GetOpenPluginInfo(struct OpenPluginInfo *Info);
	int SetDirectory(const char *Dir,int OpMode);
	int ProcessEvent( int Event, void *Param );
    int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,char *DestPath,int OpMode);
	int FileOp( int type, char *file, char *dest, int flags );

};

#endif
