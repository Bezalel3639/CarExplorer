#include "stdafx.h"
#include "Settings.h"
#include "CarExplorer.h"
#include <fstream> // ifstream

HIMAGELIST g_himlTB; 

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                                                 LPTSTR lpCmdLine, int nCmdShow)
{
    CRegisterProduct m_RP;
    MSG msg;
    HACCEL hAccelTable;
    TCHAR szUserHomeDir[MAX_PATH];
    TCHAR szAppDataDir[MAX_PATH];
    TCHAR szCEDataDir[MAX_PATH];

    lstrcpy(g_szAD_Version, "    Car Explorer�, version 1.03");
    g_nWV_HOffset = 20;
    g_nWV_VOffset = 60; 
    g_clrLV_InterlacedColor = RGB(240, 240, 240); // all LV
    g_clrWV_OuterBackground = RGB(128, 128, 128); 
    g_clrWV_InnerBackground = GetSysColor(COLOR_3DFACE); //RGB(255, 218, 117);  
    g_nWV_AnumUIRightOffset = 350; 
    g_nWV_CBPosLeft = 45; 
    g_nWV_CBBottomOffset = 40; 
    // Used once only to get initial (startup) height of inner area. 
    g_bWV_FirstPaint = TRUE; // do not change
    g_iUnits = DEFAULT_UNITS;
    // The value "-1" indicates scenario CE is installed recently and still
    // no column was clicked, meaning no sort.
    g_iSortParam = -1; // unsorted LV columns 
    g_iSortColumn_Prev = -1; // none

    if (g_bUseKagiPaymentProcessing)
    {
        // Verify registration once (and avoid checking registration multiple 
        // times throughout the code).
        if (m_RP.IsRegistered()) 
        {
            g_bRegisteredCopy = TRUE;
            lstrcpy(g_szAD_Version, "Car Explorer PRO�, version 1.03");
        }
    }

    // Set folder for car collections (can be CAR and MSS files) to default 
    // taking in account that "My Documents" on XP is "Documents" on Vista and 
    // 7. Notice that loaded registry data (LoadRegistryData call must not 
    // preceed this call) if exist override defaut value.

    // Retrive from environment current user name.
    ExpandEnvironmentStrings("%userprofile%", szUserHomeDir, 
                                                         sizeof(szUserHomeDir));

    // Get summary and detailed OS version.
    g_nOSVersion = GetOSVersion(g_dwMajorVersion, g_dwMinorVersion);
    
    if (g_dwMajorVersion > 6 || (g_dwMajorVersion == 6 /* Vista family */ 
                                               && g_dwMinorVersion > 1 /* 7 */))
    {
        // Use Window 7 settings for next OS.
        wsprintf(g_szCarFilesFolder, "%s\\%s", szUserHomeDir, 
                                                            CAR_FILES_FOLDER_7);
    }
    else if (g_dwMajorVersion == 6 /* Vista family */ 
                                       && g_dwMinorVersion == 1 /* Windows 7 */)
    {
        wsprintf(g_szCarFilesFolder, "%s\\%s", szUserHomeDir, 
                                                            CAR_FILES_FOLDER_7);
    }
    else if (g_dwMajorVersion == 6 /* Vista family  */ 
                                           && g_dwMinorVersion == 0 /* Vista */)
    {
        
        wsprintf(g_szCarFilesFolder, "%s\\%s", szUserHomeDir, 
                                                        CAR_FILES_FOLDER_VISTA);
    }
    else if (g_nOSVersion <= 5 /* Windows XP or less */)
    {
        wsprintf(g_szCarFilesFolder, "%s\\%s", szUserHomeDir, 
                                                           CAR_FILES_FOLDER_XP);
    }

    // Load settings from registry.
    LoadRegistryData();

    // Create the default directory for CAR files if it does not exist. 
    if (GetFileAttributes(g_szCarFilesFolder) == INVALID_FILE_ATTRIBUTES)
    {
        // In fact, the directory created only on the first run after install.
        CreateDirectory(g_szCarFilesFolder, NULL);
    }

    // Create application data directory for CE.
    ExpandEnvironmentStrings("%appdata%", szAppDataDir, sizeof(szAppDataDir));
    wsprintf(szCEDataDir, "%s%s", szAppDataDir, "\\Car Explorer");

    if (GetFileAttributes(szCEDataDir) == INVALID_FILE_ATTRIBUTES)
    {
        CreateDirectory(szCEDataDir, /* security */ NULL); 
    }
    
    // Unpack g_iSortParam after reading from registry.
    if (g_iSortParam != -1 /* not sorted */)
    {
        g_nSortColumn = LOWORD(g_iSortParam);
        g_bSortAscending = (BOOL)HIWORD(g_iSortParam);
        g_iSortColumn_Prev = g_nSortColumn;

        TCHAR szTest[100];
        wsprintf(szTest, "%d %d", g_iSortColumn_Prev, g_nSortColumn); 
        //MessageBox(0, szTest, "Init", 0);
    }
    //MessageBox(0, g_szCarFilesFolder, CAR_FILES_FOLDER, 0);
    

    //App_XMLReadSettings();
    //App_LoadRegistrySettings();

    // __argv[0]: always program name (with path) - not used 
    // __argv[1]: 1st argument <filename>.mss with path (__argc == 2), 
    //            or /b - Browse or /e - Extract (__argc == 3)
    // __argv[2]: 2nd argument is <filename>.mss with path) (__argc == 3)

    // The full file name (__argv[1] when __argc == 2  or __argv[2] when 
    // __argc == 3) is copied to the global variable g_szFullFileName to provide 
    // that the global can also be used to store full file name obtained with 
    // File/Open... The g_szFullFileName variable must be initialized before 
    // main window creation for it is used with extraction function also.

    // Entry A: called as EXE (double click).
    if (__argc == 1 && 
         lstrlen(__argv[1]) == 0 /* 1st argument is empty when called as EXE */)
    {
        // Initialize global strings
        LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
        LoadString(hInstance, IDS_MB_APPTITLE, szMBTitle, MAX_LOADSTRING);
        LoadString(hInstance, IDR_MENU, szWindowClass, MAX_LOADSTRING);
                                                                
        MyRegisterClass(hInstance);

        // Perform application initialization.
        if (!InitInstance (hInstance, nCmdShow)) 
        {
            return FALSE;
        }
    }

    // Entry B: double-click on CAR or MSS file.
    if (__argc == 2 && IsMSSFile(__argv[1]))
    {
        // Set variable to indicate that CE is launched with DC.
        g_bCollectionFileDBClicked = TRUE; 

        lstrcpy(g_szFullFileName, __argv[1]);

        TCHAR szCurDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, szCurDir);
        //MessageBox(0, g_szCarFilesFolder, szCurDir, 0); 
        if (lstrcmp(szCurDir, g_szCarFilesFolder) == 0 /* equal */)
            g_bShowTV = TRUE; 
        else
        {
            g_bShowTV = FALSE; 
            g_nRightPaneViewType = 3; // DC on CAR file (out)
        }

        // Initialize global strings
        LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
        LoadString(hInstance, IDR_MENU, szWindowClass, MAX_LOADSTRING);
        MyRegisterClass(hInstance);

        // Perform application initialization:
        if (!InitInstance (hInstance, nCmdShow)) 
        {
            return FALSE;
        }
    }

    // Entry C: called from right-click context menu ("Browse").
    if (__argc == 3 && lstrcmp(__argv[1], "/b") == 0)
    {
        lstrcpy(g_szFullFileName, __argv[2]);
        
        // Initialize global strings.
        LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
        LoadString(hInstance, IDR_MENU, szWindowClass, 
                                                                MAX_LOADSTRING);
        MyRegisterClass(hInstance);

        // Perform application initialization.
        if (!InitInstance (hInstance, nCmdShow)) 
        {
            return FALSE;
        }
    }

    // Entry D: called from right-click for extraction of images ("Extract").
    if (__argc == 3 && lstrcmp(__argv[1], "/e") == 0)
    {
        lstrcpy(g_szFullFileName, __argv[2]);
        
        DialogBox(hInstance, (LPCTSTR)IDD_EXTRACT, NULL, 
                                                      (DLGPROC)Extract_WndProc);
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDR_MENU);

    // Main message loop.
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW; <-- removes flickering for TV!
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_CONTEXTMENUHANDLER);
    wcex.hCursor		= LoadCursor (NULL /* essential to see cursor */, 
        (g_nRightPaneViewType != 3 /* DC on CAR file outside default folder */) 
                   ? MAKEINTRESOURCE(IDC_SIZEWE) : MAKEINTRESOURCE(IDC_SIZENS));
    wcex.hbrBackground	= (HBRUSH)(COLOR_3DFACE+1);;
    wcex.lpszMenuName	= (LPCTSTR)IDR_MENU;
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDR_MAINFRAME);

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    HMENU hSubMenu;

    LoadRegistryData();

    // Create event object to enable loading car data in the background without 
    //UI delays. Notice that threads are launched not here, but with WM_ACTIVATE 
    // so that UI shows up almost immediately. If threads were started from
    // InitInstance the results are unstable, but mostly main window shows up
    // considerably delayed downplaying the use of threads at all..

    g_hLoadDataCompleteEvent = CreateEvent(NULL, TRUE /* manual reset */, 
                                               FALSE /* none-signaled */, NULL);
    g_hTVAvailableEvent = CreateEvent(NULL, TRUE /* manual reset */, 
                                               FALSE /* none-signaled */, NULL);

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, szWindowClass, // <-- test for drag-and-drop TODO: remove if not relevent
    //hWnd = CreateWindow(szWindowClass, 
        szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	
               //CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
               100, 100, 960, 722, NULL, NULL, hInstance, NULL);
                                                 
    g_hWnd = hWnd;

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
   
    // Add toolbar.
    App_CreateToolBar();

    // Add status bar.
    hSubMenu = GetSubMenu(GetMenu(g_hWnd), 1);
    if (g_bShowStatusBar)
    {
        g_nSBHeightUI = g_nSBHeight;
        App_CreateStatusBar();
        CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND | MF_CHECKED);

        // Show collection information in the status bar.
        if (g_nRightPaneViewType == 3 /* DC on CAR file (out) */)
            SB_ShowCollectionInfo(g_szFullFileName);
    }
    else
    {
        g_nSBHeightUI = 0;
        CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, 
                                                   MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Shows correctly TV when SB is not available.
    UpdateLayout();

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    DWORD dwExStyles;
    HMENU hMenu, hSubMenu;  
    HWND hwndTB;
    HTREEITEM hRootItem;
    TCHAR szBuffer[100];
    TVSORTCB tvsort;

    switch (message) 
    {
    case WM_CREATE:
        // Center application window.
        CenterWindow2(hWnd, GetDesktopWindow() /* relative to desktop */); 

        // Run splash screen and destroy it after 2 sec.
        RunSplashScreen();
        SetTimer(NULL /* windowless */, NULL /* ignored */, 
                               2000 /* milliseconds */, SplashScreen_TimerProc);

        wsprintf(szBuffer, "WM_CREATE: %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);

        // Load cursor for right host window used for viewing single MSS files.
        hCursorNS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
        //LoadRegistryData();

        if (g_bShowTV)
        {
        //CreateWindowEx(WS_EX_CLIENTEDGE,
        hwndTopArea = CreateWindow("#32770", "", WS_CHILD | WS_VISIBLE, 
                0, 0, 0, 0, hWnd, //300, 10
                              (HMENU)IDC_TOPAREA /* does not work */, hInst, 0); 

        // The windows procedure of #32770 dialog is subclassed since the 
        // original windows is hidden and LL no way to catch messages from 
        // combobox it hosts.
        g_wpOldDialogProc = (WNDPROC)SetWindowLong(hwndTopArea, GWL_WNDPROC, 
            (DWORD)DialogProc_SubclassedFunc);


        hwndComboBox = CreateWindow("COMBOBOX", "", WS_CHILD | WS_VISIBLE | 
            CBS_DROPDOWNLIST, 10, 30, 300, 104, hwndTopArea, 
                             (HMENU)IDC_COMBOBOX /* does not work */, hInst, 0);
        App_SetFont(hwndComboBox);
        SendMessage(hwndComboBox,  CB_ADDSTRING, 0 /* must be zero */, 
                                                     (LPARAM)"All (no groups)");
        SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)"Year");  
        SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)"Make");
        SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)"Country"); 
        SendMessage(hwndComboBox, CB_SETCURSEL, g_nGroup, 0 /* must be zero */);

        wsprintf(szBuffer, "CreateWindowEx (called): %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);
        hwndBrowser = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, 
            WC_TREEVIEW, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
            TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS 
            | TVS_EDITLABELS, 0, 0, 0, 0, hWnd, 
                             (HMENU)IDC_TREEVIEW /* does not work */, hInst, 0);
                                                                             
        //DragAcceptFiles(hwndBrowser, TRUE); // TODO: LL redundant (same as WS_EX_ACCEPTFILES)
        InitImageLists(hwndBrowser);
        wsprintf(szBuffer, "EnumerateCarFiles (Enter): %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);
        EnumerateCarFiles();
        PopulateTreeView(hwndBrowser, g_nGroup /* group by country by default */);
        wsprintf(szBuffer, "PopulateTreeView (Exit): %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);
        
        // Sort items under "All" root item.
        hRootItem = TreeView_GetRoot(hwndBrowser);
        if (g_nGroup != 1 /* group is not year*/)
                TreeView_SortChildren(hwndBrowser, hRootItem, FALSE);
        else if (g_nGroup == 1 /* sort by year */)
        {
            tvsort.hParent = hRootItem;
            tvsort.lpfnCompare = TV_CompareProc;			
            TreeView_SortChildrenCB(hwndBrowser, &tvsort, 0 /* must be zero */);
        }

        SetEvent(g_hTVAvailableEvent);
        wsprintf(szBuffer, "WM_CREATE (SetEvent): %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);

        hwndMainLV = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, 
            WC_LISTVIEW , "Main LV", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | 
            LVS_REPORT, 0, 0, 0, 0, // TODO: verify the use of  LVS_SORTDESCENDING/ LVS_SORTASCENDING, remove if redundant
                                      hWnd, (HMENU)IDC_MAIN_LISTVIEW, hInst, 0);

        // Works with CreateWindow-created (without Ex) window.
        dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
        ListView_SetExtendedListViewStyle(hwndMainLV, dwExStyles);

        //Add columns. Notice that LV is populated after threads load CAR data.
        InitListViewColumns(hwndMainLV);
        }
        else
        {
            InitSingleCARFileViewer(hWnd);
            SendMessage(g_hWndSplashScreen, WM_CLOSE, 0, 0);
            ReleaseCapture();

            // Disable always in CE v1.0 File/Open... menu for the single CAR 
            // mode.
            hMenu = GetMenu(hWnd);
            hSubMenu = GetSubMenu(hMenu, 0);
            EnableMenuItem(hSubMenu, ID_FILE_OPEN, MF_BYCOMMAND | MF_GRAYED);
        }

        // When loading with threads, click on "+" of Favotites causes LV
        // polulation at the the very CE start (TVN_SELCHANGED is sent to TV). 
        // To avoid this, root item is specifically selected.
        //TreeView_SelectItem(hwndBrowser, TreeView_GetRoot(hwndBrowser));

        //if (g_bShowStartupWelcomeView)
        if (g_nRightPaneViewType != 3 /* always supress WV when DC on CAR file 
                      out of the default folder */ && g_bShowStartupWelcomeView)
        {
            g_nRightPaneViewType = 0;
            // Remove WS_VISIBLE temporary WS_VISIBLE flag. LL OS checks actual 
            // visibility of a window so that when WS_VISIBLE flag is set and 
            // window created, but not yet activated IsWindowVisible, which 
            // normally used in App_OnToolbarSearch, fails.
            ShowWindow(hwndMainLV, FALSE); 
            // At this point g_hWnd is NULL so that hWnd passed with WM_CREATE 
            // should be used. 
            App_OnToolbarSearch(hWnd);
        }

        break;
    case WM_ACTIVATE: 
        // TODO: 2nd event likely redundant (WM_ACTIVATE already says that UI is available)
        //SetEvent(g_hTVAvailableEvent);
        // Don't launch on deactivation.
        //if (wParam == WA_ACTIVE) MainWindow_ActivateHandler();
        //if (wParam != WA_INACTIVE && wParam != WA_CLICKACTIVE) MainWindow_ActivateHandler();
        // TODO: suppress when minimized/maximized
        //if (wParam != WA_INACTIVE) MainWindow_ActivateHandler();
        if (!g_bCarDataLoaded)
        {
            MainWindow_ActivateHandler();
            // The data load must be initiated only once, but WM_ACTIVATE is 
            // called also on minimaze/maximaze (unclear how to identify), 
            // deactivate so using g_bCarDataLoaded fits this purpose.
            g_bCarDataLoaded = TRUE; // TODO: verify logic since
                                     // MainWindow_ActivateHandler returns 
                                     // immediately and data are not yet loaded
        }

        break;
    case WM_SETCURSOR: // not fired when SetCapture is used
        // Make sure that WE cursor is shown for the splitter. Note that for 
        // some unknown reason "return 0" does not provide that and WE cursor 
        // is shown for splitter as in the case if "break" statement is used. 
        // TODO: figure why is it so and remove working, but unusual here 
        // DefWindowProc call ("break" or "result 0" should work).
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_LBUTTONDOWN:
        //Splitter_OnLButtonDown(hWnd, message, wParam, lParam);
        if (g_nRightPaneViewType == 3)
            Splitter2_OnLButtonDown(hWnd, message, wParam, lParam);
        else
            Splitter_OnLButtonDown(hWnd, message, wParam, lParam);
        break;
    case WM_LBUTTONUP:
        // Handle DAD when LV item is dropped.
        if (g_bDADActive) 
        {
            HWND hWFP;
            HWND hwndLV;
            // Get client coordinates for main window.
            POINT pt = {LOWORD(lParam), HIWORD(lParam)}; 

            POINT ptCopy = pt;
            // Convert client coordinates relative to main window to those of TV.
            ClientToScreen(hWnd, &ptCopy);
            ScreenToClient(hwndBrowser, &ptCopy);

            g_bDADActive = FALSE;
            
            ClientToScreen(hWnd, &pt);
            hWFP = WindowFromPoint(pt);

            // When WM_LBUTTONUP occurs over TV, add LV item to the Favorites.
            if (hWFP == hwndBrowser) 
            {	
                // Figure what LV is used.
                if (IsWindowVisible(hwndMainLV)) 
                    hwndLV = hwndMainLV;
                else if (IsWindowVisible(g_hwndLV)) 
                    hwndLV = g_hwndLV;		
                
                // Add item under favorites in the tree view.
                AddDroppedItem(hwndLV /* drag originator */, &ptCopy);
            }

            // DAD operations are closed. Stop capturing mouse.
            ReleaseCapture();

            // Restore class cursor (WE). Notice that returning zero DNW.
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        //Splitter_OnLButtonUp(hWnd, message, wParam, lParam);
        if (g_nRightPaneViewType == 3)
            Splitter2_OnLButtonUp(hWnd, message, wParam, lParam);
        else
            Splitter_OnLButtonUp(hWnd, message, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        // Handle DAD when LV item is moved over windows.
        if (g_bDADActive) 
        {
            HWND hWFP ;
            POINT pt = {LOWORD(lParam), HIWORD(lParam)}; // client coordinates

            ClientToScreen(hWnd, &pt);
            hWFP = WindowFromPoint(pt);

            if (hWFP == hwndMainLV || hWFP == g_hwndLV) // cursor over the LV
            {
                SetCursor(LoadCursor(hInst, 
                                       MAKEINTRESOURCE(IDC_CURSOR_DAD_NODROP)));

                return 1; // halt further processing to avoid class cursor
            }
            else if (hWFP == g_hWnd) // cursor moves over the splitter
            {
                SetCursor(LoadCursor(hInst, 
                                       MAKEINTRESOURCE(IDC_CURSOR_DAD_NODROP)));

                // Halt further processing while WM_MOUSEMOVE over the splitter 
                // so that class cursor is not restored.
                return 1; 
            }
            else if (hWFP == hwndBrowser) // cursor moves over the TV
            {
                SetCursor(LoadCursor(hInst, 
                                         MAKEINTRESOURCE(IDC_CURSOR_DAD_DROP)));

                return 1; // halt further processing to avoid class cursor
            }
        }

        //Splitter_OnMouseMove(hWnd, message, wParam, lParam);
        if (g_nRightPaneViewType == 3)
            Splitter2_OnMouseMove(hWnd, message, wParam, lParam);
        else 
            Splitter_OnMouseMove(hWnd, message, wParam, lParam);
        break;
    case WM_GETMINMAXINFO: // sent before actual resizing, WM_SIZE
        // Limit the width of WV.
        if (g_nRightPaneViewType == 0 /* welcome view */)
        {
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
            pmmi->ptMinTrackSize.x = WV_APPWINDOW_WIDTH_MIN;
        }
        return 0; // message is processed
    case WM_SIZE: // window is already resized
        // Resize rebar control.
        MoveWindow(g_hwndReBar, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE); 

        //SizeWindowContents(LOWORD(lParam), 
        //	                    HIWORD(lParam) /* new height of client area */);
        if (g_nRightPaneViewType == 3)
            SizeWindowContents2(LOWORD(lParam), 
                                HIWORD(lParam) /* new height of client area */);
        else
            SizeWindowContents(LOWORD(lParam), 
                                HIWORD(lParam) /* new height of client area */);
    
        // This adjusts status bar to the bottom (from MConsole). TODO: figure 
        // out the meaning of 500, 100.
        SendMessage(g_hwndStatus, WM_SIZE, (WPARAM) 500, (LPARAM) 100); 
        break;
    case WM_NOTIFY: 
        
        if (((LPNMHDR)lParam)->idFrom == IDC_TOOLBAR && 
                                   ((LPNMHDR)lParam)->code == TBN_HOTITEMCHANGE)
        {
            NMTBHOTITEM* pnmhi = (NMTBHOTITEM*)lParam;
            //pnmhi->
            //TB_SETHOTIMAGELIST 
            //SendMessage(g_hwndToolbar, TB_SETHOTIMAGELIST, 0 /* must be zero */, 
            //	(LPARAM) (HIMAGELIST) himlNewHot; );   
            //MessageBox(0, "dasdas", "", 0);
        }

        //if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW /* list view ID */ && 
        if (((LPNMHDR)lParam)->idFrom == IDC_MAIN_LISTVIEW /* list view ID */ && 
                                            ((LPNMHDR)lParam)->code == NM_DBLCLK) 
        {
            HWND hwndLV = ((LPNMHDR)lParam)->hwndFrom; 
            LV_Notify_OnDoubleClick(hwndLV);
        }

        if (((LPNMHDR)lParam)->idFrom == IDC_MAIN_LISTVIEW /* list view ID */  
                                    && ((LPNMHDR)lParam)->code == LVN_BEGINDRAG) 
        {
            NMLISTVIEW* pnmlv = (NMLISTVIEW*)lParam;
            char szText[10];

            // Ignore DAD if the Favorites folder is selected.
            if (IsFavoritesSelected()) 
            {
                MessageBox(0, 
                    "You cannot add a favorite from the list of favorites!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
                return 1; // LVN_BEGINDRAG is completely handled
            }

            // Get hit row (or with ListView_GetNextItem(hListView, -1, 
            //LVNI_SELECTED)).
            g_iLVDroppedItem = pnmlv->iItem;
            sprintf(szText, "%d", pnmlv->iItem);
            //MessageBox(0, szText, "Test", 0); // OK
            g_bDADActive = TRUE;
            SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR_DAD_NODROP))); // LL DNM
            SetCapture(g_hWnd); // hwndMainLV

            POINT pt;
            POINT ptUpLeft = {0, 0}; // LL arbitrary initialization 
            int nOffset = 10; // offset for drag image (up and to the left)
            int nSelectedImage = 1; // drag image
            
            // The use of drag image is reserved. It is not used in CE v1.0.
            // Instead different cursors indicate whether drop is allowed over 
            // the LV, splitter and TV.
            // MSDN: ptUpLeft are coordinates of "the initial location of the 
            // upper-left corner of the image, in view coordinates". 
            //hilDrag = ListView_CreateDragImage(hwndMainLV, pnmlv->iItem, &pt); //nSelectedImage, ptUpLeft
            
            // MSND: Hotspot "x-coordinate [y-coordinate] of the location of the 
            // drag position relative to the upper-left corner of the image". 
            //ImageList_BeginDrag(hilDrag, 
            //	nSelectedImage /* index of the image to drag */, 
            //	                          0 /* x-hotspot */, 0 /* y-hotspot */);
            
            // Get client coordinates where click occured (MSDN does not say specifically 
            // that ptAction are client coordinates, but presumably it is so).
            pt = ((NMLISTVIEW*) ((NMHDR*)lParam))->ptAction; 
            ClientToScreen(hwndMainLV, &pt);
            // The coordinates are screen coordinates. "The coordinate [x or y] 
            // is relative to the upper-left corner of the window, not the 
            // client area" (MSDN). 
            ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);  //
            ImageList_DragShowNolock(FALSE);
        }

        if (((LPNMHDR)lParam)->idFrom == IDC_TREEVIEW) 
        {
            OnNotify_TVItemClick(hWnd, lParam);
        }

        if (((LPNMHDR)lParam)->idFrom == IDC_MAIN_LISTVIEW) 
        {
            //g_hwndCurLV = hwndMainLV;
            //return ListView_NotifyHandler(hWnd, message, wParam, lParam);
            return ListView_NotifyHandler(((LPNMHDR)lParam)->hwndFrom, message, 
                                                                wParam, lParam);
        }

        if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW /* DC on CAR file (out) */) 
        {
            return ListView_NotifyHandler(((LPNMHDR)lParam)->hwndFrom, message, 
                                                                wParam, lParam);
        }

        // Handle LV click if DC on CAR file (out). 
        if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW &&
                                            ((LPNMHDR)lParam)->code == NM_CLICK)
        {
            short iImage;

            iImage = ListView_GetNextItem(g_hwndLV, -1, LVNI_SELECTED);

            if (iImage != -1 /* click out of image */) 
            {
                LoadPictureZoomWindow(iImage);

                // Re-paint window to see update (reqired).
                InvalidateRect(hwndPictureView, NULL, TRUE);
                UpdateWindow(hwndPictureView);

                LoadXMLData(iImage);
            }
        }
        break;
    //case WM_DROPFILES:
    //	MessageBox(0, "", "WM_DROPFILES", 0);
    //	break;
    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_FILE_SETFOLDER:
            if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */) break;

            DialogBox(hInst, (LPCTSTR)IDD_SETFOLDER, NULL, // TODO: see if NULL OK - centerring, etc.
                                                    (DLGPROC)SetFolder_WndProc);
            break;
        case ID_FILE_OPEN:
            OnFileOpen();
            break;
        // TODO: enable
        case IDM_VIEW_TOOLBAR:
            hMenu = GetMenu(g_hWnd);
            //if (hMenu) MessageBox(0, "OK", "hMenu", 0);
            hSubMenu = GetSubMenu(hMenu, 1);
            //if (hSubMenu) MessageBox(0, "OK", "GetSubMenu", 0);
            // TODO: g_hwndTB does not work, g_hwndStatus - OK.
            //hwndTB = GetDlgItem(g_hWnd, IDC_TOOLBAR);
            g_hwndReBar = GetDlgItem(g_hWnd, IDC_REBAR);
            //if (g_hwndReBar) MessageBox(0, "OK", "", 0);
            if (IsWindowVisible(g_hwndReBar))
            {
                g_nTBHeightUI = 0; 
                ShowWindow(g_hwndReBar, SW_HIDE); 
                CheckMenuItem(hSubMenu, IDM_VIEW_TOOLBAR, 
                                                    MF_BYCOMMAND | MF_UNCHECKED);
                g_bShowToolBar = FALSE; // used for persistence
                // TODO: simulate WM_SIZE
                //SendMessage(g_hwndStatus, WM_SIZE, (WPARAM) 500, (LPARAM) 200); 
            }
            else
            {
                g_nTBHeightUI = g_nTBHeight; //44; 
                ShowWindow(g_hwndReBar, SW_SHOW);
                CheckMenuItem(hSubMenu, IDM_VIEW_TOOLBAR, 
                                                        MF_BYCOMMAND | MF_CHECKED);
                g_bShowToolBar = TRUE; // used for persistence
                // TODO: simulate WM_SIZE
                //SendMessage(g_hWnd, WM_SIZE, (WPARAM) 0, (LPARAM) 0); 
            }

            // Adjust relative postions of windows.
            UpdateLayout();

            break;
        case IDM_VIEW_STATUSBAR:
            hMenu = GetMenu(g_hWnd);
            hSubMenu = GetSubMenu(hMenu, 1);
            
            if (IsWindowVisible(g_hwndStatus))
            {
                g_nSBHeightUI = 0;
                ShowWindow(g_hwndStatus, SW_HIDE);
                CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, 
                                                   MF_BYCOMMAND | MF_UNCHECKED);
                g_bShowStatusBar = FALSE; // used for persistence
            }
            else
            {
                g_nSBHeightUI = g_nSBHeight;
                if (g_hwndStatus)
                    ShowWindow(g_hwndStatus, SW_SHOW);   
                else
                    App_CreateStatusBar();

                CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, 
                                                     MF_BYCOMMAND | MF_CHECKED); 
                g_bShowStatusBar = TRUE; // used for persistence
            }

            // Adjust relative postions of windows.
            UpdateLayout();

            break;
        case IDM_TOOLS_DOWNLOAD:
            //if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */) break;

            DialogBox(hInst, (LPCTSTR)IDD_LOAD_IMAGE_SETS, hWnd, 
                                                        (DLGPROC)LoadImageSets);
            break;
        case IDM_TOOLS_EXPLORER_FOLDER:
        {
            TCHAR szOpenFolderCommand[MAX_PATH];
            wsprintf(szOpenFolderCommand, "%s%s%", "explorer.exe ", 
                                                            g_szCarFilesFolder);
            WinExec(szOpenFolderCommand, TRUE);
            
            break;
         }
        case IDM_TOOLS_SETTINGS:
            App_Settings();
            break;
        case IDM_HELP_ACTIVATEPRODUCT:
            DialogBox(hInst, (LPCTSTR)IDD_REGISTER_PRODUCT, hWnd, 
                                              (DLGPROC)RegisterProduct_WndProc);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            break;
        case ID_TOOLBAR_SEARCH:
            if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */) break;

            g_nRightPaneViewType = 0;
            App_OnToolbarSearch(hWnd);

            // Disable TB icon when WV is active.
            SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                ID_TOOLBAR_VIEW, 
                MAKELPARAM(4 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));
            break;
        case ID_TOOLBAR_FAVORITES:
            if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */) break;

            // Select and expand Favorites folder.
            TreeView_SelectItem(hwndBrowser, g_htviFavorites); 
            TreeView_Expand(hwndBrowser, g_htviFavorites, TVE_EXPAND);
            // Scroll, if necessary, to ensure that root item is visible.
            TreeView_SelectSetFirstVisible(hwndBrowser, 
                                                 TreeView_GetRoot(hwndBrowser));
            break;
        case ID_TOOLBAR_VIEW:
            {
            TBBUTTONINFO tbbi;
            DWORD dwStyle;
            // cbSize & dwMask must be set before sending TB_GETBUTTONINFO 
            // message.
            tbbi.cbSize = sizeof(tbbi);
            tbbi.dwMask = TBIF_IMAGE;
            SendMessage(g_hwndTB, TB_GETBUTTONINFO, ID_TOOLBAR_VIEW, 
                                                                 (LPARAM)&tbbi);

            //TCHAR szTest[100];
            //wsprintf(szTest, "Last Error: %d", GetLastError()); 
            //MessageBox(0, szTest, "Test", 0);

            if (tbbi.iImage == 7) // icon view
            {
                SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                    ID_TOOLBAR_VIEW, 
                    MAKELPARAM(8 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));

                dwStyle = GetWindowLong(g_hwndLV, GWL_STYLE); 
                //if ((dwStyle & LVS_TYPEMASK) == LVS_ICON) 
                {
                    g_nCarFilesLayout = 0; // report view 
                    TCHAR szTitle[100];
                    GetImageSetTitle(g_szFullFileName, szTitle);
                    dwStyle &=~ LVS_ICON;  // remove style
                    dwStyle |= LVS_REPORT; // add style 
                    SetWindowLong(g_hwndLV, GWL_STYLE, dwStyle);

                    ListView_DeleteAllItems(g_hwndLV);
                    InitListViewColumns(g_hwndLV);
                    //InitListViewItems(g_hwndLV, szTitle /* filter */, 
                    //							   TRUE /* MSS file clicked */);
                    if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */)
                    {
                        m_vecCarData.clear();
                        LoadCarData_SingleFile();
                        InitListViewItems(g_hwndLV, NULL /* filter */, 
                                                    TRUE /* MSS file clicked */);
                    }
                    else
                    {
                        InitListViewItems(g_hwndLV, szTitle /* filter */, 
                                                   TRUE /* MSS file clicked */);
                    }
                }
            }
            else if (tbbi.iImage == 8) // report view
            {
                SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                    ID_TOOLBAR_VIEW, 
                    MAKELPARAM(7 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));

                dwStyle = GetWindowLong(g_hwndLV, GWL_STYLE); 
                //if((dwStyle & LVS_TYPEMASK) == LVS_REPORT) 
                {
                    g_nCarFilesLayout = 1; // list view with images 
                    dwStyle &=~ LVS_REPORT; // remove style
                    dwStyle |= LVS_ICON | LVS_AUTOARRANGE;    // add style 
                    SetWindowLong(g_hwndLV, GWL_STYLE, dwStyle);

                    ListView_DeleteAllItems(g_hwndLV);
                    LoadPicturesAndStrings(g_hwndLV);
                }
            }
            }

            break;
        //case ID_TOOLBAR_SETTINGS:
        //	DialogBox(hInst, (LPCTSTR)IDD_SETTINGS, NULL, 
        //		                                     (DLGPROC)Settings_WndProc);
  //          break;
        case IDM_HELP_WEBSITE:
            ShellExecute(NULL, "open", 
                             "http://www.carexplorer.org", 0, 0, SW_SHOWNORMAL);
            break;
        case IDM_HELP_PRODUCTWEBPAGE:
            ShellExecute(NULL, "open", 
                "http://www.carexplorer.org/Downloads/CE/Help/CE_Help.htm", 
                                                           0, 0, SW_SHOWNORMAL);
            break;
        case IDM_HELP_DOWNLOADPACK:
            ShellExecute(NULL, "open", 
                 "http://www.carexplorer.org/RegForm.htm", 0, 0, SW_SHOWNORMAL);
            break;
        case IDM_HELP_MAKECARFILE:
            ShellExecute(NULL, "open", 
                 "http://www.carexplorer.org/MakeCARFile.htm", 
                                                           0, 0, SW_SHOWNORMAL);
            break;
        case IDM_HELP_AFFILIATES:
            ShellExecute(NULL, "open", 
                   "http://www.carexplorer.org/Hello.htm", 0, 0, SW_SHOWNORMAL);
            break;
        case IDM_HELP_DONATE: // www.paypal.com's location
            ShellExecute(NULL, "open", 
                  "http://www.carexplorer.org/donate.htm", 0, 0, SW_SHOWNORMAL); 
            break;
        case ID_TOOLBAR_HELP:
            ShellExecute(NULL, "open", 
                "http://www.carexplorer.org/Downloads/CE/Help/CE_Help.htm", 
                                                           0, 0, SW_SHOWNORMAL);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_CONTEXTMENU:
        if (wParam == (WPARAM)hwndBrowser) TV_OnContextMenu(hwndBrowser);
        if (wParam == (WPARAM)hwndMainLV) LV_OnContextMenu(hwndMainLV);
        break;
    case WM_COPYDATA: // handle commands as signals from CE_Anim_UI.dll
        switch (((COPYDATASTRUCT*)lParam)->dwData)
        {
        case 0 /* download link is called in CE_AnimUI.dll */:
            DialogBox(hInst, (LPCTSTR)IDD_LOAD_IMAGE_SETS, hWnd, 
                                                        (DLGPROC)LoadImageSets);
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY:
        App_OnSaveSettings();
        wsprintf(szBuffer, "WM_DESTROY: %d", GetTickCount()); 
        if (g_bEnableDebug) App_Debug(szBuffer);
        PostQuitMessage(0);
        // MSDN: "If an application processes this message, it should return zero."
        return 0;
        //break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK Welcome_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     RECT rc;
    PAINTSTRUCT ps;
    HDC hdc;
    HWND hwndButton;
    HWND hwndStatic_Year, hwndStatic_Type, hwndStatic_Price, hwndStatic_Fuel; 
    HIMAGELIST hIL; 
    HICON hIcon;
    RECT rcMargin = {3,1,0,0};
    BUTTON_IMAGELIST bil;
    char szYear[6];
    SYSTEMTIME st;

    switch (message) 
    {
    case WM_CREATE:
        // Load picture from file (should bew in the same directory as EXE).
        //m_SinglePicture.Load("CE_Header.gif");
        // Load picture embedded to EXE. Note the this GIF adds only 3K.
        m_SinglePicture.Load(hInst, "CUSTOM", 
                                IDR_CUSTOM_GIF /* CE_Header.gif resource ID */); 

       // Add static text (year).
        hwndStatic_Year = CreateWindow("STATIC", NULL, WS_CHILD |WS_VISIBLE,
            113 + g_nWV_HOffset, 196 + g_nWV_VOffset,
                                               30, 30, hWnd, NULL, hInst, NULL);
        App_SetFont(hwndStatic_Year);
        SendMessage(hwndStatic_Year, WM_SETTEXT, 0 /* not used */, 
                                                               (LPARAM)"Year:");
        // Create combobox for Year.
        g_hwndWelcome_CBYear = CreateWindow("COMBOBOX", "", 
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 
                164 + g_nWV_HOffset, 193 + g_nWV_VOffset, 100, 104, hWnd,
                                          (HMENU)IDC_WELCOME_CB_YEAR, hInst, 0);
        App_SetFont(g_hwndWelcome_CBYear);
        SendMessage(g_hwndWelcome_CBYear,  CB_ADDSTRING, 0 /* must be zero */, 
                                                                 (LPARAM)"All");
        // Get current year and set options.
        GetLocalTime(&st); 
        sprintf(szYear, "%d", st.wYear);
        SendMessage(g_hwndWelcome_CBYear, CB_ADDSTRING, 0, (LPARAM)szYear); 
        SendMessage(g_hwndWelcome_CBYear, CB_SETCURSEL, 0 /* selection */, 
                                                          0 /* must be zero */);
        // Add static text (car type).
        hwndStatic_Type = CreateWindow("STATIC", NULL, WS_CHILD |WS_VISIBLE,
            113 + g_nWV_HOffset, 226 + g_nWV_VOffset,
                                               30, 30, hWnd, NULL, hInst, NULL);
        App_SetFont(hwndStatic_Type);
        SendMessage(hwndStatic_Type, WM_SETTEXT, 0 /* not used */, 
                                                               (LPARAM)"Type:");
        // Create combobox for Type.
        g_hwndWelcome_CBType = CreateWindow("COMBOBOX", "", 
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 
                164 + g_nWV_HOffset, 223 + g_nWV_VOffset, 100, 104, hWnd,
                                          (HMENU)IDC_WELCOME_CB_TYPE, hInst, 0);
        App_SetFont(g_hwndWelcome_CBType);
        SendMessage(g_hwndWelcome_CBType,  CB_ADDSTRING, 0 /* must be zero */, 
                                                                 (LPARAM)"All");
        //SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"Compact");
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"Sedan");  
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"Coupe");
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"Hatchback");
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"Wagon");
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, 
                                                         (LPARAM)"Convertible");
        SendMessage(g_hwndWelcome_CBType, CB_ADDSTRING, 0, (LPARAM)"SUV");
        SendMessage(g_hwndWelcome_CBType, CB_SETCURSEL, 0 /* g_nSearch_Type */, 
                                                          0 /* must be zero */);
        // Add static text (price).
        hwndStatic_Price = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE, 
            114 + g_nWV_HOffset, 255 + g_nWV_VOffset,
                                               30, 30, hWnd, NULL, hInst, NULL);
        App_SetFont(hwndStatic_Price);
        SendMessage(hwndStatic_Price, WM_SETTEXT, 0 /* not used */, 
                                                              (LPARAM)"Price:");
        // Create combobox for Price.
        g_hwndWelcome_CBPriceRange = CreateWindow("COMBOBOX", "", 
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 
            164 + g_nWV_HOffset, 253 + g_nWV_VOffset, 
            100, 200 /* enough to see all items */, hWnd, 
                                    (HMENU)IDC_WELCOME_CB_PRICERANGE, hInst, 0);
        App_SetFont(g_hwndWelcome_CBPriceRange);
        SendMessage(g_hwndWelcome_CBPriceRange,  CB_ADDSTRING, 
                                           0 /* must be zero */, (LPARAM)"All");
        SendMessage(g_hwndWelcome_CBPriceRange,  CB_ADDSTRING, 0, 
                                                              (LPARAM)"Custom");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                            (LPARAM)"< $5,000");  
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                  (LPARAM)"$5,000 ... $10,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$10,000 ... $15,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$15,000 ... $20,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$20,000 ... $25,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$25,000 ... $30,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$30,000 ... $40,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                 (LPARAM)"$40,000 ... $50,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                (LPARAM)"$50,000 ... $100,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_ADDSTRING, 0, 
                                                          (LPARAM)"> $100,000");
        SendMessage(g_hwndWelcome_CBPriceRange, CB_SETCURSEL, 
                                  0 /* g_nSearch_Type */, 0 /* must be zero */);
        //SendMessage(g_hwndWelcome_CBPriceRange, CB_SETMINVISIBLE, 11, 0 /* must be zero */);

        // Add static text (price, low boundary).
        g_hwndStatic_MinPrice = CreateWindow("STATIC", 
            NULL, WS_CHILD | WS_VISIBLE, 
            350 + g_nWV_HOffset, 255 + g_nWV_VOffset, 50, 30,
                                                       hWnd, NULL, hInst, NULL);
        App_SetFont(g_hwndStatic_MinPrice);
        SendMessage(g_hwndStatic_MinPrice, WM_SETTEXT, 0 /* not used */, 
                                                             (LPARAM)"Between");
        g_hwndEdit_MinPrice = CreateWindow("EDIT", NULL, 
            WS_CHILD | WS_VISIBLE | WS_BORDER, 
            410 + g_nWV_HOffset, 253 + g_nWV_VOffset, 50, 20,
                                                       hWnd, NULL, hInst, NULL);  
        App_SetFont(g_hwndEdit_MinPrice);
        // Set zero initial value to Max Price that corresponds to "All".
        SendMessage(g_hwndEdit_MinPrice, WM_SETTEXT, 0 /* not used */, 
                                                         (LPARAM)"0" /* All */);

        g_hwndStatic_MaxPrice = CreateWindow("STATIC", 
            NULL, WS_CHILD | WS_VISIBLE, 
            470 + g_nWV_HOffset, 255 + g_nWV_VOffset, 30, 30, 
                                                       hWnd, NULL, hInst, NULL);
        App_SetFont(g_hwndStatic_MaxPrice);
        SendMessage(g_hwndStatic_MaxPrice, WM_SETTEXT, 0 /* not used */, 
                                                                 (LPARAM)"and");
        g_hwndEdit_MaxPrice = CreateWindow("EDIT", NULL, 
            WS_CHILD | WS_VISIBLE | WS_BORDER, 
            500 + g_nWV_HOffset, 253 + g_nWV_VOffset, 50, 20, 
                                                       hWnd, NULL, hInst, NULL); 
        // Set big enough initial value to Max Price that corresponds to "All".
        SendMessage(g_hwndEdit_MaxPrice, WM_SETTEXT, 0 /* not used */, 
                                                (LPARAM)"1000000000" /* All */);
        App_SetFont(g_hwndEdit_MaxPrice);

        // Add static text (fuel type).
        hwndStatic_Fuel = CreateWindow("STATIC", NULL, WS_CHILD |WS_VISIBLE,
                                      114 + g_nWV_HOffset, 284 + g_nWV_VOffset, 
                                               30, 30, hWnd, NULL, hInst, NULL);
        App_SetFont(hwndStatic_Fuel);
        SendMessage(hwndStatic_Fuel, WM_SETTEXT, 0 /* not used */, 
                                                               (LPARAM)"Fuel:");
        // Create combobox for Fuel.
        g_hwndWelcome_CBFuel = CreateWindow("COMBOBOX", "", 
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            164 + g_nWV_HOffset, 283 + g_nWV_VOffset,
            100, 100 /* enough to see all items */, hWnd, 
                                      (HMENU)IDC_WELCOME_CB_FUELTYPE, hInst, 0);
        App_SetFont(g_hwndWelcome_CBFuel);
        SendMessage(g_hwndWelcome_CBFuel,  CB_ADDSTRING, 0 /* must be zero */, 
                                                                 (LPARAM)"All");
        SendMessage(g_hwndWelcome_CBFuel,  CB_ADDSTRING, 0 /* must be zero */, 
                                                            (LPARAM)"Gasoline");
        SendMessage(g_hwndWelcome_CBFuel,  CB_ADDSTRING, 0 /* must be zero */, 
                                                              (LPARAM)"Diesel");
        SendMessage(g_hwndWelcome_CBFuel,  CB_ADDSTRING, 0 /* must be zero */, 
                                                              (LPARAM)"Hybrid");
        SendMessage(g_hwndWelcome_CBFuel,  CB_ADDSTRING, 0 /* must be zero */, 
                                                            (LPARAM)"Electric");
        SendMessage(g_hwndWelcome_CBFuel, CB_SETCURSEL, 0 /* selection */, 
                                                          0 /* must be zero */);
        
        // Create search button. TODO: LL don't need global for g_hwndButton.
        g_hwndButton = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | 
            BS_DEFPUSHBUTTON, 163 + g_nWV_HOffset, 
            323 + g_nWV_VOffset, 103, 41, hWnd, (HMENU)IDC_WV_BUTTON, hInst, 0);
        App_SetFont(g_hwndButton);
        SendMessage(g_hwndButton, WM_SETTEXT, 0 /* not used */, 
                                                              (LPARAM)"Search");

        // Use image list and BCM_SETIMAGELIST to insert icon for XP/Vista UI
        // compatibility.
        hIL = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK,
                                        0 /* number of initial images */, 0);

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TB_SEARCH)); 
        ImageList_AddIcon(hIL, hIcon);
        DeleteObject(hIcon);
        
        // Prepare BUTTON_IMAGELIST structure.
        bil.himl = hIL;
        bil.margin = rcMargin;
        bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        // Put image over the button.
        SendMessage(g_hwndButton,  BCM_SETIMAGELIST, 0, 
                                               (LPARAM)(PBUTTON_IMAGELIST)&bil);
                             
        // Initialize CE_AnimUI.dll.
        iOffsetLeft = 390 + g_nWV_HOffset;
        iOffsetTop = 125 + g_nWV_VOffset; 

        DLL_InitCustomUI(hInst, hWnd);

        // Create checkbox to toggle the welcome page on startup. Notice that 
        // initial rectangle is set with SizeWindowContents.
          g_hwndCheckBox = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | 
             BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, 
                                         (HMENU)IDC_WELCOME_CHECKBOX, hInst, 0);
        SendMessage(g_hwndCheckBox, WM_SETTEXT, 0 /* not used */, 
                                   (LPARAM)"Do not show this view on startup.");
        App_SetFont(g_hwndCheckBox);
        SendMessage(g_hwndCheckBox, BM_SETCHECK, 
            g_bShowStartupWelcomeView ? BST_UNCHECKED : 
                        BST_CHECKED /* inversed in UI */, 0 /* must be zero */); 

        // TODO: if selection persisted - add logic
        ShowWindow(g_hwndStatic_MinPrice, FALSE);
        ShowWindow(g_hwndEdit_MinPrice, FALSE); 
        ShowWindow(g_hwndStatic_MaxPrice, FALSE);
        ShowWindow(g_hwndEdit_MaxPrice, FALSE);

        break;
    case WM_COMMAND:
        if (LOWORD(wParam) /* command ID*/ == IDC_WELCOME_CB_PRICERANGE)
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                WV_CB_OnChangeSelection();
            }
        }
        else if (LOWORD(wParam) /* command ID*/ ==  IDC_WV_BUTTON)
        {
            int nYearSelection;
            int nCarType;
            TCHAR szCarType[20];
            int nFuelType;
            TCHAR szFuelType[20];
            int nMinPrice, nMaxPrice;
            TCHAR szMinPrice[20], szMaxPrice[20];
            TCHAR szErrorMsg[MAX_LOADSTRING];
            TCHAR szInfo[MAX_PATH];

            nMinPrice = 0;
            nMaxPrice = 100000;

            nYearSelection = SendMessage(g_hwndWelcome_CBYear, CB_GETCURSEL, 
                                    0 /* must be zero */, 0 /* must be zero */);
            nCarType = SendMessage(g_hwndWelcome_CBType, CB_GETCURSEL, 
                                    0 /* must be zero */, 0 /* must be zero */);
            SendMessage(g_hwndEdit_MinPrice, WM_GETTEXT, sizeof(szMinPrice), 
                                                            (LPARAM)szMinPrice);
            SendMessage(g_hwndEdit_MaxPrice, WM_GETTEXT, sizeof(szMaxPrice), 
                                                            (LPARAM)szMaxPrice);
            nFuelType = SendMessage(g_hwndWelcome_CBFuel, CB_GETCURSEL, 
                                    0 /* must be zero */, 0 /* must be zero */);

            // Validate price fields before running query.
            if (WV_ValidatePrices(szMinPrice, szMaxPrice, 
                                         szErrorMsg) /* validation is passed */) 
            {
                nMinPrice = Utility_ConvertPriceToNumber(szMinPrice, 
                                                      NULL /* not used here */);
                nMaxPrice = Utility_ConvertPriceToNumber(szMaxPrice, 
                                                      NULL /* not used here */);

                TCHAR szTest[100];
                wsprintf(szTest, "Max: %d, Min: %d, %s, %s", nMinPrice, 
                                             nMaxPrice, szMinPrice, szMaxPrice);
                                                            
                DestroyWindow(g_hwndWelcome);
                ShowWindow(hwndMainLV, TRUE);
                //TCHAR szMinPrice[10];
                //lstrcpy(szMinPrice, "$23,899");
                //Utility_ConvertPriceToNumber(szMinPrice, NULL /* not used here */);
                // Clear report view before re-populating.
                ListView_DeleteAllItems(hwndMainLV);
                InitListViewItems(hwndMainLV, nYearSelection, nCarType, 
                                               nMinPrice, nMaxPrice, nFuelType);

                // Update units according to loaded from registry or updated.
                if (g_iUnits != DEFAULT_UNITS) ConvertUnits(hwndMainLV, 
                                                                      g_iUnits);

                // Sort by g_nSortColumn if relevant.
                if (g_iSortParam != -1 /* no sort if was not used */) 
                                        LV_SortItems(hwndMainLV, g_nSortColumn);

                // Show search results information in the status bar.
                wsprintf(szInfo, "Search result: %d cars are found",  
                                             ListView_GetItemCount(hwndMainLV));
                SendMessage(g_hwndStatus, WM_SETTEXT, 0 /* not used */, 
                                                                (LPARAM)szInfo);

                if (g_bUseKagiPaymentProcessing)
                {
                    // Enable crippling for the main LV.
                    if (!g_bRegisteredCopy && m_RP.IsTrialUseExpired())
                    {
                        DialogBox(hInst, (LPCTSTR)IDD_REGISTER_BUY_PRODUCT, 
                                   g_hWnd, (DLGPROC)RegisterBuyProduct_WndProc);
                    }
                }
            }
            else // validation is not passed
            {
                MessageBox(0, szErrorMsg, szMBTitle, 
                                                    MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (LOWORD(wParam) /* command ID*/ == IDC_WELCOME_CHECKBOX)
        {
            //	Reflect the state of checkbox immediately in session variable.
            if (HIWORD(wParam) == BN_CLICKED)
            {
                int nRes = SendMessage(g_hwndCheckBox, BM_GETCHECK, 
                                            0 /* not used */, 0 /* not used */);
                // Notice that the name of variable is logically inversed in 
                // relation to standard "Do not show this view on startup" to 
                // avoid cumbersome negative variable name.
                if (nRes == BST_CHECKED)
                    g_bShowStartupWelcomeView = FALSE;
                else if (nRes == BST_UNCHECKED)
                    g_bShowStartupWelcomeView = TRUE;
            }
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        WV_OnPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_CTLCOLORSTATIC:
        // Set backgrond color of all WV's static and checkbox controls.
        //if ((HWND)lParam == g_hwndCheckBox) 
        {
            //SetTextColor((HDC) wParam, RGB(0,255,0));
            SetBkMode((HDC) wParam, TRANSPARENT);
            return((LRESULT)(HBRUSH)CreateSolidBrush(g_clrWV_InnerBackground));
        }
    // Notice that WM_ERASEBKGND is sent before every WM_PAINT, class brush is 
    // used to fill RECT before WM_PAINT.
    case WM_ERASEBKGND: // double buffering is used
        return TRUE; // indicate that handled
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    //return DefWindowProc(hWnd, message, wParam, lParam);
    return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HFONT hfnt;
    LOGFONT lf;
    RECT rc;
    RECT rcMainWnd;

    // Specify the font to use (stored in a LOGFONT structure).
    lf.lfHeight = 16;
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
    _tcsncpy(lf.lfFaceName, _T("Arial"), 32);

    hfnt = CreateFontIndirect(&lf);	

    TCHAR szProgramFilesDir[MAX_PATH];
    TCHAR szSplashScreenFFN[MAX_PATH];
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR szBuffer[200];
    
    switch (message)
    {
    case WM_INITDIALOG:
       // Center dialog relative to the application window.
        GetWindowRect(g_hWnd, &rcMainWnd);
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC), WM_SETTEXT, 
                                      0 /* not used */, (LPARAM)g_szAD_Version);
        // Set font and language-dependent strings for dialog's controls.
        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC), WM_SETFONT, 
                                                            (WPARAM)hfnt, TRUE);
        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC2), WM_SETFONT, 
                                                            (WPARAM)hfnt, TRUE);
        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC3), WM_SETFONT, 
                                                            (WPARAM)hfnt, TRUE);
        
        LoadString(hInst, IDS_AD_COPYRIGHT1, szBuffer, 150);
        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC4), WM_SETTEXT, 
                                            0 /* not used */, (LPARAM)szBuffer);
        App_SetFont(GetDlgItem(hDlg, IDC_AD_STATIC4), FALSE);

        LoadString(hInst, IDS_AD_COPYRIGHT2, szBuffer, 200);
        SendMessage(GetDlgItem(hDlg, IDC_AD_STATIC5), WM_SETTEXT, 
                                            0 /* not used */, (LPARAM)szBuffer);
        App_SetFont(GetDlgItem(hDlg, IDC_AD_STATIC5), FALSE);
        //SendMessage(GetDlgItem(hDlg, IDOK), WM_SETFONT, (WPARAM)hfnt, TRUE);

        // Load and set picture (jpg). Notice that the picture for About dialog 
        //  is installed into the root of install directory.
        ExpandEnvironmentStrings("%programfiles%", szProgramFilesDir, 
                                                     sizeof(szProgramFilesDir));
        wsprintf(szSplashScreenFFN, "%s%s", szProgramFilesDir, 
                                              "\\Car Explorer\\AD_Picture.jpg");
        m_AD_SinglePicture.Load(szSplashScreenFFN);

        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hDlg, &ps);
        AD_OnPaint(hDlg, &ps);
        EndPaint(hDlg, &ps);
        break;
    }
    return FALSE;
}

LRESULT CALLBACK Extract_WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;

    switch (message)
    {
    case WM_INITDIALOG:

        // Center dialog.
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL,
            ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
            ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

        return TRUE;

    case WM_COMMAND:
        
        if (LOWORD(wParam) == IDOK) 
        {
            //LoadPicturesAndStrings();
            //EndDialog(hDlg, LOWORD(wParam));
            //// Remove process from memory.
            //PostQuitMessage(0); 
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) 
        {
            // Remove process from memory.
            PostQuitMessage(0);
            return TRUE;
        }

        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_DESTINATION:
            DoBrowse(hDlg, g_szDestination /* out */);
            SendMessage(GetDlgItem(hDlg, IDC_EDIT), 
                         WM_SETTEXT, 0 /* not used */, (LPARAM)g_szDestination);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        }
        break;
    }
    return FALSE;
}

// PURPOSE: used only when DC on CAR file (out of default folder) occurs to
//          populate m_vecCarData before populating LV. 
BOOL LoadCarData_SingleFile()
{
    TCHAR szFFN[MAX_PATH];
    CCarData m_CarData;
    TCHAR szTitle[100];
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szCountry[100];
    TCHAR szPictureTitle[100]; 
 //   // Main.
 //   string sYear;
 //   string sMake;
    TCHAR szModel[100];
    TCHAR szSubModel[100];
    TCHAR szType[100];
    TCHAR szPrice[100];
    //// Engine.
    TCHAR szPower[100];
    TCHAR szFuel[100];
    //// Sizes and weight.
    TCHAR szLength[100];
    TCHAR szWidth[100];
    TCHAR szHeight[100];
    TCHAR szClearance[100];
    TCHAR szWeight[100];
    //// Misc.
    TCHAR szTankVolume[100];
    //string sCountry;

    TCHAR szImagesCount[10];
    int nImagesCount;


    lstrcpy(szFFN, g_szFullFileName);

    GetImageSetTitle(szFFN, szTitle);
    GetCarXMLData(szFFN, 1000 /* year */, szYear);
    GetCarXMLData(szFFN, 1100 /* make */, szMake);
    GetCarXMLData(szFFN, 2300 /* country */, szCountry);

    GetCarXMLData(szFFN, 10 /* images count */, szImagesCount);
    nImagesCount = atoi(szImagesCount);

    for (int i = 0; i < nImagesCount; i++)
    {
        GetCarXMLData(szFFN, 201 + i /* picture title */, szPictureTitle);
        GetCarXMLData(szFFN, 1201 + i /* model */, szModel);
        GetCarXMLData(szFFN, 1301 + i /* submodel */, szSubModel);
        GetCarXMLData(szFFN, 1401 + i /* type */, szType);
        GetCarXMLData(szFFN, 1501 + i /* price */, szPrice);
        GetCarXMLData(szFFN, 1601 + i /* power */, szPower);
        GetCarXMLData(szFFN, 1701 + i /* fuel */, szFuel);
        GetCarXMLData(szFFN, 1801 + i /* length */, szLength);
        GetCarXMLData(szFFN, 1901 + i /* width */, szWidth);
        GetCarXMLData(szFFN, 2001 + i /* height */, szHeight);
        GetCarXMLData(szFFN, 2201 + i /* clearance */, szClearance);
        GetCarXMLData(szFFN, 2101 + i /* weight */, szWeight);
        GetCarXMLData(szFFN, 2401 + i /* tank volume */, szTankVolume);
        
        // Add car details to the vector.
        m_CarData.sTitle = szTitle; 
        //m_CarData.sFileName = fd.cFileName; 
        //m_CarData.sFFN = ...;
        m_CarData.sYear = szYear; 
        m_CarData.sMake = szMake; 
        m_CarData.sModel = szModel; 
        m_CarData.sSubModel = szSubModel; 
        m_CarData.sType = szType; 
        m_CarData.sPrice = szPrice; 
        m_CarData.sPower = szPower; 
        m_CarData.sFuel = szFuel;
        m_CarData.sLength = szLength;
        m_CarData.sWidth = szWidth;
        m_CarData.sHeight = szHeight;
        m_CarData.sClearance = szClearance;
        m_CarData.sWeight = szWeight;
        m_CarData.sTankVolume = szTankVolume;
        m_CarData.sCountry = szCountry;
        m_CarData.nImageID = i; // relative order in collection
        m_CarData.nIndex = i; // in this particular case (only one CAR file) nIndex conincides with ImageID

        m_vecCarData.push_back(m_CarData);
    }
    
    return FALSE; 
}

//BOOL LoadPicturesAndStrings(HWND hwndLV, BOOL bLoadStringsOnly)
BOOL LoadPicturesAndStrings(HWND hwndLV)
{
    HINSTANCE hResInst;
    TCHAR szBuffer[100];        // accepts DLL's image count
    int iImageID; 
    int iImageResourceStartID = 100;
    int iImageCount;
    int iTitleID;               // not used 
    TCHAR szTitle[100];         // not used
    long hmWidth,hmHeight;      // not used
    TCHAR szFileDimensions[20]; // not used
    string sTitle;
    vector<string> vecImageTitles;

    // Return if run by double-click (no MSS file as argument).
    if (lstrlen(g_szFullFileName) == 0) return FALSE;
    hResInst = LoadLibrary(g_szFullFileName /* <filename>.mss with path */);

    if (hResInst == NULL)
    {
        MessageBox(NULL, g_szFullFileName, "LoadLibrary failed!", MB_OK);
        return FALSE;
    }

    // Load pictures from image set.
    LoadString(hResInst, 10 /* always ImageCount ID */ , szBuffer, 
                                          sizeof(szBuffer)/sizeof(szBuffer[0]));
    iImageCount = _tstoi(szBuffer);

    for (int i = 0; i < iImageCount; i++) 
    {
        iImageID = iImageResourceStartID + i + 1;
        // Images 101 ... 105 ...
        m_Picture[i].Load(hResInst, "IMAGES", iImageID);
        // Titles 201 ... 205 ...
        iTitleID = iImageID + 100; 
        LoadString(hResInst, iTitleID, szTitle, 
                                            sizeof(szTitle)/sizeof(szTitle[0]));
        sTitle = szTitle;
        // I avoid using global vector for storing titles, instead titles are 
        // loaded into local vector here and passed to CreateImageList. This
        // is to prevent crash (LL stack issue) for MSS with a lot of images 
        // (see also Notes.txt).
        vecImageTitles.push_back(sTitle);
        //if (_tcslen(szTitle) != 0) _tcscpy(m_Title[i], szTitle);

        if (lstrcmp(__argv[1], "/e") == 0)
        {
            //SaveToImageFile(i);
        }
    };

    //if (__argc == 2) // double-click on MSS
    //{
    //	CreateImageList(iImageCount, vecImageTitles);
    //}
    //else if (lstrcmp(__argv[1], "/b") == 0 || lstrlen(__argv[1]) == 0)
    {
        CreateImageList(hwndLV, iImageCount, vecImageTitles);
    }

    FreeLibrary(hResInst);

    return TRUE;
}

void SaveToImageFile(LPCTSTR pzsFFN, int iImage)
//void SaveToImageFile(int iImage)
{
    CComPtr<IStream> stream; 
    HGLOBAL hdata; 
    STATSTG ss; 
    TCHAR szDestinationImage[MAX_PATH];
    DWORD written; 

    // Create a stream backed by memory. 
    CreateStreamOnHGlobal(NULL, TRUE, &stream); 

    // Save the picture into memory. 
    //m_Picture[iImage].m_spIPicture->SaveAsFile(stream, TRUE, NULL);
    m_SinglePicture.m_spIPicture->SaveAsFile(stream, TRUE, NULL);
     
    // Get a handle to the memory block.
    GetHGlobalFromStream(stream, &hdata); 

    // Determine the size of the stream. 
    stream->Stat(&ss, STATFLAG_NONAME); 

    // Get a pointer to the actual memory data. 
    LPCVOID data = GlobalLock(hdata); 
     
    //// Construct destination image path.
    //wsprintf(szDestinationImage, "%s%s%s%d%s", g_szDestination, "\\", "Image", 
    //	                                                        iImage, ".jpg");

    //MessageBox(0, "Destination Image", szDestinationImage, MB_OK);
    // Open the file where the picture data will be stored. 
    //HANDLE hfile = CreateFile(szDestinationImage, GENERIC_READ | GENERIC_WRITE, 
    HANDLE hfile = CreateFile(pzsFFN, GENERIC_READ | GENERIC_WRITE, 
           (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
     
    WriteFile(hfile, data, ss.cbSize.LowPart, &written, NULL); 

    CloseHandle(hfile); 

    // Unlock the memory block. 
    GlobalUnlock(hdata); 
}

// From MySSWnd.cpp (MySS_User_Win32). This function loads a file into an IStream.
void CPicture::Load(LPCTSTR szFile)
{
    // open file
    HANDLE hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    _ASSERTE(INVALID_HANDLE_VALUE != hFile);

    // get file size
    DWORD dwFileSize = GetFileSize(hFile, NULL);
    _ASSERTE(-1 != dwFileSize);

    LPVOID pvData = NULL;
    // alloc memory based on file size
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
    _ASSERTE(NULL != hGlobal);

    pvData = GlobalLock(hGlobal);
    _ASSERTE(NULL != pvData);

    DWORD dwBytesRead = 0;
    // read file and store in global memory
    BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
    _ASSERTE(FALSE != bRead);
    GlobalUnlock(hGlobal);
    CloseHandle(hFile);

    LPSTREAM pstm = NULL;
    // create IStream* from global memory
    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
    _ASSERTE(SUCCEEDED(hr) && pstm);

    // Create IPicture from image file
    //if (m_spIPicture)
    //	m_spIPicture->Release();
    hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&m_spIPicture);
    _ASSERTE(SUCCEEDED(hr) && m_spIPicture);	
    pstm->Release();

    //InvalidateRect(ghWnd, NULL, TRUE);
}



// PURPOSE:  This function loads a file into an IStream from CAR files or 
//           application executable.
// NOTES:    Modified from MySSWnd.cpp (MySS_User_Win32) to load either from 
//           external CAR (or MSS) file or GIF from the application EXE.
BOOL CPicture::Load(HINSTANCE hResInst, LPCTSTR pszResType, UINT nIDRes)
{
    HRSRC hRsrc;
    TCHAR szResType[10]; // "IMAGES" (CAR files) or "CUSTOM" (CE.EXE's GIF)

    wsprintf(szResType, "#%d", nIDRes); // e.g. #103 (CAR), #180 for GIF
    hRsrc = FindResource(hResInst, szResType, pszResType);
    if (hRsrc == NULL)
    {
        MessageBox(0, "FindResource failed!", "Car Explorer", 
                                                            MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Load resource into memory
    BYTE* lpRsrc = (BYTE*)LoadResource(hResInst, hRsrc);
    if (lpRsrc == NULL)
    {
        //AfxMessageBox("LoadResource failed!");
        return FALSE;
    }

    // create memory file and load it
    DWORD dwFileSize = SizeofResource(hResInst, hRsrc);

    //CMemFile file(lpRsrc, len);
    //BOOL bRet; // = Load(file);
    //FreeResource(hRsrc);

        // get file size
    //DWORD dwFileSize = GetFileSize(hFile, NULL);
    //_ASSERTE(-1 != dwFileSize);

    LPVOID pvResourceData = LockResource(lpRsrc);
    //LPVOID pvData = NULL;
    // alloc memory based on file size
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
    //_ASSERTE(NULL != hGlobal);

    LPVOID pvData = GlobalLock(hGlobal);
    //_ASSERTE(NULL != pvData);

    CopyMemory(pvData , pvResourceData, dwFileSize);

    //DWORD dwBytesRead = 0;
    // read file and store in global memory
    //BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
    //_ASSERTE(FALSE != bRead);
    GlobalUnlock(hGlobal);
    //CloseHandle(hFile);

    LPSTREAM pstm = NULL;
    // create IStream* from global memory
    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
    //_ASSERTE(SUCCEEDED(hr) && pstm);

    // Create IPicture from image file
    //if (m_spIPicture)
    //	m_spIPicture->Release();
    hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&m_spIPicture);
    //_ASSERTE(SUCCEEDED(hr) && m_spIPicture);	
    pstm->Release();

    return TRUE; //bRet;
}

// From Install.cpp (RT installer)
int DoBrowse(HWND hDlg, LPTSTR pszDestination /* out */)
{
    TCHAR szDestination[MAX_PATH];
    LPMALLOC pMalloc;
    BROWSEINFO bInfo;
    LPITEMIDLIST pidl;

    if (SHGetMalloc(&pMalloc)!= NOERROR)
    {
        return 0;
    }

    ZeroMemory ((PVOID) &bInfo,sizeof (BROWSEINFO));

    bInfo.hwndOwner = hDlg; // keeps browse window close to the owner
    bInfo.pszDisplayName = szDestination; 
    bInfo.lpszTitle = "Select install directory"; 
    bInfo.ulFlags =  BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS; 
    bInfo.lpfn = NULL; // no callback function

    if ((pidl = ::SHBrowseForFolder(&bInfo)) == NULL)
    {
        return 0;
    }

    if (::SHGetPathFromIDList(pidl, szDestination /* out */) == FALSE)
    {
        pMalloc ->Free(pidl);
        pMalloc ->Release();
        return 0;
    }

    pMalloc ->Free(pidl);
    pMalloc ->Release();

    // Return destination.
    wsprintf(pszDestination, szDestination);

    return 1;
}

void SizeWindowContents(int nWidth, int nHeight)
{
    RECT rc;
    int iCBTopOffset;
    BOOL bCondition1, bCondition2;
    
    // Height should be the same as vertical offset for browser window. This 
    // makes correct painting when another window goes over and make keeps 
    // cursor always IDC_ARROW in the area.
    MoveWindow(hwndTopArea, 0, g_nTBHeightUI, nSplitterPos, 
                                             60 /* adjusted manually */, TRUE);  
                                                                          
    MoveWindow(hwndComboBox, 10, 20, nSplitterPos - 20, 20, TRUE);
    
    //We will get SetScrollInfo's called by the listview, because
    //it has been changed in size
    MoveWindow(hwndBrowser, 0, g_nTBHeightUI + 60 /* height of top area */, 
        nSplitterPos, nHeight - g_nTBHeightUI - 60 /* height of top area */ 
        - g_nSBHeightUI, TRUE);
        //- 23, TRUE);
        //, TRUE);

    if (g_nRightPaneViewType == 0) 
    {
        MoveWindow(hwndMainLV, nSplitterPos + nSplitterBorder, g_nTBHeightUI, 
            nWidth - nSplitterPos - nSplitterBorder, 
                                 nHeight - g_nTBHeightUI - g_nSBHeightUI, TRUE);

        if (g_hwndWelcome != NULL) 
        {
            MoveWindow(g_hwndWelcome, nSplitterPos + nSplitterBorder, 
              g_nTBHeightUI, nWidth - nSplitterPos - nSplitterBorder, 
                                 nHeight - g_nTBHeightUI - g_nSBHeightUI, TRUE);

            // Set intitial position for CB and update its keep its constant 
            // offset from window bottom when main window is resized.
            GetClientRect(g_hwndWelcome, &rc);
            iCBTopOffset = (rc.bottom - rc.top /* 0 */) - g_nWV_CBBottomOffset;
                                                         
            // Avoid overlapping controls when width of main window is 
            // descreased, keep checkbox at the same level of inner area.

            // 1st condition: turns TRUE (size increases) when bottom border 
            // crosses inner area (so checkbox should stop moving).
            bCondition1 = ((rc.bottom - rc.top /* 0 */) - (WV_HEADER_HEIGHT + 
                  WV_SEPARATOR_HEIGHT + g_nWV_InnerAreaHeight) > g_nWV_VOffset);

            // 2nd condition: turns TRUE (size decreases) when an oppositite 
            // of 1st condition occurs and checkbox still not overlaps search 
            // controls (so checkbox should start moving).
            //bCondition2 = ((rc.bottom - rc.top /* 0 */) - g_nWV_InnerAreaHeight  
            //	< g_nWV_VOffset + WV_HEADER_HEIGHT + WV_SEPARATOR_HEIGHT)
            //	                        && (iCBTopOffset > WV_CB_TOPOFFSET_MIN);
            bCondition2 = !bCondition1 && (iCBTopOffset > WV_CB_TOPOFFSET_MIN);

            if (bCondition1)
            {
                // Show and keep checkbox at the same level from the top.
                // TODO: g_nWV_InnerAreaHeight is actuallly a constant 
                // calculated once (line 8078) so clarify (probably make a 
                // constant removing redundancy at line 8078). The reason why 
                // 378 is used is that when WV is invoked from TB 
                // g_nWV_InnerAreaHeight turns zero (should be 378).
                iCBTopOffset = g_nWV_VOffset + 398 /* g_nWV_InnerAreaHeight */ +
                    (WV_HEADER_HEIGHT + WV_SEPARATOR_HEIGHT) - 
                                                           g_nWV_CBBottomOffset;
                MoveWindow(g_hwndCheckBox, g_nWV_CBPosLeft, iCBTopOffset, 180, 
                                                                      32, TRUE);
            }
            else if (bCondition2)
                // Move checkbox.
                MoveWindow(g_hwndCheckBox, g_nWV_CBPosLeft, iCBTopOffset, 180, 
                                                                      32, TRUE);
                                                                          
            // Keep constant offset (g_nWV_AnumUIRightOffset) for AnimUI from 
            // right window border when main window is resized.
            iOffsetLeft = (rc.right - rc.left /* 0 */) -  
                                                        g_nWV_AnumUIRightOffset;
            // Avoid overlapping controls when height of main window is 
            // descreased.
            if (iOffsetLeft > WV_ANIMUI_LEFTOFFSET_MIN) 
            {
                g_bWV_NarrowWidth = FALSE;
                MoveControl(iOffsetLeft);
            }
            else
            {
                g_bWV_NarrowWidth = TRUE;
            }
        }
    }
    else if (g_nRightPaneViewType == 2) // single picture from favorites
    {
        MoveWindow(hwndRightBottomHost, nSplitterPos + nSplitterBorder, 
        g_nTBHeightUI, nWidth - nSplitterPos - nSplitterBorder, nHeight - 
                                           g_nTBHeightUI - g_nSBHeightUI, TRUE);
    }
    else if (g_nRightPaneViewType == 3) // single CAR file without TV
    {
        MoveWindow(g_hwndLV, 0, g_nTBHeightUI, nWidth, nHeight - 
                                           g_nTBHeightUI - g_nSBHeightUI, TRUE);
    }
    else // single CAR file (with TV)
    {
        if (g_nCarFilesLayout == 0 || g_nCarFilesLayout == 1)
        {
            MoveWindow(hwndMainLV, nSplitterPos + nSplitterBorder, 
                g_nTBHeightUI, nWidth - nSplitterPos - nSplitterBorder, 
                                 nHeight - g_nTBHeightUI - g_nSBHeightUI, TRUE);
            if (hwndRightHost != NULL) 
                MoveWindow(hwndRightHost, nSplitterPos + nSplitterBorder, 
                g_nTBHeightUI, nWidth - nSplitterPos - nSplitterBorder, 
                                 nHeight - g_nTBHeightUI - g_nSBHeightUI, TRUE);
        }

        // Reserved: LV takes all right view pane.
        //MoveWindow(hwndMainLV, nSplitterPos + nSplitterBorder, 
        //    g_nTBHeightUI, nWidth - nSplitterPos - nSplitterBorder, 
        //                        nHeight - g_nTBHeightUI - g_nSBHeightUI, TRUE);
    }

    //MoveWindow(hwndRightHost, nSplitterPos + nSplitterBorder, 0, 
    //	                nWidth - nSplitterPos - nSplitterBorder, nHeight, TRUE);

    TCHAR sxText[200];
    wsprintf(sxText, "SizeWindowContents, Height: %d", nHeight);
    //App_Debug(sxText);
}

void SizeWindowContents2(int nWidth, int nHeight)
{
    int nUpperWndHeight;
    int nBottomWndHeight;

    if (g_nRightPaneViewType == 3 /* DC on CAR file (out of default folder) */)
    {
        // Notice that MoveWindow uses client coordinates (child windows).
        nUpperWndHeight = nSplitterPos2 /* client */ - g_nTBHeightUI;
        MoveWindow(g_hwndLV, 0, g_nTBHeightUI /* client */, nWidth, 
                                                         nUpperWndHeight, TRUE);
        nBottomWndHeight = nHeight - nSplitterPos2 - nSplitterBorder - 
                                                                  g_nSBHeightUI;
        MoveWindow(hwndRightBottomHost, 0, (nSplitterPos2 + 
               nSplitterBorder) /* client */, 	nWidth, nBottomWndHeight, TRUE);
    }
    else
    {
        MoveWindow(g_hwndLV, 0, 0, nWidth, nSplitterPos2, TRUE);
        MoveWindow(hwndRightBottomHost, 0, nSplitterPos2 + nSplitterBorder, 
                       nWidth, nHeight - nSplitterPos2 - nSplitterBorder, TRUE);
    }
}

void SizeWindowContents3(int nWidth, int nHeight)
{
    MoveWindow(hwndPictureView, 0, 0, nSplitterPos3, nHeight, TRUE);
                                                                 
    MoveWindow(hwndBottomLV, nSplitterPos3 + nSplitterBorder, 
        -1 /* manual adjustment */, 
                       nWidth - nSplitterPos3 - nSplitterBorder, nHeight, TRUE);

    TCHAR sxText[200];
    wsprintf(sxText, "SizeWindowContents3, Height: %d", nHeight);
    //App_Debug(sxText);
}

void DrawXorBar(HDC hdc, int x1, int y1, int width, int height)
{
    static WORD _dotPatternBmp[8] = 
    { 
        0x00aa, 0x0055, 0x00aa, 0x0055, 
        0x00aa, 0x0055, 0x00aa, 0x0055
    };

    HBITMAP hbm;
    HBRUSH  hbr, hbrushOld;

    hbm = CreateBitmap(8, 8, 1, 1, _dotPatternBmp);
    hbr = CreatePatternBrush(hbm);
    
    SetBrushOrgEx(hdc, x1, y1, 0);
    hbrushOld = (HBRUSH)SelectObject(hdc, hbr);
    
    PatBlt(hdc, x1, y1, width, height, PATINVERT);
    
    SelectObject(hdc, hbrushOld);
    
    DeleteObject(hbr);
    DeleteObject(hbm);
}

LRESULT Splitter_OnLButtonDown(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    POINT pt;
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;

    // Cursor position relative to (0,0) of the main client area (without 
    // upper bars).
    pt.x = (short)LOWORD(lParam);  
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    RECT rcClient; GetClientRect(hwnd, &rcClient);
    TCHAR sxText[200];

    wsprintf(sxText, "POINT (x: %d, y: %d)", pt.x, pt.y); //pt.y->0 when at the top of client area 
    //App_Debug(sxText);

    wsprintf(sxText, "Window RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                                  rect.left, rect.top, rect.right, rect.bottom); // same as with Spy++
    //App_Debug(sxText);
    wsprintf(sxText, "Client RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                  rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    //App_Debug(sxText);

    //wsprintf(sxText, "%d", rect.top - rcClient.top); 
    //MessageBox(0, sxText, "OnLButtonDown", 0);

    // Convert the mouse coordinates relative to the top-left of the window.
    ClientToScreen(hwnd, &pt);
    wsprintf(sxText, "POINT after ClientToScreen (x: %d, y: %d)", pt.x, pt.y);  
    //App_Debug(sxText);
    pt.x -= rect.left;
    pt.y -= rect.top;
    wsprintf(sxText, "POINT 3 (x: %d, y: %d)", pt.x, pt.y);  
    //App_Debug(sxText);
    
    // Same for the window coordinates - make them relative to 0,0.
    OffsetRect(&rect, -rect.left, -rect.top);
    wsprintf(sxText, "Window RECT 2 (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                                  rect.left, rect.top, rect.right, rect.bottom); 
    //App_Debug(sxText);
    
    // LL redundant
    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    fDragMode = TRUE;

    SetCapture(hwnd);

    // Get border width.
    WINDOWINFO wi;
    GetWindowInfo(hwnd, &wi);
    wsprintf(sxText, "Window Border: %d (SM_CYBORDER: %d, SM_CYSIZEFRAME: %d)", 
        wi.cyWindowBorders, 
        GetSystemMetrics(SM_CYBORDER), GetSystemMetrics(SM_CYSIZEFRAME)); // 4, 1, 4
    //App_Debug(sxText);

    // Get y-position of the top of splitter bar. The height of caption, toolbar ares, 
    // and border must be accounted.
    nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) - 
                                                             //wi.cyWindowBorders;
                                             wi.cyWindowBorders + g_nTBHeightUI;
    nHeight = rcClient.bottom - rcClient.top;

    hdc = GetWindowDC(hwnd);
    //DrawXorBar(hdc, 1,pt.y - 2, rect.right-2,4);
    //pt.x - 2, 50, 6, rect.bottom - 55 (rect.bottom is equal to rect.bottom - rect.top)
    //DrawXorBar(hdc, pt.x - 2, nVerOffset, 6 /* width */, nHeight); 
    DrawXorBar(hdc, pt.x - 2, nVerOffset, SPLITTER_WIDTH, 
                                       nHeight - g_nTBHeightUI - g_nSBHeightUI); 
    ReleaseDC(hwnd, hdc);
    
    oldy = pt.x;
        
    return 0;
}

LRESULT Splitter2_OnLButtonDown(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    RECT rcClient;
    POINT pt;
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;
    int nWidth;

    // Cursor position relative to (0,0) of the main client area (without 
    // upper bars).
    pt.x = (short)LOWORD(lParam);  
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);
    TCHAR sxText[200];

    wsprintf(sxText, "POINT (x: %d, y: %d)", pt.x, pt.y); //pt.y->0 when at the top of client area 
    //App_Debug(sxText);

    wsprintf(sxText, "Window RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                                  rect.left, rect.top, rect.right, rect.bottom); // same as with Spy++
    //App_Debug(sxText);
    wsprintf(sxText, "Client RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                  rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    //App_Debug(sxText);

    //wsprintf(sxText, "%d", rect.top - rcClient.top); 
    //MessageBox(0, sxText, "OnLButtonDown", 0);

    // Convert the mouse coordinates relative to the top-left of the window.
    ClientToScreen(hwnd, &pt);
    wsprintf(sxText, "POINT after ClientToScreen (x: %d, y: %d)", pt.x, pt.y);  
    //App_Debug(sxText);
    pt.x -= rect.left;
    pt.y -= rect.top;
    wsprintf(sxText, "POINT 3 (x: %d, y: %d)", pt.x, pt.y);  
    //App_Debug(sxText);
    
    // Same for the window coordinates - make them relative to 0,0.
    OffsetRect(&rect, -rect.left, -rect.top);
    wsprintf(sxText, "Window RECT 2 (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
                                  rect.left, rect.top, rect.right, rect.bottom); 
    //App_Debug(sxText);
    
    // LL redundant
    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    fDragMode2 = TRUE;

    SetCapture(hwnd);

 //   // Get border width.
    //WINDOWINFO wi;
    //GetWindowInfo(hwnd, &wi);
    //wsprintf(sxText, "Window Border: %d (SM_CYBORDER: %d, SM_CYSIZEFRAME: %d)", wi.cyWindowBorders, 
    //	GetSystemMetrics(SM_CYBORDER), GetSystemMetrics(SM_CYSIZEFRAME)); // 4, 1, 4
    ////App_Debug(sxText);

    //// Get y-position of the top of splitter bar. The height of caption, toolbar ares, 
    //// and border must be accounted.
    //nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) - 
    //	                                                     wi.cyWindowBorders;
    //nHeight = rcClient.bottom - rcClient.top;

    hdc = GetWindowDC(hwnd);
    
    if (g_nRightPaneViewType == 3 /* DC on CAR file (out of default folder) */)
    {
        nWidth = rcClient.right - rcClient.left;
        DrawXorBar(hdc, 4, pt.y - 2, nWidth, SPLITTER_WIDTH);
    }
    else
        DrawXorBar(hdc, 1, pt.y - 2, rect.right-2, SPLITTER_WIDTH);
    //DrawXorBar(hdc, pt.x - 2, nVerOffset, 6 /* width */, nHeight); 
    ReleaseDC(hwnd, hdc);
    
    oldy2 = pt.y;
        
    return 0;
}

LRESULT Splitter3_OnLButtonDown(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    POINT pt;
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;

    // Cursor position relative to (0,0) of the main client area (without 
    // upper bars).
    pt.x = (short)LOWORD(lParam);  
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    RECT rcClient; GetClientRect(hwnd, &rcClient);
    //TCHAR sxText[200];

    //wsprintf(sxText, "POINT (x: %d, y: %d)", pt.x, pt.y); //pt.y->0 when at the top of client area 
 //   //App_Debug(sxText);

    //wsprintf(sxText, "Window RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
    //	                          rect.left, rect.top, rect.right, rect.bottom); // same as with Spy++
    ////App_Debug(sxText);
    //wsprintf(sxText, "Client RECT (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
    //	          rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    ////App_Debug(sxText);

    ////wsprintf(sxText, "%d", rect.top - rcClient.top); 
    ////MessageBox(0, sxText, "OnLButtonDown", 0);

    // Convert the mouse coordinates relative to the top-left of the window.
    ClientToScreen(hwnd, &pt);
    //wsprintf(sxText, "POINT after ClientToScreen (x: %d, y: %d)", pt.x, pt.y);  
 //   App_Debug(sxText);
    pt.x -= rect.left;
    pt.y -= rect.top;
    //wsprintf(sxText, "POINT 3 (x: %d, y: %d)", pt.x, pt.y);  
 //   App_Debug(sxText);
    
    // Same for the window coordinates - make them relative to 0,0.
    OffsetRect(&rect, -rect.left, -rect.top);
    //wsprintf(sxText, "Window RECT 2 (Left: %d, Top: %d%, Right: %d, Bottom: %d)", 
    //	                          rect.left, rect.top, rect.right, rect.bottom); 
    //App_Debug(sxText);
    
    // LL redundant
    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    fDragMode3 = TRUE;

    SetCapture(hwnd); 
    //ReleaseCapture(); // <-- test only

    // Get border width.
    WINDOWINFO wi;
    GetWindowInfo(hwnd, &wi);
    //wsprintf(sxText, "Window Border: %d (SM_CYBORDER: %d, SM_CYSIZEFRAME: %d)", wi.cyWindowBorders, 
    //	GetSystemMetrics(SM_CYBORDER), GetSystemMetrics(SM_CYSIZEFRAME)); // 4, 1, 4
    //App_Debug(sxText);

    // Get y-position of the top of splitter bar. The height of caption, toolbar ares, 
    // and border must be accounted.
    nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) - 
                                                             wi.cyWindowBorders;
    nHeight = rcClient.bottom - rcClient.top;

    hdc = GetWindowDC(hwnd);
    DrawXorBar(hdc, pt.x - 2, nVerOffset, SPLITTER_WIDTH, nHeight); 
    ReleaseDC(hwnd, hdc);
    

    oldy3 = pt.x;
        
    return 0;
}

LRESULT Splitter_OnLButtonUp(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;
    RECT rcClient;

    POINT pt;
    pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
    pt.y = (short)HIWORD(lParam);

    if (fDragMode == FALSE)
        return 0;
    
    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;
    
    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    // Get border width.
    WINDOWINFO wi;
    GetWindowInfo(hwnd, &wi);

    // Get y-position of the top of splitter bar. The height of caption, 
    // toolbar ares, and border must be accounted.
    nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) 
                                                        //- wi.cyWindowBorders;
                                           - wi.cyWindowBorders + g_nTBHeightUI;
    nHeight = rcClient.bottom - rcClient.top;	

    hdc = GetWindowDC(hwnd);
    // Erase the bar.
    DrawXorBar(hdc, oldy - 2, nVerOffset, SPLITTER_WIDTH, 
                                       nHeight - g_nTBHeightUI - g_nSBHeightUI);
        
    ReleaseDC(hwnd, hdc);


    fDragMode = FALSE;

    //convert the splitter position back to screen coords.
    GetWindowRect(hwnd, &rect);
    pt.x += rect.left;
    pt.y += rect.top;

    //now convert into CLIENT coordinates
    ScreenToClient(hwnd, &pt);
    GetClientRect(hwnd, &rect);
    nSplitterPos = pt.x;
    
    //position the child controls
    SizeWindowContents(rect.right,rect.bottom);

    ReleaseCapture();

    return 0;
}

LRESULT Splitter2_OnLButtonUp(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt; // client coordinates
    HDC hdc;
    RECT rect;
    int nHeight;
    int nWidth;
    int nVerOffset;
    RECT rcClient;
    
    // Cursor position relative to (0,0) of the main client area.
    pt.x = (short)LOWORD(lParam);  
    pt.y = (short)HIWORD(lParam);

    if (fDragMode2 == FALSE)
        return 0;
    
    GetWindowRect(hwnd, &rect); // screen coordinates
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;

    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    //// Get border width.
    //WINDOWINFO wi;
    //GetWindowInfo(hwnd, &wi);

    //// Get y-position of the top of splitter bar. The height of caption, 
    //// toolbar ares, and border must be accounted.
    //nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) 
    //		                                            - wi.cyWindowBorders;
    //nHeight = rcClient.bottom - rcClient.top;

    hdc = GetWindowDC(hwnd);
    if (g_nRightPaneViewType == 
                                 3 /* DC on CAR file (out of default folder) */)
    {
        nWidth = rcClient.right - rcClient.left;
        DrawXorBar(hdc, 4, oldy2 - 2, nWidth, SPLITTER_WIDTH);
    }
    else
    {
        DrawXorBar(hdc, 1, oldy2 - 2, rect.right - 2, SPLITTER_WIDTH);
    }

    // Erase the bar.
    //DrawXorBar(hdc, oldy - 2, nVerOffset, 6 /* width */, nHeight);
    ReleaseDC(hwnd, hdc);

    oldy2 = pt.y;

    fDragMode2 = FALSE;

    // Convert the splitter position back to screen coords.
    GetWindowRect(hwnd, &rect);
    pt.x += rect.left;
    pt.y += rect.top;

    // Now convert into CLIENT coordinates.
    ScreenToClient(hwnd, &pt);
    GetClientRect(hwnd, &rect);
    // Notice that client nSplitterPos2 is in client coordinates.
    nSplitterPos2 = pt.y; 

    // Position the child controls.
    SizeWindowContents2(rect.right,rect.bottom);

    ReleaseCapture();

    // UI fix to avoid overlapping details LV when splitter is moved down.
    if (g_nRightPaneViewType == 3 /* DC on CAR file (out of default folder) */)
    {
        InvalidateRect(hwndRightBottomHost, NULL, TRUE);
        UpdateWindow(hwndRightBottomHost);
        
        InvalidateRect(hwndBottomLV, NULL, TRUE);
        UpdateWindow(hwndBottomLV);
    }

    return 0;
}

LRESULT Splitter3_OnLButtonUp(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;
    RECT rcClient;

    POINT pt;
    pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
    pt.y = (short)HIWORD(lParam);

    if (fDragMode3 == FALSE)
        return 0;
    
    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;
    
    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    // Get border width.
    WINDOWINFO wi;
    GetWindowInfo(hwnd, &wi);

    // Get y-position of the top of splitter bar. The height of caption, 
    // toolbar ares, and border must be accounted.
    nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) 
                                                        - wi.cyWindowBorders;
    nHeight = rcClient.bottom - rcClient.top;	
    hdc = GetWindowDC(hwnd);

    // Erase the bar.
    DrawXorBar(hdc, oldy3 - 2, nVerOffset, SPLITTER_WIDTH, nHeight);
    
    ReleaseDC(hwnd, hdc);

    fDragMode3 = FALSE;

    //convert the splitter position back to screen coords.
    GetWindowRect(hwnd, &rect);
    pt.x += rect.left;
    pt.y += rect.top;

    //now convert into CLIENT coordinates
    ScreenToClient(hwnd, &pt);
    GetClientRect(hwnd, &rect);
    nSplitterPos3 = pt.x;
    
    //position the child controls
    SizeWindowContents3(rect.right,rect.bottom);

    ReleaseCapture();

    return 0;
}


LRESULT Splitter_OnMouseMove(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;
    RECT rcClient;

    POINT pt;

    if (fDragMode == FALSE) return 0;

    pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;

    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    if (pt.y != oldy && wParam & MK_LBUTTON)
    {
        // Get border width.
        WINDOWINFO wi;
        GetWindowInfo(hwnd, &wi);

        // Get y-position of the top of splitter bar. The height of caption, 
        // toolbar ares, and border must be accounted.
        nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) 
                                           - wi.cyWindowBorders + g_nTBHeightUI;
        nHeight = rcClient.bottom - rcClient.top;	
        hdc = GetWindowDC(hwnd);
        
        // Erase where the bar WAS.
        DrawXorBar(hdc, oldy - 2, nVerOffset, SPLITTER_WIDTH, 
                                       nHeight - g_nTBHeightUI - g_nSBHeightUI); 
        // Draw it where it IS now.
        DrawXorBar(hdc, pt.x - 2, nVerOffset, SPLITTER_WIDTH, 
                                       nHeight - g_nTBHeightUI - g_nSBHeightUI);
        ReleaseDC(hwnd, hdc);

        oldy = pt.x;
    }

    return 0;
}

LRESULT Splitter2_OnMouseMove(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt; // client coordinates
    HDC hdc;
    RECT rect;
    RECT rcClient;
    int nWidth;

    if(fDragMode2 == FALSE) return 0;

    // Cursor position relative to (0,0) of the main client area. 
    pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;

    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom-4) 
    {
        pt.y = rect.bottom-4;
    }

    if (pt.y != oldy2 && wParam & MK_LBUTTON)
    {
        hdc = GetWindowDC(hwnd);
        if (g_nRightPaneViewType == 
                                 3 /* DC on CAR file (out of default folder) */)
        {
            nWidth = rcClient.right - rcClient.left;
            DrawXorBar(hdc, 4, oldy2 - 2, nWidth, SPLITTER_WIDTH);
            DrawXorBar(hdc, 4, pt.y - 2, nWidth, SPLITTER_WIDTH);
        }
        else
        {
            // Erase where the bar WAS.
            DrawXorBar(hdc, 1,oldy2 - 2, rect.right-2, SPLITTER_WIDTH);
            // Draw it where it IS now.
            DrawXorBar(hdc, 1,pt.y - 2, rect.right-2, SPLITTER_WIDTH);
        }
            
        ReleaseDC(hwnd, hdc);

        oldy2 = pt.y;
    }

    return 0;
}

LRESULT Splitter3_OnMouseMove(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT rect;
    int nHeight;
    int nVerOffset;
    RECT rcClient;

    POINT pt;

    if (fDragMode3 == FALSE) return 0;

    pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &rcClient);

    ClientToScreen(hwnd, &pt);
    pt.x -= rect.left;
    pt.y -= rect.top;

    OffsetRect(&rect, -rect.left, -rect.top);

    if (pt.y < 0) pt.y = 0;
    if (pt.y > rect.bottom - 4) 
    {
        pt.y = rect.bottom - 4;
    }

    if (pt.y != oldy3 && wParam & MK_LBUTTON)
    {
        // Get border width.
        WINDOWINFO wi;
        GetWindowInfo(hwnd, &wi);

        // Get y-position of the top of splitter bar. The height of caption, 
        // toolbar ares, and border must be accounted.
        nVerOffset = (rect.bottom - rect.top) - (rcClient.bottom - rcClient.top) 
                                                           - wi.cyWindowBorders;
        nHeight = rcClient.bottom - rcClient.top;	
        hdc = GetWindowDC(hwnd);
    
        // Erase where the bar WAS.
        DrawXorBar(hdc, oldy3 - 2, nVerOffset, SPLITTER_WIDTH, nHeight); 
        // Draw it where it IS now.
        DrawXorBar(hdc, pt.x - 2, nVerOffset, SPLITTER_WIDTH, nHeight);
            
        ReleaseDC(hwnd, hdc);

        oldy3 = pt.x;
    }

    return 0;
}

void CreateImageList(HWND hwndLV, short iImageCount, 
                                                  vector<string> vecImageTitles)
{
    TCHAR szCollectionTitle[100];
    HBITMAP hPic = NULL;
    int iImage;
    LVITEM lvi;

    // ImageList_Create creates a single bitmap.
    hImageList = ImageList_Create(200 /* width */, 200 /* height */, 
        ILC_COLORDDB, //ILC_COLOR (16 colors only), ILC_COLOR24 (OK) 
                                           1 /* initial number of images */, 0);
    ListView_SetImageList(hwndLV, hImageList, LVSIL_NORMAL);

    // Get CAR file title.
    GetImageSetTitle(g_szFullFileName, szCollectionTitle);
    
    for (int i = 0; i < iImageCount; i++) 
    {
        iImage = ImageList_Add(hImageList, LoadPicture(i), NULL );

        lvi.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM; 
        //lvi.iItem    = 0;
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)vecImageTitles.at(i).c_str();
        lvi.iImage   = iImage; 
        lvi.lParam = GetGlobalCarID(szCollectionTitle, i);

        //x = ListView_InsertItem(hwndChild1, &lvi);
        ListView_InsertItem(hwndLV, &lvi);
    }

    // Works here also
    //ListView_SetImageList(hwndChild1, hImageList, LVSIL_NORMAL);

    //MessageBox(0, "ListView_SetImageList", "sasa", 0);

    //hBitmapUp = (HBITMAP)LoadImage(GetModuleHandle(NULL), 
    //	 MAKEINTRESOURCE(IDB_ARROW_UP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    //ImageList_Add(hImageList, hBitmapUp, NULL ); 
}

HBITMAP LoadPicture(short iPicture)
{
    HBITMAP hPic;
    BITMAP info;
    int iThumbnalWidth = 200;
    int iThumbnalHeight = 200;
    COLORREF rgbBorderColor = RGB(192, 192, 192);
    COLORREF rgbBackgroundColor = RGB(100, 100, 100);
    int iStartCanvasWidth = 190;
    int iStartCanvasHeight = 190;
    int iCanvasWidth;
    int iCanvasHeight;
    float fOriginalAspectRatio;
    float fAspectRatio;
    int iHorOffset;
    int iVerOffset;
    //long iImageHeight;
    //long iImageWidth;
    HDC hWindowDC;
    HDC hPictureMemoryDC;
    HDC hdcScaledPictureDC;
    HBITMAP hScaledPictureBitmap;
    HBITMAP hbmPictureOld;
    HBITMAP hbmScaledPictureOld;

    //--------------------------------------------------------------------------
    // Step I load picture into memory bitmap of the same size (get sizes) and scale
    // to size less (or equal) than the canvas.
    
    m_Picture[iPicture].m_spIPicture->get_Handle((UINT*)&hPic);

    // Himetrics units (not used)
    //m_Picture[iPicture].m_spIPicture->get_Width(&iImageWidth);
    //m_Picture[iPicture].m_spIPicture->get_Height(&iImageHeight);

    
    GetObject(hPic, sizeof(info), (PTSTR) &info);

    // Landscape (horizontal) type.
    if (info.bmWidth >= info.bmHeight)
    {
       // Supposedly bmWidth > iCanvasWidth.
       fAspectRatio = (float)(iStartCanvasWidth)/(float)(info.bmWidth); 
    }
    else // portrait (vertical) type.
    {
       // Supposedly bmHeight > iCanvasHeight.
       fAspectRatio = (float)(iStartCanvasHeight)/(float)(info.bmHeight); 
    }

    // There is conversion from float to int here and not essential loss
    // of accuracy.
    iCanvasWidth =  fAspectRatio*(float)info.bmWidth;
    iCanvasHeight =  fAspectRatio*(float)info.bmHeight;
    iHorOffset =  (float)(iThumbnalWidth - iCanvasWidth)/2.;
    iVerOffset = (float)(iThumbnalHeight - iCanvasHeight)/2.;

    //fOriginalAspectRatio = (float)(info.bmWidth)/(float)(info.bmHeight);

    TCHAR szTest[100];
    wsprintf(szTest, "Width: %d, Height: %d", info.bmWidth /* pixels */, info.bmHeight /* pixels */);
    //MessageBox(0, szTest, "Test", 0);

    hWindowDC = GetDC(g_hwndLV);
    hPictureMemoryDC = CreateCompatibleDC(hWindowDC); //0
    hbmPictureOld = (HBITMAP)SelectObject(hPictureMemoryDC, hPic);
    
    // Test (passed OK)
    //HBITMAP bm = (HBITMAP)SelectObject(hPictureMemoryDC, hbmPictureOld);
    //return bm;
    //HIMETRIC: unit is 0.01 millimeter

    hdcScaledPictureDC = CreateCompatibleDC(hWindowDC);
    hScaledPictureBitmap = 
        CreateCompatibleBitmap(hWindowDC /* window (color) DC, not hMemoryDC */, 
                                                   iCanvasWidth, iCanvasHeight);
        //                                                              50, 50); 
    hbmScaledPictureOld = (HBITMAP)SelectObject(hdcScaledPictureDC, 
                                                          hScaledPictureBitmap);

    // Set stretch mode to avoid poor quality., 3000x3000 to have it visible.
    SetStretchBltMode(hdcScaledPictureDC, HALFTONE);
    //StretchBlt(hdcScaledPictureDC, 
    //	0, 0, 5000, 5000,              // destination rectangle, logical units
    //	hPictureMemoryDC, 
    //	0, 0, iImageWidth, iImageHeight, // source rectangle, logical units 
    //	                                                               SRCCOPY); 
    StretchBlt(hdcScaledPictureDC, 
        //0, 0, 50, 50,              // destination rectangle, logical units
        0, 0, iCanvasWidth, iCanvasHeight,
        hPictureMemoryDC, 
        0, 0, info.bmWidth, info.bmHeight, // source rectangle, logical units 
                                                                       SRCCOPY);

    ////HBITMAP bm = (HBITMAP)SelectObject(hPictureMemoryDC, hbmPictureOld);
    //HBITMAP bm = (HBITMAP)SelectObject(hdcScaledPictureDC, hbmScaledPictureOld);
    //return bm;

    // Canvas (200x200), still empty.
    //HBITMAP hbmOld = (HBITMAP)SelectObject(hMemoryDC, hMemoryBitmap);

    // Resulting bitmap is used in ImageList_Create(...). List view image sizes are 
    // ImageList_Create's sizes. I found that bitmap is visible if
    // CopyImage's size is equal (or greater) then ImageList_Create's size.
    //HBITMAP hPicRet = (HBITMAP)CopyImage(hPic, IMAGE_BITMAP, nWidth, nHeight, 
    //	                                                      LR_COPYRETURNORG); //LR_COPYDELETEORG);
    //return hPicRet;

    //--------------------------------------------------------------------------
    // Step II (create canvas of a fixed size and paint it: color plus border)
    HDC hClientDC = GetDC(g_hwndLV);
    HDC hCanvasMemoryDC = CreateCompatibleDC(hClientDC);
    HBITMAP hCanvasMemoryBitmap = 
        CreateCompatibleBitmap(hClientDC /* window (color) DC, not hMemoryDC */, 
                                                                     //200, 200);
                                                iThumbnalWidth, iThumbnalHeight);
    // Canvas (200x200), still empty.
    HBITMAP hbmOld = (HBITMAP)SelectObject(hCanvasMemoryDC, hCanvasMemoryBitmap);

    // Now, when bitmap is in memory DC, we can paint on it.
    HBRUSH hBrushBorder = CreateSolidBrush(rgbBorderColor);
    HBRUSH hBrushBk = CreateSolidBrush(rgbBackgroundColor);

    RECT rc;
    rc.left = 0;
    rc.top  = 0;
    rc.right = 200;
    rc.bottom = 200;

    FillRect(hCanvasMemoryDC, &rc, hBrushBk);
    // MSDN: The width and height of the border are always one logical unit. 
    FrameRect(hCanvasMemoryDC, &rc, hBrushBorder);
    //HBITMAP hBmReturn= (HBITMAP)SelectObject(hMemoryDC, hbmOld);

    //--------------------------------------------------------------------------
    // Step III (copy scaled image to 200x200 canvas):
    //BitBlt(hCanvasMemoryDC, 10, 0, 5000, 5000, hdcScaledPictureDC, 0, 0, SRCCOPY); 
    //BitBlt(hCanvasMemoryDC, 5, 20, 200, 200, hdcScaledPictureDC, 0, 0, SRCCOPY); 
    BitBlt(hCanvasMemoryDC, iHorOffset, iVerOffset, 
            iThumbnalWidth, iThumbnalHeight, hdcScaledPictureDC, 0, 0, SRCCOPY); 

    //HDC hdcSrc = CreateCompatibleDC(NULL);
 //   HDC hdcDst = CreateCompatibleDC(NULL);

    //HBITMAP hbmSrcT = (HBITMAP)SelectObject(hdcSrc, hPic);
 ////   HBITMAP hbmDstT = (HBITMAP)SelectObject(hdcDst, hBmReturn);

    //BitBlt(hdcDst, 0,0,200, 200, hdcSrc,0,0,SRCCOPY); 
    //hbmOld = (HBITMAP)SelectObject(hdcDst,hMemoryBitmap);
    //return hbmOld;


 
/*    hdcWin = GetDC(hwnd); 
    BitBlt(hdcWin, 
                    0,0, 
                    bmp.bmWidth, bmp.bmHeight, 
                    hdcScaled, 
                    0,0, 
                    SRCCOPY); 
    ReleaseDC(hwnd, hdcWin); */ 
    HBITMAP hBmReturn= (HBITMAP)SelectObject(hCanvasMemoryDC, hbmOld);
    
    //HBITMAP hBmReturn;
    return hBmReturn;

    ////GetObject(hPic, sizeof(bm), &bm);



    //
    //
    //
    //HDC hMemoryDC2 = CreateCompatibleDC(hClientDC);
    //BitBlt(hMemoryDC2, 0, 0, 200, 200, hMemoryDC, 0, 0, SRCCOPY);

    //HBITMAP hbmOld2 = (HBITMAP)SelectObject(hMemoryDC2, hbmOld);
 //   DeleteDC(hMemoryDC);




    //////HBITMAP hOldBitmapImage = (HBITMAP)
    ////SelectObject(hMemoryDC, hMemoryBitmap);
    ////
    ////// Draw background and border.
    ////FillRect(hMemoryDC, &rc, hBrushBk);
    ////FrameRect(hMemoryDC, &rc, hBrushBorder);
    //// Load bitmap (image) into memory DC.
    //return hbmOld2;



    //long mWid,mHei;
    ////pPic->get_Height(&mHei);
    ////pPic->get_Width(&mWid);
    //m_Picture[iPicture].m_spIPicture->get_Height(&mHei);
    //m_Picture[iPicture].m_spIPicture->get_Width(&mWid);

    //// Strech the original image to the desired width and height.
    //HBITMAP hPicRet = (HBITMAP)CopyImage(hPic, IMAGE_BITMAP, nWidth, nHeight, 
    //	                                                      LR_COPYDELETEORG);	

    //// Create Brushes for Border and BackGround
    //HBRUSH hBrushBorder = CreateSolidBrush(RGB(192, 192, 192));
    //HBRUSH hBrushBk = CreateSolidBrush(RGB(255, 255, 255));

    //// Border Size
    //RECT rcBorder;
    //rcBorder.left=rcBorder.top=0;
    //rcBorder.right=THUMWIDTH;
    //rcBorder.bottom=THUMHEIGHT;

    //const float fRatio=(float)THUMHEIGHT/THUMWIDTH;

    //int XDest, YDest, nDestWidth, nDestHeight;
    //// Calculate Rect to fit to canvas
    //const float fImgRatio=(float)mHei/mWid;
    //if(fImgRatio > fRatio)
    //{
    //	nDestWidth=(THUMHEIGHT/fImgRatio);
    //	XDest=(THUMWIDTH-nDestWidth)/2;
    //	YDest=0;
    //	nDestHeight=THUMHEIGHT;
    //}
    //else
    //{
    //	XDest=0;
    //	nDestWidth=THUMWIDTH;
    //	nDestHeight=(THUMWIDTH*fImgRatio);
    //	YDest=(THUMHEIGHT-nDestHeight)/2;
    //}

    //////CClientDC cdc(this);
    //////HDC hDC=::CreateCompatibleDC(cdc.m_hDC);
    //HDC hClientDC = GetDC(hwndChild1);
    //HDC hMemoryDC = CreateCompatibleDC(hClientDC);
    //HBITMAP hMemoryBitmap = CreateCompatibleBitmap(hMemoryDC, THUMWIDTH, THUMHEIGHT);
    ////HBITMAP hOldBitmapImage = (HBITMAP)
    //SelectObject(hMemoryDC, hMemoryBitmap);
    //
    //// Draw background and border.
    //FillRect(hMemoryDC, &rcBorder, hBrushBk);
    //FrameRect(hMemoryDC, &rcBorder, hBrushBorder);

    //BitBlt(hClientDC /* destination DC */, 
    // XDest,YDest,nDestWidth, nDestHeight, hMemoryDC /* source DC */,0, 0, SRCCOPY);
    //// Select object into device context.
    ////HBITMAP hBit = (HBITMAP)SelectObject(hClientDC /* original DC */, hMemoryBitmap);
    //// Usually old bitmap into compatible DC.
    //HBITMAP hBit = (HBITMAP)SelectObject(hMemoryDC,  hOldBitmapImage);
    //
 //   //DeleteObject(hMemoryDC);
    ////DeleteObject(hMemoryBitmap);
    //return hBit;
    //return hPic;

    //HBITMAP hBmReturn= (HBITMAP)::SelectObject(hDC, pOldBitmapImage);

    ////CDC hdcSrc, hdcDst;

 ////  hdcSrc.CreateCompatibleDC(NULL);
 ////  hdcDst.CreateCompatibleDC(NULL);
    // HDC hdcSrc = CreateCompatibleDC(NULL);
 //    HDC hdcDst = CreateCompatibleDC(NULL);

 ////  // Load the bitmaps into memory DC
 ////  CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(hPicRet);
 ////  CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(hBmReturn);
    // HBITMAP hbmSrcT = (HBITMAP)::SelectObject(hdcSrc, hPicRet);
    // HBITMAP hbmDstT = (HBITMAP)::SelectObject(hdcDst, hBmReturn);

 //  // This call sets up the mask bitmap.
 //  hdcDst.BitBlt(XDest,YDest,nDestWidth, nDestHeight, &hdcSrc,0,0,SRCCOPY);	
 //  //hdcDst.StretchBlt(XDest,YDest,nDestWidth, nDestHeight, &hdcSrc,0,0,48,48,SRCCOPY);
     //HDC hNewMemoryDC = CreateCompatibleDC(NULL);
     //BitBlt(hNewMemoryDC, XDest,YDest,nDestWidth, nDestHeight, hMemoryDC,0,0,SRCCOPY);	
  //   //StretchBlt(hdcDst, XDest,YDest,nDestWidth, nDestHeight, hdcSrc,0,0,48,48,SRCCOPY);

     //hOldBitmapImage = (HBITMAP)SelectObject(hNewMemoryDC,hMemoryBitmap);

    //// Release used DC and Object
    //DeleteDC(hDC);
    //DeleteObject(hBrushBorder);
    //DeleteObject(hBrushBk);

    //return hOldBitmapImage;
}

LRESULT CALLBACK BottomLeftWindow_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
    
    switch(iMsg)
    {
    case WM_CREATE:
        if (g_nRightPaneViewType == 1 || 
                           g_nRightPaneViewType == 3 /* DC on CAR file (out) */)
            LoadPictureZoomWindow(0);
        else if (g_nRightPaneViewType == 2 /* single picture from favorites */)
            LoadPictureZoomWindow((short)pcs->lpCreateParams); 
        //MessageBox(0, "", "WM_CREATE", 0);
        {
            TCHAR szText[100];
            RECT rcTest; GetClientRect(hwnd, &rcTest);
            wsprintf(szText, "hwndPictureView' RC: (%d %d) - (%d %d)", 
                rcTest.left, rcTest.top, rcTest.right - rcTest.left, 
                rcTest.bottom - rcTest.top);  
            //App_Debug(szText);
        }
        break;	
    case WM_PAINT:
        OnPaintHandler(hwnd);
        //MessageBox(0, "", "WM_PAINT", 0);
        break;
    //case WM_ERASEBKGND:
    //An application should return nonzero if it erases the background; 
    // otherwise, it should return zero. 
    //	//return 0;
    //	break;
    default:
        return DefWindowProc( hwnd, iMsg, wParam, lParam );
    }

    return 0; // never here
}

// Note: I've tried to use CPicture array (already loaded with 
// LoadPictureAndStrings), but this makes image in bottom-left windows 
// unavailable (LL sharing issue). To avoid this I load into separate CPicture 
// (not array) a single selected picture.
BOOL LoadPictureZoomWindow(short iPicture)
{
    HINSTANCE hResInst;
    TCHAR szBuffer[100];        // accepts DLL's image count
    int iImageID; 
    int iImageResourceStartID = 100;

    // Return if run by double-click (no MSS file as argument).
    if (lstrlen(g_szFullFileName) == 0) return FALSE;
    hResInst = LoadLibrary(g_szFullFileName /* <filename>.mss with path */);

    if(hResInst == NULL)
    {
        MessageBox(NULL, "LoadLibrary failed!", "LoadLibrary failed!", MB_OK);
        return FALSE;
    }

    // Load picture with given index.
    iImageID = iImageResourceStartID + iPicture + 1; // Images 101 ... 105 ...
    m_SinglePicture.Load(hResInst, "IMAGES", iImageID);

    FreeLibrary(hResInst);

    return TRUE;
}

// Shows selected picture in bottom-left window (former SHowSelectedPicture).
BOOL OnPaintHandler(HWND hwnd)
{
    HDC hdc;
    RECT rc;
    PAINTSTRUCT ps;
    long hmWidth,hmHeight; // HIMETRIC units
    HDC	hdcIC;
    int iWindowWidth, iWindowHeight;
    int iLeftIndent = 10; // set manually (landscape) or calculated (portrait)
    int	iTopIndent = 10; // set manually (portrait) or calculated (landscape)
    int	iImageWidth_Pixels, iImageHeight_Pixels;
    int	iImageWidthScaled_Pixels, iImageHeightScaled_Pixels;

    float fScale;

    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rc); 
    //FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

    if (lstrlen(g_szFullFileName) == 0) 
    {
        // Mandatory in conjunction with BeginPaint.
        EndPaint(hwnd, &ps);

        return FALSE;
    }


    // Test, remove.
    //m_SinglePicture.m_spIPicture->get_Width(&hmWidth);
    //m_Picture[1].m_spIPicture->get_Width(&hmWidth); // OK
    //m_Picture[1].m_spIPicture->get_Width(&hmHeight);
    //TCHAR szText[10];
    //wsprintf(szText, "%d %d", hmWidth, hmHeight);
    //MessageBox(0, "", szText, 0);
    TCHAR szText[100];
    wsprintf(szText, "OnPaintHandler: %d %d %d %d %d", hwnd, 
        rc.left, rc.top, rc.right,  rc.bottom);
    //MessageBox(0, "", szText, 0);
    //App_Debug(szText);

 // //   Get picture size: IPicture is using metric units (HIMETRIC units) 
    // //(1 himetric unit = 0.01 mm).
    m_SinglePicture.m_spIPicture->get_Width(&hmWidth);  
    m_SinglePicture.m_spIPicture->get_Height(&hmHeight);

    // Convert HIMETRIC to pixel units (LOGPIXELSX: number of pixels per logical 
    // inch along the screen width).
    hdcIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    iImageWidth_Pixels  = (hmWidth  * GetDeviceCaps(hdcIC, LOGPIXELSX)) / 2540;
    iImageHeight_Pixels = (hmHeight * GetDeviceCaps(hdcIC, LOGPIXELSY)) / 2540;
    DeleteDC(hdcIC);

    // Calculate scaling and position of the picture, which is centered, indented, 
    //scaled.
    iWindowWidth = rc.right-rc.left; 
    iWindowHeight = rc.bottom-rc.top;

    // TODO: this works basically OK, but additional cases are when picture is 
    // landscape, but its height is out of window height, and, similary, for 
    // portrait (it does not matter width is inside of window or not for 
    // landscape, anyway it is scaled, similary, for portrait).
    if (iImageWidth_Pixels >= iImageHeight_Pixels) // landscape picture
    {
        // Case 1: ImageWidth >= ImageHeight (1A: heigth is inside)
        fScale = ((float)(iWindowWidth - 2*iLeftIndent))/
                                                    ((float)iImageWidth_Pixels);
        // Scaled sizes.
        iImageWidthScaled_Pixels = (int)(fScale*iImageWidth_Pixels); 
        iImageHeightScaled_Pixels = (int)(fScale*iImageHeight_Pixels);

        // Calculate vertical top indent (horizontal is set manually).
        iTopIndent = (iWindowHeight - iImageHeightScaled_Pixels)/2;
    }
    else
    {
        // Case 2: ImageWidth < ImageHeight (2A: width is inside)
        fScale = ((float)(iWindowHeight - 2*iTopIndent))/
                                                   ((float)iImageHeight_Pixels);
         
        // Scaled sizes.
        iImageWidthScaled_Pixels = (int)(fScale*iImageWidth_Pixels); 
        iImageHeightScaled_Pixels = (int)(fScale*iImageHeight_Pixels);

        // Calculate vertical top indent (horizontal is set manually).
        iLeftIndent = (iWindowWidth - iImageWidthScaled_Pixels)/2;
    }

    // Now draw picture (Render means draw).
    m_SinglePicture.m_spIPicture->Render(hdc, 
    //m_Picture[1].m_spIPicture->Render(hdc, 
                                                    // Destination picture (pixels)
                                                    iLeftIndent, 
                                                    iTopIndent, 
                                                    iImageWidthScaled_Pixels, 
                                                    iImageHeightScaled_Pixels,
                                                    // Source picture (himetric units)
                                                    0, hmHeight, hmWidth, -hmHeight, NULL);
                                                    //0, 0, hmWidth, hmHeight, NULL); // 180 degrees rotated (top-down), OK 

    // Mandatory in conjunction with BeginPaint.
    EndPaint(hwnd, &ps);

    return TRUE;
}

// Based on MyScreenSaverBuilder's CMySaverPropPage1::LoadPicturesAndStrings. LoadXMLData,
// in fact, is used also for loading IDS_NOTESX from general MSS (not car MSS).
BOOL LoadXMLData(short iPicture)
{
    HINSTANCE hResInst;
    TCHAR szTitle[100];
    TCHAR szNote[260]; // reserved
    // Main.
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szModel[100];
    TCHAR szSubmodel[100];
    TCHAR szType[100];
    TCHAR szPrice[100];
    TCHAR szPower[100];
    TCHAR szFuel[100];
    // Physical dimensions.
    TCHAR szLength[100];
    TCHAR szWidth[100];
    TCHAR szHeight[100];
    TCHAR szWeight[100];
    TCHAR szClearance[100];
    // Misc
    TCHAR szCountry[100];
    TCHAR szTankVolume[100];

    LVITEM lvi;
    int iInsertedItem;

    // Return if run by double-click (no MSS file as argument).
    if (lstrlen(g_szFullFileName) == 0) return FALSE;
    hResInst = LoadLibrary(g_szFullFileName /* <filename>.mss with path */);

    if(hResInst == NULL)
    {
        MessageBox(NULL, "LoadLibrary failed!", "LoadLibrary failed!", MB_OK);
        return FALSE;
    }

    //if (g_iPicturesCategory == 0 /* "Not specified" */)
    //{
    //	// Notes 401 ... 405 ... Notes are reserved: IDS_NOTESX data are empty 
    //	// if created with wizard. The wizard does not provide, currently, a 
    //	// functionality to write to IDS_NOTESX resource, notes can be added to 
    //	// MSS by 3d-party programs.
    //	LoadString(hResInst,  401 + iPicture, szNote, 
    //									      sizeof(szNote)/sizeof(szNote[0]));
    //	SendMessage(hwndBottomChild2, WM_SETTEXT, 
    //		                                  0 /* not used */, (LPARAM)szNote);
    //}
    //else if (g_iPicturesCategory == 1 /* Cars */)
    {
        // Titles 201 ... 205 ...
        LoadString(hResInst, 201 + iPicture, szTitle, 
                                            sizeof(szTitle)/sizeof(szTitle[0]));
        // Load year string.
        LoadString(hResInst, 1000 /* year ID (same for car MSS) */, szYear, 
                                              sizeof(szYear)/sizeof(szYear[0]));
        // Load make string.
        LoadString(hResInst, 1100 /* make ID (same for car MSS) */, szMake, 
                                              sizeof(szMake)/sizeof(szMake[0]));
        // Load make string.
        LoadString(hResInst, 1201 /* model start ID */ + iPicture, szModel, 
                                            sizeof(szModel)/sizeof(szModel[0]));
        // Load submodel string.
        LoadString(hResInst, 1301 /* submodel start ID */ + iPicture, 
                          szSubmodel, sizeof(szSubmodel)/sizeof(szSubmodel[0]));
                                    
        // Load type string.
        LoadString(hResInst, 1401 /* type start ID */ + iPicture, szType, 
                                              sizeof(szType)/sizeof(szType[0]));
        // Load price string.
        LoadString(hResInst, 1501 /* price start ID */ + iPicture, szPrice, 
                                            sizeof(szPrice)/sizeof(szPrice[0]));
        // Load price string.
        LoadString(hResInst, 1601 /* power start ID */ + iPicture, szPower, 
                                            sizeof(szPower)/sizeof(szPower[0]));
        // Load fuel string.
        LoadString(hResInst, 1701 /* fuel start ID */ + iPicture, szFuel, 
                                              sizeof(szFuel)/sizeof(szFuel[0]));
        // Load length string.
        LoadString(hResInst, 1801 /* length start ID */ + iPicture, szLength, 
                                          sizeof(szLength)/sizeof(szLength[0]));
        // Load width string.
        LoadString(hResInst, 1901 /* width start ID */ + iPicture, szWidth, 
                                            sizeof(szWidth)/sizeof(szWidth[0]));
        // Load height string.
        LoadString(hResInst, 2001 /* height start ID */ + iPicture, szHeight, 
                                          sizeof(szHeight)/sizeof(szHeight[0]));
        // Load weight string.
        LoadString(hResInst, 2101 /* weight start ID */ + iPicture, szWeight, 
                                          sizeof(szWeight)/sizeof(szWeight[0]));
        // Load clearance string.
        LoadString(hResInst, 2201 /* clearance start ID */ + iPicture, 
                       szClearance, sizeof(szClearance)/sizeof(szClearance[0]));
        // Load country string.
        LoadString(hResInst, 2300 /* country ID (same for car MSS) */, 
                             szCountry, sizeof(szCountry)/sizeof(szCountry[0]));
        // Load tank volume string.
        LoadString(hResInst, 2401 /* tank volume start ID */ + iPicture, 
                    szTankVolume, sizeof(szTankVolume)/sizeof(szTankVolume[0]));

        ListView_DeleteAllItems(hwndBottomLV);
        
        // Populate list view. Notice that 3d column is filled out with account 
        // for g_iUnits: US (in/lbs/gallons) when 0 and Metric (mm/kg/liters) 
        // when 1.
        lvi.mask = LVIF_TEXT | LVIF_STATE; 
        lvi.state = 0; 

        lvi.iItem = 0;
        lvi.iSubItem = 0;
        lvi.pszText = "Car";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szTitle);

        lvi.iItem = 1;
        lvi.iSubItem = 0;
        lvi.pszText = "Year";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szYear);

        lvi.iItem = 2;
        lvi.iSubItem = 0;
        lvi.pszText = "Make";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szMake);

        lvi.iItem = 3;
        lvi.iSubItem = 0;
        lvi.pszText = "Model";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szModel);

        lvi.iItem = 4;
        lvi.iSubItem = 0;
        lvi.pszText = "Submodel";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szSubmodel);

        lvi.iItem = 5;
        lvi.iSubItem = 0;
        lvi.pszText = "Type";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szType);

        lvi.iItem = 6;
        lvi.iSubItem = 0;
        lvi.pszText = "Price";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szPrice);
        if (lstrcmp(szPrice, "No data") != 0 && lstrcmp(szPrice, ""))
                ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, "USD");

        lvi.iItem = 7;
        lvi.iSubItem = 0;
        lvi.pszText = "Power";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szPower);
        if (lstrcmp(szPower, "No data") != 0 && lstrcmp(szPower, ""))
                 ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, "hp");

        lvi.iItem = 8;
        lvi.iSubItem = 0;
        lvi.pszText = "Fuel";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szFuel);

        lvi.iItem = 9;
        lvi.iSubItem = 0;
        lvi.pszText = "Length";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szLength);
        if (lstrcmp(szLength, "No data") != 0 && lstrcmp(szLength, ""))
            ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                                       g_iUnits ?  "mm" : "in");
        lvi.iItem = 10;
        lvi.iSubItem = 0;
        lvi.pszText = "Width";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szWidth);
        if (lstrcmp(szWidth, "No data") != 0 && lstrcmp(szWidth, ""))
            ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                                       g_iUnits ?  "mm" : "in");
        lvi.iItem = 11;
        lvi.iSubItem = 0;
        lvi.pszText = "Height";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szHeight);
        if (lstrcmp(szHeight, "No data") != 0 && lstrcmp(szHeight, ""))
            ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                                       g_iUnits ?  "mm" : "in");
        lvi.iItem = 12;
        lvi.iSubItem = 0;
        lvi.pszText = "Weight";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szWeight);
        if (lstrcmp(szWeight, "No data") != 0 && lstrcmp(szWeight, ""))
            ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                                        g_iUnits ? "kg" : "lb");
        lvi.iItem = 13;
        lvi.iSubItem = 0;
        lvi.pszText = "Clearance";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szClearance);
        if (lstrcmp(szClearance, "No data") != 0 && lstrcmp(szClearance, ""))
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                                       g_iUnits ?  "mm" : "in");
        lvi.iItem = 14;
        lvi.iSubItem = 0;
        lvi.pszText = "Country";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szCountry);

        lvi.iItem = 15;
        lvi.iSubItem = 0;
        lvi.pszText = "Tank Volume";
        iInsertedItem = ListView_InsertItem(hwndBottomLV, &lvi);
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 1, szTankVolume);
        if (lstrcmp(szTankVolume, "No data") != 0 && lstrcmp(szTankVolume, ""))
        ListView_SetItemText(hwndBottomLV, iInsertedItem, 2, 
                                               g_iUnits ? "liters" : "gallons");

        FreeLibrary(hResInst);

        // Convert to metric units if such settings (there is no need to use 
        // conversion for US units since they are default for car MSS).
        if (g_iUnits == 1) ConvertXMLData(g_iUnits);	
    }
}

short GetPicturesCategory()
{
    HINSTANCE hResInst;
    TCHAR szBuffer[100];
    short iPicturesCategory = 0; // indicates category for v1.1

    // Return if run by double-click (no MSS file as argument).
    if (lstrlen(g_szFullFileName) == 0) return 0; // general 
    hResInst = LoadLibrary(g_szFullFileName /* <filename>.mss with path */);

    if (hResInst == NULL)
    {
        MessageBox(0, "LoadLibrary failed!", "MSS File Viewer", 0);
    }

    LoadString(hResInst, 4 /* pictures category ID */, szBuffer, 
                                          sizeof(szBuffer)/sizeof(szBuffer[0]));

    if (lstrcmp(szBuffer, "Not specified") == 0 /* equal */)
    {
        iPicturesCategory = 0;
    }
    else if (lstrcmp(szBuffer, "Cars") == 0) 
    {
        iPicturesCategory = 1;
    }

    FreeLibrary(hResInst);

    return iPicturesCategory;
}

// Based on function from MySaverPropPage0.cpp.
void OnFileOpen()
{
    OPENFILENAME	ofn;
    // Serves as initial directory and resulting file returned by the dialog.
    TCHAR			szMSSFile[MAX_PATH]; 
    RECT rc;
    TCHAR szCurDir[MAX_PATH];
    HTREEITEM hItem;
    HMENU hMenu;
    HMENU hSubMenu;

    // Avoid arbitrary symbols in File Name on dialog activation.
    ZeroMemory(szMSSFile, sizeof(szMSSFile)); 

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= g_hWnd;
    ofn.hInstance			= NULL;
    ofn.lpstrFilter			= _T("Image Collections (*.car)\0\0"); //(*.car;*.mss) 
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    // Older (98/Me) ofn.lpstrInitialDir can be used to set initial directory, 
    // but ofn.lpstrFile parameter can be used instead since it serves two 
    // purposes (and supposed to be used mainly for setting initial directory 
    // with 2000/XP): set initial directory and receive selected full path name. 
    // I found that when setting initial directory ofn.lpstrFile works more 
    // stable (XP), but ofn.lpstrInitialDir can fail (FNERR_INVALIDFILENAME).
    ofn.lpstrFile			= szMSSFile; // gets selected full file path
    ofn.nMaxFile			= MAX_PATH;
    ofn.lpstrFileTitle		= NULL;
    ofn.nMaxFileTitle		= 0;
    ofn.lpstrTitle			= _T("Select CAR file");
    ofn.Flags				= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; 
    ofn.nFileOffset			= 0;
    ofn.nFileExtension		= 0;
    ofn.lpstrDefExt			= NULL;
    ofn.lCustData			= 0;
    ofn.lpfnHook			= NULL;
    ofn.lpTemplateName		= NULL;

    if (GetOpenFileName(&ofn))
    {
        lstrcpy(g_szFullFileName,  szMSSFile);

        GetCurrentDirectory(MAX_PATH, szCurDir);
        //MessageBox(0, g_szFullFileName, szCurDir, 0); 

        if (lstrcmp(szCurDir, g_szCarFilesFolder) == 0 /* equal */)
        {
            //g_bShowTV = TRUE; 
            // Get TV item based on FFN.
            hItem = GetTVItemFromFFN(g_szFullFileName);
            TreeView_SelectItem(hwndBrowser, hItem);
        }
        else
        {
            // Remove basic layout.
            DestroyWindow(hwndTopArea);
            DestroyWindow(hwndBrowser);
            DestroyWindow(hwndMainLV);
            if (IsWindow(g_hwndLV)) DestroyWindow(g_hwndLV);
            if (IsWindow(hwndRightHost)) DestroyWindow(hwndRightHost);
            if (IsWindow(g_hwndWelcome)) DestroyWindow(g_hwndWelcome);

            // Cleanup all data vectors.
            m_vecCarFilesMap.clear();
            m_vecCarFavoritesMap.clear();
            m_vecCarData.clear();

            // TODO: g_nRightPaneViewType indicates originally DC on CAR file 
            // (out), now it is shared with File/Open... (essentially the same). 
            // Make own variable for File/Open i.e. avoid sharing the variable 
            // avoid using any in this mode if possible.
            g_nRightPaneViewType = 3; // helps provide right layout

            // Disable always in CE v1.0 File/Open... menu for the single CAR mode.
            hMenu = GetMenu(g_hWnd);
            hSubMenu = GetSubMenu(hMenu, 0);
            EnableMenuItem(hSubMenu, ID_FILE_OPEN, MF_BYCOMMAND | MF_GRAYED);

            // Update TB icons.
            TB_SetSingleCARFileIcons();

            // Create layout the single CAR mode. 
            InitSingleCARFileViewer(g_hWnd);
            //SendMessage(g_hWndSplashScreen, WM_CLOSE, 0, 0);
            //ReleaseCapture();
            
            // Update cursor for the horizontal splitter.
            SetClassLong(g_hWnd, GCL_HCURSOR, 
                (LONG)LoadCursor(NULL /* essential to see cursor */, 
                                                   MAKEINTRESOURCE(IDC_SIZENS)));
            // Show collection information in the status bar.
            SB_ShowCollectionInfo(g_szFullFileName);
            
            UpdateLayout();
        }
    }
}

LRESULT CALLBACK SetFolder_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    RECT rc;
    RECT rcMainWnd;
    TCHAR szFDN[MAX_PATH];
    TCHAR szErrorMsg[MAX_PATH];

    switch (message)
    {
    case WM_INITDIALOG:
        // Center dialog relative to the application window.
        GetWindowRect(g_hWnd, &rcMainWnd);
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

        SendMessage(GetDlgItem(hDlg, IDC_SF_EDIT), WM_SETTEXT, 
                                  0 /* not used */, (LPARAM)g_szCarFilesFolder);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) 
        {
            SendMessage(GetDlgItem(hDlg, IDC_SF_EDIT), WM_GETTEXT, 
                                                       MAX_PATH, (LPARAM)szFDN);

            // Test if directory exists before using it.
            if (GetFileAttributes(szFDN) == INVALID_FILE_ATTRIBUTES)
            {
                wsprintf(szErrorMsg, "%s \"%s\" %s", "The directory", 
                                         g_szCarFilesFolder, "does not exist!"); 
                MessageBox(0, szErrorMsg, szMBTitle, 
                                                    MB_OK | MB_ICONINFORMATION);
            }
            else // directory exists
            {
                // When it is actualy a new directory, remove old favorites 
                // prompting for confirmation. Notice that removal is made 
                // to avoid crash due mismatches of old favorites with contents 
                // of the new folder. One way to avoid removal is scan favorites
                // and mark some as broken links.
                if (lstrcmp(szFDN, g_szCarFilesFolder) != 0 /* equal */)
                {
                    TCHAR szMsg[MAX_LOADSTRING];
                    TCHAR szData[MAX_LOADSTRING];
                    int nRes;

                    if (m_vecCarFavoritesMap.size() > 0)
                    {
                        LoadString(hInst, IDS_SF_REMOVE_FAVORITES, szMsg, MAX_LOADSTRING);
                        nRes = MessageBox(0, szMsg, szMBTitle, MB_OKCANCEL);
                    
                        // Prompt a message before deletion of favorites.
                        if (nRes == IDOK)
                        {
                            // Persist new location on Close.
                            g_bPersistCarFilesFolder = TRUE;
                            
                            m_vecCarFavoritesMap.clear();
                            DeleteFavoritesFromRegistry();

                            lstrcpy(g_szCarFilesFolder, szFDN);

                            // Reload TV and LV clearing and remakind data vectors beforehand.
                            ReloadData();
                            EndDialog(hDlg, LOWORD(wParam));
                        }
                    }
                    else
                    {
                            // Persist new location on Close.
                            g_bPersistCarFilesFolder = TRUE;

                            lstrcpy(g_szCarFilesFolder, szFDN);	

                            // Reload TV and LV clearing and remakind data vectors beforehand.
                            ReloadData();
                            EndDialog(hDlg, LOWORD(wParam));
                    }
                }
            }
            
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        if (LOWORD(wParam) == IDC_SF_BUTTON)
        {
            SelectFolder(hDlg);
        }
        break;
    }

    return FALSE;
}

LRESULT CALLBACK AddFolder_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    RECT rc;
    RECT rcMainWnd;
    TCHAR szNewFolder[MAX_PATH];
    TCHAR szErrorMsg[MAX_PATH];
    static HTREEITEM hItemSelected; // static retains variable in WM_COMMAND
    TVITEM tviSelected;
    TVITEM tviNew;
    TVINSERTSTRUCT tvis; 
    FavoritesFolders ff;
    HTREEITEM hTVItem;
    TCHAR szMsg[MAX_LOADSTRING];
    string sNewFolder, sSelectedFolder;

    switch (message)
    {
    case WM_INITDIALOG:
        // Center dialog relative to the application window.
        GetWindowRect(g_hWnd, &rcMainWnd);
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        SetFocus(GetDlgItem(hDlg, IDC_ADDFOLDER_EDIT));
        hItemSelected = (HTREEITEM)lParam;

        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) 
        {
            SendMessage(GetDlgItem(hDlg, IDC_ADDFOLDER_EDIT), WM_GETTEXT, 
                                                 MAX_PATH, (LPARAM)szNewFolder);
            sNewFolder = szNewFolder;

            // Validate: the name name must be at least one-character-string.
            if (sNewFolder.length() == 0)
            {
                LoadString(hInst, IDS_TV_ZEROLENTHWARNING, szMsg, 
                                                                MAX_LOADSTRING);
                MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);
                return FALSE;
            }

            // Validate: the folder name must not contain reserved "," and "|" 
            // symbols.
            if (sNewFolder.find(",") != string::npos || 
                                           sNewFolder.find("|") != string::npos)
            {
                LoadString(hInst, IDS_TV_FORBIDDENSYMBOLS, szMsg, 
                                                                MAX_LOADSTRING);
                MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);
                return FALSE;
            }
            
            // Get the name of selected folder.
            char szBuf[MAX_PATH + 1] = "";
            ZeroMemory(&tviSelected, sizeof(tviSelected));
            tviSelected.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            tviSelected.hItem = hItemSelected;
            tviSelected.pszText = szBuf;
            tviSelected.cchTextMax = MAX_PATH+1;
            TreeView_GetItem(hwndBrowser, &tviSelected);
            sSelectedFolder = tviSelected.pszText;
            
            // Prepare new item (folder) for insetion into TV.
            ZeroMemory(&tviNew, sizeof(tviNew));
            tviNew.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            tviNew.pszText = szNewFolder; 
            tviNew.iImage  = 8;        // user folder
            tviNew.iSelectedImage = 9; // user open folder 
            
            tvis.hParent = hItemSelected; 
            tvis.hInsertAfter = TVI_LAST; 

            // Insert new item if not exists under selected parent.
            if (tviSelected.lParam == 0 /* favorites root selected */)
            {
                if (!IsFolderEnumerated("" /* favorites root*/, szNewFolder))
                {
                    tviNew.lParam = 1;
                    tvis.item = tviNew;
                    hTVItem = TreeView_InsertItem(hwndBrowser, &tvis);
                    ff.bStandard = 0; // user folder
                    ff.hTVItem = hTVItem;
                    ff.sParentFolder = ""; // favorites root 
                    ff.sFolder = szNewFolder;
                    vecFavoritesFolders.push_back(ff);

                    // Set flag to persist change to registry on application 
                    // close.
                    g_bUpdateRegistry = TRUE;
                }
                else
                {
                    LoadString(hInst, IDS_TV_FOLDERALREADYEXISTS, szMsg, 
                                                                MAX_LOADSTRING);
                    MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);
                }
            }

            if (tviSelected.lParam == 1 /* 1st level folder selected */)
            {
                if (!IsFolderEnumerated(sSelectedFolder, szNewFolder))
                {
                    tviNew.lParam = 2;
                    tvis.item = tviNew;
                    hTVItem = TreeView_InsertItem(hwndBrowser, &tvis);
                    ff.bStandard = 0; // user folder
                    ff.hTVItem = hTVItem;
                    ff.sParentFolder = sSelectedFolder; 
                    ff.sFolder = szNewFolder;
                    vecFavoritesFolders.push_back(ff);

                    // Set flag to persist change to registry on application 
                    // close.
                    g_bUpdateRegistry = TRUE;
                }
                else
                {
                    LoadString(hInst, IDS_TV_FOLDERALREADYEXISTS, szMsg, 
                                                                MAX_LOADSTRING);
                    MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);					
                }
            }

            EndDialog(hDlg, LOWORD(wParam));

            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        break;
    }

    return FALSE;
}

void ReloadData()
{
    TVITEM tvi;

    // Cleanup all data vectors.
    m_vecCarFilesMap.clear();
    m_vecCarFavoritesMap.clear();
    vecFavoritesFolders.clear(); 
    m_vecCarData.clear();

    // Cleanup UI of TV and LV.
    TreeView_DeleteAllItems(hwndBrowser);
    ListView_DeleteAllItems(hwndMainLV);

    // Make new data vector for the TV.
    EnumerateCarFiles();
    PopulateTreeView(hwndBrowser, g_nGroup); 
    App_LoadRegistrySettings(); // loads favorites

    // Make new data vector for the LV.
    LoadAllCarData(g_szCarFilesFolder);
    InitListViewItems(hwndMainLV);

    // When no favorites, set grey Favorites icon for TV.
    if (!TreeView_GetChild(hwndBrowser, 
                                g_htviFavorites /* Favorites folder */))
    {
        tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
        tvi.hItem = g_htviFavorites;
        tvi.iImage = 5; // grey 
        tvi.iSelectedImage = 5; // grey
        TreeView_SetItem(hwndBrowser, &tvi);
    }
}

LRESULT CALLBACK RegisterProduct_WndProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    RECT rc;
    RECT rcMainWnd;
    TCHAR szFFN[MAX_PATH];

    switch (message)
    {
    case WM_INITDIALOG:
        // Center dialog relative to the application window.
        GetWindowRect(g_hWnd, &rcMainWnd);
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) 
        {
            TCHAR szEmail[100];
            TCHAR szAC[100];
            SendMessage(GetDlgItem(hDlg, IDC_RP_EDIT_EMAIL), WM_GETTEXT, 
                                                     MAX_PATH, (LPARAM)szEmail);
            SendMessage(GetDlgItem(hDlg, IDC_RP_ACTIVATION_CODE), WM_GETTEXT, 
                                                        MAX_PATH, (LPARAM)szAC);
            CRegisterProduct m_RP;
            //if (m_RP.IsValidRC(szEmail, szAC))
            //	MessageBox(0, "", "Valid AC", 0);
            //else
   //             MessageBox(0, szEmail, szAC, 0);
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        //if (LOWORD(wParam) == IDC_SF_BUTTON)
        //{
  //          SelectFolder(hDlg);
        //}
        break;
    }

    return FALSE;
}

LRESULT CALLBACK RegisterBuyProduct_WndProc(HWND hDlg, UINT message, 
                                                   WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    RECT rcMainWnd;
    TCHAR szFFN[MAX_PATH];
    HIMAGELIST hIL, hIL_Activate, hIL_Close, hIL_OK; 
    HICON hIcon;
    RECT rcMargin = {3,1,0,0};
    //RECT rcMargin_Close = {3,1,0,0};
    BUTTON_IMAGELIST bil;

    switch (message)
    {
    case WM_INITDIALOG:
        g_nCloseButton_TimerID = 777; // arbitrary ID used to terminate timer
        //Note that one more second relates to UI settings that in the first 
        // run Close button's text is "Close", on the next runs is "Close (#).
        g_nRBD_CloseButtonMeter = RBP_CLOSE_BUTTON_TIMEOUT/1000 + 1 ; // sec

        // Make bold the main line with text (will be also colored with 
        // WM_CTLCOLORSTATIC).
        App_SetFont(GetDlgItem(hDlg, IDC_RBP_STATIC), TRUE);

        // Set image for Buy button.

           // Use image list and BCM_SETIMAGELIST to insert icon for XP/Vista. 
        hIL = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 
                                           0 /* number of initial images */, 0);

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_BUY)); 
        ImageList_AddIcon(hIL, hIcon);
        DeleteObject(hIcon);
        
        // Prepare BUTTON_IMAGELIST structure.
        bil.himl = hIL;
        bil.margin = rcMargin;
        bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        // Put image over the button.
        SendMessage(GetDlgItem(hDlg, IDC_RBP_BUTTON_BUY),  BCM_SETIMAGELIST, 0, 
                                               (LPARAM)(PBUTTON_IMAGELIST)&bil);
        
        // Set image for Activate button.
        hIL_Activate = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 
                                           0 /* number of initial images */, 0);

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_ACTIVATE)); 
        ImageList_AddIcon(hIL_Activate, hIcon);
        DeleteObject(hIcon);
        
        // Prepare BUTTON_IMAGELIST structure.
        bil.himl = hIL_Activate;
        bil.margin = rcMargin;
        bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        // Put image over the button.
        SendMessage(GetDlgItem(hDlg, IDC_RBP_BUTTON_ACTIVATE), BCM_SETIMAGELIST, 
                                            0, (LPARAM)(PBUTTON_IMAGELIST)&bil);
        // Set images for Close button.
        hIL_Close = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, // 32, 32,
                                        0 /* number of initial images */, 0);
        // Normal state.
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);

        // Hot state.
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);

        // Pressed state.
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);

        // Disabled state.
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE_GREY)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);

        // Defaulted state.
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);
     
        // Not used (tablet devices only).
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_CLOSE_GREY)); 
        ImageList_AddIcon(hIL_Close, hIcon);
        DeleteObject(hIcon);
        
        // Prepare BUTTON_IMAGELIST structure.
        bil.himl = hIL_Close;
        bil.margin = rcMargin; //rcMargin_Close;
        bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        SendMessage(GetDlgItem(hDlg, IDC_RBP_BUTTON_CLOSE), BCM_SETIMAGELIST, 0, 
                                               (LPARAM)(PBUTTON_IMAGELIST)&bil);

        // Set image for OK button (not visible in the contracted state). I 
        // don't use moved Activate button in expanded RBD instead of OK button
        // since they invoke different actions in contracted/expanded states.
        hIL_OK = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 
                                           0 /* number of initial images */, 0);

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RBP_ACTIVATE)); 
        ImageList_AddIcon(hIL_OK, hIcon);
        DeleteObject(hIcon);
        
        // Prepare BUTTON_IMAGELIST structure.
        bil.himl = hIL_OK;
        bil.margin = rcMargin;
        bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        SendMessage(GetDlgItem(hDlg, IDOK), BCM_SETIMAGELIST, 
                                            0, (LPARAM)(PBUTTON_IMAGELIST)&bil);
        
        EnableWindow(GetDlgItem(hDlg, IDC_RBP_BUTTON_CLOSE), FALSE);

        // Notice that when hWnd parameter of SetTimer is valid, the 2nd 
        // parameter (arbitrary value) to terminate timer, not the return value.
        SetTimer(GetDlgItem(hDlg, 
            IDC_RBP_BUTTON_CLOSE), g_nCloseButton_TimerID, 
            1000 /* milliseconds */, CloseButton_TimerProc);

        // Modify font for edit controls.
        App_SetFont(GetDlgItem(hDlg, IDC_RBP_EDIT_EMAIL), TRUE);
        App_SetFont(GetDlgItem(hDlg, IDC_RBP_ACTIVATION_CODE), TRUE);

        // Set dialog to the contracted state.
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, 
            160 /* contracted height (original), manually adjuststed */, 
                                                        SWP_NOZORDER | SWP_NOMOVE);

        // Center dialog relative to the application window.
        GetWindowRect(g_hWnd, &rcMainWnd);
        GetWindowRect(hDlg, &rc);
        SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        break;
    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_RBP_STATIC)) 
        {
            SetTextColor((HDC) wParam, RGB(128, 0, 0));
            SetBkMode((HDC)wParam, TRANSPARENT);
            //return ((LRESULT)(HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE)));
            return (BOOL)GetStockObject(HOLLOW_BRUSH);
        }
    case WM_CTLCOLOREDIT:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_RBP_ACTIVATION_CODE) ||
            (HWND)lParam == GetDlgItem(hDlg, IDC_RBP_EDIT_EMAIL)) 
        {
            SetTextColor((HDC) wParam, RGB(255, 255, 255));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return ((LRESULT)(HBRUSH)CreateSolidBrush(RGB(0, 0, 0)));
        }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_RBP_BUTTON_BUY) 
        {
            ShellExecute(NULL, "open", "http://order.kagi.com/?6FBHN&lang=en", 
                                                     NULL, NULL, SW_SHOWNORMAL);
        }

        if (LOWORD(wParam) == IDC_RBP_BUTTON_ACTIVATE) 
        {
            // Set the text limits for edit controls. Notice that it actually 
            // sets limit for paste since UI limits the test e.g. for email to
            // about 40 characters (even without EM_SETLIMITTEXT).
            SendMessage(GetDlgItem(hDlg, IDC_RBP_EDIT_EMAIL), EM_SETLIMITTEXT, 
                                                      50, 0 /* must be zero */);
            SendMessage(GetDlgItem(hDlg, IDC_RBP_ACTIVATION_CODE), 
                                    EM_SETLIMITTEXT, 100, 0 /* must be zero */);
            // Set focus (active cursor) for Email.
            SetFocus(GetDlgItem(hDlg, IDC_RBP_EDIT_EMAIL));
            // Hide activate button.
            ShowWindow(GetDlgItem(hDlg, IDC_RBP_BUTTON_ACTIVATE), FALSE);
            //EnableWindow(GetDlgItem(hDlg, IDOK), FALSE); // DNW
            
            // Reposition Buy button.
            GetClientRect(hDlg, &rc);
            MoveWindow(GetDlgItem(hDlg, IDC_RBP_BUTTON_BUY), 
                ((rc.right - rc.left) - 90 /* Buy button width */)/2, // center
                                                              78, 90, 42, TRUE);
            // Reposition OK and Close buttons. 
            //TODO: repositioning OK button is redundant, position right on onit
            MoveWindow(GetDlgItem(hDlg, IDOK), 140, 250, 90, 41, TRUE); // 155
            //GetClientRect(GetDlgItem(hDlg, IDC_RBP_BUTTON_CLOSE), &rc);
            MoveWindow(GetDlgItem(hDlg, IDC_RBP_BUTTON_CLOSE), 300, 250, 90, // 285 
                                                                      41, TRUE);
            // Expand dialog.
            GetWindowRect(hDlg, &rc);
            SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, 
                333 /* expanded height, manually adjusted */, 
                                                     SWP_NOZORDER | SWP_NOMOVE);
            // Center dialog relative to the application window.
            GetWindowRect(g_hWnd, &rcMainWnd);
            GetWindowRect(hDlg, &rc);
            SetWindowPos(hDlg, NULL, rcMainWnd.left + ((rcMainWnd.right - 
            rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
            ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        }

        if (LOWORD(wParam) == IDC_RBP_BUTTON_CLOSE) 
        {
            //ImageList_Destroy(hIL);
            EndDialog(hDlg, 0 /* does not matter */);
            //EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        if (LOWORD(wParam) == IDOK) 
        {
            TCHAR szEmail[MAX_LOADSTRING]; // 21 character (default user)
            TCHAR szAC[MAX_LOADSTRING]; // 53 characters
            BOOL bEmailEmpty = FALSE;
            BOOL bACEmpty = FALSE;
            TCHAR szMsg[MAX_LOADSTRING];
            TCHAR szEncryptedEmail[MAX_PATH]; // 74 characters (default user)
            TCHAR szEncryptedAC[MAX_PATH]; // 159 characters 
            CRegisterProduct m_RP;

            SendMessage(GetDlgItem(hDlg, IDC_RBP_EDIT_EMAIL), WM_GETTEXT, 
                                                     MAX_PATH, (LPARAM)szEmail);
            SendMessage(GetDlgItem(hDlg, IDC_RBP_ACTIVATION_CODE), WM_GETTEXT, 
                                                        MAX_PATH, (LPARAM)szAC);

            // Validate input by discarding an empty strings.
            if (lstrlen(szEmail) == 0) bEmailEmpty = TRUE; 
            if (lstrlen(szAC) == 0) bACEmpty = TRUE; 

            if (bEmailEmpty && !bACEmpty)			
            {
                MessageBox(0, "The email must not be empty!", szMBTitle, 
                                                    MB_OK | MB_ICONINFORMATION);
                return FALSE;
            }
            else if (bACEmpty && !bEmailEmpty)
            {
                MessageBox(0, "The activation code must not be empty!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION); 
                return FALSE;
            }
            else if (bEmailEmpty && bACEmpty)
            {
                MessageBox(0, 
                    "The email and activation code must not be empty!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
                return FALSE;
            }
       
            // Validation for empty strings is passed, valisdate if email/AC are
            // correct.
            if (m_RP.IsValidRC(szEmail, szAC))
            {
                EncryptString(szEncryptedEmail, szEmail);
                StoreTextToRegistry(REGKEY_CE, "User", szEncryptedEmail);
                EncryptString(szEncryptedAC, szAC);
                StoreTextToRegistry(REGKEY_CE, "RegKey", szEncryptedAC);
                // Update string for About dialog.
                lstrcpy(g_szAD_Version, "Car Explorer PRO�, version 1.03");
                MessageBox(0, "Car Explorer was successfully activated!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, LOWORD(wParam));
            }
            else
                MessageBox(0, "Wrong activation code :-((", szMBTitle, 
                                                    MB_OK | MB_ICONINFORMATION);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

VOID CALLBACK CloseButton_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, 
                                                                   DWORD dwTime)
{
    TCHAR szButtonText[50];

    g_nRBD_CloseButtonMeter--;
    wsprintf(szButtonText, "Close (%d)", g_nRBD_CloseButtonMeter);
    //Messa
    SendMessage(hwnd, WM_SETTEXT, 0 /* not used */, (LPARAM)szButtonText);
                                               
    if (g_nRBD_CloseButtonMeter <= 0)
    {
        EnableWindow(hwnd, TRUE);
        // MSDN: When the "window handle passed to SetTimer is valid, this 
        // [2nd parameter] parameter must be the same as the nIDEvent value 
        // passed to SetTimer". The MSDN a bit contradictory since in SetTimer
        // description it says when "the hWnd parameter is not NULL, then 
        // the return value is a nonzero integer. An application can pass the 
        // value of the nIDEvent parameter to the KillTimer function to destroy 
        // the timer." SetTimer description does not mention it and if return
        // value ius passed to KillTimer, the timer is not destroyed.
        KillTimer(hwnd, g_nCloseButton_TimerID); 
        SendMessage(hwnd, WM_SETTEXT, 0 /* not used */, (LPARAM)"Close");
    }
}

// Note: the code is based on MSDN's 179378 article.
void SelectFolder(HWND hDlg)
{
    BROWSEINFO bi;
    TCHAR szDir[MAX_PATH];
    LPITEMIDLIST pidl;
    LPMALLOC pMalloc;
    TCHAR szFFN[MAX_PATH];

    if (SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        ZeroMemory(&bi,sizeof(bi));
        bi.hwndOwner = hDlg; //NULL;
        bi.pszDisplayName = 0;
        bi.pidlRoot = 0;
        bi.ulFlags = BIF_RETURNONLYFSDIRS; // | BIF_STATUSTEXT;
        bi.lpfn = BrowseCallbackProc;

        pidl = SHBrowseForFolder(&bi); 
        if (pidl) 
        { 
            if (SHGetPathFromIDList(pidl, szFFN))
            {
                SendMessage(GetDlgItem(hDlg, IDC_SF_EDIT), WM_SETTEXT, 
                                               0 /* not used */, (LPARAM)szFFN);
            }
            pMalloc ->Free(pidl);
        }
        pMalloc ->Release();
   }
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
   TCHAR szDir[MAX_PATH];
   RECT rc;

   switch(uMsg) 
   {
   case BFFM_INITIALIZED: 
      // Center browse for folder dialog.
      GetWindowRect(hwnd, &rc);
      SetWindowPos(hwnd, NULL,
            ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
            ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

      if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir))
      {
         // WParam is TRUE since you are passing a path.
         // It would be FALSE if you were passing a pidl.
         //SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
          SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)g_szCarFilesFolder);
      }
      break;

   case BFFM_SELCHANGED: 
      // Set the status window to the currently selected path.
      if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
      {
         SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
      }
      break;
   }
   return 0;
}

void ConvertXMLData(short iSelectedUnits)
{
    float fOldUnits, fNewUnits;
    TCHAR szBuffer[20];
    string sBuffer;
    int iPos;

    if (iSelectedUnits == 1) // US to metric conversion 
    {
        // Convert length.
        ListView_GetItemText(hwndBottomLV, 9 /* length */, 1, szBuffer, 20);
        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits*25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 9 /* length */, 1, szBuffer);
        ListView_SetItemText(hwndBottomLV, 9 /* length */, 2, "mm");

        // Convert width.
        ListView_GetItemText(hwndBottomLV, 10 /* width */, 1, szBuffer, 20);
        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits*25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 10 /* width */, 1, szBuffer);
        ListView_SetItemText(hwndBottomLV, 10 /* width */, 2, "mm");
        
        // Convert height.
        ListView_GetItemText(hwndBottomLV, 11 /* height */, 1, szBuffer, 
                                                                            20);
        ListView_SetItemText(hwndBottomLV, 11 /* heigh */, 2, "mm");

        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits*25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 11 /* height */, 1, szBuffer);

        // Convert weight.
        ListView_GetItemText(hwndBottomLV, 12 /* weight */, 1, szBuffer, 
                                                                            20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            // Since weight uses thousand separator (","), which atof does not 
            // treat property (see it as decimal separator), I remove "," from 
            // original string before making atof conversion.
            sBuffer = szBuffer;
            iPos = sBuffer.find(",");
            if (iPos != string::npos /* not found */)
                sBuffer.replace(iPos, 
                                1 /* number of characters to be replaced*/, "");
            fOldUnits = atof(sBuffer.c_str());
            fNewUnits = fOldUnits*0.45359; // lbs->kg (1 lbs = 0.45359 kg)
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 12 /* weight */, 1, 
                                                                      szBuffer);
            ListView_SetItemText(hwndBottomLV, 12 /* weight */, 2, "kg");
        }

        // Convert clearance.
        ListView_GetItemText(hwndBottomLV, 13 /* clearance */, 1, szBuffer, 
                                                                            20);
        if (lstrcmp(szBuffer, "No data") !=0 && lstrcmp(szBuffer, "") !=0)
        {
            ListView_SetItemText(hwndBottomLV, 13 /* clearance */, 2, "mm");

            fOldUnits = atof(szBuffer);
            fNewUnits = fOldUnits*25.4;
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 13 /* clearance */, 1, 
                                                                      szBuffer);
        }

        // Convert tank volume.
        ListView_GetItemText(hwndBottomLV, 15 /* tank volume */, 1, 
                                                                  szBuffer, 20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            fNewUnits = fOldUnits*3.785412; // gallons -> liters
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 15 /* tank volume */, 1, 
                                                                      szBuffer);
            ListView_SetItemText(hwndBottomLV, 15 /* tank volume */, 2, 
                                                                      "liters");
        }
    }
    else  // metric to US conversion 
    {
        // Convert length.
        ListView_GetItemText(hwndBottomLV, 9 /* length */, 1, szBuffer, 20);
        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits/25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 9 /* length */, 1, szBuffer);
        ListView_SetItemText(hwndBottomLV, 9 /* length */, 2, "in");

        // Convert width.
        ListView_GetItemText(hwndBottomLV, 10 /* width */, 1, szBuffer, 20);
        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits/25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 10 /* width */, 1, szBuffer);
        ListView_SetItemText(hwndBottomLV, 10 /* width */, 2, "in");
        
        // Convert height.
        ListView_GetItemText(hwndBottomLV, 11 /* height */, 1, szBuffer, 
                                                                            20);
        fOldUnits = atof(szBuffer);
        fNewUnits = fOldUnits/25.4;
        sprintf(szBuffer, "%.2f", fNewUnits); 
        ListView_SetItemText(hwndBottomLV, 11 /* height */, 1, szBuffer);
        ListView_SetItemText(hwndBottomLV, 11 /* height */, 2, "in");

        // Convert weight.
        ListView_GetItemText(hwndBottomLV, 12 /* weight */, 1, szBuffer, 
                                                                            20);
        if (lstrcmp(szBuffer, "No data") !=0 && lstrcmp(szBuffer, "") !=0)
        {
            fOldUnits = atof(szBuffer);
            fNewUnits = fOldUnits/0.45359; // kg->lbs (1 lbs = 0.45359 kg) 
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 12 /* weight */, 1, 
                                                                      szBuffer);
            ListView_SetItemText(hwndBottomLV, 12 /* weight */, 2, "lb");
        }

        // Convert clearance.
        ListView_GetItemText(hwndBottomLV, 13 /* weight */, 1, szBuffer, 
                                                                            20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            fNewUnits = fOldUnits/25.4; 
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 13 /* weight */, 1, 
                                                                      szBuffer);
            ListView_SetItemText(hwndBottomLV, 13 /* weight */, 2, "lb");
        }

        // Convert tank volume.
        ListView_GetItemText(hwndBottomLV, 15 /* tank volume */, 1, 
                                                                  szBuffer, 20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            fNewUnits = fOldUnits/3.785412; // liters -> gallons
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndBottomLV, 15 /* tank volume */, 1, 
                                                                      szBuffer);
            ListView_SetItemText(hwndBottomLV, 15 /* tank volume */, 2, 
                                                                     "gallons");
        }
    }
}

void ConvertUnits(HWND hwndLV, short iSelectedUnits)
{
    int i;
    float fOldUnits, fNewUnits;
    TCHAR szBuffer[20];
    string sBuffer;
    int iPos;

    TCHAR szTest[100];
    wsprintf(szTest, "%d", iSelectedUnits); 
    //MessageBox(0, szTest, "Test", 0);

    for (i = 0; i < ListView_GetItemCount(hwndLV); i++)
    {
        // Convert length.
        ListView_GetItemText(hwndLV, i, 9 /* length */, szBuffer, 20);
        if (lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            if (iSelectedUnits == 0 /* metric to US */) 
                fNewUnits = fOldUnits/25.4; // mm -> inches 
            else if (iSelectedUnits == 1 /* US to metric*/) 
                fNewUnits = fOldUnits*25.4; // inches -> mm
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 9 /* length */, szBuffer);
        }

        // Convert width.
        ListView_GetItemText(hwndLV, i, 10 /* width */, szBuffer, 20);
        if (lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            if (iSelectedUnits == 0 /* metric to US */) 
                fNewUnits = fOldUnits/25.4; // mm -> inches 
            else if (iSelectedUnits == 1 /* US to metric*/) 
                fNewUnits = fOldUnits*25.4; // inches -> mm
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 10 /* width */, szBuffer);
        }

        // Convert height.
        ListView_GetItemText(hwndLV, i, 11 /* height */, szBuffer, 20);
        if (lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            if (iSelectedUnits == 0 /* metric to US */) 
                fNewUnits = fOldUnits/25.4; // mm -> inches 
            else if (iSelectedUnits == 1 /* US to metric*/) 
                fNewUnits = fOldUnits*25.4; // inches -> mm
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 11 /* height */, szBuffer);
        }

        // Convert clearance.
        ListView_GetItemText(hwndLV, i, 12 /* clearance */, szBuffer, 20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            if (iSelectedUnits == 0 /* metric to US */) 
                fNewUnits = fOldUnits/25.4; // mm -> inches 
            else if (iSelectedUnits == 1 /* US to metric*/) 
                fNewUnits = fOldUnits*25.4; // inches -> mm
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 12 /* clearance */, szBuffer);
        }

        // Convert weight.
        ListView_GetItemText(hwndLV, i, 13 /* weight */, szBuffer, 20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            if (iSelectedUnits == 0 /* metric to US */)
            {
                fOldUnits = atof(szBuffer);
                fNewUnits = fOldUnits/0.45359; // kg -> lbs (1 lbs = 0.45359 kg)
            }
            else if (iSelectedUnits == 1 /* US to metric*/) 
            {
                // Since weight uses thousand separator (","), which atof does 
                // not treat property (see it as decimal separator), I remove 
                // "," from original string before making atof conversion.
                sBuffer = szBuffer;
                iPos = sBuffer.find(",");
                if (iPos != string::npos /* not found */)
                    sBuffer.replace(iPos, 
                                1 /* number of characters to be replaced*/, "");
                fOldUnits = atof(sBuffer.c_str());
                fNewUnits = fOldUnits*0.45359; // lbs->kg 
            }
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 13 /* weight */, szBuffer);
        }

        // Convert tank volume.
        ListView_GetItemText(hwndLV, i, 14 /* tank volume */, szBuffer, 20);
        if (lstrcmp(szBuffer, "No data") != 0 && lstrcmp(szBuffer, "") != 0)
        {
            fOldUnits = atof(szBuffer);
            if (iSelectedUnits == 0 /* metric to US */) 
                fNewUnits = fOldUnits/3.785412; // liters -> gallons
            else if (iSelectedUnits == 1 /* US to metric*/) 
                fNewUnits = fOldUnits*3.785412; // gallons -> liters
            sprintf(szBuffer, "%.2f", fNewUnits); 
            ListView_SetItemText(hwndLV, i, 14 /* tank volume */, szBuffer);
        }
    }
}

// PURPOSE: Check if the file is MSS file. The week criteria is to check if 
//          string resource #10 exists (the number of images), which must always 
//          exist in MSS.    
// NOTES:   The IsMSSFile name is used instead of IsCARFile to reserve an option 
//          to view (and check) general (not file of car type) in the CE.
BOOL IsMSSFile(LPCTSTR pszFFN /* in */)
{
      HINSTANCE hResInst;
    TCHAR szBuffer[100];
    int nRes;

    hResInst = LoadLibrary(pszFFN);

    // The 1st check (weak): if it's not a loadable dynamic-link library, than 
    // this is not MSS file.
    if (hResInst == NULL)
        return FALSE;

    // The 2st check (stronger): check image count resource (#10) since this 
    // resource is always not empty for the valid MSS. 
    nRes = LoadString(hResInst, 10 /* IDS_IMAGECOUNT */, szBuffer, 
                                        sizeof(szBuffer)/sizeof(szBuffer[0]));
    if (nRes == 0 /* the string resource does not exist */)
        return FALSE;

    if (!hResInst) FreeLibrary(hResInst);

    return TRUE;
}

void InitImageLists(HWND hwndTV) 
{
    HIMAGELIST himlTV; 
    HBITMAP hBitmap;
    HICON hIcon;

    himlTV = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK,
                                           0 /* number of initial images */, 0);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPEN_FOLDER));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);
    
    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CLOSED_FOLDER));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DOCUMENT));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FAVORITES));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                           MAKEINTRESOURCE(IDI_FAVORITES_ITEM));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                         MAKEINTRESOURCE(IDI_FAVORITES_GREYED));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                       MAKEINTRESOURCE(IDI_FAVORITES_STANDARD));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                   MAKEINTRESOURCE(IDI_FAVORITES_OPENSTANDARD));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                           MAKEINTRESOURCE(IDI_FAVORITES_USER));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                       MAKEINTRESOURCE(IDI_FAVORITES_OPENUSER));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                          MAKEINTRESOURCE(IDI_FAVORITES_GREEN));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                      MAKEINTRESOURCE(IDI_FAVORITES_OPENGREEN));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                          MAKEINTRESOURCE(IDI_FAVORITES_PINK));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                      MAKEINTRESOURCE(IDI_FAVORITES_OPENPINK));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FAVORITES_RED)); 
                                            
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                        MAKEINTRESOURCE(IDI_FAVORITES_OPENRED));
    ImageList_AddIcon(himlTV, hIcon);
    DeleteObject(hIcon);
    
    // Associate.  
    TreeView_SetImageList(hwndTV, himlTV, 
                                    TVSIL_NORMAL /* or state - TVSIL_NORMAL */); 
} 

BOOL PopulateTreeView(HWND hwndTV, int nGroup) 
{
    TVITEM tvi; 
    TVINSERTSTRUCT tvis; 

    HTREEITEM hPrev;
    HTREEITEM hPrev1;
    HTREEITEM hPrev2;
    HTREEITEM hItem;
    TCHAR szText[100];
    HTREEITEM hInsertedItem;
    string sTitle;
    string sSuffix;
    string sMake;
    string sYear;
    HTREEITEM hTVItem;
    FavoritesFolders ff;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
    tvi.pszText = "All"; 
    tvi.iImage  = 0; 
    tvi.iSelectedImage = 0; // closed folder - 0, open folder - 1
    
    tvis.hParent = NULL;
    tvis.hInsertAfter = TVI_LAST; 
    tvis.item = tvi; 
    hPrev = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                              (LPARAM)(LPTVINSERTSTRUCT) &tvis);

    for (int i = 0; i < m_vecCarFilesMap.size(); i++)
    {
        if (nGroup == 0 /* all files, not groups */)
        {
            lstrcpy(szText, m_vecCarFilesMap.at(i).sTitle.c_str()); 
            
            tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
            tvi.pszText = szText; 
            tvi.iImage  = 2;         // file image
            tvi.iSelectedImage = 2;  // same file image
    
            tvis.hParent = hPrev;
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 
            
            hInsertedItem = TreeView_InsertItem(hwndTV, &tvis);
            m_vecCarFilesMap.at(i).hItem = hInsertedItem;
        }
        else
        {
            switch (nGroup)
            {
            case 1: 
                if (m_vecCarFilesMap.at(i).sYear.size() != 0) 
                {
                    lstrcpy(szText, m_vecCarFilesMap.at(i).sYear.c_str());
                }
                else // no internal XML data were found
                {
                    sTitle = m_vecCarFilesMap.at(i).sTitle;
                    sSuffix = sTitle.substr(sTitle.size() - 5, 5);

                    if (IsCarSuffix(sSuffix.c_str()))
                    {
                        sYear = sTitle.substr(sTitle.size() - 4, 4);
                        lstrcpy(szText, sYear.c_str());
                    }
                    else
                    {
                        // Write empty string  AS IS.
                        lstrcpy(szText, m_vecCarFilesMap.at(i).sYear.c_str());
                    }
                }
                break;
            case 2: 
                if (m_vecCarFilesMap.at(i).sMake.size() != 0) 
                {
                    lstrcpy(szText, m_vecCarFilesMap.at(i).sMake.c_str());
                }
                else // no internal XML data were found
                {
                    sTitle = m_vecCarFilesMap.at(i).sTitle;
                    sSuffix = sTitle.substr(sTitle.size() - 5, 5);

                    if (IsCarSuffix(sSuffix.c_str()))
                    {
                        sMake = sTitle.substr(0, sTitle.size() - 5);
                        lstrcpy(szText, sMake.c_str());
                    }
                    else
                    {
                        // Write empty string  AS IS.
                        lstrcpy(szText, m_vecCarFilesMap.at(i).sMake.c_str());
                    }
                }
                break;
            case 3:
                lstrcpy(szText, m_vecCarFilesMap.at(i).sCountry.c_str());
                break;
            }

            // Consider as special the situation when there is an empty string 
            // for the category that might occur for MSS prior to v1.1. In this 
            // case make special folder for uncategorized files (cannot be 
            // categorized) with arbitrary title like "Miscellaneous", and 
            // substitute for processing that empty string.
            if (lstrlen(szText) == 0) lstrcpy(szText, 
                                                     UNCATEGORIZED_FOLDER_NAME);
            
            //lstrcpy(tvi.pszText, m_vecCarFilesMap.at(i).sMake.c_str()); // does not work correctly
            tvi.pszText = szText; 
            tvi.iImage  = 0;        // folder
            tvi.iSelectedImage = 1; // open folder
            if (g_nGroup == 1 /* Year */) 
                tvi.lParam = atoi(szText); // ordering
            
            tvis.hParent = hPrev; 
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 
            
            hItem = IsInserted(hwndTV, szText);
            if (!hItem /* not yet inserted */)
            {
                hItem = TreeView_InsertItem(hwndTV, &tvis);

                lstrcpy(szText, m_vecCarFilesMap.at(i).sTitle.c_str()); 
                
                tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                     TVIF_PARAM; 
                tvi.pszText = szText; 
                tvi.iImage  = 2;        // file image
                tvi.iSelectedImage = 2; // same file image 
        
                tvis.hParent = hItem;
                tvis.hInsertAfter = TVI_LAST; 
                tvis.item = tvi; 
                
                hInsertedItem = TreeView_InsertItem(hwndTV, &tvis);
                m_vecCarFilesMap.at(i).hItem = hInsertedItem;
            }
            else
            {
                lstrcpy(szText, m_vecCarFilesMap.at(i).sTitle.c_str()); 
                
                tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                     TVIF_PARAM; 
                tvi.pszText = szText; 
                tvi.iImage  = 2;         // file image
                tvi.iSelectedImage = 2;  // same file image
        
                tvis.hParent = hItem;
                tvis.hInsertAfter = TVI_LAST; 
                tvis.item = tvi; 
                
                hInsertedItem = TreeView_InsertItem(hwndTV, &tvis);
                m_vecCarFilesMap.at(i).hItem = hInsertedItem;
            }
        }
    }

    TreeView_Expand(hwndTV, TreeView_GetRoot(hwndTV), TVE_EXPAND);

    // Add Favorites folder.
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
    tvi.pszText = "Favorites"; 
    tvi.iImage  = 3;        // same icon 
    tvi.iSelectedImage = 3; // same icon 
    tvi.lParam = 0; // Favorites folder
    
    tvis.hParent = NULL;
    tvis.hInsertAfter = TVI_LAST; 
    tvis.item = tvi; 
    g_htviFavorites = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    // Add default folders.

    // Shared tvi attributes.
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.iImage  = 6;        // standard folder
    tvi.iSelectedImage = 7; // open standard folder 
    tvi.lParam = 1; // 1st level subfolder 

    // Shared tvis attributes.
    tvis.hParent = g_htviFavorites;
    tvis.hInsertAfter = TVI_LAST;

    // Insert Compact folder.
    tvi.pszText = "Compact"; 
    tvis.item = tvi; 
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Sedan folder.
    tvi.pszText = "Sedan"; 
    tvis.item = tvi; 
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Coupe folder.
    tvi.pszText = "Coupe"; 
    tvis.item = tvi; 
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Hatchback folder.
    tvi.pszText = "Hatchback"; 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Wagon folder.
    tvi.pszText = "Wagon"; //"Estate"; 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Convertible folder.
    tvi.pszText = "Convertible"; 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert SUV folder.
    tvi.pszText = "SUV"; 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Luxury folder.
    tvi.pszText = "Luxury"; 
    tvi.iImage  = 12;        // standard pink folder
    tvi.iSelectedImage = 13; // open standard pink folder 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Green folder.
    tvi.pszText = "Green"; 
    tvi.iImage  = 10;        // standard green folder
    tvi.iSelectedImage = 11; // open standard green folder 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);

    // Insert Sport folder.
    tvi.pszText = "Sport"; 
    tvi.iImage  = 14;        // standard sport folder
    tvi.iSelectedImage = 15; // open standard sport folder 
    tvis.item = tvi;
    hTVItem = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
    ff.bStandard = 1;
    ff.hTVItem = hTVItem;
    ff.sFolder = tvi.pszText;
    vecFavoritesFolders.push_back(ff);
                                                   
    return TRUE;
}

HTREEITEM IsInserted(HWND hwndTV, LPCTSTR pszText)
{
    HTREEITEM hRootItem;
    HTREEITEM hFirstChildItem; 
    HTREEITEM hCurChildItem;
    HTREEITEM hNextChildItem; 
    TVITEM tvi;

    hRootItem = TreeView_GetRoot(hwndTV);
    hFirstChildItem	= TreeView_GetChild(hwndTV, hRootItem);

    // Explicit buffer setting is essential, just setting tvi.mask and tvi.hItem 
    // may leave tvi not populated. Although it might work on this step it 
    // crashes on the next. 
    char pszBuf[MAX_PATH+1] = "";
    ZeroMemory( &tvi, sizeof(tvi) );
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hFirstChildItem;
    tvi.pszText = pszBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hwndTV, &tvi);
    //if (lstrcmp(tvi.pszText, pszText) == 0 /* equal */) return TRUE;
    if (lstrcmp(tvi.pszText, pszText) == 0 /* equal */) return hFirstChildItem;
    //::MessageBox(0, tvi.pszText, "", 0);

    //TCHAR szText[100];
 //   sprintf(szText, "%d", TreeView_GetCount(hwndTV));
    //::MessageBox(0, szText, "", 0); // 59 (LL with root item)

    
    hCurChildItem = hFirstChildItem;
    for (int i = 0; i < TreeView_GetCount(hwndTV); i++)
    {
        //hNextChildItem = TreeView_GetNextItem(hwndTV, hFirstChildItem, TVGN_NEXTVISIBLE); //TVGN_NEXT); //hFirstChildItem
        hNextChildItem = TreeView_GetNextSibling(hwndTV, hCurChildItem); 
        if (hNextChildItem != NULL)
        {
            //App_Debug("not NULL");
            tvi.mask = TVIF_TEXT; 
            tvi.hItem = hNextChildItem; 
            tvi.pszText = pszBuf;
            tvi.cchTextMax = MAX_PATH+1;
            TreeView_GetItem(hwndTV, &tvi);
            //::MessageBox(0, tvi.pszText, "", 0);
            //if (lstrcmp(tvi.pszText, pszText) == 0 /* equal */) return TRUE;
            if (lstrcmp(tvi.pszText, pszText) == 0 /* equal */) return hNextChildItem;
            hCurChildItem = hNextChildItem;
        }
    }

    return FALSE;
}

// From SLite's RefreshComboBox.
// Note: return value is actually not used outside the function.
BOOL PopulateTreeView(HWND hwndTV) 
{
    TCHAR szEnvVariable[100];
    TCHAR szImagesLiteDir[MAX_PATH];
    TCHAR szSearchString[MAX_PATH];
    TCHAR szImagePath[MAX_PATH];
    TCHAR szFirstTitleName[MAX_PATH];
    //CTitleToFileName m_Names;
    TCHAR szTitle[100];
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    int iItem;
    HKEY hKey;
    DWORD dwResult;
    TCHAR szImageLibrary[100];

    //--
    TVITEM tvi; 
    TVINSERTSTRUCT tvis; 

    HTREEITEM hPrev;
    HTREEITEM hPrev1;
    HTREEITEM hPrev2;
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
    //--

    //ExpandEnvironmentStrings("%appdata%", szEnvVariable, 
    //	                                                 sizeof(szEnvVariable));

    //wsprintf(szImagesLiteDir, "%s%s", szEnvVariable, "\\MSB\\SLite_Images");
    //wsprintf(szSearchString, "%s%s%s", szEnvVariable, "\\MSB\\SLite_Images", 
    //	                                                         _T("\\*.mss"));
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.mss");

    tvi.pszText = "All"; 
    tvi.iImage  = 0; 
    tvi.iSelectedImage = 0; 
    
    tvis.hParent = NULL;
    tvis.hInsertAfter = TVI_LAST; 
    tvis.item = tvi; 

    hPrev = (HTREEITEM)::SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &tvis);

    // FindFirstFile, FindNextFile and, when found, obtain title string packed 
    // within MSS. 
    hFind = FindFirstFile(szSearchString , &fd);
    int i = 0;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintf(szImagePath, "%s%s%s", szImagesLiteDir, _T("\\"), fd.cFileName); 
                                                                  
        GetImageSetTitle(szImagePath, szTitle);
        lstrcpy(szFirstTitleName, szTitle); // error handling
        //MessageBox(0, szFirstTitleName, szTitle, 0);
        //--
        //SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), CB_ADDSTRING, 
     //                                     0 /* must be zero */, (LPARAM)szTitle);

        //
        
        tvi.pszText = szTitle; 
        tvi.iImage  = 0; 
        tvi.iSelectedImage = 0; 
     
        tvis.hParent = hPrev;
        tvis.hInsertAfter = TVI_LAST; 
        tvis.item = tvi; 

        ::SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &tvis);
        i++;
        //--

        //// Store found title/file name pair in the map.
        //_tcscpy(m_Names.szTitleName, szTitle); 
        //_tcscpy(m_Names.szFileName, szImagePath); 
        //m_vecNamesMap.push_back(m_Names);

        while (FindNextFile(hFind , &fd))
        {
              wsprintf(szImagePath, "%s%s%s", szImagesLiteDir, _T("\\"), 
                                                                  fd.cFileName);
            GetImageSetTitle(szImagePath, szTitle);
            //SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), CB_ADDSTRING, 
            //                              0 /* must be zero */, (LPARAM)szTitle);

            tvi.pszText = szTitle; 
            tvi.iImage  = 0; 
            tvi.iSelectedImage = 0; 
         
            tvis.hParent = hPrev; ;
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 

            ::SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &tvis);


            //// Store found title/file name pair in the map.
         //   _tcscpy(m_Names.szTitleName, szTitle); 
         //   _tcscpy(m_Names.szFileName, szImagePath); 
         //   m_vecNamesMap.push_back(m_Names);
        }
        FindClose(hFind);
    }

    // The same for CAR files.
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.car");
    hFind = FindFirstFile(szSearchString , &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        //wsprintf(szImagePath, "%s%s%s", szImagesLiteDir, _T("\\"), fd.cFileName); 
        wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, "\\", fd.cFileName);
                                                                  
        GetImageSetTitle(szImagePath, szTitle);
        lstrcpy(szFirstTitleName, szTitle); // error handling
        
        tvi.pszText = szTitle; 
        tvi.iImage  = 0; 
        tvi.iSelectedImage = 0; 
     
        tvis.hParent = hPrev;
        tvis.hInsertAfter = TVI_LAST; 
        tvis.item = tvi; 

        ::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
        i++;

        while (FindNextFile(hFind , &fd))
        {
              //wsprintf(szImagePath, "%s%s%s", szImagesLiteDir, _T("\\"), 
                    //											  fd.cFileName);
            wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, "\\", 
                                                                  fd.cFileName);
            GetImageSetTitle(szImagePath, szTitle);

            tvi.pszText = szTitle; 
            tvi.iImage  = 0; 
            tvi.iSelectedImage = 0; 
         
            tvis.hParent = hPrev; ;
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 

            ::SendMessage(hwndTV, TVM_INSERTITEM, 0, 
                                               (LPARAM)(LPTVINSERTSTRUCT)&tvis);
        }
        FindClose(hFind);
    }

    //iItem = SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), 
    //	CB_FINDSTRINGEXACT, -1 /* search from the beginng */, 
    //	                               (LPARAM)(LPCSTR)g_sImageLibrary.c_str());

    //// Set combobox selection according to the title persisted in registry, and 
    //// in case of error, e.g. relative MSS was removed incidentally from Images 
    //// folder, to the title of the first MSS found in the Images folder.
    //if (iItem != CB_ERR) 
    //{	
    //	//MessageBox(0, g_sImageLibrary.c_str(), "no CB_ERR", 0);
    //	SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), 
    //					            CB_SETCURSEL, iItem, 0 /* must be zero */ );
    //    return TRUE;  
 //   }
    //else
    //{
    //    //MessageBox(0, szFirstTitleName, "CB_ERR", 0);
    //	iItem = SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), 
    //	    CB_FINDSTRINGEXACT, -1 /* search from the beginng */, 
    //	                                      (LPARAM)(LPCSTR)szFirstTitleName);
    //	if (iItem != CB_ERR) // should be always OK since since MSS was found
    //	{
    //		SendMessage(GetDlgItem(hDlg, IDC_PP1_COMBO), 
    //					            CB_SETCURSEL, iItem, 0 /* must be zero */ );

    //		// Update ImageLibrary value instead of erroneous one.
    //		RegOpenKey(COMPONENT_BASE_HIVE, REGKEY_MSLITE, &hKey);

    //        _tcscpy(szImageLibrary, szFirstTitleName);
    //		dwResult = RegSetValueEx(hKey, "ImageLibrary", 0, REG_SZ, 
    //				       (LPBYTE)szImageLibrary, _tcslen(szImageLibrary) + 1);

    //		if (dwResult != ERROR_SUCCESS) 
    //		{
    //			::MessageBox(0, "RegSetValueEx error: ImageLibrary", 
    //			                               "SLite.scr", MB_ICONEXCLAMATION);
    //			 return FALSE; 
    //		}
    //		else
    //		{
 //               g_sImageLibrary = szImageLibrary;
    //			return TRUE; 
    //		}
    //	}
    //}
    
    return FALSE; // we are here when SLite_Images directory is empty
}

BOOL GetImageSetTitle(TCHAR* pszLocation /* in */, TCHAR* pszTitle /* out */)
{
      HINSTANCE hResInst;
    TCHAR szTitle[100];

    hResInst = LoadLibrary(pszLocation);

    if(hResInst == NULL)
    {
        return FALSE;
    }

    LoadString(hResInst, 1 /* always Name ID */ , szTitle, 
                                            sizeof(szTitle)/sizeof(szTitle[0]));
    FreeLibrary(hResInst);

    _tcscpy(pszTitle, szTitle);
    
    return TRUE;
}

void App_SetFont(HWND hwnd, BOOL bBold)
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
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE;
    _tcsncpy(lf.lfFaceName, _T("MS Shell Dlg"), 32); //_T("Arial") 

    hfnt = CreateFontIndirect(&lf);
    //hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT);  
    SendMessage(hwnd,  WM_SETFONT, (WPARAM)hfnt, TRUE);
}

LRESULT APIENTRY DialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                     WPARAM wParam, LONG lParam)
{
    int iCurSel;
    TCHAR szText[100];
    HTREEITEM hRootItem;
    TVITEM tvi;
    TVSORTCB tvsort;
    
    switch (Message)
    {
    case WM_COMMAND:
        // By now the dialog host only one control so all command messages are 
        // from combobox.
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            g_nGroup = SendMessage(hwndComboBox, CB_GETCURSEL, 
                                    0 /* must be zero */, 0 /* must be zero */);
            TreeView_DeleteAllItems(hwndBrowser);
        
            // Save current changes to favorites to registry if there were 
            // updates before reloading from registry. 
            if (g_bUpdateRegistry) App_OnSaveSettings();

            // Reload and repopulate favorites completely from registry as 
            // easiest way way to keep favorites with group change.
            m_vecCarFavoritesMap.clear();
            vecFavoritesFolders.clear(); 
            PopulateTreeView(hwndBrowser, g_nGroup); 
            App_LoadRegistrySettings();

            // Sort items under "All" root item.
            hRootItem = TreeView_GetRoot(hwndBrowser);
            if (g_nGroup != 1 /* group is not year*/)
                TreeView_SortChildren(hwndBrowser, hRootItem, FALSE);
            else if (g_nGroup == 1 /* sort by year */)
            {
                tvsort.hParent = hRootItem;
                tvsort.lpfnCompare = TV_CompareProc;			
                TreeView_SortChildrenCB(hwndBrowser, &tvsort, 
                                                          0 /* must be zero */);
            }

            // When no favorites, set grey Favorites icon for TV.
            if (!TreeView_GetChild(hwndBrowser, 
                                        g_htviFavorites /* Favorites folder */))
            {
                tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
                tvi.hItem = g_htviFavorites;
                tvi.iImage = 5; // grey 
                tvi.iSelectedImage = 5; // grey
                TreeView_SetItem(hwndBrowser, &tvi);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    
    return CallWindowProc((WNDPROC)g_wpOldDialogProc, hWnd, Message, 
                                                                wParam, lParam);
}

// Note: neither WM_CREATE nor WM_INITDIALOG are not fired for sublassed function.
LRESULT APIENTRY  RightHostDialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                     WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_SETCURSOR: //not fired when SetCapture is used with DAD
        SetCursor(hCursorNS);
        // Make arrow cursor inside list view.
        if (wParam == (WPARAM)g_hwndLV) SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        if (wParam == (WPARAM)hwndRightBottomHost) SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
        if (wParam == (WPARAM)hwndPictureView) SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        if (wParam == (WPARAM)hwndBottomLV) SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        
        return 1; // both 0 and 1 OK (break DNW)
    case WM_LBUTTONDOWN:
        Splitter2_OnLButtonDown(hWnd, Message, wParam, lParam);
        break;
    case WM_LBUTTONUP:
        Splitter2_OnLButtonUp(hWnd, Message, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        Splitter2_OnMouseMove(hWnd, Message, wParam, lParam);
        break;
    case WM_CONTEXTMENU:
        if (wParam == (WPARAM)g_hwndLV) LV_OnContextMenu(g_hwndLV);
        break;
    case WM_SIZE:
        SizeWindowContents2(LOWORD(lParam), HIWORD(lParam));
        break;	
    case WM_COMMAND: // <-- not fired on click
        // if (LOWORD(wParam) == IDC_LISTVIEW) MessageBox(0, "A", "", 0); 
        //MessageBox(0, "WM_COMMAND - 1", "", 0); 
        // By now the dialog host only one control so all command messages are 
        // from combobox.
        switch (HIWORD(wParam))
        {
        //case CBN_SELCHANGE:
        //	g_nGroup = SendMessage(hwndComboBox, CB_GETCURSEL, 
        //		                    0 /* must be zero */, 0 /* must be zero */);
        //	TreeView_DeleteAllItems(hwndBrowser);
        //	PopulateTreeView(hwndBrowser, g_nGroup); 
        //	break;
        default:
            break;
        }
        break;
    case WM_NOTIFY: 
        if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW /* single CAR file LV */)
        {
            // Notice that the handler is used as shared for amin LV and single 
            // CAR LV.
            return ListView_NotifyHandler(((LPNMHDR)lParam)->hwndFrom, Message, 
                                                                wParam, lParam);
        }

        break;
    default:
        break;
    }
    
    return CallWindowProc((WNDPROC)g_wpOldRightHost_DialogProc, hWnd, Message, 
                                                                wParam, lParam);
}

// Note: WM_CREATE is not fired for sublassed function.
LRESULT APIENTRY RBHDialogProc_SubclassedFunc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_SETCURSOR: 
        SetCursor(hCursorNS);
        if (wParam == (WPARAM)hwndRightBottomHost) SetCursor(LoadCursor(NULL, 
                                                  MAKEINTRESOURCE(IDC_SIZEWE)));
        if (wParam == (WPARAM)hwndPictureView) SetCursor(LoadCursor(NULL, 
                                                   MAKEINTRESOURCE(IDC_ARROW)));
        if (wParam == (WPARAM)hwndBottomLV) SetCursor(LoadCursor(NULL, 
                                                   MAKEINTRESOURCE(IDC_ARROW)));
        return 1; // both 0 and 1 OK (break DNW)
    case WM_LBUTTONDOWN:
        //MessageBox(0, "WM_LBUTTONDOWN - 3", "", 0);
        Splitter3_OnLButtonDown(hWnd, Message, wParam, lParam);
        break;
    case WM_LBUTTONUP: // not fired!!!
        //MessageBox(0, "WM_LBUTTONUP - 3", "", 0);
        Splitter3_OnLButtonUp(hWnd, Message, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        Splitter3_OnMouseMove(hWnd, Message, wParam, lParam);
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->idFrom == IDC_BOTTOM_LISTVIEW) 
            return BottomLV_NotifyHandler(((LPNMHDR)lParam)->hwndFrom, Message, 
                                                                wParam, lParam);
        break;
    case WM_SIZE:
        SizeWindowContents3(LOWORD(lParam), HIWORD(lParam));
        break;
    }

    return CallWindowProc((WNDPROC)g_wpOldRBH_DialogProc, hWnd, Message, 
                                                                wParam, lParam);
}

BOOL EnumerateCarFiles() 
{
    TCHAR szSearchString[MAX_PATH];
    int nCount = 0;

    g_nCollectionsCount = 0;
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.mss");
    nCount = 0;
    EnumerateCarFiles(szSearchString, nCount /* in, out */); 
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.car");
    EnumerateCarFiles(szSearchString, nCount /* in, out */); 
    
    return FALSE; // we are here when SLite_Images directory is empty
}

void EnumerateCarFiles(LPCTSTR pszSearchString, int& nCount /* in, out */) 
{
    TCHAR szImagesLiteDir[MAX_PATH];
    //TCHAR szSearchString[MAX_PATH];
    TCHAR szImagePath[MAX_PATH];
    TCHAR szFirstTitleName[MAX_PATH];
    CCarList m_CarFile;
    TCHAR szTitle[100];
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szCountry[100];
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    int iItem;
    TCHAR szImageLibrary[100];

    // FindFirstFile, FindNextFile and, when found, obtain title string packed 
    // within MSS. 
    hFind = FindFirstFile(pszSearchString , &fd);
    int i = nCount;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        g_nCollectionsCount++;
        wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, "\\", fd.cFileName); 
                                                                  
        GetImageSetTitle(szImagePath, szTitle);
        //::MessageBox(0, szImagePath, szTitle, 0);
        GetCarXMLData(szImagePath, 1000 /* year */, szYear);
        GetCarXMLData(szImagePath, 1100 /* make */, szMake);
        GetCarXMLData(szImagePath, 2300 /* country */, szCountry);
        //::MessageBox(0, szMake, szCountry, 0);
            
        //lstrcpy(szFirstTitleName, szTitle); // error handling

        // Store found title/file name pair in the map.
        m_CarFile.sTitle = szTitle; 
        m_CarFile.sFFN = szImagePath; 
        m_CarFile.sYear = szYear; 
        m_CarFile.sMake = szMake; 
        m_CarFile.sCountry = szCountry; 
        m_vecCarFilesMap.push_back(m_CarFile);

        while (FindNextFile(hFind , &fd))
        {
            g_nCollectionsCount++;
            if (g_bUseKagiPaymentProcessing)
            {
                // Don't populate in full m_vecCarFilesMap if CE is not 
                // registered.
                if (!g_bRegisteredCopy && 
                                    g_nCollectionsCount > COLLECTIONS_USE_LIMIT) 
                    break;
            }
            wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, "\\", 
                                                                  fd.cFileName);
            GetImageSetTitle(szImagePath, szTitle);
            GetCarXMLData(szImagePath, 1000 /* year */, szYear);
            GetCarXMLData(szImagePath, 1100 /* make */, szMake);
            GetCarXMLData(szImagePath, 2300 /* country */, szCountry);

            // Store found title/file name pair in the map.
            m_CarFile.sTitle = szTitle; 
               m_CarFile.sFFN = szImagePath; 
            m_CarFile.sYear = szYear; 
            m_CarFile.sMake = szMake; 
            m_CarFile.sCountry = szCountry; 
            m_vecCarFilesMap.push_back(m_CarFile);
        }
        FindClose(hFind);
    }

    nCount = i;
}

BOOL GetCarXMLData(LPCTSTR pszFFN, UINT nID, LPTSTR lszBuffer)
{
    HINSTANCE hResInst;
    int nLength;

    hResInst = LoadLibrary(pszFFN);

    if (hResInst == NULL)
    {
        return FALSE;
    }

    // Load string. TODO: elaborate the use of the last parameter (100).
    nLength = LoadString(hResInst, nID, lszBuffer, 100);
    if (nLength == 0) *lszBuffer = '\0';

    FreeLibrary(hResInst);

    return TRUE;
}

// Load all car data from .CAR and .MSS files located in a single folder except 
// pictures (saves RAM).
BOOL LoadAllCarData(LPCTSTR pszCarDirectory) // TODO: LL pszCarDirectory is redundant.
{
    TCHAR szSearchString[MAX_PATH];
    int nGlobalCount = 0;

    g_nCollectionsCount = 0;
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.mss");
    LoadAllCarData(szSearchString, nGlobalCount /* in, out */);
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, "\\*.car");
    LoadAllCarData(szSearchString, nGlobalCount /* in, out */);
    
    return FALSE; // we are here when the directory is empty
}

void LoadAllCarData(LPCTSTR pszSearchString, int& nGlobalCount /* in, out */)
{
    //TCHAR szSearchString[MAX_PATH];
    TCHAR szFFN[MAX_PATH];
    CCarData m_CarData;
    TCHAR szTitle[100];
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szCountry[100];
    TCHAR szPictureTitle[100]; 
 //   // Main.
 //   string sYear;
 //   string sMake;
    TCHAR szModel[100];
    TCHAR szSubModel[100];
    TCHAR szType[100];
    TCHAR szPrice[100];
    //// Engine.
    TCHAR szPower[100];
    TCHAR szFuel[100];
    //// Sizes and weight.
    TCHAR szLength[100];
    TCHAR szWidth[100];
    TCHAR szHeight[100];
    TCHAR szClearance[100];
    TCHAR szWeight[100];
    //// Misc.
    TCHAR szTankVolume[100];
    //string sCountry;
    HANDLE hFind;
    WIN32_FIND_DATA fd;

    TCHAR szImagesCount[10];
    int nImagesCount;
    //int nIndex = 0; // unique index used for ordering

    // FindFirstFile, FindNextFile and, when found, obtain title string packed 
    // within MSS. 
    hFind = FindFirstFile(pszSearchString , &fd);
    int nIndex = nGlobalCount;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        g_nCollectionsCount++;
        wsprintf(szFFN, "%s%s%s", g_szCarFilesFolder, "\\", fd.cFileName); 
                                                                  
        GetImageSetTitle(szFFN, szTitle);
        GetCarXMLData(szFFN, 1000 /* year */, szYear);
        GetCarXMLData(szFFN, 1100 /* make */, szMake);
        GetCarXMLData(szFFN, 2300 /* country */, szCountry);

        GetCarXMLData(szFFN, 10 /* images count */, szImagesCount);
        nImagesCount = atoi(szImagesCount);
        for (int i = 0; i < nImagesCount; i++)
        {
            GetCarXMLData(szFFN, 201 + i /* picture title */, szPictureTitle);
            GetCarXMLData(szFFN, 1201 + i /* model */, szModel);
            GetCarXMLData(szFFN, 1301 + i /* submodel */, szSubModel);
            GetCarXMLData(szFFN, 1401 + i /* type */, szType);
            GetCarXMLData(szFFN, 1501 + i /* price */, szPrice);
            GetCarXMLData(szFFN, 1601 + i /* power */, szPower);
            GetCarXMLData(szFFN, 1701 + i /* fuel */, szFuel);
            GetCarXMLData(szFFN, 1801 + i /* length */, szLength);
            GetCarXMLData(szFFN, 1901 + i /* width */, szWidth);
            GetCarXMLData(szFFN, 2001 + i /* height */, szHeight);
            GetCarXMLData(szFFN, 2201 + i /* clearance */, szClearance);
            GetCarXMLData(szFFN, 2101 + i /* weight */, szWeight);
            GetCarXMLData(szFFN, 2401 + i /* tank volume */, szTankVolume);
        
            // Add car details to the vector.
            m_CarData.sTitle = szTitle; 
            m_CarData.sFileName = fd.cFileName; 
            //m_CarData.sFFN = ...;
            m_CarData.sYear = szYear; 
            m_CarData.sMake = szMake; 
            m_CarData.sModel = szModel; 
            m_CarData.sSubModel = szSubModel; 
            m_CarData.sType = szType; 
            m_CarData.sPrice = szPrice; 
            m_CarData.sPower = szPower; 
            m_CarData.sFuel = szFuel;
            m_CarData.sLength = szLength;
            m_CarData.sWidth = szWidth;
            m_CarData.sHeight = szHeight;
            m_CarData.sClearance = szClearance;
            m_CarData.sWeight = szWeight;
            m_CarData.sTankVolume = szTankVolume;
            m_CarData.sCountry = szCountry;
            m_CarData.nImageID = i; // relative order in collection
            m_CarData.nIndex = nIndex; // global index among all files

            m_vecCarData.push_back(m_CarData);
            nIndex++;
        }

        while (FindNextFile(hFind , &fd))
        {
            g_nCollectionsCount++;
            if (g_bUseKagiPaymentProcessing)
            {
                // Don't populate in full m_vecCarData if CE is not registered.
                if (!g_bRegisteredCopy && 
                                    g_nCollectionsCount > COLLECTIONS_USE_LIMIT) 
                    break;
            }

            wsprintf(szFFN, "%s%s%s", g_szCarFilesFolder, "\\", fd.cFileName); 

            GetImageSetTitle(szFFN, szTitle);
            GetCarXMLData(szFFN, 1000 /* year */, szYear);
            GetCarXMLData(szFFN, 1100 /* make */, szMake);
            GetCarXMLData(szFFN, 2300 /* country */, szCountry);

            GetCarXMLData(szFFN, 10 /* images count */, szImagesCount);
            nImagesCount = atoi(szImagesCount);
            for (int i = 0; i < nImagesCount; i++)
            {
                GetCarXMLData(szFFN, 201 + i /* picture title */, szPictureTitle);
                GetCarXMLData(szFFN, 1201 + i /* model */, szModel);
                GetCarXMLData(szFFN, 1301 + i /* submodel */, szSubModel);
                GetCarXMLData(szFFN, 1401 + i /* type */, szType);
                GetCarXMLData(szFFN, 1501 + i /* price */, szPrice);
                GetCarXMLData(szFFN, 1601 + i /* power */, szPower);
                GetCarXMLData(szFFN, 1701 + i /* fuel */, szFuel);
                GetCarXMLData(szFFN, 1801 + i /* length */, szLength);
                GetCarXMLData(szFFN, 1901 + i /* width */, szWidth);
                GetCarXMLData(szFFN, 2001 + i /* height */, szHeight);
                GetCarXMLData(szFFN, 2201 + i /* clearance */, szClearance);
                GetCarXMLData(szFFN, 2101 + i /* weight */, szWeight);
                GetCarXMLData(szFFN, 2401 + i /* tank volume */, szTankVolume);
            
                // Add car details to the vector.
                m_CarData.sTitle = szTitle; 
                m_CarData.sFileName = fd.cFileName; 
                //m_CarData.sFFN = ...;
                m_CarData.sYear = szYear; 
                m_CarData.sMake = szMake; 
                m_CarData.sModel = szModel; 
                m_CarData.sSubModel = szSubModel; 
                m_CarData.sType = szType; 
                m_CarData.sPrice = szPrice; 
                m_CarData.sPower = szPower; 
                m_CarData.sFuel = szFuel;
                m_CarData.sLength = szLength;
                m_CarData.sWidth = szWidth;
                m_CarData.sHeight = szHeight;
                m_CarData.sClearance = szClearance;
                m_CarData.sWeight = szWeight;
                m_CarData.sTankVolume = szTankVolume;
                m_CarData.sCountry = szCountry;
                m_CarData.nImageID = i; // relative order in collection
                m_CarData.nIndex = nIndex; // global index among all files

                m_vecCarData.push_back(m_CarData);
                nIndex++;
            }
        }
        FindClose(hFind);
    }

    nGlobalCount = nIndex;
}

void InitListViewColumns(HWND hwndLV)
{
    HICON hIconUp, hIconDown;
    HIMAGELIST himlHeader;
    LVCOLUMN lvc;

    // Setup image list.
    himlHeader = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 
                                           0 /* number of initial images */, 0);

    hIconUp = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ARROW_UP));
    ImageList_AddIcon(himlHeader, hIconUp);

    hIconDown = LoadIcon(GetModuleHandle(NULL), 
                                               MAKEINTRESOURCE(IDI_ARROW_DOWN));
    ImageList_AddIcon(himlHeader, hIconDown);

    DeleteObject(hIconUp);
    DeleteObject(hIconDown);

    // Associate image list with List View Control.
    Header_SetImageList(ListView_GetHeader(hwndLV), himlHeader);

    // Setup list view colums.
    //lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_IMAGE | LVCF_FMT;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.iSubItem = 0;
    lvc.pszText = "Title";
    lvc.cx = 100; 
    lvc.fmt  = LVCFMT_CENTER |  LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
    lvc.iImage = 0; 
    ListView_InsertColumn(hwndLV, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.pszText = "Year";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.pszText = "Make";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 2, &lvc);

    lvc.iSubItem = 3;
    lvc.pszText = "Model";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 3, &lvc);

    lvc.iSubItem = 4;
    lvc.pszText = "SubModel";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 4, &lvc);

    lvc.iSubItem = 5;
    lvc.pszText = "Type";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 5, &lvc);

    //lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_IMAGE | LVCF_FMT;
    lvc.iSubItem = 6;
    lvc.pszText = "Price";
    lvc.fmt = LVCFMT_CENTER;
    //lvc.fmt = LVCFMT_CENTER |  LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
    lvc.cx = 70;
    ListView_InsertColumn(hwndLV, 6, &lvc);

    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.iSubItem = 7;
    lvc.pszText = "Power";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 7, &lvc);

    lvc.iSubItem = 8;
    lvc.pszText = "Fuel";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 8, &lvc);

    lvc.iSubItem = 9;
    lvc.pszText = "Length";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 9, &lvc);

    lvc.iSubItem = 10;
    lvc.pszText = "Width";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 10, &lvc);

    lvc.iSubItem = 11;
    lvc.pszText = "Height";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 11, &lvc);

    lvc.iSubItem = 12;
    lvc.pszText = "Clearance";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 12, &lvc);

    lvc.iSubItem = 13;
    lvc.pszText = "Weight";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 13, &lvc);

    lvc.iSubItem = 14;
    lvc.pszText = "Tank Volume";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 14, &lvc);

    lvc.iSubItem = 15;
    lvc.pszText = "Country";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndLV, 15, &lvc);
}

// PURPOSE: Browses items in All and its subfolders. 
// NOTES:   See similar BrowseFavorites that browses items in Favorites and its 
//          subfolders. 
void InitListViewItems(HWND hwndLV, 
    LPCTSTR pszFilter /* default value is NULL */, 
                                 BOOL bCarFileClicked /* car file is clicked */)
{
    TCHAR szBuffer[100];
    LVITEM lvi;

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
    lvi.state = 0; 
    lvi.stateMask = 0;

    //Populate list view with details from m_vecCarData vector
    for (int i = 0; i < m_vecCarData.size(); i++)
    {
        lvi.iItem = i;
        lvi.iSubItem = 0;
        //lvi.iSubItem = 0;
        lvi.lParam = m_vecCarData.at(i).nIndex; // used for ordering
        lstrcpy(szBuffer, m_vecCarData.at(i).sTitle.c_str());
        lvi.pszText = szBuffer;

        if (pszFilter == NULL /* no filter */)
        {
            ListView_InsertCarItem(hwndLV, &lvi, i);
        }
        else
        {
            if (!bCarFileClicked)
            {
                switch (g_nGroup)
                {
                case 0: // All (no groups)
                    // TODO: elaborate
                    break;
                case 1: // group by year
                    if (lstrcmp(pszFilter, 
                                        m_vecCarData.at(i).sYear.c_str()) == 0)
                        ListView_InsertCarItem(hwndLV, &lvi, i);
                    break;
                case 2: // group by make
                    // TODO: or general MSS, check type of MSS
                    if (lstrcmp(pszFilter, 
                                        m_vecCarData.at(i).sMake.c_str()) == 0) 
                        ListView_InsertCarItem(hwndLV, &lvi, i);
                    break;
                case 3: // group by country
                    if (lstrcmp(pszFilter, 
                        m_vecCarData.at(i).sCountry.c_str()) == 0 || 
                            (m_vecCarData.at(i).sCountry.size() 
                            == 0 /* no data */ && lstrcmp(pszFilter, 
                                            UNCATEGORIZED_FOLDER_NAME) == 0))
                        ListView_InsertCarItem(hwndLV, &lvi, i);
                    break;
                default:
                    // TODO: elaborate
                    break;
                }
            }
            else
            {
                //App_Debug(m_vecCarData.at(i).sTitle.c_str());
                if (lstrcmp(pszFilter, 
                    m_vecCarData.at(i).sTitle.c_str()) == 0)
                                    ListView_InsertCarItem(hwndLV, &lvi, i);
            }
        } // end of else for "filter"
    } // end of for

    //if (ListView_GetItemCount(hwndLV) > g_nEvaluationLimit)
    //	        DialogBox(hInst, (LPCTSTR)IDD_REGISTER_PRODUCT, NULL, 
    //		                                  (DLGPROC)RegisterProduct_WndProc);
}

// PURPOSE: Browses items in in Favorites and its subfolders. 
// NOTES:   See similar (and older) InitListViewItems that browses items in All 
//          and its subfolders. 
void BrowseFavorites(HWND hwndLV, int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                          LPCTSTR pszFolderName)
{
    TCHAR szBuffer[100];
    LVITEM lvi;

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
    lvi.state = 0; 
    lvi.stateMask = 0;

    //  Populate list view with details from m_vecCarFavoritesMap vector.
    //if (lstrcmp(pszFilter, "Favorites") == 0 /* Favorites folder */)
    switch (nFolderLevel)
    {
    case 0: // Favorites folder
        for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        {
            lvi.iItem = i;
            lvi.iSubItem = 0;
            lvi.lParam = m_vecCarFavoritesMap.at(i).nGlobalImageID_PK; 
            lstrcpy(szBuffer, m_vecCarFavoritesMap.at(i).sTitle.c_str());
            lvi.pszText = szBuffer;

            ListView_InsertFavoritesItem(hwndLV, &lvi, i);
        }
        break;
    case 1: // first level subfolder
        for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        {
            if (m_vecCarFavoritesMap.at(i).nItemLevel == 1 && 
                lstrcmp(m_vecCarFavoritesMap.at(i).sParentFolder.c_str(), 
                                                pszFolderName) == 0 /* equal */)
            {
                lvi.iItem = i;
                lvi.iSubItem = 0;
                lvi.lParam = m_vecCarFavoritesMap.at(i).nGlobalImageID_PK; 
                lstrcpy(szBuffer, m_vecCarFavoritesMap.at(i).sTitle.c_str());
                lvi.pszText = szBuffer;

                ListView_InsertFavoritesItem(hwndLV, &lvi, i);
            }
        }
        break;
    case 2: // second level subfolder
        for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        {
            if (m_vecCarFavoritesMap.at(i).nItemLevel == 2 /* 2nd level */ 
                    && lstrcmp(
                    m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(), 
                                        pszParentFolderName) == 0 /* equal */ &&
                    lstrcmp(m_vecCarFavoritesMap.at(i).sParentFolder.c_str(), 
                                                pszFolderName) == 0 /* equal */)
            {
                lvi.iItem = i;
                lvi.iSubItem = 0;
                lvi.lParam = m_vecCarFavoritesMap.at(i).nGlobalImageID_PK; 
                lstrcpy(szBuffer, m_vecCarFavoritesMap.at(i).sTitle.c_str());
                lvi.pszText = szBuffer;

                ListView_InsertFavoritesItem(hwndLV, &lvi, i);
            }
        }
        break;
    default:
        // Must never be here since depth of Favorites' subfolders is 2.
        break;
    }
}

void InitListViewItems(HWND hwndLV, int nYearSelection, int nCarType, 
                                    int nMinPrice, int nMaxPrice, int nFuelType)
{
    TCHAR szBuffer[100];
    LVITEM lvi;
    int nPrice;
    BOOL bYearMatched;
    BOOL bCarTypeMatched;
    BOOL bPriceRangeMatched;
    BOOL bFuelTypeMatched;
    string sYear;
    char szYear[6];
    SYSTEMTIME st;
    string sCarType;
    string sFuelType;

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
    lvi.state = 0; 
    lvi.stateMask = 0;

    // Populate list view with details from m_vecCarData vector using filters.
    for (int i = 0; i < m_vecCarData.size(); i++)
    {
        bYearMatched = FALSE;
        bCarTypeMatched = FALSE;
        bPriceRangeMatched = FALSE;
        bFuelTypeMatched = FALSE;
        
        lvi.iItem = i;
        lvi.iSubItem = 0;
        //lvi.iSubItem = 0;
        lvi.lParam = m_vecCarData.at(i).nIndex; // used for ordering
        lstrcpy(szBuffer, m_vecCarData.at(i).sTitle.c_str());
        lvi.pszText = szBuffer;

        nPrice = Utility_ConvertPriceToNumber(
            m_vecCarData.at(i).sPrice.c_str(), 
                                             NULL /* any, not relevent here */);

        // Filter by year.
        sYear = m_vecCarData.at(i).sYear;

        switch (nYearSelection)
        {
        case 0: // All
            bYearMatched = TRUE;
            break;
        case 1: // current year
            GetLocalTime(&st); 
            sprintf(szYear, "%d", st.wYear);
            if (lstrcmp(sYear.c_str(), szYear) == 0 /* equal */)
                bYearMatched = TRUE;
            break;
        default:
            bYearMatched = FALSE;
        }
        
        // Filter by car type.
        sCarType = m_vecCarData.at(i).sType;

        // The flag (reserved) g_bExactCarTypeMatch is set to FALSE, which mean
        // we search loosely e.g. "Wagon" can also mean "Estate", if set to TRUE
        // the search for "Wagon" string can only mean "Wagon".
        switch (nCarType)
        {
        case 0: // All
            bCarTypeMatched = TRUE;
            break;
        case 1: // Compact
            if (sCarType == "Compact")
                bCarTypeMatched = TRUE;
            break;
        case 2: // Sedan
            if (sCarType == "Sedan")
                bCarTypeMatched = TRUE;
            break;
        case 3: // Coupe
            if (sCarType == "Coupe")
                bCarTypeMatched = TRUE;
            break;
        case 4: // Hatchback
            if (sCarType == "Hatchback")
                bCarTypeMatched = TRUE;
            break;
        case 5: // Wagon
            if (g_bExactCarTypeMatch)
            {
                if (sCarType == "Wagon") 
                    bCarTypeMatched = TRUE;
            }
            else 
            {
                if (sCarType == "Wagon" || sCarType == "Station Wagon" || 
                                                        sCarType == "Estate")
                    bCarTypeMatched = TRUE;
            }
            break;
        case 6: // Convertible
            if (g_bExactCarTypeMatch)
            {
                if (sCarType == "Convertible") 
                    bCarTypeMatched = TRUE;
            }
            else
            {
                if (sCarType == "Convertible" || sCarType == "Cabriolet" || 
                                                        sCarType == "Roadster")
                    bCarTypeMatched = TRUE;
            }
            break;
        case 7: // SUV
            if (g_bExactCarTypeMatch)
            {
                if (sCarType == "SUV")
                    bCarTypeMatched = TRUE;
            }
            else
            {
                if (sCarType == "SUV" || sCarType == "Crossover")
                    bCarTypeMatched = TRUE;
            }
            break;
        default:
            bCarTypeMatched = FALSE;
        }

        // Filter by price range.
        if (nPrice >= nMinPrice && nPrice <= nMaxPrice)
            bPriceRangeMatched = TRUE;

        // Filter by fuel type.
        sFuelType = m_vecCarData.at(i).sFuel;

        switch (nFuelType)
        {
        case 0: // All
            bFuelTypeMatched = TRUE;
            break;
        case 1: // Gasoline
            if (g_bExactFuelTypeMatch)
            {
                if (sFuelType == "Gasoline")
                    bFuelTypeMatched = TRUE;
            }
            else 
            {
                if (sFuelType == "Gasoline" || sFuelType == "Petrol" || 
                    sFuelType == "Gas/SEFI" || sFuelType == "Gas/MPFI" || 
                    sFuelType == "Gas/EFI" || sFuelType == "Gas/NL" ||
                    sFuelType == "Gasoline/EFI" || sFuelType == "Gasoline/MPFI" 
                    || sFuelType == "Gasoline/FI" || 
                            sFuelType == "Gasoline/SEFI" || sFuelType == "MPFI")
                    bFuelTypeMatched = TRUE;
            }
            break;
        case 2: // Diesel
            if (g_bExactFuelTypeMatch)
            {
                if (sFuelType == "Diesel")
                    bFuelTypeMatched = TRUE;
            }
            else 
            {
                if (sFuelType == "Diesel" || sFuelType == "Dsl/EFI")
                    bFuelTypeMatched = TRUE;
            }
            break;
        case 3: // Hybrid
            if (g_bExactFuelTypeMatch)
            {
                if (sFuelType == "Hybrid")
                    bFuelTypeMatched = TRUE;
            }
            else 
            {
                if (sFuelType == "Hybrid" || sFuelType == "Hyb/EFI" ||
                    sFuelType == "Hyb/MPFI" || sFuelType == "Hyb/SEFI" ||
                          sFuelType == "Hyb/MPFI" || sFuelType == "Hybrid/SEFI")
                    bFuelTypeMatched = TRUE;
            }
            break;
        case 4: // Electric
            if (sFuelType == "Electric")
                bFuelTypeMatched = TRUE;
            break;
        default:
            bFuelTypeMatched = FALSE;
        }

        // Insert the item after filtering if matches the filters.
        if (bYearMatched && bCarTypeMatched && bPriceRangeMatched && 
                                                               bFuelTypeMatched) 
            ListView_InsertCarItem(hwndLV, &lvi, i);
    } 
}

void ListView_InsertFavoritesItem(HWND hwndLV, LVITEM* pLVI, 
                                                int nFavorite /* favorite ID */)
{
    int i;
    int iInsertedItem;
    
    i = m_vecCarFavoritesMap.at(nFavorite).nGlobalImageID_PK;
    iInsertedItem = ListView_InsertItem(hwndLV, pLVI);

    ListView_SetItemText(hwndLV, iInsertedItem, 1, 
                                       (LPSTR)m_vecCarData.at(i).sYear.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 2, 
                                       (LPSTR)m_vecCarData.at(i).sMake.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 3, 
                                      (LPSTR)m_vecCarData.at(i).sModel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 4, 
                                   (LPSTR)m_vecCarData.at(i).sSubModel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 5, 
                                       (LPSTR)m_vecCarData.at(i).sType.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 6, 
                                      (LPSTR)m_vecCarData.at(i).sPrice.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 7, 
                                      (LPSTR)m_vecCarData.at(i).sPower.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 8, 
                                       (LPSTR)m_vecCarData.at(i).sFuel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 9, 
                                     (LPSTR)m_vecCarData.at(i).sLength.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 10, 
                                      (LPSTR)m_vecCarData.at(i).sWidth.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 11, 
                                     (LPSTR)m_vecCarData.at(i).sHeight.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 12, 
                                  (LPSTR)m_vecCarData.at(i).sClearance.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 13, 
                                     (LPSTR)m_vecCarData.at(i).sWeight.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 14, 
                                 (LPSTR)m_vecCarData.at(i).sTankVolume.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 15, 
                                    (LPSTR)m_vecCarData.at(i).sCountry.c_str());
}

void ListView_InsertCarItem(HWND hwndLV, LVITEM* pLVI, int i /* car ID */)
{
    int iInsertedItem;
    
    iInsertedItem = ListView_InsertItem(hwndLV, pLVI);

    ListView_SetItemText(hwndLV, iInsertedItem, 1, 
                                       (LPSTR)m_vecCarData.at(i).sYear.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 2, 
                                       (LPSTR)m_vecCarData.at(i).sMake.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 3, 
                                      (LPSTR)m_vecCarData.at(i).sModel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 4, 
                                   (LPSTR)m_vecCarData.at(i).sSubModel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 5, 
                                       (LPSTR)m_vecCarData.at(i).sType.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 6, 
                                      (LPSTR)m_vecCarData.at(i).sPrice.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 7, 
                                      (LPSTR)m_vecCarData.at(i).sPower.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 8, 
                                       (LPSTR)m_vecCarData.at(i).sFuel.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 9, 
                                     (LPSTR)m_vecCarData.at(i).sLength.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 10, 
                                      (LPSTR)m_vecCarData.at(i).sWidth.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 11, 
                                     (LPSTR)m_vecCarData.at(i).sHeight.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 12, 
                                  (LPSTR)m_vecCarData.at(i).sClearance.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 13, 
                                     (LPSTR)m_vecCarData.at(i).sWeight.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 14, 
                                 (LPSTR)m_vecCarData.at(i).sTankVolume.c_str());
    ListView_SetItemText(hwndLV, iInsertedItem, 15, 
                                    (LPSTR)m_vecCarData.at(i).sCountry.c_str());
}

// Note: shared handler used for main LV and single CAR LV, which are 
//       essentically the same (main difference is height).
LRESULT ListView_NotifyHandler(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                 LPARAM lParam)
{
    LV_DISPINFO *pLV_DISPINFO = (LV_DISPINFO*)lParam;
    NMLISTVIEW* pnmlv = (NMLISTVIEW*)lParam;
    NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

    switch (pLV_DISPINFO->hdr.code)
    {
        case NM_CUSTOMDRAW:
            switch (plvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                    if (!(plvcd->nmcd.dwItemSpec % 2))
                        ; // odd row color (none).
                    else
                        plvcd->clrTextBk = g_clrLV_InterlacedColor; // even row color 
                break;
            }
            break;	    
        case NM_CLICK:
            if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW /* single CAR list view */)
            {
                int nGlobalCarIndex;
                int nCollectionCarIndex;
                LVITEM lvi;
                RECT rc;
                RECT rcMain;
                
                // When image clicked it is selected, therefore this returns clicked 
                // item (zero-based).
                short iImage = ListView_GetNextItem(g_hwndLV, -1, LVNI_SELECTED);
                //ShowSelectedPicture(iImage);

                TCHAR szTest[100];
                wsprintf(szTest, "%d", iImage);
                //MessageBox(0, szTest, "Test", 0);

                // Get global car index for the clicked item. 
                lvi.iItem = iImage;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem(g_hwndLV, &lvi);
                nGlobalCarIndex = lvi.lParam;

                // Get car index within collection. 
                nCollectionCarIndex = GetCollectionImageIDFromGlobalImageID(
                                                               nGlobalCarIndex);
                if (iImage != -1 /* click out of image */)
                                     LoadPictureZoomWindow(nCollectionCarIndex);

                GetWindowRect(hwndPictureView, &rc);
                InvalidateRect(hwndPictureView, NULL, TRUE);
                UpdateWindow(hwndPictureView);

                // TODO: with image view simply LoadXMLData(iImage) can be used 
                // (no need to call GetCollectionImageIDFromGlobalImageID since 
                // the order is fixed).
                if (iImage != -1 /* click out of image */) 
                                               LoadXMLData(nCollectionCarIndex);
            }

            break;
        case NM_DBLCLK:
            // TODO: the filter is used since it called also for the Main LV, merge 
            // somehow probably.
            if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW /* single CAR list view */)
            {
                LV_Notify_OnDoubleClick(((LPNMHDR)lParam)->hwndFrom); 
                //LV_Notify_OnDoubleClick(g_hwndLV);
            }
            break;
        case  LVN_BEGINDRAG:
            // Get row where drag originates.  
            g_iLVDroppedItem = pnmlv->iItem;
            g_bDADActive = TRUE;
            SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR_DAD_NODROP))); // LL DNM
            SetCapture(g_hWnd);

            break;
        case LVN_COLUMNCLICK:
            #define pnm ((LPNMLISTVIEW)lParam)

            // Store sort column in global variable.
            g_nSortColumn = (int)(pnm->iSubItem);

            //--
            TCHAR szTest[100];
            wsprintf(szTest, "%d %d", g_iSortColumn_Prev, g_nSortColumn); 
            //MessageBox(0, szTest, "Test", 0);

            // Make sort parameter g_iSortParam.
            if (g_iSortColumn_Prev == -1 /* none, startup */ || 
                                            g_nSortColumn != g_iSortColumn_Prev)
            {
                g_bSortAscending = TRUE;
            }
            else
            {
                if (g_bSortAscending) 
                    g_bSortAscending = FALSE;
                else 
                    g_bSortAscending = TRUE;
            }
            g_iSortParam = MAKELPARAM((WORD)g_nSortColumn, 
                                                        (WORD)g_bSortAscending);

            // Sort column and set appropriate icon.
            LV_SortItems(hwnd, g_nSortColumn);
            g_iSortColumn_Prev = g_nSortColumn;

            #undef pnm
            break;
    }

    return 0;
}

LRESULT BottomLV_NotifyHandler(HWND hwnd, UINT iMsg, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    LV_DISPINFO *pLV_DISPINFO = (LV_DISPINFO*)lParam;
    NMLISTVIEW* pnmlv = (NMLISTVIEW*)lParam;
    NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

    switch (pLV_DISPINFO->hdr.code)
    {
        case NM_CUSTOMDRAW:
            switch (plvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                    if (!(plvcd->nmcd.dwItemSpec % 2))
                        ; // odd row color (none).
                    else
                        plvcd->clrTextBk = g_clrLV_InterlacedColor; // even row color 
                break;
            }
            break;	    
    }

    return 0;
}

// Note: shared between main LV and sigle CAR LV.
int CALLBACK ListView_CompareProc(LPARAM lParam1, LPARAM lParam2, 
                          LPARAM lSortColumn /* in, application-defined value*/)
{
    HWND hwndLV;
    LV_FINDINFO lvf;
    int nItem1, nItem2;
    TCHAR szBuffer1[30], szBuffer2[30];
    TCHAR szBufLowerCase1[100], szBufLowerCase2[100];
    float fValue1, fValue2;
    int iValue1, iValue2;
    LPTSTR pszBegin1 = NULL;
    LPTSTR pszBegin2 = NULL;
    int ch = '$';
    int nSortOrder;

    // Notice that ListView_CompareProc (fixed parameters) is the shared compare 
    // function for handling main LV and single CAR LV so that proper handle is 
    // obtained dynamically.
    if (IsWindowVisible(hwndMainLV)) 
        hwndLV = hwndMainLV;
    else if (IsWindowVisible(g_hwndLV)) 
        hwndLV = g_hwndLV;
    // Case for initial sort since IsWindowVisible(g_hwndLV) returns FALSE.
    else if (g_nRightPaneViewType == 3 /* DC on CAR file (out) */)
        hwndLV = g_hwndLV; 

    // Find item based on its lParam value and obtain item's column text
    // based on supplied lSortColumn value. Notice that we divide 
    // sizeof(...) by 2 (Unicode's character length in bytes) because sizeof 
    // returns number of bytes, but we need number of chars. 
    lvf.flags = LVFI_PARAM;
    lvf.lParam = lParam1;
    nItem1 = ListView_FindItem(hwndLV, -1, &lvf);

    lvf.lParam = lParam2;
    nItem2 = ListView_FindItem(hwndLV, -1, &lvf);

    ListView_GetItemText(hwndLV, nItem1, (int)lSortColumn,
                                                szBuffer1, sizeof(szBuffer1));
    ListView_GetItemText(hwndLV, nItem2, (int)lSortColumn, 
                                                szBuffer2, sizeof(szBuffer2));
    // Get values for the utility variables. 
    nSortOrder = g_bSortAscending; // TODO: remove redundancy

    //lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
    //lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

    switch( (int)lSortColumn)
    {
        case 0:  // title
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            // Up (A..Z): put "No data" and empty records at the end. Notice that
            // 201 is some manually set number to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = 201;
            else fValue1 = (float)char(szBufLowerCase1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = 201;
            else fValue2 = (float)char(szBufLowerCase2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);			
        case 1:  // year
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (!isdigit(szBuffer1[0])) fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);
            
            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (!isdigit(szBuffer2[0])) fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);    
        case 2:  // make
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            // Up (A..Z): put "No data" and empty records at the end. Notice that
            // 201 is some manually set number to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else fValue1 = (float)char(szBufLowerCase1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else fValue2 = (float)char(szBufLowerCase2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);	
        case 3:  // model
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            // Up (A..Z): put "No data" and empty records at the end. Notice that
            // 201 is some manually set number to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 201 : -1;
            else fValue1 = (float)char(szBufLowerCase1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 201 : -1;
            else fValue2 = (float)char(szBufLowerCase2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 4:  // submodel
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            // Up (A..Z): put "No data" and empty records at the end. Notice that
            // 201 is some manually set number to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 201 : -1;
            else fValue1 = (float)char(szBufLowerCase1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 201 : -1;
            else fValue2 = (float)char(szBufLowerCase2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 5:  // type
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            // Up (A..Z): put "No data" and empty records at the end. Notice 
            // that 2000000001 is some manually set number to sort fairly.
            if (lstrlen(szBuffer1) == 0) 
                fValue1 = nSortOrder ? 2000000001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = 2000000000;
            else if (lstrlen(szBuffer1) > 0 && lstrlen(szBuffer1) < 3) 
                // Supposedly never here if no errors with data.
                fValue1 = (float)char(szBufLowerCase1[0]);
            else 
                // Make capare value using first 3 chars.
                fValue1 = (float)MakeCompareValue(char(szBufLowerCase1[0]), 
                            char(szBufLowerCase1[1]), char(szBufLowerCase1[2]));

            if (lstrlen(szBuffer2) == 0) 
                fValue2 = nSortOrder ? 2000000001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = 2000000000;
            else if (lstrlen(szBuffer2) > 0 && lstrlen(szBuffer2) < 3) 
                // Supposedly never here if no errors with data.
                fValue2 = (float)char(szBufLowerCase2[0]);
            else 
                // Make capare value using first 3 chars.
                fValue2 = (float)MakeCompareValue(char(szBufLowerCase2[0]), 
                            char(szBufLowerCase2[1]), char(szBufLowerCase2[2]));

            return 
                  ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 6:  // price
            //nSortOrder = g_aSortOrder[lSortColumn];

            // Notice that nSortOrder is used to keep "No data" and empty cells
            // always (up/down) at the bottom of the LV.
            fValue1 = (float)Utility_ConvertPriceToNumber(szBuffer1, nSortOrder);
            fValue2 = (float)Utility_ConvertPriceToNumber(szBuffer2, nSortOrder);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 7:  // power
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 2001 : -1;
            else if (!isdigit(szBuffer1[0])) fValue1 = nSortOrder ? 2000 : 0;
            else fValue1 = _tstof(szBuffer1);
            
            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 2001 : -1;
            else if (!isdigit(szBuffer2[0])) fValue2 = nSortOrder ? 2000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 8:  // fuel
            // Up (A..Z): put "No data" and empty records at the end. Notice 
            // that 200, 201 are some manually set numbers to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 201 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) fValue1 =
                                                           nSortOrder ? 200 : 0;
            else fValue1 = (float)char(szBuffer1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 201 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) fValue2 = 
                                                           nSortOrder ? 200 : 0;
            else fValue2 = (float)char(szBuffer2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 9:  // length 
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            //fValue1 = _tstof(szBuffer1);
            //fValue2 = _tstof(szBuffer2);
            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 10:  // width
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 11:  // height
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 12:  // clearance
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 13:  // weight
            //nSortOrder = g_aSortOrder[lSortColumn];

            // Notice that nSortOrder is used to keep "No data" and empty cells
            // always (up/down) at the bottom of the LV.
            fValue1 = (float)Utility_ConvertWeightToNumber(szBuffer1, 
                                                                    nSortOrder);
            fValue2 = (float)Utility_ConvertWeightToNumber(szBuffer2, 
                                                                    nSortOrder);

            return ListView_GetCompareReturnValue(fValue1, fValue2, 
                                                                   lSortColumn);
        case 14:  // tank volume
            //nSortOrder = g_aSortOrder[lSortColumn];

            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer1, "No data") == 0 /* equal */) 
                fValue1 = nSortOrder ? 10000 : 0;
            else fValue1 = _tstof(szBuffer1);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 10001 : -1;
            else if (lstrcmp(szBuffer2, "No data") == 0 /* equal */) 
                fValue2 = nSortOrder ? 10000 : 0;
            else fValue2 = _tstof(szBuffer2);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        case 15:  // country
            lstrcpy(szBufLowerCase1, CharLower(szBuffer1));
            lstrcpy(szBufLowerCase2, CharLower(szBuffer2));

            //// Up (A..Z): put "No data" and empty records at the end. Notice that
            //// 201 is some manually set number to sort faily.
            //if (lstrlen(szBuffer1) == 0) fValue1 = 201;
            //else fValue1 = (float)char(szBuffer1[0]);

            //if (lstrlen(szBuffer2) == 0) fValue2 = 201;
            //else fValue2 = (float)char(szBuffer2[0]);

            // Up (A..Z): put "No data" and empty records at the end. Notice that
            // 201 is some manually set number to sort faily.
            if (lstrlen(szBuffer1) == 0) fValue1 = nSortOrder ? 201 : -1;
            else fValue1 = (float)char(szBufLowerCase1[0]);

            if (lstrlen(szBuffer2) == 0) fValue2 = nSortOrder ? 201 : -1;
            else fValue2 = (float)char(szBufLowerCase2[0]);

            return ListView_GetCompareReturnValue(fValue1, fValue2, lSortColumn);
        default:
            break;
    }

    return 0;
}

// Note: shared between main LV and sigle CAR LV.
int ListView_GetCompareReturnValue(float fValue1, float fValue2, int iSortColumn)
{
    // Sort next time in the opposite direction.

    // MSDN: "The comparison function must return a negative value if the first 
    // item should precede the second, a positive value if the first item should 
    // follow the second, or zero if the two items are equivalent."
    if (g_bSortAscending)
    {
        if (fValue1 > fValue2)
        {
            return 1;
        }

        if (fValue1 < fValue2)
        {
            return -1;
        }

        if (fValue1 == fValue2)
        {
            return 0;
        }
    }
    else // descending
    {
        if (fValue1 > fValue2)
        {
            return -1;
        }

        if (fValue1 < fValue2)
        {
            return 1;
        }

        if (fValue1 == fValue2)
        {
            return 0;
        }
    }
    return 0; // never here
}

// TODO: probably remove LL ListView_GetCompareReturnValue(float fValue1, float 
// fValue2, int iSortColumn) is enough.
int ListView_GetCompareReturnValue(int iValue1, int iValue2, int iSortColumn)
{
    // Sort next time in the opposite direction (initial/previous sort direction
    // is stored in g_aSortOrder array).
    if (g_bSortAscending)
    {
        if (iValue1 > iValue2)
        {
            return 1;
        }

        if (iValue1 < iValue2)
        {
            return -1;
        }

        if (iValue1 == iValue2)
        {
            return 0;
        }
    }
    else // down
    {
        if (iValue1 > iValue2)
        {
            return -1;
        }

        if (iValue1 < iValue2)
        {
            return 1;
        }

        if (iValue1 == iValue2)
        {
            return 0;
        }
    }
    return 0; // never here
}

// From SLite.scr.
void GetImageSetFFN(LPCTSTR pszTitle /* in */, LPTSTR pszFFN /* out */)
{
    string sTitle = pszTitle;
    //MessageBox(NULL, pszTitle, "", MB_OK);

    for (UINT i = 0; i < m_vecCarFilesMap.size(); i++)
    {
        //MessageBox(NULL, pszTitle, m_vecCarFilesMap.at(i).sTitle.c_str(), MB_OK);
        //if (!lstrcmp(m_vecNamesMap.at(i).sTitleName.c_str(), pszTitle))
        if (m_vecCarFilesMap.at(i).sTitle == sTitle)
        {
            wsprintf(pszFFN, "%s", m_vecCarFilesMap.at(i).sFFN.c_str());
            //MessageBox(NULL, pszTitle, pszFileName, MB_OK);
            break;
        }
    }
}

// Used to get Collection's ImageID and FFN for clicked item in Favorites. 
// Notice that "Car Title" is like "Aston Martin 2008 (DB9)" and different from 
// "Title" used to identify CAR files.
int GetCollectionImageIDFromCarTitle(LPCTSTR pszCarTitle /* in */, 
                                                        LPTSTR pszFFN /* out */)
{
    int i;
    TCHAR szTitle[100];
    TCHAR szFFN[MAX_PATH];
    TCHAR szCarTitle[100];
    int nCollectionImageID;

    for (i = 0; i < m_vecCarFavoritesMap.size(); i++)
    {
        if (lstrcmp(pszCarTitle, 
                 m_vecCarFavoritesMap.at(i).sCarTitle.c_str()) == 0 /* equal */)
        {
            GetImageSetFFN(m_vecCarFavoritesMap.at(i).sTitle.c_str(), pszFFN);
            nCollectionImageID = m_vecCarFavoritesMap.at(i).nCollectionImageID;
            return nCollectionImageID;
        }
    }

    return 0; // error, not found
}

int GetCollectionImageIDFromGlobalImageID(int nGlobalImageID /* in */)
{
    int nCollectionImageID;

    for (UINT i = 0; i < m_vecCarData.size(); i++)
    {
        if (m_vecCarData.at(i).nIndex == nGlobalImageID)
        {
            nCollectionImageID = m_vecCarData.at(i).nImageID;

            return nCollectionImageID;
        }
    }
}

int GetGlobalCarID(LPCTSTR pszTitle /* in */, int nCollectionImageID /* in */)
{
    int nGlobalImageID;

    for (UINT i = 0; i < m_vecCarData.size(); i++)
    {
        if (lstrcmp(m_vecCarData.at(i).sTitle.c_str(), pszTitle) == 0 && 
                              m_vecCarData.at(i).nImageID == nCollectionImageID)
        {
            nGlobalImageID = m_vecCarData.at(i).nIndex;

            return nGlobalImageID;
        }
    }
    
    return -1; // not found
}

// TODO: LL out parameters return wrong values due to C++ issues (maybe better use references).
void GetImageSetFFNFromGlobalImageID(int nGlobalImageID /* in */, 
                                LPTSTR pszFFN /* out */, int& nImageID /* out */)
{
    //string sTitle = pszTitle;
    //MessageBox(NULL, pszTitle, "", MB_OK);

    for (UINT i = 0; i < m_vecCarData.size(); i++)
    {
        //TCHAR szBuf[10];
        //wsprintf(szBuf, "%d %d %d", m_vecCarData.at(i).nImageID, m_vecCarData.at(i).nIndex, nGlobalImageID);
        //MessageBox(0, szBuf, "", 0);

        //MessageBox(NULL, pszTitle, m_vecCarFilesMap.at(i).sTitle.c_str(), MB_OK);
        if (m_vecCarData.at(i).nIndex == nGlobalImageID)
        {
            wsprintf(pszFFN, "%s%s%s", g_szCarFilesFolder, "\\", 
                                          m_vecCarData.at(i).sFileName.c_str());
            nImageID = m_vecCarData.at(i).nImageID;

            //wsprintf(szBuf, "%d %d", nImageID, nGlobalImageID);
            //MessageBox(0, szBuf, pszFFN, 0);
            break;
        }
    }
}

// Copied from RT v1.3.
void App_CreateToolBar()
{
    INITCOMMONCONTROLSEX icex;
    TBBUTTON tbb[TB_BUTTONS_COUNT];
    TBADDBITMAP tb;
    HIMAGELIST himlTB; 
    HICON hIcon;
    REBARINFO rbi;
    REBARBANDINFO rbbi;
    DWORD dwBtnSize;
    TCHAR szBuffer[MAX_LOADSTRING];
    int aButtonString[TB_BUTTONS_COUNT];
    HMENU hSubMenu;
   
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    // Create rebar control to host toolbar. Note that at the present moment the 
    // toolbar can be created without rebar, but rebar gives proper bottom 
    // border, and makes convinient to ass other bands.
    g_hwndReBar= CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL, 
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
        CCS_NODIVIDER | CCS_TOP | RBS_BANDBORDERS | RBS_VARHEIGHT | 
               RBS_AUTOSIZE, 0, 0, 0, 0, g_hWnd, (HMENU)IDC_REBAR, hInst, NULL);

    // Create toolbar window with tooltips style enabled (tooltips handled in 
    // WM_NOTIFY). Note that CreateWindowEx is used instead of CreateToolbarEx 
    // since the latter lacks flexibility with button images (allows to use one
    // bitmap with multiple images), but the former makes possible to attach
    // an image list ot use multiple image lists.
    g_hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPTSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
        TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS, 0, 0, 0, 0, g_hwndReBar,
                                               (HMENU)IDC_TOOLBAR, hInst, NULL);
    // Create image list (LoadBitmap, LoadImage). Notice that if ILC_MASK is 
    // not used the background is black. TODO: verify if 32bit images (and 24bit
    // images are not reduced to 256 colors).
    himlTB = ImageList_Create(32, 32,  ILC_COLORDDB | ILC_MASK, // ILC_COLOR24
                                           0 /* number of initial images */, 0);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_FOLDER));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon);
    
    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_DOWNLOAD));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_SEARCH));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_FAVORITES));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_VIEW_GREY));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_SETTINGS));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_HELP));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    // Insert also alternative (button state) icons.
    hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TB_VIEW_ICON));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon);

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                           MAKEINTRESOURCE(IDI_TB_VIEW_REPORT));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                        MAKEINTRESOURCE(IDI_TB_FOLDER_GREYED));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon);
    
    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                    MAKEINTRESOURCE(IDI_TB_DOWNLOAD_GREYED));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                        MAKEINTRESOURCE(IDI_TB_SEARCH_GREYED));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon); 

    hIcon = LoadIcon(GetModuleHandle(NULL), 
                                    MAKEINTRESOURCE(IDI_TB_FAVORITES_GREYED));
    ImageList_AddIcon(himlTB, hIcon);
    DeleteObject(hIcon);

    // Send the TB_BUTTONSTRUCTSIZE message, which is required for backward 
    // compatibility before associating image list with toolbar. 
    SendMessage(g_hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
    SendMessage(g_hwndTB, TB_SETIMAGELIST, 0 /* only one image list */, 
                                                                (LPARAM)himlTB);

    // Add text descriptins for TB buttons. Notice that las string is terminated 
    // with two null characters (MSDN).
    LoadString(hInst, IDS_FOLDER, szBuffer, MAX_LOADSTRING);
    aButtonString[0] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_DOWNLOAD, szBuffer, MAX_LOADSTRING);
    aButtonString[1] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_SEARCH, szBuffer, MAX_LOADSTRING);
    aButtonString[2] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_FAVORITES, szBuffer, MAX_LOADSTRING);
    aButtonString[3] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_VIEW, szBuffer, MAX_LOADSTRING);
    aButtonString[4] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_SETTINGS, szBuffer, MAX_LOADSTRING);
    aButtonString[5] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);

    LoadString(hInst, IDS_HELP, szBuffer, MAX_LOADSTRING); // terminated with 2 NUL
    aButtonString[6] = SendMessage(g_hwndTB, TB_ADDSTRING, NULL, (LPARAM)szBuffer);
    
    // MSDN'03: If an application specifies labels in the iString member of the 
    // associated TBBUTTON structure, the toolbar control automatically uses 
    // that string as the ToolTip.
    tbb[0].iBitmap = 0;  
    tbb[0].idCommand = IDM_FILE_SETFOLDER; 
    tbb[0].fsState = TBSTATE_ENABLED; 
    tbb[0].fsStyle = BTNS_BUTTON; 
    tbb[0].dwData = 0; 
    tbb[0].iString = aButtonString[0]; 

    tbb[1].iBitmap = 1; 
    tbb[1].idCommand = IDM_TOOLS_DOWNLOAD; 
    tbb[1].fsState = TBSTATE_ENABLED; 
    tbb[1].fsStyle = BTNS_BUTTON; 
    tbb[1].dwData = 0; 
    tbb[1].iString = aButtonString[1]; 

    tbb[2].iBitmap = 2; 
    tbb[2].idCommand = ID_TOOLBAR_SEARCH; 
    tbb[2].fsState = TBSTATE_ENABLED; 
    tbb[2].fsStyle = BTNS_BUTTON; 
    tbb[2].dwData = 0; 
    tbb[2].iString = aButtonString[2]; 

    tbb[3].iBitmap = 3; 
    tbb[3].idCommand = ID_TOOLBAR_FAVORITES; 
    tbb[3].fsState = TBSTATE_ENABLED; 
    tbb[3].fsStyle = BTNS_BUTTON; 
    tbb[3].dwData = 0; 
    tbb[3].iString = aButtonString[3]; 

    tbb[4].iBitmap = 4; 
    tbb[4].idCommand = ID_TOOLBAR_VIEW; 
    tbb[4].fsState = TBSTATE_ENABLED; 
    tbb[4].fsStyle = BTNS_BUTTON; 
    tbb[4].dwData = 0; 
    tbb[4].iString = aButtonString[4];  

    tbb[5].iBitmap = 5; 
    tbb[5].idCommand = IDM_TOOLS_SETTINGS;
    tbb[5].fsState = TBSTATE_ENABLED; 
    tbb[5].fsStyle = BTNS_BUTTON; 
    tbb[5].dwData = 0; 
    tbb[5].iString = aButtonString[5];

    tbb[6].iBitmap = 6; 
    tbb[6].idCommand = ID_TOOLBAR_HELP; 
    tbb[6].fsState = TBSTATE_ENABLED; 
    tbb[6].fsStyle = BTNS_BUTTON; 
    tbb[6].dwData = 0; 
    tbb[6].iString = aButtonString[6]; 

    // Add buttons to toolbar.
    SendMessage(g_hwndTB, TB_SETMAXTEXTROWS, 
        0 /* use tooltips as strings from TBBUTTON structure */, 
                                                          0 /* must be zero */);
    SendMessage(g_hwndTB, TB_ADDBUTTONS, (WPARAM) 7 /* buttons number */, 
                                                      (LPARAM)(LPTBBUTTON)&tbb);
    SendMessage(g_hwndTB, TB_AUTOSIZE, 0, 0); 
    ShowWindow(g_hwndTB, SW_SHOW); // when missed, toolbar not shown

    // Initialize REBARINFO.
    rbi.cbSize = sizeof(REBARINFO);
    rbi.fMask = 0;
    rbi.himl = (HIMAGELIST)NULL;
    SendMessage(g_hwndReBar, RB_SETBARINFO, 0, (LPARAM)&rbi);

    // Initialize REBARBANDINFO.
    rbbi.cbSize = sizeof(REBARBANDINFO);  
    rbbi.fMask  = RBBIM_COLORS | RBBIM_TEXT | RBBIM_STYLE | RBBIM_CHILD  | 
                                                   RBBIM_CHILDSIZE | RBBIM_SIZE;

    dwBtnSize = SendMessage(g_hwndTB, TB_GETBUTTONSIZE, 0, 0);

    // Set values unique to the band with the toolbar.
    rbbi.fStyle = RBBS_CHILDEDGE;
    rbbi.lpText     = "Tool Bar";
    rbbi.hwndChild  = g_hwndTB;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = HIWORD(dwBtnSize);
    rbbi.cx         = 250;
    rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
    rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
    SendMessage(g_hwndReBar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

    hSubMenu = GetSubMenu(GetMenu(g_hWnd), 1);
    if (g_bShowToolBar) // meaning ReBar that hosts TB
    {
        g_nTBHeightUI = g_nTBHeight; 
        ShowWindow(g_hwndReBar, SW_SHOW);
        CheckMenuItem(hSubMenu, IDM_VIEW_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        g_nTBHeightUI = 0; 
        ShowWindow(g_hwndReBar, SW_HIDE);
        CheckMenuItem(hSubMenu, IDM_VIEW_TOOLBAR, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Set initial state for the toolbar.
    if (g_nRightPaneViewType == 3 /* DC on CAR file (outside) */)
    {
        TB_SetSingleCARFileIcons();
    }

    // Adjust relative postions of windows.
    UpdateLayout();
}

void App_OnToolbarSearch(HWND hWnd)
{
    WNDCLASSEX wc;
    RECT rcMain;
    
    // Destroy the window on the right if it was standard (main) LV.
    if (IsWindowVisible(hwndMainLV))
    {
        ShowWindow(hwndMainLV, FALSE);
    }

    // Destroy the window on the right if it was single CAR file view.
    if (IsWindowVisible(hwndRightHost))
    {
        DestroyWindow(hwndRightHost);
    }

    // Destroy the window on the right if it was single picture view.
    if (IsWindowVisible(hwndRightBottomHost))
    {
        DestroyWindow(hwndRightBottomHost);
    }

    // Prevent creation of multiple welcome views e.g in the case when the user 
    // clicked on TB icon before and create welcome view if the view is not 
    // created. 
    if (!IsWindowVisible(g_hwndWelcome))
    {
        // Make sure that no traces of WV are present and destroy WV even if it 
        // is invisible: as practice shows it can happen and causes overlapping 
        // of WV and other views or malfunctioning.
        DestroyWindow(g_hwndWelcome); 
        g_hwndWelcome = NULL; // LL when destroyed the handle is not set to NULL

        // Now create fresh new WV.
        wc.cbSize = sizeof(WNDCLASSEX); 

        wc.style	     = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc	 = (WNDPROC)Welcome_WndProc;
        wc.cbClsExtra	 = 0;
        wc.cbWndExtra	 = 0;
        wc.hInstance	 = hInst;
        wc.hIcon		 = LoadIcon(hInst, (LPCTSTR)IDI_CONTEXTMENUHANDLER);
        wc.hCursor		 = LoadCursor (NULL /* essential to see cursor */, 
                                                    MAKEINTRESOURCE(IDC_ARROW)); 
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1); // (HBRUSH)GetStockObject(WHITE_BRUSH); 
        //wc.hbrBackground = (HBRUSH)GetStockObject(RGB(255,0,0)); // LL DNM
        wc.lpszMenuName	 = (LPCTSTR)IDR_MENU;
        wc.lpszClassName = "Welcome";
        wc.hIconSm		 = LoadIcon(wc.hInstance, (LPCTSTR)IDR_MAINFRAME);

        RegisterClassEx(&wc);

        // In fact, the position is set in SizeWindowContents so that here are
        // are zeros for RECT. 
        //hwndRightHost = CreateWindowEx(WS_EX_CLIENTEDGE, "Welcome", "",
        g_hwndWelcome = CreateWindowEx(WS_EX_CLIENTEDGE, "Welcome", "",  
            WS_CHILD | WS_VISIBLE | 
            WS_CLIPCHILDREN /* decrease blinking for CE_AnimUI.dll */, 
            0, 0, 0, 0, hWnd, (HMENU)IDC_WELCOME /* does not work */, hInst, 0); 

        // This implicitly calls SizeWindowContents so that Welcome window fit 
        // in to the layout. Albeit artificial, it works. TODO: find natural 
        // solution. TODO: find better solution without the use of 
        // UpdateLayout2.
        UpdateLayout2(TRUE, TRUE);
    }
}

// NOTE: The function is called only if DAD target is within TV area (checked 
//       with WndProc's WM_LBUTTONUP).
BOOL AddDroppedItem(HWND hwndLV, LPPOINT ppt)
{
    LVITEM lvi;
    TVITEM tvi;
    TVINSERTSTRUCT tvis;
    TCHAR szBuf[MAX_PATH]; // TODO: use lower, make #define
    int nCollectionCarIndex;
    int nGlobalCarIndex;
    string sCarTitle;
    CCarFavorites m_CF;
    TVHITTESTINFO hti;
    HTREEITEM hTVItem;
    TVITEM tviTarget;
    char pszBuf[MAX_PATH+1] = "";
    HTREEITEM htviTargetParent;
    HTREEITEM htviParent, htviGrandparent;
    string sTargetFolder, sParentFolder, sGrandparentFolder;

    if (g_iLVDroppedItem != -1)
    {
        // Get global index in CCarData, which coincides with lParam.
        lvi.iItem = g_iLVDroppedItem;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(hwndLV, &lvi);
        nGlobalCarIndex = lvi.lParam;
    
        // Get car index within collection. 
        nCollectionCarIndex = GetCollectionImageIDFromGlobalImageID(
                                                               nGlobalCarIndex);
                                             
        // Make label string for the favorite. 
        if (m_vecCarData.at(nGlobalCarIndex).sModel.size() != 0)
        {
            wsprintf(szBuf, "%s (%s)", 
                m_vecCarData.at(nGlobalCarIndex).sTitle.c_str(), 
                               m_vecCarData.at(nGlobalCarIndex).sModel.c_str());
        }
        else // model string is empty (not provided)
        {
            wsprintf(szBuf, "%s (#%d)", 
                m_vecCarData.at(nGlobalCarIndex).sTitle.c_str(), 
                                                           nCollectionCarIndex);
        }

        // Get the folder where the item dropped.
        hti.pt = *ppt; // client coordinates for TV
        hTVItem = TreeView_HitTest(hwndBrowser, &hti);
        
        ZeroMemory( &tvi, sizeof(tvi) );
        tviTarget.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;  
        tviTarget.hItem = hTVItem;
        tviTarget.pszText = pszBuf;
        tviTarget.cchTextMax = MAX_PATH+1;
        TreeView_GetItem(hwndBrowser, &tviTarget);

        if (tviTarget.lParam == 0 /* Favorites */ || tviTarget.lParam  == 1 || 
                                                          tviTarget.lParam == 2) 
        {	
            // Set parent folder if target is folder.
            if (Utility_IsFavoritesFolder(&tviTarget)) // drop target is folder
            {
                sTargetFolder = tviTarget.pszText;
                // If target is folder, the parent for a new car item coincides 
                // with target (consistency with next segment).
                sParentFolder = sTargetFolder;

                htviParent = tviTarget.hItem; // used for car item insertion

                // Get additionally grandparent if dropped as 2nd level.
                if (tviTarget.lParam == 2)
                {
                    // Get the name of grandparent folder.
                    htviGrandparent = TreeView_GetParent(hwndBrowser, 
                                                                    htviParent);	
                    ZeroMemory(&tvi, sizeof(tvi));
                    tvi.mask = TVIF_TEXT; 
                    tvi.hItem = htviGrandparent;
                    tvi.pszText = pszBuf;
                    tvi.cchTextMax = MAX_PATH+1;
                    TreeView_GetItem(hwndBrowser, &tvi);
                    sGrandparentFolder = tvi.pszText;
                }
            }
            else // drop target is a car item
            {
                // Get parent folder for a new car item, which coincided with 
                // parent of target car item.
                htviTargetParent = TreeView_GetParent(hwndBrowser, 
                                                               tviTarget.hItem);	
                ZeroMemory( &tvi, sizeof(tvi));
                tvi.mask = TVIF_TEXT; 
                tvi.hItem = htviTargetParent;
                tvi.pszText = pszBuf;
                tvi.cchTextMax = MAX_PATH+1;
                TreeView_GetItem(hwndBrowser, &tvi);
                sParentFolder = tvi.pszText;

                htviParent = htviTargetParent; // used for car item insertion

                // Get additionally grandparent if dropped as 2nd level.
                if (tviTarget.lParam == 2)
                {
                    // Get the name of grandparent folder.
                    htviGrandparent = TreeView_GetParent(hwndBrowser, 
                                                              htviTargetParent);	
                    ZeroMemory(&tvi, sizeof(tvi));
                    tvi.mask = TVIF_TEXT; 
                    tvi.hItem = htviGrandparent;
                    tvi.pszText = pszBuf;
                    tvi.cchTextMax = MAX_PATH+1;
                    TreeView_GetItem(hwndBrowser, &tvi);
                    sGrandparentFolder = tvi.pszText;
                }
            }
            
            // Cancel operation if the item already exists under the target 
            // folder. 
            sCarTitle = szBuf;
            for (UINT i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).sParentFolder == sParentFolder 
                        && m_vecCarFavoritesMap.at(i).sCarTitle == sCarTitle)
                {
                    MessageBox(NULL, 
                        "The item already exists under this folder!", 
                                        szMBTitle, MB_OK | MB_ICONINFORMATION);
                    return FALSE;
                }
            }

            // Store dropped item in collection class CCarFavorites to be able 
            // to persist. Notice that only global car index is stored not all 
            // the specification data since the data are already stored in 
            // global CCarData and accessible with nGlobalCarIndex as reference.
            m_CF.nItemLevel = tviTarget.lParam; 
            if (tviTarget.lParam == 2) 
                m_CF.sGrandparentFolder = sGrandparentFolder;
            m_CF.sParentFolder = sParentFolder; 
            m_CF.sTitle = m_vecCarData.at(nGlobalCarIndex).sTitle;
            m_CF.nCollectionImageID = nCollectionCarIndex;
            m_CF.nGlobalImageID_PK = nGlobalCarIndex;
            m_CF.sCarTitle = szBuf; // TODO: sCarLabel
            m_vecCarFavoritesMap.push_back(m_CF);

            tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
            tvi.pszText = szBuf;
            tvi.iImage = 4; 
            tvi.iSelectedImage = 4; 
            tvi.lParam = tviTarget.lParam; 

            tvis.hParent = htviParent; 
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 

            // Insert the item into TV.
            TreeView_InsertItem(hwndBrowser, &tvis);

            // Set flag to persist change to registry on application close.
            g_bUpdateRegistry = TRUE;

            SetFocus(hwndBrowser);
            UpdateWindow(hwndBrowser); // DNW
            TreeView_Expand(hwndBrowser, g_htviFavorites, TVE_EXPAND); 
        }
        // Target anywhere else in TV (is not favorites' TV item - folder or 
        // item).
        else
        {
            // Cancel operation if the item already exists under Favorites. 
            sCarTitle = szBuf;
            for (UINT i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).sCarTitle == sCarTitle)
                {
                    MessageBox(NULL, 
                        "The item already exists under Favorites!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
                    return FALSE;
                }
            }
            
            // Store dropped item in collection class CCarFavorites to be able 
            // to persist. Notice that only global car index is stored not all 
            // the specification data since the data are already stored in 
            // global CCarData and accessible with nGlobalCarIndex as reference.
            m_CF.nItemLevel = 0;
            m_CF.sParentFolder = ""; // added under the root
            m_CF.sTitle = m_vecCarData.at(nGlobalCarIndex).sTitle;
            m_CF.nCollectionImageID = nCollectionCarIndex;
            m_CF.nGlobalImageID_PK = nGlobalCarIndex;
            m_CF.sCarTitle = szBuf; // TODO: sCarLabel
            m_vecCarFavoritesMap.push_back(m_CF);
        
            // Add label to the tree view.
            if (g_htviFavorites)
            {
                tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
                tvi.pszText = szBuf;
                tvi.iImage = 4; 
                tvi.iSelectedImage = 4; 

                tvis.hParent = g_htviFavorites;
                tvis.hInsertAfter = TVI_LAST; 
                tvis.item = tvi; 

                // Insert item.
                TreeView_InsertItem(hwndBrowser, &tvis);

                // Set flag to persist change to registry on application close.
                g_bUpdateRegistry = TRUE;

                SetFocus(hwndBrowser);
                UpdateWindow(hwndBrowser); // DNW
                TreeView_Expand(hwndBrowser, g_htviFavorites, TVE_EXPAND); 
            }
        }

        // TODO: avoid redundancy since only need to set active Favorites 
        // icons when there were no Favorites before.
        
        // Set active Favorites icon for TB.
        SendMessage(g_hwndTB, TB_CHANGEBITMAP, ID_TOOLBAR_FAVORITES, 
            MAKELPARAM(3 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));

        // Set active Favorites icon for TV. 
        tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
        tvi.hItem = g_htviFavorites;
        tvi.iImage = 3; 
        tvi.iSelectedImage = 3;
        TreeView_SetItem(hwndBrowser, &tvi);
    }

    return TRUE;
}

BOOL Utility_IsFavoritesFolder(TVITEM* ptvi)
{
    if (ptvi->iImage == 3 /* favorites root */ ||
        ptvi->iImage == 6 /* standard folder */ || 
        ptvi->iImage == 10 /* standard green folder */ || 
        ptvi->iImage == 12 /* standard pink folder */ ||
        ptvi->iImage == 14 /* standard red folder */ ||
                                            ptvi->iImage == 8 /* user folder */)
       return TRUE;

   return FALSE;
}

BOOL Utility_IsCarItem(TVITEM* ptvi)
{
   if (ptvi->iImage == 4)
       return TRUE;
   return FALSE;
}

// Copied from RT v1.3.
void App_CreateStatusBar()
{
    RECT rcClient;
    HLOCAL hloc;
    LPINT lpParts;
    int i, nWidth;
    int nParts = 3;

    g_hwndStatus = CreateWindowEx(0 /* no extended styles */,                       
        STATUSCLASSNAME, (LPCTSTR) NULL/* no text */,
        SBARS_SIZEGRIP | WS_CHILD, 0, 0, 0, 0 /* ignore size and position*/,
        g_hWnd /* parent */, (HMENU) 334 /* child window identifier */, hInst,      
                                            NULL /* no window creation data */);                   
    
    GetClientRect(g_hWnd, &rcClient);

    // Allocate an array for holding the right edge coordinates.
    hloc = LocalAlloc(LHND, sizeof(int) * nParts);
    lpParts = (LPINT)LocalLock(hloc);

    nWidth = rcClient.right / nParts;
    for (i = 0; i < nParts; i++) 
    { 
       lpParts[i] = nWidth;
       nWidth += nWidth;
    }

    // Tell the status bar to create the window parts.
    SendMessage(g_hwndStatus, SB_SETPARTS, (WPARAM) nParts, (LPARAM)lpParts);

    // Free the array, and return.
    LocalUnlock(hloc);
    LocalFree(hloc);

    ShowWindow(g_hwndStatus, SW_SHOW);
    //HMENU hSubMenu = GetSubMenu(GetMenu(g_hWnd), 1);
    //if (g_bShowStatusBar)
    //{
    //	ShowWindow(g_hwndStatus, SW_SHOW);
    //	CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND | MF_CHECKED);
    //}
    //else
    //{
    //	ShowWindow(g_hwndStatus, SW_HIDE);
    //	CheckMenuItem(hSubMenu, IDM_VIEW_STATUSBAR, 
    //		                                       MF_BYCOMMAND | MF_UNCHECKED);
    //}
}

void App_LoadRegistrySettings()
{
    TCHAR szName[10];
    int i = 0;
    TCHAR szData[MAX_REG_STRING];
    TCHAR *pToken = NULL;
    //TCHAR szRegName[10];
    TCHAR szTitle[200];
    TCHAR szCollectionCarID[10]; // intermediary string
    int nGlobalCarID;
    CCarFavorites m_CF;
    TVITEM tvi;
    TVINSERTSTRUCT tvis; 

    int nItemLevel; // 0..2 are acceptable item-levels in v1.02
    TCHAR szFolder[20];
    TCHAR szSubFolder[20];

    string sParentFolder;
    TCHAR szParentFolder[20];

    string sFolder;
    FavoritesFolders ff;

    HTREEITEM hTVItem;
    TCHAR szCarTitle[200];

    sprintf(szName, "%d", i); // TODO: opposite of atoi?

    while (ReadTextFromRegistry(REGKEY_CE_FAVORITES, szName /* in */ , 
                                                              szData /* out */))
    {
        nItemLevel = Utility_GetItemLevel(szData);

        if (_tcschr(szData, ',') != NULL) // not empty folder like SUV 
        {
            // MSDN: "On the first call to strtok, the function skips leading 
            // delimiters and returns a pointer to the first token in strToken, 
            // terminating the token with a null character". Notice that token 
            // is a word, not a delimiter.
            pToken = _tcstok(szData, ",");
            if (isdigit(*pToken))
            {
                if (nItemLevel == 1 
                         /* first-level item e.g. "1, Sedan, Nissan 2009, 0" */)
                {
                    // Get next token - Folder Name.
                    pToken = _tcstok(NULL, ",");
                    lstrcpy(szFolder, pToken);
                    sFolder = szFolder;
                    if (!IsFolderEnumerated("" /* favorites root*/, sFolder))
                    {
                        TVITEM tvi;
                        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                        TVIF_PARAM; 
                        tvi.pszText = szFolder; 
                        tvi.iImage  = 8;        // user folder
                        tvi.iSelectedImage = 9; // user open folder 
                        tvi.lParam = 1; // direct favorites subfolder (2: 2nd level)
                        
                        tvis.hParent = g_htviFavorites; // 1st level only 
                        tvis.hInsertAfter = TVI_LAST; 
                        tvis.item = tvi; 
                        hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                                TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                        ff.bStandard = 0; // user folder
                        ff.hTVItem = hTVItem;
                        ff.sParentFolder = "";
                        ff.sFolder = sFolder;
                        vecFavoritesFolders.push_back(ff);
                    }

                    // Get next token - Collection Title.
                    pToken = _tcstok(NULL, ",");
                    lstrcpy(szTitle, pToken);
                }
                else if (nItemLevel == 2)
                {
                    pToken = _tcstok(NULL, ",");
                    lstrcpy(szParentFolder, pToken);
                    sParentFolder = szParentFolder;
                    pToken = _tcstok(NULL, ","); // 2, not used
                    pToken = _tcstok(NULL, ",");
                    lstrcpy(szFolder, pToken);
                    sFolder = szFolder;

                    // Create parent folder if not standard and not exists. 
                    // Btw, standard folder always enumerated
                    if (!IsFolderEnumerated("" /* favorites root*/, sParentFolder))
                    {
                        TVITEM tvi;
                        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                        TVIF_PARAM; 
                        tvi.pszText = szParentFolder; 
                        tvi.iImage  = 8;        // user folder
                        tvi.iSelectedImage = 9; // user open folder 
                        tvi.lParam = 1; // direct favorites subfolder
                        
                        tvis.hParent = g_htviFavorites; // 1st level only 
                        tvis.hInsertAfter = TVI_LAST; 
                        tvis.item = tvi; 
                        hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                                TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                        ff.bStandard = 0; // user folder
                        ff.hTVItem = hTVItem;
                        ff.sParentFolder = ""; // root of favorites
                        ff.sFolder = sParentFolder; 
                        vecFavoritesFolders.push_back(ff);
                    }

                    // TODO: generally (arbitrary number of level) quering with
                    // (sParentFolder, sFolder) with TRUE as return does not 
                    // quarantee that sFolder listed (nFolderLevel should be used, 
                    // probably), OK however for a TV with 2 levels of subfolders.

                    // Create folder (2nd level) under parent folder (1st level).
                    if (!IsFolderEnumerated(sParentFolder, 
                                                sFolder /* this one checked */)) 		
                    {
                        TVITEM tvi;
                        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                     TVIF_PARAM; 
                        tvi.pszText = szFolder; 
                        tvi.iImage  = 8;        // user folder
                        tvi.iSelectedImage = 9; // user open folder 
                        tvi.lParam = 2; // 2nd level subfolder
                        
                        //tvis.hParent = GetFavoritesForder(sParentFolder);
                        tvis.hParent = GetFavoritesForder(
                                         "" /* favorites root*/, sParentFolder);
                                                                  
                        tvis.hInsertAfter = TVI_LAST; 
                        tvis.item = tvi; 
                        hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                            TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                        ff.bStandard = 0; // user folder
                        ff.hTVItem = hTVItem;
                        ff.sParentFolder = sParentFolder;
                        ff.sFolder = sFolder;
                        vecFavoritesFolders.push_back(ff);
                    }

                    // Get next token - Collection Title.
                    pToken = _tcstok(NULL, ",");
                    lstrcpy(szTitle, pToken);
                }
            }
            else // simplified format for zero-level (root) items
            {
                    nItemLevel = 0;
                    lstrcpy(szFolder, "Favorites");
                    // Zero-level item under root e.g. "Peugeot 2011, 9".
                    lstrcpy(szTitle, pToken); 
            }

            // Get next token - Car ID in the Collection.
            pToken = _tcstok(NULL, ",");
            lstrcpy(szCollectionCarID, pToken);

            // For example: GetGlobalCarID("Aston Martin 2008", 2).
            nGlobalCarID = GetGlobalCarID(szTitle, atoi(szCollectionCarID));

            // Prepare the next query string.
            i++; sprintf(szName, "%d", i);

            // Add item to the map of CCarFavorites items.
            m_CF.nItemLevel = nItemLevel; // 0 (right under root), 
                                          // 1 (under 1st level) etc
            // Special consideration for 2nd level car items (used when 
            // deleting).
            if (nItemLevel == 2) 
                m_CF.sGrandparentFolder = szParentFolder;
            m_CF.sParentFolder = szFolder; // Favorites (root), or Folder Name
            m_CF.sTitle = szTitle;
            m_CF.nCollectionImageID = atoi(szCollectionCarID); 
            m_CF.nGlobalImageID_PK = nGlobalCarID;

            // Make label string for the favorite. 
            if (m_vecCarData.at(nGlobalCarID).sModel.size() != 0)
            {
                wsprintf(szCarTitle, "%s (%s)", 
                    m_vecCarData.at(nGlobalCarID).sTitle.c_str(), 
                                  m_vecCarData.at(nGlobalCarID).sModel.c_str());
            }
            else // model string is empty (not provided)
            {
                wsprintf(szCarTitle, "%s (#%d)", 
                    m_vecCarData.at(nGlobalCarID).sTitle.c_str(), 
                                                       atoi(szCollectionCarID));
            }

            m_CF.sCarTitle = szCarTitle; // TODO: sCarLabel

            m_vecCarFavoritesMap.push_back(m_CF);

            // Create folder structure under favorites from CCarFavorites.
            // -- TODO: now on the fly, probably create all separately

            // Add item under Favorites in the tree view.
            if (g_htviFavorites)
            {
                tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                     TVIF_PARAM; 
                tvi.pszText = szCarTitle;
                tvi.iImage = 4; 
                tvi.iSelectedImage = 4;
                tvi.lParam = nItemLevel;

                if (nItemLevel == 0)
                    tvis.hParent = g_htviFavorites;
                else if (nItemLevel == 1) 
                    tvis.hParent = GetFavoritesForder("" /* favorites root*/, 
                                                                       sFolder);
                else if (nItemLevel == 2) 
                    tvis.hParent = GetFavoritesForder(sParentFolder, sFolder);

                tvis.hInsertAfter = TVI_LAST; 
                tvis.item = tvi; 

                TreeView_InsertItem(hwndBrowser, &tvis);
                SetFocus(hwndBrowser);
            }
        }
        // We are here because registry item coresponds to an empty folder of 
        // the first or second level.
        else // if (_tcschr(szData, ',') != NULL)
        {
            i++; sprintf(szName, "%d", i); // prepare the next query

            if (_tcschr(szData, '|') == NULL) // first-level empty subfolder
            {
                if (!IsFolderEnumerated("" /* favorites root*/, 
                                         szData)) // can be in standard folders	
                {
                    TVITEM tvi;
                    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                    TVIF_PARAM; 
                    tvi.pszText = szData; 
                    tvi.iImage  = 8;        // user folder
                    tvi.iSelectedImage = 9; // user open folder 
                    tvi.lParam = 1; // direct favorites subfolder (2: 2nd level)
                    
                    tvis.hParent = g_htviFavorites; // 1st level only 
                    tvis.hInsertAfter = TVI_LAST; 
                    tvis.item = tvi; 
                    hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                            TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                    ff.bStandard = 0; // user folder
                    ff.hTVItem = hTVItem;
                    ff.sParentFolder = ""; // root
                    ff.sFolder = szData; 
                    vecFavoritesFolders.push_back(ff);
                }
            }
            else // 2nd level empty subfolder
            {
                pToken = _tcstok(szData, "|");
                lstrcpy(szParentFolder, pToken);

                // Can be standard folder so check it first (== segment for 
                // the 1st-level).	
                if (!IsFolderEnumerated("" /* favorites root*/, szParentFolder))
                {
                    TVITEM tvi;
                    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                    TVIF_PARAM; 
                    tvi.pszText = szParentFolder; 
                    tvi.iImage  = 8;        // user folder
                    tvi.iSelectedImage = 9; // user open folder 
                    tvi.lParam = 1; // direct favorites subfolder (2: 2nd level)
                    
                    tvis.hParent = g_htviFavorites; // 1st level only 
                    tvis.hInsertAfter = TVI_LAST; 
                    tvis.item = tvi; 
                    hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                            TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                    ff.bStandard = 0; // user folder
                    ff.hTVItem = hTVItem;
                    ff.sParentFolder = ""; // root
                    ff.sFolder = szParentFolder; 
                    vecFavoritesFolders.push_back(ff);
                }

                pToken = _tcstok(NULL, "|");
                lstrcpy(szFolder, pToken);

                if (!IsFolderEnumerated(szParentFolder, szFolder)) 
                {
                    TVITEM tvi;
                    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                                    TVIF_PARAM; 
                    tvi.pszText = szFolder; 
                    tvi.iImage  = 8;        // user folder
                    tvi.iSelectedImage = 9; // user open folder 
                    tvi.lParam = 2; // 2nd level subfolder
                    
                    //tvis.hParent = GetFavoritesForder(szParentFolder);
                    tvis.hParent = GetFavoritesForder("" /* favorites root*/, 
                                                                szParentFolder);
                    tvis.hInsertAfter = TVI_LAST; 
                    tvis.item = tvi; 
                    hTVItem = (HTREEITEM)::SendMessage(hwndBrowser, 
                            TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvis);

                    ff.bStandard = 0; // user folder
                    ff.hTVItem = hTVItem;
                    ff.sParentFolder = szParentFolder;
                    ff.sFolder = szFolder; 
                    vecFavoritesFolders.push_back(ff);
                }
            }
        }
    } // while (ReadTextFromRegistry(..))

    if (g_bEnableDebug)
    {
        App_Debug("Start: printout of all entries in vecFavoritesFolders...");
        TCHAR szTest[100];
        for (int i = 0; i < vecFavoritesFolders.size(); i++)
        {
            wsprintf(szTest, 
               "i: %d, bStandard: %d, hTVItem: %d ParentFolder: %s, Folder: %s", 
                i, vecFavoritesFolders.at(i).bStandard, 
                vecFavoritesFolders.at(i).hTVItem, 
                vecFavoritesFolders.at(i).sParentFolder.c_str(),
                                     vecFavoritesFolders.at(i).sFolder.c_str());
            App_Debug(szTest);
        }
        App_Debug("End of printout");
    }
}

int Utility_GetItemLevel(LPCTSTR pszData) 
{
     LPCTSTR pch = NULL;
     int nCount = 0;
     int nLevel;

     // Get count of occurences character "'" in the string.
     pch = _tcschr(pszData, ',');
     while (pch != NULL)
     {
         nCount++;
         pch = _tcschr(pch + 1, ',');
     }

     // Translate the number of occurencies into item level.
     switch (nCount)
     {
     //case 0: // SUV
     //    break;
     case 1: // Nissan 2009,1
         nLevel = 0;
         break;
     case 3: // 1,Sedan,Toyota 2010,1
         nLevel = 1;
         break; 
     case 5: // 1,Sedan,2,$20-25K,Nissan 2009,0
         nLevel = 2;
         break; 
     default:
         nLevel = -1; // error
         break;
     }

     return nLevel;
}

// PURPOSE: Get car item level when importing favorites from CSV file.
// NOTES:   In CSV format (more simple) currently differs from registry. In 
//          future releases  the CSV format is preferrable (don't have time to 
//          unify), and this function can be merged with Utility_GetItemLevel.
//
//                  Zero level: 0,Peugeot 2011,9
//                  1st level:  31,SUV,Kia 2009,10
//                  2nd level:  65,Convertible,$25-30K,Peugeot 2011,8

int Utility_GetItemLevelFromCSV(LPCTSTR pszLine) 
{
     LPCTSTR pch = NULL;
     int nCount = 0;
     int nLevel;

     // Get count of occurences character "'" in the string.
     pch = _tcschr(pszLine, ',');
     while (pch != NULL)
     {
         nCount++;
         pch = _tcschr(pch + 1, ',');
     }

     // Translate the number of comma occurencies into item level.
     switch (nCount)
     {
     //case 0 || 1: 
     //    break;
     case 2: // 0,Peugeot 2011,9
         nLevel = 0;
         break;
     case 3: // 31,SUV,Kia 2009,10
         nLevel = 1;
         break; 
     case 4: // 65,Convertible,$25-30K,Peugeot 2011,8
         nLevel = 2;
         break; 
     default:
         nLevel = -1; // error
         break;
     }

     return nLevel;
}

void App_OnSaveSettings()
{
    TCHAR szName[10];
    TCHAR szData[MAX_REG_STRING];
    string sFolderParentFolder; 
    HTREEITEM htviItemParentFolder;
    int nEmptyFolderCount = 0;


    // Debugging code.
    if (g_bEnableDebug)
    {
        App_Debug(
        "App_OnSaveSettings(): printout all items in vecFavoritesFolders...");
        TCHAR szTest[100];
        for (int i = 0; i < vecFavoritesFolders.size(); i++)
        {
            wsprintf(szTest, 
               "i: %d, bStandard: %d, hTVItem: %d ParentFolder: %s, Folder: %s", 
                i, vecFavoritesFolders.at(i).bStandard, 
                vecFavoritesFolders.at(i).hTVItem, 
                vecFavoritesFolders.at(i).sParentFolder.c_str(),
                                     vecFavoritesFolders.at(i).sFolder.c_str());
            App_Debug(szTest);
        }

        App_Debug("-----------m_vecCarFavoritesMap:---------------");

        for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        {
            wsprintf(szTest, 
                "i: %d, GPFolder: %s, PFolder: %s, Title: %s", 
                i, m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(), 
                m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                m_vecCarFavoritesMap.at(i).sTitle.c_str());
            App_Debug(szTest);
        }
        App_Debug("End of printout");
    }

    // Notice that writing favorites to registry is always suppressed when DC 
    // on CAR file out of the default folder since favorites are not even 
    // read in this case.
    if (g_bUpdateRegistry /* set when favorites updated */ && 
               g_bPersistFavorites /* crippling */ && g_nRightPaneViewType != 3)
                                                      
    {
        // Remove existing favorites from registry. Notice that (1) removal is 
        // done to make numerical identifiers of favorites uninterrupted, (2) 
        // registry key Favorites is recreated (deleted and created).
        DeleteFavoritesFromRegistry(); 
        
        for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        {
            wsprintf(szName, "%d", i); 
            if (m_vecCarFavoritesMap.at(i).nItemLevel == 0)
            {
                wsprintf(szData, "%s,%d", 
                    m_vecCarFavoritesMap.at(i).sTitle.c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
            }
            else if (m_vecCarFavoritesMap.at(i).nItemLevel == 1)
            {
                wsprintf(szName, "%d", i); 
                wsprintf(szData, "1,%s,%s,%d", 
                    m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                    m_vecCarFavoritesMap.at(i).sTitle.c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
            }
            else if (m_vecCarFavoritesMap.at(i).nItemLevel == 2)
            {
                wsprintf(szName, "%d", i); 

                // Get the name of 1st level folder.
                htviItemParentFolder = GetFavoritesForder(
                    m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(),
                              m_vecCarFavoritesMap.at(i).sParentFolder.c_str());
                for (int j= 0; j < vecFavoritesFolders.size(); j++)
                {
                   if (vecFavoritesFolders.at(j).hTVItem == 
                                                           htviItemParentFolder)
                   {
                       sFolderParentFolder = 
                                        vecFavoritesFolders.at(j).sParentFolder;
                       break;
                   }
                }

                // Make the string for 2nd level item: 
                //    "1,<1stlevelfolder>,2,<2ndlevelfolder>,<title>,<imageID>".
                wsprintf(szData, "1,%s,2,%s,%s,%d", sFolderParentFolder.c_str(),
                    m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                    m_vecCarFavoritesMap.at(i).sTitle.c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
            }
            StoreTextToRegistry(REGKEY_CE_FAVORITES, szName, szData);
        }
        
        // After string items, store in separate format not standard empty
        // subfolders.
        for (i = 0; i < vecFavoritesFolders.size(); i++)
        {
            if (!TreeView_GetChild(hwndBrowser, 
                vecFavoritesFolders.at(i).hTVItem) && 
                    vecFavoritesFolders.at(i).bStandard != 1 /* not standard */) 
            {
                nEmptyFolderCount++;
                wsprintf(szName, "%d", (m_vecCarFavoritesMap.size() - 1) + 
                                                             nEmptyFolderCount); 
                if (vecFavoritesFolders.at(i).sParentFolder.length() 
                                                           == 0 /* 1st level empty folder */)
                {
                    wsprintf(szData, "%s", 
                                     vecFavoritesFolders.at(i).sFolder.c_str());
                }
                else /* 2nd level empty folder, deeper are prohibited */
                {
                    string sTwoFoldersRecord;
                    string sParentFolder;
                    string sSeparator = "|";
                    string sFolder;
                     
                    sParentFolder = vecFavoritesFolders.at(i).sParentFolder;
                    sFolder = vecFavoritesFolders.at(i).sFolder;
                    sTwoFoldersRecord = sParentFolder + sSeparator  + sFolder;
                    wsprintf(szData, "%s", sTwoFoldersRecord.c_str());
                }

                StoreTextToRegistry(REGKEY_CE_FAVORITES, szName, szData);
            }
        }
    }

    // Store the location of CAR collections if it was changed.
    if (g_bPersistCarFilesFolder)
        StoreTextToRegistry(REGKEY_CE, "CarFilesFolder", g_szCarFilesFolder);

    // Persist all other relevant settings.
    SaveRegistryData();
}

// Purpose: converts strings like "$45,345" to "45345" removing "$" and
//          "," symbols (default use). Can also be used to remove "$" and ","
//          from strings like "$33,455,888" to "33455888" (should work, 
//          albeit not tested). Returns 0 from empty input string and -1 for
//          common none-digit string like "None".

int Utility_ConvertPriceToNumber(LPCTSTR pszPrice, int nSortOrder)
{
    TCHAR szNumPrice[100]; // only numbers
    int nCount = 0;
    int nPrice;
    int nLength = lstrlen(pszPrice) + 1; // with NUL character

    // Note that nSortOrder is used to keep empty cells always at the very 
    // buttom.
    if (lstrlen(pszPrice) == 0) return nSortOrder ?  9000001 : -1;  // up/down

    for (int i = 0; i < nLength; i++)
    {
        switch (*pszPrice)
        {
        case '$':
            // Skip that is replace with empty space.
            *pszPrice++;
            break;
        case ',':
            // Skip that is replace with empty space.
            *pszPrice++;
            break;
        case '\0':
            szNumPrice[nCount] = '\0';
            *pszPrice++;
            break;
        //case '.': // not used, reserved
        //	// Skip that is replace with empty space.
        //	break;
        default: // supposedly number character
            if (isdigit(*pszPrice)) 
            {
                szNumPrice[nCount] = *pszPrice;
                nCount++;
            }
            else
            {
                // Note that nSortOrder is used to keep "No data" cells always 
                // at the buttom, but above empty cells. This also keeps other
                // irrelevant strings at the same level (TODO: clarify logic).
                return nSortOrder ? 9000000 : 0; // up/down
            }
            *pszPrice++;
            break;
        }
    }

    nPrice = atoi(szNumPrice);

    return nPrice;
}

// Purpose: Converts strings like "3,219" to "3219" removing comma. 
// Note:    Based on Utility_ConvertPriceToNumber.

int Utility_ConvertWeightToNumber(LPCTSTR pszWeight, int nSortOrder)
{
    TCHAR szNumWeight[10]; // only numbers
    int nCount = 0;
    int nWeight;
    int nLength = lstrlen(pszWeight) + 1; // with NUL character

    // Note that nSortOrder is used to keep empty cells always at the very 
    // buttom.
    if (lstrlen(pszWeight) == 0) return nSortOrder ?  10001 : -1;  // up/down

    for (int i = 0; i < nLength; i++)
    {
        switch (*pszWeight)
        {
        case ',':
            // Skip.
            *pszWeight++;
            break;
        case '\0':
            szNumWeight[nCount] = '\0';
            *pszWeight++;
            break;
        default: // supposedly number character
            if (isdigit(*pszWeight)) 
            {
                szNumWeight[nCount] = *pszWeight;
                nCount++;
            }
            else
            {
                // Note that nSortOrder is used to keep "No data" cells always 
                // at the buttom, but above empty cells. This also keeps other
                // irrelevant strings at the same level.
                if (lstrcmp(pszWeight, "No data") == 0)
                    return nSortOrder ? 10000 : 0; // up/down
            }
            *pszWeight++;
            break;
        }
    }

    nWeight = atoi(szNumWeight);

    return nWeight;
}

BOOL OnNotify_TVItemClick(HWND hWnd, LPARAM lParam)
{
    NMHDR* pNMHDR = (LPNMHDR) lParam;
    DWORD dwExStyles;
    DWORD dwStyle;
    TCHAR szText[MAX_LOADSTRING];
    //NMTREEVIEW* pNM_TREEVIEW = NULL; 
    //TVITEM tvi;
    //RECT rc;
    //RECT rcRBH;
    //char szBuf[MAX_PATH + 1] = "";
    //DWORD dwExStyles;
    //HTREEITEM tviParent;
    //TVITEM tviNC; // no children
    //TCHAR szFFN[MAX_PATH];
    RECT rcMain;
    int nCollectionImageID;
    //string sOldLabel, sNewLabel;
    string sNewLabel;

    if (pNMHDR->code == TVN_SELCHANGED)
    {
        g_nRightPaneViewType = 1; // 0 - welcome page, 1 - all others
        NMTREEVIEW* pNM_TREEVIEW = (LPNMTREEVIEW) lParam; 
        TVITEM tvi;
        RECT rc;
        RECT rcRBH;

        // LL unlike LVITEM, the string buffer for TVITEM must be 
        // specified manually. 
        char szBuf[MAX_PATH + 1] = "";
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_TEXT | TVIF_IMAGE;
        tvi.hItem = pNM_TREEVIEW->itemNew.hItem;
        tvi.pszText = szBuf; 
        tvi.cchTextMax = MAX_PATH + 1;
        TreeView_GetItem(hwndBrowser, &tvi); 
        // Clear report view before re-populating.
        ListView_DeleteAllItems(hwndMainLV);

        TCHAR szDebugMsg[100];
        wsprintf(szDebugMsg, "A: hwndTV: %d tvi.hItem: %d", hwndBrowser, tvi.hItem); 
        //if (g_bEnableDebug) App_Debug(szDebugMsg);

        // Populate list view depending on the item type of tree view: 
        // if an item has a child hence (1) a folder was clicked, if an item 
        // has no children hence (2) CAR file was clicked, or (3) single picture 
        // item was clicked.
        if (TreeView_GetChild(hwndBrowser, tvi.hItem) /* TV folder */)
        {
            // Disable TB icon for single CAR file view.
            SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                ID_TOOLBAR_VIEW, 
                MAKELPARAM(4 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));

            // Cause WM_SIZE to avoid layout distortion.
            UpdateLayout2(TRUE, TRUE);
            
            // Previous view on the right was the same (one of the 
            // groups). Note that standard view (group view) is never 
            // destroyed, but single Car view is destroyed and 
            // re-created.
            if (IsWindowVisible(hwndMainLV))
            {
                if (lstrcmp(tvi.pszText, "All") == 0 /* equal */)
                {
                    //if (m_vecCarFilesMap.size() != 0) InitListViewItems(hwndMainLV);
                    InitListViewItems(hwndMainLV);
                }
                else
                {
                    // Favorites (root folder or subfolder).
                    if (tvi.lParam == 0 /* Favorites */ || 
                                       tvi.lParam == 1 /* 1st level subfolder*/)
                    {
                        BrowseFavorites(hwndMainLV, 
                            tvi.lParam /* folder level */, NULL /* not used */,
                                                      tvi.pszText /* filter */);
                    }
                    else if (tvi.lParam == 2 /* 2nd level subfolder */) 
                    {
                        string sParentFolder;
                        sParentFolder = Utility_GetTVParentFolderName(tvi);
                        BrowseFavorites(hwndMainLV, 
                            tvi.lParam /* folder level */, 
                               sParentFolder.c_str(), tvi.pszText /* filter */);
                    }
                    else
                        // Folders with CAR files.
                        InitListViewItems(hwndMainLV, tvi.pszText /* filter */);
                }

                // Update status bar information.
                SB_ShowCollectionsSummary(tvi.pszText, tvi.hItem, hwndMainLV);
            
                // Enable crippling for the main LV, except Favorites folder.
                if (lstrcmp(tvi.pszText, "Favorites") != 0 /* equal */)
                {
                    if (g_bFavoritesLoaded) // TODO: clarify use of g_bFavoritesLoaded
                                            // (LL redundant since 
                                            // LoadFavorites_ThreadProc already 
                                            // checks (#4 scenario)
                    {
                        if (g_bUseKagiPaymentProcessing)
                        {
                            if (!g_bRegisteredCopy && m_RP.IsTrialUseExpired())
                            {
                                DialogBox(hInst, 
                                    (LPCTSTR)IDD_REGISTER_BUY_PRODUCT, g_hWnd, 
                                           (DLGPROC)RegisterBuyProduct_WndProc);
                            }
                        }
                    }
                }
            }
            else // previous view on the right was WV
            {
                if (g_hwndWelcome) 
                {
                    DestroyWindow(g_hwndWelcome); 
                    // Set g_hwndWelcome to NULL since LL DestroyWindow 
                    // does not zeros the handle.
                    g_hwndWelcome = NULL;  
                }
                if (hwndRightHost) DestroyWindow(hwndRightHost);
                if (hwndRightBottomHost) DestroyWindow(hwndRightBottomHost);
                ShowWindow(hwndMainLV, TRUE);
                if (lstrcmp(tvi.pszText, "All") == 0 /* equal */)
                    InitListViewItems(hwndMainLV);
                else
                {
                    if (tvi.lParam == 0 /* Favorites */ || 
                                       tvi.lParam == 1 /* 1st level subfolder*/) 
                        BrowseFavorites(hwndMainLV, 
                            tvi.lParam /* folder level */, NULL /* not used */,
                                                      tvi.pszText /* filter */);
                    else if (tvi.lParam == 2 /* 2nd level subfolder */) 
                    {
                        string sParentFolder;
                        sParentFolder = Utility_GetTVParentFolderName(tvi);
                        BrowseFavorites(hwndMainLV, 
                            tvi.lParam /* folder level */, 
                               sParentFolder.c_str(), tvi.pszText /* filter */);
                    }
                    else
                        // Folders with CAR files.
                        InitListViewItems(hwndMainLV, tvi.pszText /* filter */);
                }

                // Update status bar information.
                SB_ShowCollectionsSummary(tvi.pszText, tvi.hItem, hwndMainLV);

                if (g_bUseKagiPaymentProcessing)
                {
                    // Enable crippling for the main LV, except Favorites 
                    // folder.
                    if (lstrcmp(tvi.pszText, "Favorites") != 0 /* equal */)
                    {
                        if (!g_bRegisteredCopy && m_RP.IsTrialUseExpired())
                        {
                            DialogBox(hInst, (LPCTSTR)IDD_REGISTER_BUY_PRODUCT, 
                                   g_hWnd, (DLGPROC)RegisterBuyProduct_WndProc);
                        }
                    }
                }
            }

            // Update units according to loaded from registry or updated.
            if (g_iUnits != DEFAULT_UNITS) ConvertUnits(hwndMainLV, g_iUnits);

            // Sort by g_nSortColumn if relevant.
            if (g_iSortParam != -1 /* no sort if was not used */) // g_nSortColumn
                                        LV_SortItems(hwndMainLV, g_nSortColumn);
        }
        else if (TreeView_GetChild(hwndBrowser, tvi.hItem) == 0 &&
                             lstrcmp(tvi.pszText, "Favorites") == 0 /* equal */) 
        {
            // Update status bar info.
            wsprintf(szText, "%s folder: 0 favorites", tvi.pszText); 
            SendMessage(g_hwndStatus, WM_SETTEXT, 0 /* not used */, 
                                                                (LPARAM)szText);
            if (!g_bFavoritesLoaded) 
                // Notice that MB_TOPMOST style bring MS to the front (thread 
                // issue).
                MessageBox(0, "The favorites data are not yet loaded!", 
                            szMBTitle, MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
            else
            {
                LoadString(hInst, IDS_NO_FAVORITES, szText, MAX_LOADSTRING);
                // Avoid MB if click was not caused by mouse or key e.g. when 
                // last favotite item was deleted.
                if (((NM_TREEVIEW*)lParam)->action == TVC_BYMOUSE  ||
                               ((NM_TREEVIEW*)lParam)->action == TVC_BYKEYBOARD)
                    MessageBox(0, szText, szMBTitle, 
                                       MB_TOPMOST | MB_OK | MB_ICONINFORMATION);
            }
        }
        // Empty 1st level favorites subfolder.
        else if (TreeView_GetChild(hwndBrowser, tvi.hItem) == 0 &&
            Utility_IsFavoritesFolder(&tvi) &&
                            tvi.lParam == 1 /* 1st level favorites subfolder */)
        {
               // Notice additional check for tvi.lParam since car item has the 
            // same lParam on the same level as folder. 
            //MessageBox(0, "", "TreeView_GetChild(hwndBrowser, tvi.hItem)", 0);
        }
        // Empty 2nd level favorites subfolder.
        else if (TreeView_GetChild(hwndBrowser, tvi.hItem) == 0 &&
            Utility_IsFavoritesFolder(&tvi) &&
                            tvi.lParam == 2 /* 2nd level favorites subfolder */)
        {
               // Notice additional check for tvi.lParam since car item has the 
            // same lParam on the same level as folder.
            //MessageBox(0, "", "TreeView_GetChild(hwndBrowser, tvi.hItem)", 0);
        }
        else if (TreeView_GetChild(hwndBrowser, tvi.hItem) == 0 &&
                                   lstrcmp(tvi.pszText, "All") == 0 /* equal */)
        {
            //MessageBox(0, tvi.pszText, "TVN_SELCHANGED", 0);
            // no CAR or MSS files found
        }
        else // CAR file or a picture under Favorites (TV item has no children)
        {
            HTREEITEM tviParent;
            TVITEM tviNC; // no children
            tviParent = TreeView_GetParent(hwndBrowser, tvi.hItem);
            char szBuf[MAX_PATH + 1] = "";
            ZeroMemory(&tviNC, sizeof(tviNC));
            tviNC.mask = TVIF_TEXT | TVIF_PARAM;
            tviNC.hItem = tviParent;
            tviNC.pszText = szBuf; 
            tviNC.cchTextMax = MAX_PATH + 1;
            TreeView_GetItem(hwndBrowser, &tviNC);

            // Process CAR file selection in TV.
            if (lstrcmp(szBuf, "Favorites") != 0 /* not equal */ 
                //&& tviNC.lParam != 1 
                && tviNC.lParam != 1 && tviNC.lParam != 2 
                               /* not Favorites's 1st or 2nd level subfolder */) 
            {
                // Activate TB icon that toggles views for single CAR file view.
                if (g_nCarFilesLayout == 0 /* report view */)
                {
                    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                        ID_TOOLBAR_VIEW, 
                        MAKELPARAM(8 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));
                }
                else if (g_nCarFilesLayout == 1 /* icon view */)
                {
                    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                        ID_TOOLBAR_VIEW, 
                        MAKELPARAM(7 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));
                }
                //{
                //DWORD dwStyle = GetWindowLong(hwndRightHost, GWL_STYLE); 
                //if (dwStyle & LVS_REPORT) 
                //{
                //	dwStyle &=~ LVS_REPORT; // remove style
                //	dwStyle |= LVS_ICON;    // add style 
                //	SetWindowLong(hwndRightHost, GWL_STYLE, dwStyle);
                //}

                //// TODO: get FFN from clicked MSS.
                //TCHAR szFFN[MAX_PATH];
                //GetImageSetFFN(tvi.pszText, szFFN);
                //lstrcpy(g_szFullFileName, szFFN);
                //LoadPicturesAndStrings();

                //DestroyWindow(hwndMainLV);
                // Re-create the single Car view only if the previous 
                // view was the standard view.
                if (IsWindowVisible(hwndMainLV))
                {
                    ShowWindow(hwndMainLV, FALSE);
                }
                // Destroy alternative (or older) views if they are visible.
                if (g_hwndWelcome) DestroyWindow(g_hwndWelcome);
                if (hwndRightHost) DestroyWindow(hwndRightHost);
                if (hwndRightBottomHost) DestroyWindow(hwndRightBottomHost);

                // FFN from clicked MSS. Notice that getting 
                // g_szFullFileName must be before hwndPictureView 
                // window constuction since its WM_PAINT handler uses
                // m_SinglePicture obtained from g_szFullFileName.
                TCHAR szFFN[MAX_PATH];
                GetImageSetFFN(tvi.pszText, szFFN); 
                //MessageBox(0, tvi.pszText, szFFN, 0);
                lstrcpy(g_szFullFileName, szFFN);

                // The dialog class (#32770) is used to make horizontal 
                // splitter bar the same appearance as vertical splitter bar.
                hwndRightHost = CreateWindow("#32770", "", 
                    WS_CHILD | WS_VISIBLE, 400, 200, 400, 300, hWnd, 
                    (HMENU)IDC_RIGHT_HOST /* does not work */, hInst, 0);

                // Subclass to have separate procedure from main window 
                // procedure.
                g_wpOldRightHost_DialogProc = (WNDPROC)SetWindowLong(
                    hwndRightHost, GWL_WNDPROC, 
                                    (DWORD)RightHostDialogProc_SubclassedFunc);

                GetWindowRect(hWnd, &rc);

                if (g_nCarFilesLayout == 0)
                    dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | 
                                                                     LVS_REPORT;
                else if (g_nCarFilesLayout == 1)
                    dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | 
                                                     LVS_ICON | LVS_AUTOARRANGE;

                g_hwndLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, 
                    "Single CAR file view", dwStyle,
                    0, 0, rc.right-rc.left, 500, 
                                hwndRightHost, (HMENU)IDC_LISTVIEW, hInst, 0); 

                // Works with CreateWindow-created (without Ex) window.
                dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
                ListView_SetExtendedListViewStyle(g_hwndLV, dwExStyles);

                // The dialog class (#32770) is used to make vertical
                // splitter bar (bottom area on the right) the same appearance with
                // other splitter bars.
                hwndRightBottomHost = CreateWindow("#32770", "", 
                    WS_CHILD | WS_VISIBLE, 0, 505, rc.right-rc.left, 300, hwndRightHost, 
                    (HMENU)IDC_RIGHT_BOTTOM_HOST /* does not work */, hInst, 0);

                // Subclass to have separate procedure from main window procedure.
                g_wpOldRBH_DialogProc = (WNDPROC)SetWindowLong(hwndRightBottomHost, GWL_WNDPROC, 
                    (DWORD)RBHDialogProc_SubclassedFunc);

                // Add left and right bottom windows.
                WNDCLASSEX wcex;
                wcex.cbSize = sizeof(WNDCLASSEX); 
                wcex.style			= CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc	= (WNDPROC)BottomLeftWindow_WndProc;
                wcex.cbClsExtra		= 0;
                wcex.cbWndExtra		= 0;
                wcex.hInstance		= hInst;
                wcex.hIcon			= 0;
                wcex.hCursor		= NULL;
                wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1); 
                wcex.lpszMenuName	= NULL;
                wcex.lpszClassName	= _T("BottomLeftWindow");
                wcex.hIconSm		= NULL;
                RegisterClassEx(&wcex);

                GetWindowRect(hwndRightBottomHost, &rcRBH);

                hwndPictureView = CreateWindowEx(WS_EX_CLIENTEDGE, 
                    "BottomLeftWindow", "", WS_VISIBLE | WS_CHILD, 
                    0, 0, 200, rcRBH.bottom - rcRBH.top, 
                    hwndRightBottomHost, (HMENU)IDC_BOTTOM_PICTUREVIEW, 
                                                                hInst, 0);

                TCHAR sxText[200];
                wsprintf(sxText, "hwndRightBottomHost' RC: (%d %d) - (%d %d)", 
                    rcRBH.left, rcRBH.top, 
                    rcRBH.right - rcRBH.left, rcRBH.bottom - rcRBH.top);  
                //App_Debug(sxText);
        
                hwndBottomLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", 
                    WS_CHILD | WS_VISIBLE | WS_BORDER |LVS_REPORT, 
                    200 + 4, 0, rcRBH.right - rcRBH.left - 200 - 4, 
                    rcRBH.bottom - rcRBH.top, hwndRightBottomHost, 
                                    (HMENU)IDC_BOTTOM_LISTVIEW, hInst, 0);

                // Works with CreateWindow-created (without Ex) window.
                dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
                ListView_SetExtendedListViewStyle(hwndBottomLV, dwExStyles);

                // Setup list view colums.
                LVCOLUMN lvc;
                lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

                lvc.iSubItem = 0;
                lvc.pszText = "Property";
                lvc.cx = 100; 
                ListView_InsertColumn(hwndBottomLV, 0, &lvc);

                lvc.iSubItem = 1;
                lvc.pszText = "Value";
                lvc.fmt = LVCFMT_CENTER;
                lvc.cx = 200;
                ListView_InsertColumn(hwndBottomLV, 1, &lvc);

                lvc.iSubItem = 2;
                lvc.pszText = "Unit";
                lvc.fmt = LVCFMT_CENTER;
                lvc.cx = 60;
                ListView_InsertColumn(hwndBottomLV, 2, &lvc);

                // When initialized load first picture from MSS (loads also notes for
                // general MSS).
                LoadXMLData(0 /* zero-based index */);

                // Imitates main window resizing or horizontal bar moving to make the 
                // window visible.
                GetWindowRect(hWnd, &rc);
                //SizeWindowContents(rc.right, rc.bottom);

                // Update layout. 
                RECT rcMainClient;
                GetClientRect(hWnd, &rcMainClient); // client RECT is essentia
                SizeWindowContents(rcMainClient.right - rcMainClient.left, 
                                        rcMainClient.bottom - rcMainClient.top);

                if (g_nCarFilesLayout == 0 /* report view */)
                {
                    InitListViewColumns(g_hwndLV);
                    // Populate list view with details from m_vecCarData vector.
                    InitListViewItems(g_hwndLV, tvi.pszText /* filter */, 
                                                   TRUE /* MSS file clicked */);

                    // Update units according to loaded from registry or 
                    // updated.
                    if (g_iUnits != DEFAULT_UNITS) 
                                               ConvertUnits(g_hwndLV, g_iUnits);

                    // Sort by g_nSortColumn if relevant.
                    if (g_iSortParam != -1 /* no sort if was not used */) 
                                          LV_SortItems(g_hwndLV, g_nSortColumn);
                }
                else if (g_nCarFilesLayout == 1 /* icon view */)
                {
                    dwStyle = GetWindowLong(g_hwndLV, GWL_STYLE); 
                    if (dwStyle & LVS_REPORT) 
                    {
                        dwStyle &=~ LVS_REPORT; // remove style
                        dwStyle |= LVS_ICON;    // add style 
                        SetWindowLong(g_hwndLV, GWL_STYLE, dwStyle);
                    }

                    LoadPicturesAndStrings(g_hwndLV);
                }

                SB_ShowCollectionInfo(szFFN);
            } // lstrcmp(szBuf, "Favorites") != 0
            else // single car picture under Favorites (all levels)
            {
                // Disable TB icon for single CAR file view.
                SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                    ID_TOOLBAR_VIEW, 
                    MAKELPARAM(4 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));	
                
                g_nRightPaneViewType = 2; // SetWindowsContents

                // Hide it Main LV window if visible. Notice that by design 
                // Main LV window is always available visible or not. 
                if (IsWindowVisible(hwndMainLV))
                {
                    ShowWindow(hwndMainLV, FALSE);
                }

                // Destroy alternative (or older) views if they are visible.
                if (g_hwndWelcome) DestroyWindow(g_hwndWelcome);
                if (hwndRightHost) DestroyWindow(hwndRightHost);
                if (hwndRightBottomHost) DestroyWindow(hwndRightBottomHost);

                TCHAR szFFN[MAX_PATH];
                // Notice that collection's ImageID is passed as 
                // CreateWindowEx's lParam.
                nCollectionImageID = GetCollectionImageIDFromCarTitle(
                                                            tvi.pszText, szFFN);
                //MessageBox(0, tvi.pszText, szFFN, 0);
                lstrcpy(g_szFullFileName, szFFN);

                // The dialog class (#32770) is used to make vertical
                // splitter bar the same appearance with other splitter bars.
                hwndRightBottomHost = CreateWindow("#32770", "", 
                    WS_CHILD | WS_VISIBLE, 0, 505, 500, 600, hWnd, 
                    (HMENU)IDC_RIGHT_BOTTOM_HOST /* does not work */, hInst, 0);

                // Subclass to have separate procedure from main window 
                // procedure.
                g_wpOldRBH_DialogProc = (WNDPROC)SetWindowLong(
                    hwndRightBottomHost, GWL_WNDPROC, 
                                           (DWORD)RBHDialogProc_SubclassedFunc);

                // Add left and right bottom windows.
                WNDCLASSEX wcex;
                wcex.cbSize = sizeof(WNDCLASSEX); 
                wcex.style			= CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc	= (WNDPROC)BottomLeftWindow_WndProc;
                wcex.cbClsExtra		= 0;
                wcex.cbWndExtra		= 0;
                wcex.hInstance		= hInst;
                wcex.hIcon			= 0;
                wcex.hCursor		= NULL;
                wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1); 
                wcex.lpszMenuName	= NULL;
                wcex.lpszClassName	= _T("BottomLeftWindow");
                wcex.hIconSm		= NULL;
                RegisterClassEx(&wcex);

                GetWindowRect(hwndRightBottomHost, &rcRBH);

                hwndPictureView = CreateWindowEx(WS_EX_CLIENTEDGE, 
                    "BottomLeftWindow", "", WS_VISIBLE | WS_CHILD, 
                    0, 0, 200, rcRBH.bottom - rcRBH.top, 
                    hwndRightBottomHost, (HMENU)IDC_BOTTOM_PICTUREVIEW, 
                        hInst, (LPVOID)(LPARAM)nCollectionImageID /* lParam */);

                hwndBottomLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", 
                    WS_CHILD | WS_VISIBLE | WS_BORDER |LVS_REPORT, 
                    200 + 4, 0, rcRBH.right - rcRBH.left - 200 - 4, 
                    rcRBH.bottom - rcRBH.top, hwndRightBottomHost, 
                                          (HMENU)IDC_BOTTOM_LISTVIEW, hInst, 0);

                // Works with CreateWindow-created (without Ex) window.
                dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
                ListView_SetExtendedListViewStyle(hwndBottomLV, dwExStyles);

                // Setup list view colums.
                LVCOLUMN lvc;
                lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

                lvc.iSubItem = 0;
                lvc.pszText = "Property";
                lvc.cx = 100; 
                ListView_InsertColumn(hwndBottomLV, 0, &lvc);

                lvc.iSubItem = 1;
                lvc.pszText = "Value";
                lvc.fmt = LVCFMT_CENTER;
                lvc.cx = 200;
                ListView_InsertColumn(hwndBottomLV, 1, &lvc);

                lvc.iSubItem = 2;
                lvc.pszText = "Unit";
                lvc.fmt = LVCFMT_CENTER;
                lvc.cx = 60;
                ListView_InsertColumn(hwndBottomLV, 2, &lvc);

                // When initialized load first picture from MSS (loads also notes for
                // general MSS).
                LoadXMLData(nCollectionImageID /* zero-based index */);

                // Update layout. 
                RECT rcMainClient;
                GetClientRect(hWnd, &rcMainClient); // client RECT is essential
                SizeWindowContents(rcMainClient.right - rcMainClient.left, 
                                        rcMainClient.bottom - rcMainClient.top);
            }
        } // TV item has no children
    }
    // Get folder name in TV under Favorites before update.
    else if (pNMHDR->code == TVN_BEGINLABELEDIT)
    {
        NMTVDISPINFO* ptvdi = (LPNMTVDISPINFO)lParam; 
        g_sOldLabel = ptvdi->item.pszText;
    }
    // Update folder name in TV under Favorites if changed.
    else if (pNMHDR->code == TVN_ENDLABELEDIT)
    {
        NMTVDISPINFO* ptvdi = (LPNMTVDISPINFO)lParam; 
        TCHAR szMsg[MAX_LOADSTRING];

        // TODO: probably prompt a message (IDS_TV_ZEROLENTHWARNING) when new 
        // is zero-length (unclear how  to distiguish when not edited - NULL, 
        // zero length).
        if (ptvdi->item.pszText == NULL || lstrlen(ptvdi->item.pszText) == 0
                                    /* no label change or zero-length string*/) 
            return FALSE; // reject and revert to the original label
        else 
        {
            sNewLabel = ptvdi->item.pszText;
            if (sNewLabel.find(",") != string::npos || 
                                            sNewLabel.find("|") != string::npos)
            {
                LoadString(hInst, IDS_TV_FORBIDDENSYMBOLS, szMsg, 
                                                               MAX_LOADSTRING);
                MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);
                
                return FALSE; // reject and revert to the original label
            }
            //else
            //    TreeView_SetItem(hwndBrowser, &ptvdi->item);
        }

        // Notice that 1st level folders under Favotires have unique names.
        if (g_sOldLabel != sNewLabel && ptvdi->item.lParam == 1 
                                           /* Favorites' 1st level subfolder */)
        {
            // Halt adding new folder if already exists.
            if (IsFolderEnumerated("" /* favorites root*/, sNewLabel))
            {
                LoadString(hInst, IDS_TV_FOLDERALREADYEXISTS, szMsg, 
                                                                MAX_LOADSTRING);
                MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);

                return FALSE; // reject and revert to the original label
            }

            // Update item name in TV.
            TreeView_SetItem(hwndBrowser, &ptvdi->item);
            
            // Update name for 1st level parent folder in m_vecCarFavoritesMap 
            // map.
            for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).nItemLevel == 
                    1 /* ptvdi->item.lParam*/ && 
                        m_vecCarFavoritesMap.at(i).sParentFolder == g_sOldLabel)
                    m_vecCarFavoritesMap.at(i).sParentFolder = sNewLabel;
            }

            // Update name for 1st level folder in vecFavoritesFolders map.
            for (int i = 0; i < vecFavoritesFolders.size(); i++)
            {
                // Update the name excluding 2nd level folders that might have 
                // the same name, but different parent.
                if (vecFavoritesFolders.at(i).sParentFolder.length() == 0 
                    /* favorites root */ && vecFavoritesFolders.at(i).sFolder 
                                                                 == g_sOldLabel)
                    vecFavoritesFolders.at(i).sFolder = sNewLabel; 

                // Update also for all folders that have given folder as parent.
                if (vecFavoritesFolders.at(i).sParentFolder == g_sOldLabel)
                    vecFavoritesFolders.at(i).sParentFolder = sNewLabel; 
            }

            // Set flag to persist change to registry on application close.
            g_bUpdateRegistry = TRUE;
        }

        // Update 2nd level folder name taking into account that 2nd level 
        // folders under Favorites may have the same names under different 1st 
        // level folders.
        if (g_sOldLabel != sNewLabel && ptvdi->item.lParam == 2)
        {
            HTREEITEM htviParent;
            char szBuf[MAX_PATH + 1] = "";
            TVITEM tvi;
            string sParentFolder;

            // Get the name of parent folder.
            htviParent = TreeView_GetParent(hwndBrowser, ptvdi->item.hItem);	
            ZeroMemory( &tvi, sizeof(tvi));
            tvi.mask = TVIF_TEXT; 
            tvi.hItem = htviParent;
            tvi.pszText = szBuf;
            tvi.cchTextMax = MAX_PATH+1;
            TreeView_GetItem(hwndBrowser, &tvi);
            sParentFolder = tvi.pszText;

            // Halt adding new folder if already exists.
            if (IsFolderEnumerated(sParentFolder, sNewLabel))
            {
                LoadString(hInst, IDS_TV_FOLDERALREADYEXISTS, szMsg, 
                                                                MAX_LOADSTRING);
                MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);

                return FALSE; // reject and revert to the original label
            }

            // Update item name item in TV.
            TreeView_SetItem(hwndBrowser, &ptvdi->item);

            // Update car items in m_vecCarFavoritesMap.
            for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).nItemLevel == 
                    2 /* ptvdi->item.lParam*/ &&
                    m_vecCarFavoritesMap.at(i).sGrandparentFolder == 
                    sParentFolder && m_vecCarFavoritesMap.at(i).sParentFolder 
                                                                 == g_sOldLabel)
                    m_vecCarFavoritesMap.at(i).sParentFolder = sNewLabel;
            }

            // Update folder name in vecFavoritesFolders.
            for (int i = 0; i < vecFavoritesFolders.size(); i++)
            {
                if (vecFavoritesFolders.at(i).sParentFolder == sParentFolder 
                            && vecFavoritesFolders.at(i).sFolder == g_sOldLabel)
                    vecFavoritesFolders.at(i).sFolder = sNewLabel; 
            }
            
            // Set flag to persist change to registry on application close.
            g_bUpdateRegistry = TRUE;
        }
    }

    return TRUE; // processed
}

// Purpose: This implicitly calls SizeWindowContents so that relative window 
//          positions are updated. Albeit artificial, it works. TODO: find natural 
//          solution.

void UpdateLayout()
{
    RECT rcMain;

    GetWindowRect(g_hWnd, &rcMain);
    SetWindowPos(g_hWnd, 0, 0, 0, 
        rcMain.right - rcMain.left, rcMain.bottom - rcMain.top + 1, 
                                                                SWP_NOMOVE);
    SetWindowPos(g_hWnd, 0, 0, 0, 
        rcMain.right - rcMain.left, rcMain.bottom - rcMain.top, SWP_NOMOVE);
}

void UpdateLayout2(BOOL bAdjustNormalMode, BOOL bAdjustDesktopMode)			                               
{
    RECT rcMain;
    RECT rcMainClient;
    BOOL bDesktopExpanded = FALSE;

    // The iManualAdjustment value is adjusted manually e.g. "10" ("1", "5" DNW) 
    // to enable WV repait in scenario when CE takes  desktop, LV is active, 
    // TB's search activated.
    int iManualAdjustment;

    GetWindowRect(g_hWnd, &rcMain);
    {
        GetClientRect(g_hWnd, &rcMainClient);
        if (GetSystemMetrics(SM_CXSCREEN) == (rcMainClient.right - 
                                                             rcMainClient.left)) 
            bDesktopExpanded = TRUE;

        if (!bDesktopExpanded)
            iManualAdjustment = -1;
        else
            iManualAdjustment = 10;
        
        if ( (!bDesktopExpanded && bAdjustNormalMode) || 
                                       (bDesktopExpanded && bAdjustDesktopMode))
        {
            SetWindowPos(g_hWnd, 0, 0, 0, rcMain.right - rcMain.left, 
                   rcMain.bottom - rcMain.top -  iManualAdjustment, SWP_NOMOVE);
            SetWindowPos(g_hWnd, 0, 0, 0, 
            rcMain.right - rcMain.left, rcMain.bottom - rcMain.top, SWP_NOMOVE);
        }
    }
}

// Purpose: starts threads to download car data and favorites links. Notice 
// that calls are made on WM_ACTIVATE so that UI shows up almost immediately,
// and then threads add data under favorites. If these calls are made from 
// InitInstance the results are unstable, but mostly main window shows up
// considerably delayed downplaying the use of threads.
void MainWindow_ActivateHandler()
{
    UINT iLoadThreadID;
    UINT iFavoritesThreadID;
    HANDLE hLoadThread;
    HANDLE hFavoritesThread;

    TCHAR szBuffer[100];
    wsprintf(szBuffer, "MainWindow_ActivateHandler: %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);

    // Load all car data (except pictures) from all collections. "LoadFavorites" 
    // thread depends on data loaded with "LoadAllCarData". Event 
    // synchnonization is used to guarantee that "LoadFavorites" actually 
    // executed after "LoadAllCarData" is finished. It is presumed that TV is 
    // already evailable when "LoadFavorites" starts (TODO: check it).

    //TCHAR szBuffer[100];
    wsprintf(szBuffer, "LAC_ThreadProc (Launch): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    hLoadThread = (HANDLE)_beginthreadex(NULL, 0, 
                            &LoadAllCarData_ThreadProc, NULL, 0, &iLoadThreadID);
    wsprintf(szBuffer, "LAC_ThreadProc (Next Line): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    if (hLoadThread != NULL) CloseHandle(hLoadThread); // TODO: verify is it right to close 
                                                       // immediately after, exact meaning of 
                                                       // CloseHandle

    wsprintf(szBuffer, "LF_ThreadProc (Launch): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    hFavoritesThread = (HANDLE)_beginthreadex(NULL, 0, &LoadFavorites_ThreadProc, 
                                                    NULL, 0, &iFavoritesThreadID);
    wsprintf(szBuffer, "LF_ThreadProc (Next Line): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    if (hFavoritesThread != NULL) CloseHandle(hFavoritesThread); 
}

static UINT __stdcall LoadAllCarData_ThreadProc(LPVOID pParam)
{
    TCHAR szBuffer[100];

    wsprintf(szBuffer, "LAC_ThreadProc (Enter): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    LoadAllCarData(g_szCarFilesFolder);
    //App_LoadRegistrySettings(); // must be called after LoadAllCarData.
    //InitListViewItems(hwndMainLV);
    SetEvent(g_hLoadDataCompleteEvent); // not used for now
    wsprintf(szBuffer, "LAC_ThreadProc (Exit): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);

    // Prompt dialog to download collections if no collections in car folder. 
    // Notice that g_nCollectionsCount calculated in LoadAllCarData.
    if (g_nCollectionsCount == 0) 
        DialogBox(hInst, (LPCTSTR)IDD_LOAD_IMAGE_SETS, g_hWnd, 
                                                        (DLGPROC)LoadImageSets);
    
    // Helps to enable "+" next to Favorites in TV.
    //UpdateLayout();

    return TRUE;
}

// Must be called after LoadAllCarData_ThreadProc finishes with loading
// all car data.
static UINT __stdcall  LoadFavorites_ThreadProc(LPVOID pParam)
{
    TVITEM tvi;
    
    //Sleep(5000);
    while (WAIT_OBJECT_0 /* object is signaled */ != 
        WaitForSingleObject(g_hLoadDataCompleteEvent
                       /* initially event object is not signaled */, INFINITE));
    while (WAIT_OBJECT_0 /* object is signaled */ != 
        WaitForSingleObject(g_hTVAvailableEvent
                       /* initially event object is not signaled */, INFINITE)); 

    TCHAR szTest[100];
    wsprintf(szTest, "Total car collections: %d, Total cars: %d", 
                                  m_vecCarFilesMap.size(), m_vecCarData.size()); 
    SendMessage(g_hwndStatus, WM_SETTEXT, 0 /* not used */, (LPARAM)szTest);
    //MessageBox(0, szTest, g_szCarFilesFolder, 0); // 132

    TCHAR szBuffer[100];
    wsprintf(szBuffer, "LF_ThreadProc (After WO): %d", GetTickCount()); 
    if (g_bEnableDebug) App_Debug(szBuffer);
    App_LoadRegistrySettings();

    // Helps to enable "+" next to Favorites in TV.
    UpdateLayout();

    // Populate (all items) and show main LV in all scenarios excluding CAR file 
    // is double clicked or WV flag is set.
    if (!g_bShowStartupWelcomeView && !g_bCollectionFileDBClicked)
                TreeView_SelectItem(hwndBrowser, TreeView_GetRoot(hwndBrowser));

    // Set variable to handle correctly clicks on Favorites folder.
    g_bFavoritesLoaded = TRUE;

    // Expand Favorites folder if there are favorites or use grey icons (TB and TV).
    if (TreeView_GetChild(hwndBrowser, g_htviFavorites /* Favorites folder */))
    {
        TreeView_Expand(hwndBrowser, g_htviFavorites, TVE_EXPAND);
        // Scroll, if necessary, to ensure that root item is visible.
        TreeView_SelectSetFirstVisible(hwndBrowser, 
                                                 TreeView_GetRoot(hwndBrowser));
    }
    else // use grey icon if no favorites 
    {
        // Set grey Favorites icon for TB.
        SendMessage(g_hwndTB, TB_CHANGEBITMAP, ID_TOOLBAR_FAVORITES, 
            MAKELPARAM(12 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));
        
        // Set grey Favorites icon for TV.
        tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
        tvi.hItem = g_htviFavorites;
        tvi.iImage = 5; 
        tvi.iSelectedImage = 5;
        TreeView_SetItem(hwndBrowser, &tvi);

    }

    if (g_bCollectionFileDBClicked)
    {
        // Get TV item based on FFN.
        HTREEITEM hItem = GetTVItemFromFFN(g_szFullFileName);
        TreeView_SelectItem(hwndBrowser, hItem);
    }

    //DestroyWindow(g_hWndSplashScreen); // DNW between threads
    //PostMessage(g_hWndSplashScreen, WM_CLOSE, 0, 0);
    SendMessage(g_hWndSplashScreen, WM_CLOSE, 0, 0);
    ReleaseCapture();

    if (!g_bShowStartupWelcomeView && !g_bCollectionFileDBClicked)
    {
        if (g_bUseKagiPaymentProcessing)
        {
            // Enable crippling for the main LV.
            if (!g_bRegisteredCopy && m_RP.IsTrialUseExpired())
            {
                DialogBox(hInst, (LPCTSTR)IDD_REGISTER_BUY_PRODUCT, g_hWnd, 
                                           (DLGPROC)RegisterBuyProduct_WndProc);
            }
        }
    }

    return TRUE;
}

void TV_OnContextMenu(HWND hWnd)
{
    HTREEITEM hItemSelected;
    HTREEITEM hItemParent, hItemGrandparent;
    TVITEM tvi;
    char szBuf[MAX_PATH + 1] = "";
    HMENU hMenu,hSubMenu;
    POINT pt;
    int iMenuItem;
    string sFolder;
    string sCarLabel;
    TCHAR szMsg[MAX_LOADSTRING];
    string sParentFolder, sGrandparentFolder;
    BOOL bStandardFolder = FALSE;
    HTREEITEM hFirstChildItem;
    int nFolderLevel;

    hItemSelected = TreeView_GetSelection(hWnd);
   
    // Explicit buffer setting is essential, just setting tvi.mask and tvi.hItem 
    //may leave tvi not populated. 
    ZeroMemory(&tvi, sizeof(tvi) );
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
    tvi.hItem = hItemSelected;
    tvi.pszText = szBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hWnd, &tvi);

    sFolder = tvi.pszText; 

    if (Utility_IsFavoritesFolder(&tvi))
    {
        hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TV_CONTEXTMENU_FFOLDER));
        hSubMenu = GetSubMenu(hMenu, 0);

        // Modify default menu according to context.
        if (tvi.lParam == 0 /* Favorites' root */)
        {
            EnableMenuItem(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_RENAME, 
                                                                     MF_GRAYED);
        }
        else if (tvi.lParam == 1 /* Favorites' 1st subfolders */)
        {
            // Remove export option for 1st level folders.
            RemoveMenu(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_IMPORT, 
                                                                  MF_BYCOMMAND);
            ModifyMenu(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, 
                  MF_BYCOMMAND, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, "Delete");

            // Disable folder renaming for standard folders.
            if (tvi.iImage == 6 || tvi.iImage == 10 /* green */ || 
                      tvi.iImage == 12 /* pink */ || tvi.iImage == 14 /* red */) 
            {
                EnableMenuItem(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_RENAME, 
                                                                     MF_GRAYED);
                ModifyMenu(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, 
                  MF_BYCOMMAND, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, 
                                                              "Remove content");
            }
        }
        if (tvi.lParam == 2 /* Favorites' 1st subfolders */)
        {
            // Remove export option for 2nd level folders.
            RemoveMenu(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_IMPORT, 
                                                                  MF_BYCOMMAND);
            // Prohibit adding subfolders to 2nd level subfolders (2 levels of 
            // favorites subfolders in CE 1.02 by design) by disabling the item.
            EnableMenuItem(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_ADD, MF_GRAYED);
            ModifyMenu(hSubMenu, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, 
                  MF_BYCOMMAND, IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL, "Delete");
        }

        GetCursorPos(&pt);
        
        // Make the current window the foreground window so that this window
        // disapears when focus is lost (Q135788).
        SetForegroundWindow(hWnd);
    
        // Display menu at (x,y) and use TPM_RETURNCMD flag to return selection.
        iMenuItem = TrackPopupMenu(hSubMenu, 
            TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, 
                                                                    hWnd, NULL);
        switch (iMenuItem) 
        {
            case IDM_TV_CONTEXTMENU_FFOLDER_EXPORT:
            {
                if (tvi.lParam == 0 /* favorites root */)
                    OnExportFavorites(tvi.lParam /* folder level */, 
                                              NULL /* not used */, tvi.pszText);
                                                                   
                else if (tvi.lParam == 1 /* 1st level folder */)
                    OnExportFavorites(tvi.lParam /* folder level */, 
                                              NULL /* not used */, tvi.pszText);
                else if (tvi.lParam == 2 /* 2nd level folder */)
                {
                    nFolderLevel = tvi.lParam; // prevent override with parent
                    sFolder = tvi.pszText;				

                    // Get the name of the parent folder.
                    hItemParent = TreeView_GetParent(hWnd, tvi.hItem);
                    ZeroMemory(&tvi, sizeof(tvi) );
                    tvi.mask = TVIF_TEXT;
                    tvi.hItem = hItemParent;
                    tvi.pszText = szBuf;
                    tvi.cchTextMax = MAX_PATH+1;
                    TreeView_GetItem(hWnd, &tvi);
                    sParentFolder = tvi.pszText;

                    OnExportFavorites(nFolderLevel, sParentFolder.c_str(), 
                                                               sFolder.c_str());
                }
                break;
            }
            case IDM_TV_CONTEXTMENU_FFOLDER_IMPORT:
                OnImportFavorites();
                break;
            case IDM_TV_CONTEXTMENU_FFOLDER_ADD:
                DialogBoxParam(hInst, (LPCTSTR)IDD_ADDFOLDER, NULL,
                             (DLGPROC)AddFolder_WndProc, (LPARAM)hItemSelected);
                break;
            case IDM_TV_CONTEXTMENU_FFOLDER_RENAME:
                TreeView_EditLabel(hWnd, hItemSelected);
                break;
            case IDM_TV_CONTEXTMENU_FFOLDER_DELETEALL: // specific delete also
                // Prompt a message to confirm deletion of favorites.	
                if (tvi.lParam == 0 /* favorites root */)
                    LoadString(hInst, IDS_TV_DELETE_ALL_FAVORITES, szMsg, 
                                                                MAX_LOADSTRING);
                else if (tvi.lParam == 1 || 2 /* 1st or 2nd level folder */)
                {
                    if (tvi.iImage == 6 /* blue */ || 
                        tvi.iImage == 10 /* green */ || 
                        tvi.iImage == 12 /* pink */ || 
                                                     tvi.iImage == 14 /* red */) 
                        LoadString(hInst, IDS_TV_CLEAR_FAVORITES_FOLDER, 
                                                         szMsg, MAX_LOADSTRING);
                    else
                        LoadString(hInst, IDS_TV_DELETE_FAVORITES_FOLDER,  
                                                         szMsg, MAX_LOADSTRING);
                }
                if (MessageBox(0, szMsg, szMBTitle, 
                                     MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
                    break;

                // The deletion is confirmed, proceed to delete.
                if (tvi.lParam == 0 /* favorites root */)
                    TV_ClearAllFavorites();
                else if (tvi.lParam == 1 /* 1st level subfolder */)
                {
                    // Get bStandardFolder of the selected (1st level) folder.  
                    for (int i = 0; i < vecFavoritesFolders.size(); i++)
                    {
                        //if (vecFavoritesFolders.at(i).sFolder == sFolder) // can be other in the tree
                        if (vecFavoritesFolders.at(i).hTVItem == hItemSelected) //tvi.hItem)
                        {
                            bStandardFolder = 
                                            vecFavoritesFolders.at(i).bStandard;
                            break;
                        }
                    }

                    // Delete child car items if any from the map of car 
                    // favorites. Notice that with each deletion the size of
                    // the vector is decreased, and previous positions are 
                    // generally not valid, therefore the posiotion of next
                    // item to delete returned dynamically with function.
                    int i = Utility_IsVectorCarItemFound(sFolder);
                    while (i >= 0)
                    {
                        m_vecCarFavoritesMap.erase(m_vecCarFavoritesMap.begin() + i);
                        i = Utility_IsVectorCarItemFound(sFolder);
                    }

                    // Delete child 2nd level folders from the map of favorites' 
                    // folders if any.
                    for (int i = 0; i < vecFavoritesFolders.size(); i++)
                    {
                        if (vecFavoritesFolders.at(i).sParentFolder == sFolder)
                        {
                            tvi.hItem = vecFavoritesFolders.at(i).hTVItem;
                            TV_DeleteFavoritesFolder(&tvi);
                        }
                    }

                    // If standard folder, remove context under under the folder 
                    // (standard folders are permanent and must not be deleted), 
                    // or delete selected folder if not standard. 
                    if (bStandardFolder)
                    {
                        hFirstChildItem = TreeView_GetChild(hwndBrowser, 
                                                                 hItemSelected);
                        while (hFirstChildItem)
                        {
                            TreeView_DeleteItem(hwndBrowser, hFirstChildItem);
                            hFirstChildItem = TreeView_GetChild(hwndBrowser, 
                                                                 hItemSelected);		                 
                        }
                    }
                    else
                    {
                        // Delete folder itself.
                        for (int i = 0; i < vecFavoritesFolders.size(); i++)
                            //if (vecFavoritesFolders.at(i).sFolder == sFolder)
                            if (vecFavoritesFolders.at(i).hTVItem == 
                                                                  hItemSelected) 
                            // Supposed to be single delete (possible change of 
                            // index, which matters with multiple deletes, does 
                            // not matter).
                                vecFavoritesFolders.erase(
                                               vecFavoritesFolders.begin() + i);

                        // Finally, delete TV folder.
                        TreeView_DeleteItem(hwndBrowser, hItemSelected);
                    }
                }
                else if (tvi.lParam == 2 /* 2nd level subfolder */)
                {
                    TV_DeleteFavoritesFolder(&tvi);
                }

                // Set flag to persist change to registry on application close.
                g_bUpdateRegistry = TRUE;

                break;
        }
    }
    // A car item item, any level, under favorites is clicked.
    else if (Utility_IsCarItem(&tvi))
    {
        // Load TV context menu. Note that context menu items are actually 
        // top-level submenu items (see resource view).
        hMenu = LoadMenu(hInst,  MAKEINTRESOURCE(IDR_TV_CONTEXTMENU_FITEM));
                                   
        hSubMenu = GetSubMenu(hMenu, 0);

        GetCursorPos(&pt);
        
        // Make the current window the foreground window so that this window
        // disapears when focus is lost (Q135788).
        SetForegroundWindow(hWnd);
    
        // Display menu at (x,y) and use TPM_RETURNCMD flag to return 
        // selection.
        iMenuItem = TrackPopupMenu(hSubMenu, 
            TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, 
                                                                hWnd, NULL);
        // Handle menu selection.
        switch (iMenuItem) 
        {
        case IDM_TV_CONTEXTMENU_DELETE:
            LoadString(hInst, IDS_TV_DELETE_FAVORITESCARITEM, szMsg, 
                                                                MAX_LOADSTRING);
            if (MessageBox(0, szMsg, szMBTitle, 
                                     MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
                break;

            ZeroMemory(&tvi, sizeof(tvi));
            tvi.mask = TVIF_TEXT | TVIF_PARAM;
            tvi.hItem = hItemSelected;
            tvi.pszText = szBuf;
            tvi.cchTextMax = MAX_PATH+1;
            TreeView_GetItem(hWnd, &tvi);

            sCarLabel = szBuf;

            if (tvi.lParam == 0 /* under favorites root */)
            {
                for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
                {
                    if (m_vecCarFavoritesMap.at(i).nItemLevel == 0 &&
                              m_vecCarFavoritesMap.at(i).sCarTitle == sCarLabel) 
                    {
                        // Supposed to be single erase (possible change of 
                        // index, which matters with multiple deletes, does not 
                        // matter). Notice that just "i" does not work.
                        m_vecCarFavoritesMap.erase(
                                              m_vecCarFavoritesMap.begin() + i);
                        break;
                    }
                }
            }
            
            if (tvi.lParam == 1 /* under 1st level folder */)
            {
                // Get parent folder for the selected car item.
                hItemParent = TreeView_GetParent(hWnd, tvi.hItem);
                
                // Get the name of the parent folder.
                ZeroMemory(&tvi, sizeof(tvi) );
                tvi.mask = TVIF_TEXT;
                tvi.hItem = hItemParent;
                tvi.pszText = szBuf; 
                tvi.cchTextMax = MAX_PATH+1;
                TreeView_GetItem(hWnd, &tvi);

                sParentFolder = tvi.pszText;

                for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
                {
                    // Could be the same car items under different 1st level 
                    // folders, therefore be sure that parent folder is correct.
                    if (m_vecCarFavoritesMap.at(i).nItemLevel == 1 &&
                        m_vecCarFavoritesMap.at(i).sParentFolder == 
                        sParentFolder && 
                              m_vecCarFavoritesMap.at(i).sCarTitle == sCarLabel)
                    {
                        // Supposed to be single erase (possible change of 
                        // index, which matters with multiple deletes, does not 
                        // matter).
                        m_vecCarFavoritesMap.erase(
                                              m_vecCarFavoritesMap.begin() + i);
                        break;
                    }
                }
            }

            if (tvi.lParam == 2 /* under 2nd level folder */)
            {
                // Get parent folder for the selected car item.
                hItemParent = TreeView_GetParent(hWnd, tvi.hItem);
                
                // Get the name of the parent folder.
                ZeroMemory(&tvi, sizeof(tvi) );
                tvi.mask = TVIF_TEXT;
                tvi.hItem = hItemParent;
                tvi.pszText = szBuf;
                tvi.cchTextMax = MAX_PATH+1;
                TreeView_GetItem(hWnd, &tvi);
                sParentFolder = tvi.pszText;

                // Get grandparent folder for the selected car item.
                hItemGrandparent = TreeView_GetParent(hWnd, hItemParent);
                
                // Get the name of the grandparent folder.
                ZeroMemory(&tvi, sizeof(tvi) );
                tvi.mask = TVIF_TEXT;
                tvi.hItem = hItemGrandparent;
                tvi.pszText = szBuf;
                tvi.cchTextMax = MAX_PATH+1;
                TreeView_GetItem(hWnd, &tvi);
                sGrandparentFolder = tvi.pszText;

                for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
                {
                    // Could be the same car items under different 1st level 
                    // and 2nd level folders, therefore be sure that parent and 
                    //grandparent folder is correct.
                    if (m_vecCarFavoritesMap.at(i).nItemLevel == 2 &&
                        m_vecCarFavoritesMap.at(i).sParentFolder == 
                        sParentFolder && 
                        m_vecCarFavoritesMap.at(i).sGrandparentFolder == 
                        sGrandparentFolder && 
                              m_vecCarFavoritesMap.at(i).sCarTitle == sCarLabel)
                    {
                        // Supposed to be single erase (possible change of 
                        // index, which matters with multiple deletes, does not 
                        // matter).
                        m_vecCarFavoritesMap.erase(
                                              m_vecCarFavoritesMap.begin() + i);
                        break;
                    }
                }
            }

            // Delete the car item from UI of TV.
            TreeView_DeleteItem(hWnd, hItemSelected);

            // Set flag to persist change to registry on application close.
            g_bUpdateRegistry = TRUE;

            // When last Favorites item is deleted, update to grey related 
            // TB and TV icons.
            if (!TreeView_GetChild(hwndBrowser, 
                                    g_htviFavorites /* Favorites folder */))
            {
                // Set grey Favorites icon for TB.
                SendMessage(g_hwndTB, TB_CHANGEBITMAP, ID_TOOLBAR_FAVORITES, 
                    MAKELPARAM(12 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));
                
                // Set grey Favorites icon for TV.
                tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
                tvi.hItem = g_htviFavorites;
                tvi.iImage = 5; 
                tvi.iSelectedImage = 5;
                TreeView_SetItem(hwndBrowser, &tvi);
            }

            break;
        }
    }
}

int Utility_IsVectorCarItemFound(string sFolder)
{
    for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
    {
        if (m_vecCarFavoritesMap.at(i).nItemLevel == 1 && 
            m_vecCarFavoritesMap.at(i).sParentFolder /* parent for car item */ 
                                                                     == sFolder)
             return i;
    }

    return -1; 
}

 void LV_OnContextMenu(HWND hWnd)
 {
    HMENU hMenu,hSubMenu;
    POINT pt;
    int iMenuItem;
    TCHAR szTitle[100];
    //int i; 
    DWORD dwStyle; 
    TVITEM tvi;
    char szBuf[MAX_PATH + 1] = "";
    HTREEITEM hTVItemSelected;

    // Load main menu. Note that context menu items are actually top-level 
    // submenu items (see resource view).
    hMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_LV_CONTEXTMENU));
    hSubMenu = ::GetSubMenu(hMenu, 0);

    dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    if ((dwStyle & LVS_TYPEMASK) == LVS_ICON)
    {
        CheckMenuItem(hSubMenu, IDM_LV_CONTEXTMENU_REPORTVIEW, 
                                                   MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hSubMenu, IDM_LV_CONTEXTMENU_THUMBNAILSVIEW, 
                                                     MF_BYCOMMAND | MF_CHECKED);
        EnableMenuItem(hSubMenu, IDM_LV_CONTEXTMENU_EXPORT, 
                                                      MF_BYCOMMAND | MF_GRAYED);
    }
    else if ((dwStyle & LVS_TYPEMASK) == LVS_REPORT)
    {
        CheckMenuItem(hSubMenu, IDM_LV_CONTEXTMENU_REPORTVIEW, 
                                                     MF_BYCOMMAND | MF_CHECKED);
        CheckMenuItem(hSubMenu, IDM_LV_CONTEXTMENU_THUMBNAILSVIEW, 
                                                   MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Removed not used menu items for the context menu of the main LV.
    if (hWnd == hwndMainLV)
    {
        RemoveMenu(hSubMenu, IDM_LV_CONTEXTMENU_REPORTVIEW, MF_BYCOMMAND);
        RemoveMenu(hSubMenu, IDM_LV_CONTEXTMENU_THUMBNAILSVIEW, MF_BYCOMMAND);
        
        // Check if favorites folder is selected, and (if yes) remove "Add to
        // Favorites..." option as having no sense.
        hTVItemSelected = TreeView_GetSelection(hwndBrowser);
       
        // Figure what TV folder is selected.
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_IMAGE;
        tvi.hItem = hTVItemSelected;
        tvi.pszText = szBuf;
        tvi.cchTextMax = MAX_PATH+1;
        TreeView_GetItem(hwndBrowser, &tvi);

        // Remove menu item if favorites folder is selected.
        if (Utility_IsFavoritesFolder(&tvi))
            RemoveMenu(hSubMenu, IDM_LV_CONTEXTMENU_ADDTOFAVORITES, 
                                                                  MF_BYCOMMAND);
    }

    ::GetCursorPos(&pt);

    // Make the current window the foreground window so that this window
    // disapears when focus is lost (Q135788).
    ::SetForegroundWindow(hWnd);
    
    // Display menu at (x,y) and use TPM_RETURNCMD flag to return selection.
    iMenuItem = ::TrackPopupMenu(hSubMenu, 
        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd,
                                                                          NULL);
    // Shared switch for menu/submenu items (when submenu handled, iMenuItem
    // is ID of submenu item).
    switch (iMenuItem) 
    {
        case IDM_LV_CONTEXTMENU_REPORTVIEW: 
        {
            {
                // Update TB icon.
                SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                    ID_TOOLBAR_VIEW, 
                    MAKELPARAM(8 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));			
                
                dwStyle = GetWindowLong(hWnd, GWL_STYLE); // GWL_ID
                TCHAR szTest[100];
                wsprintf(szTest, "Last Error: %d, Style: %d", GetLastError(), dwStyle); 
                //MessageBox(0, szTest, "Test", 0);
                //MessageBox(0, "IDM_LV_CONTEXTMENU_REPORTVIEW", szTest, 0);

                ////if (dwStyle & LVS_REPORT) 
                //if((dwStyle & LVS_TYPEMASK) == LVS_REPORT) 
                //{
                //	//MessageBox(0, "1", "", 0);
                //	g_nCarFilesLayout = 0; // report view //TODO: DNW
                //	dwStyle &=~ LVS_REPORT; // remove style
                //	dwStyle |= LVS_ICON;    // add style 
                //	SetWindowLong(hWnd, GWL_STYLE, dwStyle);

                //	ListView_DeleteAllItems(hWnd);
                //	LoadPicturesAndStrings(hWnd);
                //}
                //else if (dwStyle & LVS_ICON)
                if ((dwStyle & LVS_TYPEMASK) == LVS_ICON) // OK
                //else if ((dwStyle & LVS_TYPEMASK) == LVS_ICON) // OK
                //if (dwStyle & LVS_ICON) // DNW
                {
                    //MessageBox(0, "2", "", 0);
                    g_nCarFilesLayout = 0; // report view //TODO: DNW
                    TCHAR szTitle[100];
                    GetImageSetTitle(g_szFullFileName, szTitle);
                    dwStyle &=~ LVS_ICON;  // remove style
                    dwStyle |= LVS_REPORT; // add style 
                    SetWindowLong(hWnd, GWL_STYLE, dwStyle);

                    ListView_DeleteAllItems(hWnd);
                    InitListViewColumns(hWnd);
                    InitListViewItems(hWnd, szTitle /* filter */, 
                    //InitListViewItems(hWnd, "Lada 2010" /* filter */, 
                                                   TRUE /* MSS file clicked */);
                }
            }
            break;
        }
        case IDM_LV_CONTEXTMENU_THUMBNAILSVIEW:
                dwStyle = GetWindowLong(hWnd, GWL_STYLE); 
                if((dwStyle & LVS_TYPEMASK) == LVS_REPORT) 
                {
                    // Update TB icon.
                    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                        ID_TOOLBAR_VIEW, 
                        MAKELPARAM(7 /* zero-based image in IL */, 
                                          0 /* zero-based image list index */));
                    
                    g_nCarFilesLayout = 1; // list view with images //TODO: DNW
                    dwStyle &=~ LVS_REPORT; // remove style
                    dwStyle |= LVS_ICON;    // add style 
                    SetWindowLong(hWnd, GWL_STYLE, dwStyle);

                    ListView_DeleteAllItems(hWnd);
                    LoadPicturesAndStrings(hWnd);
                }
            break;
        case IDM_LV_CONTEXTMENU_ADDTOFAVORITES:
            AddItemToFavorites(hWnd);
            break;
        case IDM_LV_CONTEXTMENU_EXPORT:
            OnExportListView(hWnd);
            break;
         default: 
            //...
            break;
    }

    // Destroy only after handling menu items in switch i.e. getting items 
    // names.
    ::DestroyMenu(hSubMenu);
    ::DestroyMenu(hMenu);
 }

 // Note: called from context menu.
void AddItemToFavorites(HWND hwndLV)
{
    short iItem;
    iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);

    if (iItem != -1 /* click out of image */)
    {
        g_iLVDroppedItem = iItem;
        //AddDroppedItem(hwndLV);
        AddDroppedItem(hwndLV, NULL);
    }
}

// Purpose: get OS type to properly set default folder for CAR files i.e.
//          "My Documents" for XP and "Documents" for "Vista" (plus Vista 
//          family) and supposedly future OS.
// Note:    copied from MSB 2.1.

int GetOSVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion)
//int GetOSVersion()
{
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;
    
    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if(!(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)))
    {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
            return -1;
    }

    dwMajorVersion = osvi.dwMajorVersion;
    dwMinorVersion = osvi.dwMinorVersion;

    switch (osvi.dwPlatformId)
    {
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:
        {
            // Future OS.
            if (osvi.dwMajorVersion > 6) return 6;
            // Future OS (Vista family).
            if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion > 1) return 6;
            // Microsoft Windows 7.
            if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) return 6;
            // Microsoft Windows Vista.
            if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) return 6;
            // Microsoft Windows Server 2003 family.
            if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) return 5;
            // Microsoft Windows XP.
            if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) return 5;
            // Microsoft Windows 2000.
            if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) return 5; 
            // All older relevant OS.
            if (osvi.dwMajorVersion <= 4) return 0;

            break;
        }

        // All older OS.
        default:
            return 0;
    }
    
    return -1; // should be never here
}

HTREEITEM GetTVItemFromFFN(LPCTSTR pszFFN)
{
    UINT i;
    string sCarFilesFolder = g_szCarFilesFolder;
    string sFFN; 
    string sThisFFN = pszFFN;

    for (i = 0; i < m_vecCarFilesMap.size(); i++)
    {
        sFFN = m_vecCarFilesMap.at(i).sFFN; 
        if (sFFN == sThisFFN)
        {
            return m_vecCarFilesMap.at(i).hItem;
        }
    }

    return 0; // item not found
}

void PopulateTreeView_Favorites(HWND hwndTV)
{
    TCHAR szCarTitle[200];
    TVITEM tvi;
    TVINSERTSTRUCT tvis; 

    for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
    {
        // Add item under Favorites in the tree view.
        if (g_htviFavorites)
        {
            lstrcpy(szCarTitle, m_vecCarFavoritesMap.at(i).sCarTitle.c_str());
            
            tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
            tvi.pszText = szCarTitle;
            tvi.iImage = 4;     
            tvi.iSelectedImage = 4;  

            tvis.hParent = g_htviFavorites;
            tvis.hInsertAfter = TVI_LAST; 
            tvis.item = tvi; 

            TreeView_InsertItem(hwndTV, &tvis);
            SetFocus(hwndTV);
        }
    }
}

// PURPOSE: used to ensure that vecFavoritesFolders has only unique members.
BOOL IsFolderEnumerated(string sParentFolder, string sFolder)
{
    int i;

    for (i = 0; i < vecFavoritesFolders.size(); i++)
    {
        if (vecFavoritesFolders.at(i).sParentFolder == sParentFolder &&
                                  vecFavoritesFolders.at(i).sFolder == sFolder) 
            return TRUE;
    }
    
    return FALSE;
}

// PURPOSE: ...
HTREEITEM GetFavoritesForder(string sParentFolder, string sFolder)
{
    int i;

    for (i = 0; i < vecFavoritesFolders.size(); i++)
    {
        if (vecFavoritesFolders.at(i).sParentFolder == sParentFolder &&
                                   vecFavoritesFolders.at(i).sFolder == sFolder)
            return vecFavoritesFolders.at(i).hTVItem;
    }
    
    return 0;
}

// Purpose: returns TRUE if string (suffix of MSS title) has " 2010" form
//           that is one space character followed by 4 digits.
// Note:    copied from MSB_STControl (MSB 2.1).
BOOL IsCarSuffix(LPCTSTR pszSuffix)
{
    BOOL bCond1 = FALSE;
    BOOL bCond2 = FALSE;
    BOOL bCond3 = FALSE;
    BOOL bCond4 = FALSE;
    BOOL bCond5 = FALSE;

    if (*pszSuffix == ' ') bCond1 = TRUE; pszSuffix++;
    if (isdigit(*pszSuffix)) bCond2 = TRUE; pszSuffix++; // or _istdigit 
    if (isdigit(*pszSuffix)) bCond3 = TRUE; pszSuffix++;
    if (isdigit(*pszSuffix)) bCond4 = TRUE; pszSuffix++;
    if (isdigit(*pszSuffix)) bCond5 = TRUE;

    return bCond1*bCond2*bCond3*bCond4*bCond5;
}

// PURPOSE: called from LIS to add newly downloaded CAR file.

void AddCarFile(LPCTSTR pszTitle, LPCTSTR pszFFN)
{
    TCHAR szFileName[MAX_PATH];
    TCHAR szBaseFileName[MAX_PATH]; // no extension
    TCHAR szExt[10]; // includes leading period (.)
    
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szCountry[100];
    TCHAR szGroup[100];
    CCarList m_CarFile;

    // m_vecCarData
    TCHAR szImagesCount[10];
    int nImagesCount;
    //TCHAR szFFN[MAX_PATH];
    CCarData m_CarData;
    TCHAR szPictureTitle[100]; 
    // Main.
 //   string sYear;
 //   string sMake;
    TCHAR szModel[100];
    TCHAR szSubModel[100];
    TCHAR szType[100];
    TCHAR szPrice[100];
    // Engine.
    TCHAR szPower[100];
    TCHAR szFuel[100];
    // Sizes and weight.
    TCHAR szLength[100];
    TCHAR szWidth[100];
    TCHAR szHeight[100];
    TCHAR szClearance[100];
    TCHAR szWeight[100];
    // Misc.
    TCHAR szTankVolume[100];
    int nIndex;

    string sTitle;
    string sMake;
    string sYear;
    string sSuffix;

    HTREEITEM hItem;
    TVITEM tvi;
    TVINSERTSTRUCT tvis; 
    HTREEITEM hInsertedItem;
    HTREEITEM hRootItem;
    HTREEITEM hFirstChildItem;
    TVSORTCB tvsort;

    // Get file name from FFN.
    _splitpath(pszFFN, NULL, NULL, szBaseFileName, szExt);
    wsprintf(szFileName, "%s%s", szBaseFileName, szExt); 

    // Get XML data from CAR file for m_vecCarFilesMap (TV).
    GetCarXMLData(pszFFN, 1000 /* year */, szYear); 
    GetCarXMLData(pszFFN, 1100 /* make */, szMake);
    GetCarXMLData(pszFFN, 2300 /* country */, szCountry); 
    
    // If XML data are not available try get Make and Year from the title 
    // string and use default string instead of Country.
    if (lstrlen(szMake) == 0 /* internal XML data were found */) 
    {
        sTitle = pszTitle;
        sSuffix = sTitle.substr(sTitle.size() - 5, 5);

        if (IsCarSuffix(sSuffix.c_str()))
        {
            sMake = sTitle.substr(0, sTitle.size() - 5);
            lstrcpy(szMake, sMake.c_str());
        }
    }

    if (lstrlen(szYear) == 0 /* internal XML data were found */) 
    {
        sTitle = pszTitle;
        sSuffix = sTitle.substr(sTitle.size() - 5, 5);

        if (IsCarSuffix(sSuffix.c_str()))
        {
            sYear = sTitle.substr(sTitle.size() - 4, 4);
            lstrcpy(szYear, sYear.c_str());
        }
    }

    if (lstrlen(szCountry) == 0) lstrcpy(szCountry, UNCATEGORIZED_FOLDER_NAME);
    
    // Store data in the m_vecCarFilesMap.
    m_CarFile.sTitle = pszTitle; 
    m_CarFile.sFFN = pszFFN;
    m_CarFile.sYear = szYear; 
    m_CarFile.sMake = szMake; 
    m_CarFile.sCountry = szCountry; 
    m_vecCarFilesMap.push_back(m_CarFile);

    // Get XML data from CAR file for m_vecCarData (all car data except images).
    GetCarXMLData(pszFFN, 10 /* images count */, szImagesCount);
    nImagesCount = atoi(szImagesCount);
    nIndex = m_vecCarData.size(); // points to next element since zero-based

    for (int i = 0; i < nImagesCount; i++)
    {
        GetCarXMLData(pszFFN, 201 + i /* picture title */, szPictureTitle);
        GetCarXMLData(pszFFN, 1201 + i /* model */, szModel);
        GetCarXMLData(pszFFN, 1301 + i /* submodel */, szSubModel);
        GetCarXMLData(pszFFN, 1401 + i /* type */, szType);
        GetCarXMLData(pszFFN, 1501 + i /* price */, szPrice);
        GetCarXMLData(pszFFN, 1601 + i /* power */, szPower);
        GetCarXMLData(pszFFN, 1701 + i /* fuel */, szFuel);
        GetCarXMLData(pszFFN, 1801 + i /* length */, szLength);
        GetCarXMLData(pszFFN, 1901 + i /* width */, szWidth);
        GetCarXMLData(pszFFN, 2001 + i /* height */, szHeight);
        GetCarXMLData(pszFFN, 2201 + i /* clearance */, szClearance);
        GetCarXMLData(pszFFN, 2101 + i /* weight */, szWeight);
        GetCarXMLData(pszFFN, 2401 + i /* tank volume */, szTankVolume);

        // Add car details to the vector. 
        m_CarData.sTitle = pszTitle; 
        m_CarData.sFileName = szFileName; // TODO: figure what to use
        //m_CarData.sFFN = ...;
        m_CarData.sYear = szYear; 
        m_CarData.sMake = szMake; 
        m_CarData.sModel = szModel; 
        m_CarData.sSubModel = szSubModel; 
        m_CarData.sType = szType; 
        m_CarData.sPrice = szPrice; 
        m_CarData.sPower = szPower; 
        m_CarData.sFuel = szFuel;
        m_CarData.sLength = szLength;
        m_CarData.sWidth = szWidth;
        m_CarData.sHeight = szHeight;
        m_CarData.sClearance = szClearance;
        m_CarData.sWeight = szWeight;
        m_CarData.sTankVolume = szTankVolume;
        m_CarData.sCountry = szCountry;
        m_CarData.nImageID = i; // relative order of image in collection
        m_CarData.nIndex = nIndex; // global index of images among all files 

        m_vecCarData.push_back(m_CarData);
        nIndex++;
    }

    // Add TV folder (if not available) and item (the CAR's title) taking in 
    // account the currently used group.
    switch (g_nGroup)
    {
    case 0: // all files, not group
        // do nothing
        break;
    case 1: // year
        lstrcpy(szGroup, szYear);
        break;
    case 2: // make
        lstrcpy(szGroup, szMake);
        break;
    case 3: // country
        lstrcpy(szGroup, szCountry);
        break;
    }

    hRootItem = TreeView_GetRoot(hwndBrowser);

    // Check if folder is already inserted.
    if (g_nGroup == 0 /* All */)
        hItem = hRootItem; // "All" item is always inserted
    else
    {
        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
        tvi.pszText = szGroup; 
        tvi.iImage  = 0;         // folder
        tvi.iSelectedImage = 1;  // open folder
        if (g_nGroup == 1 /* Year */) 
            tvi.lParam = atoi(szGroup); // ordering

        tvis.hParent = hRootItem; // <--
        tvis.hInsertAfter = TVI_LAST; 
        tvis.item = tvi;

        hItem = IsInserted(hwndBrowser, szGroup); 
    }
    
    // Insert folder if not yet inserted.
    if (!hItem /* not yet inserted */) hItem = TreeView_InsertItem(hwndBrowser, &tvis);

    // Inseat item under folder.
    //lstrcpy(szText, pszTitle); //m_vecCarFilesMap.at(i).sTitle.c_str()); 
    
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | 
                                                            TVIF_PARAM; 
    tvi.pszText = (LPSTR)pszTitle; 
    tvi.iImage  = 2;         // file image
    tvi.iSelectedImage = 2;  // same file image

    tvis.hParent = hItem;
    tvis.hInsertAfter = TVI_LAST; 
    tvis.item = tvi; 
    
    hInsertedItem = TreeView_InsertItem(hwndBrowser, &tvis);
    m_vecCarFilesMap.at(m_vecCarFilesMap.size() - 1 /* zero-based */).hItem = 
                                                                 hInsertedItem; 
    // Sort newly inserted folder.
    if (g_nGroup != 1 /* group is not year*/)
            TreeView_SortChildren(hwndBrowser, hRootItem, FALSE);
    else if (g_nGroup == 1 /* sort by year */)
    {
        tvsort.hParent = hRootItem;
        tvsort.lpfnCompare = TV_CompareProc;			
        TreeView_SortChildrenCB(hwndBrowser, &tvsort, 0 /* must be zero */);
    }

    // Expand "All" (makes sense for the first CAR in TV).
    TreeView_Expand(hwndBrowser, hRootItem, TVE_EXPAND);

    // Get TV item based on FFN.
    hItem = GetTVItemFromFFN(pszFFN);
    TreeView_SelectItem(hwndBrowser, hItem);
}

LRESULT LV_Notify_OnDoubleClick(HWND hwndLV)
{
    int iItem;
    int nGlobalCarIndex;
    LVITEM lvi;
    TCHAR szImageSetFFN[MAX_PATH];
    TCHAR szPictureFFN[MAX_PATH];
    TCHAR szAppDataDir[MAX_PATH];
    TCHAR szCEDataDir[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
    int nCollectionImageID;
    HINSTANCE hResInst;
    int iImageID; 
    int iImageResourceStartID = 100;
    TCHAR szYear[10], szMake[50], szModel[50]; //szCarTitle[100];
    TCHAR szBuf[50];

    iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);

    // Make silent return when click occurs out of content.
    if (iItem == -1 /* click out of item */)
        return FALSE;
    
    // Run "Windows Picture and Fax Viewer" on XP and higher (it is unavailable 
    // on Windows 2000 or older).
    if (g_dwMajorVersion >= 6 || (g_dwMajorVersion == 5 && 
                           g_dwMinorVersion > 0) /* higher than Windows 2000 */)
    {
        // Get global index in CCarData, which coincides with lParam.
        lvi.iItem = iItem;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(hwndLV, &lvi);
        nGlobalCarIndex = lvi.lParam;

        TCHAR szTest[100];
        wsprintf(szTest, "%d %d", iItem, nGlobalCarIndex);
        //MessageBox(0, szTest, "Test", 0); // N, zero

        // Retrive from environment application data directory and set FFN for 
        // temporary image.
        ExpandEnvironmentStrings("%appdata%", szAppDataDir, 
                                                          sizeof(szAppDataDir));
        wsprintf(szCEDataDir, "%s%s", szAppDataDir, "\\Car Explorer");
        wsprintf(szPictureFFN, "%s%s", szCEDataDir, "\\Temp.jpg");
        
        // Create application data directory for CE. The temporary image will
        // will be created there. Notice that if it exists already 
        // CreatedDirectory does nothing. TODO: probably remove since always
        // created in WinMain.
        CreateDirectory(szCEDataDir, /* security */ NULL); 

        if (g_nRightPaneViewType != 3 /* DC on CAR file (out) */)
        {
            // Get collection its FFN and collection image ID based on its 
            // global ID (index).
            GetImageSetFFNFromGlobalImageID(nGlobalCarIndex /* in */, 
                         szImageSetFFN /* out */, nCollectionImageID /* out */); 
            //GetImageSetTitle(szImageSetFFN, szTitle);
        }
        else
        {
            lstrcpy(szImageSetFFN, g_szFullFileName);
        }

        // Load CAR file and save clicked imaged as Temp.jpg, pointed by 
        // szPictureFFN.
        hResInst = LoadLibrary(szImageSetFFN /* <filename>.car with path */);
        if (hResInst == NULL)
        {
            wsprintf(szBuf, "%d", GetLastError()); 
            MessageBox(NULL, szBuf, "LoadLibrary failed!", MB_OK);
                                                                         
            return FALSE;
        }

        // Load picture with given index.
        iImageID = iImageResourceStartID + nCollectionImageID + 1; 
        // iImageID: 101 ... 105 ...
        m_SinglePicture.Load(hResInst, "IMAGES", iImageID);

        // Save image temporary in the %appdata%, in the Car Explorer directory.
        SaveToImageFile(szPictureFFN, 999 /* LL any, DNM */);

        // Run "Windows Picture and Fax Viewer".
        wsprintf(szCmdLine, "%s %s", 
                 "rundll32.exe shimgvw.dll,ImageView_Fullscreen", szPictureFFN);
        WinExec(szCmdLine, TRUE);
        
        // Update WPFV title to the car title instead of default value, for 
        // example with XP: "Temp.jpg - Windows Picture and Fax Viewer".

        // Make the string for the car title.
        ListView_GetItemText(hwndLV, iItem, 1 /* make */, szYear, 
                                                                sizeof(szYear));
        ListView_GetItemText(hwndLV, iItem, 2 /* make */, szMake, 
                                                                sizeof(szMake));
        ListView_GetItemText(hwndLV, iItem, 3 /* model */, szModel, 
                                                               sizeof(szModel));
        if (lstrlen(szModel) == 0)
        {
            ListView_GetItemText(hwndLV, iItem, 0 /* CAR file title */,
                                            g_szCarTitle, sizeof(g_szCarTitle));
        }
        else
            // Use CAR file title as single picture title if no relevant XML 
            // data.
            wsprintf(g_szCarTitle, "%s %s %s", szYear, szMake, szModel); 

        // Run timer to have a guarantee that title is updated.
        g_nTimerCallsCount = 0;
        g_nTimerID = SetTimer(NULL /* windowless */, NULL /* ignored */, 
                                              10 /* milliseconds */, TimerProc);

        // Delete exctacted and saved picture after viewing.
        // TODO: makes WPFV show nothing. Find more elaborate way to delete 
        // after use (like with thread).
        //DeleteFile(szPictureFFN); 
    }
    else
    {
       MessageBox(0, "This feature is available on Windows XP or higher.", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
       return FALSE;
    }

    return TRUE;
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    HWND hwndWPFV;

    g_nTimerCallsCount++;
    if (g_nTimerCallsCount > 50) KillTimer(NULL /* windowless */, g_nTimerID);

    // TODO: more tests with different OS.
    if (g_nOSVersion == 6 /* Vista family and supposedly future OS */)
    {
        // Windows 7.
        hwndWPFV = FindWindow(NULL, "Temp.jpg - Windows Photo Viewer");

  //      if (hwndWPFV) 
        //   MessageBox(0, "Found!!", "Windows Photo Viewer", 0); //OK
        //else
        //   MessageBox(0, "NOT found((((((", "Windows Photo Viewer", 0);

    }
    if (g_dwMajorVersion == 6 && g_dwMinorVersion == 0 /* Vista */)
    {
        // Windows Vista.
        hwndWPFV = FindWindow(NULL, "Temp.jpg - Windows Photo Gallery");
    }
    else if (g_dwMajorVersion == 5 && g_dwMinorVersion == 1 /* Windows XP */)
    {
        //hwndWPFV = FindWindow("ShImgVw:CPreviewWnd", NULL); // LL unstable
        // Windows XP.
        hwndWPFV = 
                  FindWindow(NULL, "Temp.jpg - Windows Picture and Fax Viewer");
    }
        
    //hwndWPFV = FindWindow("ShImgVw:CPreviewWnd", NULL); // LL unstable	
    //hwndWPFV = FindWindow(NULL, "Temp.jpg - Windows Picture and Fax Viewer");

    if (hwndWPFV)
    {
        //SetWindowText(hwndWPFV, g_szCarTitle);
        // MSDN: "To set the text of a control in another process, send the 
        // WM_SETTEXT message directly instead of calling SetWindowText." 
        SendMessage(hwndWPFV, WM_SETTEXT, 0 /* not used */, 
                                                          (LPARAM)g_szCarTitle);

        // MSDN: "If the application calls SetTimer with hWnd set to NULL, 
        // this [2nd] parameter must be the timer identifier returned by 
        // SetTimer."
        //KillTimer(NULL /* windowless */, g_nTimerID);
    }
}

BOOL IsFavoritesSelected()
{
    HTREEITEM hItemSelected;
    TVITEM tvi;
    char szBuf[MAX_PATH+1] = "";
    HMENU hMenu,hSubMenu;
    POINT pt;
    int iMenuItem;
   
    hItemSelected = TreeView_GetSelection(hwndBrowser);


    // Explicit buffer setting is essential, just setting tvi.mask and tvi.hItem 
    // may leave tvi not populated. 
    ZeroMemory(&tvi, sizeof(tvi) );
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hItemSelected;
    tvi.pszText = szBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hwndBrowser, &tvi);

    if (lstrcmp(szBuf, "Favorites") == 0) // favorites folder is selected
        return TRUE;
    else
        return FALSE;
}

void WV_CB_OnChangeSelection()
{
    TCHAR szMinPrice[20]; // low boundary
    TCHAR szMaxPrice[20]; // high boundary
    int nPriceRange;

    // Destroy AnimUI visuals (all its windows are destroyed).
    DLL_DestroyCustomUI();

    // Get string for current year selection.
    nPriceRange = SendMessage(g_hwndWelcome_CBPriceRange, 
                      CB_GETCURSEL, 0 /* must be zero */, 0 /* must be zero */);

    switch (nPriceRange)
    {
    case 0: // all
        //lstrcpy(szMinPrice, "$0"); // zero min
        //lstrcpy(szMaxPrice, "$1,000,000,000"); // artificial big number (what if $ inflated...)
        lstrcpy(szMinPrice, "0"); // zero min
        lstrcpy(szMaxPrice, "1000000000"); // artificial big number (what if $ inflated...)
        break;
    case 1: // custom
        lstrcpy(szMinPrice, "0"); // zero min
        lstrcpy(szMaxPrice, "0"); 
        break;
    case 2: // < $5,000
        lstrcpy(szMinPrice, "$0"); // zero min
        lstrcpy(szMaxPrice, "$5,000");
        break;
    case 3: // $5,000 ... $10,000
        lstrcpy(szMinPrice, "$5,000");
        lstrcpy(szMaxPrice, "$10,000");
        break;
    case 4: // $10,000 ... $15,000
        lstrcpy(szMinPrice, "$10,000");
        lstrcpy(szMaxPrice, "$15,000");
        break;
    case 5: // $15,000 ... $20,000
        lstrcpy(szMinPrice, "$15,000");
        lstrcpy(szMaxPrice, "$20,000");
        break;
    case 6: // $20,000 ... $25,000
        lstrcpy(szMinPrice, "$20,000");
        lstrcpy(szMaxPrice, "$25,000");
        break;
    case 7: // $25,000 ... $30,000
        lstrcpy(szMinPrice, "$25,000");
        lstrcpy(szMaxPrice, "$30,000");
        break;
    case 8: // $30,000 ... $40,000
        lstrcpy(szMinPrice, "$30,000");
        lstrcpy(szMaxPrice, "$40,000");
        break;
    case 9: // $40,000 ... $50,000
        lstrcpy(szMinPrice, "$40,000");
        lstrcpy(szMaxPrice, "$50,000");
        break;
    case 10: // $50,000 ... $100,000
        lstrcpy(szMinPrice, "$50,000");
        lstrcpy(szMaxPrice, "$100,000");
        break;
    case 11: // > $100,000
        lstrcpy(szMinPrice, "$100,000");
        lstrcpy(szMaxPrice, "$1,000,000,000"); // artificial big number (what if $ inflated...)
        break;
    }

    // Set limits always even if edit made hidden (used when reading settings).
    SendMessage(g_hwndEdit_MinPrice, WM_SETTEXT, 0 /* not used */, 
                                                             (LPARAM)szMinPrice);
    SendMessage(g_hwndEdit_MaxPrice, WM_SETTEXT, 0, (LPARAM)szMaxPrice);
    
    if (nPriceRange == 0 /* All */)
    {
        ShowWindow(g_hwndStatic_MinPrice, FALSE);
        ShowWindow(g_hwndEdit_MinPrice, FALSE); 
        ShowWindow(g_hwndStatic_MaxPrice, FALSE);
        ShowWindow(g_hwndEdit_MaxPrice, FALSE);
    }
    else if (nPriceRange == 11 /* > $100,000 */)
    {
        ShowWindow(g_hwndStatic_MinPrice, TRUE);
        ShowWindow(g_hwndEdit_MinPrice, TRUE); 
        ShowWindow(g_hwndStatic_MaxPrice, FALSE);
        ShowWindow(g_hwndEdit_MaxPrice, FALSE);
    }
    else
    {
        ShowWindow(g_hwndStatic_MinPrice, TRUE);
        ShowWindow(g_hwndEdit_MinPrice, TRUE); 
        ShowWindow(g_hwndStatic_MaxPrice, TRUE);
        ShowWindow(g_hwndEdit_MaxPrice, TRUE);
    }

 //   // Clear remote listbox before repopulation.
    //SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
    //    IDC_LIS_LISTBOX1), LB_RESETCONTENT, 0 /* must be zero */, 
    //					                                  0 /* must be zero */); 
}

BOOL WV_DrawHeaderPictureAndInnerBackground(HWND hwnd, HDC hdc)
{
    RECT rc;
    long hmWidth,hmHeight; // HIMETRIC units
    HDC	hdcIC;
    int iWindowWidth, iWindowHeight;
    int iLeftIndent = 30; 
    int	iTopIndent = 15; 
    int	iImageWidth_Pixels, iImageHeight_Pixels;
    RECT rcHeader;
    COLORREF rgbBackgroundColor = RGB(28, 169, 0); 
    RECT rcCanvas;

    GetClientRect(hwnd, &rc);

    // Calculate the value of helper variable g_nWV_InnerAreaHeight used to
    // keep constant the height of inner area while vertical re-sizing.
    if (g_bWV_FirstPaint /* used to get initial height of inner area */)
    {
        g_bWV_FirstPaint = FALSE;
        // It is actually a constant (can be #define), actually this line
        // let get it once.
        g_nWV_InnerAreaHeight =  rc.bottom - rc.top -  2*g_nWV_VOffset - 
                                         WV_HEADER_HEIGHT - WV_SEPARATOR_HEIGHT;
    }

    // Paint header's background (except separator).
    InflateRect(&rc, -g_nWV_HOffset, -g_nWV_VOffset);
    iLeftIndent = iLeftIndent + g_nWV_HOffset; 
    iTopIndent = iTopIndent + g_nWV_VOffset;
    rcHeader = rc; rcHeader.bottom = WV_HEADER_HEIGHT + g_nWV_VOffset;
    // Special case when right border is moved essentially: return to original.
    if (g_bWV_NarrowWidth) rcHeader.right = rcHeader.right + g_nWV_HOffset; 
    FillRect(hdc, &rcHeader, CreateSolidBrush(rgbBackgroundColor));

    // Paint inner area's background that hosts controls.
    rcCanvas = rc; rcCanvas.top = rcHeader.bottom + 
                                  WV_SEPARATOR_HEIGHT /* height of separator */;
    rcCanvas.bottom = rcCanvas.top + g_nWV_InnerAreaHeight;
    if (g_bWV_NarrowWidth) rcCanvas.right = rcCanvas.right + 
                                                    g_nWV_HOffset; // like above
    FillRect(hdc, &rcCanvas, CreateSolidBrush(g_clrWV_InnerBackground));

    // Paint picture on the header.

    // Get picture size: IPicture is using metric units (HIMETRIC units) 
    // (1 himetric unit = 0.01 mm).
    m_SinglePicture.m_spIPicture->get_Width(&hmWidth);  
    m_SinglePicture.m_spIPicture->get_Height(&hmHeight);

    // Convert HIMETRIC to pixel units (LOGPIXELSX: number of pixels per logical 
    // inch along the screen width).
    hdcIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    iImageWidth_Pixels  = (hmWidth  * GetDeviceCaps(hdcIC, LOGPIXELSX)) / 2540;
    iImageHeight_Pixels = (hmHeight * GetDeviceCaps(hdcIC, LOGPIXELSY)) / 2540;
    DeleteDC(hdcIC);

    // Calculate scaling and position of the picture, which is centered, 
    // indented, scaled.
    iWindowWidth = rc.right-rc.left; 
    iWindowHeight = rc.bottom-rc.top;

    // Now draw picture (Render means draw).
    m_SinglePicture.m_spIPicture->Render(hdc, iLeftIndent, iTopIndent, 
        iImageWidth_Pixels, iImageHeight_Pixels, 0, hmHeight, hmWidth, 
                                                               -hmHeight, NULL);
    return TRUE;
}

void WV_OnPaint(HWND hWnd, LPPAINTSTRUCT pps)
{
    HDC hdc; 
    RECT rc;
    HDC hdcBuffer;
    HBITMAP hbmBuffer, hbmOld;
    HBITMAP hbmp;
    HBRUSH  hBackgroundBrush;
    HBRUSH hBrush, hbrushOld;

    // Step I.
    GetClientRect(hWnd, &rc); 
    hdc = pps->hdc;
    hdcBuffer = CreateCompatibleDC(hdc); // 1x1 pixel monochrome bitmap is also created
    hbmBuffer = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);
    hbmOld = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

    hBackgroundBrush = CreateSolidBrush(g_clrWV_OuterBackground);
    FillRect(hdcBuffer, &rc, hBackgroundBrush);
    DeleteObject(hBackgroundBrush);

    // Step II. Draw on buffer bitmap.
    WV_DrawHeaderPictureAndInnerBackground(hWnd, hdcBuffer);

    hbmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SEPARATOR_PATTERN));
    hBrush = CreatePatternBrush(hbmp);
    hbrushOld = (HBRUSH)SelectObject(hdcBuffer, hBrush);

    if (!g_bWV_NarrowWidth) 
        PatBlt(hdcBuffer, g_nWV_HOffset, WV_HEADER_HEIGHT + g_nWV_VOffset, 
                      rc.right - 2*g_nWV_HOffset, WV_SEPARATOR_HEIGHT, PATCOPY);
    else
        PatBlt(hdcBuffer, g_nWV_HOffset, WV_HEADER_HEIGHT + g_nWV_VOffset, 
                        rc.right - g_nWV_HOffset, WV_SEPARATOR_HEIGHT, PATCOPY);
    //SelectObject(hdc, hbrushOld);
    DeleteObject(hBrush);

    // Step III. Assosiate buffer bitmap with original DC and restore original DC.
    BitBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
                                                      hdcBuffer, 0, 0, SRCCOPY);
    SelectObject(hdcBuffer, hbmOld);
    //SelectObject(hdcBuffer, hbrushOld);

    // Step IV.
    //ReleaseDC(hWnd, hdc); // used for DC created with GetDC (TODO: destroyed with EndPaint, by caller?)
    DeleteObject(hbmBuffer);
    DeleteDC(hdcBuffer);
}

void LV_SortItems(HWND hwndLV, int nSortColumn)
{
    int nColumn;

    // Used for ordering with no click (startup).
    if (g_iSortColumn_Prev == -1 /* none */)
        g_bSortAscending = TRUE;

    // Main list view (folders), which is always available (visible or 
    // not), and car list view (single collection) are essentialy the 
    // same. One cannot pass list view handler to compare function 
    // since the default ListView_CompareProc parameters are fixed and
    // ListView_CompareProc obtains internally proper handle for 
    // currently visible LV.
    ListView_SortItems(hwndLV, ListView_CompareProc, (LPARAM)nSortColumn);
    LVCOLUMN lvc;

    if (g_iSortColumn_Prev != -1 /* unsorted, startup */)
    {
        if (g_bSortAscending)
        {
            //  Set up sort image.
            lvc.mask = LVCF_FMT | LVCF_IMAGE;
            lvc.fmt = LVCFMT_CENTER | LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
            lvc.iImage = 0;

            ListView_SetColumn(hwndLV, nSortColumn, &lvc );
        }
        else
        {
            //  Set down sort image.
            lvc.mask = LVCF_FMT|LVCF_IMAGE;
            lvc.fmt = LVCFMT_CENTER | LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
            lvc.iImage = 1;

            ListView_SetColumn(hwndLV, nSortColumn, &lvc );
        }
    }
    else
    {
        if (g_bSortAscending)
        {
            //  Set up sort image.
            lvc.mask = LVCF_FMT | LVCF_IMAGE;
            lvc.fmt = LVCFMT_CENTER | LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
            lvc.iImage = 0;

            ListView_SetColumn(hwndLV, nSortColumn, &lvc );
        }
        else // TODO: this "else" LL redundant since if g_iSortColumn_Prev is -1
             // g_bSortAscending is TRUE always
        {
            //  Set down sort image.
            lvc.mask = LVCF_FMT|LVCF_IMAGE;
            lvc.fmt = LVCFMT_CENTER | LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;
            lvc.iImage = 1;

            ListView_SetColumn(hwndLV, nSortColumn, &lvc );
        }
    }
    
    // Now we drop column image for all other columns.
    lvc.mask = LVCF_FMT;
    lvc.fmt  = LVCFMT_CENTER;

    for (nColumn = 0; nColumn < 16; nColumn++)
    {
        if (nColumn != g_nSortColumn)
        {
            ListView_SetColumn(hwndLV, nColumn, &lvc );
        }
    }
}

void  RunSplashScreen()
{
    TCHAR szProgramFilesDir[MAX_PATH];
    TCHAR szSplashScreenFFN[MAX_PATH];
    
    // Load splash screen from JPG. Notice that "Splash_screen.jpg" is installed
    // intoi the root of install directory.
    ExpandEnvironmentStrings("%programfiles%", szProgramFilesDir, 
                                                     sizeof(szProgramFilesDir));

    wsprintf(szSplashScreenFFN, "%s%s", szProgramFilesDir, 
                                           "\\Car Explorer\\Splash_screen.jpg");
    m_SinglePicture.Load(szSplashScreenFFN);
    //m_SinglePicture.Load("Splash_screen.jpg");

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)SplashScreen_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= TEXT("SplashScreen");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);

    g_hWndSplashScreen = CreateWindowEx(WS_EX_TOPMOST, TEXT("SplashScreen"), "", 
                   WS_POPUP, 0, 0, 320, 470, ::GetDesktopWindow(), 0, hInst, 0);
    // Application window is already centered at WndProc's WM_CREATE relative to 
    // the desktop, center splash screen now before showing it.
    CenterWindow2(g_hWndSplashScreen, 
                                  GetDesktopWindow() /* relative to desktop */);
    SetCapture(g_hWndSplashScreen);

    ::ShowWindow(g_hWndSplashScreen, TRUE);
    ::UpdateWindow(g_hWndSplashScreen);
}

LRESULT APIENTRY SplashScreen_WndProc(HWND hWnd, UINT Message, 
                                                    WPARAM wParam, LONG lParam)
{
    switch (Message)
    {
    case WM_CREATE:
        break;
    case WM_PAINT:
        OnPaint_Handler(hWnd); 
        //break;
        return 1;
    case WM_CHAR:
        break;
    case WM_ERASEBKGND:
        //break;
        return 1;
    case WM_LBUTTONDOWN:
        ::PostMessage(hWnd, WM_NCLBUTTONDOWN,HTCAPTION,
                                    MAKELPARAM(LOWORD(lParam), HIWORD(lParam)));
        break;
    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    return DefWindowProc(hWnd, Message, wParam, lParam);
}

void OnPaint_Handler(HWND hWnd)
{
    HDC hdc;
    RECT rc;
    PAINTSTRUCT ps;

    hdc = BeginPaint(hWnd, &ps);
    if (m_SinglePicture.m_spIPicture)
    {
        // get width and height of picture
        long hmWidth;
        long hmHeight;
        m_SinglePicture.m_spIPicture->get_Width(&hmWidth);
        m_SinglePicture.m_spIPicture->get_Height(&hmHeight);
        // convert himetric to pixels
        int nWidth	= MulDiv(hmWidth, GetDeviceCaps(hdc, LOGPIXELSX), 2540); //HIMETRIC_INCH);
        int nHeight	= MulDiv(hmHeight, GetDeviceCaps(hdc, LOGPIXELSY), 2540); //HIMETRIC_INCH);
        RECT rc;
        GetClientRect(hWnd, &rc);
        // Display picture using IPicture::Render.
        // MSDN: "Renders (draws) a specified portion of the picture defined by 
        // the offset (xSrc,ySrc) of the source picture and the dimensions to 
        // copy (cxSrc,xySrc). This picture is rendered onto the specified 
        // device context, positioned at the point (x,y), and scaled to the 
        // dimensions (cx,cy)."
        m_SinglePicture.m_spIPicture->Render(hdc, 0, 0, nWidth /* destination width */, 
                          nHeight /* destination height */,  
                          0 /* horizontal offset, source to copy from */, 
                          hmHeight /* verical offset, source to copy from */, 
                                                       hmWidth, -hmHeight, &rc); 
    }
    EndPaint(hWnd, &ps);
}

// Note: center dialog relative to application or desktop window.
// TODO: merge with CenterWindow
BOOL CenterWindow2(HWND hWnd, HWND hBaseWnd /* application or desktop window */) 
{
    RECT rc;
    RECT rcBaseWnd;
    
    int nWindowWidth, nWindowHeight; 
    int nBaseWindowWidth, nBaseWindowHeight; 

    BOOL bRes = GetWindowRect(hWnd, &rc);

    if (bRes != FALSE)
    {
        nWindowWidth = rc.right - rc.left;
        nWindowHeight = rc.bottom - rc.top;

        GetWindowRect(hBaseWnd, &rcBaseWnd);
        nBaseWindowWidth= rcBaseWnd.right - rcBaseWnd.left;
        nBaseWindowHeight = rcBaseWnd.bottom - rcBaseWnd.top;

        SetWindowPos(hWnd, NULL, rcBaseWnd.left + 
            (nBaseWindowWidth - nWindowWidth) / 2, rcBaseWnd.top + 
            (nBaseWindowHeight  - nWindowHeight) / 2, 0, 0, 
                                                   SWP_NOSIZE | SWP_NOACTIVATE);

        return TRUE;
    }

    return FALSE;
}

VOID CALLBACK SplashScreen_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, 
                                                                   DWORD dwTime)
{
    //MessageBox(0, "sadasd", "", 0);
    //DestroyWindow(g_hWndSplashScreen);
}

// TODO: copied from OnNotify_TVItemClick to handle DC on CAR file when CAR file
// is out of default CAR directory. Merge with OnNotify_TVItemClick.
void InitSingleCARFileViewer(HWND hWnd)
{
    RECT rc;
    DWORD dwExStyles;
    DWORD dwStyle;
    RECT rcRBH;

    GetWindowRect(hWnd, &rc);

    //// Subclass to have separate procedure from main window 
    //// procedure.
    //g_wpOldRightHost_DialogProc = (WNDPROC)SetWindowLong(
    //				hWnd, GWL_WNDPROC, 
    //					            (DWORD)RightHostDialogProc_SubclassedFunc);

    if (g_nCarFilesLayout == 0 /* report */)
        dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT;
    else if (g_nCarFilesLayout == 1 /* icons */)
        dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_ICON | 
                                                                LVS_AUTOARRANGE;

    g_hwndLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, 
        "Single CAR file view", dwStyle,
        0, 0, rc.right-rc.left, 500, 
                    hWnd, (HMENU)IDC_LISTVIEW, hInst, 0); 

    // Works with CreateWindow-created (without Ex) window.
    dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyle(g_hwndLV, dwExStyles);

    // The dialog class (#32770) is used to make vertical
    // splitter bar (bottom area on the right) the same appearance with
    // other splitter bars.
    hwndRightBottomHost = CreateWindow("#32770", "", 
        WS_CHILD | WS_VISIBLE, 0, 505, rc.right-rc.left, 300, hWnd, 
        (HMENU)IDC_RIGHT_BOTTOM_HOST /* does not work */, hInst, 0);

    // Subclass to have separate procedure from main window procedure.
    g_wpOldRBH_DialogProc = (WNDPROC)SetWindowLong(hwndRightBottomHost, GWL_WNDPROC, 
        (DWORD)RBHDialogProc_SubclassedFunc);

    // Add left and right bottom windows.
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)BottomLeftWindow_WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= 0;
    wcex.hCursor		= NULL;
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1); 
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= _T("BottomLeftWindow");
    wcex.hIconSm		= NULL;
    RegisterClassEx(&wcex);

    GetWindowRect(hwndRightBottomHost, &rcRBH);

    hwndPictureView = CreateWindowEx(WS_EX_CLIENTEDGE, 
        "BottomLeftWindow", "", WS_VISIBLE | WS_CHILD, 
        0, 0, 200, rcRBH.bottom - rcRBH.top, 
        hwndRightBottomHost, (HMENU)IDC_BOTTOM_PICTUREVIEW, 
                                                    hInst, 0);

    TCHAR sxText[200];
    wsprintf(sxText, "hwndRightBottomHost' RC: (%d %d) - (%d %d)", 
        rcRBH.left, rcRBH.top, rcRBH.right - rcRBH.left, rcRBH.bottom - rcRBH.top);  
    //App_Debug(sxText);

    //hwndBottomLV = CreateWindow(WC_LISTVIEW, "", 
    hwndBottomLV = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", 
        WS_CHILD | WS_VISIBLE | WS_BORDER |LVS_REPORT, 
        200 + 4, 0, rcRBH.right - rcRBH.left - 200 - 4, 
        rcRBH.bottom - rcRBH.top, hwndRightBottomHost, 
                        (HMENU)IDC_BOTTOM_LISTVIEW, hInst, 0);

    // Works with CreateWindow-created (without Ex) window.
    dwExStyles = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyle(hwndBottomLV, dwExStyles);

    // Setup list view colums.
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

    lvc.iSubItem = 0;
    lvc.pszText = "Property";
    lvc.cx = 100; 
    ListView_InsertColumn(hwndBottomLV, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.pszText = "Value";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 200;
    ListView_InsertColumn(hwndBottomLV, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.pszText = "Unit";
    lvc.fmt = LVCFMT_CENTER;
    lvc.cx = 60;
    ListView_InsertColumn(hwndBottomLV, 2, &lvc);

    // When initialized load first picture from MSS (loads also notes for
    // general MSS).
    LoadXMLData(0 /* zero-based index */);

    // Imitates main window resizing or horizontal bar moving to make the 
    // window visible.
    GetWindowRect(hWnd, &rc);
    //SizeWindowContents(rc.right, rc.bottom);

    // Update layout. 
    RECT rcMainClient;
    GetClientRect(hWnd, &rcMainClient); // client RECT is essentia
    SizeWindowContents(rcMainClient.right - rcMainClient.left, 
                            rcMainClient.bottom - rcMainClient.top);

    if (g_nCarFilesLayout == 0 /* report view */)
    {
        InitListViewColumns(g_hwndLV);
        LoadCarData_SingleFile();
        // Populate list view with details from m_vecCarData vector.
        InitListViewItems(g_hwndLV, NULL /* filter */, 
                                    TRUE /* MSS file clicked */);

        // Update units according to loaded from registry or updated.
        if (g_iUnits != DEFAULT_UNITS) 
            ConvertUnits(g_hwndLV, g_iUnits);

        // Sort by g_nSortColumn if relevant.
        if (g_iSortParam != -1 /* no sort if was not used */) 
                                          LV_SortItems(g_hwndLV, g_nSortColumn);
    }
    else if (g_nCarFilesLayout == 1 /* icon view */)
    {
        dwStyle = GetWindowLong(g_hwndLV, GWL_STYLE); 
        if (dwStyle & LVS_REPORT) 
        {
            dwStyle &=~ LVS_REPORT; // remove style
            dwStyle |= LVS_ICON;    // add style 
            SetWindowLong(g_hwndLV, GWL_STYLE, dwStyle);
        }

        LoadPicturesAndStrings(g_hwndLV);
    }
}

void AD_OnPaint(HWND hWnd, LPPAINTSTRUCT pps)
{
    HDC hdc; 
    long hmWidth,hmHeight; // HIMETRIC units (1 himetric unit = 0.01 mm)
    HDC	hdcIC;
    int	iImageWidth_Pixels, iImageHeight_Pixels;

    int iLeftIndent = 10; 
    int	iTopIndent = 10;
    hdc = pps->hdc;

    //  Get picture size in HIMETRIC units that are used by IPicture.
    m_AD_SinglePicture.m_spIPicture->get_Width(&hmWidth);  
    m_AD_SinglePicture.m_spIPicture->get_Height(&hmHeight);

    // Convert HIMETRIC to pixel units (LOGPIXELSX: number of pixels per logical 
    // inch along the screen width).
    hdcIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    iImageWidth_Pixels  = (hmWidth  * GetDeviceCaps(hdcIC, LOGPIXELSX)) / 2540;
    iImageHeight_Pixels = (hmHeight * GetDeviceCaps(hdcIC, LOGPIXELSY)) / 2540;
    DeleteDC(hdcIC);

    // Now draw picture (Render means draw).
    m_AD_SinglePicture.m_spIPicture->Render(hdc, iLeftIndent, iTopIndent, 
        iImageWidth_Pixels, iImageHeight_Pixels, 0, hmHeight, hmWidth, 
                                                               -hmHeight, NULL);
}

// PURPOSE: Shows status bar information for collections (number of of 
//          collection and number of cars in these collections).
// NOTE:    The count method does not use m_vecCarFilesMap, m_vecCarData vectors
//          (except all folder) to avoid complications when "Country", "Year", or
//          "Make " is empty or general MSS file is read. The method uses count
//          of items as seen in UI (TV, LV).

void SB_ShowCollectionsSummary(LPCTSTR pszFolderName, HTREEITEM hItem, 
                                                                    HWND hwndLV)
{
    TCHAR szInfo[MAX_PATH];
    int nCollectionsCount = 0;
    int nImagesCount; // generally cars count, can be images if misc folder

    if (lstrcmp(pszFolderName, "All") == 0 /* equal */) // "All" folder
    {
        wsprintf(szInfo, "Total car collections: %d, Total cars: %d", 
                                  m_vecCarFilesMap.size(), m_vecCarData.size()); 
    }
    else
    {
        nCollectionsCount = TV_GetChildItemCount(hItem);
        nImagesCount = ListView_GetItemCount(hwndLV);
                                             
        // Show the collections information in the status bar.
        if (lstrcmp(pszFolderName, "Favorites") != 0 /* equal */) 
            wsprintf(szInfo, "%s folder: %d collections, %d cars", 
                                pszFolderName, nCollectionsCount, nImagesCount); 
        else // Favorites folder
            wsprintf(szInfo, "%s folder: %d favorites", 
                                                   pszFolderName, nImagesCount);
    }

    SendMessage(g_hwndStatus, WM_SETTEXT, 0 /* not used */, (LPARAM)szInfo);
}

// PURPOSE: utility function for SB_ShowCollectionsSummary that counts 1st level 
//          child items under a selected folder.

int TV_GetChildItemCount(HTREEITEM hItem)
{
    int nCount = 0;
    HTREEITEM hChildItem;

    hChildItem = TreeView_GetChild(hwndBrowser, hItem);
    if (hChildItem == NULL) return 0;

    nCount++;
    hChildItem = TreeView_GetNextSibling(hwndBrowser, hChildItem);
    if (hChildItem == NULL) return nCount; // 1

    nCount++;
    while(hChildItem != NULL)
    {
        hChildItem = TreeView_GetNextSibling(hwndBrowser, hChildItem);
        if (hChildItem == NULL) return nCount; 
        nCount++;
    }

    return nCount; // never here
}

// PURPOSE: shows status bar information for a single collection (CAR or MSS 
//          file).
BOOL SB_ShowCollectionInfo(LPCTSTR pszFFN)
{
      HINSTANCE hResInst;
    TCHAR szTitle[100];
    BOOL bCars;
    TCHAR szBuffer[100];
    int nCarsCount;
    int nRes;
    TCHAR szInfo[MAX_PATH];

    hResInst = LoadLibrary(pszFFN);
    if (hResInst == NULL)
        return FALSE;

    nRes = LoadString(hResInst, 1 /* always Name ID */ , szTitle, 
                                            sizeof(szTitle)/sizeof(szTitle[0]));
    if (nRes == 0 /* the string resource does not exist */)
        return FALSE;

    LoadString(hResInst, 4 /* pictures category ID */, szBuffer, 
                                          sizeof(szBuffer)/sizeof(szBuffer[0]));
    if (nRes == 0 /* the string resource does not exist */)
        return FALSE;

    if (lstrcmp("Cars", szBuffer) == 0 /* equal */)
        bCars = TRUE;
    else 
        bCars = FALSE;

    nRes = LoadString(hResInst, 10 /* IDS_IMAGECOUNT */, szBuffer, 
                                          sizeof(szBuffer)/sizeof(szBuffer[0]));
    if (nRes == 0 /* the string resource does not exist */)
        return FALSE;

    nCarsCount = atoi(szBuffer);
    
    // Show the collection information in the status bar.
    wsprintf(szInfo, "%s collection: %d %s", szTitle, nCarsCount,
                                                     bCars ? "cars" : "images");
                                                                    
    SendMessage(g_hwndStatus, WM_SETTEXT, 0 /* not used */, (LPARAM)szInfo);

    if (!hResInst) FreeLibrary(hResInst); 

    return TRUE;
}

// From MSB_SCfg.scr (application class).
BOOL DecryptString(LPTSTR pszDecryptedPassword, LPCTSTR pszPassword)
{
    int n;
    TCHAR szResult[MAX_LOADSTRING];
    TCHAR szNumber[3];

    TCHAR szLength[1];
    int iLength;
    int iNumber;
    
    for (n = 0; *pszPassword != _T('\0'); n ++)
    {
        // Length must be 1 (really values are 1 ... 3 and this is OK)
        szLength[0]= *pszPassword;
        szLength[1]= _T('\0'); 
        iLength = _ttoi(szLength);

        pszPassword++;
        for( int m = 0; m < iLength; m++)
        {
            szNumber[m] = *pszPassword++; 
        }
        szNumber[iLength] = _T('\0');
        iNumber = _ttoi(szNumber);

        szResult[n] = _T(char(iNumber));
    }
    szResult[n] = _T('\0');
    _tcscpy(pszDecryptedPassword, szResult);

    return TRUE;
}

// From MSB_SCfg.scr (application class).
BOOL EncryptString(LPTSTR pszEncryptedPassword, LPCTSTR pszPassword)
{
    TCHAR szResult[MAX_PATH] = _T("");

    TCHAR szLength[5];
    TCHAR szNumber[5];
    
    int iPasswordLength = _tcslen(pszPassword);
    
    for(int i = 0; i < iPasswordLength; i++)  
    {
        _stprintf(szNumber, "%d", int(*pszPassword)); 
        _stprintf(szLength, "%d", _tcslen(szNumber));

        _tcscat(szResult,szLength);
        _tcscat(szResult,szNumber);

        pszPassword++;
    };

    _tcscpy(pszEncryptedPassword, szResult);

    return TRUE;
}

void TB_SetSingleCARFileIcons()
{
    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
            IDM_FILE_SETFOLDER, 
            MAKELPARAM(9 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));

    //SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
    //		IDM_TOOLS_DOWNLOAD, 
    //		MAKELPARAM(10 /* zero-based image in IL */, 
    //								  0 /* zero-based image list index */));

    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
            ID_TOOLBAR_SEARCH, 
            MAKELPARAM(11 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));
    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
            ID_TOOLBAR_FAVORITES, 
            MAKELPARAM(12 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));

    // Activate TB icon that toggles views for single CAR file view.
    if (g_nCarFilesLayout == 0 /* report view */)
    {
        SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
            ID_TOOLBAR_VIEW, 
            MAKELPARAM(8 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));
    }
    else if (g_nCarFilesLayout == 1 /* icon view */)
    {
        SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
            ID_TOOLBAR_VIEW, 
            MAKELPARAM(7 /* zero-based image in IL */, 
                                        0 /* zero-based image list index */));
    }
}

BOOL WV_ValidatePrices(LPCTSTR pszMinPrice, LPCTSTR pszMaxPrice, 
                                                             LPTSTR pszErrorMsg)
{
    TCHAR szBuffer[MAX_LOADSTRING];
    
    if (lstrlen(pszMinPrice) == 0 || lstrlen(pszMaxPrice) == 0)
    {
        lstrcpy(pszErrorMsg, "The price should not be empty!");        
        return FALSE;
    }

    for (int i = 0; i < lstrlen(pszMinPrice); i++)
    {
        if (!isdigit(*pszMinPrice) && *pszMinPrice != '$' && 
                                                            *pszMinPrice != ',')
        {
            LoadString(hInst, IDS_WV_VALIDATION_MSG, szBuffer, MAX_LOADSTRING);
            lstrcpy(pszErrorMsg, szBuffer);
            return FALSE;
        }
        pszMinPrice++;
    }

    for (int i = 0; i < lstrlen(pszMaxPrice); i++)
    {
        if (!isdigit(*pszMaxPrice) && *pszMaxPrice != '$' && 
                                                            *pszMaxPrice != ',')
        {
            LoadString(hInst, IDS_WV_VALIDATION_MSG, szBuffer, MAX_LOADSTRING);
            lstrcpy(pszErrorMsg, szBuffer);			
            return FALSE;
        }
        pszMaxPrice++;
    }

    return TRUE;
}

void OnExportFavorites(int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                          LPCTSTR pszFolderName)
{
    TCHAR			szFileName[100]; 
    TCHAR			szFFN[MAX_PATH];
    OPENFILENAME	ofn;

    switch (nFolderLevel)
    {
    case 0: // favorites root
        lstrcpy(szFileName, "CE Favorites.csv");
        break;
    case 1: // 1st level
        wsprintf(szFileName, "CE Favorites, %s.csv", pszFolderName);
        break;
    case 2: // 2nd level
        wsprintf(szFileName, "CE Favorites, %s, %s.csv", pszParentFolderName, 
                                                                 pszFolderName);
        break;
    default: // not used 
         lstrcpy(szFileName, "CE Favorites.csv");
    }

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= g_hWnd;
    ofn.hInstance			= NULL;
    ofn.lpstrFilter			= _T("CSV (Comma delimited) (*.csv)\0\0"); 
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    ofn.lpstrFile			= szFileName; // file name as in, FFN as out
    ofn.nMaxFile			= MAX_PATH;
    ofn.lpstrFileTitle		= NULL;
    ofn.nMaxFileTitle		= 0;
    ofn.lpstrTitle			= _T("Export Favorites");
    ofn.Flags				= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; 
    ofn.nFileOffset			= 0;
    ofn.nFileExtension		= 0;
    ofn.lpstrDefExt			= "csv";
    ofn.lCustData			= 0;
    ofn.lpfnHook			= NULL;
    ofn.lpTemplateName	    = NULL;

    if (GetSaveFileName(&ofn))
    {
        lstrcpy(szFFN, ofn.lpstrFile);
        SaveAsCSV(szFFN, nFolderLevel, pszParentFolderName, pszFolderName);
    }
}

void OnExportListView(HWND hwndLV)
{
    TCHAR			szFileName[100]; 
    TCHAR			szFFN[MAX_PATH];
    OPENFILENAME	ofn;

    lstrcpy(szFileName, "CE Export.csv");

    // Avoid arbitrary symbols in File Name on dialog activation.
    ZeroMemory(szFFN, sizeof(szFFN)); 

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= g_hWnd;
    ofn.hInstance			= NULL;
    ofn.lpstrFilter			= _T("CSV (Comma delimited) (*.csv)\0\0"); 
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    ofn.lpstrFile			= szFileName; // file name as in, FFN as out
    ofn.nMaxFile			= MAX_PATH;
    ofn.lpstrFileTitle		= NULL;
    ofn.nMaxFileTitle		= 0;
    ofn.lpstrTitle			= _T("Export List View Data");
    ofn.Flags				= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; 
    ofn.nFileOffset			= 0;
    ofn.nFileExtension		= 0;
    ofn.lpstrDefExt			= "csv";
    ofn.lCustData			= 0;
    ofn.lpfnHook			= NULL;
    ofn.lpTemplateName	    = NULL;

    if (GetSaveFileName(&ofn))
    {
        lstrcpy(szFFN, ofn.lpstrFile);
        SaveListViewAsCSV(szFFN, hwndLV);
    }
}

BOOL OnImportFavorites()
{
    OPENFILENAME	ofn;
    // Serves as initial directory and resulting file returned by the dialog.
    TCHAR szFFN[MAX_PATH]; 
    TCHAR *pToken = NULL;
    char szLine[100];
    ifstream csvfile;
    CCarFavorites m_CF;
    TCHAR szTitle[100];
    TCHAR szCollectionCarID[10]; // intermediary string
    TCHAR szCarTitle[200];
    int nGlobalCarID;
    vector<CCarFavorites> m_vecCarFavoritesMap_Temp;
    TCHAR szMsg[256];
    TVITEM tvi;
    TVINSERTSTRUCT tvis; 
    int nRes;
    BOOL bBrokenFavorites = FALSE;
    BOOL bOldFavorites;
    int nItemLevel;
    TCHAR szGrandparentFolder[100];
    TCHAR szParentFolder[100];
    vector<string> vecRegItems;
    TCHAR szName[10];
    TCHAR szData[200];
    string sData;
    TCHAR szFavoritesBackup[100];
    HKEY hKeyFavoritesBackup, hKeyFavorites;
    DWORD dwValue, dwResult;
    
    // Avoid arbitrary symbols in File Name on dialog activation.
    ZeroMemory(szFFN, sizeof(szFFN)); 

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= g_hWnd;
    ofn.hInstance			= NULL;
    ofn.lpstrFilter			= _T("CSV (Comma delimited) (*.csv)\0\0");
    ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 0;
    // Older (98/Me) ofn.lpstrInitialDir can be used to set initial directory, 
    // but ofn.lpstrFile parameter can be used instead since it serves two 
    // purposes (and supposed to be used mainly for setting initial directory 
    // with 2000/XP): set initial directory and receive selected full path name. 
    ofn.lpstrFile			= szFFN; // gets selected full file path
    ofn.nMaxFile			= MAX_PATH;
    ofn.lpstrFileTitle		= NULL;
    ofn.nMaxFileTitle		= 0;
    ofn.lpstrTitle			= _T("Import Favorites");
    ofn.Flags				= OFN_EXPLORER | OFN_FILEMUSTEXIST | 
                                                              OFN_PATHMUSTEXIST; 
    ofn.nFileOffset			= 0;
    ofn.nFileExtension		= 0;
    ofn.lpstrDefExt			= "csv";
    ofn.lCustData			= 0;
    ofn.lpfnHook			= NULL;
    ofn.lpTemplateName	    = NULL;

    if (GetOpenFileName(&ofn))
    {
        csvfile.open(szFFN);

        if (csvfile) 
        {
            while (csvfile.good())
            {
                csvfile.getline(szLine, MAX_PATH);

                // Prevent insertion of the last line twice.
                if (lstrlen(szLine) == 0) 
                    break; 
                
                nItemLevel = Utility_GetItemLevelFromCSV(szLine);
                
                // MSDN: "On the first call to strtok, the function skips 
                // leading delimiters and returns a pointer to the first token 
                // in strToken, terminating the token with a null character". 
                // Notice that token is a word, not a delimiter.

                // Get CollectionCarID and verify it exists based on available 
                // (local) CAR files.
                switch(nItemLevel)
                {
                case 0: // 0,Peugeot 2011,9
                    pToken = _tcstok(szLine, ","); // not used item number (9) 
                    pToken = _tcstok(NULL, ",");   // Peugeot 2011
                    lstrcpy(szTitle, pToken);
                    pToken = _tcstok(NULL, ",");   // 9
                    lstrcpy(szCollectionCarID, pToken);

                    wsprintf(szData, "%s,%s", szTitle, szCollectionCarID);
                    sData = szData; 
                    vecRegItems.push_back(sData);
                    break;
                case 1: // 31,SUV,Kia 2009,10
                    pToken = _tcstok(szLine, ","); // not used item number (31) 
                    pToken = _tcstok(NULL, ",");   // SUV 
                    lstrcpy(szParentFolder, pToken);
                    pToken = _tcstok(NULL, ",");   // Kia 2009 
                    lstrcpy(szTitle, pToken);
                    pToken = _tcstok(NULL, ",");   // 10
                    lstrcpy(szCollectionCarID, pToken);

                    wsprintf(szData, "1,%s,%s,%s", szParentFolder, szTitle, 
                                                             szCollectionCarID);
                    sData = szData;
                    vecRegItems.push_back(sData);
                    break;
                case 2: // 65,Convertible,$25-30K,Peugeot 2011,8
                    pToken = _tcstok(szLine, ","); // not used item number (65) 
                    pToken = _tcstok(NULL, ",");   // Convertible
                    lstrcpy(szGrandparentFolder, pToken);
                    pToken = _tcstok(NULL, ",");   // $25-30K
                    lstrcpy(szParentFolder, pToken);
                    pToken = _tcstok(NULL, ",");   // Peugeot 2011 
                    lstrcpy(szTitle, pToken);
                    pToken = _tcstok(NULL, ",");   // 8
                    lstrcpy(szCollectionCarID, pToken);

                    wsprintf(szData, "1,%s,2,%s,%s,%s", szGrandparentFolder, 
                                    szParentFolder, szTitle, szCollectionCarID);
                    sData = szData;
                    vecRegItems.push_back(sData);
                    break;
                default:
                    nGlobalCarID = -1; // error
                    break;
                };

                nGlobalCarID = GetGlobalCarID(szTitle, atoi(szCollectionCarID));
                if (nGlobalCarID == -1 /* not found */)
                {
                    bBrokenFavorites = TRUE;
                    LoadString(hInst, IDS_IMPORTFAVORITES_ABORTED, szMsg, 256);
                    MessageBox(0, szMsg, szMBTitle, MB_OK | MB_ICONINFORMATION);
                    break;
                }
            }

            if (!bBrokenFavorites)
            {
                // Prompt a message to confirm replacement of older favorites if 
                // any (meanwhile they are silently backuped in registry.
                if (g_htviFavorites) // likely redundant
                {
                    bOldFavorites = (BOOL)TreeView_GetChild(hwndBrowser, 
                                        g_htviFavorites /* Favorites folder */);
                    if (bOldFavorites)
                    {
                        LoadString(hInst, IDS_IMPORTFAVORITES_REPLACE, szMsg, 
                                                                           256);
                        nRes = MessageBox(0, szMsg, szMBTitle, 
                                                    MB_ICONQUESTION | MB_YESNO);
                        if (nRes == IDYES)
                        {
                            TV_ClearAllFavorites();
                        }
                    }

                    if (bOldFavorites == FALSE || nRes == IDYES)
                    {
                        // Make name for favorites registry backup in the format 
                        // FavoritesBackup_MMDDYY_HHMM.
                        wsprintf(szFavoritesBackup, "%s\\FavoritesBackup_%s", 
                                      REGKEY_CE, Utility_GetDateTime().c_str());

                        // Create favorites backup registry.
                        dwResult = RegCreateKey(APP_BASE_HIVE, 
                                       szFavoritesBackup, &hKeyFavoritesBackup);
                        if (dwResult != ERROR_SUCCESS) 
                        {
                            MessageBox(NULL, "RegCreateKey error!", 
                                                  "OnImportFavorites()", MB_OK);
                            return FALSE;
                        }

                        // The standard Win32 API does not have a function to 
                        // rename registry key, therefore SHCopyKey SHCopyKey 
                        // is used.
                        dwResult = SHCopyKey(APP_BASE_HIVE, REGKEY_CE_FAVORITES, 
                                  hKeyFavoritesBackup, NULL /* must be NULL */);
                        if (dwResult != ERROR_SUCCESS) 
                        {
                            MessageBox(NULL, "Favorites backup error!", 
                                                  "OnImportFavorites()", MB_OK);
                            return FALSE;
                        }

                        // Since we checked above that current favorites are 
                        // backuped successully we can safely delete current 
                        // Favorites from the original location. 
                        RegDeleteKey(APP_BASE_HIVE, REGKEY_CE_FAVORITES); 

                        dwResult = RegCreateKey(APP_BASE_HIVE, 
                                           REGKEY_CE_FAVORITES, &hKeyFavorites);
                        if (dwResult != ERROR_SUCCESS) 
                        {
                            MessageBox(NULL, "Favorites RegCreateKey error", 
                                                  "OnImportFavorites()", MB_OK);
                            return  FALSE;
                        }

                        // Create favorites items in registry from CSV file.
                        for (int i = 0; i < vecRegItems.size(); i++)
                        {
                            wsprintf(szName, "%d", i);
                            StoreTextToRegistry(REGKEY_CE_FAVORITES, szName, 
                                                     vecRegItems.at(i).c_str());
                        }

                        // Reload and repopulate favorites in TV from registry.
                        //m_vecCarFavoritesMap.clear();
                        //vecFavoritesFolders.clear(); 
                        TreeView_DeleteAllItems(hwndBrowser);
                        // Need to include PopulateTreeView (and therefore 
                        // TreeView_DeleteAllItem aboves) since it includes 
                        // setup for standard favorites folders.
                        PopulateTreeView(hwndBrowser, g_nGroup); 
                        App_LoadRegistrySettings();

                        // ??
                        if (TreeView_GetChild(hwndBrowser, 
                                        g_htviFavorites /* Favorites folder */)) // LL redundant
                        {
                            // Set active Favorites icon for TB.
                            SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
                                ID_TOOLBAR_FAVORITES, 
                                MAKELPARAM(3 /* zero-based image in IL */, 
                                         0 /* zero-based image list index */)); 

                            // Set active Favorites icon for TV. 
                            tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
                            tvi.hItem = g_htviFavorites;
                            tvi.iImage = 3; 
                            tvi.iSelectedImage = 3;
                            TreeView_SetItem(hwndBrowser, &tvi);
                        }

                        SetFocus(hwndBrowser); 
                    } // nRes == IDOK
                } // g_htviFavorites
            } // broken favorites
        }
    }

    return TRUE; // processed
}

void SaveAsCSV(LPCTSTR pszFFN, int nFolderLevel, LPCTSTR pszParentFolderName, 
                                                          LPCTSTR pszFolderName)
{
    HANDLE hFileCSV;
    DWORD dwWritten;
    DWORD dwOffset;
    char szLine[100];
    //TCHAR szMsg[256];
    int nRes;
    FILE* pfile = NULL;
    int j = 0;

    // Test opening.
    pfile = fopen(pszFFN, "r");

    // Prompt to replace if file exits.
    if (pfile != NULL) 
        nRes = MessageBox(0, 
            "Would you like to replace existing file?", szMBTitle, 
                                                    MB_ICONQUESTION | MB_YESNO);
    if (pfile == NULL || nRes == IDYES)
    {
        if (pfile != NULL) fclose(pfile);
        
        hFileCSV = CreateFile(pszFFN, 
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL /* security - not used */,  
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

        // Tell the writer we need to write at the end of file.
        dwOffset = SetFilePointer(hFileCSV, 0, NULL, FILE_END);

        switch(nFolderLevel)
        {
        case 0: // Favorites folder
            for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).nItemLevel == 0 
                                                   /* Favorites folder items */)
                {
                    sprintf(szLine, "%d,%s,%d\r\n", i, 
                        AddQuotes(
                            m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                    WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                }
                else if (m_vecCarFavoritesMap.at(i).nItemLevel == 1 
                                                                /* 1st level */)
                {
                    sprintf(szLine, "%d,%s,%s,%d\r\n", i, 
                        m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                        AddQuotes(
                             m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                    WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                }
                else if (m_vecCarFavoritesMap.at(i).nItemLevel == 2 
                                                                /* 2nd level */)
                {
                    sprintf(szLine, "%d,%s,%s,%s,%d\r\n", i, 
                        m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(),
                        m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                        AddQuotes(
                             m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                    WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                }
            }
            break;
        case 1: // first level subfolder
            for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).nItemLevel == 1 /* 1st level */)
                {	                                            
                    if (lstrcmp(
                               m_vecCarFavoritesMap.at(i).sParentFolder.c_str(), 
                                                pszFolderName) == 0 /* equal */)
                    {
                        sprintf(szLine, "%d,%s,%s,%d\r\n", j, 
                            m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                            AddQuotes(
                             m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                        WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                        j++;
                    }

                }
                else if (m_vecCarFavoritesMap.at(i).nItemLevel == 2 
                                                                /* 2nd level */)
                {
                    if (lstrcmp(
                        m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(), 
                                                pszFolderName) == 0 /* equal */)
                    {
                        sprintf(szLine, "%d,%s,%s,%s,%d\r\n", j, 
                          m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(),
                            m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                            AddQuotes(
                             m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                        WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                        j++;
                    }
                }
            }
            break;
        case 2: // second level subfolder
            for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
            {
                if (m_vecCarFavoritesMap.at(i).nItemLevel == 2 /* 2nd level */ 
                    && lstrcmp(
                    m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(), 
                                        pszParentFolderName) == 0 /* equal */ &&
                    lstrcmp(m_vecCarFavoritesMap.at(i).sParentFolder.c_str(), 
                                                pszFolderName) == 0 /* equal */)
                {
                    sprintf(szLine, "%d,%s,%s,%s,%d\r\n", j, 
                        m_vecCarFavoritesMap.at(i).sGrandparentFolder.c_str(),
                        m_vecCarFavoritesMap.at(i).sParentFolder.c_str(),
                        AddQuotes(
                             m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
                                 m_vecCarFavoritesMap.at(i).nCollectionImageID);
                    WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, 
                                                                          NULL);
                    j++;
                }
            }
            break;
        default:
            break;
        }

        //for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
        //{
        //	sprintf(szLine, "%d,%s,%d\r\n", i, 
        //		AddQuotes(m_vecCarFavoritesMap.at(i).sTitle.c_str()).c_str(), 
        //						 m_vecCarFavoritesMap.at(i).nCollectionImageID);

        //	WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, NULL);
        //}
                                                                                
        CloseHandle(hFileCSV);
    }

    if (pfile != NULL) fclose(pfile);
}

void SaveListViewAsCSV(LPCTSTR pszFFN, HWND hwndLV)
{
    TCHAR szTitle[100];
    TCHAR szYear[10];
    TCHAR szMake[100];
    TCHAR szModel[100];
    TCHAR szSubModel[100];
    TCHAR szType[100];
    TCHAR szPrice[100];
    TCHAR szPower[100];
    TCHAR szFuel[100];
    TCHAR szLength[100];
    TCHAR szWidth[100];
    TCHAR szHeight[100];
    TCHAR szClearance[100];
    TCHAR szWeight[100];
    TCHAR szTankVolume[100];
    TCHAR szCountry[100];

    FILE* pfile = NULL;
    int nRes;
    HANDLE hFileCSV;
    DWORD dwWritten;
    DWORD dwOffset;
    char szLine[500];

    // Test opening.
    pfile = fopen(pszFFN, "r");

    // Prompt to replace if file exits.
    if (pfile != NULL) 
        nRes = MessageBox(0, 
            "Would you like to replace existing file?", szMBTitle, 
                                                MB_ICONQUESTION | MB_YESNO);
    if (pfile == NULL || nRes == IDYES)
    {
        // Close stream used to check existence of the file only .
        if (pfile != NULL) fclose(pfile); 

        hFileCSV = CreateFile(pszFFN, 
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ | 
                FILE_SHARE_WRITE, NULL /* security - not used */,          
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

        // Tell the writer we need to write at the end of file.
        dwOffset = SetFilePointer(hFileCSV, 0, NULL, FILE_END);

        for (int i = 0; i < ListView_GetItemCount(hwndLV); i++)
        {
            ListView_GetItemText(hwndLV, i, 0 /* title */, szTitle, 
                                                               sizeof(szTitle));
            ListView_GetItemText(hwndLV, i, 1 /* make */, szYear, 
                                                                sizeof(szYear));
            ListView_GetItemText(hwndLV, i, 2 /* make */, szMake, 
                                                                sizeof(szMake));
            ListView_GetItemText(hwndLV, i, 3 /* model */, szModel, 
                                                               sizeof(szModel));
            ListView_GetItemText(hwndLV, i, 4 /* submodel */, szSubModel, 
                                                            sizeof(szSubModel));
            ListView_GetItemText(hwndLV, i, 5 /* type */, szType, 
                                                                sizeof(szType));
            ListView_GetItemText(hwndLV, i, 6 /* price */, szPrice, 
                                                               sizeof(szPrice));
            ListView_GetItemText(hwndLV, i, 7 /* power */, szPower, 
                                                               sizeof(szPower));
            ListView_GetItemText(hwndLV, i, 8 /* fuel */, szFuel, 
                                                                sizeof(szFuel));
            ListView_GetItemText(hwndLV, i, 9 /* length */,  
                                                    szLength, sizeof(szLength));
            ListView_GetItemText(hwndLV, i, 10 /* width */,  
                                                      szWidth, sizeof(szWidth));
            ListView_GetItemText(hwndLV, i, 11 /* height */,  
                                                    szHeight, sizeof(szHeight));
            ListView_GetItemText(hwndLV, i, 12 /* clearance */,  
                                              szClearance, sizeof(szClearance));
            ListView_GetItemText(hwndLV, i, 13 /* weight */, szWeight, 
                                                              sizeof(szWeight));
            ListView_GetItemText(hwndLV, i, 14 /* tank volume */,  
                                            szTankVolume, sizeof(szTankVolume));
            ListView_GetItemText(hwndLV, i, 15 /* country */, szCountry, 
                                                             sizeof(szCountry));

            sprintf(szLine, 
                "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n", i,  
                AddQuotes(szTitle).c_str(), AddQuotes(szYear).c_str(), 
                AddQuotes(szMake).c_str(), AddQuotes(szModel).c_str(), 
                AddQuotes(szSubModel).c_str(), AddQuotes(szType).c_str(), 
                AddQuotes(szPrice).c_str(), AddQuotes(szPower).c_str(),
                AddQuotes(szFuel).c_str(), AddQuotes(szLength).c_str(),
                AddQuotes(szWidth).c_str(), AddQuotes(szHeight).c_str(),
                AddQuotes(szClearance).c_str(), AddQuotes(szWeight).c_str(),
                 AddQuotes(szTankVolume).c_str(), AddQuotes(szCountry).c_str());

            WriteFile(hFileCSV, szLine, strlen(szLine), &dwWritten, NULL);
        } // for (...)
        CloseHandle(hFileCSV);
    } // if (pfile == NULL || nRes == IDYES)

    if (pfile != NULL) fclose(pfile);
}

void TV_ClearAllFavorites()
{
    HTREEITEM hChildItem;
    TVITEM tvi;

    hChildItem	= TreeView_GetChild(hwndBrowser,  g_htviFavorites);
    while (hChildItem)
    {
        TreeView_DeleteItem(hwndBrowser, hChildItem);
        hChildItem = TreeView_GetChild(hwndBrowser, g_htviFavorites);
    }
    m_vecCarFavoritesMap.clear();
    vecFavoritesFolders.clear();

    // Set grey Favorites icon for TB.
    SendMessage(g_hwndTB, TB_CHANGEBITMAP, 
        ID_TOOLBAR_FAVORITES, 
        MAKELPARAM(12 /* zero-based image in IL */, 
                    0 /* zero-based image list index */));

    // Set grey Favorites icon in the TV.
    tvi.mask =  TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
    tvi.hItem = g_htviFavorites;
    tvi.iImage = 5; // grey 
    tvi.iSelectedImage = 5; // grey
    TreeView_SetItem(hwndBrowser, &tvi);
}

// PURPOSE: Deletes 2nd level favorites' folders.
// NOTES:   Can function only if folder structure consists of 1st and 2nd level 
//          subfolders (no 3d level or deeper). This is because the function 
//          uses 2 names only (grandparent and parent folder names) for unique 
//          lookup. With unique 1st level folder names, the pair grandparent & 
//          parent folder uniquely identifies 2nd level folder (may be not 
//          unique under different 1st level subfolders).

void TV_DeleteFavoritesFolder(TVITEM* ptvi)
{
    HTREEITEM htviParent;
    char szBuf[MAX_PATH + 1] = "";
    TVITEM tvi;
    string sParentFolder, sFolder;
   
    // Get the name of parent folder.
    htviParent = TreeView_GetParent(hwndBrowser, ptvi->hItem);	
    ZeroMemory( &tvi, sizeof(tvi));
    tvi.mask = TVIF_TEXT; 
    tvi.hItem = htviParent;
    tvi.pszText = szBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hwndBrowser, &tvi);
    sParentFolder = tvi.pszText;	
    
    // Get the name of the folder.
    ZeroMemory(&tvi, sizeof(tvi));
    tvi.mask = TVIF_TEXT;
    tvi.hItem = ptvi->hItem;
    tvi.pszText = szBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hwndBrowser, &tvi);
    sFolder = tvi.pszText;

    // Since supposedly 1st level folder names are unique, the pair of 
    // grandparent & parent folder names can be used for unique lookup of
    // 2nd level folder or car item, which itself are not supposed to 
    // be unique (2nd level items from 2 different folders can have the 
    // same names under different folders of the 1st level).
    for (int i = 0; i < m_vecCarFavoritesMap.size(); i++)
    {
        if (m_vecCarFavoritesMap.at(i).sParentFolder 
              /* parent for car item ! */ == sFolder && 
              m_vecCarFavoritesMap.at(i).sGrandparentFolder 
                              /* grandparent for car item ! */ == sParentFolder) 
            // Supposed to be single erase, no duplicates (possible change of 
            // index, which matters with multiple deletes, does not matter).
            m_vecCarFavoritesMap.erase(m_vecCarFavoritesMap.begin() + i);
    }

    // Remove folder from the map of favorites' folders.
    for (int i = 0; i < vecFavoritesFolders.size(); i++)
    {
        if (vecFavoritesFolders.at(i).sParentFolder == sParentFolder && 
                                   vecFavoritesFolders.at(i).sFolder == sFolder)
             // Supposed to be single erase. 
             vecFavoritesFolders.erase(vecFavoritesFolders.begin() + i);
    }
    
    // Finally, delete TV folder.
    TreeView_DeleteItem(hwndBrowser, ptvi->hItem);
}

// PURPOSE: adds quotes to text string containing comma and returns the same 
//          (unmodified) string if no comma to conform to CSV format.

string AddQuotes(LPCTSTR pszText)
{
    TCHAR szQuotedText[100];
    string sResult;

    if (_tcsstr(pszText, ",") != NULL)
    {
        wsprintf(szQuotedText, "\"%s\"\0", pszText);
        sResult = szQuotedText; 
        return sResult;
    }
    else
    {
        sResult = pszText;
        return sResult;
    }
}

// PURPOSE: Makes compare value for ListView_GetCompareReturnValue using first 
//          three charactes.
// NOTE:    With  Type colum it was not enough to use only first character e.g. 
//          "Coupe" and "Convertible". 

int MakeCompareValue(int nPart1, int nPart2, int nPart3)
{
    TCHAR szValue[10];
    
    if (nPart2 < 10) // one-digit value
        if (nPart3 < 10) // one-digit value
            wsprintf(szValue, "%d00%d00%d", nPart1, nPart2, nPart3);
        else if (nPart3 >= 10 && nPart3 < 100) // two-digit value
            wsprintf(szValue, "%d00%d00%0d", nPart1, nPart2, nPart3);
        else // three-digit value
            wsprintf(szValue, "%d00%d00%d", nPart1, nPart2, nPart3);
    else if (nPart2 >= 10 && nPart2 < 100) // two-digit value
        if (nPart3 < 10) // one-digit value
            wsprintf(szValue, "%d0%d00%d", nPart1, nPart2, nPart3);
        else if (nPart3 >= 10 && nPart3 < 100) // two-digit value
            wsprintf(szValue, "%d0%d0%d", nPart1, nPart2, nPart3);
        else // three-digit value
            wsprintf(szValue, "%d0%d%d", nPart1, nPart2, nPart3);
    else  // three-digit value
        if (nPart3 < 10) // one-digit value
            wsprintf(szValue, "%d%d00%d", nPart1, nPart2, nPart3);
        else if (nPart3 >= 10 && nPart3 < 100) // two-digit value
            wsprintf(szValue, "%d%d0%d", nPart1, nPart2, nPart3);
        else // three-digit value
            wsprintf(szValue, "%d%d%d", nPart1, nPart2, nPart3);

    return atoi(szValue);
}

// MSDN: The lParam1 and lParam2 parameters correspond to the lParam member 
//       of the TVITEM structure for the two items being compared. The 
//       lParamSort parameter corresponds to the lParam member of this 
//       structure.

int CALLBACK TV_CompareProc(LPARAM lParam1, LPARAM lParam2, 
                           LPARAM lParamSort /* in, application-defined value*/)
{
    if (lParam1 > lParam2)
        return -1;

    if (lParam1 < lParam2)
        return 1;

    if (lParam1 == lParam2)
        return 0;

    return 0;
}

// PURPOSE: Used while importing favorites to provide unique name for registry 
//          backup in format FavoritesBackup_MMDDYY_HHMM.
// NOTES:   Based on 4S Lock 1.09's function (copied from MSB's 2.1).
string  Utility_GetDateTime()
{
    string sDateTime;
    SYSTEMTIME st;
    char szTime[60];
    char szHour[5];
    char szMinute[5];
    char szMonth[5];
    char szDay[5];
    char szYear[3];
    char szTemp[10];

    // Get time and then format it to MMDDYY_HHMM.
    GetLocalTime(&st);

    // Convert "2:6" time into "02:06" time
    sprintf(szHour, st.wHour < 10 ? "0%d" : "%d",  st.wHour);
    sprintf(szMinute, st.wMinute < 10 ? "0%d" : "%d",  st.wMinute);

    // Convert "1/29/2005" date into "01/29/05" date
    sprintf(szMonth, st.wMonth < 10 ? "0%d" : "%d",  st.wMonth);
    sprintf(szDay, st.wDay < 10 ? "0%d" : "%d",  st.wDay);

    // Convert "2005" into "05" (there is no C-string truncation routine, but
    // this trick works.
    sprintf(szTemp, "%d", st.wYear);
    sprintf(szYear, &szTemp[2]);

    // Format resulting string.
    sprintf(szTime, "%s%s%s_%s%s", szMonth, szDay, szYear, szHour, szMinute);
    
    sDateTime = szTime;

    return sDateTime;
}

string  Utility_GetTVParentFolderName(TVITEM tvi)
{
    HTREEITEM hItemParent;
    char szBuf[MAX_PATH + 1] = "";
    string sParentFolder;

    // Get the name of the parent folder.
    hItemParent = TreeView_GetParent(hwndBrowser, tvi.hItem);
    ZeroMemory(&tvi, sizeof(tvi) );
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hItemParent;
    tvi.pszText = szBuf;
    tvi.cchTextMax = MAX_PATH+1;
    TreeView_GetItem(hwndBrowser, &tvi);
    sParentFolder = tvi.pszText;

    return sParentFolder;
}

void App_Debug(LPCTSTR pszMessage)
{
    TCHAR szEnvVariable[MAX_PATH];
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

    //if (g_bEnableDebug)
    {
        // Set the location of the debug file SLiteTraceFile.txt.
        ExpandEnvironmentStrings("%appdata%", szEnvVariable, 
                                                         sizeof(szEnvVariable));
        wsprintf(szFFN, "%s%s", szEnvVariable, 
                                            "\\Car Explorer\\CE_TraceFile.txt");

        // Convert "2:6" time into "02:06" time.
        sprintf(szHour, st.wHour < 10 ? "0%d" : "%d", st.wHour);
        sprintf(szMinute, st.wMinute < 10 ? "0%d" : "%d", st.wMinute);
        sprintf(szSecond, st.wSecond < 10 ? "0%d" : "%d", st.wSecond);
        sprintf(szYear, "%d", st.wYear);

        // Convert "1/29/2005" date into "01/29/05" date.
        sprintf(szMonth, st.wMonth < 10 ? "0%d" : "%d", st.wMonth);
        sprintf(szDay, st.wDay < 10 ? "0%d" : "%d", st.wDay);

        sprintf(szDate, "%s/%s/%s", szMonth, szDay, szYear); 
        sprintf(szTime, "%s:%s:%s", szHour, szMinute, szSecond);

        hTraceFile = CreateFileA(szFFN, 
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /* security - not used */,          
            OPEN_ALWAYS /* if exists - just open; if does not exist, create it */,
                                    FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

        // Tell the writer we need to write at the end of file.
        dwOffset = SetFilePointer(hTraceFile, 0, NULL, FILE_END);

        sprintf(szLine, "%s,%s: %s\r\n", szDate, szTime, pszMessage); 
        //sprintf(szLine, "%s\r\n", pszMessage);
        WriteFile(hTraceFile, szLine, strlen(szLine), &dWritten, NULL);
                                                                              
        CloseHandle(hTraceFile);
    }
}