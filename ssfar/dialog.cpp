// ss.h

#include "ss.h"

void MsgBox( char *title, char *message ) 
{
	const char *g_m[4] = { NULL, NULL, "", "OK" };

	g_m[0] = title;
	g_m[1] = message;
	Info.Message( Info.ModuleNumber, 0, NULL, g_m, 4, 1 );
}


void InitDialogItems( const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber )
{
	int i;
	struct FarDialogItem *PItem = Item;
	const struct InitDialogItem *PInit = Init;

	for( i = 0; i < ItemsNumber; i++, PItem++, PInit++ ) {
		PItem->Type=PInit->Type;
		PItem->X1 = PInit->X1;
		PItem->Y1 = PInit->Y1;
		PItem->X2 = PInit->X2;
		PItem->Y2 = PInit->Y2;
		PItem->Focus=PInit->Focus;
		PItem->Selected=PInit->Selected;
		PItem->Flags=PInit->Flags;
		PItem->DefaultButton=PInit->DefaultButton;
		if( (unsigned int)PInit->Data < 2000 )
			strcpy( PItem->Data, MSG((unsigned int)PInit->Data) );
		else
			strcpy( PItem->Data, PInit->Data );
	}
}


void create_fdlg( FarDialog *fdlg, int X, int Y, int Title, struct InitDialogItem *Item, int number )
{
	fdlg->itemsNumber = number + 1;
	fdlg->di = (FarDialogItem *)m_malloc( sizeof(FarDialogItem)*fdlg->itemsNumber );

	fdlg->X = X;
	fdlg->Y = Y;

	struct FarDialogItem *pi = fdlg->di;
	const struct InitDialogItem *ps = Item;
	int i;

	pi->Type = DI_DOUBLEBOX;
	pi->X1 = 3; pi->Y1 = 1; pi->X2 = X - 4; pi->Y2 = Y - 2;
	strcpy( pi->Data, MSG(Title) );

	for( i = 0, pi++; i < number; i++, pi++, ps++ ) {

		pi->Type = ps->Type;
		pi->X1 = ps->X1 >= 0 ? ps->X1 + 4 : X - 4 + ps->X1 ;
		pi->Y1 = ps->Y1 >= 0 ? ps->Y1 + 2 : Y - 2 + ps->Y1 ;
		pi->X2 = ps->X2 >= 0 ? ps->X2 + 4 : X - 4 + ps->X2 ;
		pi->Y2 = ps->Y2 >= 0 ? ps->Y2 + 2 : Y - 2 + ps->Y2 ;
		pi->Focus = ps->Focus;
		pi->Selected = ps->Selected;
		pi->Flags = ps->Flags;
		pi->DefaultButton = ps->DefaultButton;
		if( (unsigned int)ps->Data < 2000 )
			strcpy( pi->Data, MSG((unsigned int)ps->Data) );
		else
			strcpy( pi->Data, ps->Data );

		if( ps->position )
			*ps->position = i+1;
	}
}

void destroy_fdlg( FarDialog *fdlg )
{
	if( fdlg->di )
		m_free(fdlg->di);
}


int process_fdlg( FarDialog *fdlg )
{
	return Info.Dialog( Info.ModuleNumber, -1, -1, fdlg->X, fdlg->Y, NULL, fdlg->di, fdlg->itemsNumber );
}
