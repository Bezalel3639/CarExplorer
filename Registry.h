#pragma once

void LoadRegistryData();
void SaveRegistryData();

BOOL ReadTextFromRegistry(LPCTSTR pszSubKey /* in */, LPCTSTR pszName /* in */, 
                                                      LPTSTR pszData /* out */);
void StoreTextToRegistry(LPCTSTR pszSubKey, LPCTSTR pszName, LPCTSTR pszData);
//void StoreIntToRegistry(LPCTSTR pszSubKey, LPCTSTR pszName, int iData);
void DeleteFavoritesFromRegistry();

void LoadRegistryData();


