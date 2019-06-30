#pragma once

#include <string>
using namespace std; // for string declaration

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define COMPILED_AS_DLL

// Make visible CE_AnimUI.cpp variable with UpdatesWindow.cpp.
extern HWND g_hwndCanvas4; 

// Make extern to see UpdatesWindows.cpp variables from CE_AnimUI.cpp to enable 
// 4th canvas updates.

// Initial position positions of 4th canvas areas.
extern string g_sTextArea1; 
extern string g_sTextArea2;
extern string g_sTextArea3;
extern string g_sTextArea4;
extern string g_sTextArea5;
extern string g_sTextArea6;

// Dynamic positions of areas of 4th canvas areas.
extern string g_sTextArea1_Dyn;
extern string g_sTextArea2_Dyn;
extern string g_sTextArea3_Dyn;
extern string g_sTextArea4_Dyn;
extern string g_sTextArea5_Dyn;
extern string g_sTextArea6_Dyn;

extern BOOL g_bCanvas4Contracting;
