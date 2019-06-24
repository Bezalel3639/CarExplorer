#pragma once

#include "LoadImageSets.h"
#include "Registry.h"
#include "CE_AnimUI/Interface.h" // welcome page
#include "CE_AnimUI_Resource.h" // welcome page
#include "RegisterProduct.h"
#include <stdio.h>    // sprintf
#include "resource.h"
#include <atlbase.h> // IPicture
#include "shlobj.h"   // SHGetDesktopFolder
#include "shellapi.h"
#include "shlwapi.h"  // SHCopyKey
#include "commctrl.h" // ImageList_Create 
#include "commdlg.h"  // file/open dialog [comdlg32.lib]
#include "wininet.h"  // wininet
//#define MAX_LOADSTRING 100
#define THUMWIDTH  75
#define THUMHEIGHT 75
#include <string>
#include <vector>

using namespace std ; //for vector declaration

#define IMAGE_COUNT 50 //max array size 
#define	COMPONENT_BASE_HIVE HKEY_CURRENT_USER
#define	REGKEY_MSSFILEVIEWER "SOFTWARE\\MSB\\MSSFileViewer"
#define UNCATEGORIZED_FOLDER_NAME "Miscellaneous"

// TOIDO: figure why these declations from commctrl.h are not found in 
// commctrl.h, which is included (!).
#define BCM_FIRST		0x1600
#define BCM_SETIMAGELIST	(BCM_FIRST + 0x0002)

#define BUTTON_IMAGELIST_ALIGN_LEFT     0
#define BUTTON_IMAGELIST_ALIGN_RIGHT    1
#define BUTTON_IMAGELIST_ALIGN_TOP      2
#define BUTTON_IMAGELIST_ALIGN_BOTTOM   3
#define BUTTON_IMAGELIST_ALIGN_CENTER   4       // Doesn't draw text

