#include "stdafx.h"
#include "prsht.h" // PROPSHEETHEADER
#include "commctrl.h" // ListView_GetItemCount
#include "Resource.h"
#include "Registry.h"
#include "Settings.h"

void App_Settings()
{
    PROPSHEETHEADER	m_PropSheet;
    PROPSHEETPAGE m_psp[1];
    
    memset(m_psp, 0, sizeof(m_psp));
    memset(&m_PropSheet, 0, sizeof(m_PropSheet));

    m_psp[0].dwSize = sizeof(PROPSHEETPAGE);
    m_psp[0].dwFlags = PSP_DEFAULT | PSP_USETITLE; // | PSP_HASHELP;
    m_psp[0].hInstance = GetModuleHandle(NULL); //g_hInst;
    m_psp[0].pszTemplate = (LPCSTR)IDD_PP_GENERAL;
    m_psp[0].pszTitle = "General";
    m_psp[0].pfnDlgProc = (DLGPROC)PageDlgProc;

 //   m_psp[1].dwSize = sizeof(PROPSHEETPAGE);
 //   m_psp[1].dwFlags = PSP_USETITLE | PSP_HASHELP;
 //   m_psp[1].hInstance = g_hInst;
 //   m_psp[1].pszTemplate = (LPCSTR)IDD_PP_IMAGESETS;
 //   m_psp[1].pszTitle = "Image Sets";
    //m_psp[1].pfnDlgProc = (DLGPROC)Page1DlgProc;

    m_PropSheet.dwSize = sizeof(PROPSHEETHEADER);
    m_PropSheet.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | 
                                 PSH_NOAPPLYNOW | PSH_USECALLBACK; // | PSH_HASHELP;
    m_PropSheet.hInstance = GetModuleHandle(NULL); //g_hInst;
    m_PropSheet.pszCaption = (LPSTR) "Car Explorer Settings";
    m_PropSheet.nPages = 1; // 3
    m_PropSheet.nStartPage = 0;
    m_PropSheet.ppsp = (LPCPROPSHEETPAGE)m_psp;
    m_PropSheet.pfnCallback = (PFNPROPSHEETCALLBACK) PropSheetProc;

     PropertySheet(&m_PropSheet);
}


// Callback function for property sheet. TODO: likely remove since not used.
LRESULT CALLBACK PropSheetProc(HWND hDlg, UINT message, LPARAM lParam)
{
    switch (message)
    {
    case PSCB_PRECREATE: 
        // none  
        return TRUE;
    case PSCB_INITIALIZED:
        // none  
        break;
    }

    return FALSE; // always returns zero
}

