// ss.cpp

#include "ss.h"


void BSTR_2_char( BSTR b, char *dest, int max ) {

	if( b )
		WideCharToMultiByte(CP_OEMCP, 0, b, *(((long *)b)-1), dest, max	, NULL, NULL);
	else
		*dest = 0;
}


BSTR char_2_BSTR( char *src ) {

	BSTR ret;
	wchar_t *wc;
	int len = strlen( src );

	if( !len )
		return NULL;
	wc = new wchar_t[len+10];
	MultiByteToWideChar(CP_OEMCP, 0, src, len, wc, len+10 );
	ret = SysAllocString(wc);
	delete[] wc;
	return ret;
}


void comError( char *title, HRESULT code ){
	if( code != S_OK ) {
		char errorDesc[255];
		char str[255];

		IErrorInfo *info=NULL;
		GetErrorInfo(0,&info);
		if(info!=NULL) {
			BSTR desc;
			info->GetDescription(&desc);
			BSTR_2_char( desc, errorDesc, 254 );
			::SysFreeString(desc);
			info->Release();
		}
		
		FSF.sprintf( str, "%s\nHRESULT = %08x\nDesc: '%s'", title, code, errorDesc );

		Info.Message(Info.ModuleNumber,
			FMSG_WARNING|FMSG_ALLINONE|FMSG_MB_OK|FMSG_ERRORTYPE,
			(const char *)NULL,(const char * const *)str,2,0);
	};
}


SS::SS( int of, int item ) 
{
	InitializeCriticalSection( &cs );
	strcpy( cdir, "$\\" );
	project_first_open = 1;
	file_pos_state = 0;
	next_dir_is_default = 0;

	if( of == OPEN_COMMANDLINE ) {
		char *start = (char *)item;
		int force_pos = 0;

		while( *start == '\t' || *start == ' ' ) start++;
		if( *start == '"' ) start++;

		if( *start == '!' ) force_pos = 1, start++;

		if( start[0] == '$' && start[1] == '\\' )
			strcpy( cdir, start );
		else {
			strcpy( cdir, "$\\" );
			strcpy( cdir+2, start );
		}

		char *s;

		if( (s=strchr(cdir, '"')) ) *s = 0;

		while( s = strchr( cdir, '/' ) ) *s = '\\';
		s = cdir + strlen( cdir ) - 1;
		while( *s == '\\' ) *s-- = 0;

		split_cdir();

		if( strcmp( cdir, "$\\" ) ) {

			BSTR l;
			HRESULT h;
			IVSSItem *item;

			if( !db_connect() ) {
				MsgBox( "wrong path", start );
				goto fail;
			}

			if( !strcmp( cpath, "$\\" ) ) {
				h = db->get_CurrentProject( &l );

				if( SUCCEEDED(h) ) {
					BSTR_2_char( l, tmp_dir2, MAX_PATH );
					SysFreeString( l );
					if( *tmp_dir2 == '$' ) {
						char *s;
						while( s = strchr( tmp_dir2, '/' ) ) *s = '\\';
						FSF.sprintf( cdir, "$\\%s%s", cdbname, tmp_dir2+1 );
						split_cdir();
					}
				}
			}

			l = char_2_BSTR( cpath );
			h = db->get_VSSItem( l, false, &item );
			SysFreeString( l );
			if( FAILED(h) ) {
				db_disconnect();
				comError( "Item not opened", h );
			fail:
				strcpy( cdir, "$\\" );
				goto exit;
			}

			int type;
			h = item->get_Type( &type );
			if( FAILED(h) )
				type = VSSITEM_FILE;

			item->Release();
			db_disconnect();

			if( type == VSSITEM_FILE || force_pos ) {
				s = strrchr( cdir, '\\' );

				if( s ) {
					strcpy( file_to_pos, s+1 );

					if( s && s > cdir + 1 )
						*s = 0;
					else
						cdir[2] = 0;

					// position to s
					file_pos_state = 1;
				}
			}
		}
	}
exit:
	split_cdir();
	db = NULL;
}


SS::~SS() 
{
	DeleteCriticalSection( &cs );
}


int SS::db_connect()
{
	HRESULT h;

	if( !*ssini )
		return FALSE;

	if( FAILED(h=CoInitialize(NULL) ) ) {
		comError( "COMInit", h );
		return FALSE;
	}

	if( FAILED(h=CoCreateInstance(__uuidof(VSSDatabase),0,CLSCTX_ALL,__uuidof(IVSSDatabase),(void**)&db)) ) {
		comError( "VSSDatabase not created", h );
		return FALSE;
	}

	// TODO username, pwd handling
	BSTR l, p = NULL, q = NULL;

	l = char_2_BSTR( ssini );
	h = db->raw_Open( l, p, q );
	SysFreeString( l );
	if( FAILED(h) ) {
		db->Release();
		db = NULL;
		comError( "IVSSDatabase.Open failed", h );
		return FALSE;
	}

	return TRUE;
}


void SS::db_disconnect()
{
	if( db )
		db->Release();
	db = NULL;
	::CoUninitialize();
}


