#include "stdafx.h"
#include "UpdatesWindow.h"
#include "wininet.h"  // wininet
#include <fstream> //ifstream

HANDLE g_hUpdatesThread;

string g_sText1;
string g_sTextColor1;
string g_sTextArea1;
string g_sText2;
string g_sTextColor2;
string g_sTextArea2;
string g_sText3;
string g_sTextColor3;
string g_sTextArea3;
string g_sText4;
string g_sTextColor4;
string g_sTextArea4;

string g_sText5;
string g_sTextColor5;
string g_sTextArea5;
string g_sText6;
string g_sTextColor6;
string g_sTextArea6;

string g_sTextArea1_Dyn;
string g_sTextArea2_Dyn;
string g_sTextArea3_Dyn;
string g_sTextArea4_Dyn;
string g_sTextArea5_Dyn;
string g_sTextArea6_Dyn;
COLORREF g_clrContractingText = RGB(100, 100, 100); // RGB(150, 150, 150)

void OnPaint_Canvas4(HWND hWnd, LPPAINTSTRUCT pps)
{
    HDC hdc;
    RECT rcUpdatesWnd;
    long iRed, iGreen, iBlue;
    char szHTMLColorValue[10]; // e.g. #A52A2A
    char szTextArea[30];
    RECT rc;

    hdc = pps->hdc;

    // Helps correctly repaint the window when downloading CE_Updates.xml is 
    // delayed (cached CE_Updates.xml is shown temporary). Notice that if 
    // SetBkMode is not used, DrawText paints background in white.
    SetBkMode(hdc, TRANSPARENT); 
    GetClientRect(hWnd, &rcUpdatesWnd);
    FillRect(hdc, &rcUpdatesWnd, CreateSolidBrush(RGB(0, 0, 0)));

    // Show the data from CE_Updates.xml.

    // Line 1.
    strcpy(szHTMLColorValue, g_sTextColor1.c_str());
    // Skip when OnPaint is called for the 1st time even before CE_Updates.xml 
    // is parsed.
    if (strlen(szHTMLColorValue) != 0)  
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        strcpy(szTextArea, g_sTextArea1_Dyn .c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText1.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }

    strcpy(szHTMLColorValue, g_sTextColor2.c_str());
    if (strlen(szHTMLColorValue) != 0)
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        strcpy(szTextArea, g_sTextArea2_Dyn .c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText2.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }

    // Line 2.
    strcpy(szHTMLColorValue, g_sTextColor3.c_str());
    if (strlen(szHTMLColorValue) != 0)
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        strcpy(szTextArea, g_sTextArea3_Dyn .c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText3.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }

    strcpy(szHTMLColorValue, g_sTextColor4.c_str());
    if (strlen(szHTMLColorValue) != 0)
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        strcpy(szTextArea, g_sTextArea4_Dyn .c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText4.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }

    // Line 3.
    strcpy(szHTMLColorValue, g_sTextColor5.c_str());
    if (strlen(szHTMLColorValue) != 0)
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        strcpy(szTextArea, g_sTextArea5_Dyn .c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText5.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }

    strcpy(szHTMLColorValue, g_sTextColor6.c_str());
    if (strlen(szHTMLColorValue) != 0)
    {
        ParseToDecColorValues(szHTMLColorValue, iRed, iGreen, iBlue);
        if (!g_bCanvas4Contracting)
            SetTextColor(hdc, RGB(iRed, iGreen, iBlue));
        else
            SetTextColor(hdc, g_clrContractingText);
        //strcpy(szTextArea, g_sTextArea6.c_str());
        strcpy(szTextArea, g_sTextArea6_Dyn.c_str());
        ParseToRectValues(szTextArea, rc);
        DrawText(hdc, g_sText6.c_str(), -1, &rc, 
                                              DT_LEFT | DT_VCENTER | DT_NOCLIP);
    }
}

void StartDownloadUpdatesInfoThread()
{
    UINT iThreadID;
    // Thread auto-terminates when thread function returns. 
    g_hUpdatesThread = (HANDLE)_beginthreadex(NULL, 0, 
                       &DownloadUpdatesInfo_ThreadProc, NULL, 0, &iThreadID);
    if (g_hUpdatesThread != NULL) CloseHandle(g_hUpdatesThread);
}

