#pragma once

class CRegisterProduct 
{
// Access member variables (Attributes)
public:
    short m_iTrialPeriod; //better private?

// Access member functions (Operations)
public:
    BOOL IsRegistered();
    BOOL IsValidRC(char* email, char* regCode);
    BOOL RegisterProduct(LPCTSTR pszEmail, LPCTSTR pszRegCode);

    BOOL IsTrialUseExpired();
    BOOL IsAntiHackingEnabled();
    void EnableAntiHacking();
    int GetCollectionFilesCount(LPCTSTR pszExt); 
};
