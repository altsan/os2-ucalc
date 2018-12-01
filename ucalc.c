#include "ucalc.h"


/* ------------------------------------------------------------------------- *
 * Main program (including initialization, message loop, and final cleanup)  *
 * ------------------------------------------------------------------------- */
int main( int argc, char *argv[] )
{
    CALGLOBAL global;
    HAB        hab;                          // anchor block handle
    HMQ        hmq;                          // message queue handle
    HWND       hwndFrame,                    // window handle
               hwndAccel,                    // acceleration table
               hwndHelp;                     // help instance
    QMSG       qmsg;                         // message queue
    HELPINIT   helpInit;                     // help init structure
    CHAR       szRes[ SZRES_MAXZ ],          // buffer for string resources
               szError[ SZRES_MAXZ ] = "Error";
    BOOL       fInitFailure = FALSE;
#ifdef NO_BUTTON_ICONS
    ULONG      cb = 0;
#endif

    hab = WinInitialize( 0 );
    if ( hab == NULLHANDLE ) {
        sprintf( szError, "WinInitialize() failed.");
        fInitFailure = TRUE;
    }

    if ( ! fInitFailure ) {
        hmq = WinCreateMsgQueue( hab, 0 );
        if ( hmq == NULLHANDLE ) {
            sprintf( szError, "Unable to create message queue:\nWinGetLastError() = 0x%X\n", WinGetLastError(hab) );
            fInitFailure = TRUE;
        }
    }

    if ( ! fInitFailure ) {

        memset( &global, 0, sizeof( global ));
        global.hab       = hab;
        global.hmq       = hmq;

#ifdef NO_BUTTON_ICONS
        // Set PM codepage to UPF-8 (Unicode)
        WinSetCp( global.hmq, 1207 );

        // Workaround for bug in PM Unicode support: if an association font is set,
        // bitmap and PS fonts won't display Unicode characters.
        PrfQueryProfileSize( HINI_USERPROFILE, "PM_SystemFonts", "PM_AssociateFont", &cb );
        if ( cb )
            global.fUniFont = TRUE;
#endif

        if ( global.flStyle & FL_SIZE_LARGE ) {
            strcpy( global.szBtnFont, SZ_FONT_BUTTON_LG );
            strcpy( global.szSymFont, SZ_FONT_SYMBOL_LG );
            strcpy( global.szNumFont, SZ_FONT_ENTRY_LG  );
        }
        else {
            strcpy( global.szBtnFont, SZ_FONT_BUTTON_SM );
            strcpy( global.szSymFont, SZ_FONT_SYMBOL_SM );
            strcpy( global.szNumFont, SZ_FONT_ENTRY_SM );
        }
        global.fHexMode = FALSE;
        global.pOperations = (PCALCULATION) calloc( 1, sizeof( CALCULATION ));

        // Load the main dialog window
        hwndFrame = WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP, MainWndProc, 0, ID_UCALC, &global );
        if ( hwndFrame == NULLHANDLE ) {
            sprintf( szError, "Failed to load dialog resource:\nWinGetLastError() = 0x%X\n", WinGetLastError(hab) );
            fInitFailure = TRUE;
        }
    }

    if ( fInitFailure ) {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError, "Program Initialization Error", 0, MB_CANCEL | MB_ERROR );
    } else {

        // Initialize acceleration table
        hwndAccel = WinLoadAccelTable( hab, 0, ID_UCALC );
        WinSetAccelTable( hab, hwndAccel, hwndFrame );

        // Initialize online help
//        if ( ! WinLoadString( hab, 0, IDS_HELP_TITLE, SZRES_MAXZ-1, szRes ))
            sprintf( szRes, "Help");

        helpInit.cb                       = sizeof( HELPINIT );
        helpInit.pszTutorialName          = NULL;
        helpInit.phtHelpTable             = (PHELPTABLE) MAKELONG( ID_UCALC, 0xFFFF );
        helpInit.hmodHelpTableModule      = 0;
        helpInit.hmodAccelActionBarModule = 0;
        helpInit.fShowPanelId             = 0;
        helpInit.idAccelTable             = 0;
        helpInit.idActionBar              = 0;
        helpInit.pszHelpWindowTitle       = szRes;
        helpInit.pszHelpLibraryName       = HELP_FILE;

        hwndHelp = WinCreateHelpInstance( hab, &helpInit );
        WinAssociateHelpInstance( hwndHelp, hwndFrame );

        // Now run the main program message loop
        while ( WinGetMsg( hab, &qmsg, 0, 0, 0 )) WinDispatchMsg( hab, &qmsg );

        SaveSettings( hwndFrame );
    }

    // Clean up and exit
    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * Window procedure for the main program dialog.                             *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PCALGLOBAL pGlobal;
    PSWP   pswp,
           pswpOld;


    switch( msg ) {

        case WM_INITDLG:
            // Save the global data to a window word
            pGlobal = (PCALGLOBAL) mp2;
            WinSetWindowPtr( hwnd, 0, pGlobal );
            WindowSetup( hwnd );
            WinShowWindow( hwnd, TRUE );
            return (MRESULT) FALSE;


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                // Command buttons
                //
                case IDD_CLEAR:
                    ClearAll( hwnd );
                    break;

                case IDD_CE:
                    // clear entryfield
                    WinSetDlgItemText( hwnd, IDD_ENTRY, "0");
                    WinSetDlgItemText( hwnd, IDD_HEXVALUE, "00000000");
                    break;

                case IDD_BACK:
                    DeleteDigit( hwnd );
                    break;

                // Memory operations
                //
                case IDD_MEMIN:
                    MemorySet( hwnd, pGlobal );
                    break;
                case IDD_MEMOUT:
                    MemoryClear( pGlobal );
                    break;
                case IDD_MEMREC:
                    MemoryGet( hwnd, pGlobal );
                    break;

                // Number buttons
                //
                case IDD_ZERO:
                    AppendDigit( hwnd, '0');
                    break;
                case IDD_ONE:
                    AppendDigit( hwnd, '1');
                    break;
                case IDD_TWO:
                    AppendDigit( hwnd, '2');
                    break;
                case IDD_THREE:
                    AppendDigit( hwnd, '3');
                    break;
                case IDD_FOUR:
                    AppendDigit( hwnd, '4');
                    break;
                case IDD_FIVE:
                    AppendDigit( hwnd, '5');
                    break;
                case IDD_SIX:
                    AppendDigit( hwnd, '6');
                    break;
                case IDD_SEVEN:
                    AppendDigit( hwnd, '7');
                    break;
                case IDD_EIGHT:
                    AppendDigit( hwnd, '8');
                    break;
                case IDD_NINE:
                    AppendDigit( hwnd, '9');
                    break;
                case IDD_HEXA:
                    AppendDigit( hwnd, 'A');
                    break;
                case IDD_HEXB:
                    AppendDigit( hwnd, 'B');
                    break;
                case IDD_HEXC:
                    AppendDigit( hwnd, 'C');
                    break;
                case IDD_HEXD:
                    AppendDigit( hwnd, 'D');
                    break;
                case IDD_HEXE:
                    AppendDigit( hwnd, 'E');
                    break;
                case IDD_HEXF:
                    AppendDigit( hwnd, 'F');
                    break;
                case IDD_DECIMAL:
                    AppendDigit( hwnd, '.');
                    break;

                // Other operations
                //
                case IDD_SIGN:
                    FlipSign( hwnd );
                    break;

                case IDD_OPENPAREN:
                case IDD_CLOSEPAREN:
                    // TODO - push a new calculation on pGlobal->pOperations stack
                    break;

                case IDD_EQUALS:
                    OperationFinish( hwnd, pGlobal->pOperations );
                    break;

                case IDD_PLUS:
                case IDD_MINUS:
                    OperationAdditive( hwnd, SHORT1FROMMP( mp1 ), pGlobal->pOperations );
                    break;

                case IDD_MULTIPLY:
                case IDD_DIVIDE:
                case IDD_MODULO:
                case IDD_BITOR:
                case IDD_BITXOR:
                case IDD_BITAND:
                case IDD_BITLEFT:
                case IDD_BITRIGHT:
                    OperationMultiplicative( hwnd, SHORT1FROMMP( mp1 ), pGlobal->pOperations );
                    break;

                case IDD_POWER:
                case IDD_ROOT:
                    OperationExponential( hwnd, SHORT1FROMMP( mp1 ), pGlobal->pOperations );
                    break;

                case IDD_PI:
                case IDD_SQUARE:
                case IDD_SQROOT:
                case IDD_RECIPROCAL:
                case IDD_LOG:
                case IDD_LN:
                case IDD_BITNOT:
                    if ( ! OperationUnitary( hwnd, SHORT1FROMMP( mp1 ), pGlobal->pOperations )) {
                        ClearAll( hwnd );
                        WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
                    }
                    break;

                case ID_ABOUT:                  // Product information dialog
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) AboutDlgProc, 0, IDD_ABOUT, NULL );
                    break;

                case ID_VIEWONTOP:
                    SetTopmost( hwnd );
                    break;

                case ID_LARGE:
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_LARGE, TRUE );
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_SMALL, FALSE );
                    SetTextSize( hwnd, TRUE );
                    break;

                case ID_SMALL:
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_LARGE, FALSE );
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_SMALL, TRUE );
                    SetTextSize( hwnd, FALSE );
                    break;

                case ID_CLRLIGHT:
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRLIGHT, TRUE );
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRMONO, FALSE );
                    pGlobal->flStyle &= ~FL_CLR_MONO;
                    SetColourScheme( hwnd, pGlobal );
                    break;

                case ID_CLRMONO:
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRLIGHT, FALSE );
                    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRMONO, TRUE );
                    pGlobal->flStyle |= FL_CLR_MONO;
                    SetColourScheme( hwnd, pGlobal );
                    break;


                case ID_EXIT:                   // Exit the program
                    WinPostMsg( hwnd, WM_CLOSE, 0, 0 );
                    return (MRESULT) 0;

                default: break;

            } // end WM_COMMAND messages
            return (MRESULT) 0;


        case WM_CONTROL:
            switch( SHORT1FROMMP( mp1 )) {
            } // end WM_CONTROL messages
            break;


        case WM_MINMAXFRAME:
            pswp = (PSWP) mp1;
            if ( pswp->fl & SWP_MINIMIZE ) {
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITLEFT),  FALSE );
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITRIGHT), FALSE );
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITOR),    FALSE );
            } else if ( pswp->fl & SWP_RESTORE ) {
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITLEFT),  TRUE );
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITRIGHT), TRUE );
                WinShowWindow( WinWindowFromID(hwnd, IDD_BITOR),    TRUE );
            }
            break;


        case WM_SIZE:
            //UpdateWindowSize( hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2) );
            break;


        case WM_WINDOWPOSCHANGED:
            pswp = PVOIDFROMMP( mp1 );
            pswpOld = pswp + 1;
            // WinDefDlgProc doesn't dispatch WM_SIZE, so we do it here.
            if ( pswp->fl & SWP_SIZE ) {
                WinSendMsg( hwnd, WM_SIZE, MPFROM2SHORT(pswpOld->cx,pswpOld->cy), MPFROM2SHORT(pswp->cx,pswp->cy) );
            }
            break;


        case WM_CLOSE:
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;

    } /* end event handlers */

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * WindowSetup                                                               *
 *                                                                           *
 * Perform some initial setup on the application window.                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Handle of the main application window.                       *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void WindowSetup( HWND hwnd )
{
    PCALGLOBAL pGlobal;                 // global data
    HPOINTER   hicon;                   // main application icon
    CHAR       szIni[ CCHMAXPATH ],     // name of program INI file
               szFont[ FACESIZE+5 ];    // last used font (from INI)
    LONG       x  = 0,                  // window position values (from INI)
               y  = 0,
               cx = 0,
               cy = 0;
    BOOL       fLarge = FALSE,
               fMono  = FALSE;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );

    // Load the menu-bar
    WinLoadMenu( hwnd, 0, ID_UCALC );
    WinSendMsg( hwnd, WM_UPDATEFRAME, (MPARAM) FCF_MENU, MPVOID );

    // Set the window mini-icon
    hicon = WinLoadPointer( HWND_DESKTOP, 0, ID_UCALC );
    WinSendMsg( hwnd, WM_SETICON, MPFROMP(hicon), MPVOID );

    // Check for saved INI file settings
    memset( szIni,  0, sizeof(szIni) );
    memset( szFont, 0, sizeof(szFont) );
    LocateProfile( szIni );
    pGlobal->hIni = PrfOpenProfile( pGlobal->hab, szIni );

    LoadIniData( &x,  pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_X );
    LoadIniData( &y,  pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_Y );
    LoadIniData( &cx, pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_CX );
    LoadIniData( &cy, pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_CY );
    if ( !(x && y && cx && cy ))
        CentreWindow( hwnd );
    else
        WinSetWindowPos( hwnd, HWND_TOP, x, y, cx, cy, SWP_MOVE | SWP_SIZE | SWP_ACTIVATE );
    LoadIniData( &(pGlobal->flStyle), pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_STYLE );

    LoadIniData( &(pGlobal->dMemory), pGlobal->hIni, PRF_APP_DATA, PRF_KEY_MEMORY );

    // Subclass the entryfield rectangle to fill in the background
    pfnRecProc = WinSubclassWindow( WinWindowFromID(hwnd, IDD_EFRAME), PanelProc );

    // Update the text size
    fLarge = pGlobal->flStyle & FL_SIZE_LARGE? TRUE: FALSE;
    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_LARGE, fLarge );
    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_SMALL, !fLarge );
    SetTextSize( hwnd, fLarge );

    // Set the button colours
    fMono = pGlobal->flStyle & FL_CLR_MONO? TRUE: FALSE;
    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRLIGHT, !fMono );
    WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_CLRMONO, fMono );
    SetColourScheme( hwnd, pGlobal );
}


