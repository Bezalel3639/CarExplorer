; Include
!include "MUI.nsh" ; or "MUI2.nsh"
!include "FileFunc.nsh" ; GetFileName

Name "Car Explorer 1.03"
RequestExecutionLevel admin ; UAC requirement (Vista) to write to ProgramFiles

; Modern Interface Configuration
!insertmacro MUI_PAGE_WELCOME 
!insertmacro MUI_PAGE_LICENSE "F:\Development\CarExplorer.NET\NSIS_Install\CE_EULA.txt" 
;!insertmacro MUI_PAGE_DIRECTORY 
!insertmacro MUI_PAGE_INSTFILES 
    !define MUI_FINISHPAGE_RUN "$PROGRAMFILES\Car Explorer\CE.exe"
    !define MUI_FINISHPAGE_RUN_TEXT "Launch Car Explorer 1.03"
!insertmacro MUI_PAGE_FINISH 

; Languages
!insertmacro MUI_LANGUAGE "English"

Var CAR_FILES_FOLDER

;Set installer file name.
OutFile "CE_v1.03_Setup.exe"
InstallDir "$PROGRAMFILES\Car Explorer"
 
; Install Car Explorer application files into the install directory $INSTDIR. 
; If directory is not available it is auto-created with SetOutPath. 
Section "Application"
    ; Set target installation directory.
    SetOutPath $INSTDIR
    
    ; Enumerate files to install and install them.
    File  "F:\Development\CarExplorer.NET\Release\CE.exe"
    File  "F:\Development\CarExplorer.NET\CE_AnimUI\Release\CE_AnimUI.dll"
    File  "F:\Development\CarExplorer.NET\res\Splash_Screen.jpg"
    File  "F:\Development\CarExplorer.NET\res\AD_Picture.jpg"
    
    ; Tell the compiler to write an uninstaller and to look for a "Uninstall" 
    ; section.
    WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd 

; Include sample CAR files.
Section "Install Samples"
     ${GetFileName} $PICTURES $R0
    
    ${If} $R0 == "My Pictures"
         StrCpy $CAR_FILES_FOLDER "$DOCUMENTS\My Cars"
    ${ElseIf} $R0 == "Pictures"
        StrCpy $CAR_FILES_FOLDER "$DOCUMENTS\Cars"
    ${Else} 
        StrCpy $CAR_FILES_FOLDER "$DOCUMENTS\My Cars"
    ${EndIf}
    
    ; Set specific target for sample CAR files. Notice that SetOutPath creates 
    ; the target directory (recursively if necessary), if it does not exist.
    SetOutPath $CAR_FILES_FOLDER ; 
    
    ; Copy sample CAR files into the target directory. 
    File  "F:\Development\carexplorer.org\Downloads\CE\Collections\Nissan_2012.car"
    File  "F:\Development\carexplorer.org\Downloads\CE\Collections\Chrysler_2013.car"
    File  "F:\Development\carexplorer.org\Downloads\CE\Collections\Audi_2013.car"

SectionEnd 

Section "Create default application registry entries"
    ; Create CE root key.
    ;WriteRegStr HKCU  "Software\Car Explorer" 
    
    WriteRegStr HKCU "Software\Car Explorer" "User" ""
    WriteRegStr HKCU "Software\Car Explorer" "CarFilesFolder" $CAR_FILES_FOLDER
    WriteRegDWORD HKCU "Software\Car Explorer" "ShowToolBar" 1
    WriteRegDWORD HKCU "Software\Car Explorer" "ShowStatusBar" 1
    WriteRegDWORD HKCU "Software\Car Explorer" "ShowWelcomeView" 1
    WriteRegDWORD HKCU "Software\Car Explorer" "CarFilesLayout" 1
    WriteRegDWORD HKCU "Software\Car Explorer" "Units" 0
    WriteRegDWORD HKCU "Software\Car Explorer" "Group" 3
    
    ; Create Favorites subkey.
    ;WriteRegStr HKCU  "Software\Car Explorer\Favorites" 
