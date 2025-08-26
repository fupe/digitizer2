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
InstallDir "$LOCALAPPDATA\\${COMPANY}\\${APP_NAME}"
RequestExecutionLevel user
SetCompressor /SOLID lzma

Var Arm1Length
Var Arm2Length

; ==== STRÁNKY ====
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
    WriteRegStr HKCU "Software\\LiborSoft\\Digitizer2\\Program" "arm1_length" "$Arm1Length"
    WriteRegStr HKCU "Software\\LiborSoft\\Digitizer2\\Program" "arm2_length" "$Arm2Length"
  ${EndIf}
done:
  Pop $0
FunctionEnd

; ==== INSTALACE ====
Section "Application (required)" SecMain
  SectionIn RO

  SetShellVarContext current

  ; Cíl instalace
  SetOutPath "$INSTDIR"

  ; Zkopíruj aplikaci (předpokládá se, že obsah je v ./dist)
  ; - pokud používáš jinou strukturu, cestu uprav
  File /r "dist\\*.*"

  ; Zástupci
  CreateDirectory "$SMPROGRAMS\\${APP_NAME}"
  CreateShortCut "$SMPROGRAMS\\${APP_NAME}\\${APP_NAME}.lnk" "$INSTDIR\\digitizer2.exe"
  CreateShortCut "$DESKTOP\\${APP_NAME}.lnk" "$INSTDIR\\digitizer2.exe"

  ; --- REGISTR: načti délky ramen z arms.cfg (pokud existuje) ---
  Push "$EXEDIR\\configs\\arms.cfg"
  Call WriteArmLengths

  ; Pokud arms.cfg neexistuje, appka si vytvoří defaulty sama přes QSettings.

  ; Odinstalátor + položka v „Programs and Features“
  WriteUninstaller "$INSTDIR\\Uninstall.exe"
  WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${APP_NAME}" "DisplayName" "${APP_NAME} ${APP_VERSION}"
  WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${APP_NAME}" "UninstallString" '"$INSTDIR\\Uninstall.exe"'
SectionEnd

; ==== ODINSTALACE ====
Section "Uninstall"
  SetShellVarContext current

  Delete "$DESKTOP\\${APP_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\\${APP_NAME}"
  RMDir /r "$INSTDIR"

  DeleteRegKey HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\${APP_NAME}"
SectionEnd
