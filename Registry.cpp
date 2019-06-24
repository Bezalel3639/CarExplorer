#include "stdafx.h"
#include "Registry.h"

//#define	APP_BASE_HIVE HKEY_CURRENT_USER
//#define	REGKEY_CE "SOFTWARE\\Car Explorer"
//#define	REGKEY_CE_FAVORITES "SOFTWARE\\Car Explorer\\Favorites"

BOOL ReadTextFromRegistry(LPCTSTR pszSubKey /* in */, LPCTSTR pszName /* in */, 
                                                       LPTSTR pszData /* out */)
{
    HKEY hKey;
    DWORD dwValue, dwResult;
    TCHAR szBuf[MAX_REG_STRING];
    DWORD dwLength = MAX_REG_STRING;
    DWORD dwType = REG_SZ;

    dwResult = RegCreateKey(APP_BASE_HIVE, pszSubKey, &hKey);

    if (dwResult != ERROR_SUCCESS)  
    {
        MessageBox(NULL, "RegCreateKey error", "StoreTextToRegistry", MB_OK);
        return FALSE;
    }

    dwResult = RegQueryValueEx(hKey, pszName, NULL, &dwType,
                                                       (BYTE*)szBuf, &dwLength);
    lstrcpy(pszData, szBuf);

    if (dwResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE; // supposedly Name entry is not found
        //MessageBox(NULL, "RegQueryValueEx", "ReadTextFromRegistry", MB_OK);
    }

    RegCloseKey(hKey);

    return TRUE;
}

// NOTES: The registry key Favorites is recreated for simplicity. One 
//        alternative would be to pass number of items under Favorites, delete 
//        one-by-one as in v1.01, but this number (size of favorites map) can be
//        less than actual number of items (favorites items could have been 
//        deleted manually), and hence not acceptable in this simple version. 
//        Another option with RegEnuKeysEx is cumbersome and probably risky 
//        with different OS (the key must be opened with 
//        KEY_ENUMERATE_SUB_KEYS).

void DeleteFavoritesFromRegistry()
{
    HKEY hKey;
    DWORD dwResult;

    dwResult = RegDeleteKey(APP_BASE_HIVE, REGKEY_CE_FAVORITES);
    if (dwResult != ERROR_SUCCESS)
        MessageBox(NULL, "Favorites cannot be deleted!", 
                                              "DeleteFavoritesFromRegistry", 0);
    dwResult = RegCreateKey(APP_BASE_HIVE, REGKEY_CE_FAVORITES, &hKey);
    if (dwResult != ERROR_SUCCESS)
        MessageBox(NULL, "An error while recreating favorites registry key!", 
                                              "DeleteFavoritesFromRegistry", 0);
     RegCloseKey(hKey); 
}

void StoreTextToRegistry(LPCTSTR pszSubKey, LPCTSTR pszName, LPCTSTR pszData)
{
    HKEY hKey;
    DWORD dwValue, dwResult;

    dwResult = RegCreateKey(APP_BASE_HIVE, pszSubKey, &hKey);

    if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
                            "RegCreateKey error", "StoreTextToRegistry", MB_OK);

    // Save string to registry.
    dwResult = RegSetValueEx(hKey, pszName, 0, REG_SZ, 
                                         (LPBYTE)pszData, _tcslen(pszData) + 1);

    if (dwResult != ERROR_SUCCESS) MessageBox(NULL,
                                 "RegSetValueEx", "StoreTextToRegistry", MB_OK);

    RegCloseKey(hKey); 
}

// Reserved (not used): TODO: remove is not used.
//void StoreIntToRegistry(LPCTSTR pszSubKey, LPCTSTR pszName, int iData)
//{
//	HKEY hKey;
//	DWORD dwValue, dwResult;
//
//	// Open the key.
//	dwResult = RegCreateKey(APP_BASE_HIVE, pszSubKey, &hKey);
//
//	// Persist data only if relative key exists.
//	if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
//		                     "RegCreateKey error", "StoreIntToRegistry", MB_OK);
//
//	// Save DWORD to registry.
//	dwValue = (DWORD)iData;
//	dwResult = RegSetValueEx(hKey, pszName, 0, REG_DWORD, 
//						                     (LPBYTE)&dwValue, sizeof(dwValue));
//
//	if (dwResult != ERROR_SUCCESS) MessageBox(NULL,
//	             "RegSetValueEx", "StoreIntToRegistry", MB_OK | MB_ICONWARNING);
//
//	RegCloseKey(hKey); 
//}

