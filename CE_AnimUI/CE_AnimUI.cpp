#include "stdafx.h"
#include "resource.h"
#include "Interface.h"
#include "UpdatesWindow.h"
#include "ControlsPositioning.h"
#include "commctrl.h"
#include "commdlg.h" // colors dialog
#include "shellapi.h" // ShellExecute

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

HFONT g_hfnt;
BOOL g_bAnyButtonMoving = FALSE;

// Canvas1 controls.
HWND g_hwndStatic11; 
HWND g_hwndStatic12; 
HWND g_hwndStatic13; 
HWND g_hwndStatic14;
HWND g_hwndStatic15;
HWND g_hwndStaticLink11;

// Canvas2 controls.
HWND g_hwndStatic21; 
HWND g_hwndStatic22; 
HWND g_hwndStatic23; 
HWND g_hwndStatic24;

// Canvas3 controls.
HWND g_hwndStatic31; 
HWND g_hwndStatic32; 
HWND g_hwndStatic33; 
HWND g_hwndStatic34;

// Canvas5 controls.
HWND g_hwndStatic51; 
HWND g_hwndStaticLink51;
HWND g_hwndStaticLink52;
HWND g_hwndStaticLink53;

WNDPROC  g_wpWndProc;
WNDPROC  g_wpWndProc2;
WNDPROC  g_wpWndProc3;
WNDPROC  g_wpWndProc4;
WNDPROC  g_wpWndProc5;

WNDPROC g_wpOldStaticLink11_WndProc;
LRESULT APIENTRY StaticLink11_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);

WNDPROC g_wpOldStaticLink51_WndProc;
WNDPROC g_wpOldStaticLink52_WndProc;
WNDPROC g_wpOldStaticLink53_WndProc;
LRESULT APIENTRY StaticLink51_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY StaticLink52_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY StaticLink53_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);

int iTimerCounter;

HBRUSH g_hbrCanvasBackgroungColor = CreateSolidBrush(g_clrCanvasBackground); // green, MSB RGB(100, 153, 0)
//HBRUSH g_hbrCanvasBackgroungColor = CreateSolidBrush(RGB(221, 105, 202)); // bruni
//HBRUSH g_hbrCanvasBackgroungColor = CreateSolidBrush(RGB(214, 223, 247)); // blue, IBP
                    //g_hbrCanvasBackgroungColor(HBRUSH)(COLOR_3DFACE + 1); // or CreateSolidBrush(RGB(...))
void App_Debug_FunctionCallTrace(LPCTSTR pszMessage);

int g_iButton1_TimerID;
int g_iButton2_TimerID;
int g_iButton3_TimerID;
int g_iButton4_TimerID;
int g_iButton5_TimerID;

HWND g_hwndButton1;
HWND g_hwndButton2;
HWND g_hwndButton3;
HWND g_hwndButton4;
HWND g_hwndButton5;

HWND g_hwndCanvas1;  
HWND g_hwndCanvas2;      
HWND g_hwndCanvas3; 
HWND g_hwndCanvas4; 
HWND g_hwndCanvas5;   

BOOL g_bCanvas1Expanded = TRUE;
BOOL g_bCanvas2Expanded = FALSE;
BOOL g_bConvas3Expanded = FALSE;
BOOL g_bCanvas4Expanded = FALSE;
BOOL g_bCanvas5Expanded = FALSE;

HFONT CreateControlsFont();
void SetCanvasState();
BOOL IsCanvasExpanded(HWND hwnd /* canvas window */, int iCanvas);

void Button1_StartTimer();
void CALLBACK Button1_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime);
void Button2_StartTimer();
void CALLBACK Button2_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

void Button3_StartTimer();
void CALLBACK Button3_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime);
void Button4_StartTimer();
void CALLBACK Button4_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime);
void Button5_StartTimer();
void CALLBACK Button5_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, 
                                                              LPVOID lpReserved)
{
    return TRUE;
}

void CreateButton1(HINSTANCE hInst, HWND hWnd);
void ShowCanvas1(HINSTANCE hInst, HWND hWnd);
void CreateButton2(HINSTANCE hInst, HWND hWnd);
void ShowCanvas2(HINSTANCE hInst, HWND hWnd);
void CreateButton3(HINSTANCE hInst, HWND hWnd);
void ShowConvas3(HINSTANCE hInst, HWND hWnd);
void CreateButton4(HINSTANCE hInst, HWND hWnd);
void ShowCanvas4(HINSTANCE hInst, HWND hWnd);
void CreateButton5(HINSTANCE hInst, HWND hWnd);
void ShowCanvas5(HINSTANCE hInst, HWND hWnd);

LRESULT APIENTRY Button1_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Button2_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Button3_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Button4_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Button5_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);

LRESULT APIENTRY Canvas1_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Canvas2_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Canvas3_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Canvas4_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);
LRESULT APIENTRY Canvas5_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam);

//--
HINSTANCE g_hInst;
int g_nImage = 0;
int g_nImage2 = 0;
void RegisterImageWindowClasses(HINSTANCE hInst, HWND hWnd);
int g_nButtonLastClicked = 0; // LL DNM
int GetButtonID(HWND hWnd); // TODO: LL redundant (was used with share "ImageWindow").
void SetButtonImage(HWND hWnd, int nImage);

HBITMAP g_hBitmapRight;
HBITMAP	g_hBitmapDown;
HBITMAP	g_hButton1Bitmap;
HBITMAP	g_hButton2Bitmap;
void LoadBitmaps();

//--
void SetFont(HWND hwnd, BOOL bBold = FALSE);
//--

void MoveControls_Canvas1(int nVTimer, BOOL bMoveUp);
void MoveControls_Canvas2(int nVTimer, BOOL bMoveUp);
void MoveControls_Canvas3(int nVTimer, BOOL bMoveUp);
void MoveControls_Canvas4(int nVTimer, BOOL bMoveUp);
BOOL g_bCanvas4Contracting = FALSE;

FUNCTION_PREFIX void DLL_InitCustomUI(HINSTANCE hInst, HWND hWnd)
{
    g_hInst = hInst; // CE added
    g_hfnt = CreateControlsFont();

    //TCHAR szBuf[100];
    //wsprintf(szBuf, "DLL_InitCustomUI, g_hInst: %d", g_hInst);
    //App_Debug_FunctionCallTrace(szBuf);

    // Register window classes for image windows (one class for one button).
    RegisterImageWindowClasses(hInst, hWnd);

    LoadBitmaps();

    // Create buttons and canvases.
    CreateButton1(hInst, hWnd);
    ShowCanvas1(hInst, hWnd);
    CreateButton2(hInst, hWnd);
    ShowCanvas2(hInst, hWnd);
    CreateButton3(hInst, hWnd);
    ShowConvas3(hInst, hWnd);
    CreateButton4(hInst, hWnd);
    ShowCanvas4(hInst, hWnd);
    CreateButton5(hInst, hWnd);
    ShowCanvas5(hInst, hWnd);

    SetButtonImage(g_hwndButton1, 1 /* down */);
    SetButtonImage(g_hwndButton2, 0 /* right */);
    SetButtonImage(g_hwndButton3, 0 /* right */);
    SetButtonImage(g_hwndButton4, 0 /* right */);
    SetButtonImage(g_hwndButton5, 0 /* right */);

    // Create thread that downloads CE_Updates.xml from server 
    // (http://www.carexplorer.org) using HTTP protocol.
    StartDownloadUpdatesInfoThread();
}

FUNCTION_PREFIX void DLL_DestroyCustomUI()
{
    // Destroy buttons.
    DestroyWindow(g_hwndButton1);
    DestroyWindow(g_hwndButton2);
    DestroyWindow(g_hwndButton3);
    DestroyWindow(g_hwndButton4);
    DestroyWindow(g_hwndButton5);

    // Destroy canvases.
    DestroyWindow(g_hwndCanvas1);  
    DestroyWindow(g_hwndCanvas2);      
    DestroyWindow(g_hwndCanvas3); 
    DestroyWindow(g_hwndCanvas4); 
    DestroyWindow(g_hwndCanvas5);
}

void CreateButton1(HINSTANCE hInst, HWND hWnd)
{
    g_hwndButton1 = CreateWindowEx(g_dwHeaderExStyle, "BUTTON", "", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, iOffsetLeft, iOffsetTop, 
        g_iButtonWidth, g_iButtonHeight, hWnd, (HMENU)IDC_BUTTON1, 
                                                                      hInst, 0);
    g_wpWndProc = (WNDPROC)SetWindowLong(g_hwndButton1, GWL_WNDPROC, 
                                                (DWORD)Button1_SubclassedFunc);
    // Setup font for button control.
    SendMessage(g_hwndButton1, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndButton1, WM_SETTEXT, 0 /* not used */, 
                                                          (LPARAM)"Start Here          ");
}