int SS::try_to_change( int first_enter ) 
{
	IVSSItem *item = NULL;
	HRESULT h;
	BSTR l;

	if( !db_connect() )
		return FALSE;

	if( first_enter ) {
		h = db->get_CurrentProject( &l );

		if( SUCCEEDED(h) ) {
			BSTR_2_char( l, tmp_dir2, MAX_PATH );
			SysFreeString( l );
			if( *tmp_dir2 == '$' ) {
				char *s;
				while( s = strchr( tmp_dir2, '/' ) ) *s = '\\';
				FSF.sprintf( cdir, "$\\%s%s", cdbname, tmp_dir2+1 );
				split_cdir();
			}
		}
	}

	l = char_2_BSTR( cpath );
	h = db->get_VSSItem( l, false, &item );
	SysFreeString( l );
	if( FAILED(h) ) {
		db_disconnect();
		comError( "Item not opened", h );
		return FALSE;
	}

	item->Release();
	db_disconnect();
	return TRUE;
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


int SS::get_date( IVSSItem *item, DATE *change ) 
{
	HRESULT h;
	IVSSVersions *vers;

	h = item->get_Versions( 0, &vers );
	if( FAILED(h) ) {
		comError( "IVSSItem.get_Versions failed", h );
		return FALSE;
	}

	IUnknown *gv;

	h = vers->raw__NewEnum( &gv );
	if( FAILED(h) ) {
		comError( "IVSSItem.NewEnum failed", h );
	  exit_free_versions:
		vers->Release();
		return FALSE;
	}

	IEnumVARIANT *ev;

	h = gv->QueryInterface( __uuidof( IEnumVARIANT ), (void **)&ev );
	gv->Release();
	if( FAILED(h) ) {
		comError( "IUnknown.QueryInterface(IEnumVARIANT) failed", h );
		goto exit_free_versions;
	}

	VARIANT v;
	unsigned long fetched;
	h = ev->Next( 1, &v, &fetched );
	if( FAILED(h) || !fetched ) {
		comError( "IEnumVARIANT.Next failed", h );
		ev->Release();
		goto exit_free_versions;
	}

	IVSSVersion *ver;
	
	h = v.punkVal->QueryInterface( __uuidof( IVSSVersion ), (void **)&ver );
	v.punkVal->Release();

	if( FAILED(h) ) {
		comError( "IUnknown.QueryInterface(IVSSVersion) failed", h );
		ev->Release();
		goto exit_free_versions;
	}

	h = ver->get_Date( change );
	if( FAILED(h) ) {
		comError( "IVSSVersion.get_Date failed", h );
	exit_free_all:
		ver->Release();
		ev->Release();
		goto exit_free_versions;
	}

	ver->Release();
	ev->Release();
	vers->Release();
	return TRUE;
}


int SS::GetFindData( PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode )
{
	long count = 0;
	int show_dates, markro, marksys;
	IVSSItem *item = NULL;
	IVSSItems *items = NULL;
	PluginPanelItem *NewPanelItem = NULL;
	HRESULT h;
	BSTR l;

	*pItemsNumber = 0;
	*pPanelItem = NULL;

	/*
	 *	 Root directory
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

	if( !db_connect() ) {
		MsgBox( "not connected", "wrong" );
		return FALSE;
	}

	l = char_2_BSTR( cpath );
	h = db->get_VSSItem( l, false, &item );
	SysFreeString( l );
	if( FAILED(h) ) {
		db_disconnect();
		comError( "Item not opened", h );
		return FALSE;
	}

	h = item->get_Items(false,&items);
	if( FAILED(h) ) {
		item->Release();
		db_disconnect();
		comError( "items collection was not opened", h );
		return FALSE;
	}

	h = items->get_Count(&count);
	if( FAILED(h) ) {
		items->Release();
		item->Release();
		db_disconnect();
		comError( "impossible to get count", h );
		return FALSE;
	}

	if( count < 0 ) {
		items->Release();
		item->Release();
		db_disconnect();
		comError( "IVSSItem.count < 0", h );
		return FALSE;
	}

	GetRegKey( "", "showdates", show_dates, 0 );
	GetRegKey( "", "markro", markro, 1 );
	GetRegKey( "", "marksys", marksys, 1 );

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
			comError( "impossible to get item", h );
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

		if( co && marksys )
			p->FindData.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;

		if( co != VSSFILE_CHECKEDOUT_ME && markro )
			p->FindData.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;

		if( show_dates && co != VSSFILE_CHECKEDOUT_ME && !(OpMode&OPM_FIND) ) {

			SYSTEMTIME st;
			DATE change;
			if( get_date( item, &change ) ) {
				VariantTimeToSystemTime( change, &st );
				SystemTimeToFileTime( &st, &p->FindData.ftLastAccessTime );
				p->FindData.ftLastWriteTime = p->FindData.ftLastAccessTime;
			}
		}
		
		p->CustomColumnData = (LPSTR*)m_malloc(sizeof(LPSTR)*2);
		if( co && !(OpMode&OPM_FIND) ) {
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

			if( co == VSSFILE_CHECKEDOUT_ME ) {

				strcat( tmp_dir2, "\\" );
				strcat( tmp_dir2, p->FindData.cFileName );

				WIN32_FIND_DATA fd;
				HANDLE hf = FindFirstFile( tmp_dir2, &fd );
				if( hf != INVALID_HANDLE_VALUE ) {
					p->FindData.ftCreationTime = fd.ftCreationTime;
					p->FindData.ftLastAccessTime = fd.ftLastAccessTime;
					p->FindData.ftLastWriteTime = fd.ftLastWriteTime;
					p->FindData.nFileSizeHigh = fd.nFileSizeHigh;
					p->FindData.nFileSizeLow = fd.nFileSizeLow;
					FindClose( hf );
				}

			}

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

	if( file_pos_state )
		file_pos_state = 2;

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

		FSF.sprintf( Title, MSG(msg_title_nonroot), cdbname, cpath );
		kb.Titles[5-1] = MSG(msg_kb5);
		kb.Titles[6-1] = MSG(msg_kb6);
		kb.Titles[7-1] = MSG(msg_kb7);
		kb.Titles[8-1] = MSG(msg_kb8);
		kb.CtrlTitles[4-1] = MSG(msg_kb_c4);
		kb.CtrlTitles[5-1] = MSG(msg_kb_c5);
		kb.CtrlTitles[6-1] = MSG(msg_kb_c6);
		kb.CtrlTitles[7-1] = MSG(msg_kb_c7);
		kb.CtrlTitles[8-1] = MSG(msg_kb_c8);

		kb.AltTitles[5-1] = MSG(msg_kb_a5);
		kb.AltTitles[6-1] = MSG(msg_kb_a6);

		int showdates;
		GetRegKey( "", "showdates", showdates, 0 );

		kb.CtrlAltTitles[5-1] = MSG(showdates ? msg_kb_ca5off : msg_kb_ca5on );
		kb.CtrlAltTitles[6-1] = MSG(msg_kb_ca6);
		kb.CtrlAltTitles[7-1] = MSG(msg_kb_ca7);

	} else {
		info->PanelModesArray = PanelModesArrayRoot;
		info->PanelModesNumber = sizeof(PanelModesArrayRoot)/sizeof(PanelModesArrayRoot[0]);
		info->StartPanelMode = '3';

		strcpy( Title, MSG(msg_title_root) );
		kb.Titles[3-1] = "";
		kb.Titles[4-1] = MSG(msg_kb_root4);
		kb.Titles[5-1] = "";
		kb.Titles[6-1] = "";
		kb.Titles[7-1] = MSG(msg_kb_root7);
		kb.Titles[8-1] = MSG(msg_kb_root8);
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
		else if( project_first_open )
			project_first_open = 2;

		strcat( cdir, Dir );
	}

	char *s;
	while( s = strchr( cdir, '/' ) ) *s = '\\';
	s = cdir + strlen( cdir ) - 1;
	while( *s == '\\' ) *s-- = 0;

	split_cdir();

	BOOL changed = try_to_change( project_first_open == 2 );

	if( project_first_open == 2 )
		project_first_open = 0;

	if( !changed ) {
		strcpy( cdir, tmp_dir );
		split_cdir();
	}

	return changed;
}


int SS::ProcessEvent( int Event, void *Param )
{
	if( file_pos_state == 2 ) {

		// ASSERT( Event == FE_REDRAW );

		file_pos_state = 0;

		PanelInfo pi;
		Info.Control( this, FCTL_GETPANELINFO, &pi );

		for( int i = 0; i < pi.ItemsNumber; i++ ) 
			if (!strcmpi( pi.PanelItems[i].FindData.cFileName, file_to_pos ) ) {
				PanelRedrawInfo info;
				info.CurrentItem = i;
				info.TopPanelItem = 0;
				Info.Control( this, FCTL_REDRAWPANEL, &info );
	  			break;
			}
	}

	return FALSE;
}

enum {
	GetLatest,
	CheckOut,			// support comment
	CheckIn,			//	 -- " --
	UndoCO,
	Remove,
	Destroy,
	ShowComment,
	AddFile,
	MakeDir,
	SetWF,
	Rename,
	ShowFullHist,
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
			comError( "get_VSSItem failed", h );
			return FALSE;
		}
	}

	if( type == GetLatest || type == CheckOut ) {

		l = char_2_BSTR( dest );

		SetFileApisToANSI();
		if( type == GetLatest ) 
			h = item->raw_Get( &l, flags );
		else {
			BSTR w = char_2_BSTR( comment );
			h = item->raw_Checkout( w, l, flags);
			SysFreeString( w );
		}
		SetFileApisToOEM();
		SysFreeString( l );
		if( FAILED(h) ) {
			comError( type == GetLatest ? "Get failed" : "CheckOut failed", h );
			item->Release();
			return FALSE;
		}

	} else if( type == ShowComment ) {

		IVSSVersions *vers;

		h = item->get_Versions( 0, &vers );
		if( FAILED(h) ) {
			comError( "IVSSItem.get_Versions failed", h );
			item->Release();
			return FALSE;
		}

		IUnknown *gv;

		h = vers->raw__NewEnum( &gv );
		if( FAILED(h) ) {
			comError( "IVSSItem.NewEnum failed", h );
		  exit_free_versions:
			vers->Release();
			item->Release();
			return FALSE;
		}

		IEnumVARIANT *ev;

		h = gv->QueryInterface( __uuidof( IEnumVARIANT ), (void **)&ev );
		gv->Release();
		if( FAILED(h) ) {
			comError( "IUnknown.QueryInterface(IEnumVARIANT) failed", h );
			goto exit_free_versions;
		}

		VARIANT v;
		unsigned long fetched;
		h = ev->Next( 1, &v, &fetched );
		if( FAILED(h) || !fetched ) {
			comError( "IEnumVARIANT.Next failed", h );
			ev->Release();
			goto exit_free_versions;
		}

		IVSSVersion *ver;
		
		h = v.punkVal->QueryInterface( __uuidof( IVSSVersion ), (void **)&ver );
		v.punkVal->Release();

		if( FAILED(h) ) {
			comError( "IUnknown.QueryInterface(IVSSVersion) failed", h );
			ev->Release();
			goto exit_free_versions;
		}

		long version;

		h = ver->get_VersionNumber( &version );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_VersionNumber failed", h );
		exit_free_all:
			ver->Release();
			ev->Release();
			goto exit_free_versions;
		}

		DATE date;

		h = ver->get_Date( &date );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_Date failed", h );
			goto exit_free_all;
		}

		BSTR comment, action, username, label;

		h = ver->get_Comment( &comment );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_Comment failed", h );
			goto exit_free_all;
		}

		h = ver->get_Action( &action );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_Action failed", h );
			SysFreeString( comment );
			goto exit_free_all;
		}

		h = ver->get_Username( &username );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_Username failed", h );
			SysFreeString( comment );
			SysFreeString( action );
			goto exit_free_all;
		}

		h = ver->get_Label( &label );
		if( FAILED(h) ) {
			comError( "IVSSVersion.get_Label failed", h );
			SysFreeString( comment );
			SysFreeString( action );
			SysFreeString( username );
			goto exit_free_all;
		}

		int size = 2048 + SysStringLen(comment) + SysStringLen(action) + SysStringLen(username) + SysStringLen(label);
		char *mem = (char *)m_malloc( size );

		SYSTEMTIME st;
		VariantTimeToSystemTime( date, &st );

		FSF.sprintf( mem, "File: %s\nVersion: %i (%02i:%02i %02i.%02i.%04i)\nUsername: ", file, version, st.wHour, st.wMinute, st.wDay, st.wMonth, st.wYear );

		BSTR_2_char( username, mem + strlen(mem), size - strlen(mem) );
		SysFreeString( username );

		strcat( mem, "\nComment: " );

		BSTR_2_char( comment, mem + strlen(mem), size - strlen(mem) );
		SysFreeString( comment );

		strcat( mem, "\nAction: " );

		BSTR_2_char( action, mem + strlen(mem), size - strlen(mem) );
		SysFreeString( action );

		strcat( mem, "\nLabel: " );

		BSTR_2_char( label, mem + strlen(mem), size - strlen(mem) );
		SysFreeString( label );

		Info.Message(Info.ModuleNumber,
			FMSG_ALLINONE|FMSG_MB_OK|FMSG_LEFTALIGN,
			(const char *)NULL,(const char * const *)mem,0,0);

		m_free( mem );

		ver->Release();
		ev->Release();
		vers->Release();

	} else if( type == UndoCO ) {

		l = char_2_BSTR( dest );
		SetFileApisToANSI();
		h = item->raw_UndoCheckout( l, flags );
		SetFileApisToOEM();
		SysFreeString( l );
		if( FAILED(h) ) {
			comError( "UndoCheckOut failed", h );
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
				comError( "Directory for item creation was not opened", h );
				return FALSE;
			}

			l = char_2_BSTR( name );
			cm = char_2_BSTR( comment );
			SetFileApisToANSI();
			h = item->raw_Add( l, cm, VSSFLAG_BINBINARY, &it );
			SetFileApisToOEM();
			SysFreeString( cm );
			SysFreeString( l );
			if( FAILED(h) ) {
				item->Release();
				comError( "IVSSItem.Add failed", h );
				return FALSE;
			}

			// due to BUGS in SS VSSFLAG_DEL* handling
			if( !(flags&VSSFLAG_DELYES) ) {

				l = char_2_BSTR( dest );
				SetFileApisToANSI();
				h = it->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
				SetFileApisToOEM();
				SysFreeString( l );
				if( FAILED(h) ) {
					comError( "IVSSItem.Get of just created file failed", h );
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
			comError( "Remove failed", h );
			item->Release();
			return FALSE;
		}

	} else if( type == ShowFullHist ) {

		ShowHistory( item );

	} else if( type == SetWF ) {

		l = char_2_BSTR( dest ); 
		h = item->put_LocalSpec( l );
		SysFreeString( l );
		if( FAILED(h) ) {
			comError( "Set localspec failed", h );
			item->Release();
			return FALSE;
		}

	} else if( type == Rename ) {

		l = char_2_BSTR( dest ); 
		h = item->put_Name( l );
		SysFreeString( l );
		if( FAILED(h) ) {
			comError( "rename failed", h );
			item->Release();
			return FALSE;
		}

	} else if( type == Destroy ) {

		h = item->raw_Destroy();
		if( FAILED(h) ) {
			comError( "Destroy failed", h );
			item->Release();
			return FALSE;
		}

	} else if( type == MakeDir ) {

		IVSSItem *it;

		l = char_2_BSTR( dest ); 
		cm = char_2_BSTR( comment );
		h = item->raw_NewSubproject( l, cm, &it );
		SysFreeString( cm );
		SysFreeString( l );
		if( FAILED(h) ) {
			comError( "NewSubproject failed", h );
			item->Release();
			return FALSE;
		}

		it->Release();

	} else if( type == CheckIn ) {

		long co;

		h = item->get_IsCheckedOut( &co );
		if( FAILED(h) ) {
			comError( "IVSSItem.IsCheckedOut failed", h );
		bad_co:
			item->Release();
			return FALSE;
		}

		if( co == VSSFILE_NOTCHECKEDOUT ) {

			const char *items[4] = { "CheckIn warning", "Not checked out.","Must be checked out before. CheckOut?", NULL };

			items[3] = file;
			int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 4, 0 );

			if( res != 0 ) {
				item->Release();
				return FALSE;
			}

			l = char_2_BSTR( dest );
			SetFileApisToANSI();
			h = item->raw_Checkout( NULL, l, VSSFLAG_GETNO );
			SetFileApisToOEM();
			SysFreeString( l );
			if( FAILED(h) ) {
				comError( "IVSSItem.Checkout failed", h );
				item->Release();
				return FALSE;
			}
		} // TODO else if( co == VSSFILE_CHECKEDOUT )

		l = char_2_BSTR( dest );
		cm = char_2_BSTR( comment );
			SetFileApisToANSI();
		h = item->raw_Checkin( cm, l, flags );
			SetFileApisToOEM();
		SysFreeString( cm );
		SysFreeString( l );

		if( FAILED(h) ) {
			comError( "IVSSItem.Checkin failed", h );
			item->Release();
			return FALSE;
		}

		// due to BUGS in SS VSSFLAG_DEL* handling
		if( !(flags&VSSFLAG_DELYES) ) {

			l = char_2_BSTR( dest );
			SetFileApisToANSI();
			h = item->raw_Get( &l, VSSFLAG_REPREPLACE|VSSFLAG_TIMEMOD );
			SetFileApisToOEM();
			SysFreeString( l );
			if( FAILED(h) ) {
				comError( "IVSSItem.Get of checked in file failed", h );
				item->Release();
				return FALSE;
			}
		}
	}

	item->Release();

	return TRUE;
}


char *SS::get_default_dir_for( char *sspath ) 
{
	IVSSItem *item;
	HRESULT h;

	if( !db_connect() )
		return "<error connecting to database>";

	BSTR l = char_2_BSTR( sspath );
	h = db->get_VSSItem( l, false, &item );
	SysFreeString( l );
	if( FAILED(h) ) {
		db_disconnect();
		return "<item not found>";
	}

	item->get_LocalSpec( &l );
	if( FAILED(h) ) {
		item->Release();
		db_disconnect();
		return "<get localspec failed>";
	}

	BSTR_2_char( l, tmp_dir, MAX_PATH );
	SysFreeString( l );

	item->Release();
	db_disconnect();

	return tmp_dir;
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
	 { DI_TEXT,	   1,0,0,0,		0,0,0,0,"", &id_get_to },
	 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_get",DIF_HISTORY,0, "", &id_get_where },
	 { DI_TEXT,	   1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,3,0,0,		0,1,0,0,(char *)msg_get_clear_ro, &id_get_clearro },
	 { DI_CHECKBOX,   1,4,0,0,		0,1,0,0,(char *)msg_get_recurs, &id_get_rec },
	 { DI_TEXT,	   1,5,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,	 0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_get },
	 { DI_BUTTON,	 0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_get_cancel },
	};

	/////////////////////// CheckOut Dialog
	static int id_co_where, id_co_dontget, id_co_repl, id_co_cancel, id_co_to;
	static struct InitDialogItem co_dlg[] = {
	 { DI_TEXT,	   1,0,0,0,		0,0,0,0,"", &id_co_to },
	 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_get",DIF_HISTORY,0, "", &id_co_where },
	 { DI_TEXT,	   1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,3,0,0,		0,1,0,0,(char*)msg_co_dontgetloc, &id_co_dontget },
	 { DI_CHECKBOX,   1,4,0,0,		0,1,0,0,(char*)msg_co_replwr, &id_co_repl },
	 { DI_TEXT,	   1,5,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,	 0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_checkout },
	 { DI_BUTTON,	 0,6,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_co_cancel },
	};

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
			strcpy( dlg.di[id_co_where].Data, next_dir_is_default ? get_default_dir_for(cpath) : DestPath );
			GetRegKey( "", "checkout_dontgetlocal", dlg.di[id_co_dontget].Selected, 0 );
			GetRegKey( "", "checkout_replacewritable", dlg.di[id_co_repl].Selected, 1 );

			int ec = process_fdlg( &dlg );

			if( ec < 0 || ec == id_co_cancel ) {
				destroy_fdlg( &dlg );
				next_dir_is_default = 0;
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
			strcpy( DestPath, dlg.di[id_co_where].Data );
			destroy_fdlg( &dlg );
		} else {
			//
			// Get
			//
			create_fdlg( &dlg, 70, 11, msg_getss, get_dlg, array_size( get_dlg ) );
			if( ItemsNumber == 1 )
				FSF.sprintf( dlg.di[id_get_to].Data, MSG(msg_get_title1), PanelItem[0].FindData.cFileName );
			else 
				FSF.sprintf( dlg.di[id_get_to].Data, MSG(msg_get_title2), ItemsNumber );
			strcpy( dlg.di[id_get_where].Data, next_dir_is_default ? get_default_dir_for(cpath) : DestPath );
			GetRegKey( "", "get_recursive", dlg.di[id_get_rec].Selected, 0 );
			GetRegKey( "", "get_clearro", dlg.di[id_get_clearro].Selected, 1 );

			int ec = process_fdlg( &dlg );

			if( ec < 0 || ec == id_get_cancel ) {
				destroy_fdlg( &dlg );
				next_dir_is_default = 0;
				return FALSE;
			}

			if( dlg.di[id_get_rec].Selected )
				flags |= VSSFLAG_RECURSYES|VSSFLAG_FORCEDIRNO;

			SetRegKey( "", "get_recursive", dlg.di[id_get_rec].Selected );
			SetRegKey( "", "get_clearro", dlg.di[id_get_clearro].Selected );
			clearro = dlg.di[id_get_clearro].Selected;
			strcpy( DestPath, dlg.di[id_get_where].Data );
			destroy_fdlg( &dlg );
		}

		next_dir_is_default = 0;
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

	return retval;
}


int SS::PutFiles( struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode )
{
	/////////////////////// Put Dialog
	static int id_dest, id_cmnt, id_keepco, id_remlocal, id_cancel;
	static struct InitDialogItem items[] = {
	 { DI_TEXT,	   1,0,0,0,		0,0,0,0,(char *)msg_put_dest },
	 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_put",DIF_HISTORY,0, "", &id_dest },
	 { DI_TEXT,	   1,2,0,0,		0,0,0,0,(char *)msg_put_comment },
	 { DI_EDIT,	   1,3,-2,0,		0,(int)"ss_comments",DIF_HISTORY,0, "", &id_cmnt },
	 { DI_TEXT,	   1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,5,0,0,		0,0,0,0,(char *)msg_put_keepco, &id_keepco },
	 { DI_CHECKBOX,   1,6,0,0,		0,1,0,0,(char *)msg_put_remloc, &id_remlocal },
	 { DI_TEXT,	   1,7,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,	 0,8,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_put },
	 { DI_BUTTON,	 0,8,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
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

		if( ec < 0 || ec == id_cancel ) {
			destroy_fdlg( &dlg );
			return FALSE;
		}

		Dest = dlg.di[id_dest].Data;
		comments = dlg.di[id_cmnt].Data;

		if( dlg.di[id_remlocal].Selected )
			flags |= VSSFLAG_DELYES;
		if( dlg.di[id_keepco].Selected )
			flags |= VSSFLAG_KEEPYES;
	}

	if( !db_connect() ) {
		destroy_fdlg( &dlg );
		return FALSE;
	}

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
	/////////////////////// Delete Dialog
	static int id_perm, id_fname, id_cancel, id_action;
	static struct InitDialogItem items[] = {
	 { DI_TEXT,	   1,0,0,0,		0,0,DIF_CENTERGROUP,0,(char *)msg_del_msg, &id_action },
	 { DI_TEXT,	   1,1,0,0,		0,0,DIF_CENTERGROUP,0,"", &id_fname },
	 { DI_TEXT,	   1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_CHECKBOX,   1,3,0,0,		0,0,0,0,(char *)msg_del_perm, &id_perm },
	 { DI_TEXT,	   1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
	 { DI_BUTTON,	 0,5,0,0,		1,0,DIF_CENTERGROUP,0,(char*)msg_del_ok },
	 { DI_BUTTON,	 0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_del_cancel, &id_cancel },
	};

	FarDialog dlg;
	int perm = 0;

	dlg.di = NULL;
	if( !strcmp( cdir, "$\\" ) || ItemsNumber < 1 )
		return FALSE;

	if( !(OpMode&OPM_SILENT) ) {

		int w1 = max(strlen(PanelItem[0].FindData.cFileName),strlen(MSG(msg_del_msg)))+10;
		int w2 = max(strlen(MSG(msg_del_several1)),strlen(MSG(msg_del_several2))+10)+10;
		int w3 = max(strlen(MSG(msg_del_ok))+strlen(MSG(msg_del_cancel))+18,strlen(MSG(msg_del_perm)) + 14);
		int width = max( ItemsNumber > 1 ? w2 : w1, w3 );

		create_fdlg( &dlg, width, 10, msg_del_title, items, array_size( items ) );

		GetRegKey( "", "del_perm", dlg.di[id_perm].Selected, 0 );

		if( ItemsNumber > 1 ) {
			FSF.sprintf( dlg.di[id_action].Data, MSG(msg_del_several1) );
			FSF.sprintf( dlg.di[id_fname].Data, MSG(msg_del_several2), ItemsNumber );

		} else
			strcpy( dlg.di[id_fname].Data, PanelItem[0].FindData.cFileName );

		int ec = process_fdlg( &dlg );

		if( ec < 0 || ec == id_cancel ) {
			destroy_fdlg( &dlg );
			return FALSE;
		}

		SetRegKey( "", "del_perm", dlg.di[id_perm].Selected );
		perm = dlg.di[id_perm].Selected;
	}

	if( !db_connect() ) {
		destroy_fdlg( &dlg );
		return FALSE;
	}

	for( int i = 0; i < ItemsNumber; i++ ) {
		char *name = PanelItem[i].FindData.cFileName;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, name );

		if( !perm )
			FileOp( Remove, tmp_dir, NULL, NULL, 0 );
		else 
			FileOp( Destroy, tmp_dir, NULL, NULL, 0 );
	}

	db_disconnect();
	destroy_fdlg( &dlg );

	return TRUE;
}


int SS::ProcessKey(int Key,unsigned int ControlState)
{
	PanelInfo pi, pa;
	Info.Control( this, FCTL_GETPANELINFO, &pi );

	// create/modify
	if( ControlState == 0 && ( Key == VK_F7 || Key == VK_F4 ) && !strcmp( cdir, "$\\" ) ) {

		FarDialog dlg;
		static int id_name, id_ssini, id_accept, id_cancel;
		static struct InitDialogItem items[] = {
		 { DI_TEXT,	   1,0,0,0,		0,0,0,0,(char*)msg_db_name },
		 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_dbname",DIF_HISTORY,0, "", &id_name },
		 { DI_TEXT,	   1,2,0,0,		0,0,0,0,(char*)msg_db_file },
		 { DI_EDIT,	   1,3,-2,0,		1,(int)"ss_ini",DIF_HISTORY,0, "", &id_ssini },
		 { DI_TEXT,	   1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
		 { DI_BUTTON,	 0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_create, &id_accept },
		 { DI_BUTTON,	 0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
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

		const char *items[3] = { MSG(msg_dlgdel_title), MSG(msg_dlgdel_message), NULL };

		items[2] = pi.PanelItems[pi.CurrentItem].FindData.cFileName;
		int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 3, 0 );

		if( res != 0 )
			return TRUE;

		KillRegVal( "Repositories", items[2] );

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;


	// in the project

	} else if( ControlState == PKF_CONTROL && Key == VK_F8 && strcmp( cdir, "$\\" ) ) {
		if( !*pi.PanelItems[pi.CurrentItem].CustomColumnData[0] ) {
			MsgBox( MSG(msg_undoco_warn_title), MSG(msg_undoco_warn_msg) );
			return TRUE;
		}

		const char *items[9] = { MSG(msg_undoco_title), MSG(msg_undoco_msg), NULL, NULL, "", MSG(msg_undoco_rep),MSG(msg_undoco_leave),MSG(msg_undoco_del),MSG(msg_undoco_cancel) };

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
	} else if( ControlState == PKF_CONTROL && (Key == VK_F5 || Key == VK_F6) && strcmp( cdir, "$\\" ) ) {
		if( !*pi.PanelItems[pi.CurrentItem].CustomColumnData[0] ) {
			MsgBox( MSG(msg_undoco_warn_title), MSG(msg_undoco_warn_msg) );
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
		const char *msg[4] = { MSG(msg_ci_action), NULL, MSG(msg_ci_to), NULL };
		msg[1] = tmp_dir2;
		msg[3] = tmp_dir;
		Info.Message( Info.ModuleNumber, 0, NULL, msg, 4, 0 );

		FileOp( CheckIn, tmp_dir, tmp_dir2, "", Key == VK_F5 ? 0 : VSSFLAG_DELYES );

		Info.RestoreScreen( sc );

		db_disconnect();

		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

		return TRUE;

	} else if( ControlState == PKF_CONTROL && Key == VK_F7 && strcmp( cdir, "$\\" ) ) {

		if( !db_connect() )
			return TRUE;

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		FileOp( ShowComment, tmp_dir, tmp_dir2, NULL, 0 );

		db_disconnect();

		return TRUE;

	} else if( ControlState == PKF_CONTROL && Key == VK_F4 && strcmp( cdir, "$\\" ) ) {

		Info.Control (this, FCTL_GETANOTHERPANELINFO, &pa );

		const char *items[5] = { MSG(msg_defdir_title), MSG(msg_defdir_msg), NULL, MSG(msg_defdir_to), NULL };

		items[2] = cpath;
		items[4] = pa.CurDir;
		int res = Info.Message( Info.ModuleNumber, FMSG_MB_YESNO, NULL, items, 5, 0 );

		if( res != 0 )
			return TRUE;

		if( !db_connect() )
			return TRUE;

		FileOp( SetWF, cpath, pa.CurDir, NULL, 0 );

		db_disconnect();

		return TRUE;

	} else if( ControlState == (PKF_CONTROL|PKF_ALT) && Key == VK_RETURN && pi.PanelItems[pi.CurrentItem].CustomColumnNumber == 2 &&
		*pi.PanelItems[pi.CurrentItem].CustomColumnData[1] ) {

		Info.Control( this, FCTL_SETANOTHERPANELDIR, pi.PanelItems[pi.CurrentItem].CustomColumnData[1] );
		Info.Control( this, FCTL_UPDATEANOTHERPANEL, NULL );

		Info.Control (this, FCTL_GETANOTHERPANELINFO, &pa );

		for( int i = 0; i < pa.ItemsNumber; i++ ) 
			if (!strcmpi( pa.PanelItems[i].FindData.cFileName, pi.PanelItems[pi.CurrentItem].FindData.cFileName)) {
				PanelRedrawInfo info;
				info.CurrentItem = i;
				info.TopPanelItem = 0;
				Info.Control( this, FCTL_REDRAWANOTHERPANEL, &info);
	  			break;
			}
		if( i == pi.ItemsNumber )
			Info.Control( this, FCTL_REDRAWANOTHERPANEL, NULL );

		return TRUE;

	} else if( ControlState == (PKF_CONTROL|PKF_ALT) && Key == VK_F6 ) {

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		FarDialog dlg;
		static int id_name, id_cancel;
		static struct InitDialogItem items[] = {
		 { DI_TEXT,	   1,0,0,0,		0,0,0,0,(char*)msg_ren_msg },
		 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_dbname",DIF_HISTORY,0, "", &id_name },
		 { DI_TEXT,	   1,2,0,0,		0,0,DIF_SEPARATOR,0,"" },
		 { DI_BUTTON,	 0,3,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_ren_ok },
		 { DI_BUTTON,	 0,3,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
		};

		create_fdlg( &dlg, 70, 8, msg_ren_title, items, array_size( items ) );

		strcpy( dlg.di[id_name].Data, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		int ec = process_fdlg( &dlg );

		if( ec < 0 || ec == id_cancel ) {
			destroy_fdlg( &dlg );
			return TRUE;
		}

		strcpy( tmp_dir2, dlg.di[id_name].Data );

		if( !db_connect() )
			return FALSE;

		FileOp( Rename, tmp_dir, tmp_dir2, NULL, 0 );

		db_disconnect();
		destroy_fdlg( &dlg );
		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );
		return TRUE;

	} else if( ControlState == PKF_ALT && (Key == VK_F6 || Key == VK_F5) ) {

		KeySequence ks;
		static DWORD kcode[1];

		if( strcmp( pi.PanelItems[pi.CurrentItem].FindData.cFileName, ".." ) ) {

			kcode[0] = Key == VK_F5 ? 0x0174 : 0x0175;  // KEY_F5 | KEY_F6
			ks.Count = 1;
			ks.Flags = 0;
			ks.Sequence = kcode;
			next_dir_is_default = 1;
			Info.AdvControl( Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks );
		}
		return TRUE;
	} else if( ControlState == (PKF_ALT|PKF_CONTROL) && Key == VK_F5 ) {
		int state;

		GetRegKey( "", "showdates", state, 0 );
		state = !state;

		SetRegKey( "", "showdates", state );
		Info.Control( this, FCTL_UPDATEPANEL, NULL );
		Info.Control( this, FCTL_REDRAWPANEL, NULL );

	} else if( ControlState == (PKF_ALT|PKF_CONTROL) && Key == VK_F7 ) {

		strcpy( tmp_dir, cpath );
		strcat( tmp_dir, "\\" );
		strcat( tmp_dir, pi.PanelItems[pi.CurrentItem].FindData.cFileName );

		if( !db_connect() )
			return FALSE;

		FileOp( ShowFullHist, tmp_dir, NULL, NULL, 0 );

		db_disconnect();
	}

	return FALSE;
}


int SS::MakeDirectory( char *Name, int OpMode )
{
	char comment[MAX_PATH] = "";

	if( !(OpMode&OPM_SILENT) ) {

		FarDialog dlg;
		static int id_name, id_cancel, id_cmnt;
		static struct InitDialogItem items[] = {
		 { DI_TEXT,	   1,0,0,0,		0,0,0,0,(char*)msg_dir_name },
		 { DI_EDIT,	   1,1,-2,0,		1,(int)"ss_dirname",DIF_HISTORY,0, "", &id_name },
		 { DI_TEXT,	   1,2,0,0,		0,0,0,0,(char*)msg_dir_cmnt },
		 { DI_EDIT,	   1,3,-2,0,		1,(int)"ss_dircmnt",DIF_HISTORY,0, "", &id_cmnt },
		 { DI_TEXT,	   1,4,0,0,		0,0,DIF_SEPARATOR,0,"" },
		 { DI_BUTTON,	 0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_create },
		 { DI_BUTTON,	 0,5,0,0,		0,0,DIF_CENTERGROUP,0,(char*)msg_key_cancel, &id_cancel },
		};

		create_fdlg( &dlg, 70, 10, msg_createnew, items, array_size( items ) );
		strcpy( dlg.di[id_name].Data, Name );
		int ec = process_fdlg( &dlg );

		if( ec < 0 || ec == id_cancel ) {
			destroy_fdlg( &dlg );
			return -1;
		}

		strcpy( Name, dlg.di[id_name].Data );
		strncpy( comment, dlg.di[id_cmnt].Data,MAX_PATH );
		comment[MAX_PATH-1] = 0;
		destroy_fdlg( &dlg );
	}

	if( !*Name )
		return -1;

	if( !db_connect() )
		return FALSE;

	FileOp( MakeDir, cpath, Name, comment, 0 );

	db_disconnect();

	return TRUE;
}


// History related functions

static int get_item_to_temp_file( IVSSItem* item, char* tempFileName ){

	char tTempPath[ MAX_PATH ];
	GetTempPath( sizeof( tTempPath ), tTempPath );

	if( GetTempFileName( tTempPath, "vss", 0, tempFileName ) == 0 )
		return GetLastError();

	DeleteFile( tempFileName );
	CreateDirectory( tempFileName, NULL );

	OLECHAR tPath[ MAX_PATH ];
	MultiByteToWideChar( CP_OEMCP, 0, tempFileName, -1, tPath, MAX_PATH );

	BSTR tItemFile = NULL;
	item->get_Name( &tItemFile );
	wcscat( tPath, L"\\" );
	wcscat( tPath, tItemFile );
	::SysFreeString( tItemFile );

	BSTR tFileNameW = ::SysAllocString( tPath );
	HRESULT tResult = item->raw_Get( &tFileNameW, 0 );

	if ( FAILED( tResult )){
		::SysFreeString( tFileNameW );
		return tResult;
	}

	BSTR_2_char( tFileNameW, tempFileName, MAX_PATH );
	SetFileAttributes( tempFileName, 0 );

	::SysFreeString( tFileNameW );
	return 0;
}


static void ViewItem( IVSSItem *item, int version ){

	char tFileName[ MAX_PATH ];

	if( !item )
		return;

	if ( get_item_to_temp_file( item, tFileName ))
		return;

	char tTitle[ MAX_PATH ];

	BSTR tItemFile = NULL;
	item->get_Name( &tItemFile );
	BSTR_2_char( tItemFile, tTitle+1, MAX_PATH-1 );
    ::SysFreeString( tItemFile );
    tTitle[0] = '[';

    FSF.sprintf( tTitle + strlen( tTitle ), ":%d]", version );

	HANDLE tSavedScreen = Info.SaveScreen( 0, 0, -1, -1 );
	Info.Viewer( tFileName, tTitle, 0, 0, -1, -1, VF_DELETEONCLOSE | VF_DISABLEHISTORY );
	Info.RestoreScreen( tSavedScreen );
}


long WINAPI sttDlgProc( HANDLE hDlg, int Msg, int Param1, long Param2 ){

	static SS *ss;
	static int current;

	switch( Msg ) {
	case DN_INITDIALOG:
		ss = (SS *)Param2;
		current = 0;
		break;
	case DN_LISTCHANGE:
		current = Param2;
		break;
	case DN_BTNCLICK:
		switch( Param1 ){
		case 3: 
			ViewItem( ss->dlg_list[current].item, ss->dlg_list[current].version );
			return TRUE;
		}
		break;
	case DN_KEY:
		if( Param1 == 1 && (Param2 == 0x172 /*F3*/ || Param2 == 0xd /*Enter*/ ) ) {
			ViewItem( ss->dlg_list[current].item, ss->dlg_list[current].version );
			return TRUE;
		}
		break;
	}
	return Info.DefDlgProc( hDlg, Msg, Param1, Param2 );
}


