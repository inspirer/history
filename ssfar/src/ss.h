// Eugeniy Gryaznov

#ifndef SS_H_INCLUDED
#define SS_H_INCLUDED

#define VERSION   "1.05"

#include <windows.h>
#include <string.h>
#include "plugin.hpp"

//#define MAKE_LOG

#import "C:\MSVS\Common\VSS\win32\ssapi.dll" no_namespace

#define MSG(id) (char *)Info.GetMsg( Info.ModuleNumber, id )

extern struct PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern long allocated_blocks;

#define heapNEW GetProcessHeap()

inline void *m_malloc(size_t size) {
	InterlockedIncrement(&allocated_blocks);
	return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

inline char *m_strdup( const char * str ) {

	char *dest = (char *)HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, strlen(str)+1);
	InterlockedIncrement(&allocated_blocks);
	strcpy( dest, str );
	return dest;
}


inline void m_free(void *block) {
	InterlockedDecrement(&allocated_blocks);
	HeapFree(heapNEW,0,block);
}

inline void * cdecl operator new(size_t size) {
  return m_malloc(size);
}

inline void cdecl operator delete(void *block) {
  m_free(block);
}


// Dialog related /////////////////

void MsgBox( char *title, char *message );

struct InitDialogItem {
	unsigned char Type;
	char X1,Y1,X2,Y2;
	unsigned char Focus;
	unsigned int Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	char *Data;
	int *position;
};

#define array_size(i) (sizeof(i)/sizeof(i[0]))

struct FarDialog {
	int X, Y;
	int itemsNumber;
	struct FarDialogItem *di;
};

void create_fdlg( FarDialog *fdlg, int X, int Y, int Title, struct InitDialogItem *Item, int number );
int process_fdlg( FarDialog *fdlg );
void destroy_fdlg( FarDialog *fdlg );


void InitDialogItems( const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber );


// //////////////

struct list_of_id {
	struct list_of_id *next;
	char *val;
	char name[2];
};


class SS {
private:
	IVSSDatabase *db;

	char cdir[MAX_PATH];
	char cpath[MAX_PATH], cdbname[MAX_PATH], ssini[MAX_PATH]; // filled by split_cdir
	char Title[NM];			// by GetOpenPluginInfo

	// temporary
	char tmp_dir[MAX_PATH], tmp_dir2[MAX_PATH];
	struct KeyBarTitles kb;
	int next_dir_is_default;

	char file_to_pos[MAX_PATH];
	int file_pos_state;

	int project_first_open;

public:
    CRITICAL_SECTION cs;
	struct list_of_id *db_found;	//
	int db_count;					//   by GetFindData

private:
	int db_connect();
	void db_disconnect();
	char *get_default_dir_for( char *sspath );
	void split_cdir( );
	int try_to_change( int first_enter );
	int get_date( IVSSItem *item, DATE *change );

public:
	SS( int of, int item );
	~SS();

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
	int DeleteFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode );
	int MakeDirectory( char *Name, int OpMode );
};

void SetRegKey( const char *Key, const char *ValueName, char *ValueData );
void SetRegKey( const char *Key, const char *ValueName, DWORD ValueData );
int GetRegKey( const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey( const char *Key, const char *ValueName, int &ValueData, DWORD Default );
void KillRegVal( const char *Key, const char *ValueName );
int EnumRegKey( const char *key, void (*save)( char *name, char *val, void *data ), void *data );


enum {
	msg_plugin_name,
	msg_title_nonroot,
	msg_title_root,

	msg_getss,
	msg_checkout,
	msg_checkin,
	msg_createdb,
	msg_modifydb,
	msg_createnew,
	msg_config,

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

	msg_put_dest,
	msg_put_comment,
	msg_put_keepco,
	msg_put_remloc,
	msg_put_title,
	msg_put_to,

	msg_get_clear_ro,
	msg_get_recurs,
	msg_get_title1,
	msg_get_title2,
	msg_get_action,
	msg_getco_to,

	msg_co_dontgetloc,
	msg_co_replwr,
	msg_co_title1,
	msg_co_title2,
	msg_co_action,

	msg_db_name,
	msg_db_file,
	msg_dir_name,
	msg_dir_cmnt,

	msg_dlgdel_title,
	msg_dlgdel_message,
	msg_dlgdel2_title,
	msg_dlgdel2_message,

	msg_undoco_warn_title,
	msg_undoco_warn_msg,
	msg_undoco_title,
	msg_undoco_msg,
	msg_undoco_rep,
	msg_undoco_leave,
	msg_undoco_del,
	msg_undoco_cancel,

	msg_ci_action,
	msg_ci_to,

	msg_del_title,
	msg_del_msg,
	msg_del_perm,
	msg_del_ok,
	msg_del_cancel,
	msg_del_several1,
	msg_del_several2,

	msg_kb_root4,
	msg_kb_root7,
	msg_kb_root8,

	msg_kb5,
	msg_kb6,
	msg_kb7,
	msg_kb8,
	msg_kb_c4,
	msg_kb_c5,
	msg_kb_c6,
	msg_kb_c7,
	msg_kb_c8,
	msg_kb_a5,
	msg_kb_a6,
	msg_kb_ca6,

	msg_config_alloc,
	msg_config_ok,
	msg_config_can,
	msg_config_prefix,
	msg_config_version,
	msg_config_showdates,

	msg_defdir_title,
	msg_defdir_msg,
	msg_defdir_to,

	msg_ren_title,
	msg_ren_msg,
	msg_ren_ok,
};


#endif