typedef struct
{
    HIMAGELIST  himl;   // Index: Normal, hot pushed, disabled. If count is less than 4, we use index 1
    RECT        margin; // Margin around icon.
    UINT        uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;
// -- end of TODO

// Global variables.
int g_nEvaluationLimit = 10; // evaluation
TCHAR g_szAD_Version[50];
BOOL g_bUseKagiPaymentProcessing = FALSE;
BOOL g_bRegisteredCopy = FALSE;
int g_nCollectionsCount;
int g_nOSVersion; // summary OS version
DWORD g_dwMajorVersion, g_dwMinorVersion; // detailed OS version

// Application variables.
HINSTANCE hInst;
TCHAR g_szCarFilesFolder[MAX_PATH];
BOOL g_bShowTV = TRUE; // used to indicate whether to use TV upon DC of CAR file
BOOL g_bCarDataLoaded = FALSE; // used to prevent redundant data load with WM_ACTIVATE
BOOL g_bFavoritesLoaded = FALSE; // used to handle click on favorites folder
BOOL g_bCollectionFileDBClicked = FALSE; // indicates when TRUE that CE is launched with DC
short iSelectedImage = -1;
int g_iLVDroppedItem = -1; // no item is dropped yet
BOOL g_bPersistCarFilesFolder = FALSE;
BOOL g_bPersistFavorites = TRUE; // TODO: cripple to FALSE in release
BOOL g_bExactCarTypeMatch = FALSE; // "Wagon" can also mean "Estate" (reserved)
BOOL g_bExactFuelTypeMatch = FALSE; // "Gasoline" can also mean "Gas/SEFI", "Petrol" (reserved)
HTREEITEM g_htviFavorites;
short g_iPicturesCategory;
BOOL g_bDADActive = FALSE; // drag-and-drop is inactive
BOOL LoadPictureZoomWindow(short iPicture);
BOOL LoadXMLData(short iPicture);
short GetPicturesCategory();
void OnFileOpen();
TCHAR g_szFullFileName[MAX_PATH];
short g_iUnits; 
int g_nTimerID;
int g_nCloseButton_TimerID;
int g_nTimerCallsCount; // debug only
TCHAR g_szCarTitle[100]; // title for "Windows Picture and Fax Viewer"
CRegisterProduct m_RP;
int g_nRBD_CloseButtonMeter;
string g_sOldLabel;

HWND g_hWnd;
HWND g_hwndReBar;
HWND g_hwndTB;
HWND hwndBrowser;
HWND hwndMainLV;
HWND hwndRightHost;
HWND hwndPictureView;
HWND hwndBottomLV;
HWND hwndBottomChild1; // TODO: rename or remove
HWND hwndBottomChild2; // TODO: rename or remove
HWND g_hwndWelcome;
HWND g_hwndButton;
HWND g_hwndImageWindow;
//BOOL g_LeftMouse = FALSE;
WNDPROC  g_wpButton_WndProc;
HWND g_hwndWelcome_CBYear;
HWND g_hwndWelcome_CBType;
HWND g_hwndWelcome_CBPriceRange;
HWND g_hwndEdit_MinPrice, g_hwndEdit_MaxPrice;
HWND g_hwndStatic_MinPrice, g_hwndStatic_MaxPrice;
HWND g_hwndWelcome_CBFuel;
HWND g_hwndStatus;
TCHAR szTitle[MAX_LOADSTRING];			// the title bar text
TCHAR szMBTitle[MAX_LOADSTRING];	    // application title for message boxes
TCHAR szWindowClass[MAX_LOADSTRING];    // the main window class name
TCHAR g_szDestination[MAX_PATH];
HIMAGELIST hilDrag;

int	nSplitterBorder = 2;
int	nSplitterPos = 200;
static BOOL fDragMode = FALSE;
static int  oldy = -4;

int	nSplitterPos2 = 400;
static int  oldy2 = -4;
static BOOL fDragMode2 = FALSE;

int	nSplitterPos3 = 370; 
static int  oldy3 = -4;
static BOOL fDragMode3 = FALSE;
//static BOOL fMoved = FALSE;
//static BOOL fMoved2 = FALSE;

int	nBottomSplitterPos = 500;
static int  oldyBottom = -4;
static BOOL fBottomMoved = FALSE;
static BOOL fBottomDragMode = FALSE;

BOOL g_bEnableDebug = TRUE;
BOOL g_bUpdateRegistry = FALSE; // save to registry only if changes occurred
int g_nGroup = 3; // group by country
// It should be 2 parameters (not one) g_nRightPaneViewType and g_nCarFilesLayout 
// since g_nCarFilesLayout (persisted value) indicates what view to make as a 
// result of click on single CAR file.In any case welcome (seach) page use is an 
// additional option.
int  g_nRightPaneViewType = 1; // welcome (0), all car files views (1), single picture from favorites (2)
int g_nCarFilesLayout = 1; // report in list view (0), pictures in list view (1)
HCURSOR hCursorNS;
HCURSOR hCursorWE; 
WNDPROC g_wpOldDialogProc;
WNDPROC g_wpOldRightHost_DialogProc;
WNDPROC g_wpOldRBH_DialogProc;

HWND hwndTopArea;
HWND g_hwndLV;
HWND hwndRightBottomHost;
HWND g_hwndPicturesView;
HWND g_hwndRightBottom_PictureView;
HWND g_hwndRightBottom_ReportView;
HWND hwndComboBox;
int g_nSortColumn;
int g_iSortColumn_Prev;
int g_iSortParam; // sort column and sort direction
BOOL g_bSortAscending;

HWND g_hwndCheckBox;
int g_nWV_HOffset, g_nWV_VOffset;
BOOL g_bWV_FirstPaint;
COLORREF g_clrLV_InterlacedColor; // all LV
COLORREF g_clrWV_OuterBackground;
COLORREF g_clrWV_InnerBackground;
int g_nWV_InnerAreaHeight;
BOOL g_bWV_NarrowWidth;
int g_nWV_AnumUIRightOffset;
int g_nWV_CBPosLeft, g_nWV_CBBottomOffset;
BOOL g_bShowStartupWelcomeView = TRUE;
BOOL g_bShowToolBar = TRUE;
BOOL g_bShowStatusBar = TRUE;

class CPicture 
{
public:
    void Load(LPCTSTR pszPathName);
    BOOL Load(HINSTANCE hInst, LPCTSTR pszResType, UINT nIDRes);
public:
    CComQIPtr<IPicture> m_spIPicture;		
    HRESULT m_hr;								 
};

CPicture m_Picture[IMAGE_COUNT];
CPicture m_SinglePicture;
CPicture m_AD_SinglePicture; // about dialog picture
TCHAR* m_Title[IMAGE_COUNT];

int GetOSVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion);
//int GetOSVersion();
//BOOL IsMSSFile(LPCTSTR pszFFN /* in */);

