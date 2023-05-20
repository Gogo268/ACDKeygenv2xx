#include <windows.h>
#include <windowsx.h>
#include "resource.h"

static int MagicNum[] = {
	0x3fee, 0x3a9b, 0x3977, 0x37a4, 0x3583, 0x3019, 0x2f86, 0x2e55,
	0x2bd1, 0x2836, 0x2746, 0x25df, 0x22a8, 0x20a0, 0x1ce6, 0x1996,
	0x18c8, 0x18c1, 0x13eb, 0x111f, 0x0eae, 0x0d6f, 0x0cd8, 0x0be2,
	0x0bd2, 0x0acb, 0x0a8c, 0x044f, 0x0 };

static WORD MagXor[] = {
	0x7a01, 0x49a0, 0x534f };

BYTE	RegStr[ 30 ], bLen, szTotal[ 30 ];
BYTE	RegCode[ 30 ], bNum, Bits, Byte, Count;
WORD	Max, Min;

BYTE GetRegStr( HWND hDlg )
{
	BYTE	pStr[ 128 ], i, j, t, len;
	HWND	hwnd;

	hwnd = GetDlgItem( hDlg, IDC_NAMEEDIT );
	GetWindowText( hwnd, pStr, 20 );
	strupr( pStr );
	t = strlen( pStr );
	len = 0;
	for( i = 0; i < t; i ++ ){
		if( ( pStr[ i ] > 0x40 && pStr[ i ] < 0x5b ) || pStr[ i ] == 0x20 )
			pStr[ len ++ ] = pStr[ i ];
	}
	pStr[ len ] = 0;
	if( len <= 5 ) {
		hwnd = GetDlgItem( hDlg, IDC_NUMEDIT );
		SetWindowText( hwnd, "Error: Invalid registration name." );
		return 0;
	}

	t = len / 6 + 1;
	i = 1;
	for( j = 0; j < len; j += t )
		RegStr[ i ++ ] = pStr[ j ] > 0x40 ? pStr[ j ] - 0x40 : 0x1b;
	RegStr[ i ++ ] = 0x1c;
	bLen = i;
	return 1;
}

void SetBit( int Flag )
{
	Byte >>= 1;
	if( Flag )
		Byte |= 0x80;
	if( ( -- Bits ) == 0 ) {
		if( bNum < 0xf ) {
			RegCode[ bNum ] = Byte;
			Bits = 8;
			bNum ++;
		}
	}
}

void GenByte( int Flag )
{
	SetBit( Flag );
	if( Count ) {
		Flag = !Flag;
		do {
			SetBit( Flag );
			Count --;
		} while( Count );
	}
}

void CalcReg( void )
{
	int		i;
	DWORD	j, k;

	for( i = 0; i < bLen; i ++ ) {
		j = MagicNum[ RegStr[ i ] - 1 ] *
			( Max - Min + 1l ) / 0x3feel + Min - 1;
		k = MagicNum[ RegStr[ i ] ] *
			( Max - Min + 1l ) / 0x3feel + Min;
		Max = j;
		Min = k;
		while( 1 ) {
			if( Max >= 0x8000 ) {
				if( Min < 0x8000 ) {
					if( Min < 0x4000 || Max >= 0xc000 )
						break;
					Max += 0xc000;
					Min += 0xc000;
					Count ++;
				} else {
					Max += 0x8000;
					Min += 0x8000;
					GenByte( 1 );
				}
			} else {
				GenByte( 0 );
			}
			Max <<= 1;
			Max ++;
			Min <<= 1;
		}
	}
	GenByte( 1 );
	if( Bits < 8 ) {
		Byte >>= Bits;
		if( Byte ) RegCode[ bNum++ ] = Byte;
	}
	RegCode[ bNum++ ] = 0;
	bNum >>= 1;
}

void ShowUserRegNum( HWND hDlg )
{
	int		i;
	HWND	hwnd;
	BYTE	buf[ 30 ];

	if( !GetRegStr( hDlg ) )
		return;
	hwnd = GetDlgItem( hDlg, IDC_NUMEDIT );
	buf[ 0 ] = 0;
	RegStr[ 0 ] = 1;
	Max = 0xffff;
	Min = 0x0;
	bNum = 0;
	Bits = 8;
	Byte = 0;
	Count = 0;
	CalcReg();

	if( bNum < 2 ) {
		SetWindowText( hwnd, "Error: Invalid registration name." );
		return;
	}

	for( i = 0; i < bNum; i ++ )
		wsprintf( buf, "%s%05u", buf, ( ( WORD* )RegCode )[ i ] ^ MagXor[ i ] );
	if( bNum < 3 )
		strcat( buf, "21327" );
	SetWindowText( hwnd, buf );
}

BOOL CALLBACK RegDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_SHOWWINDOW:
		SetFocus( GetDlgItem( hDlg, IDC_NAMEEDIT ) );
		break;

	case WM_COMMAND:
		switch( GET_WM_COMMAND_ID( wParam, lParam ) ) {
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			break;

		case IDC_GETBUTTON:
			ShowUserRegNum( hDlg );
			break;

		case IDC_ABOUTBUTTON:
			MessageBox( hDlg, 
				"ACDSee32 v2.41 Keymaker by [eGIS!/CORE].",
				"About",
				MB_OK + MB_ICONQUESTION );
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow )
{
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_REGDIALOG ),
		NULL,
		RegDlgProc );
	return FALSE;
}