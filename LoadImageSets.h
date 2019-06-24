#pragma once

#include <vector>
#include <algorithm> // sort
#include <process.h> //_beginththredex (/ML -> /MT!)
#include "shellapi.h"

//using namespace std ; //for vector declaration

LRESULT CALLBACK LoadImageSets(HWND hDlg, UINT message, WPARAM wParam, 
                                                                 LPARAM lParam);
void OnInitDialog_Handler(HWND hwnd);
static UINT __stdcall GetPWD_ThreadProc(LPVOID param);
static UINT __stdcall ThreadProcDownloadListOfImageSets(LPVOID param);
static UINT __stdcall ThreadProcDownloadImageSet(LPVOID param);
