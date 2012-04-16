;Installer for SigmaSpectra
;Written by Albert Kottke

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Revision of the respository
  !system 'getSvnVersion.py "!define REVISION" > %TEMP%\revision.nsh'
  !include "$%TEMP%\revision.nsh"

;--------------------------------
; Path to Qt
!Define QT_PATH "C:\devel\QtSDK\Desktop\Qt\4.7.4\mingw"

;--------------------------------
;Variables
  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;General

  ;Name and file
  Name "SigmaSpectra"
  OutFile "SigmaSpectra-rev-${REVISION}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SigmaSpectra"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SigmaSpectra" ""

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

  ;Set the type of compress to LZMA
  SetCompressor lzma

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ; Start menu folder page configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SigmaSpectra"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU SigmaSpectra $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES
  ; Finish page configuration
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
  !define MUI_FINISHPAGE_RUN "$INSTDIR\sigmaSpectra.exe"
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Association macros -- from: http://nsis.sourceforge.net/FileAssoc
!macro APP_ASSOCIATE EXT FILECLASS DESCRIPTION ICON COMMANDTEXT COMMAND
  ; Backup the previously associated file class
  ReadRegStr $R0 HKCR ".${EXT}" ""
  WriteRegStr HKCR ".${EXT}" "${FILECLASS}_backup" "$R0"
 
  WriteRegStr HKCR ".${EXT}" "" "${FILECLASS}"
 
  WriteRegStr HKCR "${FILECLASS}" "" `${DESCRIPTION}`
  WriteRegStr HKCR "${FILECLASS}\DefaultIcon" "" `${ICON}`
  WriteRegStr HKCR "${FILECLASS}\shell" "" "open"
  WriteRegStr HKCR "${FILECLASS}\shell\open" "" `${COMMANDTEXT}`
  WriteRegStr HKCR "${FILECLASS}\shell\open\command" "" `${COMMAND}`
!macroend

!macro APP_UNASSOCIATE EXT FILECLASS
  ; Backup the previously associated file class
  ReadRegStr $R0 HKCR ".${EXT}" `${FILECLASS}_backup`
  WriteRegStr HKCR ".${EXT}" "" "$R0"
 
  DeleteRegKey HKCR `${FILECLASS}`
!macroend

;--------------------------------
;Installer Sections

Section "!Core Files" SecProgram
  SectionIn RO
  SetOutPath "$INSTDIR"

  ;Main SigmaSpectra files
  file "/oname=$INSTDIR\sigmaSpectra.exe" "release\sigmaSpectra.exe"
  file "/oname=$INSTDIR\readme.txt" "README"

  ;Icons
  file "/oname=$INSTDIR\sigmaSpectra.ico" "resources\images\application-icon.ico"
  ;file "/oname=$INSTDIR\strata-input.ico" "resources\images\file-input.ico"
  ;file "/oname=$INSTDIR\strata-output.ico" "resources\images\file-output.ico"

  ;Main libraries
  file "C:\devel\qwt-6.0\lib\qwt.dll"
  file "C:\devel\GnuWin32\bin\libgsl.dll"
  file "C:\devel\GnuWin32\bin\libgslcblas.dll"
  file "C:\devel\fftw-3.2.2\libfftw3-3.dll"
  file "${QT_PATH}\bin\libgcc_s_dw2-1.dll"
  file "${QT_PATH}\bin\mingwm10.dll"
  file "${QT_PATH}\bin\QtCore4.dll"
  file "${QT_PATH}\bin\QtGui4.dll"
  file "${QT_PATH}\bin\QtSvg4.dll"
  file "${QT_PATH}\bin\QtXml4.dll"
  
  ;Plugins for SVG icons
  SetOutPath "$INSTDIR\iconengines" 
  file "${QT_PATH}\plugins\iconengines\qsvgicon4.dll"
  SetOutPath "$INSTDIR\imageformats" 
  file "${QT_PATH}\plugins\imageformats\qsvg4.dll"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\SigmaSpectra" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN SigmaSpectra
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SigmaSpectra.lnk" "$INSTDIR\SigmaSpectra.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Manual" SecManual
	SetOutPath "$INSTDIR"

	;Files to install
	file "manual\manual.pdf"
SectionEnd

Section /o "Example" SecExample

	SetOutPath "$INSTDIR\example"

	;Files to install
	file /r "example\*.AT2"
SectionEnd

Section /o "Source" SecSource

	SetOutPath "$INSTDIR\src"

	;Files to install
	file "/oname=..\sigmaSpectra.pro" "sigmaSpectra.pro"
	file "src\*.h"
	file "src\*.cpp"
SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
  LangString DESC_SecProgram ${LANG_ENGLISH} "Binaries required to run SigmaSpectra."
  LangString DESC_SecManual ${LANG_ENGLISH} "A pdf version of the help manual. The program includes a help manual, but this version can be printed and therefore may be more convenient"
  LangString DESC_SecExample ${LANG_ENGLISH} "AT2 Files for the example presented in the help manual."
  LangString DESC_SecSource ${LANG_ENGLISH} "Source code."


  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecManual} $(DESC_SecManual)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExample} $(DESC_SecExample)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSource} $(DESC_SecSource)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ;Remove the files
  Delete "$INSTDIR\libfftw3-3.dll"
  Delete "$INSTDIR\libgsl.dll"
  Delete "$INSTDIR\libgslcblas.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtScript4.dll"
  Delete "$INSTDIR\QtSvg4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\qwt5.dll"
  Delete "$INSTDIR\iconengines\qsvg4.dll"
  RMDir  "$INSTDIR\iconengines"
  Delete "$INSTDIR\imageformats\qsvg4.dll"
  RMDir  "$INSTDIR\imageformats"
  Delete "$INSTDIR\sigmaSpectra.pro"

  ;Remove all source files
  RMDir /r "$INSTDIR\src"

  ;Remove the install directory
  RMDir $INSTDIR

  ;Remove the association with SigmaSpectra
  ;!insertmacro APP_UNASSOCIATE "stri" "strata.inputfile"
  ;!insertmacro APP_UNASSOCIATE "stro" "strata.outputfile"

  ;Remove the Start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER SigmaSpectra $MUI_TEMP
   
  ;Delete the shortcuts
  Delete "$SMPROGRAMS\$MUI_TEMP\SigmaSpectra.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\SigmaSpectra"
SectionEnd
