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
	if( of == OPEN_COMMANDLINE ) {
		if( !strncmp( "$\\", (char*)item, 2 ) ) {
			strcpy( cdir, (char *) item );
			split_cdir();
			try_to_change = 1;
			BOOL changed = GetFindData( NULL, NULL, 0 );
			if( !changed ) {
				MsgBox( "wrong path", (char *)item );
				strcpy( cdir, "$\\" );
				split_cdir();
			}

		} else
			MsgBox( "wrong path", (char*)item );
	}
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

	if( !*ssini )
		return FALSE;

	if( FAILED(h=CoCreateInstance(__uuidof(VSSDatabase),0,CLSCTX_ALL,__uuidof(IVSSDatabase),(void**)&db)) ) {
		char *msg = (char *)m_malloc( 1024 );
		FSF.sprintf( msg, "VSSDatabase not created (hres=%08x)", h );
		MsgBox( "COM error", msg );
		m_free( msg );
		return FALSE;
	}

	// TODO username, pwd handling
	BSTR l, p = NULL, q = NULL;

	l = char_2_BSTR( ssini );
	h = db->raw_Open( l, p, q );
	SysFreeString( l );
	if( FAILED(h) ) {
		MsgBox( "IVSSDatabase error", "Open failed" );
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


int SS::GetFindData( PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode )
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
		db_disconnect();
		MsgBox( "Item not opened", cpath );
		return FALSE;
	}

	if( try_to_change ) {
		item->Release();
		db_disconnect();
		return TRUE;
	}

	h = item->get_Items(false,&items);
	if( FAILED(h) ) {
		item->Release();
		db_disconnect();
		MsgBox( "IVSSItem error", "items collection was not opened" );
		return FALSE;
	}

	h = items->get_Count(&count);
	if( FAILED(h) ) {
		items->Release();
		item->Release();
		db_disconnect();
		MsgBox( "IVSSItem error", "impossible to get count" );
		return FALSE;
	}

	if( count < 0 ) {
		items->Release();
		item->Release();
		db_disconnect();
		MsgBox( "IVSSItem error", "count < 0" );
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
			MsgBox( "IVSSItems error", "impossible to get item" );
			items->Release();
			item->Release();
			db_disconnect();
			return FALSE;
		}

		BSTR name,local;
		int type, co;

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

		if( name )
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
			if( FAILED(h) ) {
				cot->Release();
				couts->Release();
				goto bad_co;
			}

			BSTR_2_char( str, tmp_dir, MAX_PATH );
			if( str )
				SysFreeString( str );

			h = cot->get_LocalSpec( &str );
			if( FAILED(h) ) {
				cot->Release();
				couts->Release();
				goto bad_co;
			}

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

	if( ItemsNumber )
		m_free( PanelItem );
}


