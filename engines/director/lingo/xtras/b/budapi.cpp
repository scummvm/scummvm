/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/b/budapi.h"

/**************************************************
 *
 * USED IN:
 * I Spy Spooky Mansion
 *
 **************************************************/

/*
-- xtra BudAPI
new object me

-- Information Functions
* baVersion string Type -- returns version information
* baSysFolder string FolderType -- returns Windows special folders location
* baCpuInfo string InfoType -- returns information about the processor installed
* baDiskInfo string Disk, string InfoType -- returns information about Disk
* baMemoryInfo string InfoType -- returns memory information
* baFindApp string Extension -- finds application associated with Extension
* baReadIni string Section, string Keyname, string Default, string IniFile -- reads ini file entry
* baWriteIni string Section, string Keyname, string Default, string IniFile -- writes ini file entry
* baFlushIni string IniFile -- flushes ini file to disk
* baReadRegString string Keyname, string Value, string Default, string Branch -- returns text from the registry
* baWriteRegString string Keyname, string Value, string NewData, string Branch -- writes a number into the registry
* baReadRegNumber string Keyname, string Value, integer Default, string Branch -- returns a number from the registry
* baWriteRegNumber string Keyname, string Value, integer NewData, string Branch  -- writes a number into the registry
* baDeleteReg string Keyname, string Value, string Branch -- deletes Keyname/Value from registry
* baRegKeyList string Keyname, string Branch -- returns list of keys in Keyname
* baRegValueList string Keyname, string Branch -- returns list of values in Keyname
* baSoundCard -- returns 1 if a sound card is installed
* baFontInstalled string FontName, string FontStyle -- returns 1 if font is installed
* baFontList string FontType -- returns list of installed FontType fonts
* baFontStyleList string FontName -- returns list of available FontName styles
* baCommandArgs -- returns the arguments the projector was started with
* baPrevious integer Activate -- returns window handle of a previous instance of application
* baScreenInfo string InfoType -- returns information about the screen


-- System Functions
* baDisableDiskErrors integer Disable -- disable/enable disk errors
* baDisableKeys integer Disable, integer WinHandle -- disables key presses in window
* baDisableMouse integer Disable, integer WinHandle -- disables/enables mouse clicks in window
* baDisableSwitching integer Disable -- disables/enables task switching
* baDisableScreenSaver integer Disable -- sets the screen saver on or off
* baScreenSaverTime integer Time -- sets the screen saver time out
* baSetScreenSaver string FileName -- sets FileName as the active screen saver
* baSetWallpaper string FileName, integer Tile -- sets FileName as the desktop wallpaper
* baSetPattern string Name, string Pattern -- sets Pattern as the desktop pattern
* baSetDisplay integer Width, integer Height, integer Depth, string Mode, integer Force -- changes the screen display settings
* baExitWindows string Option -- shuts down Windows
* baRunProgram string FileName, string State, integer Wait -- runs external program
* baWinHelp string Command, string FileName, string Data -- shows windows help file
* baMsgBox string Message, string Caption, string Buttons, string Icon, integer DefaultButton -- shows message box
* baHideTaskBar integer Hide -- returns whether TaskBar is showing
* baSetCurrentDir string Dir -- sets the current directory to Dir
* baCopyText string Text -- copies Text to the clipboard
* baPasteText -- returns text from clipboard
* baEncryptText string Text, string Key -- encrypt Text with Key
* baDecryptText string Text, string Key -- decrypt Text with Key
* baPlaceCursor integer X, integer Y -- positions the cursor
* baRestrictCursor integer Left, integer Top, integer Right, integer Bottom -- restirct cursor
* baFreeCursor -- allows cursor to move freely
* baSetVolume string Type, integer Volume -- sets current volume for wave, cd, midi
* baGetVolume string Type -- gets current volume for wave, cd, midi
* baInstallFont string fontfile, string fontname -- installs the font
* baKeyIsDown integer Key -- returns 1 if Key is down
* baKeyBeenPressed integer Key -- returns 1 if Key has been pressed since the last call to baKeyBeenPressed
* baSleep integer milliSecs -- sleeps for milliSecs
* baCreatePMGroup string Group -- creates Program Manager / Start Menu group
* baDeletePMGroup string Group -- deletes Program Manager / Start Menu group
* baCreatePMIcon string Cmd, string Title, string Icon, integer IconNumber -- creates Program Manager / Start Menu icon
* baDeletePMIcon string Icon -- deletes Program Manager / Start Menu icon
* baPMGroupList -- returns list of Program Manager / Start Menu groups
* baPMIconList string Group -- returns list of icons in Group
* baPMSubGroupList string Group -- returns list of Start Menu groups in Group
* baSystemTime string Format -- gets system date/time
* baSetSystemTime string Format, string Time -- sets system date/time
* baPrinterInfo string InfoType -- gets information about installed printers
* baSetPrinter string InfoType, any Data -- sets printer settings
* baRefreshDesktop integer Wait -- refreshed the desktop icons

-- File Functions
* baFileAge string FileName -- returns the age of the file
* baFileExists string FileName -- returns 1 if file exists
* baFolderExists string DirName -- returns 1 if directory exists
* baCreateFolder string DirName -- creates directory
* baDeleteFolder string DirName -- deletes dirname if empty
* baRenameFile string FileName, string NewName -- renames filename to NewName
* baDeleteFile string FileName -- deletes filename
* baDeleteXFiles string DirName, string FileSpec -- deletes files matching FileSpec from DirName
* baXDelete string DirName, string FileSpec -- deletes files matching FileSpec from DirName, including sub-directories
* baFileDate string FileName, string dateformat, string timeformat -- returns date of file
* baFileSize string FileName -- returns size of file in bytes, or -1 if FileName doesn't exist
* baFileAttributes string FileName -- returns string of set attributes in FileName
* baSetFileAttributes string FileName, string Attributes -- sets Attributes for FileName
* baRecycleFile string Filename -- place filename in Recycle Bin
* baCopyFile string SrcFile, string DestFile, string Overwrite -- copies SrcFile to DestFile
* baCopyXFiles string SrcDir, string DestDir, string FileSpec, string Overwrite -- copies all FileSpec files in SrcDir to DestDir
* baXCopy string SrcDir, string DestDir, string FileSpec, string Overwrite, integer MakeDirs -- copies all FileSpec files in SrcDir to DestDir, including sub-directories
* baFileList string Folder, string Pattern -- returns list of files in Folder
* baFolderList string Folder -- returns list of folders in Folder
* baFindFirstFile string Dir, string FileSpec -- finds first file matching FileSpec in Src
* baFindNextFile -- finds next file
* baFindClose -- closes a FindFirst/FindNext loop
* baGetFilename string Operation, string StartDir, string Filename, string Filter, integer Flags, string Instruction, integer NoChangeFolder, integer X, integer Y -- shows Open/save file dialog box
* baGetFolder string StartDir, string Instruction, integer Flags, string Title, integer X, integer Y -- shows directory selector dialog
* baFileVersion string FileName -- returns version of FileName
* baEncryptFile string FileName, string Key -- encrypts FileName with Key
* baFindDrive string StartDrive, string FileName -- finds drive containing FileName
* baOpenFile string FileName, string State -- open FileName
* baOpenURL string FileName, string State -- opens the URL
* baPrintFile string FileName -- prints a file
* baShell string Operation, string FileName, string Args, string WorkDir, string State -- executes a file
* baShortFileName string FileName -- returns the DOS short name of FileName
* baTempFileName string Prefix -- returns a temporary file name
* baMakeShortcut string Filename, string Path, string Caption -- creates a Win95/NT shortcut
* baMakeShortcutEx string File, string Folder, string Title, string Args, string WorkDir, string Icon, integer IconNumber, integer Hotkey, string State -- creates a Win95/NT Shortcut
* baResolveShortcut string Linkname -- returns file that Linkname points to

-- Window Functions
* baWindowInfo integer WinHandle, string Info -- gets information about a window
* baFindWindow string Classname, string caption -- finds a window
* baActiveWindow -- returns the active window
* baCloseWindow integer WinHandle -- closes window
* baCloseApp integer WinHandle -- closes application owning window
* baSetWindowState integer WinHandle, string State -- sets state of window
* baActivateWindow integer WinHandle -- activates window
* baSetWindowTitle integer WinHandle, string Title -- sets title of window
* baMoveWindow integer WinHandle, integer x, integer y, integer width, integer height, integer activate -- moves/resizes WinHandle
* baWindowToFront integer WinHandle -- brings window to front
* baWindowToBack integer WinHandle -- sends window to back
* baGetWindow integer WinHandle, string relation -- gets window related to WinHandle
* baWaitTillActive integer WinHandle -- waits till window becomes the active one
* baWaitForWindow integer WinHandle, string State, integer Ticks -- waits till window is State, for maximuum of Ticks (1 tick = 1/60th sec)
* baNextActiveWindow integer secs -- returns the next active window apart from the Director/Authorware window, with a timeout of Ticks (1 tick = 1/60th sec)
* baWindowExists integer WinHandle -- returns 1 if WinHandle is a valid window handle
* baWindowList string Classname, string Caption, integer Match -- returns list of all matching windows
* baChildWindowList integer ParentWnd, string Classname, string Caption, integer Match -- returns list of all matching child windows of ParentWnd
* baWindowDepth integer WinHandle -- returns z-order depth of specified window
* baSetWindowDepth integer WinHandle, integer Depth -- sets WinHandle to z-order Depth
* baSendKeys string Keys -- send Keys to the active window
* baSendMsg integer WinHandle, integer msg, integer wParam, integer lParam, integer wait -- sends msg to window
* baAddSysItems integer WinHandle, integer SysMenu, integer MinBox, integer MaxBox  -- adds system items
* baRemoveSysItems integer WinHandle, integer SysMenu, integer MinBox, integer MaxBox -- removes system items
* baWinHandle -- returns the handle of the Director or Authorware window
* baStageHandle -- returns the handle of the Director Stage window

-- Buddy Functions
* baAbout -- shows info about Buddy API
* baRegister string UserName, integer Number -- enter your registration information
* baSaveRegistration string UserName, integer UserName -- saves your registration information
* baGetRegistration -- retrieves your registration information
* baFunctions -- retrieves the number of functions you are licensed to use

 */

