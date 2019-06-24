#include "stdafx.h"
//#include "CarExplorer.h" 
#include "Resource.h"
#include "LoadImageSets.h"
#include "wininet.h"  // wininet
#include "commctrl.h" // progress bar

//--
using namespace std ; //for vector declaration
HINSTANCE g_hInst = NULL; //TODO: get actual instance ...
HANDLE hListReadingCompleteEvent;
BOOL g_bDownloadActive = FALSE;
BOOL g_bFailedConnectionWarningActive = FALSE;
//--

// Definitions for global variables.
HWND g_hLoadImageSetsDlg;
HANDLE g_hPWDThread; 
HANDLE g_hDLISThread;
HANDLE g_hDISThread; 
// Made global to delete MSS with thread termination on cancel.
TCHAR g_szLocalFile[MAX_PATH]; 
HANDLE g_hLocalFile;
DWORD dwContext;
HANDLE hConnectedEvent, hRequestOpenedEvent, hDownloadCompleteEvent, 
                                                                  hResultHandle;
BOOL bUseAlternativeRemoteFileURL = FALSE;
TCHAR g_szPWD[50];

// Definitions for function.
void PopulateLocalListBox(LPCTSTR pszSerchString);
BOOL GetImageSetTitle(LPCTSTR pszLocation /* in */, LPTSTR pszTitle /* out */);
void GetImageSetFile(LPCTSTR pszTitle /* in */, LPTSTR pszFileName /* out */);
int GetImageSetFileSize(LPCTSTR pszTitleName);
void OnListBox1DblClick();
void CALLBACK OnStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
    DWORD dwInternetStatus, LPVOID lpvStatusInformation, 
                                               DWORD dwStatusInformationLength);
BOOL CheckInternetConnection(LPCTSTR pszURL);

// Misc definitions.
struct THREADDATA
{
    TCHAR szTitleName[100]; // passing wstring to thread does not work
};

class CTitleNameToFileName
{
public:
    string sTitleName; // Image set title name
    string sYear; 
    string sFileName;  // Image set file name
    string sFileSize;  // Image set file size (KB)
};

vector<CTitleNameToFileName> m_vecNamesMap;

// Members used to initialize combobox from pre-sorted vector m_vecYearsMap. 
vector<string> m_vecYearsMap;
vector<string>::iterator it;

struct YearSortDesc
{
    bool operator() (const string & x, const string & y )
    {
        return atoi(x.c_str()) > atoi(y.c_str());
    }
};

BOOL IsYearStored(string sYear);

void OnInitDialog_Handler(HWND hwnd)
{
    RECT rc;
    RECT rcMainWnd;
    //HANDLE hThread;
    UINT iThreadID;
    UINT iPWDThreadID;
    //HANDLE hPWDThread;
    TCHAR szSearchString[MAX_PATH];
    HWND hwndComboBox;
    
    // Center dialog relative to the application window.
    GetWindowRect(GetWindow(hwnd, GW_OWNER) /* application window */, 
                                                                    &rcMainWnd);
    GetWindowRect(hwnd, &rc);
    SetWindowPos(hwnd, NULL, rcMainWnd.left + ((rcMainWnd.right - 
        rcMainWnd.left) - (rc.right - rc.left)) / 2, rcMainWnd.top + 
        ((rcMainWnd.bottom - rcMainWnd.top) - (rc.bottom - rc.top)) / 2,
                                             0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    g_hLoadImageSetsDlg = hwnd;

    // Disable button until list is downloaded and PWD is obtained.
    EnableWindow(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_BUTTON1), FALSE);

    // Clear the map of titles to prevent having identical items during one CE
    // session (when LIS downalod called more than once).
    m_vecNamesMap.clear();

    // Download list of image sets from server and populate left list box
    // asynchronously in separate thread.
    hListReadingCompleteEvent = CreateEvent(NULL, TRUE /* manual reset */, 
                                               FALSE /* none-signaled */, NULL);

    // Hide progress bar.
    ShowWindow(GetDlgItem(hwnd, IDC_LIS_PROGRESS_BAR), FALSE);

    //// Thread auto-terminates when thread function returns. 
    //hPWDThread = (HANDLE)_beginthreadex(NULL, 0, 
    //	                            &GetPWD_ThreadProc, NULL, 0, &iPWDThreadID);
    //if (hPWDThread != NULL) CloseHandle(hPWDThread);
    
    // Thread auto-terminates when thread function returns. TODO: LL in my case,
    // this is thread still active when thread function returns. Check out!
    g_hDLISThread = (HANDLE)_beginthreadex(NULL, 0, 
                       &ThreadProcDownloadListOfImageSets, NULL, 0, &iThreadID);
    if (g_hDLISThread != NULL) CloseHandle(g_hDLISThread);

    // Thread auto-terminates when thread function returns. 
    g_hPWDThread = (HANDLE)_beginthreadex(NULL, 0, 
                                    &GetPWD_ThreadProc, NULL, 0, &iPWDThreadID);
    if (g_hPWDThread != NULL) CloseHandle(g_hPWDThread);
    
    // Now populate right list box based on collections found in the
    // g_szCarFilesFolder. If found, the name string for the list box is 
    // extracted from DLL (CAR or MSS).

    // Populate local list box with MSS files.
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, _T("\\*.mss")); 
    PopulateLocalListBox(szSearchString);

    // Populate local list box with CAR files.
    wsprintf(szSearchString, "%s%s", g_szCarFilesFolder, _T("\\*.car")); 
    PopulateLocalListBox(szSearchString);

    // Add "All" item to the combobox. Notice that all other available years are
    // added to the combox dynamically based on the server data (with static 
    //data old CE releases would be impossible to update).
    hwndComboBox = GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_COMBOBOX);
    SendMessage(hwndComboBox,  CB_ADDSTRING, 0 /* must be zero */, 
                                                                 (LPARAM)"All");
    SendMessage(hwndComboBox, CB_SETCURSEL, 
                                  0 /* g_nSearch_Type */, 0 /* must be zero */);
}

