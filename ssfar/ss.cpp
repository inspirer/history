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


SS::SS( int of, int item ) 
{
	strcpy( cdir, "$\\" );
	split_cdir();
	try_to_change = 0;
	db = NULL;
}

SS::~SS() 
{
}


int SS::db_connect()
{
	HRESULT h;

	if( db )
		db_disconnect();

	if( !*ssini )
		return FALSE;

	if( FAILED(h=CoCreateInstance(__uuidof(VSSDatabase),0,CLSCTX_ALL,__uuidof(IVSSDatabase),(void**)&db)) ) {
		char *msg = (char *)m_malloc( 1024 );
		FSF.sprintf( msg, "COM: VSSDatabase: not created (hres=%08x)", h );
		MSGBOX( msg );
		m_free( msg );
		return FALSE;
	}

	BSTR l,p = NULL,q = NULL;

	l = char_2_BSTR( ssini );
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


void add_item( char *name, char *val, void *data ) {

	SS *ss = (SS *)data;
	list_of_id *n = (list_of_id *)m_malloc( sizeof(list_of_id) + strlen(name) + strlen(val) );

	strcpy( n->name, name );
	n->val = n->name + strlen(name) + 1;
	strcpy( n->val, val );
	n->next = ss->db_found;
	ss->db_found = n;
    ss->db_count++;
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

	/*
     *     Root directory
     */
	if( !strcmp( cdir, "$\\" ) ) {
		db_count = 0;
		db_found = NULL;

		EnumRegKey( "Repositories", add_item, this );

		if( db_count )
			NewPanelItem = (PluginPanelItem *)m_malloc( sizeof(PluginPanelItem) * (db_count) );

		for ( int i = 0; i < db_count; i++ ) {
			list_of_id *curr = db_found;
			PluginPanelItem *p = &NewPanelItem[i];
			db_found = db_found->next;

			p->CustomColumnData = (LPSTR*)m_malloc(sizeof(LPSTR));
			p->CustomColumnData[0] = m_strdup( curr->val );
			p->CustomColumnNumber = 1;

			strcpy( p->FindData.cFileName, curr->name );
			p->FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			m_free( curr );
		}

		*pItemsNumber = db_count;
		*pPanelItem = NewPanelItem;

		return TRUE;
	}

	if( !db_connect() )
		return FALSE;

	l = char_2_BSTR( cpath );
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

		BSTR name,local;
		int type, co, version;

		h = item->get_Type( &type );
		if( FAILED(h) )
			type = VSSITEM_FILE;

		h = item->get_IsCheckedOut( (long *)&co );
		if( FAILED(h) )
			co = 0;

		h = item->get_Name( &name );
		if( FAILED(h) )
			name = NULL;

		BSTR_2_char( name, p->FindData.cFileName, MAX_PATH );

		SysFreeString( name );

		p->FindData.dwFileAttributes = ( VSSITEM_FILE == type ) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

		p->CustomColumnData = (LPSTR*)m_malloc(sizeof(LPSTR)*2);
		if( co ) {
			IVSSCheckouts *couts;
			IVSSCheckout *cot;

			h = item->get_Checkouts( &couts );
			if( FAILED(h) )
				goto bad_co;

			VariantInit( &v );
			v.lVal = 1;
			v.vt = VT_I4;
			h = couts->get_Item( v,&cot );
			if( FAILED(h) ) {
				couts->Release();
				goto bad_co;
			}

			BSTR str;

			h = cot->get_Username( &str );
			if( FAILED(h) )
				goto bad_co;

			BSTR_2_char( str, tmp_dir, MAX_PATH );
			SysFreeString( str );

			h = cot->get_LocalSpec( &str );
			if( FAILED(h) )
				goto bad_co;

			BSTR_2_char( str, tmp_dir2, MAX_PATH );
			SysFreeString( str );

			cot->Release();
			couts->Release();

			p->CustomColumnData[0] = m_strdup( tmp_dir );
			p->CustomColumnData[1] = m_strdup( tmp_dir2 );
			goto good_co;
		bad_co:
			p->CustomColumnData[0] = m_strdup("bad_checkout");
			p->CustomColumnData[1] = m_strdup( "" );
		good_co: ;
		} else {
			p->CustomColumnData[1] = m_strdup("");
			p->CustomColumnData[0] = m_strdup("");
		}
		p->CustomColumnNumber = 2;

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
	for( int i = 0; i < ItemsNumber; i++ ) {
		for( int e = 0; e < PanelItem[i].CustomColumnNumber; e++ )
			m_free( PanelItem[i].CustomColumnData[e] );
		m_free( PanelItem[i].CustomColumnData );
	}

	m_free( PanelItem );
}


void SS::GetOpenPluginInfo( struct OpenPluginInfo *info )
{
	info->StructSize = sizeof(*info);
	info->Flags = OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_RAWSELECTION|OPIF_SHOWPRESERVECASE|OPIF_FINDFOLDERS;
	info->HostFile = NULL;

	info->CurDir = cdir;

	info->Format = (char *) "SourceSafe";

	info->PanelTitle = Title;
	memset( &kb, 0, sizeof( kb ) );
	info->KeyBar = &kb;

	static char *ColumnTitles[3];
	ColumnTitles[0] = MSG(msg_title1);
	ColumnTitles[1] = MSG(msg_title2);
	ColumnTitles[2] = MSG(msg_title3);

	static char *ColumnTitlesRoot[2];
	ColumnTitlesRoot[0] = MSG(msg_root_title1);
	ColumnTitlesRoot[1] = MSG(msg_root_title2);

	static struct PanelMode PanelModesArrayRoot[10];
	PanelModesArrayRoot[3].ColumnTypes = "N,C0";
	PanelModesArrayRoot[3].ColumnWidths = "14,0";
	PanelModesArrayRoot[3].ColumnTitles = ColumnTitlesRoot;
	PanelModesArrayRoot[3].FullScreen = FALSE;

	static struct PanelMode PanelModesArray[10];
	PanelModesArray[3].ColumnTypes = "N,C0,C1";
	PanelModesArray[3].ColumnWidths = "20,10,0";
	PanelModesArray[3].ColumnTitles = ColumnTitles;
	PanelModesArray[3].FullScreen = FALSE;

	if( *cdbname ) {
		info->PanelModesArray = PanelModesArray;
		info->PanelModesNumber = sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
		info->StartPanelMode = '3';

		FSF.sprintf(Title," SS: %s, %s ", cdbname, cpath );
		kb.Titles[5-1] = "Get";
		kb.Titles[6-1] = "ChkOut";
		kb.Titles[7-1] = "MkProj";
		kb.Titles[8-1] = "Remove";
		kb.CtrlTitles[5-1] = "CheckI";
		kb.CtrlTitles[6-1] = "UndoCO";
		kb.CtrlTitles[7-1] = "Commnt";
		kb.CtrlTitles[8-1] = "";

	} else {
		info->PanelModesArray = PanelModesArrayRoot;
		info->PanelModesNumber = sizeof(PanelModesArrayRoot)/sizeof(PanelModesArrayRoot[0]);
		info->StartPanelMode = '3';

		FSF.sprintf(Title," SS: list " );
		kb.Titles[3-1] = "";
		kb.Titles[4-1] = "Modify";
		kb.Titles[5-1] = "";
		kb.Titles[6-1] = "";
		kb.Titles[7-1] = "Create";
		kb.Titles[8-1] = "Remove";
	}
}


// fills: cpath, cdbname, ssini
void SS::split_cdir() {

	char *l = cdir + 2, c;
	
	while( *l && *l != '\\' ) l++;
	c = *l; *l = 0;
	strcpy( cdbname, cdir + 2 );
    *l = c;

    FSF.sprintf( cpath, "$%s", l );
	if( !cpath[1] )
		cpath[1] = '\\', cpath[2] = 0;

	if( *cdbname )
		if( !GetRegKey( "Repositories", cdbname, ssini, "", MAX_PATH ) )
			*ssini = 0;
}

int SS::SetDirectory( const char *Dir, int OpMode )
{
	if( !strcmp(Dir,"\\") ) {
		strcpy( cdir, "$\\" );
		split_cdir();
		return(TRUE);
	}

	if( !strcmp( Dir, ".." ) ) {
		if( !strcmp( cdir, "$\\" ) ) {
			//Info.Control( this, FCTL_CLOSEPLUGIN, NULL );
			return FALSE;
		}

		char *s = cdir + strlen(cdir) - 1;

		while( s > cdir && *s != '\\' ) 
			s--;

		if( s > cdir + 1 )
			*s = 0;
		else
			s[1] = 0;
		split_cdir();
		return TRUE;
	}

	strcpy( tmp_dir, cdir );

	if( Dir[0] == '$' ) {
		strcpy( cdir, Dir );

	} else {
		if( strcmp( cdir, "$\\" ) )
			strcat( cdir, "\\" );
		strcat( cdir, Dir );
	}

	char *s;
	while( s = strchr( cdir, '/' ) ) *s = '\\';
	s = cdir + strlen( cdir ) - 1;
	while( *s == '\\' ) *s-- = 0;

	split_cdir();

	try_to_change = 1;
	BOOL changed = GetFindData( NULL, NULL, 0 );
	if( !changed ) {
		split_cdir();
		strcpy( cdir, tmp_dir );
	}
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
	UndoCO,
	AddFile,
	Remove,
	ShowComment,
};


int SS::FileOp( int type, char *file, char *dest, char *comment, int flags ) {

	HRESULT h;
	IVSSItem *item;
	BSTR cm;

	BSTR l = char_2_BSTR( file );
	h = db->get_VSSItem( l, false, &item );
	SysFreeString( l );
	if( FAILED(h) ) {
		if( type == CheckIn ) {
			type = AddFile;

		} else {
			MSGBOX( "item not opened" );
			return FALSE;
		}
	}

	if( type == GetLatest || type == CheckOut ) {

		l = char_2_BSTR( dest );

		if( type == GetLatest ) 
			h = item->raw_Get( &l, flags );
		else {
			h = item->raw_Checkout( NULL, l, flags);
		}
		SysFreeString( l );
		if( FAILED(h) ) {
			MSGBOX( "Get/CheckOut fails" );
			item->Release();
			return FALSE;
		}

	} else if( type == ShowComment ) {

		MSGBOX( "not realized due to BUG in SourceSafe TypeLib" );		

	} else if( type == UndoCO ) {

		l = char_2_BSTR( dest );
		h = item->raw_UndoCheckout( l, flags );
		SysFreeString( l );
		if( FAILED(h) ) {
			MSGBOX( "Undo CheckOut fails" );
			item->Release();
			return FALSE;
		}
	} else if( type == AddFile ) {
		char *name = strrchr( file, '\\' );

		if( name ) {
			*name++ = 0;

			IVSSItem *it;

			BSTR l = char_2_BSTR( file );
			h = db->get_VSSItem( l, false, &item );
			SysFreeString( l );
			if( FAILED(h) ) {
				MSGBOX( "item not opened" );
				return FALSE;
			}

			l = char_2_BSTR( name );
			cm = char_2_BSTR( comment );
			h = item->raw_Add( l, cm, VSSFLAG_BINBINARY, &it );
			SysFreeString( l );
			SysFreeString( cm );
			if( FAILED(h) ) {
				item->Release();
				MSGBOX( "item not opened" );
				return FALSE;
			}

			// due to BUGS in SS VSSFLAG_DEL* handling
			if( !(flags&VSSFLAG_DELYES) ) {

				l = char_2_BSTR( dest );
				h = it->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
				SysFreeString( l );
				if( FAILED(h) ) {
					MSGBOX( "Get failed" );
					it->Release();
					item->Release();
					return FALSE;
				}
			}
			it->Release();

		} else
			return FALSE;

	} else if( type == Remove ) {

		h = item->put_Deleted(1);
		if( FAILED(h) ) {
			MSGBOX( "Destroy failed" );
			item->Release();
			return FALSE;
		}

	} else if( type == CheckIn ) {

		long co;

		h = item->get_IsCheckedOut( &co );
		if( FAILED(h) ) {
		bad_co:
			item->Release();
			return FALSE;
		}

		if( !co ) {

			const char *items[3] = { "CheckIn", "Not checked out. Do you wish to checkout before", NULL };

			items[2] = file;
			int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 3, 0 );

			if( res != 0 ) {
				item->Release();
				return FALSE;
			}

			l = char_2_BSTR( dest );
			h = item->raw_Checkout( NULL, l, VSSFLAG_GETNO );
			SysFreeString( l );
			if( FAILED(h) ) {
				MSGBOX( "CheckOut fails" );
				item->Release();
				return FALSE;
			}
		}

		l = char_2_BSTR( dest );
		cm = char_2_BSTR( comment );
		h = item->raw_Checkin( cm, l, flags );
		SysFreeString( l );
		SysFreeString( cm );

		if( FAILED(h) ) {
			MSGBOX( "CheckIn fails" );
			item->Release();
			return FALSE;
		}

		// due to BUGS in SS VSSFLAG_DEL* handling
		if( !(flags&VSSFLAG_DELYES) ) {

			l = char_2_BSTR( dest );
			h = item->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
			SysFreeString( l );
			if( FAILED(h) ) {
				MSGBOX( "Get fails" );
				item->Release();
				return FALSE;
			}
		}
	}

	item->Release();

	return TRUE;
}