void LoadRegistryData()
{
    // Parameters found in registry override default parameter values set in 
    // in InitInstance.

    HKEY hKey;
    DWORD dwType, dwLength, dwResult;
    DWORD dwValue;
    TCHAR szBuffer[MAX_PATH];

    // Open the root CE key.
    dwResult = RegOpenKey(APP_BASE_HIVE, REGKEY_CE, &hKey);

    // Read initial data only if relative key exists.
    if (dwResult == ERROR_SUCCESS )  
    {
        // Get CAR files folder location.
        dwType = REG_SZ; 
        dwLength = MAX_PATH; // TODO: MAX_PATH + 1?
        dwResult = RegQueryValueEx(hKey, "CarFilesFolder", NULL, &dwType,
                                                    (BYTE*)szBuffer, &dwLength);
        if (dwResult == ERROR_SUCCESS && lstrlen(szBuffer) != 0)
            lstrcpy(g_szCarFilesFolder, szBuffer);
        else
        {
            // Do nothing if missing i.e. use CAR_FILES_FOLDER. TODO: verify logic.
            //MessageBox(NULL, "RegQueryValueEx error: CarFilesFolder", 
            //                              "LoadRegistryData()", MB_ICONWARNING);
        }

          // Get g_bShowStartupWelcomeView.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "ShowWelcomeView", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        if (dwResult == ERROR_SUCCESS)
            //g_bDo_Not_ShowWelcomeView = (dwValue ? FALSE : TRUE);
            g_bShowStartupWelcomeView = (BOOL)dwValue;
        else 
            // If "ShowWelcomeView" is missing in the registry set default to 
            // show the welcome view.
            if (dwResult != ERROR_SUCCESS /* succeeds */) 
                                               g_bShowStartupWelcomeView = TRUE;
        
        // Get g_bShowToolBar.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "ShowToolBar", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        // Notice that if "ShowToolBar" is missing the hardcoded value will be 
        // used.
        if (dwResult == ERROR_SUCCESS) g_bShowToolBar = (BOOL)dwValue;

        // Get g_bShowStatusBar.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "ShowStatusBar", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        // Notice that if "ShowStatusBar" is missing the hardcoded value will be 
        // used.
        if (dwResult == ERROR_SUCCESS) g_bShowStatusBar = (BOOL)dwValue;

        // Get g_nCarFilesLayout.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "CarFilesLayout", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        if (dwResult == ERROR_SUCCESS) g_nCarFilesLayout = (int)dwValue;

        // Get g_iSortParam.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "SortParam", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        // Notice that if "SortParam" is missing the hardcoded value will be 
        // used.
        if (dwResult == ERROR_SUCCESS) g_iSortParam = (int)dwValue; // g_nSortParam 

        // Get g_iUnits.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "Units", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        if (dwResult == ERROR_SUCCESS) g_iUnits = (int)dwValue;

        // Get g_nGroup.
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "Group", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        if (dwResult == ERROR_SUCCESS) g_nGroup = (int)dwValue;

        // Get EnableDebug and set g_bEnableDebug (fragment from MSB).
        dwType = REG_DWORD;
        dwLength = sizeof(DWORD);
        dwResult = RegQueryValueEx(hKey, "EnableDebug", 0, &dwType, 
                                                   (LPBYTE)&dwValue, &dwLength);
        // By default EnableDebug is not available in the registry. Set result 
        // to zero in this case.
        if (dwResult != ERROR_SUCCESS /* succeeds */) dwValue = 0;

        // The debugging mode switch g_bEnableDebug is composed, in effect, from 
        // hard-coded g_bEnableDebug (set now to TRUE) and EnableDebug's boolean 
        // value from registry.
        g_bEnableDebug  = g_bEnableDebug && (BOOL)dwValue;
    }

    RegCloseKey(hKey); 
}

void SaveRegistryData()
{
    HKEY hKey;
    DWORD dwValue, dwResult;
    //MessageBox(0, "", "", 0);

    // Open the root CE key.
    dwResult = RegOpenKey(APP_BASE_HIVE, REGKEY_CE, &hKey);

    // Persist data only if relative key exists.
    if (dwResult == ERROR_SUCCESS )  
    {
        // Save g_bShowStatusBar.
        dwResult = RegSetValueEx(hKey, "ShowStatusBar", 0, REG_DWORD,
                                    (LPBYTE)&g_bShowStatusBar, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: ShowStatusBar", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);
        // Save g_bShowToolBar.
        dwResult = RegSetValueEx(hKey, "ShowToolBar", 0, REG_DWORD,
                                      (LPBYTE)&g_bShowToolBar, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: ShowToolBar", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);

        // Save g_bShowWelcomeView.
        dwResult = RegSetValueEx(hKey, "ShowWelcomeView", 0, REG_DWORD,
                           (LPBYTE)&g_bShowStartupWelcomeView, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: ShowWelcomeView", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);

        // Save g_nCarFilesLayout.
        dwResult = RegSetValueEx(hKey, "CarFilesLayout", 0, REG_DWORD,
                                   (LPBYTE)&g_nCarFilesLayout, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: CarFilesLayout", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);

        // Save g_iSortParam.
        if (g_iSortParam != -1 /* no sort */) // "SortParam" is absent on 
            // install, create on the fly is sort was used
        {
            dwResult = RegSetValueEx(hKey, "SortParam", 0, REG_DWORD,
                                        (LPBYTE)&g_iSortParam, sizeof(dwValue));

            if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
                "RegSetValueEx: SortColumn", "SaveRegistryData()", 
                                                            MB_OK | MB_ICONWARNING);
        }

        // Save g_iUnits.
        dwValue = (DWORD)g_iUnits;
        dwResult = RegSetValueEx(hKey, "Units", 0, REG_DWORD,
                                             (LPBYTE)&dwValue, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: Units", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);
        // Save g_nGroup.
        dwValue = (DWORD)g_nGroup;
        dwResult = RegSetValueEx(hKey, "Group", 0, REG_DWORD,
                                             (LPBYTE)&dwValue, sizeof(dwValue));

        if (dwResult != ERROR_SUCCESS) MessageBox(NULL, 
            "RegSetValueEx: Units", "SaveRegistryData()", 
                                                        MB_OK | MB_ICONWARNING);
    }

    RegCloseKey(hKey); 
}