void CreateButton2(HINSTANCE hInst, HWND hWnd)
{
    g_hwndButton2 = CreateWindowEx(g_dwHeaderExStyle, "BUTTON", "", 
    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, iOffsetLeft, iOffsetTop + 
        g_iWindowCanvasHeight + g_iButtonHeight, g_iButtonWidth, 
                       g_iButtonHeight, hWnd, (HMENU)IDC_BUTTON2, hInst, 0);
    g_wpWndProc2 = (WNDPROC)SetWindowLong(g_hwndButton2, GWL_WNDPROC, 
                                                    (DWORD)Button2_SubclassedFunc);
    // Setup font for button control.
    SendMessage(g_hwndButton2, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndButton2, WM_SETTEXT, 0 /* not used */, 
                                                 (LPARAM)"Favorites          ");
}

void CreateButton3(HINSTANCE hInst, HWND hWnd)
{
    g_hwndButton3 = CreateWindowEx(g_dwHeaderExStyle, "BUTTON", "", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, iOffsetLeft, iOffsetTop + 
        g_iWindowCanvasHeight + 2*g_iButtonHeight, g_iButtonWidth, 
                   g_iButtonHeight, hWnd, (HMENU)IDC_BUTTON3, hInst, 0);
    g_wpWndProc3 = (WNDPROC)SetWindowLong( g_hwndButton3, GWL_WNDPROC, 
                                                (DWORD)Button3_SubclassedFunc);
    // Setup font for button control.
    SendMessage(g_hwndButton3, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage( g_hwndButton3, WM_SETTEXT, 0 /* not used */, 
                                    (LPARAM)"Integration with Windows        ");
}

void CreateButton4(HINSTANCE hInst, HWND hWnd)
{
    g_hwndButton4 = CreateWindowEx(g_dwHeaderExStyle, "BUTTON", "", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, iOffsetLeft, iOffsetTop + 
        g_iWindowCanvasHeight + 3*g_iButtonHeight, g_iButtonWidth, 
                  g_iButtonHeight, hWnd, (HMENU)IDC_BUTTON4, hInst, 0);
    g_wpWndProc4 = (WNDPROC)SetWindowLong(g_hwndButton4, GWL_WNDPROC, 
                                               (DWORD)Button4_SubclassedFunc);
    // Setup font for button control.
    SendMessage(g_hwndButton4, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndButton4, WM_SETTEXT, 0 /* not used */, 
                                             (LPARAM)"News and Updates       ");
}

void CreateButton5(HINSTANCE hInst, HWND hWnd)
{
    g_hwndButton5 = CreateWindowEx(g_dwHeaderExStyle, "BUTTON", "", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, iOffsetLeft, iOffsetTop + 
        g_iWindowCanvasHeight + 4*g_iButtonHeight, g_iButtonWidth, 
                     g_iButtonHeight, hWnd, (HMENU)IDC_BUTTON5, hInst, 0);
    g_wpWndProc5 = (WNDPROC)SetWindowLong(g_hwndButton5, GWL_WNDPROC, 
                                                  (DWORD)Button5_SubclassedFunc);
    // Setup font for button control.
    SendMessage(g_hwndButton5, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndButton5, WM_SETTEXT, 0 /* not used */, 
                                                      (LPARAM)"Help         ");
}

void ShowCanvas1(HINSTANCE hInst, HWND hWnd)
{
    WNDCLASSEX wcex;
    int nLeft, nTop, nWidth, nHeight;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)Canvas1_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = g_hbrCanvasBackgroungColor;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("Canvas1");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hwndCanvas1 = CreateWindowEx(g_dwCanvasExStyle, TEXT("Canvas1"), 
        "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, iOffsetLeft, iOffsetTop + 
        g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight, hWnd, 0, hInst, 0);

    // Static text, line 1.
    nLeft = g_rcStatic11.left;
    nTop = g_rcStatic11.top;
    nWidth = g_rcStatic11.right - g_rcStatic11.left;
    nHeight = g_rcStatic11.bottom - g_rcStatic11.top;
    g_hwndStatic11 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
                           g_hwndCanvas1, (HMENU)IDC_CANVAS1_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic11, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic11, WM_SETTEXT, 0 /* not used */, 
         (LPARAM)"Car Explorer makes use of the XCD car description format");

    // Static text, line 2.
    nLeft = g_rcStatic12.left;
    nTop = g_rcStatic12.top;
    nWidth = g_rcStatic12.right - g_rcStatic12.left;
    nHeight = g_rcStatic12.bottom - g_rcStatic12.top;
    g_hwndStatic12 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas1, (HMENU)IDC_CANVAS1_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic12, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic12, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"that groups cars by year and make. Simply ");

    nLeft = g_rcStaticLink11.left;
    nTop = g_rcStaticLink11.top;
    nWidth = g_rcStaticLink11.right - g_rcStaticLink11.left;
    nHeight = g_rcStaticLink11.bottom - g_rcStaticLink11.top;
    g_hwndStaticLink11 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas1, (HMENU)IDC_STATIC_LINK11, hInst, NULL);
    //SendMessage(g_hwndStaticLink11, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SetFont(g_hwndStaticLink11, TRUE);
    SendMessage(g_hwndStaticLink11, WM_SETTEXT, 0 /* not used */, 
                 (LPARAM)"download");
    g_wpOldStaticLink11_WndProc = (WNDPROC)SetWindowLong(g_hwndStaticLink11, 
                               GWL_WNDPROC, (DWORD)StaticLink11_SubclassedFunc);

    // Static text, line 3. Empty field to drop cursor when move to the right
    // from the static link.
    nLeft = g_rcStatic13.left;
    nTop = g_rcStatic13.top;
    nWidth = g_rcStatic13.right - g_rcStatic13.left;
    nHeight = g_rcStatic13.bottom - g_rcStatic13.top;
    g_hwndStatic13 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas1, (HMENU)IDC_CANVAS1_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic13, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic13, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)" ");
                
    nLeft = g_rcStatic14.left;
    nTop = g_rcStatic14.top;
    nWidth = g_rcStatic14.right - g_rcStatic14.left;
    nHeight = g_rcStatic14.bottom - g_rcStatic14.top;
    g_hwndStatic14 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas1, (HMENU)IDC_CANVAS1_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic14, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic14, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"collections (CAR files in XCD) to sort out easily your");

    // Static text, line 4.
    nLeft = g_rcStatic15.left;
    nTop = g_rcStatic15.top;
    nWidth = g_rcStatic15.right - g_rcStatic15.left;
    nHeight = g_rcStatic15.bottom - g_rcStatic15.top;
    g_hwndStatic15 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas1, (HMENU)IDC_CANVAS1_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic15, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic15, WM_SETTEXT, 0 /* not used */, 
                 (LPARAM)"favorite cars among all major alternatives.");
}

void ShowCanvas2(HINSTANCE hInst, HWND hWnd)
{
    WNDCLASSEX wcex;
    int nLeft, nTop, nWidth, nHeight;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)Canvas2_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = g_hbrCanvasBackgroungColor;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("Canvas2");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hwndCanvas2 = CreateWindowEx(g_dwCanvasExStyle, TEXT("Canvas2"), "", 
        WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, iOffsetLeft, iOffsetTop + 
        g_iWindowCanvasHeight + 2*g_iButtonHeight, g_iButtonWidth, 0, hWnd, 0, 
                                                                      hInst, 0);

    // Static text, line 1.
    nLeft = g_rcStatic21.left;
    nTop = g_rcStatic21.top;
    nWidth = g_rcStatic21.right - g_rcStatic21.left;
    nHeight = g_rcStatic21.bottom - g_rcStatic21.top;
    g_hwndStatic21 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas2, (HMENU)IDC_CANVAS2_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic21, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic21, WM_SETTEXT, 0 /* not used */, 
         (LPARAM)"To keep track of your favorite cars, drop-and-drop a car");

    // Static text, line 2.
    nLeft = g_rcStatic22.left;
    nTop = g_rcStatic22.top;
    nWidth = g_rcStatic22.right - g_rcStatic22.left;
    nHeight = g_rcStatic22.bottom - g_rcStatic22.top;
    g_hwndStatic22 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas2, (HMENU)IDC_CANVAS2_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic22, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic22, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"from the right pane to the left. Selecting Favorites and");

    // Static text, line 3.
    nLeft = g_rcStatic23.left;
    nTop = g_rcStatic23.top;
    nWidth = g_rcStatic23.right - g_rcStatic23.left;
    nHeight = g_rcStatic23.bottom - g_rcStatic23.top;
    g_hwndStatic23 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas2, (HMENU)IDC_CANVAS2_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic23, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic23, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"sorting list views enable you easily analyze relative data");

    // Static text, line 4.
    nLeft = g_rcStatic24.left;
    nTop = g_rcStatic24.top;
    nWidth = g_rcStatic24.right - g_rcStatic24.left;
    nHeight = g_rcStatic24.bottom - g_rcStatic24.top;
    g_hwndStatic24 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas2, (HMENU)IDC_CANVAS2_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic24, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic24, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"such as body types, engine powers, prices.");
}