// Purpose: Downloads CE_Updates.xml to install directory, calls its parsing and 
//          makes refresh of Updates Window.
// Notes: 1) Whether internet connection is available or not the CE_Updates.xml 
//        is always parsed from install directory: (a) if connection available  
//        the CE_Updates.xml is, firstly, downloaded to install directory and  
//        then parsed, (b) if connection is not availabe the earlier downloaded 
//        CE_Updates.xml is parsed from the install directory, (c) if connection  
//        is not available and CE_Updates.xml is not downloaded yet (e.g. fresh 
//        install) the CUpdatesWindow shows default screen.
//        2) CE 1.02: http://www.carexplorer.org/Downloads/MSB/Image_Sets/CE_Updates.xml
//        Test: http://synaptex.tripod.com/MyScreenSaver_Builder/Updates.xml

static UINT __stdcall DownloadUpdatesInfo_ThreadProc(LPVOID param)
{
    HINTERNET hInternetSession;
    HINTERNET hFile; 
    TCHAR szRemoteFileURL[MAX_PATH];
    // TODO: probably allocate size dynamically
    TCHAR szRemoteFile[1000]; // long enough to keep all entries from CE_Updates.xml
    BOOL bRes;
    DWORD dwBytesRead;
    LPVOID lpOutBuffer = NULL;
    DWORD dwSize = 0;
    BOOL bCachedUpdatesFileExists;
    short nConnAttemptsCount = 0;
    short nMaxConnAttempts = 10; // some value, even 2 practically OK
    TCHAR szErrorMsg[100];
//
    // Check if CE_Updates.xml file was already downloaded.
    bCachedUpdatesFileExists = IsCachedUpdatesFileExists();
    
    // Always show cached CE_Updates.xml before updating Updates.xml from the 
    // site hence it helps to avoid visible delay.
    if (bCachedUpdatesFileExists) 
    {
        ParseUpdatesFile();
        ::RedrawWindow(g_hwndCanvas4,  
            NULL /* invalidate entire client area */,
                      NULL /* invalidate entire client area */, RDW_INVALIDATE);
    }												

    // Check internet connection and if it is not available show in the Updates 
    // Window the default information or according to cached CE_Updates.xml. The
    // check might be very slow (~40 sec), but works.
    if (!CheckInternetConnection("http://www.google.com" /* any major site */))  	
    {
        if (!bCachedUpdatesFileExists)
        {
            // CE_Updates.xml is not yet downloaded: show default information 
            // in the Updates Window. 
            g_sText1 = "Turn on internet connection to see updates!";
            g_sTextColor1 = "#00FF00";
            g_sTextArea1 = "10,40,280,40";
        }
        else
        {
            // CE_Updates.xml is is already cached: parse CE_Updates.xml.
            ParseUpdatesFile();
        }

        ::RedrawWindow(g_hwndCanvas4,  
            NULL /* invalidate entire client area */,
                      NULL /* invalidate entire client area */, RDW_INVALIDATE);

        // TODO: dont't prompt message that internet connection is not 
        // available, instead use BMP for header area. Test with offline mode 
        // (IE), when proxy server is used.
        return FALSE;
    }

    ////
    // From LIS's GetPWD_ThreadProc:

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
        InternetOpen("Car Explorer 1.01" /* calling agent */, 
        INTERNET_OPEN_TYPE_DIRECT, NULL /* proxy */, NULL /*proxy bypass */, 0);
                                                                             
    if (hInternetSession)
    {
        // TODO: use alternative if MSB is unavailable.
        //// Load the string for CE_Updates.xml location from MSB site or use 
        //// alternative (permanent) location if the MSB site is down.
        //if (!bUseAlternativeRemoteFileURL)
        //    LoadString(g_hInst, IDS_PWD_IFFN, szRemoteFileURL, 
        //					      sizeof(szRemoteFileURL) /* valid for ANSI */); 
        //else
        //	LoadString(g_hInst, IDS_PWD_ALT_IFFN, szRemoteFileURL, 
        //					      sizeof(szRemoteFileURL) /* valid for ANSI */); 

        // Construct URL string  for retrieving CE_Updates.xml file:
        lstrcpy(szRemoteFileURL, "http://www.carexplorer.org");
        lstrcat(szRemoteFileURL, "/Downloads/MSB/Image_Sets/");
        lstrcat(szRemoteFileURL, "CE_Updates.xml");

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
        
            // Read the target XML file.	
            bRes = InternetReadFile(hFile, (LPVOID)szRemoteFile, 
                1000 /* must be enough for CE_Update.xml size */, &dwBytesRead);
            szRemoteFile[dwBytesRead] = '\0';
            if (bRes)
            {
                // We might be here when the MSB site exists, but still need to 
                // check if the server response is CE_Updates.xml. The 
                // CE_Updates.xml must contain <Car_Explorer> tag as its ID.
                if (IsValidRemoteUpdatesFile(szRemoteFile))
                {
                    // Store (cache) CE_Updates.xml locally overwriting existing 
                    // if present. 
                    CacheUpdatesFile(szRemoteFile);

                    // Parse the CE_Updates.xml file with XML parser and show
                    // results in updates window. 
                    ParseUpdatesFile();

                    ::RedrawWindow(g_hwndCanvas4,  
                        NULL /* invalidate entire client area */,
                        NULL /* invalidate entire client area */, 
                                                            RDW_INVALIDATE);
                }
                else
                {
                    // It is not the CE_Updates.xml file.
                    if (lpOutBuffer) delete lpOutBuffer;
                    if (hFile) CloseHandle(hInternetSession);
                    if (hInternetSession) CloseHandle(hInternetSession);
                    //bUseAlternativeRemoteFileURL = TRUE;
                    nConnAttemptsCount = 0;
                    goto retry_session;
                }
            }
        }
        else
        {
   //         // We might be here when the MSB site does not exist. Before 
            //// prompting an error message, try to use the alternative URL if it 
            //// was not yet used. 
            //if (!bUseAlternativeRemoteFileURL) 
            //{
            //	if (lpOutBuffer) delete lpOutBuffer;
            //	if (hFile) CloseHandle(hInternetSession);
            //	if (hInternetSession) CloseHandle(hInternetSession);
            //	bUseAlternativeRemoteFileURL = TRUE;
            //	nConnAttemptsCount = 0;
            //	goto retry_session;
            //}
            
            // When alternative URL also does not work, prompt an error message.
            wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
            //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
            MessageBox(NULL, szErrorMsg, "InternetOpenUrl failure!", 
                                                            MB_ICONEXCLAMATION); 
            if (hInternetSession) InternetCloseHandle(hInternetSession);

            return FALSE;
        } // end of InternetOpenUrl, hFile
    }
    else
    {
        wsprintf(szErrorMsg, "Last Error: %d",  GetLastError());
        //if (g_bEnableDebug) App_Debug_FunctionCallTrace(szErrorMsg);
        MessageBox(NULL, szErrorMsg, "InternetOpen failure!", 
                                                            MB_ICONEXCLAMATION);
        return FALSE;
    } // end of InternetOpen, hInternetSession

    if (lpOutBuffer) delete lpOutBuffer;
    if (hFile) InternetCloseHandle(hFile);
    if (hInternetSession) InternetCloseHandle(hInternetSession);

