!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "UAC.nsh"

; ==== ZÁKLADNÍ INFO (uprav podle sebe) ====
!define APP_NAME     "Digitizer2"
!define COMPANY      "LiborSoft"
!define APP_VERSION  "1.0.0"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "digitizer2-setup-${APP_VERSION}.exe"
InstallDir "$PROGRAMFILES\${COMPANY}\${APP_NAME}"
RequestExecutionLevel user
SetCompressor /SOLID lzma

Var InstallScope
Var RadioAll
Var RadioCurrent
Var Arm1Length
Var Arm2Length

; ==== STRÁNKY ====
Page custom InstallScopePage InstallScopePageLeave
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; ==== FUNKCE: Načti arms.cfg a zapiš délky ramen do registru ====
Function WriteArmLengths
  Exch $0  ; $0 = cesta k arms.cfg
  IfFileExists "$0" 0 done
  ReadINIStr $Arm1Length "$0" "Arms" "ARM1"
  ReadINIStr $Arm2Length "$0" "Arms" "ARM2"
  ${If} $Arm1Length != ""
  ${AndIf} $Arm2Length != ""
    ${If} $InstallScope == "all"
      WriteRegStr HKLM "Software\LiborSoft\Digitizer2\Program" "arm1_length" "$Arm1Length"
      WriteRegStr HKLM "Software\LiborSoft\Digitizer2\Program" "arm2_length" "$Arm2Length"
    ${Else}
      WriteRegStr HKCU "Software\LiborSoft\Digitizer2\Program" "arm1_length" "$Arm1Length"
      WriteRegStr HKCU "Software\LiborSoft\Digitizer2\Program" "arm2_length" "$Arm2Length"
    ${EndIf}
  ${EndIf}
done:
  Pop $0
FunctionEnd

; ==== INSTALACE ====
Section "Application (required)" SecMain
  SectionIn RO

  ${If} $InstallScope == "all"
    SetShellVarContext all
  ${Else}
    SetShellVarContext current
  ${EndIf}

  ; Cíl instalace
  SetOutPath "$INSTDIR"

  ; Zkopíruj aplikaci (předpokládá se, že obsah je v ./dist)
  ; - pokud používáš jinou strukturu, cestu uprav
  File /r "dist\*.*"

  ; Zástupci
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\digitizer2.exe"
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\digitizer2.exe"

  ; --- REGISTR: načti délky ramen z arms.cfg (pokud existuje) ---
  Push "$EXEDIR\configs\arms.cfg"
  Call WriteArmLengths

  ; Pokud arms.cfg neexistuje, appka si vytvoří defaulty sama přes QSettings.

  ; Odinstalátor + položka v „Programs and Features“
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ${If} $InstallScope == "all"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME} ${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallScope" "$InstallScope"
  ${Else}
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME} ${APP_VERSION}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallScope" "$InstallScope"
  ${EndIf}
SectionEnd

; ==== ODINSTALACE ====
Section "Uninstall"
  ReadRegStr $InstallScope HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallScope"
  ${If} $InstallScope == ""
    ReadRegStr $InstallScope HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallScope"
  ${EndIf}

  ${If} $InstallScope == "all"
    SetShellVarContext all
  ${Else}
    SetShellVarContext current
  ${EndIf}

  Delete "$DESKTOP\${APP_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\${APP_NAME}"
  RMDir /r "$INSTDIR"

  ${If} $InstallScope == "all"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
  ${Else}
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
  ${EndIf}
SectionEnd

Function InstallScopePage
  nsDialogs::Create 1018
  Pop $0
  ${If} $0 == error
    Abort
  ${EndIf}
  !insertmacro MUI_HEADER_TEXT "Installation Scope" "Select who can use ${APP_NAME}"
  ${NSD_CreateRadioButton} 0 0 100% 12u "All users"
  Pop $RadioAll
  ${NSD_CreateRadioButton} 0 13u 100% 12u "Current user"
  Pop $RadioCurrent
  ${NSD_Check} $RadioCurrent
  nsDialogs::Show
FunctionEnd

Function InstallScopePageLeave
  ${NSD_GetState} $RadioAll $0
  ${If} $0 == 1
    StrCpy $InstallScope "all"
    StrCpy $INSTDIR "$PROGRAMFILES\${COMPANY}\${APP_NAME}"
    UAC::RunElevated
    Pop $0
    StrCmp $0 0 +3
      Quit
  ${Else}
    StrCpy $InstallScope "current"
    StrCpy $INSTDIR "$LOCALAPPDATA\${COMPANY}\${APP_NAME}"
  ${EndIf}
FunctionEnd
