#pragma once

void App_Settings();
LRESULT CALLBACK PropSheetProc(HWND hDlg, UINT message, LPARAM lParam);
LRESULT CALLBACK PageDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK Page1DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK Page2DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CenterWindow(HWND hWnd, BOOL bAppRelative = TRUE);	                                      
							                                      
							                                     