void SS::GetOpenPluginInfo( struct OpenPluginInfo *info )
{
	info->StructSize = sizeof(*info);
	info->Flags = OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_RAWSELECTION|OPIF_SHOWPRESERVECASE|OPIF_FINDFOLDERS;
	info->HostFile = NULL;

	info->CurDir = cdir;

	info->Format = MSG(msg_plugin_name);

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
		kb.CtrlTitles[5-1] = "ChckIn";
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
			// TODO close plugin
			return FALSE;
		}

		char *s = strrchr( cdir, '\\' );

		if( s && s > cdir + 1 )
			*s = 0;
		else
			cdir[2] = 0;
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
		strcpy( cdir, tmp_dir );
		split_cdir();
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
	CheckOut,			// support comment
	CheckIn,            //     -- " --
	UndoCO,
	Remove,
	ShowComment,
	AddFile,
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
			MsgBox( "get_VSSItem failed", file );
			return FALSE;
		}
	}

	if( type == GetLatest || type == CheckOut ) {

		l = char_2_BSTR( dest );

		if( type == GetLatest ) 
			h = item->raw_Get( &l, flags );
		else {
			BSTR w = char_2_BSTR( comment );
			h = item->raw_Checkout( w, l, flags);
			SysFreeString( w );
		}
		SysFreeString( l );
		if( FAILED(h) ) {
			MsgBox( "IVSSItem error", type == GetLatest ? "Get failed" : "CheckOut failed" );
			item->Release();
			return FALSE;
		}

	} else if( type == ShowComment ) {

		MsgBox( "Item's comment", "not realized due to BUG in SourceSafe TypeLib" );

	} else if( type == UndoCO ) {

		l = char_2_BSTR( dest );
		h = item->raw_UndoCheckout( l, flags );
		SysFreeString( l );
		if( FAILED(h) ) {
			MsgBox( "UndoCheckOut failed", dest );
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
				MsgBox( "Directory for item creation was not opened", file );
				return FALSE;
			}

			l = char_2_BSTR( name );
			cm = char_2_BSTR( comment );
			h = item->raw_Add( l, cm, VSSFLAG_BINBINARY, &it );
			SysFreeString( cm );
			SysFreeString( l );
			if( FAILED(h) ) {
				item->Release();
				MsgBox( "IVSSItem.Add failed", name );
				return FALSE;
			}

			// due to BUGS in SS VSSFLAG_DEL* handling
			if( !(flags&VSSFLAG_DELYES) ) {

				l = char_2_BSTR( dest );
				h = it->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
				SysFreeString( l );
				if( FAILED(h) ) {
					MsgBox( "IVSSItem.Get of just created file failed", dest );
					it->Release();
					item->Release();
					return FALSE;
				}
			}
			it->Release();

		} else
			return FALSE;

	} else if( type == Remove ) {

		h = item->put_Deleted( 1 );
		if( FAILED(h) ) {
			MsgBox( "Remove failed", file );
			item->Release();
			return FALSE;
		}

	} else if( type == CheckIn ) {

		long co;

		h = item->get_IsCheckedOut( &co );
		if( FAILED(h) ) {
			MsgBox( "IVSSItem.IsCheckedOut failed", file );
		bad_co:
			item->Release();
			return FALSE;
		}

		if( !co ) {

			const char *items[4] = { "CheckIn warning", "Not checked out.","Must be checked out before. CheckOut?", NULL };

			items[3] = file;
			int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 4, 0 );

			if( res != 0 ) {
				item->Release();
				return FALSE;
			}

			l = char_2_BSTR( dest );
			h = item->raw_Checkout( NULL, l, VSSFLAG_GETNO );
			SysFreeString( l );
			if( FAILED(h) ) {
				MsgBox( "IVSSItem.Checkout failed", dest );
				item->Release();
				return FALSE;
			}
		}

		l = char_2_BSTR( dest );
		cm = char_2_BSTR( comment );
		h = item->raw_Checkin( cm, l, flags );
		SysFreeString( cm );
		SysFreeString( l );

		if( FAILED(h) ) {
			MsgBox( "IVSSItem.Checkin failed", dest );
			item->Release();
			return FALSE;
		}

		// due to BUGS in SS VSSFLAG_DEL* handling
		if( !(flags&VSSFLAG_DELYES) ) {

			l = char_2_BSTR( dest );
			h = item->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
			SysFreeString( l );
			if( FAILED(h) ) {
				MsgBox( "IVSSItem.Get of checked in file failed", dest );
				item->Release();
				return FALSE;
			}
		}
	}

	item->Release();

	return TRUE;
}