// Fom MSB's UpdatesWindow.cpp:
//	
//	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
//	{
//		AfxMessageBox("Failed to initialize WinInit!");
//		return FALSE;
//	}
//	else
//	{
//		// To avoid returning erroneous Updates.xml from server several 
//		// attempts to connect are made. They are limited to nMaxConnAttempts. 
//		// Each attempt recreates CInternetSession which was closed by 
//		// previous unsuccessful attempt. Typically connection error occurs 
//		// on reboot or after some period of inactivity. In this case, the 
//		// string beginning with "<!DOCTYPE ..." is returned, however the 
//		// 2nd connect was always OK. This is why I make additional attempts 
//		// programmatically. The issue is known (internet), the workaround 
//		// is mine. Each attempt starts for the label:
//        retry_session: 
//
//		nConnAttemptsCount++;
//		if (nConnAttemptsCount > nMaxConnAttempts) 
//			                                    throw "Cannot load Updates.xml";
//	    pSession = new CInternetSession("MSB 2.2", 1, 
//			   INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_DONT_CACHE);
//
//		if (!pSession)
//		{
//			AfxMessageBox("Session creation aborted!");
//			return FALSE;
//		}
//		else
//		{
//			try
//			{
//				// TODO: when myscreensaverbuilder.com is registered, use its
//				// Updates.xml, switch to synaptex.tripod.con if not available.
//
//				// Construct URL string  for retrieving Updates.xml file:
//				lstrcpy(szRemoteFileURL, "http://www.myscreensaverbuilder.com");
//				lstrcat(szRemoteFileURL, "/Downloads/MSB/Image_Sets/");
//			    lstrcat(szRemoteFileURL, "Updates.xml");
//
//                // Open remote file (Updates.xml) using HTTP protocol (OpenURL
//				// automatically determines what protocol to use based on prefix
//				// (http:// in this case). In case of HTTP, it returns CHttpFile
//				// (CHttpFile's base is CInternetFile, CInternetFile's base is
//				// CStdioFile, CStdioFile's base is CFile).
// 				file = pSession->OpenURL(szRemoteFileURL, 
//					1 /* LL not mandatory */,
//				          INTERNET_FLAG_TRANSFER_BINARY /* LL not mandatory */);
//				if (file != NULL)
//				{
//					// Make sure that this is the Updates.xml file and not an 
//					// error file. In fact, the server might return 
//					// "<!DOCTYPE ..." error file even though correct 
//					// Updates.xml exists and CFile's GetFileName returns  
//					// Updates.xml. This can occur during first connection 
//					// attempt for unknown reason (see also handling in Win32 
//					// version). Thus, the IsValidRemoteUpdatesFile() function 
//					// checks if the first line of Updates.xml coincides with 
//					// "<MyScreenSaver_Builder>". If the file name is not 
//					// "Updates.xml", make no parsing from internet file and 
//					// show updates info from cache if available and otherwise 
//					// the standard BMP header. 
//
//					// Load the contents of internet's Updates.xml into the 
//				    // buffer. In most cases it should be true Updates.xml file,
//					// but it can be also an error file. Anyway the full content 
//					// is read to buffer once to avoid the problem arising from
//					// impossibility (how-to obscurity) of moving file pointer on 
//					// server back if one need to re-read the file (incomplete 
//					// caching of Updates.xml in MSB 2.0 repaired in 2.1).
//					nBytesRead = file->Read(szRemoteFile, 
//							     1000 /* must be enough for Update.xml size */);
// 					                            szRemoteFile[nBytesRead] = '\0';
//					if (IsValidRemoteUpdatesFile(szRemoteFile))
//					{
//						// Store (cache) Updates.xml locally overwriting existing 
//						// if present. 
//						CacheUpdatesFile(szRemoteFile);
//
//						// Parse the Updates.xml file with XML parser and show
//						// results in updates window. 
//						ParseUpdatesFile();
//
//						::RedrawWindow(g_pPseudoThis->m_hWnd,  
//							NULL /* invalidate entire client area */,
//							NULL /* invalidate entire client area */, 
//															    RDW_INVALIDATE);
//					}
//					else
//					{
//  						if (file) file->Close();
//						if (pSession) pSession->Close();
//						if (theApp.m_bEnableDebug) 
//							theApp.App_Debug_FunctionCallTrace(
//							                   "The Updates.xml is not valid!");
//						goto retry_session;
//					}
//				}
//				else
//				{
//                  // TODO: make handler when OpenURL returns error (probably 
//				  // parse from Updates.xml).
//				}
//			}
//			catch (CInternetException* pEx)
//			{
//				pEx->ReportError(MB_ICONEXCLAMATION); 
//				pEx->Delete();
//				// TODO: make handler when CInternetException occurs (probably 
//				// parse from Updates.xml).
//			}
//		}
//		
//		// Close file (Updates.xml or error file) if OpenURL succeeded.
//        if (file != NULL) file->Close();
//
//		if (pSession != NULL) // cautious overkill
//		{
//			pSession->Close();
//			delete pSession;
//		}
//	}
//
    return 0;
}