void ShowConvas3(HINSTANCE hInst, HWND hWnd)
{
    WNDCLASSEX wcex;
    int nLeft, nTop, nWidth, nHeight;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)Canvas3_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = g_hbrCanvasBackgroungColor;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("Convas3");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hwndCanvas3 = CreateWindowEx(g_dwCanvasExStyle, 
        TEXT("Convas3"), "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, 
        iOffsetLeft, iOffsetTop + g_iWindowCanvasHeight + 3*g_iButtonHeight, 
                                          g_iButtonWidth, 0, hWnd, 0, hInst, 0);

    // Static text, line 1.
    nLeft = g_rcStatic31.left;
    nTop = g_rcStatic31.top;
    nWidth = g_rcStatic31.right - g_rcStatic31.left;
    nHeight = g_rcStatic31.bottom - g_rcStatic31.top;
    g_hwndStatic31 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas3, (HMENU)IDC_CANVAS3_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic31, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic31, WM_SETTEXT, 0 /* not used */, 
         (LPARAM)"Double-click a car in the right pane. This opens picture");

    // Static text, line 2.
    nLeft = g_rcStatic32.left;
    nTop = g_rcStatic32.top;
    nWidth = g_rcStatic32.right - g_rcStatic32.left;
    nHeight = g_rcStatic32.bottom - g_rcStatic32.top;
    g_hwndStatic32 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas3, (HMENU)IDC_CANVAS3_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic32, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic32, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"viewer integrated with Windows so that you can make");

    // Static text, line 3.
    nLeft = g_rcStatic33.left;
    nTop = g_rcStatic33.top;
    nWidth = g_rcStatic33.right - g_rcStatic33.left;
    nHeight = g_rcStatic33.bottom - g_rcStatic33.top;
    g_hwndStatic33 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas3, (HMENU)IDC_CANVAS3_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic33, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic33, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"your favorite operations with a single image: scale,");

    // Static text, line 4.
    nLeft = g_rcStatic34.left;
    nTop = g_rcStatic34.top;
    nWidth = g_rcStatic34.right - g_rcStatic34.left;
    nHeight = g_rcStatic34.bottom - g_rcStatic34.top;
    g_hwndStatic34 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas3, (HMENU)IDC_CANVAS3_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic34, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic34, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"transform, print, set as desktop background and so on.");
}

void ShowCanvas4(HINSTANCE hInst, HWND hWnd)
{
    WNDCLASSEX wcex;
    int nLeft, nTop, nWidth, nHeight;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW; //0
    wcex.lpfnWndProc	= (WNDPROC)Canvas4_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = g_hbrCanvasBackgroungColor;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("Canvas4");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hwndCanvas4 = CreateWindowEx(g_dwCanvasExStyle, 
        TEXT("Canvas4"), "", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
        //TEXT("Canvas4"), "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, 
        iOffsetLeft, iOffsetTop + g_iWindowCanvasHeight + 4*g_iButtonHeight, 
                                          g_iButtonWidth, 0, hWnd, 0, hInst, 0);
}

void ShowCanvas5(HINSTANCE hInst, HWND hWnd)
{
    WNDCLASSEX wcex;
    int nLeft, nTop, nWidth, nHeight;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)Canvas5_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = g_hbrCanvasBackgroungColor;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("Canvas5");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hwndCanvas5 = CreateWindowEx(g_dwCanvasExStyle, 
        TEXT("Canvas5"), "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, 
        iOffsetLeft, iOffsetTop + g_iWindowCanvasHeight + 5*g_iButtonHeight, 
                                          g_iButtonWidth, 0, hWnd, 0, hInst, 0);

    // Static text, line 1.
    nLeft = g_rcStatic51.left;
    nTop = g_rcStatic51.top;
    nWidth = g_rcStatic51.right - g_rcStatic51.left;
    nHeight = g_rcStatic51.bottom - g_rcStatic51.top;
    g_hwndStatic51 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas5, (HMENU)IDC_CANVAS5_STATIC, hInst, NULL);
    SendMessage(g_hwndStatic51, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SendMessage(g_hwndStatic51, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"You can get more details from:");

    // Static text, line 2.
    nLeft = g_rcStaticLink51.left;
    nTop = g_rcStaticLink51.top;
    nWidth = g_rcStaticLink51.right - g_rcStaticLink51.left;
    nHeight = g_rcStaticLink51.bottom - g_rcStaticLink51.top;
    g_hwndStaticLink51 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas5, (HMENU)IDC_STATIC_LINK51, hInst, NULL);
    //SendMessage(g_hwndStatic52, WM_SETFONT, (WPARAM)g_hfnt, TRUE);
    SetFont(g_hwndStaticLink51, TRUE);
    SendMessage(g_hwndStaticLink51, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"Car Explorer Web Pages");

    g_wpOldStaticLink51_WndProc = (WNDPROC)SetWindowLong(g_hwndStaticLink51, 
                               GWL_WNDPROC, (DWORD)StaticLink51_SubclassedFunc);

    // Static text, line 3.
    nLeft = g_rcStaticLink52.left;
    nTop = g_rcStaticLink52.top;
    nWidth = g_rcStaticLink52.right - g_rcStaticLink52.left;
    nHeight = g_rcStaticLink52.bottom - g_rcStaticLink52.top;
    g_hwndStaticLink52 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas5, (HMENU)IDC_STATIC_LINK52, hInst, NULL);
    SetFont(g_hwndStaticLink52, TRUE);
    SendMessage(g_hwndStaticLink52, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"EMail Support");

    g_wpOldStaticLink52_WndProc = (WNDPROC)SetWindowLong(g_hwndStaticLink52, 
                               GWL_WNDPROC, (DWORD)StaticLink52_SubclassedFunc);

    // Static text, line 4.
    nLeft = g_rcStaticLink53.left;
    nTop = g_rcStaticLink53.top;
    nWidth = g_rcStaticLink53.right - g_rcStaticLink53.left;
    nHeight = g_rcStaticLink53.bottom - g_rcStaticLink53.top;
    g_hwndStaticLink53 =  CreateWindow("STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, nLeft, nTop, nWidth, nHeight, 
             g_hwndCanvas5, (HMENU)IDC_STATIC_LINK53, hInst, NULL);
    SetFont(g_hwndStaticLink53, TRUE);
    SendMessage(g_hwndStaticLink53, WM_SETTEXT, 0 /* not used */, 
        (LPARAM)"Dedicated Forum");

    g_wpOldStaticLink53_WndProc = (WNDPROC)SetWindowLong(g_hwndStaticLink53, 
                               GWL_WNDPROC, (DWORD)StaticLink53_SubclassedFunc);
}


