#ifdef COMPILED_AS_DLL

#define FUNCTION_PREFIX __declspec( dllexport )
#pragma message( "------------------------------------------------------------------------" )
#pragma message( "                OK - COMPILED_AS_DLL - is defined!!!                    " )
#pragma message( "------------------------------------------------------------------------" )
#else
#define FUNCTION_PREFIX  __declspec( dllimport )
#pragma message( "------------------------------------------------------------------------" )
#pragma message( "             Warning - COMPILED_AS_DLL - is NOT defined!!!              " )
#pragma message( "------------------------------------------------------------------------" )

#endif

// When  data are to be used externally (out of DLL), they must be exported 
// with __declspec(dllexport), and imported with __declspec(dllimport) 
// [Newcomer: p.1130]. Than I use the same trick, albeit LL it is possible to
// avoid using COMPILED_AS_DLL (_WINDLL, _WINDOWS). 

// General initialization variables and functions.
FUNCTION_PREFIX int iOffsetLeft, iOffsetTop;
FUNCTION_PREFIX void DLL_InitCustomUI(HINSTANCE hInst, HWND hWnd);
FUNCTION_PREFIX void DLL_DestroyCustomUI();
FUNCTION_PREFIX void MoveControl(int iOffsetLeft);

//// Handles to canvases.
//FUNCTION_PREFIX HWND hwndRotationCanvas;  // probably do not export
//FUNCTION_PREFIX HWND hwndSizeCanvas;      // used with CheckRadioButton externally 
//FUNCTION_PREFIX HWND hwndPositionCanvas;  // used with CheckRadioButton externally 
//FUNCTION_PREFIX HWND hwndAnimationCanvas; // probably do not export
//FUNCTION_PREFIX HWND hwndColorsCanvas;    // probably do not export
