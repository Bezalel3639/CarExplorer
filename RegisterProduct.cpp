#include "stdafx.h"
#include "RegisterProduct.h"

// Notice that the use of extern "C" (mere extern does not work) makes possible 
// not even include the relative headers (GenericDecode.h and BigInt.h headers) 
// anywhere directly (the files are merely included in project). In properties 
// for BigInt.c and GenericDecode.c the use of precompiled headers is excluded.
extern "C" int decodeGenericRegCode(char* , char*);
extern "C" void freememory(void);
extern "C" char* getUserSeed(void);
extern "C" char* getConstant(void);
extern "C" char* getSequenceNumber(void);
extern "C" char* getDate(void);
extern "C" char* getDebugMessage(void);
extern "C" void setUserName(char*);
extern "C" void setUserEmail(char*);
extern "C" void createUserSeed(void);
extern "C" char *getCreatedUserSeed(void);

BOOL CRegisterProduct::IsValidRC(char* email, char* regCode)
{
    int value, result;
    
    //Fixed generic ACG configuration string (configuration manually and named smalevannyMySS)
    char* config = "SUPPLIERID:smalevanny%E:4%N:1%H:1%COMBO:ee%SDLGTH:20%CONSTLGTH:2%CONSTVAL:AC%SEQL:2%ALTTEXT:Contact support@myscreensaverbuilder.com to obtain your registration code.%SCRMBL:U12,,U0,,D3,,S0,,U3,,U2,,U10,,U15,,U1,,U5,,D2,,U9,,U4,,U6,,U11,,U19,,S1,,U18,,C0,,U8,,U13,,U16,,U14,,U7,,D1,,U17,,C1,,D0,,%ASCDIG:2%MATH:2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,2A,,%BASE:26%BASEMAP:XHDUAVJOFWEYRGMBZNTKQCILSP%REGFRMT:MSB2-^#####-#####-#####-#####-#####-#####-#####-#####[-#]";
    // Obtain seed from registration code provided.
    value = decodeGenericRegCode(config, regCode);
    
    // Create seed from user data (email).
    setUserEmail(email);
    createUserSeed();

    //CString str; 
 //   str.Format("%s, %s", getUserSeed(), getCreatedUserSeed()); 
 //   //AfxMessageBox(str); // LAMBERT1791GMAILCOML and  LAMBERT1791GMAILCOML

    // These two seeds must be the same for successful activation.
    result = strcmp(getUserSeed(), getCreatedUserSeed());
    if (result == 0)
    {
        //AfxMessageBox("Successful activation");
        return TRUE;
    }
    else
    {
        //AfxMessageBox("Activation failed!");
        return FALSE;
    }
}