enum {
	get_width = 70,
	get_height = 11,
};


static struct InitDialogItem get_dlg[]={
 { DI_DOUBLEBOX,  3,1,get_width-4,get_height-2, 0,0,0,0,(char *)msg_getss },
 { DI_TEXT,       5,2,0,0,                      0,0,0,0,"Get Latest version:" },
 { DI_EDIT,       5,3,get_width-6,0,            1,(int)"ss_get",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,4,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_CHECKBOX,   5,5,0,0,                      0,1,0,0,"&Clear Read-Only attribute (only first level files)" },
 { DI_CHECKBOX,   5,6,0,0,                      0,1,0,0,"&Recursive" },
 { DI_TEXT,       5,7,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_BUTTON,     0,8,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_get },
 { DI_BUTTON,     0,8,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel },
};


enum {
	gdlg_title = 0,
	gdlg_Text = 2,
	gdlg_ro = 4,
	gdlg_dontgetlocal = 4,
	gdlg_recurs = 5,
	gdlg_replace_wr = 5,
	gdlg_key_accept = 7,
	gdlg_key_cancel = 8,
};

/*
 *   Get AND CheckOut. 
 */

int SS::GetFiles( struct PluginPanelItem *PanelItem, int ItemsNumber,
					int Move, char *DestPath, int OpMode )
{
	int retval = 1, clearro = 1;
	int flags = VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD;
	struct FarDialogItem DialogItems[ sizeof(get_dlg) / sizeof(get_dlg[0]) ];

	//
	//   Get/CheckOut Dialog
	//
	if( !((OPM_SILENT|OPM_VIEW|OPM_EDIT|OPM_FIND) & OpMode )  || Move ) {

		InitDialogItems( get_dlg, DialogItems, sizeof(get_dlg) / sizeof(get_dlg[0]) );
		strcpy( DialogItems[gdlg_Text].Data, DestPath );

		if( Move ) {
			//
			// Checkout
			//
			strcpy( DialogItems[gdlg_title].Data, MSG(msg_checkout) );
			strcpy( DialogItems[gdlg_dontgetlocal].Data, "&Don't get local copy" );
			GetRegKey( "", "checkout_dontgetlocal", DialogItems[gdlg_dontgetlocal].Selected, 0 );
			//DialogItems[gdlg_recurs].Flags |= DIF_DISABLE;
			strcpy( DialogItems[gdlg_replace_wr].Data, "&Replace writable (skip otherwise)" );
			GetRegKey( "", "checkout_replacewritable", DialogItems[gdlg_replace_wr].Selected, 1 );
			strcpy( DialogItems[gdlg_key_accept].Data, MSG(msg_key_checkout) );
			DialogItems[gdlg_key_accept].X1 -= 5;
		} else {
		    //
		    // Get
		    //
			GetRegKey( "", "get_recursive", DialogItems[gdlg_recurs].Selected, 0 );
			GetRegKey( "", "get_clearro", clearro, clearro );
			DialogItems[gdlg_ro].Selected = clearro;
		}

		if( ItemsNumber == 1 )
			FSF.sprintf( DialogItems[1].Data, "%s \"%s\" to:", Move ? "CheckOut" : "Get", PanelItem[0].FindData.cFileName );
		else 
			FSF.sprintf( DialogItems[1].Data, "%s %i items to:", Move ? "CheckOut" : "Get", ItemsNumber );

		int ec = Info.Dialog( Info.ModuleNumber, -1, -1, get_width, get_height, NULL, DialogItems, sizeof(DialogItems)/sizeof(DialogItems[0]) );

		if( ec < 0 || ec == gdlg_key_cancel )
			return FALSE;

		if( !Move ) {
		    //
		    // Get
		    //
			if( DialogItems[gdlg_recurs].Selected )
				flags |= VSSFLAG_RECURSYES|VSSFLAG_FORCEDIRYES;

			SetRegKey( "", "get_recursive", DialogItems[gdlg_recurs].Selected );
			SetRegKey( "", "get_clearro", DialogItems[gdlg_ro].Selected );
			clearro = DialogItems[gdlg_ro].Selected;

		} else {
		    //
		    // CheckOut
		    //
			if( DialogItems[gdlg_dontgetlocal].Selected )
				flags |= VSSFLAG_GETNO;

			if( !DialogItems[gdlg_replace_wr].Selected ) {
				flags &= ~VSSFLAG_REPREPLACE;
				flags |= VSSFLAG_REPSKIP;
			}

			SetRegKey( "", "checkout_dontgetlocal", DialogItems[gdlg_dontgetlocal].Selected );
			SetRegKey( "", "checkout_replacewritable", DialogItems[gdlg_replace_wr].Selected );
		}

		DestPath = DialogItems[gdlg_Text].Data;
	}

	if( !db_connect() )
		return FALSE;

	for( int i = 0; i < ItemsNumber; i++ ) {
		char *name = PanelItem[i].FindData.cFileName;

		if( PanelItem[i].FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
			retval = 0;
			continue;
		}

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, name );

		strcpy( tmp_dir2, DestPath );
		strcat( tmp_dir2, "\\" );
		strcat( tmp_dir2, name );

		HANDLE sc;
		
		if( !(OpMode & OPM_SILENT ) ) {
			sc = Info.SaveScreen(0,0,-1,-1);
			const char *msg[4] = { "Getting", "", "to", "" };
			if( Move )
				msg[0] = "CheckingOut";
			msg[1] = tmp_dir;
			msg[3] = tmp_dir2;
			Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );
		}

		if( FileOp( Move ? CheckOut : GetLatest, tmp_dir, tmp_dir2, "", flags ) ) {
			PanelItem[i].Flags &= ~(PPIF_SELECTED);
			if( clearro && !Move )
				SetFileAttributes( tmp_dir2, GetFileAttributes( tmp_dir2 ) & ~(FILE_ATTRIBUTE_READONLY) );
		} else
			retval = 0;

		if( !(OpMode & OPM_SILENT ) )
			Info.RestoreScreen( sc );
	}

	db_disconnect();

	return retval;
}