LRESULT APIENTRY Canvas1_WndProc(HWND hWnd, UINT Message, 
                                                     WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_STATIC_LINK11) // handle URL click
        {
            HWND hwnd;
            COPYDATASTRUCT cds;

            // TODO: probably simplify with g_hWnd (passed from Main CE) of 
            // GetParent(hWnd) - failed in test. Could also use window name.
            hwnd = FindWindow("CAREXPLORER" /* class name of receiving window */, 
                                                                          NULL);
            if (hwnd != NULL)
            {
                cds.dwData = 0; // signals download link click
                cds.cbData = 0;
                cds.lpData = NULL;
                SendMessage(hwnd /* receiving window */, WM_COPYDATA,
                                            0 /* sender window */, (LPARAM)&cds);
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
        if (g_hwndStaticLink11 == (HWND)lParam /* handle to static control */)
        {
            SetTextColor((HDC)wParam, RGB(255,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            //return (BOOL)GetStockObject(HOLLOW_BRUSH);
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }

        if (g_hwndStatic11 == (HWND)lParam /* handle to static control */ ||
            g_hwndStatic12 == (HWND)lParam || g_hwndStatic13 == (HWND)lParam ||
               g_hwndStatic14 == (HWND)lParam || g_hwndStatic15 == (HWND)lParam)
        {
            SetTextColor((HDC)wParam, RGB(0,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }
        break;
    case WM_ERASEBKGND:
        //return 0; // 0 - not erased, 1 -erase (LL does not matter, still
        // blinking)
        break;
    default:
        return DefWindowProc( hWnd, Message, wParam, lParam );
    }
    return DefWindowProc( hWnd, Message, wParam, lParam );
 }

LRESULT APIENTRY StaticLink11_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    if (Message == WM_SETCURSOR)
    {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return 1; // both 1 and 0 are OK 
    }

    return CallWindowProc((WNDPROC)g_wpOldStaticLink11_WndProc, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY Canvas2_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        // ...
        break;
    case WM_COMMAND:
        //switch (LOWORD(wParam))
        break;
    case WM_CTLCOLORSTATIC:
        if (g_hwndStatic21 == (HWND)lParam /* handle to static control */ ||
            g_hwndStatic22 == (HWND)lParam ||
               g_hwndStatic23 == (HWND)lParam || g_hwndStatic24 == (HWND)lParam)
        {
            SetTextColor((HDC)wParam, RGB(0,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }
        break;
    case WM_ERASEBKGND:
        //return 0; // 0 - not erased, 1 -erase (LL does not matter, still
        // blinking)
        break;
    default:
        return DefWindowProc( hWnd, Message, wParam, lParam );
    }
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

LRESULT APIENTRY Canvas3_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        // ...
        break;
    case WM_COMMAND:
        //switch (LOWORD(wParam))
        break;
    case WM_CTLCOLORSTATIC:
        if (g_hwndStatic31 == (HWND)lParam /* handle to static control */ ||
            g_hwndStatic32 == (HWND)lParam ||
               g_hwndStatic33 == (HWND)lParam || g_hwndStatic34 == (HWND)lParam)
        {
            SetTextColor((HDC)wParam, RGB(0,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }
        break;
    case WM_ERASEBKGND:
        //return 0; // 0 - not erased, 1 -erase (LL does not matter, still
        // blinking)
        break;
    default:
        return DefWindowProc( hWnd, Message, wParam, lParam );
    }
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

LRESULT APIENTRY Canvas4_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;
    
    switch (Message)
    {
    case WM_COMMAND:
        //switch (HIWORD(wParam))
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnPaint_Canvas4(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_ERASEBKGND: // 0 or 1 removes blinking when resising
        return 1; // 0 - not erased, 1 -erase (LL does not matter, still
    //	// blinking)
    default:
        return DefWindowProc( hWnd, Message, wParam, lParam );
    }
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

LRESULT APIENTRY Canvas5_WndProc(HWND hWnd, UINT Message, 
                                                     WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_COMMAND:
        // Handle URL clicks.
        if (LOWORD(wParam) == IDC_STATIC_LINK51) // CE web pages
        {
            //MessageBox(0, "IDC_STATIC_LINK51", "", 0);
            ShellExecute(NULL, "open", 
                             "http://www.carexplorer.org", 0, 0, SW_SHOWNORMAL);
        }
        if (LOWORD(wParam) == IDC_STATIC_LINK52) // email
        {
            ShellExecute(NULL, "open", 
                         "mailto:support@carexplorer.org", 0, 0, SW_SHOWNORMAL); 
                                                                 
        }
        if (LOWORD(wParam) == IDC_STATIC_LINK53) // forum
        {
            //MessageBox(0, "IDC_STATIC_LINK53", "", 0);
            ShellExecute(NULL, "open", 
                       "http://www.carexplorer.org/forum", 0, 0, SW_SHOWNORMAL);
        }
        break;
    case WM_CTLCOLORSTATIC:
        if (g_hwndStatic51 == (HWND)lParam /* handle to static control */)
        {
            SetTextColor((HDC)wParam, RGB(0,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }

        if (g_hwndStaticLink51 == (HWND)lParam ||
            g_hwndStaticLink52 == (HWND)lParam || 
                                             g_hwndStaticLink53 == (HWND)lParam)
        {
            SetTextColor((HDC)wParam, RGB(255, 0, 0));
            SetBkMode((HDC)wParam, TRANSPARENT); // keep the same background
            //return (BOOL)GetStockObject(HOLLOW_BRUSH);
            return ((LRESULT)(HBRUSH)CreateSolidBrush(g_clrCanvasBackground)); 
        }
        break;
    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    return DefWindowProc(hWnd, Message, wParam, lParam);
}

LRESULT APIENTRY StaticLink51_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    if (Message == WM_SETCURSOR)
    {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return 1; 
    }

    return CallWindowProc((WNDPROC)g_wpOldStaticLink51_WndProc, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY StaticLink52_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    if (Message == WM_SETCURSOR)
    {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return 1; 
    }

    return CallWindowProc((WNDPROC)g_wpOldStaticLink52_WndProc, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY StaticLink53_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    if (Message == WM_SETCURSOR)
    {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return 1; 
    }

    return CallWindowProc((WNDPROC)g_wpOldStaticLink53_WndProc, hWnd, Message, 
                                                                wParam, lParam);
}

void Button1_StartTimer()
{
    g_bAnyButtonMoving = TRUE;
    iTimerCounter = 0;
    // Updates state for all canvases and set g_b...WindowExpanded flags.
    SetCanvasState();
    g_iButton1_TimerID = SetTimer(NULL, NULL, g_nButtonSpeed, 
                                                 (TIMERPROC)Button1_TimerProc);
}

void CALLBACK Button1_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                   DWORD dwTime)
{
    int nLeft, nTop, nWidth, nHeight;

    if (g_nSpeedFactor == 1)
        iTimerCounter++; 
    else if (g_nSpeedFactor == 2)
    {
        iTimerCounter++;iTimerCounter++; // timer speed 2 titmes increased.
    }
    else if (g_nSpeedFactor == 3)
    {
        iTimerCounter++;iTimerCounter++;iTimerCounter++; 
    }

    if (iTimerCounter >= g_iWindowCanvasHeight/2) 
    {
        //SetButtonImage(g_hwndButton1, 1 /* right */);
        KillTimer(NULL, g_iButton1_TimerID);
        g_bAnyButtonMoving = FALSE;
    }

    if (g_bCanvas2Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
                        g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);
        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + g_iButtonHeight 
                      + 2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight 
         + 2*iTimerCounter, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                         2*iTimerCounter, TRUE); 
    }

    if (g_bConvas3Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
                        g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);
        
        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas4Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
                        g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);
        MoveControls_Canvas1(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            + 3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas5Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
                        g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);

        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter - g_iWindowCanvasHeight, 0 /* move down */);
        
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            + 3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            + 4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            5*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }
}

void Button2_StartTimer()
{
    g_bAnyButtonMoving = TRUE;
    iTimerCounter = 0;
    SetCanvasState();
    g_iButton2_TimerID = SetTimer(NULL, NULL, g_nButtonSpeed, 
                                                     (TIMERPROC)Button2_TimerProc);
}
void CALLBACK Button2_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    int nLeft, nTop, nWidth, nHeight;	

    if (g_nSpeedFactor == 1)
        iTimerCounter++; 
    else if (g_nSpeedFactor == 2)
    {
        iTimerCounter++;iTimerCounter++; // timer speed 2 titmes increased.
    }
    else if (g_nSpeedFactor == 3)
    {
        iTimerCounter++;iTimerCounter++;iTimerCounter++; 
    }

    if (iTimerCounter >= g_iWindowCanvasHeight/2) 
    {
        //SetButtonImage(g_hwndButton1, 0 /* right */);
        //SetButtonImage(g_hwndButton2, 1 /* down */);
        KillTimer(NULL, g_iButton2_TimerID);
        g_bAnyButtonMoving = FALSE;
    }
                                                    
    if (g_bCanvas1Expanded) 
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
           g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                         2*iTimerCounter, TRUE);
        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter, 1 /* move up */);
        
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + g_iButtonHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, 2*iTimerCounter, TRUE);
    }

    if (g_bConvas3Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
                                         g_iButtonWidth, 2*iTimerCounter, TRUE);
        MoveControls_Canvas2(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
               2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas4Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
                                         g_iButtonWidth, 2*iTimerCounter, TRUE);
        MoveControls_Canvas2(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
               2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
             + 3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas5Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
                                         g_iButtonWidth, 2*iTimerCounter, TRUE);

        MoveControls_Canvas2(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                            0 /* move down */);

        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
               2*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
             + 3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
             + 4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            5*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }
}