void PopulateLocalListBox(LPCTSTR pszSearchString)
{
    TCHAR szTitle[100];
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    TCHAR szImagePath[MAX_PATH];

    // FindFirstFile, FindNextFile and, when found, obtain title string packed 
    // within MSS. 
    hFind = FindFirstFile(pszSearchString , &fd);

    if(hFind != INVALID_HANDLE_VALUE)
    {
        wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, _T("\\"), 
                                                                  fd.cFileName); 
                                                                  
        GetImageSetTitle(szImagePath, szTitle);
        SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX2), 
                           LB_ADDSTRING, 0 /* must be zero */, (LPARAM)szTitle);

        while(FindNextFile(hFind , &fd))
        {
            wsprintf(szImagePath, "%s%s%s", g_szCarFilesFolder, _T("\\"), 
                                                                  fd.cFileName);
            GetImageSetTitle(szImagePath, szTitle);
            SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX2), 
                           LB_ADDSTRING, 0 /* must be zero */, (LPARAM)szTitle);

        }
        FindClose(hFind);
    }
}

// Purpose: read encrypted PWD with HTTP for later used with FTP to download 
// MSS files.
static UINT __stdcall GetPWD_ThreadProc(LPVOID param)
{
    TCHAR szMsg[100];
    HINTERNET hInternetSession; 
    HINTERNET hFile; 
    TCHAR szRemoteFileURL[MAX_PATH];
    TCHAR szBuffer[100]; // long enough to keep all entries from PWD.txt
    string sBuffer;
    DWORD dwRead;
    TCHAR *pToken = NULL;
    string sLine;
    TCHAR szText[100];
    TCHAR szErrorMsg[100];
    LPVOID lpOutBuffer = NULL;
    DWORD dwSize = 0;
    BOOL bRes;
    short nConnAttemptsCount = 0;
    short nMaxConnAttempts = 10; // some value, even 2 practically OK

    // Make a warning message when internet connection is not available. 
    lstrcpy(szMsg, "There was a problem while connecting to the internet! ");
    lstrcat(szMsg, "Please check connection settings and try again ...");
    
    // Very slow (~40 sec), but works.
    if (!CheckInternetConnection("http://www.google.com" /* any major site */))  	
    {
        if (!g_bFailedConnectionWarningActive)
        {
            g_bFailedConnectionWarningActive = TRUE;
            MessageBox(0, szMsg, "Car Explorer", MB_ICONINFORMATION);
        }
        return FALSE;
    }

    SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                               "Downloading the server list of collections...");
    
    // To avoid ERROR_HTTP_HEADER_NOT_FOUND (12150) the provisions to make 
    // several attempts to connect are made. They are limited to 
    // nMaxConnAttempts. Each attempt recreates InternetOpen, 
    // InternetOpenUrl, which were closed by previous unsuccessful attempt. 
    // Typically ERROR_HTTP_HEADER_NOT_FOUND connection error occurs on reboot 
    // or after some period of inactivity. In this case, the string beginning 
    // with "<!DOCTYPE ..." is returned, however the 2nd connect was always OK. 
    // This is why I make additional attempts programmatically. The issue is 
    // known (internet), the workaround is mine. Each attempt starts for the 
    // label:
    retry_session: 
    
    nConnAttemptsCount++;
    hInternetSession  = 
        InternetOpen("Car Explorer 1.03" /* calling agent */, 
        INTERNET_OPEN_TYPE_DIRECT, NULL /* proxy */, NULL /*proxy bypass */, 0);
                                                                             
    if (hInternetSession)
    {
        // Load the string for PWD.txt location from MSB site or use 
        // alternative (permanent) location if the MSB site is down.
        if (!bUseAlternativeRemoteFileURL)
            LoadString(g_hInst, IDS_PWD_IFFN, szRemoteFileURL, 
                                  sizeof(szRemoteFileURL) /* valid for ANSI */); 
        else
            LoadString(g_hInst, IDS_PWD_ALT_IFFN, szRemoteFileURL, 
                                  sizeof(szRemoteFileURL) /* valid for ANSI */); 

        hFile = InternetOpenUrl(hInternetSession, szRemoteFileURL, 
        NULL, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 
                                                               0 /* context */); 
        if (hFile)
        {
            // Retry label to avoid ERROR_HTTP_HEADER_NOT_FOUND (12150) error.
            retry:

            // This call will fail on the first pass, because no buffer is 
            // allocated (code from MSDN, a little bit modified). TODO: there is 
            // a potential eternal loop in the code, make sure to avoid (like 
            // variable for count of retries).
            if (!HttpQueryInfo(hFile, HTTP_QUERY_CONTENT_LENGTH, //HTTP_QUERY_RAW_HEADERS_CRLF,
                                            (LPVOID)lpOutBuffer, &dwSize, NULL))
            {
                if (GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND)
                {
                    // Code to handle the case where the header isn't available.
                    //if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                    //	                         "ERROR_HTTP_HEADER_NOT_FOUND");

                    if (lpOutBuffer) delete lpOutBuffer;
                    if (hFile) InternetCloseHandle(hFile);
                    if (hInternetSession) InternetCloseHandle(hInternetSession);

                    if (nConnAttemptsCount <= nMaxConnAttempts)
                        goto retry_session;
                    else
                    {
                        // if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                        //   "The maximum number of connection attempts is exceeded!");
                        return FALSE;
                    }
                }		
                else
                {
                    // Check for an insufficient buffer.
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        //if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                        //	                       "ERROR_INSUFFICIENT_BUFFER");
                        // Allocate the necessary buffer.
                        lpOutBuffer = new char[dwSize];
                            
                        // Retry the call.
                        goto retry;				
                    }		
                }		
            }
        
            // Read the target TXT file.	
            bRes = InternetReadFile(hFile, (LPVOID)szBuffer, 
                                               100 /* long enough */, &dwRead);
            if (bRes)
            {
                // We might be here when the MSB site exists, but still need to 
                // check if the server response is PWD.txt.
                // Note: one alternative method to check if the right file is
                // read is to use InternetReadFile to read say 100 initial 
                // characters. This, however, does not work perfectly since file
                // pointer is moved before the next normal use of 
                // InternetReadFile and, consequently, the list of images set 
                // would be read incorrectly.
                sBuffer = szBuffer;

                // First string of PWD.txt must be "PWD".
                if (sBuffer.find(_T("PWD")) == -1)
                {
                    // It is not the PWD.txt since "PWD" string is not found.
                    if (lpOutBuffer) delete lpOutBuffer;
                    if (hFile) CloseHandle(hInternetSession);
                    if (hInternetSession) CloseHandle(hInternetSession);
                    bUseAlternativeRemoteFileURL = TRUE;
                    nConnAttemptsCount = 0;
                    goto retry_session;
                }
                
                // The server response is the PWD.txt file.
                // Continue processing.
                wsprintf(szText, "InternetReadFile result: %d %d", bRes, 
                                                                        dwRead);
                ////if (g_bEnableDebug) 
                ////   App_Debug_FunctionCallTrace(szText); // 1789!! as of 07/25/09

                pToken = _tcstok( szBuffer, _T("\n\r"));
                //sLine = pToken;
                pToken = _tcstok( NULL, _T("\n\r"));
                //sLine = pToken;
                DecryptString(g_szPWD /* decrypted password */, pToken); // OK wocYjvk1
                //MessageBox(0, pToken, g_szPWD, 0);
                // Set event object into signaled state to allow downloads.
                SetEvent(hListReadingCompleteEvent);
            }
        }
        else
        {
            // We might be here when the MSB site does not exist. Before 
            // prompting an error message, try to use the alternative URL if it 
            // was not yet used. 
            if (!bUseAlternativeRemoteFileURL) 
            {
                if (lpOutBuffer) delete lpOutBuffer;
                if (hFile) CloseHandle(hInternetSession);
                if (hInternetSession) CloseHandle(hInternetSession);
                bUseAlternativeRemoteFileURL = TRUE;
                nConnAttemptsCount = 0;
                goto retry_session;
            }
            
            // When alternative URL also does not work, prompt an error message.
            wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
            //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
            MessageBox(NULL, szErrorMsg, "InternetOpenUrl failure!", 
                                                            MB_ICONEXCLAMATION); 
            if (hInternetSession) InternetCloseHandle(hInternetSession);

            return FALSE;
        }
    }
    else
    {
        wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
        //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
        MessageBox(NULL, szErrorMsg, "InternetOpen failure!", 
                                                            MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (lpOutBuffer) delete lpOutBuffer;
    if (hFile) InternetCloseHandle(hFile);
    if (hInternetSession) InternetCloseHandle(hInternetSession);

    return TRUE;
}

// NOTES: _beginthreadex's procedure must return a thread exit code 
//        (_beginthreadex's thread returns 0 on failure) (MSDN).
// USES:  "http://www.myscreensaverbuilder.com/Downloads/MSB/Image_Sets/
//         CE_ImageSetsList_v1.1.txt" or
//         "http://netston.tripod.com/Software/MSB/Image_Sets/
//         CE_ImageSetsList_v1.1.txt".
static UINT __stdcall ThreadProcDownloadListOfImageSets(LPVOID param)
{
    TCHAR szMsg[100];
    HINTERNET hInternetSession; 
    HINTERNET hFile; 
    TCHAR szRemoteFileURL[MAX_PATH];
    TCHAR szBuffer[4000]; // long enough to keep all entries from text file 
    string sBuffer;
    DWORD dwRead;
    TCHAR *pToken = NULL;
    string sLine;
    TCHAR szText[100];
    string sTitleName, sYear, sFileName, sFileSize;
    CTitleNameToFileName  m_Names;
    TCHAR szErrorMsg[100];
    LPVOID lpOutBuffer = NULL;
    DWORD dwSize = 0;
    BOOL bRes;
    HWND hwndCombobox;
    LRESULT lRes;
     short nConnAttemptsCount = 0;
    short nMaxConnAttempts = 10; // some value, even 2 practically OK

    // Make a warning message when internet connection is not available. 
    lstrcpy(szMsg, "There was a problem while connecting to the internet! ");
    lstrcat(szMsg, "Please check connection settings and try again ...");
    
    // Very slow (~40 sec), but works.
    if (!CheckInternetConnection("http://www.google.com" /* any major site */))  	
    {
        if (!g_bFailedConnectionWarningActive)
        {
            g_bFailedConnectionWarningActive = TRUE;
            MessageBox(0, szMsg, "Car Explorer", MB_ICONINFORMATION);
        }
        return FALSE;
    }

    SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                               "Downloading the server list of collections...");
    
    // To avoid ERROR_HTTP_HEADER_NOT_FOUND (12150) the provisions to make 
    // several attempts to connect are made. They are limited to 
    // nMaxConnAttempts. Each attempt recreates InternetOpen, 
    // InternetOpenUrl, which were closed by previous unsuccessful attempt. 
    // Typically ERROR_HTTP_HEADER_NOT_FOUND connection error occurs on reboot 
    // or after some period of inactivity. In this case, the string beginning 
    // with "<!DOCTYPE ..." is returned, however the 2nd connect was always OK. 
    // This is why I make additional attempts programmatically. The issue is 
    // known (internet), the workaround is mine. Each attempt starts for the 
    // label:
    retry_session: 
    
    nConnAttemptsCount++;
    hInternetSession  = 
        InternetOpen("Car Explorer 1.03" /* calling agent */, 
        INTERNET_OPEN_TYPE_DIRECT, NULL /* proxy */, NULL /*proxy bypass */, 0);
                                                                             
    if (hInternetSession)
    {
        // Load the string for CE_ImageSetsList_v1.1.txt location from MSB
        // site of use alternative (permanent) location if the MSB site is down.
        if (!bUseAlternativeRemoteFileURL)
            LoadString(g_hInst, IDS_IMAGESETSLIST_LOCATION, szRemoteFileURL, 
                                  sizeof(szRemoteFileURL) /* valid for ANSI */); 
        else
            LoadString(g_hInst, IDS_IMAGESETSLIST_ALT_LOCATION, szRemoteFileURL, 
                                  sizeof(szRemoteFileURL) /* valid for ANSI */); 

        hFile = InternetOpenUrl(hInternetSession, szRemoteFileURL, 
        NULL, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 
                                                               0 /* context */); 
        if (hFile)
        {
            // Retry label to avoid ERROR_HTTP_HEADER_NOT_FOUND (12150) error.
            retry:

            // This call will fail on the first pass, because no buffer is 
            // allocated (code from MSDN, alittle bit modified). TODO: there is 
            // a potential eternal loop in the code, make sure to avoid (like 
            // variable for count of retries).
            if (!HttpQueryInfo(hFile, HTTP_QUERY_CONTENT_LENGTH, //HTTP_QUERY_RAW_HEADERS_CRLF,
                                            (LPVOID)lpOutBuffer, &dwSize, NULL))
            {
                if (GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND)
                {
                    // Code to handle the case where the header isn't available.
                    //if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                    //	                         "ERROR_HTTP_HEADER_NOT_FOUND");

                    if (lpOutBuffer) delete lpOutBuffer;
                    if (hFile) InternetCloseHandle(hFile);
                    if (hInternetSession) InternetCloseHandle(hInternetSession);

                    if (nConnAttemptsCount <= nMaxConnAttempts)
                        goto retry_session;
                    else
                    {
                        // if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                        //   "The maximum number of connection attempts is exceeded!");
                        return FALSE;
                    }
                }		
                else
                {
                    // Check for an insufficient buffer.
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        //if (g_bEnableDebug) App_Debug_FunctionCallTrace(
                        //	                       "ERROR_INSUFFICIENT_BUFFER");
                        // Allocate the necessary buffer.
                        lpOutBuffer = new char[dwSize];
                            
                        // Retry the call.
                        goto retry;				
                    }		
                }		
            }
        
            // Read the target TXT file.	
            bRes = InternetReadFile(hFile, (LPVOID)szBuffer, 
                                               4000 /* long enough */, &dwRead);
            if (bRes)
            {
                // We might be here when the MSB site exists, but still need to 
                // check if the server response is CE_ImageSetsList_v1.1.txt.
                // Note: one alternative method to check if the right file is
                // read is to use InternetReadFile to read say 100 initial 
                // characters. This, however, does not work perfectly since file
                // pointer is moved before the next normal use of 
                // InternetReadFile and, consequently, the list of images set 
                // would be read incorrectly.
                sBuffer = szBuffer;
                if (sBuffer.find(_T(".mss")) == -1)
                {
                    // It is not the CE_ImageSetsList_v1.1.txt since ".mss" 
                    // string is not found.
                    if (lpOutBuffer) delete lpOutBuffer;
                    if (hFile) CloseHandle(hInternetSession);
                    if (hInternetSession) CloseHandle(hInternetSession);
                    bUseAlternativeRemoteFileURL = TRUE;
                    nConnAttemptsCount = 0;
                    goto retry_session;
                }
                
                // The server response is the CE_ImageSetsList_v1.1.txt file.
                // Continue processing.
                wsprintf(szText, "InternetReadFile result: %d %d", bRes, 
                                                                        dwRead);
                //if (g_bEnableDebug) 
                //   App_Debug_FunctionCallTrace(szText); // 1789!! as of 07/25/09

                pToken = _tcstok( szBuffer, _T("\n\r"));
                sLine = pToken; 

                sTitleName = sLine.substr(0, sLine.find(_T("|")));
                sYear = sTitleName.substr(sTitleName.length() - 4, 4);
                sFileName = sLine.substr(sLine.find(_T("|")) + 1, 
                         sLine.find_last_of(_T("|")) - sLine.find(_T("|")) - 1);
                sFileSize = sLine.substr(sLine.find_last_of(_T("|")) + 1, 
                     sLine.length() - sTitleName.length() - sFileName.length()); 

                m_Names.sTitleName = sTitleName;
                m_Names.sYear = sYear;
                m_Names.sFileName  = sFileName; 
                m_Names.sFileSize = sFileSize;
                m_vecNamesMap.push_back(m_Names); 

                hwndCombobox = GetDlgItem(g_hLoadImageSetsDlg, 
                                                              IDC_LIS_COMBOBOX); 
                // Add to m_vecYearsMap only unique years.
                if (!IsYearStored(sYear)) 
                                         m_vecYearsMap.push_back(sYear.c_str());

                // Add title (item) for the remote listbox.
                SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX1), 
                    LB_ADDSTRING, 0 /* must be zero */, 
                                                    (LPARAM)sTitleName.c_str());

                pToken = _tcstok( NULL, _T("\n\r"));
                while (pToken)
                {
                    sLine = pToken; 
                    // Prevent from parsing gibberish. Since each line ends with 
                    // "CR|LF", this algorithm makes possible to read after the 
                    // last line and the line's content is gibberish symbols. 
                    // Strong walkaround is to test if ".mss" substring is 
                    // contained in the line.
                    if (sLine.find(_T(".mss")) == -1) break; 

                    sTitleName = sLine.substr(0, sLine.find(_T("|")));
                    sYear = sTitleName.substr(sTitleName.length() - 4, 4);
                    sFileName = sLine.substr(sLine.find(_T("|")) + 1, 
                         sLine.find_last_of(_T("|")) - sLine.find(_T("|")) - 1);
                    sFileSize = sLine.substr(sLine.find_last_of(_T("|")) + 1, 
                     sLine.length() - sTitleName.length() - sFileName.length()); 

                    m_Names.sTitleName = sTitleName;
                    m_Names.sYear = sYear;
                    m_Names.sFileName  = sFileName;
                    m_Names.sFileSize = sFileSize;
                    m_vecNamesMap.push_back(m_Names);

                    // Add to m_vecYearsMap only unique years.
                    if (!IsYearStored(sYear)) 
                                         m_vecYearsMap.push_back(sYear.c_str());

                    // Add title (item) for the remote listbox.
                    SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
                        IDC_LIS_LISTBOX1), LB_ADDSTRING, 0 /* must be zero */, 
                                                    (LPARAM)sTitleName.c_str());
                    pToken = _tcstok( NULL, _T("\n\r"));
                }

                // Sort m_vecYearsMap before adding its members to combobox 
                // ("All" member is added already with WM_INITDIALOG).
                std::sort(m_vecYearsMap.begin(), m_vecYearsMap.end(), 
                                                                YearSortDesc());
                for (it = m_vecYearsMap.begin(); it != m_vecYearsMap.end(); 
                                                                           it++)
                {
                    SendMessage(hwndCombobox, CB_ADDSTRING, 0, 
                                                         (LPARAM)it->c_str());
                }
                
                SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                    "Double click to download a collection from Car Server...");

                // Be sure before enabling download button that 2nd thread is 
                // finished.
                while (WAIT_OBJECT_0 /* object is signaled */ != 
                        WaitForSingleObject(hListReadingCompleteEvent 
                       /* initially event object is not signaled */, INFINITE));
                EnableWindow(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_BUTTON1), 
                                                                          TRUE);
            }
        }
        else
        {
            // We might be here when the MSB site does not exist. Before 
            // prompting an error message, try to use the alternative URL if it 
            // was not yet used. 
            if (!bUseAlternativeRemoteFileURL) 
            {
                if (lpOutBuffer) delete lpOutBuffer;
                if (hFile) CloseHandle(hInternetSession);
                if (hInternetSession) CloseHandle(hInternetSession);
                bUseAlternativeRemoteFileURL = TRUE;
                nConnAttemptsCount = 0;
                goto retry_session;
            }
            
            // When alternative URL also does not work, prompt an error message.
            wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
            //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
            MessageBox(NULL, szErrorMsg, "InternetOpenUrl failure!", 
                                                            MB_ICONEXCLAMATION); 
            if (hInternetSession) InternetCloseHandle(hInternetSession);

            return FALSE;
        }
    }
    else
    {
        wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
        //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
        MessageBox(NULL, szErrorMsg, "InternetOpen failure!", 
                                                            MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (lpOutBuffer) delete lpOutBuffer;
    if (hFile) InternetCloseHandle(hFile);
    if (hInternetSession) InternetCloseHandle(hInternetSession);

    return TRUE;
}

// USES:  "ftp://ftp1801807:srhHylg8du@034b905.netsolhost.com/htdocs/Downloads/
//         MSB/Image_Sets" (former MSB site was active up to 07/31/12) 
//        "ftp://netston:Moscow1964@ftp.tripod.com/Software/MSB/Image_Sets" 
static UINT __stdcall ThreadProcDownloadImageSet(LPVOID pParam)
{
    THREADDATA* ptd = NULL;
    //THREADDATA td;
    TCHAR szFileName[MAX_PATH];
    int nFileSize;
    TCHAR szTitle[100];
    TCHAR szErrorMsg[100];
    HINTERNET hInternetSession;
    //TCHAR szLocalFile[MAX_PATH];
    TCHAR szFTPUser [MAX_PATH];
    TCHAR szFTPHost [MAX_PATH];
    TCHAR szFTPRepoDir [MAX_PATH];
    TCHAR szRemoteDir [MAX_PATH];
    TCHAR szRemoteFileURL[MAX_PATH];
    TCHAR szCarFileName_Server[MAX_PATH];
    LPTSTR pszCFN = NULL;
    TCHAR szCarFileName[MAX_PATH];
    HINTERNET hURL;
    //HANDLE hLocalFile;
    DWORD dwCount;
    int iPos;
    TCHAR szReadBuffer[1024];
    DWORD dwRead, dwWritten;

    ptd = (THREADDATA*)pParam; 

    GetImageSetFile(ptd->szTitleName, szCarFileName_Server); 
    nFileSize = GetImageSetFileSize(ptd->szTitleName);

     SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                                            _T("Connecting to Car Server ..."));
    hInternetSession  = 
        InternetOpen("Car Explorer 1.03" /* calling agent */, 
        INTERNET_OPEN_TYPE_PRECONFIG, NULL /* proxy */, NULL /*proxy bypass */, 
                                                           INTERNET_FLAG_ASYNC);
    if (hInternetSession) 
    {
        // Make full file name for the local file, updating extension to CAR.
        // Notice that server name is trimed off on the left from the "Images_" 
        // string e.g. "Images_Acura_2009.mss" is transformed to 
        // "Acura_2009.car".
        ZeroMemory(szCarFileName, sizeof(szCarFileName)); 
        strncpy(szCarFileName, szCarFileName_Server, 
                                  lstrlen(szCarFileName_Server) - 4 /* .mss */);
        lstrcat(szCarFileName, ".car");
        pszCFN = szCarFileName;
        // Notice that LL C-function to copy from the left n-position of 
        // C-string is not available  in C/C++ (due to the simplicity of the 
        // operation?).
        lstrcpy(szCarFileName, pszCFN + 7 /* car make starts */); 
        // Or: lstrcpy(szCarFileName, szCarFileName + 7 /* car make starts */); 
        wsprintf(g_szLocalFile, "%s%s%s", g_szCarFilesFolder, "\\", 
                                                                 szCarFileName);
        //MessageBox(0, szCarFileName, g_szLocalFile, 0);
        // Make full file name for the remote file.
        if (!bUseAlternativeRemoteFileURL)
        {
            LoadString(g_hInst, IDS_FTP_HOST, szFTPHost, 
                                        sizeof(szFTPHost) /* valid for ANSI */); 
            LoadString(g_hInst, IDS_FTP_USER, szFTPUser, 
                                        sizeof(szFTPUser) /* valid for ANSI */); 
            LoadString(g_hInst, IDS_FTP_REPO_DIR, szFTPRepoDir, 
                                     sizeof(szFTPRepoDir) /* valid for ANSI */); 

            wsprintf(szRemoteFileURL, "ftp://%s:%s@%s/%s/%s",szFTPUser, g_szPWD, 
                                 szFTPHost, szFTPRepoDir, szCarFileName_Server);
        }
        else // use alternative MSS repository
        {
            LoadString(g_hInst, IDS_MSS_REPO_ALT_LOCATION, szRemoteDir, 
                                  sizeof(szRemoteFileURL) /* valid for ANSI */);
            wsprintf(szRemoteFileURL, "%s/%s", szRemoteDir, 
                                                          szCarFileName_Server);
                                                            
        }

        // Settings are like for MSB's MSB_SCfg.scr, which uses (OpenURL): 
        // INTERNET_FLAG_PASSIVE and INTERNET_FLAG_TRANSFER_BINARY (no analog 
        // with InternetOpenUrl, LL binary is deafult). Context is set to 
        // zero unlike OpenURL's default - 1 to avoid 997 error.
        hURL = InternetOpenUrl(hInternetSession, szRemoteFileURL, 
                              NULL, 0, INTERNET_FLAG_PASSIVE, 0 /* context */); 
        if (hURL) 
        {
            // Show progress bar.
            ShowWindow(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_PROGRESS_BAR), 
                                                                          TRUE);

            //InternetSetStatusCallback(hURL, 
            //	                    (INTERNET_STATUS_CALLBACK)OnStatusCallback);
            SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                           _T("Downloading the collection from Car Server..."));
            
            g_hLocalFile = CreateFile(g_szLocalFile, GENERIC_WRITE, 0, 
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_PROGRESS_BAR),
                (UINT) PBM_SETRANGE, 0 /* not used, must be zero */,
                (LPARAM) MAKELPARAM (0 /* min position */, 
                                                 nFileSize /* max position */));
    
            // Write from remote to local file.
            dwCount = 0;
            while (InternetReadFile(hURL, (LPVOID)szReadBuffer, 
                                                 sizeof(szReadBuffer), &dwRead))
            {
                if (!dwRead) break;
                WriteFile(g_hLocalFile, (LPVOID)szReadBuffer, dwRead, &dwWritten, 
                                                                          NULL);
                dwCount += dwRead; //dwCount =+ dwRead; does not increament!

                iPos = (int)(float)(dwCount/1000.);

                SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
                    IDC_LIS_PROGRESS_BAR), (UINT) PBM_SETPOS, iPos, 
                                                0 /* not used, must be zero */);
            }
            CloseHandle(g_hLocalFile);

            // Verify if downloaded file is CAR file. For example, it can be
            // zero-sized file. Notice that if Cancel is clicked during 
            // download it generally results in normal download since Cancel
            // does not terminate downloading thread. The IsMSSFile name is 
            // used instead of IsCARFile to reserve an option to view (and  
            // check) general (not file of car type) in the CE.
            if (IsMSSFile(g_szLocalFile))
            {
                SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                                 "The collection was downloaded successfully!");
                GetImageSetTitle(g_szLocalFile, szTitle);
                SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX2), 
                                    LB_ADDSTRING, 0 /* must be zero */, 
                                                       (LPARAM)(LPTSTR)szTitle);
                
                // Update TV and vectors in main window.
                AddCarFile(szTitle, g_szLocalFile);
            }
            else
            {
                DeleteFile(g_szLocalFile);
                SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                          "An error occured while downloading the collection!");
                //MessageBox(0, "Not CAR file", "", 0);
            }

            // Set the flag to allow the next download.
            g_bDownloadActive = FALSE;

            // Hide progress bar when downloaded is finished.
            ShowWindow(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_PROGRESS_BAR), 
                                                                         FALSE);

            if (hURL) InternetCloseHandle(hURL);
        }
        else
        {
            wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
            // CAREFULL: aleways remove in release!!
            //wsprintf(szErrorMsg, "Last Error: %d %s",  GetLastError(), 
            //                                                 szRemoteFileURL); 
            MessageBox(NULL, szErrorMsg, "InternetOpenUrl failure!", 
                                                            MB_ICONEXCLAMATION); 
        }

        if (hInternetSession) InternetCloseHandle(hInternetSession);
    }
    else
    {
        wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
        MessageBox(NULL, szErrorMsg, "InternetOpen failure!", 
                                                            MB_ICONEXCLAMATION); 
    }
    
    return 0;
}

