#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSRESOURCES
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <uconv.h>
//#include <unidef.h>
#include "ids.h"

#ifndef WS_TOPMOST
#define WS_TOPMOST  0x00200000L
#endif

// ----------------------------------------------------------------------------
// CONSTANTS

#define SZ_VERSION              "0.5.0"
#define SZ_COPYRT               "2018-2019"

#define SZ_ABOUT_VERSION        "version %s"
#define SZ_ABOUT_COPYRT         "(C) %s Alexander Taylor"

#define HELP_FILE               "ucalc.hlp"
#define INI_FILE                "ucalc.ini"

// min. size of the program window
#define US_MIN_WIDTH            333
#define US_MIN_HEIGHT           195

#define MAX_DIGITS              16      // maximum number of digits in entryfield

// Maximum string length...
#define SZRES_MAXZ              256                 // ...of a generic string resource
#define SZENTRY_MAX             (MAX_DIGITS + 1)    // ...of the entryfield contents


// Profile (INI) file entries
#define PRF_APP_LOOK            "Appearance"
#define PRF_KEY_X               "X"
#define PRF_KEY_Y               "Y"
#define PRF_KEY_CX              "CX"
#define PRF_KEY_CY              "CY"
#define PRF_KEY_STYLE           "Style"
#define PRF_APP_DATA            "Data"
#define PRF_KEY_MEMORY          "Memory"

// Colours
#define CLR_NUMBERS_FG          0x00000000L
#define CLR_NUMBERS_BG          0x00C0C0C0L
#define CLR_BITOPS_FG           0x00000000L
#define CLR_BITOPS_BG           0x00C0C0B0L
#define CLR_COMMANDS_FG         0x00FFFFFFL
#define CLR_COMMANDS_BG         0x00207070L
#define CLR_MATHOPS_FG          0x00000000L
#define CLR_MATHOPS_BG          0x00A0A0A0L
#define CLR_ALGEBRA_BG          0x00A0B0C0L
#define CLR_ALGEBRA_FG          0x00000000L
#define CLR_CLEAR_BG            0x00800000L
#define CLR_CLEAR_FG            0x00FFFFFFL


// Fonts
#define SZ_FONT_BUTTON_LG       "12.Helv"
#define SZ_FONT_SYMBOL_LG       "13.Symbol Set"
#define SZ_FONT_UNICODE_LG      "12.Arial"
#define SZ_FONT_ENTRY_LG        "12.MMPMDigital"
#define SZ_FONT_BUTTON_SM       "9.WarpSans"
#define SZ_FONT_SYMBOL_SM       "10.Symbol Set"
#define SZ_FONT_UNICODE_SM      "10.Arial"
#define SZ_FONT_ENTRY_SM        "10.MMPMDigital"


// Custom messages
#define UM_SETPS                WM_USER - 1

// UI flags
#define FL_SIZE_LARGE   0x1
#define FL_CLR_MONO     0x10


// Operations

#define OP_ADD_PLUS         0x1
#define OP_ADD_MINUS        0x2
#define OP_MUL_MULTIPLY     0x10
#define OP_MUL_DIVIDE       0x20
#define OP_MUL_MODULO       0x30
#define OP_BIT_OR           0x40
#define OP_BIT_XOR          0x50
#define OP_BIT_AND          0x60
#define OP_BIT_LEFT         0x70
#define OP_BIT_RIGHT        0x80
#define OP_EXP_POWER        0x100
#define OP_EXP_NROOT        0x200
#define OP_UNI_SQROOT       0x1000
#define OP_UNI_RECIPROCAL   0x2000
#define OP_UNI_LOG          0x3000
#define OP_UNI_LN           0x4000

#ifndef M_PI
#define M_PI                3.1415926535
#endif


// ----------------------------------------------------------------------------
// MACROS

// Handy message box for errors and debug messages
#define ErrorPopup( text ) \
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text, "Error", 0, MB_OK | MB_ERROR )