LRESULT APIENTRY Button1_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    //PAINTSTRUCT ps;
    //RECT rc, rcIW;
    //HDC hdc, hMemDC;
    //HBITMAP hBitmap, hOldBitmap;
    //POINT pt;
    //BOOL bLeftMouse; 

    switch (Message)
    {
    //case WM_CREATE:
    //	break;
    // I handle WM_LBUTTONDOWN for button click instead of WM_COMMAND since
    // WM_COMMAND for reason unknown to me does not work in subclassed button.
    // Message equal to 15 is fired ... (WM_CHAR is handled here if I change, 
    // for testing, BUTTON to EDIT class). WM_COMMAND however trapped in 
    // application procedure.
    case WM_LBUTTONDOWN:
        g_nButtonLastClicked = 1;
        if (!g_bAnyButtonMoving) Button1_StartTimer();
        SetButtonImage(g_hwndButton1, 1 /* down */);
        SetButtonImage(g_hwndButton2, 0 /* right */);
        SetButtonImage(g_hwndButton3, 0 /* right */);
        SetButtonImage(g_hwndButton4, 0 /* right */);
        SetButtonImage(g_hwndButton5, 0 /* right */);
        if (g_bUsePushStyle) 
            break;
        else
            return 0;
    //case WM_COMMAND:
    //	//switch (LOWORD(wParam) /* control ID*/)
    //	//{
    //	//wmId    = LOWORD(wParam); 
    //	//wmEvent = HIWORD(wParam); 
    //	MessageBox(0, "asdasd", "WM_COMMAND", 0); //OK
    //	break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)g_wpWndProc, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY Button2_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    //RECT rc, rcIW;
    //POINT pt;
    //BOOL bLeftMouse;
    
    switch (Message)
    {
    //case WM_CREATE:
    //	//MessageBox(0, "WM_CREATE", "SubclassedFunc", 0);
    //	break;
    //// I handle WM_LBUTTONDOWN for button click instead of WM_COMMAND since
    //// WM_COMMAND for reason unknown to me does not work in subclassed button.
    //// Message equal to 15 is fired ... (WM_CHAR is handled here if I change, 
    //// for testing, BUTTON to EDIT class). WM_COMMAND however trapped in 
    //// application procedure.
    case WM_LBUTTONDOWN:
        g_nButtonLastClicked = 2;
        if (!g_bAnyButtonMoving) Button2_StartTimer();
        SetButtonImage(g_hwndButton1, 0 /* right */);
        SetButtonImage(g_hwndButton2, 1 /* down */);
        SetButtonImage(g_hwndButton3, 0 /* right */);
        SetButtonImage(g_hwndButton4, 0 /* right */);
        SetButtonImage(g_hwndButton5, 0 /* right */);
        if (g_bUsePushStyle) 
            break;
        else
            return 0;
    //case WM_COMMAND:
    //	//switch (LOWORD(wParam) /* control ID*/)
    //	//{
    //	//wmId    = LOWORD(wParam); 
    //	//wmEvent = HIWORD(wParam); 
    //	MessageBox(0, "asdasd", "WM_COMMAND", 0); //OK
    //	break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)g_wpWndProc2, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY Button3_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        //MessageBox(0, "WM_CREATE", "SubclassedFunc", 0);
        break;
    // I handle WM_LBUTTONDOWN for button click instead of WM_COMMAND since
    // WM_COMMAND for reason unknown to me does not work in subclassed button.
    // Message equal to 15 is fired ... (WM_CHAR is handled here if I change, 
    // for testing, BUTTON to EDIT class). WM_COMMAND however trapped in 
    // application procedure.
    case WM_LBUTTONDOWN:
        g_nButtonLastClicked = 3;
        if (!g_bAnyButtonMoving) Button3_StartTimer();
        SetButtonImage(g_hwndButton1, 0 /* right */);
        SetButtonImage(g_hwndButton2, 0 /* right */);
        SetButtonImage(g_hwndButton3, 1 /* down */);
        SetButtonImage(g_hwndButton4, 0 /* right */);
        SetButtonImage(g_hwndButton5, 0 /* right */);
        if (g_bUsePushStyle) 
            break;
        else
            return 0;
    case WM_COMMAND:
        //switch (LOWORD(wParam) /* control ID*/)
        //{
        //wmId    = LOWORD(wParam); 
        //wmEvent = HIWORD(wParam); 
        //MessageBox(0, "asdasd", "WM_COMMAND", 0); //OK
        break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)g_wpWndProc3, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY Button4_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        //MessageBox(0, "WM_CREATE", "SubclassedFunc", 0);
        break;
    // I handle WM_LBUTTONDOWN for button click instead of WM_COMMAND since
    // WM_COMMAND for reason unknown to me does not work in subclassed button.
    // Message equal to 15 is fired ... (WM_CHAR is handled here if I change, 
    // for testing, BUTTON to EDIT class). WM_COMMAND however trapped in 
    // application procedure.
    case WM_LBUTTONDOWN:
        g_nButtonLastClicked = 4;
        if (!g_bAnyButtonMoving) Button4_StartTimer();
        SetButtonImage(g_hwndButton1, 0 /* right */);
        SetButtonImage(g_hwndButton2, 0 /* right */);
        SetButtonImage(g_hwndButton3, 0 /* right */);
        SetButtonImage(g_hwndButton4, 1 /* down */);
        SetButtonImage(g_hwndButton5, 0 /* right */);
        if (g_bUsePushStyle) 
            break;
        else
            return 0;
    case WM_COMMAND:
        //switch (LOWORD(wParam) /* control ID*/)
        //{
        //wmId    = LOWORD(wParam); 
        //wmEvent = HIWORD(wParam); 
        //MessageBox(0, "asdasd", "WM_COMMAND", 0); //OK
        break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)g_wpWndProc4, hWnd, Message, 
                                                                wParam, lParam);
}

LRESULT APIENTRY Button5_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        //MessageBox(0, "WM_CREATE", "SubclassedFunc", 0);
        break;
    // I handle WM_LBUTTONDOWN for button click instead of WM_COMMAND since
    // WM_COMMAND for reason unknown to me does not work in subclassed button.
    // Message equal to 15 is fired ... (WM_CHAR is handled here if I change, 
    // for testing, BUTTON to EDIT class). WM_COMMAND however trapped in 
    // application procedure.
    case WM_LBUTTONDOWN:
        g_nButtonLastClicked = 5;
        if (!g_bAnyButtonMoving) Button5_StartTimer();
        SetButtonImage(g_hwndButton1, 0 /* right */);
        SetButtonImage(g_hwndButton2, 0 /* right */);
        SetButtonImage(g_hwndButton3, 0 /* right */);
        SetButtonImage(g_hwndButton4, 0 /* right */);
        SetButtonImage(g_hwndButton5, 1 /* down */);
        if (g_bUsePushStyle) 
            break;
        else
            return 0;
    case WM_COMMAND:
        //switch (LOWORD(wParam) /* control ID*/)
        //{
        //wmId    = LOWORD(wParam); 
        //wmEvent = HIWORD(wParam); 
        //MessageBox(0, "asdasd", "WM_COMMAND", 0); //OK
        break;
    default:
        break;
    }

    return CallWindowProc((WNDPROC)g_wpWndProc5, hWnd, Message, 
                                                                wParam, lParam);
}


void Button3_StartTimer()
{
    g_bAnyButtonMoving = TRUE;
    iTimerCounter = 0;
    SetCanvasState();
    g_iButton3_TimerID = SetTimer(NULL, NULL, g_nButtonSpeed, 
                                                (TIMERPROC)Button3_TimerProc);
}

void CALLBACK Button3_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime)
{
    int nLeft, nTop, nWidth, nHeight;
    
    if (g_nSpeedFactor == 1)
        iTimerCounter++; 
    else if (g_nSpeedFactor == 2)
    {
        iTimerCounter++;iTimerCounter++; // timer speed 2 titmes increased.
    }
    else if (g_nSpeedFactor == 3)
    {
        iTimerCounter++;iTimerCounter++;iTimerCounter++;
    }

    if (iTimerCounter >= g_iWindowCanvasHeight/2) 
    {
        KillTimer(NULL, g_iButton3_TimerID);
        g_bAnyButtonMoving = FALSE;
    }

    if (g_bCanvas1Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop +  
            g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                         2*iTimerCounter, TRUE);

        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter, 1 /* move up */);
                                                          
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + g_iButtonHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 
                          2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
                        2*iTimerCounter, g_iButtonWidth, 2*iTimerCounter, TRUE); 
                                                
    }

    if (g_bCanvas2Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
                 g_iButtonWidth, g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
                                                                          
        // Move controls of canvas 2 (static text, line 1-4, and URL link).
        MoveControls_Canvas2(2*iTimerCounter, 1 /* move up */);

        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
           2*g_iButtonHeight + g_iWindowCanvasHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight + g_iWindowCanvasHeight 
                      - 2*iTimerCounter, g_iButtonWidth, 2*iTimerCounter, TRUE);
                                               
    }

    if (g_bCanvas4Expanded)
    {
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
                      3*g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);
        MoveControls_Canvas3(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas5Expanded)
    {
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
                      3*g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);

        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            5*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                 g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
        MoveControls_Canvas3(2*iTimerCounter - g_iWindowCanvasHeight, 
                                                             0 /* move down */);
    }
}

void Button4_StartTimer()
{
    g_bAnyButtonMoving = TRUE;
    iTimerCounter = 0;
    SetCanvasState();
    g_bCanvas4Contracting = FALSE;
    // Set dynamic positions of 4th canvas to initial positions.
    g_sTextArea1_Dyn = g_sTextArea1;
    g_sTextArea2_Dyn = g_sTextArea2;
    g_sTextArea3_Dyn = g_sTextArea3;
    g_sTextArea4_Dyn = g_sTextArea4;
    g_sTextArea5_Dyn = g_sTextArea5;
    g_sTextArea6_Dyn = g_sTextArea6;
    g_iButton4_TimerID = SetTimer(NULL, NULL, g_nButtonSpeed, 
                                               (TIMERPROC)Button4_TimerProc);
}

void CALLBACK Button4_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, 
                                                                  DWORD dwTime)
{
    int nLeft, nTop, nWidth, nHeight;

    if (g_nSpeedFactor == 1)
        iTimerCounter++; 
    else if (g_nSpeedFactor == 2)
    {
        iTimerCounter++;iTimerCounter++; // timer speed 2 titmes increased.
    }
    else if (g_nSpeedFactor == 3)
    {
        iTimerCounter++;iTimerCounter++;iTimerCounter++; 
    }
    
    if (iTimerCounter >= g_iWindowCanvasHeight/2) 
    {
        KillTimer(NULL, g_iButton4_TimerID);
        g_bAnyButtonMoving = FALSE;
    }

    if (g_bCanvas1Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
            g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                               2*iTimerCounter, TRUE);

        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter, 1 /* move up */);
                                                         
        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + g_iButtonHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 
                         2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
                         2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                               2*iTimerCounter, TRUE); 
    }
    
    if (g_bCanvas2Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
            g_iButtonWidth, g_iWindowCanvasHeight - 2*iTimerCounter, 
                                                                          TRUE);
        // Move controls of canvas 2 (static text, line 1-4, and URL link).
        MoveControls_Canvas2(2*iTimerCounter, 1 /* move up */);

        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 
              2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
              2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                               2*iTimerCounter, TRUE); 
    }

    if (g_bConvas3Expanded)
    {
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                               2*iTimerCounter, TRUE);
        MoveControls_Canvas3(2*iTimerCounter, 1 /* move up */);

        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
              2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                               2*iTimerCounter, TRUE);
    }

    if (g_bCanvas5Expanded)
    {
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight, g_iButtonWidth, 2*iTimerCounter, TRUE);

        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);

        // TODO: this MoveWindow LL redundant. Probably remove.
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            5*g_iButtonHeight + 2*iTimerCounter, g_iButtonWidth, 
                      g_iWindowCanvasHeight - 2*iTimerCounter, TRUE);
    }
}

