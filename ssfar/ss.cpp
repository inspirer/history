// ss.cpp

#include "ss.h"


void BSTR_2_char( BSTR b, char *dest, int max ) {

	if( b )
		WideCharToMultiByte(CP_ACP, 0, b, *(((long *)b)-1), dest, max	, NULL, NULL);
	else
		*dest = 0;
}


BSTR char_2_BSTR( char *src ) {

	BSTR ret;
	wchar_t *wc;
	int len = strlen( src );

	if( !len )
		return NULL;
	wc = new wchar_t[len+1];
	MultiByteToWideChar(CP_ACP, 0, src, len, wc, len+1 );
	ret = SysAllocString(wc);
	delete[] wc;
	return ret;
}


SS::SS() {
	strcpy( cdir, "$\\" );
	try_to_change = 0;
	db = NULL;
	strcpy( ssafe_ini, "\\\\AKE-SERVER\\VSS60_DB\\EFB_REQUIREMENTS\\srcsafe.ini" );
}


int SS::db_connect()
{
	HRESULT h;

	if( db )
		db_disconnect();

	if( FAILED(h=CoCreateInstance(__uuidof(VSSDatabase),0,CLSCTX_ALL,__uuidof(IVSSDatabase),(void**)&db)) ) {
		MSGBOX( "VSSDatabase is not created" );
		return FALSE;
	}

	BSTR l,p = NULL,q = NULL;

	l = char_2_BSTR( ssafe_ini );
	h = db->raw_Open(l,p,q);
	SysFreeString( l );
	if( FAILED(h) ) {
		MSGBOX( "Database was not opened" );
		return FALSE;
	}
	
	return TRUE;
}


void SS::db_disconnect()
{
	if( db )
		db->Release();
	db = NULL;
}


int SS::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
	long count = 0;
	IVSSItem *item = NULL;
	IVSSItems *items = NULL;
	PluginPanelItem *NewPanelItem = NULL;
	HRESULT h;
	BSTR l;

	if( !try_to_change ) {
		*pItemsNumber = 0;
		*pPanelItem = NULL;
	}

	if( !db_connect() )
		return FALSE;

	l = char_2_BSTR( cdir );
	h = db->get_VSSItem( l, false, &item );
	SysFreeString( l );
	if( FAILED(h) ) {
		MSGBOX( "item not opened" );
		return FALSE;
	}

	if( try_to_change ) {
		item->Release();
		db_disconnect();
		return TRUE;
	}

	h = item->get_Items(false,&items);
	if( FAILED(h) ) {
		MSGBOX( "collection not opened" );
		return FALSE;
	}

	h = items->get_Count(&count);
	if( FAILED(h) ) {
		MSGBOX( "no count" );
		return FALSE;
	}

	if( count < 0 ) {
		MSGBOX( "count < 0" );
		return FALSE;
	}

	if( count )
		NewPanelItem = (PluginPanelItem *)m_malloc( sizeof(PluginPanelItem) * (count) );

	for ( int i = 0; i < count; i++ ) {
		PluginPanelItem *p = &NewPanelItem[i];
		VARIANT v;

		VariantInit( &v );
		v.lVal = i+1;
		v.vt = VT_I4;

		IVSSItem *item;
		h = items->get_Item(v,&item);
		if( FAILED(h) ) {
			m_free( NewPanelItem );
			MSGBOX( "no item" );
			return FALSE;
		}

		BSTR name;
		int type;

		h = item->get_Type( &type );
		if( FAILED(h) )
			type = VSSITEM_FILE;

		h = item->get_Name( &name );
		if( FAILED(h) )
			name = NULL;

		BSTR_2_char( name, p->FindData.cFileName, MAX_PATH );

		SysFreeString( name );

		p->FindData.dwFileAttributes = ( VSSITEM_FILE == type ) ? 0 : FILE_ATTRIBUTE_DIRECTORY;
		item->Release();
	}

	*pItemsNumber = count;
	*pPanelItem = NewPanelItem;

	items->Release();
	item->Release();

	db_disconnect();

	return TRUE;
}

void SS::FreeFindData( PluginPanelItem *PanelItem, int ItemsNumber )
{
	m_free( PanelItem );
}


void SS::GetOpenPluginInfo( struct OpenPluginInfo *Info )
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_RAWSELECTION|OPIF_SHOWPRESERVECASE|OPIF_FINDFOLDERS;
	Info->HostFile = NULL;

	Info->CurDir = cdir;

	Info->Format = (char *) "SourceSafe";

	static char Title[NM];
	FSF.sprintf(Title," %s: %s ", "SourceSafe", Info->CurDir);
	Info->PanelTitle = Title;
}