enum {
	put_width = 70,
	put_height = 13,
};


static struct InitDialogItem put_dlg[]={
 { DI_DOUBLEBOX,  3,1,put_width-4,put_height-2, 0,0,0,0,(char *)msg_checkin },
 { DI_TEXT,       5,2,0,0,                      0,0,0,0,"To:" },
 { DI_EDIT,       5,3,put_width-6,0,            1,(int)"ss_put",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,4,0,0,                      0,0,0,0,"Comment:" },
 { DI_EDIT,       5,5,put_width-6,0,            0,(int)"ss_comments",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,6,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_CHECKBOX,   5,7,0,0,                      0,0,0,0,"&Keep checked out" },
 { DI_CHECKBOX,   5,8,0,0,                      0,1,0,0,"&Remove local copy" },
 { DI_TEXT,       5,9,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_BUTTON,     0,10,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_put },
 { DI_BUTTON,     0,10,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel },
};

enum {
	pdlg_from = 2,
	pdlg_cmnt = 4,
	pdlg_keepco = 6,
	pdlg_remlocal = 7,
	pdlg_count = sizeof(put_dlg) / sizeof(put_dlg[0]),
	pdlg_cancel = pdlg_count-1,
};


int SS::PutFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode )
{
	struct FarDialogItem DialogItems[ pdlg_count ];
	char *Dest = cpath, *comments = "";
	int flags = 0;

	if( !strcmp( cdir, "$\\" ) )
		return FALSE;

	if( !(OpMode&OPM_SILENT) ) {

		InitDialogItems( put_dlg, DialogItems, pdlg_count );
		strcpy( DialogItems[pdlg_from].Data, cpath );
		DialogItems[pdlg_remlocal].Selected = Move;

		int ec = Info.Dialog( Info.ModuleNumber, -1, -1, put_width, put_height, NULL, DialogItems, pdlg_count );

		if( ec < 0 || ec == pdlg_cancel )
			return FALSE;

		Dest = DialogItems[pdlg_from].Data;
		comments = DialogItems[pdlg_cmnt].Data;

		if( DialogItems[pdlg_remlocal].Selected )
			flags |= VSSFLAG_DELYES;
		if( DialogItems[pdlg_keepco].Selected )
			flags |= VSSFLAG_KEEPYES;
	}

	if( !db_connect() )
		return FALSE;

	for( int i = 0; i < ItemsNumber; i++ ) {
		char *name = PanelItem[i].FindData.cFileName;

		strcpy( tmp_dir, Dest );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, name );

		GetCurrentDirectory( MAX_PATH, tmp_dir2 );
		strcat( tmp_dir2, "\\" );
		strcat( tmp_dir2, name );

		HANDLE sc;
		
		if( !(OpMode & OPM_SILENT ) ) {
			sc = Info.SaveScreen(0,0,-1,-1);
			const char *msg[4] = { "CheckingIn", NULL, "to", NULL };
			msg[1] = tmp_dir2;
			msg[3] = tmp_dir;
			Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );
		}

		if( FileOp( CheckIn, tmp_dir, tmp_dir2, comments, flags ) )
			PanelItem[i].Flags &= ~(PPIF_SELECTED);

		if( !(OpMode & OPM_SILENT ) )
			Info.RestoreScreen( sc );
	}

	db_disconnect();

	return TRUE;
}