BOOL CRegisterProduct::IsRegistered()
{
    HKEY hKey;
    DWORD dwResult;
    DWORD dwType = REG_SZ; 
    TCHAR szEncryptedUser[MAX_PATH];
    TCHAR szUser[100]; 
    TCHAR szEncryptedRegCode[MAX_PATH];
    TCHAR szRegCode[200]; 
    DWORD dwLength = 200;

    // Open CE registry key.
    dwResult = RegOpenKey(APP_BASE_HIVE, REGKEY_CE, &hKey); 

    if (dwResult != ERROR_SUCCESS) return FALSE;
    dwResult = RegQueryValueEx(hKey, "RegKey",  NULL, &dwType, 
                                          (BYTE*)szEncryptedRegCode, &dwLength);
    DecryptString(szRegCode, szEncryptedRegCode);
    //MessageBox(0, "", szRegCode, 0);

    // If product is not registered, "RegKey" does not exist, return FALSE
    // therefore.
    if (dwResult != ERROR_SUCCESS ) return FALSE; //RegCloseKey(hKey);

    // Return also FALSE immediately when RegKey is empty. 
    if (lstrlen(szRegCode) == 0) return FALSE;
    
    dwType = REG_SZ; 
    dwLength = 100;

    dwResult = RegQueryValueEx(hKey, "User", NULL, &dwType, 
                                             (BYTE*)szEncryptedUser, &dwLength);
    DecryptString(szUser, szEncryptedUser);
    //MessageBox(0, szRegCode, szUser, 0);

    // If product is not registered, "User" does not exist, return FALSE 
    // therefore.
    if (dwResult != ERROR_SUCCESS ) return FALSE; RegCloseKey(hKey);

    // Return also FALSE immediately when User is empry. 
    if (lstrlen(szUser) == 0) return FALSE;

    // Finally, verify validity of User, RegCode pair.
    if (IsValidRC(szUser, szRegCode))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//------------------------------------------------------------------------------
// Method: Date of creation the install directory "Program Files\MSB" is
//         compared with system date. Note that we are not using sss.scr date 
//         of creation intentionally because setup.exe ships this module with
//         build date, which is not changed on a user machine to copy (install)
//         date.
//------------------------------------------------------------------------------

BOOL CRegisterProduct::IsTrialUseExpired()
{
    int nCARFilesCount;
    int nMSSFilesCount;
    int nCollectionFilesCount;

    nCARFilesCount = GetCollectionFilesCount("car"); 
    nMSSFilesCount = GetCollectionFilesCount("mss"); 
    nCollectionFilesCount = nCARFilesCount + nMSSFilesCount;

    TCHAR szText[100];
    wsprintf(szText, "%d %d", nCARFilesCount, nMSSFilesCount);
    //MessageBox(0, szText, "", 0); 

    if (nCollectionFilesCount > COLLECTIONS_USE_LIMIT)
        return TRUE;
    else
        return FALSE;
}

BOOL CRegisterProduct::RegisterProduct(LPCTSTR pszEmail, LPCTSTR pszRegCode)
{
    HKEY hKey;
    DWORD dwResult;

    //--------------------------------------------------------------------------
    // Open MyScreenSaver key
    //--------------------------------------------------------------------------

    //dwResult = RegOpenKey(theApp.m_hComponentBaseHive,
    //	                                         theApp.m_strRegKeySCfg, &hKey);

    //if(dwResult != ERROR_SUCCESS ) 
    //	   AfxMessageBox("CRegisterProduct::RegisterProduct error: RegOpenKey");

    //
    //// Use TCHAR with RegSetValueEx (CString produce gibberish symbols)
    //TCHAR szRC[200];
    //_tcscpy(szRC, pszRegCode);

    //dwResult = RegSetValueEx(hKey, _T("RegKey"), 0, REG_SZ, (LPBYTE)&szRC, 200);		     

    //if(dwResult != ERROR_SUCCESS ) 
    //	AfxMessageBox("CRegisterProduct::RegisterProduct error: RegSetValueEx");

    //RegCloseKey(hKey); 

 //   //--------------------------------------------------------------------------
    //// Open MyScreenSaver/Application key
    ////--------------------------------------------------------------------------

    //dwResult = RegOpenKey(theApp.m_hComponentBaseHive,
    //	                                  theApp.m_strRegKeyApplication, &hKey);

    //if(dwResult != ERROR_SUCCESS ) 
    //	   AfxMessageBox("CRegisterProduct::RegisterProduct error: RegOpenKey");

    //
    //// Use TCHAR with RegSetValueEx (CString produce gibberish symbols)
    //TCHAR szUser[100];
    //_tcscpy(szUser, pszEmail); 
    //
    //dwResult = RegSetValueEx(hKey, _T("User"), 0, REG_SZ, (LPBYTE)&szUser, 100);

    //if(dwResult != ERROR_SUCCESS ) 
 //       AfxMessageBox("CRegisterProduct::RegisterProduct error: RegSetValueEx");

    //RegCloseKey(hKey); 
    
    return FALSE;
}

//------------------------------------------------------------------------------
// Description: returns TRUE if msbuse.log exists, and FALSE if not.
//------------------------------------------------------------------------------

BOOL CRegisterProduct::IsAntiHackingEnabled()
{
    //CFile file;
    //TCHAR szWinDir[100];

    //ExpandEnvironmentStrings("%appdata%", szWinDir, sizeof(szWinDir)); 
    //SetCurrentDirectory(szWinDir);

    //if(file.Open(_T("msbuse.log"), CFile::modeRead))
    //{
    //	return TRUE;
    //}
    //else
    {
        return FALSE;
    }
}

void CRegisterProduct::EnableAntiHacking()
{
    TCHAR szWinDir[100];
    ExpandEnvironmentStrings("%appdata%", szWinDir, sizeof(szWinDir)); 
    SetCurrentDirectory(szWinDir);
    
    //// Create msbuse.log if not exists, do nothing if exists.
    //CFile file;

    //// If msbuse.log exists do nothing.
    //if(!file.Open(_T("msbuse.log"), CFile::modeRead))
    //{
 // 		file.Open(_T("msbuse.log"), CFile::modeCreate);
    //	file.Close(); // prevent sharing violation by the way

    //	// Set msbuse.log file attributes to hidden, read-only (no sense really).
    //	CFileStatus status;
    //	CFile::GetStatus(_T("msbuse.log"), status);
    //	status.m_attribute = CFile::Attribute::hidden | CFile::Attribute::readOnly;
    //	CFile::SetStatus(_T("msbuse.log"), status );
    //}
}

int CRegisterProduct::GetCollectionFilesCount(LPCTSTR pszExt) 
{
    TCHAR szSearchString[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    int nFilesCount = 0;
    TCHAR szText[100];

    wsprintf(szSearchString, "%s\\*.%s", g_szCarFilesFolder, pszExt);

    // FindFirstFile, FindNextFile.
    hFind = FindFirstFile(szSearchString , &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        nFilesCount++;

        while (FindNextFile(hFind , &fd))
        {
            nFilesCount++;
        }
        FindClose(hFind);
    }

    return nFilesCount;
}