void OnComboBoxChangeSelection()
{
    TCHAR szYear[10];
    LRESULT nYear;
    int i;
    
    // Get string for current year selection.
    nYear = SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_COMBOBOX), 
                      CB_GETCURSEL, 0 /* must be zero */, 0 /* must be zero */);
    SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_COMBOBOX), 
                                           CB_GETLBTEXT, nYear, (LPARAM)szYear);

    // Clear remote listbox before repopulation.
    SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
        IDC_LIS_LISTBOX1), LB_RESETCONTENT, 0 /* must be zero */, 
                                                          0 /* must be zero */); 

    // Re-populate remote list box filtering m_vecNamesMap data with selectied 
    // year.
    for (i = 0; i < m_vecNamesMap.size(); i++)
    {
        if (nYear == 0 /* All */)
        {
            SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX1), 
                LB_ADDSTRING, 0 /* must be zero */, 
                                (LPARAM)m_vecNamesMap.at(i).sTitleName.c_str());
        }
        else if (lstrcmp(m_vecNamesMap.at(i).sYear.c_str(), szYear) == 0)
        {
            SendMessage(GetDlgItem(g_hLoadImageSetsDlg, IDC_LIS_LISTBOX1), 
                LB_ADDSTRING, 0 /* must be zero */, 
                                (LPARAM)m_vecNamesMap.at(i).sTitleName.c_str());
        }
    }
}

