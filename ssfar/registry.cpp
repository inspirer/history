// registry.cpp

// (originaly from NetReg.cpp)

#include "ss.h"

char *FmtSSS="%s%s%s";

HKEY CreateRegKey( const char *Key )
{
	HKEY hKey;
	DWORD Disposition;
	char FullKeyName[512];

	FSF.sprintf( FullKeyName, FmtSSS, PluginRootKey, *Key ? "\\":"", Key );

	RegCreateKeyEx( HKEY_CURRENT_USER, FullKeyName, 0, NULL, 0, KEY_WRITE, NULL,
			&hKey, &Disposition );
	return hKey;
}


HKEY OpenRegKey( const char *Key )
{
	HKEY hKey;
	char FullKeyName[512];
	FSF.sprintf( FullKeyName, FmtSSS, PluginRootKey, *Key ? "\\":"", Key );
	if( RegOpenKeyEx( HKEY_CURRENT_USER, FullKeyName, 0, 
		KEY_QUERY_VALUE|KEY_SET_VALUE, &hKey ) != ERROR_SUCCESS )
			return NULL;
	return hKey;
}


void SetRegKey( const char *Key, const char *ValueName, char *ValueData )
{
	HKEY hKey = CreateRegKey( Key );
	RegSetValueEx( hKey, ValueName, 0, REG_SZ, (BYTE*)ValueData, strlen(ValueData) + 1 );
	RegCloseKey( hKey );
}


void KillRegVal( const char *Key, const char *ValueName )
{
	HKEY hKey = OpenRegKey( Key );
	RegDeleteValue( hKey, ValueName );
	RegCloseKey( hKey );
}

void SetRegKey( const char *Key, const char *ValueName, DWORD ValueData )
{
	HKEY hKey = CreateRegKey( Key );
	RegSetValueEx( hKey, ValueName, 0, REG_DWORD, (BYTE *)&ValueData, sizeof(ValueData) );
	RegCloseKey( hKey );
}


int GetRegKey( const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize)
{
	HKEY hKey = OpenRegKey( Key );
	DWORD Type;
	int ExitCode = RegQueryValueEx( hKey, ValueName, 0, &Type, (BYTE*)ValueData, &DataSize );

	RegCloseKey( hKey );
	if( hKey == NULL || ExitCode != ERROR_SUCCESS ) {
		strcpy( ValueData, Default );
		return FALSE;
	}
	return TRUE;
}


int GetRegKey( const char *Key, const char *ValueName, int &ValueData, DWORD Default )
{
	HKEY hKey = OpenRegKey( Key );
	DWORD Type, Size = sizeof( ValueData );
	int ExitCode = RegQueryValueEx( hKey, ValueName, 0, &Type, (BYTE *)&ValueData, &Size );

	RegCloseKey(hKey);
	if( hKey == NULL || ExitCode != ERROR_SUCCESS ) {
		ValueData = Default;
		return FALSE;
	}
	return TRUE;
}


int EnumRegKey( const char *key, void (*save)( char *name, void *data ), void *data )
{

	HKEY hKey = OpenRegKey( key );
	int res = ERROR_SUCCESS;
	char name[MAX_PATH];

	if( !hKey )
		hKey = CreateRegKey( key );

	if( !hKey )
		return 0;

	for( int index = 0; res == ERROR_SUCCESS; index++ ) {

		unsigned long sz1, type;

		sz1 = MAX_PATH;

		res = RegEnumValue( hKey, index, name, &sz1, NULL, &type, NULL, NULL );
		if( res == ERROR_SUCCESS && type == REG_SZ ) {
			save(name, data);
		}
	}

	RegCloseKey(hKey);
	return 1;
}