int SS::SetDirectory( const char *Dir, int OpMode )
{
	if( !strcmp(Dir,"\\") ) {
		strcpy( cdir, "$\\" );
		return(TRUE);
	}

	if( !strcmp( Dir, ".." ) ) {
		if( !strcmp( cdir, "$\\" ) )
			return FALSE;

		char *s = cdir + strlen(cdir) - 1;

		while( s > cdir && *s != '\\' ) 
			s--;

		if( s > cdir + 1 )
			*s = 0;
		else
			s[1] = 0;
		return TRUE;
	}

	strcpy( tmp_dir, cdir );

	if( !strncmp( Dir, "$\\", 2) ) {
		strcpy( cdir, Dir );

	} else {
		if( (cdir + strlen(cdir))[-1] != '\\' )
			strcat( cdir, "\\" );
		strcat( cdir, Dir );
	}

	char *s;
	while( s = strchr( cdir, '/' ) ) *s = '\\';
	s = cdir + strlen( cdir ) - 1;
	while( *s == '\\' ) *s-- = 0;

	try_to_change = 1;
	BOOL changed = GetFindData( NULL, NULL, 0 );
	if( !changed )
		strcpy( cdir, tmp_dir );
	try_to_change = 0;

	return changed;
}


int SS::ProcessEvent( int Event, void *Param )
{
	return FALSE;
}

enum {
	GetLatest,
	CheckOut,
	CheckIn,
};


int SS::FileOp( int type, char *file, char *dest, int flags ) {

	HRESULT h;

	if( type == GetLatest || type == CheckOut ) {

		IVSSItem *item;
		BSTR l = char_2_BSTR( file );
		h = db->get_VSSItem( l, false, &item );
		SysFreeString( l );
		if( FAILED(h) ) {
			MSGBOX( "item not opened" );
			return FALSE;
		}

		l = char_2_BSTR( dest );
		h = item->raw_Get( &l, flags );
		SysFreeString( l );
		if( FAILED(h) ) {
			MSGBOX( "Get fails" );
			item->Release();
			return FALSE;
		}

		item->Release();
	}

	return TRUE;
}

enum {
	CFG_width = 70,
	CFG_height = 10,
};


static struct InitDialogItem get_dlg[]={
 { DI_DOUBLEBOX,  3,1,CFG_width-4,CFG_height-2, 0,0,0,0,(char *)1 },
 { DI_TEXT,       5,2,0,0,                      0,0,0,0,"Get Latest version:" },
 { DI_EDIT,       5,3,CFG_width-6,0,            1,(int)"ss_get",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,4,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_CHECKBOX,   5,5,0,0,                      0,1,0,0,"&Clear Read-Only attribute (only first level files)" },
 { DI_TEXT,       5,6,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_BUTTON,     CFG_width-26,7,0,0,           0,0,0,0,"&Get" },
 { DI_BUTTON,     CFG_width-17,7,0,0,           0,0,0,0,"&Cancel" },
};


enum {
   gdlg_Text = 2,
   gdlg_ro = 4,
};


int SS::GetFiles( struct PluginPanelItem *PanelItem, int ItemsNumber,
					int Move, char *DestPath, int OpMode )
{
	int retval = 1, clearro = 1;
	static int flags = VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD;
	struct FarDialogItem DialogItems[ sizeof(get_dlg) / sizeof(get_dlg[0]) ];

	if( Move )
		return 0;

	if( !((OPM_SILENT|OPM_VIEW|OPM_EDIT|OPM_FIND) & OpMode )) {

		InitDialogItems( get_dlg, DialogItems, sizeof(get_dlg) / sizeof(get_dlg[0]) );
		strcpy( DialogItems[gdlg_Text].Data, DestPath );
		DialogItems[gdlg_ro].Selected = clearro;
		if( ItemsNumber == 1 )
			FSF.sprintf( DialogItems[1].Data, "Get \"%s\" to:", PanelItem[0].FindData.cFileName );
		else 
			FSF.sprintf( DialogItems[1].Data, "Get %i items to:", ItemsNumber );

		int ec = Info.Dialog( Info.ModuleNumber, -1, -1, CFG_width, CFG_height, NULL, DialogItems, sizeof(DialogItems)/sizeof(DialogItems[0]) );

		if( ec < 0 || ec == (sizeof(get_dlg) / sizeof(get_dlg[0]))-1 )
			return FALSE;

		clearro = DialogItems[gdlg_ro].Selected;
		DestPath = DialogItems[gdlg_Text].Data;
	}

	if( !db_connect() )
		return FALSE;

	for( int i = 0; i < ItemsNumber; i++ ) {
		char *name = PanelItem[i].FindData.cFileName;

		strcpy( tmp_dir, cdir );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, name );

		strcpy( tmp_dir2, DestPath );
		strcat( tmp_dir2, "\\" );
		strcat( tmp_dir2, name );

		HANDLE sc = Info.SaveScreen(0,0,-1,-1);
		const char *msg[4] = { "Getting", "", "to", "" };
		msg[1] = tmp_dir;
		msg[3] = tmp_dir2;
		Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );

		if( FileOp( GetLatest, tmp_dir, tmp_dir2, flags ) ) {
			PanelItem[i].Flags &= ~(PPIF_SELECTED);
			if( clearro )
				SetFileAttributes( tmp_dir2, GetFileAttributes( tmp_dir2 ) & ~(FILE_ATTRIBUTE_READONLY) );
		} else
			retval = 0;

		Info.RestoreScreen( sc );
	}

	db_disconnect();

	return retval;
}