// Notes: The valid CE_Updates.xml must have <Car_Explorer> tag. The function 
//        returns FALSE if the tag is not found. 

//        TODO: check validity of this note for Win32:
//        In fact, the server can return "<!DOCTYPE ..." error file even though 
//        correct CE_Updates.xml exists and CFile's GetFileName returns  
//        CE_Updates.xml.     
BOOL IsValidRemoteUpdatesFile(string sBuffer)
{
    if (sBuffer.find("<Car_Explorer>" /* case-sensitive */) 
                                                          != -1 /* not found */)
    {
        return TRUE;
    }

    return FALSE;
}

void ParseUpdatesFile()
{
    string sTagValue;		
    
    sTagValue = ReadXMLValue("Text1");
    g_sText1 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor1");
    g_sTextColor1 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea1");
    g_sTextArea1 = sTagValue.c_str();

    sTagValue = ReadXMLValue("Text2");
    g_sText2 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor2");
    g_sTextColor2 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea2");
    g_sTextArea2 = sTagValue.c_str();

    sTagValue = ReadXMLValue("Text3");
    g_sText3 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor3");
    g_sTextColor3 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea3");
    g_sTextArea3 = sTagValue.c_str();

    sTagValue = ReadXMLValue("Text4");
    g_sText4 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor4");
    g_sTextColor4 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea4");
    g_sTextArea4 = sTagValue.c_str();

    // Line 3.
    sTagValue = ReadXMLValue("Text5");
    g_sText5 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor5");
    g_sTextColor5 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea5");
    g_sTextArea5 = sTagValue.c_str();

    sTagValue = ReadXMLValue("Text6");
    g_sText6 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextColor6");
    g_sTextColor6 = sTagValue.c_str();
    sTagValue = ReadXMLValue("TextArea6");
    g_sTextArea6 = sTagValue.c_str();

    // Copy initials positions to dynamic positions.
    g_sTextArea1_Dyn = g_sTextArea1;
    g_sTextArea2_Dyn = g_sTextArea2;
    g_sTextArea3_Dyn = g_sTextArea3;
    g_sTextArea4_Dyn = g_sTextArea4;
    g_sTextArea5_Dyn = g_sTextArea5;
    g_sTextArea6_Dyn = g_sTextArea6;
}