enum {
	cm_width = 70,
	cm_height = 10,
};


static struct InitDialogItem create_dlg[]={
 { DI_DOUBLEBOX,  3,1,cm_width-4,cm_height-2, 0,0,0,0,(char *)msg_createdb },
 { DI_TEXT,       5,2,0,0,                      0,0,0,0,"Database name:" },
 { DI_EDIT,       5,3,cm_width-6,0,    			1,(int)"ss_dbname",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,4,0,0,                      0,0,0,0,"ssafe.ini (with path):" },
 { DI_EDIT,       5,5,cm_width-6,0,				1,(int)"ss_ini",DIF_HISTORY,0, "" },
 { DI_TEXT,       5,6,0,0,                      0,0,DIF_SEPARATOR,0,"" },
 { DI_BUTTON,     0,7,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_create },
 { DI_BUTTON,     0,7,0,0,           			0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel },
};

enum {
	cdlg_title = 0,
	cdlg_name = 2,
	cdlg_ssini = 4,
	cdlg_accept = 6,
};


int SS::ProcessKey(int Key,unsigned int ControlState)
{
	PanelInfo pi;
	struct FarDialogItem DialogItems[ sizeof(create_dlg) / sizeof(create_dlg[0]) ];

	Info.Control( this, FCTL_GETPANELINFO, &pi );

	// create/modify
	if( ControlState == 0 && ( Key == VK_F7 || Key == VK_F4 ) && !strcmp( cdir, "$\\" ) ) {

		InitDialogItems( create_dlg, DialogItems, sizeof(create_dlg) / sizeof(create_dlg[0]) );

		if( Key == VK_F4 ) {
			strcpy( DialogItems[cdlg_title].Data, MSG(msg_modifydb) );
			strcpy( DialogItems[cdlg_name].Data, pi.PanelItems[pi.CurrentItem].FindData.cFileName );
			GetRegKey( "Repositories", DialogItems[cdlg_name].Data, DialogItems[cdlg_ssini].Data, "", 512 );
			strcpy( DialogItems[cdlg_accept].Data, MSG(msg_key_modify) );
		}

		int ec = Info.Dialog( Info.ModuleNumber, -1, -1, cm_width, cm_height, NULL, DialogItems, sizeof(DialogItems)/sizeof(DialogItems[0]) );

		if( ec < 0 || ec == (sizeof(create_dlg) / sizeof(create_dlg[0]))-1 )
			return TRUE;

		if( Key == VK_F4 ) {

			if( strcmp( DialogItems[cdlg_name].Data, pi.PanelItems[pi.CurrentItem].FindData.cFileName ) )
				KillRegVal( "Repositories", pi.PanelItems[pi.CurrentItem].FindData.cFileName );

			SetRegKey( "Repositories", DialogItems[cdlg_name].Data, DialogItems[cdlg_ssini].Data );

		} else {
			SetRegKey( "Repositories", DialogItems[cdlg_name].Data, DialogItems[cdlg_ssini].Data );
		}

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;

	} else if( ControlState == 0 && Key == VK_F8 && !strcmp( cdir, "$\\" ) ) {

		const char *items[3] = { "Delete", "Do you wish to delete the entry", NULL };

		items[2] = pi.PanelItems[pi.CurrentItem].FindData.cFileName;
		int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 3, 0 );

		if( res != 0 )
			return TRUE;

		KillRegVal( "Repositories", items[2] );

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;

	} else if( ControlState == 0 && Key == VK_F8 && strcmp( cdir, "$\\" ) ) {

		if( pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			return TRUE;

		const char *items[3] = { "Delete", "Do you wish to delete the file", NULL };

		items[2] = pi.PanelItems[pi.CurrentItem].FindData.cFileName;
		int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 3, 0 );

		if( res != 0 )
			return TRUE;

		if( !db_connect() )
			return TRUE;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		FileOp( Remove, tmp_dir, NULL, NULL, 0 );

		db_disconnect();

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;

	} else if( ControlState == PKF_CONTROL && Key == VK_F6 && strcmp( cdir, "$\\" ) ) {
		if( !*pi.PanelItems[pi.CurrentItem].CustomColumnData[0] ) {
			MSGBOX( "File is not checked out" );
			return TRUE;
		}

		const char *items[9] = { "Undo CheckOut", "Do you wish to undo the checkout", NULL, NULL, "", "Replace", "Leave", "Delete", "Cancel" };

		items[2] = pi.PanelItems[pi.CurrentItem].FindData.cFileName;
		items[3] = pi.PanelItems[pi.CurrentItem].CustomColumnData[0];
		int res = Info.Message( Info.ModuleNumber, 0, NULL, items, 9, 4 );

		if( res < 0 || res > 2 )
			return TRUE;

		int flags = 0;

		switch( res ) {
		case 0: // Replace
			flags |= VSSFLAG_REPREPLACE|VSSFLAG_DELNO;
			break;
		case 1: // Leave
			flags |= VSSFLAG_DELNOREPLACE;
			break;
		case 2: // Delete
			flags |= VSSFLAG_DELYES;
			break;
		}


		if( !db_connect() )
			return TRUE;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		strcpy( tmp_dir2, pi.PanelItems[pi.CurrentItem].CustomColumnData[1] );
		strcat( tmp_dir2, "\\" );
		strcat( tmp_dir2, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		FileOp( UndoCO, tmp_dir, tmp_dir2, "", flags );

		db_disconnect();

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;
	} else if( ControlState == PKF_CONTROL && Key == VK_F5 && strcmp( cdir, "$\\" ) ) {
		if( !*pi.PanelItems[pi.CurrentItem].CustomColumnData[0] ) {
			MSGBOX( "File is not checked out" );
			return TRUE;
		}

		if( !db_connect() )
			return FALSE;

		char *name = pi.PanelItems[pi.CurrentItem].FindData.cFileName;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, name );

		strcpy( tmp_dir2, pi.PanelItems[pi.CurrentItem].CustomColumnData[1] );
		strcat( tmp_dir2, "\\" );
		strcat( tmp_dir2, name );

		HANDLE sc = Info.SaveScreen(0,0,-1,-1);
		const char *msg[4] = { "CheckingIn", NULL, "to", NULL };
		msg[1] = tmp_dir2;
		msg[3] = tmp_dir;
		Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );

		FileOp( CheckIn, tmp_dir, tmp_dir2, "", VSSFLAG_DELYES );

		Info.RestoreScreen( sc );

		db_disconnect();

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;

	} else if( ControlState == PKF_CONTROL && Key == VK_F7 && strcmp( cdir, "$\\" ) ) {

		if( pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			return TRUE;

		if( !db_connect() )
			return TRUE;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		FileOp( ShowComment, tmp_dir, tmp_dir2, NULL, 0 );

		db_disconnect();

		return TRUE;
	}

	return FALSE;
}
