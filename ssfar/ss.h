// Eugeniy Gryaznov

#ifndef SS_H_INCLUDED
#define SS_H_INCLUDED

#include <windows.h>
#include <string.h>
#include "plugin.hpp"

//#define MAKE_LOG

#import "C:\MSVS\Common\VSS\win32\ssapi.dll" no_namespace

// debug
#define MSGBOX(msg) g_m[1] = msg;Info.Message( Info.ModuleNumber, 0, NULL, g_m, 4, 1 );
#define MSG(id) (char *)Info.GetMsg( Info.ModuleNumber, id )
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


struct list_of_id {
	struct list_of_id *next;
	char *val;
	char name[2];
};


class SS {
private:
	IVSSDatabase *db;
	int db_counter;

	#ifdef MAKE_LOG
	HANDLE log;
	char log_buffer[1024];
	unsigned long log_written;
	#define write_log FSF.sprintf 
	#define flush_log() WriteFile( log, log_buffer, strlen(log_buffer), &log_written, NULL )
	#else
	//#define flush_log() ;
	//#define write_log(...) ;
	#endif

	CRITICAL_SECTION db_own;

	char cdir[MAX_PATH];
	char cpath[MAX_PATH], cdbname[MAX_PATH], ssini[MAX_PATH]; // filled by split_cdir
	int try_to_change;
	char Title[NM];			// by GetOpenPluginInfo

	// temporary
	char tmp_dir[MAX_PATH], tmp_dir2[MAX_PATH];
	struct KeyBarTitles kb;

public:
	struct list_of_id *db_found;
	int db_count;

private:
	int db_connect();
	void db_disconnect();

public:
	SS( int of, int item );
	~SS();

	void split_cdir( );
	int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
	void GetOpenPluginInfo(struct OpenPluginInfo *Info);
	int SetDirectory(const char *Dir,int OpMode);
	int ProcessEvent( int Event, void *Param );
    int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,char *DestPath,int OpMode);
	int PutFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode );
	int FileOp( int type, char *file, char *dest, char *comment, int flags );
	int ProcessKey(int Key,unsigned int ControlState);

};

extern char PluginRootKey[MAX_PATH];

void SetRegKey( const char *Key, const char *ValueName, char *ValueData );
void SetRegKey( const char *Key, const char *ValueName, DWORD ValueData );
int GetRegKey( const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey( const char *Key, const char *ValueName, int &ValueData, DWORD Default );
void KillRegVal( const char *Key, const char *ValueName );
int EnumRegKey( const char *key, void (*save)( char *name, char *val, void *data ), void *data );


enum {
	msg_plugin_name,

	msg_getss,
	msg_checkout,
	msg_checkin,
	msg_createdb,
	msg_modifydb,

	msg_key_checkout,
	msg_key_create,
	msg_key_modify,
	msg_key_get,
	msg_key_cancel,
	msg_key_put,

	msg_title1,
	msg_title2,
	msg_title3,

	msg_root_title1,
	msg_root_title2,
};	


#endif