namespace Director {

const char *BudAPIXtra::xlibName = "BudAPI";
const XlibFileDesc BudAPIXtra::fileNames[] = {
	{ "budapi",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudAPIXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baVersion", BudAPIXtra::m_baVersion, 1, 1, 500, HBLTIN },
	{ "baSysFolder", BudAPIXtra::m_baSysFolder, 1, 1, 500, HBLTIN },
	{ "baCpuInfo", BudAPIXtra::m_baCpuInfo, 1, 1, 500, HBLTIN },
	{ "baDiskInfo", BudAPIXtra::m_baDiskInfo, 2, 2, 500, HBLTIN },
	{ "baMemoryInfo", BudAPIXtra::m_baMemoryInfo, 1, 1, 500, HBLTIN },
	{ "baFindApp", BudAPIXtra::m_baFindApp, 1, 1, 500, HBLTIN },
	{ "baReadIni", BudAPIXtra::m_baReadIni, 4, 4, 500, HBLTIN },
	{ "baWriteIni", BudAPIXtra::m_baWriteIni, 4, 4, 500, HBLTIN },
	{ "baFlushIni", BudAPIXtra::m_baFlushIni, 1, 1, 500, HBLTIN },
	{ "baReadRegString", BudAPIXtra::m_baReadRegString, 4, 4, 500, HBLTIN },
	{ "baWriteRegString", BudAPIXtra::m_baWriteRegString, 4, 4, 500, HBLTIN },
	{ "baReadRegNumber", BudAPIXtra::m_baReadRegNumber, 4, 4, 500, HBLTIN },
	{ "baWriteRegNumber", BudAPIXtra::m_baWriteRegNumber, 4, 4, 500, HBLTIN },
	{ "baDeleteReg", BudAPIXtra::m_baDeleteReg, 3, 3, 500, HBLTIN },
	{ "baRegKeyList", BudAPIXtra::m_baRegKeyList, 2, 2, 500, HBLTIN },
	{ "baRegValueList", BudAPIXtra::m_baRegValueList, 2, 2, 500, HBLTIN },
	{ "baSoundCard", BudAPIXtra::m_baSoundCard, 0, 0, 500, HBLTIN },
	{ "baFontInstalled", BudAPIXtra::m_baFontInstalled, 2, 2, 500, HBLTIN },
	{ "baFontList", BudAPIXtra::m_baFontList, 1, 1, 500, HBLTIN },
	{ "baFontStyleList", BudAPIXtra::m_baFontStyleList, 1, 1, 500, HBLTIN },
	{ "baCommandArgs", BudAPIXtra::m_baCommandArgs, 0, 0, 500, HBLTIN },
	{ "baPrevious", BudAPIXtra::m_baPrevious, 1, 1, 500, HBLTIN },
	{ "baScreenInfo", BudAPIXtra::m_baScreenInfo, 1, 1, 500, HBLTIN },
	{ "baDisableDiskErrors", BudAPIXtra::m_baDisableDiskErrors, 1, 1, 500, HBLTIN },
	{ "baDisableKeys", BudAPIXtra::m_baDisableKeys, 2, 2, 500, HBLTIN },
	{ "baDisableMouse", BudAPIXtra::m_baDisableMouse, 2, 2, 500, HBLTIN },
	{ "baDisableSwitching", BudAPIXtra::m_baDisableSwitching, 1, 1, 500, HBLTIN },
	{ "baDisableScreenSaver", BudAPIXtra::m_baDisableScreenSaver, 1, 1, 500, HBLTIN },
	{ "baScreenSaverTime", BudAPIXtra::m_baScreenSaverTime, 1, 1, 500, HBLTIN },
	{ "baSetScreenSaver", BudAPIXtra::m_baSetScreenSaver, 1, 1, 500, HBLTIN },
	{ "baSetWallpaper", BudAPIXtra::m_baSetWallpaper, 2, 2, 500, HBLTIN },
	{ "baSetPattern", BudAPIXtra::m_baSetPattern, 2, 2, 500, HBLTIN },
	{ "baSetDisplay", BudAPIXtra::m_baSetDisplay, 5, 5, 500, HBLTIN },
	{ "baExitWindows", BudAPIXtra::m_baExitWindows, 1, 1, 500, HBLTIN },
	{ "baRunProgram", BudAPIXtra::m_baRunProgram, 3, 3, 500, HBLTIN },
	{ "baWinHelp", BudAPIXtra::m_baWinHelp, 3, 3, 500, HBLTIN },
	{ "baMsgBox", BudAPIXtra::m_baMsgBox, 5, 5, 500, HBLTIN },
	{ "baHideTaskBar", BudAPIXtra::m_baHideTaskBar, 1, 1, 500, HBLTIN },
	{ "baSetCurrentDir", BudAPIXtra::m_baSetCurrentDir, 1, 1, 500, HBLTIN },
	{ "baCopyText", BudAPIXtra::m_baCopyText, 1, 1, 500, HBLTIN },
	{ "baPasteText", BudAPIXtra::m_baPasteText, 0, 0, 500, HBLTIN },
	{ "baEncryptText", BudAPIXtra::m_baEncryptText, 2, 2, 500, HBLTIN },
	{ "baDecryptText", BudAPIXtra::m_baDecryptText, 2, 2, 500, HBLTIN },
	{ "baPlaceCursor", BudAPIXtra::m_baPlaceCursor, 2, 2, 500, HBLTIN },
	{ "baRestrictCursor", BudAPIXtra::m_baRestrictCursor, 4, 4, 500, HBLTIN },
	{ "baFreeCursor", BudAPIXtra::m_baFreeCursor, 0, 0, 500, HBLTIN },
	{ "baSetVolume", BudAPIXtra::m_baSetVolume, 2, 2, 500, HBLTIN },
	{ "baGetVolume", BudAPIXtra::m_baGetVolume, 1, 1, 500, HBLTIN },
	{ "baInstallFont", BudAPIXtra::m_baInstallFont, 2, 2, 500, HBLTIN },
	{ "baKeyIsDown", BudAPIXtra::m_baKeyIsDown, 1, 1, 500, HBLTIN },
	{ "baKeyBeenPressed", BudAPIXtra::m_baKeyBeenPressed, 1, 1, 500, HBLTIN },
	{ "baSleep", BudAPIXtra::m_baSleep, 1, 1, 500, HBLTIN },
	{ "baCreatePMGroup", BudAPIXtra::m_baCreatePMGroup, 1, 1, 500, HBLTIN },
	{ "baDeletePMGroup", BudAPIXtra::m_baDeletePMGroup, 1, 1, 500, HBLTIN },
	{ "baCreatePMIcon", BudAPIXtra::m_baCreatePMIcon, 4, 4, 500, HBLTIN },
	{ "baDeletePMIcon", BudAPIXtra::m_baDeletePMIcon, 1, 1, 500, HBLTIN },
	{ "baPMGroupList", BudAPIXtra::m_baPMGroupList, 0, 0, 500, HBLTIN },
	{ "baPMIconList", BudAPIXtra::m_baPMIconList, 1, 1, 500, HBLTIN },
	{ "baPMSubGroupList", BudAPIXtra::m_baPMSubGroupList, 1, 1, 500, HBLTIN },
	{ "baSystemTime", BudAPIXtra::m_baSystemTime, 1, 1, 500, HBLTIN },
	{ "baSetSystemTime", BudAPIXtra::m_baSetSystemTime, 2, 2, 500, HBLTIN },
	{ "baPrinterInfo", BudAPIXtra::m_baPrinterInfo, 1, 1, 500, HBLTIN },
	{ "baSetPrinter", BudAPIXtra::m_baSetPrinter, 2, 2, 500, HBLTIN },
	{ "baRefreshDesktop", BudAPIXtra::m_baRefreshDesktop, 1, 1, 500, HBLTIN },
	{ "baFileAge", BudAPIXtra::m_baFileAge, 1, 1, 500, HBLTIN },
	{ "baFileExists", BudAPIXtra::m_baFileExists, 1, 1, 500, HBLTIN },
	{ "baFolderExists", BudAPIXtra::m_baFolderExists, 1, 1, 500, HBLTIN },
	{ "baCreateFolder", BudAPIXtra::m_baCreateFolder, 1, 1, 500, HBLTIN },
	{ "baDeleteFolder", BudAPIXtra::m_baDeleteFolder, 1, 1, 500, HBLTIN },
	{ "baRenameFile", BudAPIXtra::m_baRenameFile, 2, 2, 500, HBLTIN },
	{ "baDeleteFile", BudAPIXtra::m_baDeleteFile, 1, 1, 500, HBLTIN },
	{ "baDeleteXFiles", BudAPIXtra::m_baDeleteXFiles, 2, 2, 500, HBLTIN },
	{ "baXDelete", BudAPIXtra::m_baXDelete, 2, 2, 500, HBLTIN },
	{ "baFileDate", BudAPIXtra::m_baFileDate, 3, 3, 500, HBLTIN },
	{ "baFileSize", BudAPIXtra::m_baFileSize, 1, 1, 500, HBLTIN },
	{ "baFileAttributes", BudAPIXtra::m_baFileAttributes, 1, 1, 500, HBLTIN },
	{ "baSetFileAttributes", BudAPIXtra::m_baSetFileAttributes, 2, 2, 500, HBLTIN },
	{ "baRecycleFile", BudAPIXtra::m_baRecycleFile, 1, 1, 500, HBLTIN },
	{ "baCopyFile", BudAPIXtra::m_baCopyFile, 3, 3, 500, HBLTIN },
	{ "baCopyXFiles", BudAPIXtra::m_baCopyXFiles, 4, 4, 500, HBLTIN },
	{ "baXCopy", BudAPIXtra::m_baXCopy, 5, 5, 500, HBLTIN },
	{ "baFileList", BudAPIXtra::m_baFileList, 2, 2, 500, HBLTIN },
	{ "baFolderList", BudAPIXtra::m_baFolderList, 1, 1, 500, HBLTIN },
	{ "baFindFirstFile", BudAPIXtra::m_baFindFirstFile, 2, 2, 500, HBLTIN },
	{ "baFindNextFile", BudAPIXtra::m_baFindNextFile, 0, 0, 500, HBLTIN },
	{ "baFindClose", BudAPIXtra::m_baFindClose, 0, 0, 500, HBLTIN },
	{ "baGetFilename", BudAPIXtra::m_baGetFilename, 9, 9, 500, HBLTIN },
	{ "baGetFolder", BudAPIXtra::m_baGetFolder, 6, 6, 500, HBLTIN },
	{ "baFileVersion", BudAPIXtra::m_baFileVersion, 1, 1, 500, HBLTIN },
	{ "baEncryptFile", BudAPIXtra::m_baEncryptFile, 2, 2, 500, HBLTIN },
	{ "baFindDrive", BudAPIXtra::m_baFindDrive, 2, 2, 500, HBLTIN },
	{ "baOpenFile", BudAPIXtra::m_baOpenFile, 2, 2, 500, HBLTIN },
	{ "baOpenURL", BudAPIXtra::m_baOpenURL, 2, 2, 500, HBLTIN },
	{ "baPrintFile", BudAPIXtra::m_baPrintFile, 1, 1, 500, HBLTIN },
	{ "baShell", BudAPIXtra::m_baShell, 5, 5, 500, HBLTIN },
	{ "baShortFileName", BudAPIXtra::m_baShortFileName, 1, 1, 500, HBLTIN },
	{ "baTempFileName", BudAPIXtra::m_baTempFileName, 1, 1, 500, HBLTIN },
	{ "baMakeShortcut", BudAPIXtra::m_baMakeShortcut, 3, 3, 500, HBLTIN },
	{ "baMakeShortcutEx", BudAPIXtra::m_baMakeShortcutEx, 9, 9, 500, HBLTIN },
	{ "baResolveShortcut", BudAPIXtra::m_baResolveShortcut, 1, 1, 500, HBLTIN },
	{ "baWindowInfo", BudAPIXtra::m_baWindowInfo, 2, 2, 500, HBLTIN },
	{ "baFindWindow", BudAPIXtra::m_baFindWindow, 2, 2, 500, HBLTIN },
	{ "baActiveWindow", BudAPIXtra::m_baActiveWindow, 0, 0, 500, HBLTIN },
	{ "baCloseWindow", BudAPIXtra::m_baCloseWindow, 1, 1, 500, HBLTIN },
	{ "baCloseApp", BudAPIXtra::m_baCloseApp, 1, 1, 500, HBLTIN },
	{ "baSetWindowState", BudAPIXtra::m_baSetWindowState, 2, 2, 500, HBLTIN },
	{ "baActivateWindow", BudAPIXtra::m_baActivateWindow, 1, 1, 500, HBLTIN },
	{ "baSetWindowTitle", BudAPIXtra::m_baSetWindowTitle, 2, 2, 500, HBLTIN },
	{ "baMoveWindow", BudAPIXtra::m_baMoveWindow, 6, 6, 500, HBLTIN },
	{ "baWindowToFront", BudAPIXtra::m_baWindowToFront, 1, 1, 500, HBLTIN },
	{ "baWindowToBack", BudAPIXtra::m_baWindowToBack, 1, 1, 500, HBLTIN },
	{ "baGetWindow", BudAPIXtra::m_baGetWindow, 2, 2, 500, HBLTIN },
	{ "baWaitTillActive", BudAPIXtra::m_baWaitTillActive, 1, 1, 500, HBLTIN },
	{ "baWaitForWindow", BudAPIXtra::m_baWaitForWindow, 3, 3, 500, HBLTIN },
	{ "baNextActiveWindow", BudAPIXtra::m_baNextActiveWindow, 1, 1, 500, HBLTIN },
	{ "baWindowExists", BudAPIXtra::m_baWindowExists, 1, 1, 500, HBLTIN },
	{ "baWindowList", BudAPIXtra::m_baWindowList, 3, 3, 500, HBLTIN },
	{ "baChildWindowList", BudAPIXtra::m_baChildWindowList, 4, 4, 500, HBLTIN },
	{ "baWindowDepth", BudAPIXtra::m_baWindowDepth, 1, 1, 500, HBLTIN },
	{ "baSetWindowDepth", BudAPIXtra::m_baSetWindowDepth, 2, 2, 500, HBLTIN },
	{ "baSendKeys", BudAPIXtra::m_baSendKeys, 1, 1, 500, HBLTIN },
	{ "baSendMsg", BudAPIXtra::m_baSendMsg, 5, 5, 500, HBLTIN },
	{ "baAddSysItems", BudAPIXtra::m_baAddSysItems, 4, 4, 500, HBLTIN },
	{ "baRemoveSysItems", BudAPIXtra::m_baRemoveSysItems, 4, 4, 500, HBLTIN },
	{ "baWinHandle", BudAPIXtra::m_baWinHandle, 0, 0, 500, HBLTIN },
	{ "baStageHandle", BudAPIXtra::m_baStageHandle, 0, 0, 500, HBLTIN },
	{ "baAbout", BudAPIXtra::m_baAbout, 0, 0, 500, HBLTIN },
	{ "baRegister", BudAPIXtra::m_baRegister, 2, 2, 500, HBLTIN },
	{ "baSaveRegistration", BudAPIXtra::m_baSaveRegistration, 2, 2, 500, HBLTIN },
	{ "baGetRegistration", BudAPIXtra::m_baGetRegistration, 0, 0, 500, HBLTIN },
	{ "baFunctions", BudAPIXtra::m_baFunctions, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudAPIXtraObject::BudAPIXtraObject(ObjectType ObjectType) :Object<BudAPIXtraObject>("BudAPI") {
	_objType = ObjectType;
}

bool BudAPIXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudAPIXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudAPIXtra::xlibName);
	warning("BudAPIXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudAPIXtra::open(ObjectType type, const Common::Path &path) {
    BudAPIXtraObject::initMethods(xlibMethods);
    BudAPIXtraObject *xobj = new BudAPIXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudAPIXtra::close(ObjectType type) {
    BudAPIXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudAPIXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudAPIXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudAPIXtra::m_baVersion, 0)
XOBJSTUB(BudAPIXtra::m_baSysFolder, 0)
XOBJSTUB(BudAPIXtra::m_baCpuInfo, 0)
XOBJSTUB(BudAPIXtra::m_baDiskInfo, 0)
XOBJSTUB(BudAPIXtra::m_baMemoryInfo, 0)
XOBJSTUB(BudAPIXtra::m_baFindApp, 0)
XOBJSTUB(BudAPIXtra::m_baReadIni, 0)
XOBJSTUB(BudAPIXtra::m_baWriteIni, 0)
XOBJSTUB(BudAPIXtra::m_baFlushIni, 0)
XOBJSTUB(BudAPIXtra::m_baReadRegString, 0)
XOBJSTUB(BudAPIXtra::m_baWriteRegString, 0)
XOBJSTUB(BudAPIXtra::m_baReadRegNumber, 0)
XOBJSTUB(BudAPIXtra::m_baWriteRegNumber, 0)
XOBJSTUB(BudAPIXtra::m_baDeleteReg, 0)
XOBJSTUB(BudAPIXtra::m_baRegKeyList, 0)
XOBJSTUB(BudAPIXtra::m_baRegValueList, 0)
XOBJSTUB(BudAPIXtra::m_baSoundCard, 0)
XOBJSTUB(BudAPIXtra::m_baFontInstalled, 0)
XOBJSTUB(BudAPIXtra::m_baFontList, 0)
XOBJSTUB(BudAPIXtra::m_baFontStyleList, 0)
XOBJSTUB(BudAPIXtra::m_baCommandArgs, 0)
XOBJSTUB(BudAPIXtra::m_baPrevious, 0)
XOBJSTUB(BudAPIXtra::m_baScreenInfo, 0)
XOBJSTUB(BudAPIXtra::m_baDisableDiskErrors, 0)
XOBJSTUB(BudAPIXtra::m_baDisableKeys, 0)
XOBJSTUB(BudAPIXtra::m_baDisableMouse, 0)
XOBJSTUB(BudAPIXtra::m_baDisableSwitching, 0)
XOBJSTUB(BudAPIXtra::m_baDisableScreenSaver, 0)
XOBJSTUB(BudAPIXtra::m_baScreenSaverTime, 0)
XOBJSTUB(BudAPIXtra::m_baSetScreenSaver, 0)
XOBJSTUB(BudAPIXtra::m_baSetWallpaper, 0)
XOBJSTUB(BudAPIXtra::m_baSetPattern, 0)
XOBJSTUB(BudAPIXtra::m_baSetDisplay, 0)
XOBJSTUB(BudAPIXtra::m_baExitWindows, 0)
XOBJSTUB(BudAPIXtra::m_baRunProgram, 0)
XOBJSTUB(BudAPIXtra::m_baWinHelp, 0)
XOBJSTUB(BudAPIXtra::m_baMsgBox, 0)
XOBJSTUB(BudAPIXtra::m_baHideTaskBar, 0)
XOBJSTUB(BudAPIXtra::m_baSetCurrentDir, 0)
XOBJSTUB(BudAPIXtra::m_baCopyText, 0)
XOBJSTUB(BudAPIXtra::m_baPasteText, 0)
XOBJSTUB(BudAPIXtra::m_baEncryptText, 0)
XOBJSTUB(BudAPIXtra::m_baDecryptText, 0)
XOBJSTUB(BudAPIXtra::m_baPlaceCursor, 0)
XOBJSTUB(BudAPIXtra::m_baRestrictCursor, 0)
XOBJSTUB(BudAPIXtra::m_baFreeCursor, 0)
XOBJSTUB(BudAPIXtra::m_baSetVolume, 0)
XOBJSTUB(BudAPIXtra::m_baGetVolume, 0)
XOBJSTUB(BudAPIXtra::m_baInstallFont, 0)
XOBJSTUB(BudAPIXtra::m_baKeyIsDown, 0)
XOBJSTUB(BudAPIXtra::m_baKeyBeenPressed, 0)
XOBJSTUB(BudAPIXtra::m_baSleep, 0)
XOBJSTUB(BudAPIXtra::m_baCreatePMGroup, 0)
XOBJSTUB(BudAPIXtra::m_baDeletePMGroup, 0)
XOBJSTUB(BudAPIXtra::m_baCreatePMIcon, 0)
XOBJSTUB(BudAPIXtra::m_baDeletePMIcon, 0)
XOBJSTUB(BudAPIXtra::m_baPMGroupList, 0)
XOBJSTUB(BudAPIXtra::m_baPMIconList, 0)
XOBJSTUB(BudAPIXtra::m_baPMSubGroupList, 0)
XOBJSTUB(BudAPIXtra::m_baSystemTime, 0)
XOBJSTUB(BudAPIXtra::m_baSetSystemTime, 0)
XOBJSTUB(BudAPIXtra::m_baPrinterInfo, 0)
XOBJSTUB(BudAPIXtra::m_baSetPrinter, 0)
XOBJSTUB(BudAPIXtra::m_baRefreshDesktop, 0)
XOBJSTUB(BudAPIXtra::m_baFileAge, 0)
XOBJSTUB(BudAPIXtra::m_baFileExists, 0)
XOBJSTUB(BudAPIXtra::m_baFolderExists, 0)
XOBJSTUB(BudAPIXtra::m_baCreateFolder, 0)
XOBJSTUB(BudAPIXtra::m_baDeleteFolder, 0)
XOBJSTUB(BudAPIXtra::m_baRenameFile, 0)
XOBJSTUB(BudAPIXtra::m_baDeleteFile, 0)
XOBJSTUB(BudAPIXtra::m_baDeleteXFiles, 0)
XOBJSTUB(BudAPIXtra::m_baXDelete, 0)
XOBJSTUB(BudAPIXtra::m_baFileDate, 0)
XOBJSTUB(BudAPIXtra::m_baFileSize, 0)
XOBJSTUB(BudAPIXtra::m_baFileAttributes, 0)
XOBJSTUB(BudAPIXtra::m_baSetFileAttributes, 0)
XOBJSTUB(BudAPIXtra::m_baRecycleFile, 0)
XOBJSTUB(BudAPIXtra::m_baCopyFile, 0)
XOBJSTUB(BudAPIXtra::m_baCopyXFiles, 0)
XOBJSTUB(BudAPIXtra::m_baXCopy, 0)
XOBJSTUB(BudAPIXtra::m_baFileList, 0)
XOBJSTUB(BudAPIXtra::m_baFolderList, 0)
XOBJSTUB(BudAPIXtra::m_baFindFirstFile, 0)
XOBJSTUB(BudAPIXtra::m_baFindNextFile, 0)
XOBJSTUB(BudAPIXtra::m_baFindClose, 0)
XOBJSTUB(BudAPIXtra::m_baGetFilename, 0)
XOBJSTUB(BudAPIXtra::m_baGetFolder, 0)
XOBJSTUB(BudAPIXtra::m_baFileVersion, 0)
XOBJSTUB(BudAPIXtra::m_baEncryptFile, 0)
XOBJSTUB(BudAPIXtra::m_baFindDrive, 0)
XOBJSTUB(BudAPIXtra::m_baOpenFile, 0)
XOBJSTUB(BudAPIXtra::m_baOpenURL, 0)
XOBJSTUB(BudAPIXtra::m_baPrintFile, 0)
XOBJSTUB(BudAPIXtra::m_baShell, 0)
XOBJSTUB(BudAPIXtra::m_baShortFileName, 0)
XOBJSTUB(BudAPIXtra::m_baTempFileName, 0)
XOBJSTUB(BudAPIXtra::m_baMakeShortcut, 0)
XOBJSTUB(BudAPIXtra::m_baMakeShortcutEx, 0)
XOBJSTUB(BudAPIXtra::m_baResolveShortcut, 0)
XOBJSTUB(BudAPIXtra::m_baWindowInfo, 0)
XOBJSTUB(BudAPIXtra::m_baFindWindow, 0)
XOBJSTUB(BudAPIXtra::m_baActiveWindow, 0)
XOBJSTUB(BudAPIXtra::m_baCloseWindow, 0)
XOBJSTUB(BudAPIXtra::m_baCloseApp, 0)
XOBJSTUB(BudAPIXtra::m_baSetWindowState, 0)
XOBJSTUB(BudAPIXtra::m_baActivateWindow, 0)
XOBJSTUB(BudAPIXtra::m_baSetWindowTitle, 0)
XOBJSTUB(BudAPIXtra::m_baMoveWindow, 0)
XOBJSTUB(BudAPIXtra::m_baWindowToFront, 0)
XOBJSTUB(BudAPIXtra::m_baWindowToBack, 0)
XOBJSTUB(BudAPIXtra::m_baGetWindow, 0)
XOBJSTUB(BudAPIXtra::m_baWaitTillActive, 0)
XOBJSTUB(BudAPIXtra::m_baWaitForWindow, 0)
XOBJSTUB(BudAPIXtra::m_baNextActiveWindow, 0)
XOBJSTUB(BudAPIXtra::m_baWindowExists, 0)
XOBJSTUB(BudAPIXtra::m_baWindowList, 0)
XOBJSTUB(BudAPIXtra::m_baChildWindowList, 0)
XOBJSTUB(BudAPIXtra::m_baWindowDepth, 0)
XOBJSTUB(BudAPIXtra::m_baSetWindowDepth, 0)
XOBJSTUB(BudAPIXtra::m_baSendKeys, 0)
XOBJSTUB(BudAPIXtra::m_baSendMsg, 0)
XOBJSTUB(BudAPIXtra::m_baAddSysItems, 0)
XOBJSTUB(BudAPIXtra::m_baRemoveSysItems, 0)
XOBJSTUB(BudAPIXtra::m_baWinHandle, 0)
XOBJSTUB(BudAPIXtra::m_baStageHandle, 0)
XOBJSTUB(BudAPIXtra::m_baAbout, 0)
XOBJSTUB(BudAPIXtra::m_baRegister, 0)
XOBJSTUB(BudAPIXtra::m_baSaveRegistration, 0)
XOBJSTUB(BudAPIXtra::m_baGetRegistration, 0)
XOBJSTUB(BudAPIXtra::m_baFunctions, 0)

}