int SS::GetFiles( struct PluginPanelItem *PanelItem, int ItemsNumber,
					int Move, char *DestPath, int OpMode )
{
	int retval = 1, clearro = 1;
	int flags = VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD;
	FarDialog dlg;

	/////////////////////// Get Dialog
	static int id_get_where, id_get_clearro, id_get_rec, id_get_cancel, id_get_to;
	static struct InitDialogItem get_dlg[] = {
	 { DI_TEXT,       1,0,0,0,		0,0,0,0,"", &id_get_to },
	 { DI_EDIT,       1,1,-2,0,		1,(int)"ss_get",DIF_HISTORY,0, "", &id_get_where },
	 { DI_TEXT,       1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,3,0,0,		0,1,0,0,(char *)msg_get_clear_ro, &id_get_clearro },
	 { DI_CHECKBOX,   1,4,0,0,		0,1,0,0,(char *)msg_get_recurs, &id_get_rec },
	 { DI_TEXT,       1,5,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_get },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_get_cancel },
	};

	/////////////////////// CheckOut Dialog
	static int id_co_where, id_co_dontget, id_co_repl, id_co_cancel, id_co_to;
	static struct InitDialogItem co_dlg[] = {
	 { DI_TEXT,       1,0,0,0,		0,0,0,0,"", &id_co_to },
	 { DI_EDIT,       1,1,-2,0,		1,(int)"ss_get",DIF_HISTORY,0, "", &id_co_where },
	 { DI_TEXT,       1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,3,0,0,		0,1,0,0,(char*)msg_co_dontgetloc, &id_co_dontget },
	 { DI_CHECKBOX,   1,4,0,0,		0,1,0,0,(char*)msg_co_replwr, &id_co_repl },
	 { DI_TEXT,       1,5,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_checkout },
	 { DI_BUTTON,     0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_co_cancel },
	};

    dlg.di = NULL;

	//
	//   Get/CheckOut Dialog
	//
	if( !((OPM_SILENT|OPM_VIEW|OPM_EDIT|OPM_FIND) & OpMode ) || Move ) {

		if( Move ) {
			//
			// Checkout
			//
			create_fdlg( &dlg, 70, 11, msg_checkout, co_dlg, array_size( co_dlg ) );
			if( ItemsNumber == 1 )
				FSF.sprintf( dlg.di[id_co_to].Data, MSG(msg_co_title1), PanelItem[0].FindData.cFileName );
			else 
				FSF.sprintf( dlg.di[id_co_to].Data, MSG(msg_co_title2), ItemsNumber );
			strcpy( dlg.di[id_co_where].Data, DestPath );
			GetRegKey( "", "checkout_dontgetlocal", dlg.di[id_co_dontget].Selected, 0 );
			GetRegKey( "", "checkout_replacewritable", dlg.di[id_co_repl].Selected, 1 );

			int ec = process_fdlg( &dlg );

			if( ec < 0 || ec == id_co_cancel ) {
				destroy_fdlg( &dlg );
				return FALSE;
			}

			if( dlg.di[id_co_dontget].Selected )
				flags |= VSSFLAG_GETNO;

			if( !dlg.di[id_co_repl].Selected ) {
				flags &= ~VSSFLAG_REPREPLACE;
				flags |= VSSFLAG_REPSKIP;
			}

			SetRegKey( "", "checkout_dontgetlocal", dlg.di[id_co_dontget].Selected );
			SetRegKey( "", "checkout_replacewritable", dlg.di[id_co_repl].Selected );
			DestPath = dlg.di[id_co_where].Data;

		} else {
		    //
		    // Get
		    //
			create_fdlg( &dlg, 70, 11, msg_getss, get_dlg, array_size( get_dlg ) );
			if( ItemsNumber == 1 )
				FSF.sprintf( dlg.di[id_get_to].Data, MSG(msg_get_title1), PanelItem[0].FindData.cFileName );
			else 
				FSF.sprintf( dlg.di[id_get_to].Data, MSG(msg_get_title2), ItemsNumber );
			strcpy( dlg.di[id_get_where].Data, DestPath );
			GetRegKey( "", "get_recursive", dlg.di[id_get_rec].Selected, 0 );
			GetRegKey( "", "get_clearro", dlg.di[id_get_clearro].Selected, 1 );

			int ec = process_fdlg( &dlg );

			if( ec < 0 || ec == id_get_cancel ) {
				destroy_fdlg( &dlg );
				return FALSE;
			}

			if( dlg.di[id_get_rec].Selected )
				flags |= VSSFLAG_RECURSYES|VSSFLAG_FORCEDIRNO;

			SetRegKey( "", "get_recursive", dlg.di[id_get_rec].Selected );
			SetRegKey( "", "get_clearro", dlg.di[id_get_clearro].Selected );
			clearro = dlg.di[id_get_clearro].Selected;
			DestPath = dlg.di[id_get_where].Data;
		}
	}

	if( !db_connect() )
		return FALSE;

	for( int i = 0; i < ItemsNumber; i++ ) {
		char *name = PanelItem[i].FindData.cFileName;

		if( PanelItem[i].FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && Move ) {
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
			const char *msg[4] = { MSG(msg_get_action), tmp_dir, MSG(msg_getco_to), tmp_dir2 };
			if( Move ) msg[0] = MSG(msg_co_action);
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

	destroy_fdlg( &dlg );
	return retval;
}


int SS::PutFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode )
{
	/////////////////////// Put Dialog
	static int id_dest, id_cmnt, id_keepco, id_remlocal, id_cancel;
	static struct InitDialogItem items[] = {
	 { DI_TEXT,       1,0,0,0,		0,0,0,0,(char *)msg_put_dest },
	 { DI_EDIT,       1,1,-2,0,		1,(int)"ss_put",DIF_HISTORY,0, "", &id_dest },
	 { DI_TEXT,       1,2,0,0,		0,0,0,0,(char *)msg_put_comment },
	 { DI_EDIT,       1,3,-2,0,		0,(int)"ss_comments",DIF_HISTORY,0, "", &id_cmnt },
	 { DI_TEXT,       1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,5,0,0,		0,0,0,0,(char *)msg_put_keepco, &id_keepco },
	 { DI_CHECKBOX,   1,6,0,0,		0,1,0,0,(char *)msg_put_remloc, &id_remlocal },
	 { DI_TEXT,       1,7,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,     0,8,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_put },
	 { DI_BUTTON,     0,8,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
	};

	char *Dest = cpath, *comments = "";
	int flags = 0;
	FarDialog dlg;

    dlg.di = NULL;
	if( !strcmp( cdir, "$\\" ) )
		return FALSE;

	if( !(OpMode&OPM_SILENT) ) {

		create_fdlg( &dlg, 70, 13, msg_checkin, items, array_size( items ) );

		strcpy( dlg.di[id_dest].Data, cpath );
		dlg.di[id_remlocal].Selected = Move;

		int ec = process_fdlg( &dlg );

		if( ec < 0 || ec == id_cancel )
			return FALSE;

		Dest = dlg.di[id_dest].Data;
		comments = dlg.di[id_cmnt].Data;

		if( dlg.di[id_remlocal].Selected )
			flags |= VSSFLAG_DELYES;
		if( dlg.di[id_keepco].Selected )
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
			const char *msg[4] = { MSG(msg_put_title), tmp_dir2, MSG(msg_put_to), tmp_dir };
			Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );
		}

		if( FileOp( CheckIn, tmp_dir, tmp_dir2, comments, flags ) )
			PanelItem[i].Flags &= ~(PPIF_SELECTED);

		if( !(OpMode & OPM_SILENT ) )
			Info.RestoreScreen( sc );
	}

	db_disconnect();
	destroy_fdlg( &dlg );

	return TRUE;
}