// NOTES: the function is a copy from LoadImageSets.
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

// Based on RT v1.0's function (almost exact).  TODO: CStdioFile* also can be used, remove
// (initial idea was to use CStdioFile reading CE_Updates.xml top-to-bottom several times, but
// this dows not work since setting file pointer does not work, so I alway cache before parsing).
//string CUpdatesWindow::ReadXMLValue(CStdioFile* updates_file, char* pszTagName /* always ANSI*/)
//string CUpdatesWindow::ReadXMLValue(CInternetFile* updates_file, char* pszTagName /* always ANSI*/)
//string CUpdatesWindow::ReadXMLValue(char* pszTagName /* always ANSI*/)
string ReadXMLValue(char* pszTagName /* always ANSI*/)
{
    TCHAR szAppDataDir[MAX_PATH];
    char szLine[100];
    char szFullFileName[MAX_PATH];     // argument of open is ANSI string
    //CString strLine;
    string sLine, sLeftTrimmedLine, sParameter;
    string sOpenTag;
    string sTagName, sTagValue;

    sTagName = pszTagName;
    sOpenTag = "<" + sTagName + ">";

    ifstream updates_file;
    ////CStdioFile updates_file;
    //wsprintf(szFullFileName, "%s%s", theApp.m_strMSBDataDirectory, 
    //	                                                       "\\Updates.xml");
    // TODO: check if "Car Explorer" folder exists (created with WPFV).
    // Make full file name for CE_Updates.xml.
    ExpandEnvironmentStrings("%appdata%", szAppDataDir, sizeof(szAppDataDir)); 
    wsprintf(szFullFileName, "%s%s%s", szAppDataDir, "\\Car Explorer",
                                                            "\\CE_Updates.xml");
    //MessageBox(0, szFullFileName, "", 0);
    
    //updates_file.Open(szFullFileName, CFile::modeRead);
    updates_file.open(szFullFileName);

    if (updates_file) 
    {
        while (updates_file.good())
        //while (updates_file->ReadString(strLine))
        {
            updates_file.getline(szLine, MAX_PATH);
            //sLine = strLine;
            sLine = szLine;

            if (sLine.find(sOpenTag) !=  -1)
            {
                // Left-trim line found out of spaces if they exist, leave 
                // original line as it is if spaces are not found.
                sLeftTrimmedLine = 
                             sLine.substr(sLine.find_first_not_of(" ")).c_str();

                // Extract tag value.
                sTagValue =  sLeftTrimmedLine.substr(sOpenTag.length(), 
                           sLeftTrimmedLine.length() - 2*sOpenTag.length() - 1);
            }
        }
    }
  
    return sTagValue; 
}

// Based on RT v1.0's function.
void ParseToDecColorValues(char* pszColorValue /* in */, 
            long& iRed /* out */, long& iGreen /* out */, long& iBlue /* out */)
{
    string sColorValue;
    string sHexRed, sHexGreen, sHexBlue;

    sColorValue = pszColorValue;

    sHexRed =  sColorValue.substr(1 /* zero-based start */, 
                                                       2 /* number of chars */);
    sHexGreen =  sColorValue.substr(3, 2);
    sHexBlue =  sColorValue.substr(5, 2);

    iRed = strtol(sHexRed.c_str(), '\0', 16 /* hex base */);
    iGreen = strtol(sHexGreen.c_str(), '\0', 16);
    iBlue = strtol(sHexBlue.c_str(), '\0', 16);
}