// Convert a pair of bytes to a UniChar
#define BYTES2UNICHAR( bFirst, bSecond ) \
    (( bFirst << 8 ) | bSecond )


// ----------------------------------------------------------------------------
// TYPEDEFS

// Basic logic adapted from http://doc.qt.io/qt-5/qtwidgets-widgets-calculator-example.html
typedef struct _Calculation {
    double dCurrent;                 // current accumulated value
    double dFactor;                  // pending multiplication lvalue
    double dEbase;                   // pending exponentiation base (lvalue)
    ULONG  flAddOp;                  // currently pending operator (additive)
    ULONG  flMulOp;                  // currently pending operator (multiplicative)
    ULONG  flExpOp;                  // currently pending operator (exponentiation)
    BOOL   fNeedOperand;             // is an operand needed to complete the operation?
    struct _Calculation *pNext;
} CALCULATION, *PCALCULATION;


typedef struct _Global_Data {
    HAB    hab;                      // anchor-block handle
    HMQ    hmq;                      // main message queue
    HINI   hIni;                     // program INI file
    CHAR   szBtnFont[ FACESIZE+5 ];  // current button font
    CHAR   szNumFont[ FACESIZE+5 ];  // current number panel font
    CHAR   szSymFont[ FACESIZE+5 ];  // font used for certain symbol buttons
    ULONG  flStyle;                  // current style flags
    BOOL   fHexMode;                 // are we in hex mode? (not implemented)
#ifdef NO_BUTTON_ICONS
    BOOL   fUniFont;                 // use Unicode TT font?
#endif
    double dMemory;                  // current value saved in memory
    PCALCULATION pOperations;        // operations stack

} CALGLOBAL, *PCALGLOBAL;


// ----------------------------------------------------------------------------
// GLOBALS

PFNWP pfnRecProc;                   // Default SS_FGNDRECT window procedure
PFNWP pfnButtonProc;                // Default WC_BUTTON window procedure


// ----------------------------------------------------------------------------
// FUNCTIONS

MRESULT EXPENTRY MainWndProc( HWND, ULONG, MPARAM, MPARAM );
void             WindowSetup( HWND hwnd );
void             CentreWindow( HWND hwnd );
void             LocateProfile( PSZ pszProfile );
void             LoadIniData( PVOID pszData, HINI hIni, PSZ pszApp, PSZ pszKey );
void             SaveSettings( HWND hwnd );
MRESULT EXPENTRY PanelProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY AboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             SetButtonIcon( HWND hwndBtn, USHORT usResID );
void             SetColourScheme( HWND hwnd, PCALGLOBAL pGlobal );
void             SetTextSize( HWND hwnd, BOOL fLarge );
void             SetTopmost( HWND hwnd );
void             DoCopy( HWND hwnd );
void             DoPaste( HWND hwnd );
void             AppendDigit( HWND hwnd, CHAR chNew );
void             DeleteDigit( HWND hwnd );
void             FlipSign( HWND hwnd );
void             SetCurrentValue( HWND hwnd, double dValue );
BOOL             GetCurrentValue( HWND hwnd, double *pdValue );
void             UpdateNotation( HWND hwnd );
void             ClearAll( HWND hwnd );
void             MemoryGet( HWND hwnd, PCALGLOBAL pGlobal );
void             MemoryClear( PCALGLOBAL pGlobal );
void             MemorySet( HWND hwnd, PCALGLOBAL pGlobal );
BOOL             DoCalculation( double dRValue, ULONG ulOperator, PCALCULATION pCalc );
BOOL             OperationAdditive( HWND hwnd, USHORT usControl, PCALCULATION pCalc );
BOOL             OperationMultiplicative( HWND hwnd, USHORT usControl, PCALCULATION pCalc );
BOOL             OperationExponential( HWND hwnd, USHORT usControl, PCALCULATION pCalc );
BOOL             OperationUnitary( HWND hwnd, USHORT usControl, PCALCULATION pCalc );
BOOL             OperationFinish( HWND hwnd, PCALCULATION pCalc );