void Button5_StartTimer()
{
    g_bAnyButtonMoving = TRUE;
    iTimerCounter = 0;
    SetCanvasState();
    g_iButton5_TimerID = SetTimer(NULL, NULL, g_nButtonSpeed, 
                                                   (TIMERPROC)Button5_TimerProc);
}

void CALLBACK Button5_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    int nLeft, nTop, nWidth, nHeight;

    if (g_nSpeedFactor == 1)
        iTimerCounter++; 
    else if (g_nSpeedFactor == 2)
    {
        iTimerCounter++;iTimerCounter++; // timer speed 2 titmes increased.
    }
    else if (g_nSpeedFactor == 3)
    {
        iTimerCounter++;iTimerCounter++;iTimerCounter++; 
    }

    if (iTimerCounter >= g_iWindowCanvasHeight/2) 
    {
        KillTimer(NULL, g_iButton5_TimerID);
        g_bAnyButtonMoving = FALSE;
    }

    if (g_bCanvas1Expanded)
    {
        MoveWindow(g_hwndCanvas1, iOffsetLeft, iOffsetTop + 
            g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                  2*iTimerCounter, TRUE);

        // Move controls of canvas 1 (static text, line 1-4, and URL link).
        MoveControls_Canvas1(2*iTimerCounter, 1 /* move up */);

        MoveWindow(g_hwndButton2, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + g_iButtonHeight - 2*iTimerCounter, 
                                         g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 
                         2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
                         2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
                         2*iTimerCounter, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 5*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                                  2*iTimerCounter, TRUE); 
    }
    
    if (g_bCanvas2Expanded)
    {
        MoveWindow(g_hwndCanvas2, iOffsetLeft, iOffsetTop + 2*g_iButtonHeight, 
            g_iButtonWidth, g_iWindowCanvasHeight - 2*iTimerCounter, 
                                                                          TRUE);
        // Move controls of canvas 2 (static text, line 1-4, and URL link).
        MoveControls_Canvas2(2*iTimerCounter, 1 /* move up */);
        
        MoveWindow(g_hwndButton3, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 2*g_iButtonHeight - 
                 2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
                 2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
                 2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 5*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                                  2*iTimerCounter, TRUE); 
    }

    if (g_bConvas3Expanded)
    {
        MoveWindow(g_hwndCanvas3, iOffsetLeft, iOffsetTop + 
            3*g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                         2*iTimerCounter, TRUE);
        MoveControls_Canvas3(2*iTimerCounter, 1 /* move up */);
        MoveWindow(g_hwndButton4, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 3*g_iButtonHeight - 
                        2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
                        2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 5*g_iButtonHeight - 
                        2*iTimerCounter, g_iButtonWidth, 2*iTimerCounter, TRUE);
    }

    if (g_bCanvas4Expanded)
    {
        MoveWindow(g_hwndCanvas4, iOffsetLeft, iOffsetTop + 
            4*g_iButtonHeight, g_iButtonWidth, g_iWindowCanvasHeight - 
                                                  2*iTimerCounter, TRUE);
        g_bCanvas4Contracting = TRUE;
        MoveControls_Canvas4(2*iTimerCounter, 1 /* move up */);

        MoveWindow(g_hwndButton5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 4*g_iButtonHeight - 
                 2*iTimerCounter, g_iButtonWidth, g_iButtonHeight, TRUE);
        MoveWindow(g_hwndCanvas5, iOffsetLeft, iOffsetTop + 
            g_iWindowCanvasHeight + 5*g_iButtonHeight - 
            2*iTimerCounter, g_iButtonWidth, 
                                                  2*iTimerCounter, TRUE);
    }
}

BOOL IsCanvasExpanded(HWND hwnd /* canvas window */, int iCanvas)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);

    TCHAR szTest[100];
    wsprintf(szTest, "Height: %d", rc.bottom - rc.top);
    //MessageBox(0, szTest, szTest, 0);

    if (rc.bottom - rc.top >= 50) //== g_iWindowCanvasHeight) 
        return TRUE;
    else 
        return FALSE;
}

void SetCanvasState()
{
    g_bCanvas1Expanded = FALSE; 
    g_bCanvas2Expanded = FALSE;
    g_bConvas3Expanded = FALSE;
    g_bCanvas4Expanded = FALSE;
    g_bCanvas5Expanded = FALSE;

    //MessageBox(0, "szTest", "szTest", 0);

    if (IsCanvasExpanded(g_hwndCanvas1, 0)) 
    {	
        g_bCanvas1Expanded = TRUE; 
    }
    if (IsCanvasExpanded(g_hwndCanvas2, 1))
    {	
        g_bCanvas2Expanded = TRUE; 
    }
    if (IsCanvasExpanded(g_hwndCanvas3, 2))
    {	
        g_bConvas3Expanded = TRUE;  
    }
    if (IsCanvasExpanded(g_hwndCanvas4, 3))
    {	
        g_bCanvas4Expanded = TRUE;  
    }
    if (IsCanvasExpanded(g_hwndCanvas5, 4))
    {	
        g_bCanvas5Expanded = TRUE;  
    }
}

// Create font for controls. 
HFONT CreateControlsFont()
{
    HFONT hfnt;
    LOGFONT lf;
    
    lf.lfHeight = 14;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_DONTCARE;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE;
    //strcpy(lf.lfFaceName, "Arial");  
    strcpy(lf.lfFaceName, "MS Shell Dlg");

    hfnt = CreateFontIndirect(&lf);

    return hfnt;
}

void SetFont(HWND hwnd, BOOL bBold)
{
    HFONT hfnt;
    LOGFONT lf;

    // Specify the font to use (stored in a LOGFONT structure).
    lf.lfHeight = 14;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight =  bBold ? FW_BOLD : FW_DONTCARE; 
    lf.lfItalic = FALSE;
    lf.lfUnderline = TRUE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE;
    lstrcpy(lf.lfFaceName, "MS Shell Dlg");

    hfnt = CreateFontIndirect(&lf);
    //hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT);  
    SendMessage(hwnd,  WM_SETFONT, (WPARAM)hfnt, TRUE);
}


//--



// TODO: LL not used, remove
int GetButtonID(HWND hWnd)
{
    RECT rc;
    POINT pt;

    if (GetParent(hWnd) == g_hwndButton1)
        return 1;
    else if (GetParent(hWnd) == g_hwndButton2)
        return 2;

    return 0;
}

// TODO: optimize if possible to use one shared class.
void RegisterImageWindowClasses(HINSTANCE hInst, HWND hWnd)
{
    // Was used to register ImageWindow[x] classes.
}

void LoadBitmaps()
{
    g_hBitmapRight = LoadBitmap(GetModuleHandle("CE_AnimUI.dll"), 
                                                    MAKEINTRESOURCE(IDB_RIGHT));
    g_hBitmapDown = LoadBitmap(GetModuleHandle("CE_AnimUI.dll"), 
                                                     MAKEINTRESOURCE(IDB_DOWN));
}