SectionEnd

Section "Enable double-click on CAR files by adding entries to registry"
    ; Create CE root key.
    ;WriteRegStr HKCU  "Software\Car Explorer" 
    
    WriteRegStr HKCR ".car" "" "CARFile" ; (Default) 
    WriteRegStr HKCR "CARFile" "" "Car Explorer Collection Format" 
    WriteRegStr HKCR "CARFile\DefaultIcon" "" \
                                        "$PROGRAMFILES\Car Explorer\CE.exe,-195"
    WriteRegStr HKCR "CARFile\shell\open\command" "" \
                              "$\"$PROGRAMFILES\Car Explorer\CE.exe$\" $\"%1$\""
    
    ; Create Favorites subkey.
    ;WriteRegStr HKCU  "Software\Car Explorer\Favorites" 
SectionEnd

Section "Add/Remove Programs entries"
    ; Create Display Name.
    WriteRegStr HKCU \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\Car Explorer" \
                                                "DisplayName" "Car Explorer 1.0"
    WriteRegStr HKCU \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\Car Explorer" \
                                                           "Publisher" "MSB LLC"											
    ; Point to uninstaller.
    WriteRegStr HKCU \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\Car Explorer" \
                                "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
SectionEnd

Section "Start menu shortcuts"
    ; Create shorcuts under Start menu.
    SetShellVarContext all ; install for all accounts
    CreateDirectory "$SMPROGRAMS\Car Explorer 1.0"
    ; Notice that "$PROGRAMFILES\Car Explorer\CarExplorer.exe" also OK.
    CreateShortCut "$SMPROGRAMS\Car Explorer 1.0\Car Explorer 1.0.lnk" \
                                                               "$INSTDIR\CE.exe"
    ; To show properly HTM icon with Vista, WriteINIStr is used to create
    ; shortcut file instead of CreateShortCut, which show internet icon with XP
    ; only.
    WriteINIStr "$SMPROGRAMS\Car Explorer 1.0\Web Site.url" "InternetShortcut" \
                                                 "URL" "http://www.carexplorer.org" 
    CreateShortCut "$SMPROGRAMS\Car Explorer 1.0\Uninstall.lnk" \
                                                        "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
    ; Remove install files and relative directory.
    Delete $INSTDIR\CE.exe
    Delete $INSTDIR\CE_AnimUI.dll
    Delete $INSTDIR\Splash_Screen.jpg
    Delete $INSTDIR\AD_Picture.jpg	
    Delete $INSTDIR\Uninstall.exe
    RMDir $INSTDIR
    
    Delete "$APPDATA\Car Explorer\CE_Updates.xml"
    Delete "$APPDATA\Car Explorer\Temp.jpg"
    Delete "$APPDATA\Car Explorer\Thumbs.db"
    Delete "$APPDATA\Car Explorer\CE_TraceFile.txt"
    RMDir  "$APPDATA\Car Explorer"	

    ; Remove application registry entries (including those possibly created 
    ; while using the program). 
    DeleteRegKey HKCU "SOFTWARE\Car Explorer"
    ; Remove appwiz.cpl registry entries.
    DeleteRegKey HKCU \
              "Software\Microsoft\Windows\CurrentVersion\Uninstall\Car Explorer"
              
    ; Remove CAR file tipe registry entries.
    DeleteRegKey HKCR ".car"
    DeleteRegKey HKCR "CARFile"

    ; Remove shorcuts from Start menu.
    SetShellVarContext all ; uninstall for all accounts
    Delete "$SMPROGRAMS\Car Explorer 1.0\Car Explorer 1.0.lnk"
    Delete "$SMPROGRAMS\Car Explorer 1.0\Web Site.url"
    Delete "$SMPROGRAMS\Car Explorer 1.0\Uninstall.lnk"
    
    ;Remove install directory with its files.
    RMDIR "$SMPROGRAMS\Car Explorer 1.0"
    
    ;Remove CE application data directory.
    RMDIR "$APPDATA\Car Explorer"
SectionEnd