/* ------------------------------------------------------------------------- *
 * CentreWindow                                                              *
 *                                                                           *
 * Centres the given window on the screen.                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Handle of the window to be centred.                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void CentreWindow( HWND hwnd )
{
    LONG scr_width, scr_height;
    LONG x, y;
    SWP wp;

    scr_width = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    scr_height = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    if ( WinQueryWindowPos( hwnd, &wp )) {
        x = ( scr_width - wp.cx ) / 2;
        y = ( scr_height - wp.cy ) / 2;
        WinSetWindowPos( hwnd, HWND_TOP, x, y, wp.cx, wp.cy, SWP_MOVE | SWP_ACTIVATE );
    }

}


/* ------------------------------------------------------------------------- *
 * LocateProfile                                                             *
 *                                                                           *
 * Figure out where to place our INI file.  This will be in the same         *
 * directory as OS2.INI (the OS/2 user profile).                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     PSZ pszProfile: Character buffer to receive the INI filename.         *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void LocateProfile( PSZ pszProfile )
{
    ULONG ulRc;
    PSZ   pszUserIni,
          c;

    // Query the %USER_INI% environment variable which points to OS2.INI
    ulRc = DosScanEnv("USER_INI", &pszUserIni );
    if ( ulRc != NO_ERROR ) return;
    strncpy( pszProfile, pszUserIni, CCHMAXPATH );

    // Now change the filename portion to point to our own INI file
    if (( c = strrchr( pszProfile, '\\') + 1 ) != NULL ) {
        memset( c, 0, strlen(c) );
        strncat( pszProfile, INI_FILE, CCHMAXPATH - 1 );
    }

}


/* ------------------------------------------------------------------------- *
 * LoadIniData                                                               *
 *                                                                           *
 * Retrieve a value from the program INI file.                               *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     PVOID pszData: Buffer where the data will be written.                 *
 *     HINI  hIni   : Handle to the INI file.                                *
 *     PSZ   pszApp : INI application name.                                  *
 *     PSZ   pszKey : INI key name.                                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void LoadIniData( PVOID pszData, HINI hIni, PSZ pszApp, PSZ pszKey )
{
    BOOL  fOK;
    ULONG ulBytes;

    fOK = PrfQueryProfileSize( hIni, pszApp, pszKey, &ulBytes );
    if ( fOK && ( ulBytes > 0 )) {
        PrfQueryProfileData( hIni, pszApp, pszKey, pszData, &ulBytes );
    }
}


/* ------------------------------------------------------------------------- *
 * SaveSettings                                                              *
 *                                                                           *
 * Saves various settings to the INI file.  Called on program exit.          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Main window handle.                                          *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SaveSettings( HWND hwnd )
{
    PCALGLOBAL pGlobal;                 // global data
    SWP        wp;                      // window size/position

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    // Save the window position
    if ( WinQueryWindowPos( hwnd, &wp )) {
        PrfWriteProfileData( pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_X,  &(wp.x),  sizeof(wp.x) );
        PrfWriteProfileData( pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_Y,  &(wp.y),  sizeof(wp.y) );
        PrfWriteProfileData( pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_CX, &(wp.cx), sizeof(wp.cx) );
        PrfWriteProfileData( pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_CY, &(wp.cy), sizeof(wp.cy) );
    }
    PrfWriteProfileData( pGlobal->hIni, PRF_APP_LOOK, PRF_KEY_STYLE, &(pGlobal->flStyle), sizeof(ULONG) );
    PrfWriteProfileData( pGlobal->hIni, PRF_APP_DATA, PRF_KEY_MEMORY, &(pGlobal->dMemory), sizeof(double) );
    PrfCloseProfile( pGlobal->hIni );
}


/* ------------------------------------------------------------------------- *
 * PanelProc()                                                               *
 *                                                                           *
 * Subclassed window procedure for the rectangle control.                    *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY PanelProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    RECTL  rcl;
//    RECTL  rclFill;
    POINTL ptl;
    HPS    hps;

    switch( msg ) {
        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, CLR_BLACK );
            GpiSetColor( hps, SYSCLR_BUTTONDARK );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yBottom;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yTop;
            GpiLine( hps, &ptl );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yTop;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop;
            GpiLine( hps, &ptl );
            GpiSetColor( hps, SYSCLR_BUTTONLIGHT );
            ptl.x = rcl.xLeft + 1;
            ptl.y = rcl.yBottom;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yBottom;
            GpiLine( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop-1;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yBottom;
            GpiLine( hps, &ptl );
//          rclFill.xLeft = rcl.xLeft + 1;
//          rclFill.yBottom = rcl.yBottom + 1;
//          rclFill.xRight = rcl.xRight - 1;
//          rclFill.yTop = rcl.yTop - 1;
            WinEndPaint( hps );
            break;

        default: break;
    }

    return (MRESULT) pfnRecProc( hwnd, msg, mp1, mp2 );

}


/* ------------------------------------------------------------------------- *
 * AboutDlgProc                                                              *
 *                                                                           *
 * Dialog procedure for the product information dialog.                      *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY AboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    CHAR szBuffer[ SZRES_MAXZ ];

    switch ( msg ) {

        case WM_INITDLG:
            sprintf( szBuffer, SZ_ABOUT_VERSION, SZ_VERSION );
            WinSetDlgItemText( hwnd, IDD_ABOUT_VERSION, szBuffer );
            sprintf( szBuffer, SZ_ABOUT_COPYRT, SZ_COPYRT );
            WinSetDlgItemText( hwnd, IDD_ABOUT_COPYRT, szBuffer );
            CentreWindow( hwnd );
            break;

        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void SetButtonIcon( HWND hwndBtn, USHORT usResID )
{
    BTNCDATA  data = {0};
    WNDPARAMS wp = {0};
    HPOINTER  hicon;

    hicon = WinLoadPointer( HWND_DESKTOP, 0, usResID );
    if ( !hicon ) return;

	data.cb = sizeof( data );
	data.hImage = hicon;
	wp.fsStatus = WPM_CTLDATA;
	wp.cbCtlData = data.cb;
	wp.pCtlData = &data;

    WinSendMsg( hwndBtn, WM_SETWINDOWPARAMS, MPFROMP( &wp ), 0L );
    WinUpdateWindow( hwndBtn );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void SetColourScheme( HWND hwnd, PCALGLOBAL pGlobal )
{
    USHORT ausCommandBtns[] = { IDD_CE, IDD_BACK, IDD_MEMREC, IDD_MEMIN, IDD_MEMOUT };
    USHORT ausOperatorBtns[]= { IDD_OPENPAREN, IDD_CLOSEPAREN, IDD_MODULO, IDD_DIVIDE,
                                IDD_MULTIPLY, IDD_MINUS, IDD_PLUS, IDD_EQUALS };
    USHORT ausAlgebraBtns[] = { IDD_PI, IDD_SQUARE, IDD_POWER, IDD_SQROOT,
                                IDD_ROOT, IDD_RECIPROCAL, IDD_LOG, IDD_LN };
    USHORT ausBitwiseBtns[] = { IDD_BITOR, IDD_BITAND, IDD_BITXOR,
                                IDD_BITNOT, IDD_BITLEFT, IDD_BITRIGHT };
    ULONG  cb = sizeof( RGB );
    USHORT i;
    LONG   lClrFG, lClrBG;


    lClrBG = CLR_MATHOPS_BG;
    lClrFG = CLR_MATHOPS_FG;
    for ( i = 0; i < 8; i++ ) {
        WinSetPresParam( WinWindowFromID( hwnd, ausOperatorBtns[i] ),
                         PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
        WinSetPresParam( WinWindowFromID( hwnd, ausOperatorBtns[i] ),
                         PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );
    }

    if ( ! (pGlobal->flStyle & FL_CLR_MONO )) {
        lClrBG = CLR_COMMANDS_BG;
        lClrFG = CLR_COMMANDS_FG;
    }
    for ( i = 0; i < 5; i++ ) {
        WinSetPresParam( WinWindowFromID( hwnd, ausCommandBtns[i] ),
                         PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
        WinSetPresParam( WinWindowFromID( hwnd, ausCommandBtns[i] ),
                         PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );
    }

    if ( ! (pGlobal->flStyle & FL_CLR_MONO )) {
        lClrBG = CLR_ALGEBRA_BG;
        lClrFG = CLR_ALGEBRA_FG;
    }
    for ( i = 0; i < 8; i++ ) {
        WinSetPresParam( WinWindowFromID( hwnd, ausAlgebraBtns[i] ),
                         PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
        WinSetPresParam( WinWindowFromID( hwnd, ausAlgebraBtns[i] ),
                         PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );
    }

    if ( ! (pGlobal->flStyle & FL_CLR_MONO )) {
        lClrBG = CLR_BITOPS_BG;
        lClrFG = CLR_BITOPS_FG;
    }
    for ( i = 0; i < 6; i++ ) {
        WinSetPresParam( WinWindowFromID( hwnd, ausBitwiseBtns[i] ),
                         PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
        WinSetPresParam( WinWindowFromID( hwnd, ausBitwiseBtns[i] ),
                         PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );
    }

    lClrBG = 0x00404040L;
    lClrFG = 0x00FFFFFFL;
    if ( ! (pGlobal->flStyle & FL_CLR_MONO )) {
        lClrBG = CLR_CLEAR_BG;
        lClrFG = CLR_CLEAR_FG;
    }
    WinSetPresParam( WinWindowFromID( hwnd, IDD_CLEAR ),
                     PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_CLEAR ),
                     PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );
    WinSetPresParam( WinWindowFromID( hwnd, ID_EXIT ),
                     PP_FOREGROUNDCOLOR, cb, (RGB *)(&lClrFG) );
    WinSetPresParam( WinWindowFromID( hwnd, ID_EXIT ),
                     PP_BACKGROUNDCOLOR, cb, (RGB *)(&lClrBG) );

}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void SetTextSize( HWND hwnd, BOOL fLarge )
{
    PCALGLOBAL pGlobal;
    PSZ        pszFontB, pszFontS, pszFontN;
    ULONG      lenb, lens, lenn;
    USHORT i;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( fLarge ) {
        pGlobal->flStyle |= FL_SIZE_LARGE;
#ifdef NO_BUTTON_ICONS
        if ( pGlobal->fUniFont )
            strcpy( pGlobal->szBtnFont, SZ_FONT_UNICODE_LG );
        else
#endif
        strcpy( pGlobal->szBtnFont, SZ_FONT_BUTTON_LG );
        strcpy( pGlobal->szSymFont, SZ_FONT_SYMBOL_LG );
        strcpy( pGlobal->szNumFont, SZ_FONT_ENTRY_LG );
    }
    else {
        pGlobal->flStyle &= ~FL_SIZE_LARGE;
#ifdef NO_BUTTON_ICONS
        if ( pGlobal->fUniFont )
            strcpy( pGlobal->szBtnFont, SZ_FONT_UNICODE_SM );
        else
#endif
        strcpy( pGlobal->szBtnFont, SZ_FONT_BUTTON_SM );
        strcpy( pGlobal->szSymFont, SZ_FONT_SYMBOL_SM );
        strcpy( pGlobal->szNumFont, SZ_FONT_ENTRY_SM );
    }
    pszFontB = pGlobal->szBtnFont;
    pszFontS = pGlobal->szSymFont;
    pszFontN = pGlobal->szNumFont;
    lenb = strlen( pszFontB ) + 1;
    lens = strlen( pszFontS ) + 1;
    lenn = strlen( pszFontN ) + 1;

    WinSetPresParam( WinWindowFromID( hwnd, IDD_ENTRY ),
                     PP_FONTNAMESIZE, lenn, (PVOID)pszFontN );

    // For text buttons, switch the font size
    WinSetPresParam( WinWindowFromID( hwnd, IDD_CLEAR ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_CE ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BACK ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MEMIN ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MEMOUT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MEMREC ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MODULO ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MINUS ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_PLUS ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_DIVIDE ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_MULTIPLY ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_EQUALS ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_SIGN ),
                     PP_FONTNAMESIZE, lens, (PVOID)pszFontS );
    for ( i = 0; i < 16; i++ ) {
        WinSetPresParam( WinWindowFromID( hwnd, IDD_ZERO+i ),
                         PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    }
    WinSetPresParam( WinWindowFromID( hwnd, IDD_DECIMAL ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITOR ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITXOR ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITAND ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITNOT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITLEFT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_BITRIGHT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
/*
    WinSetPresParam( WinWindowFromID( hwnd, IDD_HEXVALUE ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_HEXLEN ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
*/
#ifdef NO_BUTTON_ICONS
    WinSetPresParam( WinWindowFromID( hwnd, IDD_PI ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_SQUARE ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_POWER ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_SQROOT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_ROOT ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_RECIPROCAL ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_LOG ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
    WinSetPresParam( WinWindowFromID( hwnd, IDD_LN ),
                     PP_FONTNAMESIZE, lenb, (PVOID)pszFontB );
#else
    SetButtonIcon( WinWindowFromID( hwnd, IDD_PI ), fLarge? 31: 11 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_SQUARE ), fLarge? 32: 12 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_POWER ), fLarge? 33: 13 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_SQROOT ), fLarge? 34: 14 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_ROOT ), fLarge? 35: 15 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_RECIPROCAL ), fLarge? 37: 17 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_LOG ), fLarge? 38: 18 );
    SetButtonIcon( WinWindowFromID( hwnd, IDD_LN ), fLarge? 39: 19 );
