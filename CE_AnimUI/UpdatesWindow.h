#pragma once

#include <string>
#include <process.h> //_beginththredex (/ML -> /MT!)

using namespace std; // for string declaration

void OnPaint_Canvas4(HWND hWnd, LPPAINTSTRUCT pps);
void StartDownloadUpdatesInfoThread();
BOOL IsCachedUpdatesFileExists();
BOOL IsValidRemoteUpdatesFile(string sBuffer);
void ParseUpdatesFile();
void ParseToDecColorValues(char* pszColorValue /* in */, 
           long& iRed /* out */, long& iGreen /* out */, long& iBlue /* out */);
void ParseToRectValues(char* pszTextArea /* in */, RECT& rcText /* out */);
static UINT __stdcall DownloadUpdatesInfo_ThreadProc(LPVOID param);
BOOL CheckInternetConnection(LPCTSTR pszURL);
string ReadXMLValue(char* pszTagName /* always ANSI*/);
void CacheUpdatesFile(LPCTSTR pszUpdatesFile);