void OnListBox1DblClick()
{
    wstring sTitleName;
    LRESULT lCurSelResult;
    LRESULT lFindResult;

    //HANDLE hThread;
    UINT iThreadID;

    lCurSelResult = 
        SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
                        IDC_LIS_LISTBOX1), LB_GETCURSEL, 0, 0);

    SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
        IDC_LIS_LISTBOX1), LB_GETTEXT, lCurSelResult, 
                        (LPARAM)(LPTSTR) sTitleName.c_str());

    lFindResult = 
        SendMessage(GetDlgItem(g_hLoadImageSetsDlg, 
                IDC_LIS_LISTBOX2), LB_FINDSTRING, 0, 
                        (LPARAM)(LPTSTR)sTitleName.c_str());

    if(lFindResult == LB_ERR)
    {
        // Set the flag to prohibit concurrent downloads.
        g_bDownloadActive = TRUE;

        THREADDATA* ptd = new THREADDATA;
        //ptd->sTitleName = sTitleName; // not passedfor unknown reason
        lstrcpy(ptd->szTitleName, 
                                (LPTSTR)sTitleName.c_str());
        
        g_hDISThread = (HANDLE)_beginthreadex(NULL, 0, 
                       &ThreadProcDownloadImageSet, (LPVOID)ptd, 0, &iThreadID);
    }
    else
    {
        TCHAR szMessage[100];
        wsprintf(szMessage,"%s%s%s", _T("The collection "), 
                          (LPTSTR)sTitleName.c_str(), _T(" is already local!"));
        MessageBox(NULL, szMessage, _T("Car Explorer"), 0);
    }

}