int SS::DeleteFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode )
{
	return FALSE;
}


int SS::ProcessKey(int Key,unsigned int ControlState)
{
	PanelInfo pi;
	Info.Control( this, FCTL_GETPANELINFO, &pi );

	// create/modify
	if( ControlState == 0 && ( Key == VK_F7 || Key == VK_F4 ) && !strcmp( cdir, "$\\" ) ) {

		FarDialog dlg;
		static int id_name, id_ssini, id_accept, id_cancel;
		static struct InitDialogItem items[] = {
		 { DI_TEXT,       1,0,0,0,		0,0,0,0,(char*)msg_db_name },
		 { DI_EDIT,       1,1,-2,0,		1,(int)"ss_dbname",DIF_HISTORY,0, "", &id_name },
		 { DI_TEXT,       1,2,0,0,		0,0,0,0,(char*)msg_db_file },
		 { DI_EDIT,       1,3,-2,0,		1,(int)"ss_ini",DIF_HISTORY,0, "", &id_ssini },
		 { DI_TEXT,       1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
		 { DI_BUTTON,     0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_create, &id_accept },
		 { DI_BUTTON,     0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
		};

		create_fdlg( &dlg, 70, 10, Key == VK_F7 ? msg_createdb : msg_modifydb, items, array_size( items ) );

		if( Key == VK_F4 ) {
			strcpy( dlg.di[id_name].Data, pi.PanelItems[pi.CurrentItem].FindData.cFileName );
			GetRegKey( "Repositories", dlg.di[id_name].Data, dlg.di[id_ssini].Data, "", 512 );
			strcpy( dlg.di[id_accept].Data, MSG(msg_key_modify) );
		}

		int ec = process_fdlg( &dlg );

		if( ec < 0 || ec == id_cancel ) {
			destroy_fdlg( &dlg );
			return TRUE;
		}

		if( Key == VK_F4 ) {

			if( strcmp( dlg.di[id_name].Data, pi.PanelItems[pi.CurrentItem].FindData.cFileName ) )
				KillRegVal( "Repositories", pi.PanelItems[pi.CurrentItem].FindData.cFileName );
		}

		SetRegKey( "Repositories", dlg.di[id_name].Data, dlg.di[id_ssini].Data );
		destroy_fdlg( &dlg );

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
			MsgBox( "Warning", "File is not checked out" );
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
			MsgBox( "Warning", "File is not checked out" );
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