LRESULT CALLBACK PageDlgProc(HWND hDlg, UINT message, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    RECT rc;
    RECT rcMainWnd;
    int iSelectedUnits;
    int iOriginalUnits;
    HWND hwndLV;
    int nRes;
    
    iOriginalUnits = g_iUnits;

    switch (message)
    {
        case WM_INITDIALOG:
            // Center property sheet dialog relative to the application 
            // window. Notice that the call is placed in the default 
            // property page instead of placing it after the PropertySheet 
            // call, which returns window handle (the latter does not work). 
            // In fact in Win32, contrary to MFC, it is not a simple task  
            // and one general solution includes the use of WH_CBT hook and 
            // PropertySheet procedure.
            CenterWindow(hDlg);
            
            SendMessage(GetDlgItem(hDlg, IDC_SETTINGS_COMBO), CB_ADDSTRING, 
                    0 /* must be zero */, (LPARAM)"US (inches/pounds/gallons)");
            SendMessage(GetDlgItem(hDlg, IDC_SETTINGS_COMBO), CB_ADDSTRING, 
                         0 /* must be zero */, (LPARAM)"Metric (mm/kg/liters)");

            SendMessage(GetDlgItem(hDlg, IDC_SETTINGS_COMBO), CB_SETCURSEL, 
                                                g_iUnits, 0 /* must be zero */);

            // Set initial state of checkbox. Notice that the variable
            // g_bShowStartupWelcomeView refers to the registry. 
            SendMessage(GetDlgItem(hDlg, IDC_SETTINGS_CHECKBOX_SHOWWV), 
                BM_SETCHECK, g_bShowStartupWelcomeView ? BST_CHECKED : 
                        BST_UNCHECKED /* inversed in UI */, 0 /* must be zero */);
            
            //if (GetPicturesCategory() != 1 /* Cars */) 
            //{
            //	EnableWindow(GetDlgItem(hDlg, IDC_SETTINGS_COMBO), FALSE);
            //	EnableWindow(GetDlgItem(hDlg, IDC_SETTINGS_GROUPTBOX), FALSE);
            //	EnableWindow(GetDlgItem(hDlg, IDC_SETTINGS_STATIC), FALSE);
            //}

            return TRUE;
        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
            {
            case PSN_APPLY: // OK, Apply, Close buttons of property sheet
                iSelectedUnits = SendMessage(GetDlgItem(hDlg, 
                    IDC_SETTINGS_COMBO), CB_GETCURSEL, 0 /* must be zero */, 
                                                          0 /* must be zero */);
                if (iOriginalUnits != iSelectedUnits) 
                {
                    g_iUnits = iSelectedUnits;

                    if (IsWindowVisible(hwndMainLV)) 
                    {
                        hwndLV = hwndMainLV;
                        ConvertUnits(hwndLV, iSelectedUnits);
                    }
                    else if (IsWindowVisible(g_hwndLV)) 
                    {
                        hwndLV = g_hwndLV;
                        ConvertUnits(hwndLV, iSelectedUnits);
                    }

                    // Convert data to new units (iSelectedUnits) if layout is 
                    // not empty (ConvertXMLData uses list view).
                    if (IsWindowVisible(hwndBottomLV) && 
                                       ListView_GetItemCount(hwndBottomLV) != 0)
                    {
                        ConvertXMLData(iSelectedUnits);	
                    }
                }

                // Update welcome view variable accoding to the chackbox state.
                nRes = SendMessage(GetDlgItem(hDlg, 
                    IDC_SETTINGS_CHECKBOX_SHOWWV), BM_GETCHECK, 
                                            0 /* not used */, 0 /* not used */);
                if (nRes == BST_CHECKED)
                    g_bShowStartupWelcomeView = TRUE;
                else if (nRes == BST_UNCHECKED)
                {
                    g_bShowStartupWelcomeView = FALSE;
                    //DestroyWindow(g_hwndWelcome);
                }
                
                // Update check box in WV if this view is visible. Notice that 
                // state of checkbox in WV is inverse to the registry variable 
                // g_bShowStartupWelcomeView (and checkbox in Setting dialog).
                // This is to keep standard title: "Do not show...".
                if (IsWindowVisible(g_hwndWelcome))
                {
                    SendMessage(GetDlgItem(g_hwndWelcome, 
                        IDC_SETTINGS_CHECKBOX_SHOWWV), BM_SETCHECK, 
                        g_bShowStartupWelcomeView ? BST_UNCHECKED : 
                        BST_CHECKED /* inversed in UI */, 0 /* must be zero */);
                }

                // Save dialog setting to registry.
                //SaveRegistryData();
                //PostQuitMessage(0);
                break;
            case PSN_RESET: 
                //// User clicked Cancel or system upper-right "x".
                //PostQuitMessage(0);
                break;
            default:
                // nothing
                break;
            }
    }
    return FALSE;
}

// Note: center dialog relative to application (default) or desktop window.
BOOL CenterWindow(HWND hWnd, 
                          BOOL bAppRelative /* default is TRUE (declaration) */)
{
    RECT rc;
    RECT rcMainWnd;
    int nScreenWidth, nScreenHeight;
    int nWindowWidth, nWindowHeight; 

    BOOL bRes = GetWindowRect(hWnd, &rc);

    if (bRes != FALSE)
    {
        nWindowWidth = rc.right - rc.left;
        nWindowHeight = rc.bottom - rc.top;

        if (bAppRelative) // center relative to application
        {
            GetWindowRect(g_hWnd, &rcMainWnd);

            SetWindowPos(GetParent(hWnd), NULL, 
                rcMainWnd.left + ((rcMainWnd.right - 
                rcMainWnd.left) - nWindowWidth) / 2, rcMainWnd.top + 
                ((rcMainWnd.bottom - rcMainWnd.top) - nWindowHeight) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else // center relative to desktop
        {
            nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
            nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

            SetWindowPos(GetParent(hWnd), NULL, (nScreenWidth - nWindowWidth)/2,
                (nScreenHeight  - nWindowHeight)/2, 0, 0, 
                                                     SWP_NOSIZE | SWP_NOZORDER);
        }
        return TRUE;
    }

    return FALSE;
}