#endif
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void SetTopmost( HWND hwnd )
{
    ULONG  fl;
    USHORT usState;

    usState = (USHORT) WinSendDlgItemMsg( hwnd, FID_MENU, MM_QUERYITEMATTR,
                                          MPFROM2SHORT( ID_VIEWONTOP, TRUE ),
                                          MPFROMSHORT( MIA_CHECKED ));
    if ( usState == MIA_CHECKED ) {
        fl = 0;
        WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_VIEWONTOP, FALSE );
    }
    else {
        fl = WS_TOPMOST;
        WinCheckMenuItem( WinWindowFromID( hwnd, FID_MENU ), ID_VIEWONTOP, TRUE );
    }
    WinSetWindowBits( hwnd, QWL_STYLE, fl, WS_TOPMOST );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void AppendDigit( HWND hwnd, CHAR chNew )
{
    CHAR achValue[ SZENTRY_MAX + 1 ];
    PSZ  pszDigits = achValue;
    LONG cb;

    PCALGLOBAL   pGlobal;
    PCALCULATION pCalc;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( !pGlobal ) return;
    pCalc = pGlobal->pOperations;
    if ( !pCalc ) return;

    if ( pCalc->fNeedOperand ) {
        pCalc->fNeedOperand = FALSE;
        sprintf( achValue, "");
        cb = 0;
    }
    else {
        cb = WinQueryDlgItemText( hwnd, IDD_ENTRY, SZENTRY_MAX, (PSZ) achValue );
    }

    // skip over the - sign, if present
    if ( cb && ( achValue[0] == '-')) {
        pszDigits++;
        cb--;
    }

    if ( cb == MAX_DIGITS ) return;
    if (( chNew == '.') && ( strchr( pszDigits, '.') != NULL )) return;

    if (( cb == 1 ) && ( pszDigits[0] == '0') && ( chNew != '.'))
        cb = 0;
    pszDigits[ cb ] = chNew;
    pszDigits[ cb+1 ] = '\0';
    WinSetDlgItemText( hwnd, IDD_ENTRY, (PSZ) achValue );
    UpdateNotation( hwnd );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void DeleteDigit( HWND hwnd )
{
    CHAR achValue[ SZENTRY_MAX + 1 ];
    PSZ  pszDigits = achValue;
    LONG cb;

    cb = WinQueryDlgItemText( hwnd, IDD_ENTRY, SZENTRY_MAX, (PSZ) achValue );

    // skip over the - sign, if present
    if ( cb && ( achValue[0] == '-')) {
        pszDigits++;
        cb--;
    }

    if ( cb < 1 ) return;
    if ( cb == 1 )
        pszDigits[ cb-1 ] = '0';
    else
        pszDigits[ cb-1 ] = '\0';
    WinSetDlgItemText( hwnd, IDD_ENTRY, (PSZ) achValue );
    UpdateNotation( hwnd );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void FlipSign( HWND hwnd )
{
    CHAR achValue[ SZENTRY_MAX + 2 ];
    PSZ  pszDigits = achValue + 1;
    LONG cb;

    cb = WinQueryDlgItemText( hwnd, IDD_ENTRY, SZENTRY_MAX, pszDigits );
    if ( !cb ) return;
    //if (( cb == 1 ) && ( pszDigits[ 0 ] == '0')) return;

    // skip the - sign, if present
    if ( pszDigits[0] == '-')
        pszDigits++;
    else {
        pszDigits = achValue;
        achValue[0] = '-';
    }

    WinSetDlgItemText( hwnd, IDD_ENTRY, pszDigits );
    UpdateNotation( hwnd );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void SetCurrentValue( HWND hwnd, double dValue )
{
    CHAR achValue[ SZENTRY_MAX + 2 ];

    sprintf( achValue, "%G", dValue );
    WinSetDlgItemText( hwnd, IDD_ENTRY, (PSZ) achValue );
    UpdateNotation( hwnd );

}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL GetCurrentValue( HWND hwnd, double *pdValue )
{
    CHAR   achValue[ SZENTRY_MAX + 1 ];
    LONG cb;

    if ( !pdValue ) return FALSE;
    cb = WinQueryDlgItemText( hwnd, IDD_ENTRY, SZENTRY_MAX, achValue );
    if ( !cb ) return FALSE;
    if ( sscanf( achValue, "%lf", pdValue ) != 1 ) return FALSE;
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void UpdateNotation( HWND hwnd )
{
    CHAR      achValue[ SZENTRY_MAX + 1 ];
    CHAR      achNotation[ SZENTRY_MAX + 1 ];
    LONG      cb;
    long long llValue;

    cb = WinQueryDlgItemText( hwnd, IDD_ENTRY, SZENTRY_MAX, achValue );
    if ( !cb ) return;
    if ( sscanf( achValue, "%lld", &llValue ) != 1 ) return;

    if ( llabs( llValue ) > 0xFFFFFFFF )
        sprintf( achNotation, "%016llX", llValue );
    else

        sprintf( achNotation, "%08X", (long) llValue );
    WinSetDlgItemText( hwnd, IDD_HEXVALUE, achNotation );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void ClearAll( HWND hwnd )
{
    PCALGLOBAL   pGlobal;
    PCALCULATION pOps;
//    PCALCULATION pTmp;

    pGlobal = WinQueryWindowPtr( hwnd, 0 );
    if ( pGlobal ) {
        pOps = pGlobal->pOperations;
        while ( pOps != NULL ) {
            pOps->dCurrent = 0;
            pOps->dFactor  = 0;
            pOps->flAddOp  = 0;
            pOps->flMulOp  = 0;
            pOps->fNeedOperand = TRUE;
            //pTmp = pOps;
            pOps = pOps->pNext;
            //free( pTmp );
        }
    }
    WinSetDlgItemText( hwnd, IDD_ENTRY, "0");
    WinSetDlgItemText( hwnd, IDD_HEXVALUE, "00000000");
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void MemorySet( HWND hwnd, PCALGLOBAL pGlobal )
{
    double dValue;
    if ( GetCurrentValue( hwnd, &dValue ))
        pGlobal->dMemory = dValue;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void MemoryGet( HWND hwnd, PCALGLOBAL pGlobal )
{
    SetCurrentValue( hwnd, pGlobal->dMemory );
    pGlobal->pOperations->fNeedOperand = TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
void MemoryClear( PCALGLOBAL pGlobal )
{
    pGlobal->dMemory = 0;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL DoCalculation( double dRValue, ULONG ulOperator, PCALCULATION pCalc )
{
    if ( !pCalc ) return FALSE;
    switch ( ulOperator ) {
        case OP_ADD_PLUS:
            pCalc->dCurrent += dRValue;
            break;
        case OP_ADD_MINUS:
            pCalc->dCurrent -= dRValue;
            break;
        case OP_MUL_MULTIPLY:
            pCalc->dFactor *= dRValue;
            break;
        case OP_MUL_DIVIDE:
            if ( dRValue == 0 ) return FALSE;
            pCalc->dFactor = pCalc->dFactor / dRValue;
            break;
        case OP_MUL_MODULO:
            if ( dRValue == 0 ) return FALSE;
            pCalc->dFactor = (long)pCalc->dFactor % (long)dRValue;
            break;
        case OP_EXP_POWER:
            pCalc->dEbase = pow( pCalc->dEbase, dRValue );
            break;
        case OP_EXP_NROOT:
            if ( pCalc->dEbase > 0 )
                pCalc->dEbase = exp( log( pCalc->dEbase ) / dRValue );
            else if ( pCalc->dEbase == 0 )
                return TRUE;
            else
                return FALSE;
            break;
        case OP_BIT_OR:
            pCalc->dFactor = (long long) pCalc->dFactor | (long long) dRValue;
            break;
        case OP_BIT_XOR:
            pCalc->dFactor = (long long) pCalc->dFactor ^ (long long) dRValue;
            break;
        case OP_BIT_AND:
            pCalc->dFactor = (long long) pCalc->dFactor & (long long) dRValue;
            break;
        case OP_BIT_LEFT:
            pCalc->dFactor = (long long) pCalc->dFactor << (long long) dRValue;
            break;
        case OP_BIT_RIGHT:
            pCalc->dFactor = (long long) pCalc->dFactor >> (long long) dRValue;
            break;
        default: return FALSE;
    }
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL OperationUnitary( HWND hwnd, USHORT usControl, PCALCULATION pCalc )
{
    double dOperand;

    if ( !pCalc ) return FALSE;
    if ( ! GetCurrentValue( hwnd, &dOperand )) return FALSE;

    switch ( usControl ) {
        case IDD_PI:
            dOperand = M_PI;
            break;
        case IDD_SQUARE:
            dOperand = pow( dOperand, 2 );
            break;
        case IDD_SQROOT:
            dOperand = sqrt( dOperand );
            break;
        case IDD_RECIPROCAL:
            if ( dOperand == 0 ) return FALSE;
            dOperand = 1 / dOperand;
            break;
        case IDD_LOG:
            if ( dOperand == 0 ) return FALSE;
            dOperand = log10( dOperand );
            break;
        case IDD_LN:
            if ( dOperand == 0 ) return FALSE;
            dOperand = log( dOperand );
            break;
        case IDD_BITNOT:
            dOperand = ~((long long) dOperand);
            break;
        default: return FALSE;
    }

    SetCurrentValue( hwnd, dOperand );
    pCalc->fNeedOperand = TRUE;
    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL OperationAdditive( HWND hwnd, USHORT usControl, PCALCULATION pCalc )
{
    ULONG  ulAction;
    double dOperand;

    if ( !pCalc ) return FALSE;
    if ( ! GetCurrentValue( hwnd, &dOperand )) return FALSE;

    switch ( usControl ) {
        case IDD_PLUS:  ulAction = OP_ADD_PLUS;  break;
        case IDD_MINUS: ulAction = OP_ADD_MINUS; break;
        default: ErrorPopup("Undefined operation");
            return FALSE;
    }

    // If there's a pending exponentiation operation, finish it
    if ( pCalc->flExpOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flExpOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dEbase );
        dOperand = pCalc->dEbase;
        pCalc->dEbase = 0;
        pCalc->flExpOp = 0;
    }

    // If there's a pending multiplicative operation, finish it
    if ( pCalc->flMulOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flMulOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dFactor );
        dOperand = pCalc->dFactor;
        pCalc->dFactor = 0;
        pCalc->flMulOp = 0;
    }

    // If there's a pending additive operation, finish it
    if ( pCalc->flAddOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flAddOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dCurrent );
    }
    else
        pCalc->dCurrent = dOperand;

    pCalc->flAddOp = ulAction;
    pCalc->fNeedOperand = TRUE;

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL OperationMultiplicative( HWND hwnd, USHORT usControl, PCALCULATION pCalc )
{
    ULONG  ulAction;
    double dOperand;

    if ( !pCalc ) return FALSE;
    if ( ! GetCurrentValue( hwnd, &dOperand )) return FALSE;

    switch ( usControl ) {
        case IDD_MULTIPLY:  ulAction = OP_MUL_MULTIPLY; break;
        case IDD_DIVIDE:    ulAction = OP_MUL_DIVIDE;   break;
        case IDD_MODULO:    ulAction = OP_MUL_MODULO;   break;
//        case IDD_POWER:     ulAction = OP_EXP_POWER;    break;
//        case IDD_ROOT:      ulAction = OP_EXP_NROOT;    break;
        case IDD_BITOR:     ulAction = OP_BIT_OR;       break;
        case IDD_BITXOR:    ulAction = OP_BIT_XOR;      break;
        case IDD_BITAND:    ulAction = OP_BIT_AND;      break;
        case IDD_BITLEFT:   ulAction = OP_BIT_LEFT;     break;
        case IDD_BITRIGHT:  ulAction = OP_BIT_RIGHT;    break;
        default: ErrorPopup("Undefined operation");
            return FALSE;
    }

    // If there's a pending exponentiation operation, finish it
    if ( pCalc->flExpOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flExpOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dEbase );
        dOperand = pCalc->dEbase;
        pCalc->dEbase = 0;
        pCalc->flExpOp = 0;
    }

    // If there's a previous multiplicative operation pending, finish it
    if ( pCalc->flMulOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flMulOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dFactor );
    }
    else
        pCalc->dFactor = dOperand;

    pCalc->flMulOp = ulAction;
    pCalc->fNeedOperand = TRUE;

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL OperationExponential( HWND hwnd, USHORT usControl, PCALCULATION pCalc )
{
    ULONG  ulAction;
    double dOperand;

    if ( !pCalc ) return FALSE;
    if ( ! GetCurrentValue( hwnd, &dOperand )) return FALSE;

    switch ( usControl ) {
        case IDD_POWER:     ulAction = OP_EXP_POWER;    break;
        case IDD_ROOT:      ulAction = OP_EXP_NROOT;    break;
        default: ErrorPopup("Undefined operation");
            return FALSE;
    }

    // If there's a previous exponentiation operation pending, finish it
    if ( pCalc->flExpOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flExpOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        SetCurrentValue( hwnd, pCalc->dEbase );
    }
    else
        pCalc->dEbase = dOperand;

    pCalc->flExpOp = ulAction;
    pCalc->fNeedOperand = TRUE;

    return TRUE;
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
BOOL OperationFinish( HWND hwnd, PCALCULATION pCalc )
{
    double dOperand;

    if ( !pCalc ) return FALSE;
    if ( ! GetCurrentValue( hwnd, &dOperand )) return FALSE;
    if ( pCalc->flExpOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flExpOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        dOperand = pCalc->dEbase;
        pCalc->dEbase = 0;
        pCalc->flExpOp = 0;
    }
    if ( pCalc->flMulOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flMulOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        dOperand = pCalc->dFactor;
        pCalc->dFactor = 0;
        pCalc->flMulOp = 0;
    }
    if ( pCalc->flAddOp ) {
        if ( ! DoCalculation( dOperand, pCalc->flAddOp, pCalc )) {
            ClearAll( hwnd );
            WinSetDlgItemText( hwnd, IDD_ENTRY, "####");
            return FALSE;
        }
        pCalc->flAddOp = 0;
    }
    else
        pCalc->dCurrent = dOperand;

    SetCurrentValue( hwnd, pCalc->dCurrent );
    pCalc->dCurrent = 0;
    pCalc->fNeedOperand = TRUE;

    // TODO - pop calculation off pGlobal->pOperations stack

    return TRUE;
}

