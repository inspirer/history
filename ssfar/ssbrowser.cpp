// edimp.cpp

#include "ss.h"

// static info

struct PluginStartupInfo Info;			// psi
FARSTANDARDFUNCTIONS FSF;				// Far standart functions
char PluginRootKey[MAX_PATH];			// REGISTRY root key for plugin
long allocated_blocks = 0;				// count of allocated memory blocks



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

	pi->CommandPrefix = "ss";
}


int WINAPI _export Configure( int ItemNumber )
{
	char s[128];

	FSF.sprintf( s, "%i", allocated_blocks ); 
	MsgBox( "Configure", s );
	return TRUE;
}

int WINAPI _export GetFindData( HANDLE hPlugin, struct PluginPanelItem **pPanelItem, 
		int *pItemsNumber, int OpMode ) 
{
	SS *ss = (SS *)hPlugin;
	//MSGBOX("Get");
    return ss->GetFindData( pPanelItem, pItemsNumber, OpMode );
}


void WINAPI _export FreeFindData( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber )
{
	SS *ss = (SS *)hPlugin;
	//MSGBOX("Free");
    ss->FreeFindData( PanelItem, ItemsNumber );
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info1)
{
	SS *ss = (SS *)hPlugin;
	//MSGBOX("Info");
	ss->GetOpenPluginInfo( Info1 );
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
	SS *ss = (SS *)hPlugin;
	//MSGBOX( Dir );
	return ss->SetDirectory( Dir, OpMode );
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
	SS *ss = (SS *)hPlugin;
	//MSGBOX("Event");
	return ss->ProcessEvent (Event, Param);
}

int WINAPI _export GetFiles( HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                   int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  SS *ss = (SS *)hPlugin;
  return ss->GetFiles( PanelItem, ItemsNumber, Move, DestPath, OpMode );
}

int WINAPI _export ProcessKey( HANDLE hPlugin, int Key, unsigned int ControlState )
{
	SS *ss = (SS *)hPlugin;
	return ss->ProcessKey( Key, ControlState );
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
	//MsgBox( "1", "Delete" );
	delete (SS *)hPlugin;
}

int WINAPI _export DeleteFiles( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode )
{
	SS *ss = (SS *)hPlugin;
	return ss->DeleteFiles( PanelItem, ItemsNumber, OpMode );
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item) {

	int i;
	SS *ss;
	
	static int cominit = 0;
	if( !cominit ) {
		::CoInitialize(NULL);
		cominit = 1;
	}

	//MsgBox( "1", "Allocate" );
	ss = new SS(OpenFrom,item);
	return ss ? ss : INVALID_HANDLE_VALUE;
}


int WINAPI _export PutFiles( HANDLE hPlugin, struct PluginPanelItem *PanelItem,
								int ItemsNumber, int Move, int OpMode )
{
	SS *ss = (SS *)hPlugin;
	return ss->PutFiles( PanelItem, ItemsNumber, Move, OpMode );
}