//void ConvertXMLData(short iSelectedUnits);
//void LoadRegistryData();
//void SaveRegistryData();
BOOL IsMSSFile(LPCTSTR pszFFN /* in */);

// how much to scale a design that assumes 96-DPI pixels
double scaleX;
double scaleY;
#define SCALEX(argX) ((int) ((argX) * scaleX))
#define SCALEY(argY) ((int) ((argY) * scaleY))

void InitScaling() {
   HDC screen = GetDC(0);
   scaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0;
   scaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0;
   ReleaseDC(0, screen);
}


//vector<string> vecImageTitles; 

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Welcome_WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    BottomParentWindow_WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL WV_DrawHeaderPictureAndInnerBackground(HWND hwnd, HDC hdc);
void WV_OnPaint(HWND hWnd, LPPAINTSTRUCT pps);
LRESULT CALLBACK AddFolder_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                 LPARAM lParam);
LRESULT CALLBACK SetFolder_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                 LPARAM lParam);
LRESULT CALLBACK RegisterProduct_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                 LPARAM lParam);
LRESULT CALLBACK RegisterBuyProduct_WndProc(HWND hDlg, UINT message, 
                                                  WPARAM wParam, LPARAM lParam);
VOID CALLBACK CloseButton_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, 
                                                                  DWORD dwTime);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Extract_WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL LoadPicturesAndStrings(HWND hwndLV);
int DoBrowse(HWND hDlg, LPTSTR pszDestination /* out */);
//void SaveToImageFile(int iImage);
void SaveToImageFile(LPCTSTR pzsFFN, int iImage);
void App_OnSaveSettings();
void App_LoadRegistrySettings();
int Utility_GetItemLevel(LPCTSTR pszData); 
int Utility_GetItemLevelFromCSV(LPCTSTR pszLine);
string  Utility_GetDateTime(); 
void App_OnToolbarSearch(HWND hWnd);
int Utility_ConvertPriceToNumber(LPCTSTR pszPrice, int nSortOrder);
int Utility_ConvertWeightToNumber(LPCTSTR pszWidth, int nSortOrder);

