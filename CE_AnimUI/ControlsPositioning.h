int g_iButtonWidth = 300; // also canvas width
int g_iButtonHeight = 41; //28;
int g_iWindowCanvasHeight = 100; //70;

// Buttons.
BOOL g_bUsePushStyle = FALSE;
UINT g_nButtonSpeed = 1; // milliseconds (was 0, see USER_TIMER_MINIMUM, some posts 10ms for min)
int g_nSpeedFactor = 2; // 1 (single), 2 (double), 3 (LL wrong painting).

// Canvas background.
COLORREF g_clrCanvasBackground = RGB(250, 250, 250);
// RGB(230, 254, 205); light green
//RGB(248, 254, 205); lighter yellow 
//RGB(245, 255, 177); yellow
// RGB(104, 139, 221); WE's Search, blue
// RGB(100, 153, 0) MSB's green

DWORD g_dwHeaderExStyle = 0; // WS_EX_CLIENTEDGE
DWORD g_dwCanvasExStyle = 0; // WS_EX_CLIENTEDGE


// 1st canvas. Left, top, width, height are given on the right as comments.
RECT g_rcStatic11 = {15, 12, 285, 32};
RECT g_rcStatic12 = {15, 33, 220, 53};
RECT g_rcStaticLink11 = {220, 33, 280, 53}; 
RECT g_rcStatic13 = {275, 33, 290, 53}; 
RECT g_rcStatic14 = {15, 54, 285, 74}; 
RECT g_rcStatic15 = {15, 75, 285, 95};

// 2nd canvas. Left, top, width, height are given on the right as comments.
RECT g_rcStatic21 = {15, 12, 285, 32};
RECT g_rcStatic22 = {15, 33, 285, 53};
RECT g_rcStatic23 = {15, 54, 285, 74};
RECT g_rcStatic24 = {15, 75, 285, 95};

// 3d canvas. Left, top, width, height are given on the right as comments.
RECT g_rcStatic31 = {15, 12, 285, 32};
RECT g_rcStatic32 = {15, 33, 285, 53};
RECT g_rcStatic33 = {15, 54, 285, 74};
RECT g_rcStatic34 = {15, 75, 285, 95};

// 5th canvas. Left, top, width, height are given on the right as comments.
RECT g_rcStatic51 = {15, 12, 285, 32};
RECT g_rcStaticLink51 = {80, 33, 285, 53};
RECT g_rcStaticLink52 = {80, 54, 285, 74};
RECT g_rcStaticLink53 = {80, 75, 285, 95};