void GetImageSetFile(LPCTSTR pszTitle /* in */, LPTSTR pszFileName /* out */)
{
    for (UINT i = 0; i < m_vecNamesMap.size(); i++)
    {
        if (!lstrcmp(m_vecNamesMap.at(i).sTitleName.c_str(), pszTitle))
        {
            lstrcpy(pszFileName, m_vecNamesMap.at(i).sFileName.c_str());
            break;
        }
    }
}

int GetImageSetFileSize(LPCTSTR pszTitleName)
{
    for (UINT i = 0; i < m_vecNamesMap.size(); i++)
    {
        if(!lstrcmp(m_vecNamesMap.at(i).sTitleName.c_str(), pszTitleName))
        {
            int nFileSize = atoi(m_vecNamesMap.at(i).sFileSize.c_str());
            return nFileSize; 
            //lstrcpy(pszFileName, m_vecNamesMap.at(i).sFileName.c_str());
        }
    }
}

// TODO: OnStatusCallback is used only with async HTTP or FTP downloads, which
//       I don't use for now. Probably remove.
void CALLBACK OnStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
    DWORD dwInternetStatus, LPVOID lpvStatusInformation, 
                                                DWORD dwStatusInformationLength)
{
    LPINTERNET_ASYNC_RESULT pIAR = 
                                  (LPINTERNET_ASYNC_RESULT)lpvStatusInformation;
    TCHAR szStatusMessage[100];

    switch (dwInternetStatus)
    {
        case INTERNET_STATUS_HANDLE_CREATED:
            // Store actual FTP session handle in global variable.
            hResultHandle = (HINTERNET)pIAR->dwResult;
            //SetEvent(hConnectedEvent); //too early
            lstrcpy(szStatusMessage, 
                       _T("Downloading collection ..."));
            break;
        case INTERNET_STATUS_REQUEST_COMPLETE:
            SetEvent(hConnectedEvent);
            //SetEvent(hDownloadCompleteEvent);
            lstrcpy(szStatusMessage, "INTERNET_STATUS_REQUEST_COMPLETE");
            break;
        case INTERNET_STATUS_CLOSING_CONNECTION:
            lstrcpy(szStatusMessage, _T("Closing connection ..."));
            break;
        case INTERNET_STATUS_CONNECTED_TO_SERVER:
            lstrcpy(szStatusMessage, _T("Connected to server ..."));
            break;
        case INTERNET_STATUS_CONNECTING_TO_SERVER:
            lstrcpy(szStatusMessage, _T("Connecting to server ..."));
            break;
        case INTERNET_STATUS_CONNECTION_CLOSED:
            lstrcpy(szStatusMessage, _T("Connection closed ..."));
            break;
        case INTERNET_STATUS_HANDLE_CLOSING:
            //"Double click to download the collection  ..."
            lstrcpy(szStatusMessage, 
                                 "The collection was downloaded successfully!");
            break;
        case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
            lstrcpy(szStatusMessage, "INTERNET_STATUS_INTERMEDIATE_RESPONSE");
            break;
        case INTERNET_STATUS_NAME_RESOLVED:
            lstrcpy(szStatusMessage, _T("Name resolved ..."));
            break;
        case INTERNET_STATUS_RECEIVING_RESPONSE:
            lstrcpy(szStatusMessage, _T("Receiving response ..."));
            break;
        case INTERNET_STATUS_RESPONSE_RECEIVED:
            //"Double click to download image ...";
            SetEvent(hDownloadCompleteEvent);
            lstrcpy(szStatusMessage, "INTERNET_STATUS_RESPONSE_RECEIVED");
            break;
        case INTERNET_STATUS_REDIRECT:
            lstrcpy(szStatusMessage, "INTERNET_STATUS_REDIRECT");
            break;
        case INTERNET_STATUS_REQUEST_SENT:
            lstrcpy(szStatusMessage, _T("Request sent ..."));
            break;
        case INTERNET_STATUS_RESOLVING_NAME:
            lstrcpy(szStatusMessage, _T("Resolving name ..."));
            break;
        case INTERNET_STATUS_SENDING_REQUEST:
            lstrcpy(szStatusMessage, _T("Sending request ..."));
            break;
        case INTERNET_STATUS_STATE_CHANGE:
            lstrcpy(szStatusMessage, "INTERNET_STATUS_STATE_CHANGE");
            break;
        default:
            lstrcpy(szStatusMessage, "default");
            break;
        }

        SetDlgItemText(g_hLoadImageSetsDlg, IDC_LIS_STATIC_STATUS, 
                                                               szStatusMessage);
}