void App_Debug_FunctionCallTrace(LPCTSTR pszMessage)
{
    TCHAR szFFN[MAX_PATH];
    HANDLE hTraceFile;
    DWORD dwOffset;
    DWORD dWritten;
    char szLine[500];

    SYSTEMTIME st;
    GetLocalTime(&st);

    char szMonth[5];
    char szDay[5];
    char szYear[6];
    char szHour[5];
    char szMinute[5];
    char szSecond[5];

    char szDate[12];
    char szTime[10];
    //DeleteFile(_T("TraceFile.txt"));

    // Set the location of the debug file CE_TraceFile.txt.
    wsprintf(szFFN, "%s%s", 
       "C:\\MyFiles\\Development\\VCC\\CarExplorer.NET\\", 
                                                   "\\CE_AnimUI_TraceFile.txt");
                                                                
    // Convert "2:6" time into "02:06" time
    wsprintf(szHour, st.wHour < 10 ? "0%d" : "%d",  st.wHour);
    wsprintf(szMinute, st.wMinute < 10 ? "0%d" : "%d",  st.wMinute);
    wsprintf(szSecond, st.wSecond < 10 ? "0%d" : "%d",  st.wSecond);
    wsprintf(szYear, "%d", st.wYear);

    // Convert "1/29/2005" date into "01/29/05" date
    wsprintf(szMonth, st.wMonth < 10 ? "0%d" : "%d",  st.wMonth);
    wsprintf(szDay, st.wDay < 10 ? "0%d" : "%d",  st.wDay);

    wsprintf(szDate, "%s/%s/%s", szMonth, szDay, szYear); 
    wsprintf(szTime, "%s:%s:%s", szHour, szMinute, szSecond);

    //if (g_bEnableDebug) 
        hTraceFile = CreateFileA(szFFN, 
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /* security - not used */,          
        OPEN_ALWAYS /* if exists - just open; if does not exist, create it */,
                                  FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

    // Tell the writer we need to write at the end of file.
    dwOffset = SetFilePointer(hTraceFile, 0, NULL, FILE_END);

    wsprintf(szLine, "%s,%s: %s\r\n", szDate, szTime, pszMessage); 
    //sprintf(szLine, "%s\r\n", pszMessage);
    WriteFile(hTraceFile, szLine, strlen(szLine), &dWritten, NULL);
                                                                          
    CloseHandle(hTraceFile);
}

void SetButtonImage(HWND hWnd, int nImage) // 0 - right, 1 - down (clockwise)
{
    HIMAGELIST hIL; 
    HICON hIcon;
    RECT rc = {3,1,0,0};
    BUTTON_IMAGELIST bil;

    // MSDN: The image list referred to in the himl member of the BUTTON_IMAGELIST 
    // structure should contain either a single image to be used for all states or 
    // individual images for each state.
    //hIL = ImageList_Create(33, 33, ILC_COLOR24 | ILC_MASK, // OK
    hIL = ImageList_Create(33, 33, ILC_COLORDDB | ILC_MASK,
    //hIL = ImageList_Create(30, 30, ILC_COLORDDB | ILC_MASK, // OK, decreases size from original
    //hIL = ImageList_Create(33, 33, ILC_COLOR24, // no transparency
                                    0 /* number of initial images */, 0);

    if (hWnd == g_hwndButton1)
    {
        //g_hButton1Bitmap = g_hBitmapDown; // weird, but global bitmap handle DNW 
        
        if (nImage == 0 /* right */)
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_RIGHT)); 
            g_nImage = 0;
        }
        else
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_DOWN)); 
            g_nImage = 1;
        }
    }
    else if (hWnd == g_hwndButton2)
    {
        //g_hButton2Bitmap = g_hBitmapDown; 
        
        if (nImage == 0 /* right */)
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_RIGHT)); 
            g_nImage2 = 0;
        }
        else
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_DOWN));
            g_nImage2 = 1;
        }
    }
    else if (hWnd == g_hwndButton3)
    {
        if (nImage == 0 /* right */)
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_RIGHT)); 
            g_nImage2 = 0;
        }
        else
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_DOWN));
            g_nImage2 = 1;
        }
    }
    else if (hWnd == g_hwndButton4)
    {
        if (nImage == 0 /* right */)
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_RIGHT)); 
            g_nImage2 = 0;
        }
        else
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_DOWN));
            g_nImage2 = 1;
        }
    }
    else if (hWnd == g_hwndButton5)
    {
        if (nImage == 0 /* right */)
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_RIGHT)); 
            g_nImage2 = 0;
        }
        else
        {
            hIcon = LoadIcon(GetModuleHandle("CE_AnimUI.dll"), MAKEINTRESOURCE(IDI_DOWN));
            g_nImage2 = 1;
        }
    }

    ImageList_AddIcon(hIL, hIcon);
    DeleteObject(hIcon);
        
    // Prepare BUTTON_IMAGELIST struce
    bil.himl = hIL;
    bil.margin = rc;
    bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

    // Put image over the button.
    SendMessage(hWnd,  BCM_SETIMAGELIST, 0, 
                                        (LPARAM)(PBUTTON_IMAGELIST)&bil);

    RedrawWindow(hWnd, 
        NULL /* invalidate entire client area */, 
                      NULL /* invalidate entire client area */, RDW_INVALIDATE);

    // TODO: probably use 
    // ImageList_Destroy(hIL);
}