LRESULT Splitter_OnLButtonDown (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter_OnLButtonUp   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter_OnMouseMove   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//--
LRESULT Splitter2_OnLButtonDown (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter2_OnLButtonUp   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter2_OnMouseMove   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void SizeWindowContents2(int nWidth, int nHeight);

LRESULT Splitter3_OnLButtonDown (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter3_OnLButtonUp   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT Splitter3_OnMouseMove   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void SizeWindowContents3(int nWidth, int nHeight);
void DrawXorBar(HDC hdc, int x1, int y1, int width, int height);
//--

LRESULT BottomSplitter_OnLButtonDown (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT BottomSplitter_OnLButtonUp   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT BottomSplitter_OnMouseMove   (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RightHostingWindow_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BottomLeftWindow_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void SizeWindowContents(int nWidth, int nHeight);
void CreateImageList(HWND hwndLV, short iImageCount, vector<string> vecImageTitles);
HIMAGELIST hImageList;
HBITMAP LoadPicture(short iPicture);

BOOL OnPaintHandler(HWND hwnd);

//--
void InitImageLists(HWND hwndTV); 
BOOL PopulateTreeView(HWND hwndTV);
void PopulateTreeView_Favorites(HWND hwndTV);
BOOL IsFolderEnumerated(string sParentFolder, string sFolder);
HTREEITEM GetFavoritesForder(string sParentFolder, string sFolder);
BOOL IsCarSuffix(LPCTSTR pszSuffix);
BOOL GetImageSetTitle(TCHAR* pszLocation /* in */, TCHAR* pszTitle /* out */);
//void App_SetFont(HWND hwnd);
void App_SetFont(HWND hwnd, BOOL bBold = FALSE);
BOOL OnNotify_TVItemClick(HWND hWnd, LPARAM lParam);
HTREEITEM GetTVItemFromFFN(LPCTSTR pszFFN);
LRESULT APIENTRY DialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY RightHostDialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY RBHDialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);

void SelectFolder(HWND hDlg);
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData);
//BOOL EnumerateCarFiles();
void App_Debug(LPCTSTR pszMessage);

class CCarList // based on MSS as collection
{
public:
    string sTitle; 
    //string sFileName; // not used
    string sFFN;
public:
    string sYear;
    string sMake;
    string sCountry;
   
    HTREEITEM hItem; // used for text-to-item search
};

vector<CCarList> m_vecCarFilesMap;

struct FavoritesFolders
{
    BOOL bStandard;
    HTREEITEM hTVItem;
    string sParentFolder; // added
    string sFolder;  
};

vector <FavoritesFolders> vecFavoritesFolders;

class CCarFavorites // single images from different collections
{
public:
    int nItemLevel; // 0 (right under Favorites), 1 (under 1st level) etc
    string sGrandparentFolder; // used for 2nd level only when deleting
    string sParentFolder; // Favorites (ROOT), or Folder Name
    string sCarTitle; // can differ from title of collection (not the same as sPictureTitle since sPictureTitle can be empty)
    string sTitle;
    //string sModel;
    //string sSubModel;
    //string sType;
    int nCollectionImageID;
    int nGlobalImageID_PK; // primary key to reference CCarData
};

vector<CCarFavorites> m_vecCarFavoritesMap;

class CCarData // all data from all MSS (except images)
{
public:
    string sTitle;
    string sFileName;
    string sFFN;
public:
    string sPictureTitle; 
    // Main.
    string sYear;
    string sMake;
    string sModel;
    string sSubModel;
    string sType;
    string sPrice;
    // Engine.
    string sPower;
    string sFuel;
    // Sizes and weight.
    string sLength;
    string sWidth;
    string sHeight;
    string sClearance;
    string sWeight;
    // Misc.
    string sTankVolume;
    string sCountry;
    // Utility.
    int nImageID; // unique ID within image set used for getting image from main LV
public:
    int nIndex; // unique ID used for ordering
};

vector<CCarData> m_vecCarData;

BOOL EnumerateCarFiles();
void EnumerateCarFiles(LPCTSTR pszSearchString, int& nCount /* out */);
BOOL GetCarXMLData(LPCTSTR pszFFN, UINT nID, LPTSTR lszBuffer);
BOOL PopulateTreeView(HWND hwndTV, int nGroup);
HTREEITEM IsInserted(HWND hwndTV, LPCTSTR pszText);
BOOL LoadAllCarData(LPCTSTR pszCarDirectory);
BOOL LoadCarData_SingleFile();
HANDLE g_hLoadDataCompleteEvent;
HANDLE g_hTVAvailableEvent;
HANDLE g_hLoadFavoritesCompleteEvent;
void MainWindow_ActivateHandler();
static UINT __stdcall LoadAllCarData_ThreadProc(LPVOID pParam);
static UINT __stdcall LoadFavorites_ThreadProc(LPVOID pParam);
void LoadAllCarData(LPCTSTR pszSearchString, int& nGlobalCount /* in, out */);
void InitListViewColumns(HWND hwndLV);
void InitListViewItems(HWND hwndLV, LPCTSTR pszFilter = NULL, 
                                                         BOOL bCarFile = FALSE);
void BrowseFavorites(HWND hwndLV, int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                         LPCTSTR pszFolderName);
void ReloadData();

// Separate function to populate LV with search option invoked.
void InitListViewItems(HWND hwndLV, int nYearSelection, int nCarType, 
                                   int nMinPrice, int nMaxPrice, int nFuelType);
void ListView_InsertCarItem(HWND hwndLV, LVITEM* pLVI, int i /* car ID */);
void ListView_InsertFavoritesItem(HWND hwndLV, LVITEM* pLVI, 
                                               int nFavorite /* favorite ID */);
void GetImageSetFFN(LPCTSTR pszTitle /* in */, LPTSTR pszFFN/* out */);
int GetCollectionImageIDFromCarTitle(LPCTSTR pszCarTitle /* in */, 
                                                       LPTSTR pszFFN /* out */);
LRESULT ListView_NotifyHandler(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                 LPARAM lParam);
LRESULT BottomLV_NotifyHandler(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                 LPARAM lParam);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT LV_Notify_OnDoubleClick(HWND hwndLV);
void LV_SortItems(HWND hwndLV, int nSortColumn);
BOOL IsFavoritesSelected();
//HWND g_hwndCurLV; // current main LV
int CALLBACK TV_CompareProc(LPARAM lParam1, LPARAM lParam2, 
                          LPARAM lParamSort /* in, application-defined value*/);
int CALLBACK ListView_CompareProc(LPARAM lParam1, LPARAM lParam2, 
                         LPARAM lSortColumn /* in, application-defined value*/);
int ListView_GetCompareReturnValue(float fValue1, float fValue2, 
                                                               int iSortColumn);
int ListView_GetCompareReturnValue(int iValue1, int iValue2, int iSortColumn);
//void GetImageSetFFNFromGlobalImageID(int nImageID /* in */, 
//									                   LPTSTR pszFFN /* out */);
void GetImageSetFFNFromGlobalImageID(int nGlobalImageID /* in */, 
                               LPTSTR pszFFN /* out */, int& nImageID /* out */);
int GetCollectionImageIDFromGlobalImageID(int nGlobalImageID);
int GetGlobalCarID(LPCTSTR pszTitle /* in */, int nCollectionImageID /* in */);
const int g_nTBHeight = 44; // manually adjusted, fixed
int g_nTBHeightUI = 44; // equals ehither to zero or g_nTBHeight 
const int g_nSBHeight = 23; // manual adjustment, fixed  // 68; 
int g_nSBHeightUI = 23; // equals ehither to zero or g_nSBHeight 
void App_CreateToolBar();
void App_CreateStatusBar();
BOOL AddDroppedItem(HWND hwndLV, LPPOINT ppt); // utility function
BOOL Utility_IsFavoritesFolder(TVITEM* ptvi);
BOOL Utility_CarItem(TVITEM* ptvi);
string  Utility_GetTVParentFolderName(TVITEM tvi);

// TODO: UpdateLayout functions must be replaced with natural Win32 solution
// in nest releases.
void UpdateLayout();
void UpdateLayout2(BOOL bAdjustNormalMode = FALSE /* when desktop-expanded */,
                                               BOOL bAdjustDesktopMode = FALSE);
void TV_OnContextMenu(HWND hWnd);
int Utility_IsVectorCarItemFound(string sFolder);
void LV_OnContextMenu(HWND hwndLV);
void AddItemToFavorites(HWND hwndLV);
void WV_CB_OnChangeSelection();
//--

HWND g_hWndSplashScreen;
void  RunSplashScreen();
void OnPaint_Handler(HWND hWnd);
LRESULT APIENTRY SplashScreen_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
//BOOL CenterWindow2(HWND hWnd, BOOL bAppRelative = TRUE);
BOOL CenterWindow2(HWND hWnd, HWND hBaseWnd); 
VOID CALLBACK SplashScreen_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, 
                                                                  DWORD dwTime);
void InitSingleCARFileViewer(HWND hWnd);
void AD_OnPaint(HWND hWnd, LPPAINTSTRUCT pps);
BOOL SB_ShowCollectionInfo(LPCTSTR pszFFN);
void SB_ShowCollectionsSummary(LPCTSTR pszFolderName, HTREEITEM hItem, HWND hwndLV);
int TV_GetChildItemCount(HTREEITEM hitem);
void TB_SetSingleCARFileIcons();
BOOL WV_ValidatePrices(LPCTSTR pszMinPrice, LPCTSTR pszMaxPrice, LPTSTR pszErrorMsg);
void OnExportFavorites(int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                         LPCTSTR pszFolderName);
void OnExportListView(HWND hwndLV);
BOOL OnImportFavorites();
void SaveAsCSV(LPCTSTR pszFFN, int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                         LPCTSTR pszFolderName);
void SaveListViewAsCSV(LPCTSTR pszFFN, HWND hwndLV);
void TV_ClearAllFavorites();
//void TV_DeleteFavoritesFolder(TVITEM* ptvi, string sFolder);
void TV_DeleteFavoritesFolder(TVITEM* ptvi);
string AddQuotes(LPCTSTR pszText);
int MakeCompareValue(int nPart1, int nPart2, int nPart3);