LRESULT CALLBACK LoadImageSets(HWND hDlg, UINT message, WPARAM wParam, 
                                                                  LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        OnInitDialog_Handler(hDlg);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDC_LIS_COMBOBOX:
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    OnComboBoxChangeSelection();
                }
                break;
            case  IDC_LIS_LISTBOX1:
                switch(HIWORD(wParam))
                {
                    case LBN_DBLCLK :
                        if (!g_bDownloadActive)
                            OnListBox1DblClick();
                        else
                            MessageBox(0, 
                             "Please wait until current download is completed!", 
                                         szMBTitle, MB_OK | MB_ICONINFORMATION);
                        break;
                }
                break;
            case IDC_LIS_BUTTON1:
                OnListBox1DblClick();
                break;
            case IDC_LIS_BUTTON2:
                ShellExecute(NULL, "open", 
                 "http://www.carexplorer.org/RegForm.htm", 0, 0, SW_SHOWNORMAL);
                break;
            case IDOK:
                EndDialog(hDlg, 0);
                break;
            case IDCANCEL:
                EndDialog(hDlg, 0);
                break;
            default:
                break;
        }
        return TRUE;
    case WM_DESTROY:
        {
            DWORD dwExitCode;
            if (g_hPWDThread != NULL) TerminateThread(g_hPWDThread, dwExitCode);
            if (g_hDLISThread != NULL) 
                                     TerminateThread(g_hDLISThread, dwExitCode); 
            if (g_hDISThread != NULL) 
            {
                CloseHandle(g_hLocalFile);
                // SLite.scr v1.4.16 copy: Delete the file being downloaded if 
                // the dialog is manually closed while download (if this is the 
                // rare case an ugly message box "Bad Image" might show up, 
                // which must be avoided (TODO), don't delete if the file 
                // download is complete. Note: for some unknown reason DL thread 
                // sometimes (unstable with XP, OK with Windows 7) doesn't auto-
                // terminate (TODO) when download is complete, consequently file 
                // downloaded in full might be deleted wrongly, therefore the 
                // check is added (v1.4.16) with IsMSSFile.
                if (!IsMSSFile(g_szLocalFile)) DeleteFile(g_szLocalFile);
                //DeleteFile(g_szLocalFile);
                g_bDownloadActive = FALSE;

                TerminateThread(g_hDISThread, dwExitCode);
            }
            // Reset g_bFailedConnectionWarningActive flag to FALSE so that if 
            // LIS is invoked for the second time in one session, the warning 
            // shows up if no connection.
            if (g_bFailedConnectionWarningActive) 
                                       g_bFailedConnectionWarningActive = FALSE;
        }
        break;
    //case WM_HELP:
    //	OnHelp_Handler(); // works but now handled in suclassed proc.
    //	break;
    }

    return FALSE;
}

BOOL GetImageSetTitle(LPCTSTR pszLocation /* in */, LPTSTR pszTitle /* out */)
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

BOOL CheckInternetConnection(LPCTSTR pszURL)
{
    DWORD flag = 0;
    
    // Check if wininet.dll is available.
    if (!LoadLibrary("wininet.dll"))
    {
        MessageBox(0, "IE is not installed likely: wininet.dll not found!", 
                                            "Car Explorer", MB_ICONINFORMATION);
        return FALSE;
    }
    
    // Check for internet connection.
    if (InternetGetConnectedState(&flag, 0))
    {	
        if (InternetCheckConnection(pszURL, FLAG_ICC_FORCE_CONNECTION, 0))
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

// PURPOSE: used to ensure that m_vecYearsMap has only unique members.
BOOL IsYearStored(string sYear)
{
    int i;

    for (i = 0; i < m_vecYearsMap.size(); i++)
    {
        if (lstrcmp(m_vecYearsMap.at(i).c_str(),
                                   sYear.c_str()) == 0 /* equal */) return TRUE;
    }
    
    return FALSE;
}