// edimp.cpp

#include "ss.h"

// static info

struct PluginStartupInfo Info;			// psi
FARSTANDARDFUNCTIONS FSF;				// Far standart functions
char PluginRootKey[MAX_PATH];			// REGISTRY root key for plugin
long allocated_blocks = 0;				// count of allocated memory blocks


static void get_SS( SS *ss )
{
	EnterCriticalSection( &ss->cs );
}


static void inline put_SS( SS *ss  )
{
	LeaveCriticalSection( &ss->cs );
}


int WINAPI _export GetMinFarVersion( void ) {

	return MAKEFARVERSION(1,70,1282);
}


void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi) {

	Info = *psi;
	if(psi->StructSize >= sizeof(struct PluginStartupInfo)) {
		::FSF = *psi->FSF;
		::Info.FSF = &::FSF;
	}
	FSF.sprintf( PluginRootKey, "%s\\SourceSafe", Info.RootKey );
}

char cmdprefix[256] = "ss";

void WINAPI _export GetPluginInfo(struct PluginInfo *pi) {

	static const char *plugin_name_ptr[1];

	pi->StructSize = sizeof( struct PluginInfo );

	plugin_name_ptr[0] = MSG(msg_plugin_name);
	pi->DiskMenuStrings = plugin_name_ptr;
	pi->DiskMenuNumbers = NULL;
	pi->DiskMenuStringsNumber = 1;

	pi->PluginConfigStrings = plugin_name_ptr;
	pi->PluginConfigStringsNumber = 1;

	pi->PluginMenuStrings = plugin_name_ptr;
	pi->PluginMenuStringsNumber = 1;

	GetRegKey( "", "cmdline_prefix", cmdprefix, cmdprefix, 256 );
	pi->CommandPrefix = cmdprefix;
}


int WINAPI _export Configure( int ItemNumber )
{
	char s[128];

	FarDialog dlg;
	static int id_allocated, id_cancel, id_prefix, id_ver;
	static struct InitDialogItem items[] = {
	 { DI_TEXT,       1,0,0,0,		0,0,0,0,(char*)msg_config_prefix },
	 { DI_EDIT,       1,1,-2,0,		1,(int)"ss_prefix",DIF_HISTORY,0, "", &id_prefix },

	 { DI_TEXT,       1,3,0,0,		0,0,0,0,"",&id_ver },
	 { DI_TEXT,       1,4,0,0,		0,0,0,0,"",&id_allocated },

	 { DI_TEXT,       1,5,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_config_ok },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_config_can, &id_cancel },
	};

	create_fdlg( &dlg, 50, 11, msg_config, items, array_size( items ) );
	FSF.sprintf( dlg.di[id_allocated].Data, MSG(msg_config_alloc), allocated_blocks ); 
	FSF.sprintf( dlg.di[id_ver].Data, MSG(msg_config_version), VERSION ); 
	GetRegKey( "", "cmdline_prefix", cmdprefix, cmdprefix, 256 );
	strcpy( dlg.di[id_prefix].Data, cmdprefix );

	int ec = process_fdlg( &dlg );

	if( ec < 0 || ec == id_cancel ) {
		destroy_fdlg( &dlg );
		return FALSE;
	}

	strcpy( cmdprefix, dlg.di[id_prefix].Data );
	SetRegKey( "", "cmdline_prefix", cmdprefix );
	destroy_fdlg( &dlg );

	return TRUE;
}

int WINAPI _export GetFindData( HANDLE hPlugin, struct PluginPanelItem **pPanelItem, 
		int *pItemsNumber, int OpMode ) 
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
    int res = ss->GetFindData( pPanelItem, pItemsNumber, OpMode );
	put_SS( ss );
    return res;
}


void WINAPI _export FreeFindData( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber )
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
    ss->FreeFindData( PanelItem, ItemsNumber );
	put_SS( ss );
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info1)
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	ss->GetOpenPluginInfo( Info1 );
	put_SS( ss );
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->SetDirectory( Dir, OpMode );
	put_SS( ss );
    return res;
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->ProcessEvent (Event, Param);
	put_SS( ss );
    return res;
}

int WINAPI _export ProcessKey( HANDLE hPlugin, int Key, unsigned int ControlState )
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->ProcessKey( Key, ControlState );
	put_SS( ss );
    return res;
}

int WINAPI _export GetFiles( HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->GetFiles( PanelItem, ItemsNumber, Move, DestPath, OpMode );
	put_SS( ss );
    return res;
}

int WINAPI _export PutFiles( HANDLE hPlugin, struct PluginPanelItem *PanelItem,
								int ItemsNumber, int Move, int OpMode )
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->PutFiles( PanelItem, ItemsNumber, Move, OpMode );
	put_SS( ss );
    return res;
}

int WINAPI _export DeleteFiles( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode )
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->DeleteFiles( PanelItem, ItemsNumber, OpMode );
	put_SS( ss );
    return res;
}

int WINAPI _export MakeDirectory( HANDLE hPlugin, char *Name, int OpMode )
{
	SS *ss = (SS *)hPlugin;
	get_SS( ss );
	int res = ss->MakeDirectory( Name, OpMode );
	put_SS( ss );
    return res;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item) {

	int i;
	SS *ss;

	ss = new SS(OpenFrom,item);
	return ss ? ss : INVALID_HANDLE_VALUE;
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
	delete (SS *)hPlugin;
}