void SS::ShowHistory( IVSSItem *item ){

	struct THistoryItem {
		long mVersion;
		DATE mDate;
		char *mAction, *mUser;
		bool mIsLabel;
		IVSSItem *mItem;
		THistoryItem *next;
	};

	HRESULT h;
	char title[255];
	FarList locHistory;
	unsigned colWidth[ 3 ] = { 0, 0, 0 };
	char tBuffer[ 256 ];

	THistoryItem *list = NULL, *list_end = NULL;
	int list_count = 0;

	IVSSVersions* vers = NULL;

	h = item->get_Versions( 0, &vers );
	if( FAILED(h) ) {
		comError( "IVSSItem.get_Versions failed", h );
		return;
	}

	IUnknown *gv;

	h = vers->raw__NewEnum( &gv );
	if( FAILED(h) ) {
		comError( "IVSSItem.NewEnum failed", h );
	  exit_free_versions:
		vers->Release();
		while( list ) {
			list_end = list;
			m_free( list->mAction );
			m_free( list->mUser );
			if( list->mItem )
				list->mItem->Release();
			list = list->next;
			m_free( list_end );
		}
		return;
	}

	IEnumVARIANT *ev;

	h = gv->QueryInterface( __uuidof( IEnumVARIANT ), (void **)&ev );
	gv->Release();
	if( FAILED(h) ) {
		comError( "IUnknown.QueryInterface(IEnumVARIANT) failed", h );
		goto exit_free_versions;
	}

	while( 1 ) {

		VARIANT v;
		unsigned long fetched;
		h = ev->Next( 1, &v, &fetched );

		if( fetched == 0 )
			break;

		if( FAILED(h) ) {
			comError( "IEnumVARIANT.Next failed", h );
			ev->Release();
			goto exit_free_versions;
		}

		IVSSVersion *ver;

		h = v.punkVal->QueryInterface( __uuidof( IVSSVersion ), (void **)&ver );
		v.punkVal->Release();

		if( FAILED(h) ) {
			comError( "IUnknown.QueryInterface(IVSSVersion) failed", h );
			ev->Release();
			goto exit_free_versions;
		}

		THistoryItem* li = (THistoryItem*)m_malloc(sizeof(THistoryItem));

		ver->get_Date( &li->mDate );
		ver->get_VersionNumber( &li->mVersion );

		BSTR l;

		if( SUCCEEDED( ver->get_Action( &l )) ) {
			BSTR_2_char( l, tBuffer, 256 );
			SysFreeString( l );
		} else 
			strcpy( tBuffer, "<wrong>" );
		li->mAction = m_strdup( tBuffer );

		if( SUCCEEDED( ver->get_Username( &l )) ) {
			BSTR_2_char( l, tBuffer, 256 );
			SysFreeString( l );
		} else 
			strcpy( tBuffer, "<wrong>" );
		li->mUser = m_strdup( tBuffer );

		if( SUCCEEDED( ver->get_Label( &l )) ) {
			if ( l[ 0 ] )
				li->mIsLabel = true;
			::SysFreeString( l );
		}       

		if( !li->mIsLabel ) {
			VARIANT tVersionNumber;
			tVersionNumber.vt = VT_INT;
			tVersionNumber.lVal = li->mVersion;
			if( FAILED( item->get_Version( tVersionNumber, &li->mItem )) )
				li->mItem = NULL;
		}

		colWidth[ 0 ] = max( colWidth[ 0 ], strlen( ltoa( li->mVersion, tBuffer, 10 )) );
		colWidth[ 1 ] = max( colWidth[ 1 ], strlen( li->mUser ) );
		colWidth[ 2 ] = max( colWidth[ 2 ], strlen( li->mAction ) );

		if( list ) {
			list_end = list_end->next = li;
		} else {
			list_end = list = li;
		}
		list_count++;

		ver->Release();
	}

	ev->Release();
	vers->Release();

	InitDialogItem items[]={
	 { DI_LISTBOX,   1,0,-18,-2,	1,0,DIF_BTNNOCLOSE|DIF_LISTNOBOX,0, "" },
	 { DI_BUTTON,	-15,0,10,1,		0,0,0,1,( char* )msg_hist_ok },
	 { DI_BUTTON,	-15,2,10,1,		0,0,DIF_BTNNOCLOSE,0,(char*)msg_hist_view },
	 { DI_BUTTON,	-15,4,10,1,		0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_details },
	 { DI_BUTTON,	-15,6,10,1,		0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_get },
	 { DI_BUTTON,	-15,8,10,1,		0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_co },
	 { DI_BUTTON,	-15,10,10,1,	0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_diff },
	 { DI_BUTTON,	-15,12,10,1,	0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_pin },
	 { DI_BUTTON,	-15,14,10,1,	0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_rollback },
	 { DI_BUTTON,	-15,16,10,1,	0,0,DIF_BTNNOCLOSE|DIF_DISABLE,0,(char*)msg_hist_help }
	};

	int tDialogWidth = 80, tDialogHeight = 21;

	int tListWidth = colWidth[ 0 ] + colWidth[ 1 ] + colWidth[ 2 ] + 29;

	if( tListWidth > tDialogWidth -8 + items[0].X2 - items[0].X1 ) {
		CONSOLE_SCREEN_BUFFER_INFO tInfo;
		GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &tInfo );

		tDialogWidth = min( tInfo.dwSize.X, tListWidth-items[0].X2+8+items[0].X1 );
		tDialogHeight = max( 21, min( tInfo.dwSize.Y, ( tDialogWidth/4 + 1 ) ) );
	}

	FarDialog dlg;
	create_fdlg( &dlg, tDialogWidth, tDialogHeight, msg_hist_title, items, array_size( items ) );

	BSTR name;
	h = item->get_Name(&name);
	if( SUCCEEDED(h) ) {
		BSTR_2_char( name, dlg.di[0].Data + strlen(dlg.di[0].Data), 512 - strlen(dlg.di[0].Data) );
		SysFreeString( name );
	}

	dlg.di[1].ListItems = &locHistory;
	locHistory.ItemsNumber = list_count;
	locHistory.Items = (FarListItem *)m_malloc( sizeof(FarListItem) * list_count );

	dlg_list = (DLGList *)m_malloc( sizeof(DLGList) * list_count );

	FSF.sprintf( tBuffer,
		"%%%dd | %%-%ds | %%02d/%%02d/%%04d %%02d:%%02d | %%-%ds",
		colWidth[ 0 ], colWidth[ 1 ], colWidth[ 2 ] );

	// for safety
	char *buffer = (char *)m_malloc( tListWidth + 128 );

	FarListItem* fi = locHistory.Items;
	for( int i = 0; i < list_count; i++, fi++ ){
		SYSTEMTIME st;

		VariantTimeToSystemTime( list->mDate, &st );
		FSF.sprintf( buffer, tBuffer, 
				list->mVersion, list->mUser, st.wDay, st.wMonth, st.wYear,
				st.wHour, st.wMinute, list->mAction );

		strncpy( fi->Text, buffer, sizeof( fi->Text ) );
		dlg_list[i].version = list->mVersion;
		dlg_list[i].item = list->mItem;

		if ( list->mIsLabel )
			memset( fi->Text, '>', colWidth[ 0 ] );

		list_end = list;
		m_free( list->mAction );
		m_free( list->mUser );
		list = list->next;
		m_free( list_end );
	}

	m_free( buffer );

	Info.DialogEx( Info.ModuleNumber, -1, -1, dlg.X, dlg.Y,
		NULL, dlg.di, dlg.itemsNumber, 0, 0, sttDlgProc, long( this ));

	for( i = 0; i < list_count; i++ )
		if( dlg_list[i].item )
			dlg_list[i].item->Release();
	m_free( dlg_list );
	m_free( locHistory.Items );
	destroy_fdlg( &dlg );
}
