// edimp.cpp

#include "ss.h"

#define  MITEMS  2

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
char PluginRootKey[MAX_PATH];

// Message Box (DEBUG-time)
const char *g_m[4] = { "Title", NULL, "", "OK" };


int WINAPI _export GetMinFarVersion( void ) {

	return FARMANAGERVERSION;
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

	static const char *DiskMenuStrings[1];

	pi->StructSize = sizeof( struct PluginInfo );

	DiskMenuStrings[0] = MSG(msg_plugin_name);
	pi->DiskMenuStrings = DiskMenuStrings;
	pi->DiskMenuNumbers = NULL;
	pi->DiskMenuStringsNumber = 1;

	pi->CommandPrefix = "ss";
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
	delete (SS *)hPlugin;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item) {

	int i;
	SS *ss;
	
	//MSGBOX( "open plugin" );

	static int cominit = 0;
	if( !cominit ) {
		::CoInitialize(NULL);
		cominit = 1;
	}

	ss = new SS(OpenFrom,item);
	return ss ? ss : INVALID_HANDLE_VALUE;
}

void InitDialogItems( const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber )
{
	int i;
	struct FarDialogItem *PItem = Item;
	const struct InitDialogItem *PInit = Init;

	for( i=0; i < ItemsNumber; i++, PItem++, PInit++ ) {
		PItem->Type=PInit->Type;
		PItem->X1=PInit->X1;
		PItem->Y1=PInit->Y1;
		PItem->X2=PInit->X2;
		PItem->Y2=PInit->Y2;
		PItem->Focus=PInit->Focus;
		PItem->Selected=PInit->Selected;
		PItem->Flags=PInit->Flags;
		PItem->DefaultButton=PInit->DefaultButton;
		if( (unsigned int)PInit->Data < 2000 )
			strcpy(PItem->Data,MSG((unsigned int)PInit->Data));
		else
			strcpy( PItem->Data, PInit->Data );
	}
}