// Note: used externally to move control when main window is resized.
void MoveControl(int iOffsetLeft)
{
    int nLeftButton1, nTopButton1, nLeftCanvas1, nTopCanvas1, nHeightCanvas1; 
    int nLeftButton2, nTopButton2, nLeftCanvas2, nTopCanvas2, nHeightCanvas2; 
    int nLeftButton3, nTopButton3, nLeftCanvas3, nTopCanvas3, nHeightCanvas3; 
    int nLeftButton4, nTopButton4, nLeftCanvas4, nTopCanvas4, nHeightCanvas4; 
    int nLeftButton5, nTopButton5, nLeftCanvas5, nTopCanvas5, nHeightCanvas5; 

    if (g_nButtonLastClicked == 0 /* no clicks */ || g_nButtonLastClicked == 1)
    {
        nLeftButton1 = iOffsetLeft; 
        nTopButton1  = iOffsetTop; 
        nLeftCanvas1 = iOffsetLeft;
        nTopCanvas1  = iOffsetTop + g_iButtonHeight; 
        nHeightCanvas1 = g_iWindowCanvasHeight; 

        nLeftButton2 = iOffsetLeft; 
        nTopButton2  = iOffsetTop + g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas2 = iOffsetLeft;
        nTopCanvas2  = iOffsetLeft,iOffsetTop + 2*g_iButtonHeight + 
                                                          g_iWindowCanvasHeight; 
        nHeightCanvas2 = 0;

        nLeftButton3 = iOffsetLeft; 
        nTopButton3  = iOffsetTop + 2*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas3 = iOffsetLeft;
        nTopCanvas3  = iOffsetTop + 3*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas3 = 0;

        nLeftButton4 = iOffsetLeft; 
        nTopButton4  = iOffsetTop + 3*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas4 = iOffsetLeft;
        nTopCanvas4  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas4 = 0;

        nLeftButton5 = iOffsetLeft; 
        nTopButton5  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas5 = iOffsetLeft;
        nTopCanvas5  = iOffsetTop + 5*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas5 = 0;
    }
    else if (g_nButtonLastClicked == 2)
    {
        nLeftButton1 = iOffsetLeft; 
        nTopButton1  = iOffsetTop; 
        nLeftCanvas1 = iOffsetLeft;
        nTopCanvas1  = iOffsetTop + g_iButtonHeight; 
        nHeightCanvas1 = 0; 

        nLeftButton2 = iOffsetLeft; 
        nTopButton2  =iOffsetTop + g_iButtonHeight; 
        nLeftCanvas2 = iOffsetLeft;
        nTopCanvas2  = iOffsetTop + 2*g_iButtonHeight; 
        nHeightCanvas2 = g_iWindowCanvasHeight;

        nLeftButton3 = iOffsetLeft; 
        nTopButton3  = iOffsetTop + 2*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas3 = iOffsetLeft;
        nTopCanvas3  = iOffsetTop + 3*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas3 = 0;

        nLeftButton4 = iOffsetLeft; 
        nTopButton4  = iOffsetTop + 3*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas4 = iOffsetLeft;
        nTopCanvas4  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas4 = 0;

        nLeftButton5 = iOffsetLeft; 
        nTopButton5  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas5 = iOffsetLeft;
        nTopCanvas5  = iOffsetTop + 5*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas5 = 0;
    }
    else if (g_nButtonLastClicked == 3)
    {
        nLeftButton1 = iOffsetLeft; 
        nTopButton1  = iOffsetTop; 
        nLeftCanvas1 = iOffsetLeft;
        nTopCanvas1  = iOffsetTop + g_iButtonHeight; 
        nHeightCanvas1 = 0; 

        nLeftButton2 = iOffsetLeft; 
        nTopButton2  =iOffsetTop + g_iButtonHeight; 
        nLeftCanvas2 = iOffsetLeft;
        nTopCanvas2  = iOffsetTop + 2*g_iButtonHeight; 
        nHeightCanvas2 = 0;

        nLeftButton3 = iOffsetLeft; 
        nTopButton3  = iOffsetTop + 2*g_iButtonHeight; 
        nLeftCanvas3 = iOffsetLeft;
        nTopCanvas3  = iOffsetTop + 3*g_iButtonHeight; 
        nHeightCanvas3 = g_iWindowCanvasHeight;

        nLeftButton4 = iOffsetLeft; 
        nTopButton4  = iOffsetTop + 3*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas4 = iOffsetLeft;
        nTopCanvas4  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas4 = 0;

        nLeftButton5 = iOffsetLeft; 
        nTopButton5  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas5 = iOffsetLeft;
        nTopCanvas5  = iOffsetTop + 5*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas5 = 0;
    }
    else if (g_nButtonLastClicked == 4)
    {
        nLeftButton1 = iOffsetLeft; 
        nTopButton1  = iOffsetTop; 
        nLeftCanvas1 = iOffsetLeft;
        nTopCanvas1  = iOffsetTop + g_iButtonHeight; 
        nHeightCanvas1 = 0; 

        nLeftButton2 = iOffsetLeft; 
        nTopButton2  =iOffsetTop + g_iButtonHeight; 
        nLeftCanvas2 = iOffsetLeft;
        nTopCanvas2  = iOffsetTop + 2*g_iButtonHeight; 
        nHeightCanvas2 = 0;

        nLeftButton3 = iOffsetLeft; 
        nTopButton3  = iOffsetTop + 2*g_iButtonHeight; 
        nLeftCanvas3 = iOffsetLeft;
        nTopCanvas3  = iOffsetTop + 3*g_iButtonHeight; 
        nHeightCanvas3 = 0;

        nLeftButton4 = iOffsetLeft; 
        nTopButton4  = iOffsetTop + 3*g_iButtonHeight; 
        nLeftCanvas4 = iOffsetLeft;
        nTopCanvas4  = iOffsetTop + 4*g_iButtonHeight; 
        nHeightCanvas4 = g_iWindowCanvasHeight;

        nLeftButton5 = iOffsetLeft; 
        nTopButton5  = iOffsetTop + 4*g_iButtonHeight + g_iWindowCanvasHeight; 
        nLeftCanvas5 = iOffsetLeft;
        nTopCanvas5  = iOffsetTop + 5*g_iButtonHeight + g_iWindowCanvasHeight; 
        nHeightCanvas5 = 0;
    }
    else if (g_nButtonLastClicked == 5)
    {
        nLeftButton1 = iOffsetLeft; 
        nTopButton1  = iOffsetTop; 
        nLeftCanvas1 = iOffsetLeft;
        nTopCanvas1  = iOffsetTop + g_iButtonHeight; 
        nHeightCanvas1 = 0; 

        nLeftButton2 = iOffsetLeft; 
        nTopButton2  =iOffsetTop + g_iButtonHeight; 
        nLeftCanvas2 = iOffsetLeft;
        nTopCanvas2  = iOffsetTop + 2*g_iButtonHeight; 
        nHeightCanvas2 = 0;

        nLeftButton3 = iOffsetLeft; 
        nTopButton3  = iOffsetTop + 2*g_iButtonHeight; 
        nLeftCanvas3 = iOffsetLeft;
        nTopCanvas3  = iOffsetTop + 3*g_iButtonHeight; 
        nHeightCanvas3 = 0;

        nLeftButton4 = iOffsetLeft; 
        nTopButton4  = iOffsetTop + 3*g_iButtonHeight; 
        nLeftCanvas4 = iOffsetLeft;
        nTopCanvas4  = iOffsetTop + 4*g_iButtonHeight; 
        nHeightCanvas4 = 0;

        nLeftButton5 = iOffsetLeft; 
        nTopButton5  = iOffsetTop + 4*g_iButtonHeight; 
        nLeftCanvas5 = iOffsetLeft;
        nTopCanvas5  = iOffsetTop + 5*g_iButtonHeight; 
        nHeightCanvas5 = g_iWindowCanvasHeight;
    }

    MoveWindow(g_hwndButton1, nLeftButton1, nTopButton1, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
    MoveWindow(g_hwndCanvas1, nLeftCanvas1, nTopCanvas1, g_iButtonWidth, 
                                                          nHeightCanvas1, TRUE);

    MoveWindow(g_hwndButton2, nLeftButton2, nTopButton2, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
    MoveWindow(g_hwndCanvas2, nLeftCanvas2, nTopCanvas2, g_iButtonWidth, 
                                                          nHeightCanvas2, TRUE);

    MoveWindow(g_hwndButton3, nLeftButton3, nTopButton3, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
    MoveWindow(g_hwndCanvas3, nLeftCanvas3, nTopCanvas3, g_iButtonWidth, 
                                                          nHeightCanvas3, TRUE);

    MoveWindow(g_hwndButton4, nLeftButton4, nTopButton4, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
    MoveWindow(g_hwndCanvas4, nLeftCanvas4, nTopCanvas4, g_iButtonWidth, 
                                                          nHeightCanvas4, TRUE);

    MoveWindow(g_hwndButton5, nLeftButton5, nTopButton5, g_iButtonWidth, 
                                                         g_iButtonHeight, TRUE);
    MoveWindow(g_hwndCanvas5, nLeftCanvas5, nTopCanvas5, g_iButtonWidth, 
                                                          nHeightCanvas5, TRUE);
}

void MoveControls_Canvas1(int nVTimer, BOOL bMoveUp)
{
    int nLeft, nTop, nWidth, nHeight;

    nLeft = g_rcStatic11.left;
    nTop = g_rcStatic11.top;
    nWidth = g_rcStatic11.right - g_rcStatic11.left;
    nHeight = g_rcStatic11.bottom - g_rcStatic11.top;
    MoveWindow(g_hwndStatic11, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop,  
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic12.left;
    nTop = g_rcStatic12.top;
    nWidth = g_rcStatic12.right - g_rcStatic12.left;
    nHeight = g_rcStatic12.bottom - g_rcStatic12.top;
    MoveWindow(g_hwndStatic12, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic13.left;
    nTop = g_rcStatic13.top;
    nWidth = g_rcStatic13.right - g_rcStatic13.left;
    nHeight = g_rcStatic13.bottom - g_rcStatic13.top;
    MoveWindow(g_hwndStatic13, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStaticLink11.left;
    nTop = g_rcStaticLink11.top;
    nWidth = g_rcStaticLink11.right - g_rcStaticLink11.left;
    nHeight = g_rcStaticLink11.bottom - g_rcStaticLink11.top;
    MoveWindow(g_hwndStaticLink11, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + 
                                                   nTop, nWidth, nHeight, TRUE);
    nLeft = g_rcStatic14.left;
    nTop = g_rcStatic14.top;
    nWidth = g_rcStatic14.right - g_rcStatic14.left;
    nHeight = g_rcStatic14.bottom - g_rcStatic14.top;
    MoveWindow(g_hwndStatic14, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic15.left;
    nTop = g_rcStatic15.top;
    nWidth = g_rcStatic15.right - g_rcStatic15.left;
    nHeight = g_rcStatic15.bottom - g_rcStatic15.top;
    MoveWindow(g_hwndStatic15, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
}

void MoveControls_Canvas2(int nVTimer, BOOL bMoveUp)
{
    int nLeft, nTop, nWidth, nHeight;

    nLeft = g_rcStatic21.left;
    nTop = g_rcStatic21.top;
    nWidth = g_rcStatic21.right - g_rcStatic21.left;
    nHeight = g_rcStatic21.bottom - g_rcStatic21.top;
    MoveWindow(g_hwndStatic21, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic22.left;
    nTop = g_rcStatic22.top;
    nWidth = g_rcStatic22.right - g_rcStatic22.left;
    nHeight = g_rcStatic22.bottom - g_rcStatic22.top;
    MoveWindow(g_hwndStatic22, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic23.left;
    nTop = g_rcStatic23.top;
    nWidth = g_rcStatic23.right - g_rcStatic23.left;
    nHeight = g_rcStatic23.bottom - g_rcStatic23.top;
    MoveWindow(g_hwndStatic23, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic24.left;
    nTop = g_rcStatic24.top;
    nWidth = g_rcStatic24.right - g_rcStatic24.left;
    nHeight = g_rcStatic24.bottom - g_rcStatic24.top;
    MoveWindow(g_hwndStatic24, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
}

void MoveControls_Canvas3(int nVTimer, BOOL bMoveUp)
{
    int nLeft, nTop, nWidth, nHeight;

    nLeft = g_rcStatic31.left;
    nTop = g_rcStatic31.top;
    nWidth = g_rcStatic31.right - g_rcStatic31.left;
    nHeight = g_rcStatic31.bottom - g_rcStatic31.top;
    MoveWindow(g_hwndStatic31, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic32.left;
    nTop = g_rcStatic32.top;
    nWidth = g_rcStatic32.right - g_rcStatic32.left;
    nHeight = g_rcStatic32.bottom - g_rcStatic32.top;
    MoveWindow(g_hwndStatic32, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic33.left;
    nTop = g_rcStatic33.top;
    nWidth = g_rcStatic33.right - g_rcStatic23.left;
    nHeight = g_rcStatic33.bottom - g_rcStatic23.top;
    MoveWindow(g_hwndStatic33, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
    nLeft = g_rcStatic34.left;
    nTop = g_rcStatic34.top;
    nWidth = g_rcStatic34.right - g_rcStatic34.left;
    nHeight = g_rcStatic34.bottom - g_rcStatic34.top;
    MoveWindow(g_hwndStatic34, nLeft, bMoveUp ? nTop - nVTimer : nVTimer + nTop, 
                                                         nWidth, nHeight, TRUE);
}

void MoveControls_Canvas4(int nVTimer, BOOL bMoveUp)
{
    RECT rc;
    char szTextArea[30];
    char szNewTextArea[30];

    strcpy(szTextArea, g_sTextArea1.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea1_Dyn  = szNewTextArea;

    strcpy(szTextArea, g_sTextArea2.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea2_Dyn  = szNewTextArea;

    strcpy(szTextArea, g_sTextArea3.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea3_Dyn  = szNewTextArea;

    strcpy(szTextArea, g_sTextArea4.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea4_Dyn  = szNewTextArea;

    strcpy(szTextArea, g_sTextArea5.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea5_Dyn  = szNewTextArea;

    strcpy(szTextArea, g_sTextArea6.c_str());
    ParseToRectValues(szTextArea, rc);
    rc.top = rc.top - nVTimer;
    sprintf(szNewTextArea, "%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
    g_sTextArea6_Dyn = szNewTextArea;

    //MessageBox(0, szNewTextArea, g_sTextArea1.c_str(), 0);

    RedrawWindow(g_hwndCanvas4,  
            NULL /* invalidate entire client area */,
                      NULL /* invalidate entire client area */, RDW_INVALIDATE);
}