// Purpose: parses the string like 30,5,100,20 into RECT value. The string
//          to be parsed must not contain spaces.
void ParseToRectValues(char* pszTextArea /* in */, 
                                                         RECT& rcText /* out */)
{
    string sTextArea;
    string sLeft, sTop, sRight, sBottom;
    int nPosStart, nPosEnd, nCharNumber; 

    sTextArea = pszTextArea;
    
    nPosEnd = sTextArea.find(",");
    nCharNumber = nPosEnd;
    sLeft =  sTextArea.substr(0 /* zero-based start */, nCharNumber);

    nPosStart = nPosEnd + 1;
    nPosEnd = sTextArea.find(",", nPosStart);
    nCharNumber = nPosEnd - nPosStart;
    sTop =  sTextArea.substr(nPosStart /* zero-based start */, nCharNumber);

    nPosStart = nPosEnd + 1;
    nPosEnd = sTextArea.find(",", nPosStart);
    nCharNumber = nPosEnd - nPosStart;
    sRight =  sTextArea.substr(nPosStart /* zero-based start */, nCharNumber);

    nPosStart = nPosEnd + 1;
    sBottom =  sTextArea.substr(nPosStart /* zero-based start */);

    //AfxMessageBox(sLeft.c_str());
    //AfxMessageBox(sTop.c_str());
    //AfxMessageBox(sRight.c_str());
    //AfxMessageBox(sBottom.c_str());

    rcText.left = atol(sLeft.c_str());
    rcText.top = atol(sTop.c_str());
    rcText.right = atol(sRight.c_str());
    rcText.bottom = atol(sBottom.c_str());
}

void CacheUpdatesFile(LPCTSTR pszUpdatesFile)
{
    TCHAR szFullFileName[MAX_PATH];  // FFN for CE_Updates.xml
    TCHAR szAppDataDir[MAX_PATH];
    string sResult;
    HANDLE hUpdatesFile;
    DWORD dWritten;
    DWORD dwOffset; 

    sResult = pszUpdatesFile;

    // Make full file name for CE_Updates.xml.
    ExpandEnvironmentStrings("%appdata%", szAppDataDir, sizeof(szAppDataDir)); 
    wsprintf(szFullFileName, "%s%s%s", szAppDataDir, "\\Car Explorer",
                                                            "\\CE_Updates.xml");

    // Re-write CE_Updates.xml file completely: delete, then write again.
    DeleteFile(szFullFileName);
    hUpdatesFile = CreateFile(szFullFileName, GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /* security - not used*/,          
        OPEN_ALWAYS /* if exists - just open; if does not exist, create it */,
                                  FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

    // Tell the writer we need to write at the beginning of the file.
    dwOffset = SetFilePointer(hUpdatesFile, 0, NULL, FILE_BEGIN);

    // [As in RTP v1.0] using string might help to avoid a *buffer overrun* (in 
    // contrast to using TCHAR) when the size of resulting string is bigger than 
    // declared in TCHAR.
    WriteFile(hUpdatesFile, sResult.c_str(), strlen(sResult.c_str()), &dWritten, 
                                                                          NULL);
    CloseHandle(hUpdatesFile);
}

BOOL IsCachedUpdatesFileExists()
{
    TCHAR szFullFileName[MAX_PATH];  // FFN for ImageSetsList_v1.1.txt
    TCHAR szAppDataDir[MAX_PATH];
    HANDLE hUpdatesFile;

    // TODO: check if "Car Explorer" folder exists (created with WPFV).
    // Make full file name for CE_Updates.xml.
    ExpandEnvironmentStrings("%appdata%", szAppDataDir, sizeof(szAppDataDir)); 
    wsprintf(szFullFileName, "%s%s%s", szAppDataDir, "\\Car Explorer",
                                                            "\\CE_Updates.xml");
    //MessageBox(0, szFullFileName, "", 0);
                                                          
    hUpdatesFile = CreateFile(szFullFileName, GENERIC_READ, 
        FILE_SHARE_READ, NULL /* security - not used*/,          
        OPEN_EXISTING /* opens file if exists, fails if does not */,
                                  FILE_ATTRIBUTE_NORMAL /* normal use */, NULL);

    if (hUpdatesFile == INVALID_HANDLE_VALUE) // UCE_pdates.xml does not exist
    {
        return FALSE;
    }
    else
    {
        CloseHandle(hUpdatesFile);
        return TRUE;
    }
}