#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define MAX_LOADSTRING 100

#define COLLECTIONS_USE_LIMIT 10
#define	APP_BASE_HIVE HKEY_CURRENT_USER
#define	REGKEY_CE "SOFTWARE\\Car Explorer"
#define	REGKEY_CE_FAVORITES "SOFTWARE\\Car Explorer\\Favorites"
#define MAX_REG_STRING  200 // title plus identifying description for a car
#define TB_BUTTONS_COUNT 7
#define SPLITTER_WIDTH 5
#define DEFAULT_UNITS 0 // US (in/lbs/gallons)

#define WV_APPWINDOW_WIDTH_MIN 650
#define WV_HEADER_HEIGHT 60
#define WV_SEPARATOR_HEIGHT 20
#define WV_CB_TOPOFFSET_MIN 360 
#define WV_ANIMUI_LEFTOFFSET_MIN 280
#define RBP_CLOSE_BUTTON_TIMEOUT 5000 // milliseconds

// TODO: make swith for Vista (Documents).
#define CAR_FILES_FOLDER_XP "My Documents\\My Cars"
#define CAR_FILES_FOLDER_VISTA "Cars" //"Documents\\My Cars"
#define CAR_FILES_FOLDER_7 "Cars" //"Documents\\My Cars"

// Globals shared between files.
extern TCHAR szMBTitle[MAX_LOADSTRING];
extern TCHAR g_szCarFilesFolder[MAX_PATH];
extern BOOL g_bShowStartupWelcomeView; 
extern HWND g_hwndWelcome; // shared with setting dialog
extern BOOL g_bShowToolBar;
extern BOOL g_bShowStatusBar;
extern int g_nCarFilesLayout;
extern int g_iSortParam; // sort column and sort direction
extern HWND g_hWnd;
extern HWND hwndMainLV;
extern HWND g_hwndLV;
extern HWND hwndBottomLV;
extern short g_iUnits;
extern int g_nGroup;
extern BOOL g_bEnableDebug;

// Shared with LIS dialog.
BOOL IsMSSFile(LPCTSTR pszFFN);
void AddCarFile(LPCTSTR pszTitle, LPCTSTR pszFFN);
BOOL DecryptString(LPTSTR pszDecryptedPassword, LPCTSTR pszPassword);
BOOL EncryptString(LPTSTR pszEncryptedPassword, LPCTSTR pszPassword); // not shared, kept bundled with Decrypt
// Shared with LoadXMLData function of main file.
void ConvertXMLData(short iSelectedUnits); 
void ConvertUnits(HWND hwndLV, short iSelectedUnits);
