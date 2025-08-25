!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "nsDialogs.nsh"

; ==== ZÁKLADNÍ INFO (uprav podle sebe) ====
!define APP_NAME     "Digitizer2"
!define COMPANY      "LiborSoft"
!define APP_VERSION  "1.0.0"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "digitizer2-setup-${APP_VERSION}.exe"
InstallDir "$PROGRAMFILES\${COMPANY}\${APP_NAME}"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

; ==== STRÁNKY ====
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; ==== FUNKCE: Import .reg (jen když existuje) ====
Function ImportRegIfExists
  Exch $0  ; $0 = cesta k .reg
  IfFileExists "$0" +2 0
    Goto done
  SetRegView 64
  DetailPrint "Importing registry file: $0"
  nsExec::ExecToStack 'reg import "$0"'
  Pop $1    ; exit code
  Pop $2    ; output
  StrCmp $1 "0" +2
    MessageBox MB_ICONEXCLAMATION "Registry import failed ($1).$\\r$\\n$2"
done:
FunctionEnd

; ==== INSTALACE ====
Section "Application (required)" SecMain
  SectionIn RO

  ; Cíl instalace
  SetOutPath "$INSTDIR"

  ; Zkopíruj aplikaci (předpokládá se, že obsah je v ./dist)
  ; - pokud používáš jinou strukturu, cestu uprav
  File /r "dist\*.*"

  ; Zástupci
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\digitizer2.exe"
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\digitizer2.exe"

  ; --- REGISTR: import jen pokud existuje user.reg ---
  ; 1) user.reg vedle instalátoru
  Push "$EXEDIR\user.reg"
  Call ImportRegIfExists

  ; 2) nebo v podsložce configs\user.reg (nepovinné)
  Push "$EXEDIR\configs\user.reg"
  Call ImportRegIfExists

  ; (ŽÁDNÝ jiný zápis do registru se neprovádí.
  ;  Pokud user.reg neexistuje, appka si vytvoří defaulty sama přes QSettings.)

  ; Odinstalátor + položka v „Programs and Features“
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME} ${APP_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

; ==== ODINSTALACE ====
Section "Uninstall"
  Delete "$DESKTOP\${APP_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\${APP_NAME}"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd
