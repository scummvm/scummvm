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
#include "director/lingo/xtras/b/bawindows.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra baWindows --- Mac Only Xtra
-- Copyright: Magic Modules Pty Ltd 1998 - 2004.
-- Email: buddy@mods.com.au
-- Web site: http://www.buddyapi.com
new object me
---Information Functions---
baCpuInfo string Type -- returns information about the processor
baFlushIni string IniFile -- flushes ini file to disk 
baDeleteIniEntry string Section, string Entry, string IniFile 
baDeleteIniSection string Section, string Inifile
baReadRegString string Keyname, string Value, string Default, string Branch -- returns text from the registry
baWriteRegString string Keyname, string Value, string NewData, string Branch -- writes a number into the registry
baReadRegNumber string Keyname, string Value, integer Default, string Branch -- returns a number from the registry
baWriteRegNumber string Keyname, string Value, integer NewData, string Branch  -- writes a number into the registry
baReadRegBinary string Key, string Value, string Default, string Branch -- reads registry value
baWriteRegBinary string Key, string Value, list Data, string Branch -- writes registry value
baReadRegMulti string Key, string Value, string Default, string Branch -- reads registry value
baWriteRegMulti string Key, string Value, list Data, string Branch -- writes registry value
baDeleteReg string Keyname, string Value, string Branch -- deletes Keyname/Value from registry
baRegKeyList string Keyname, string Branch -- returns list of keys in Keyname
baRegValueList string Keyname, string Branch -- returns list of values in Keyname
baSoundCard -- returns 1 if a sound card is installed
baFontList string FontType -- returns list of installed FontType fonts
baFontStyleList string FontName -- returns list of available FontName styles
baCommandArgs -- returns the arguments the projector was started with
baPrevious integer Activate -- returns window handle of a previous instance of application
baScreenInfo string InfoType -- returns information about the screen
---System Functions--- 
baDisableDiskErrors integer Disable -- disable/enable disk errors
baDisableKeys integer Disable, integer WinHandle -- disables key presses in window
baDisableMouse integer Disable, integer WinHandle -- disables/enables mouse clicks in window
baDisableSwitching integer Disable -- disables/enables task switching
baDisableScreenSaver integer Disable -- sets the screen saver on or off
baScreenSaverTime integer Time -- sets the screen saver time out
baSetPattern string Name, string Pattern -- sets Pattern as the desktop pattern
baExitWindows string Option -- shuts down Windows
baWinHelp string Command, string FileName, string Data -- shows windows help file
baSetCurrentDir string Dir -- sets the current directory to Dir
baRestrictCursor integer Left, integer Top, integer Right, integer Bottom -- restirct cursor
baFreeCursor -- allows cursor to move freely
baInstallFont string fontfile, string fontname -- installs the font
baKeyBeenPressed integer Key -- returns 1 if Key has been pressed since the last call to baKeyBeenPressed
baSleep integer milliSecs -- sleeps for milliSecs
baSetDisplayEx integer Width, integer Height, integer Depth, integer Frequency, string Mode, integer Force -- changes screen settings
baEnvironment string Environment -- gets Environment variable
baSetEnvironment string Environment, string Value -- set Environment variable
baCreatePMGroup string Group -- creates Program Manager / Start Menu group
baDeletePMGroup string Group -- deletes Program Manager / Start Menu group
baCreatePMIcon string Cmd, string Title, string Icon, integer IconNumber -- creates Program Manager / Start Menu icon
baDeletePMIcon string Icon -- deletes Program Manager / Start Menu icon
baPMGroupList -- returns list of Program Manager / Start Menu groups
baPMIconList string Group -- returns list of icons in Group
baPMSubGroupList string Group -- returns list of Start Menu groups in Group
baPrinterInfo string InfoType -- gets information about installed printers
baSetPrinter string InfoType, any Data -- sets printer settings
baPrintDlg integer Flags
baPageSetupDlg integer Flags
baRefreshDesktop integer Wait -- refreshed the desktop icons
---File Functions---
baFileDate string FileName, string dateformat, string timeformat -- returns date of file
baFileDateEx string FileName, string DateType, string DateFormat, string TimeFormat -- returns date of file
baSetFileDate string FileName, integer Year, integer Month, integer Day, integer Hour, integer Minute, integer Second -- sets date and time of file 
baRecycleFile string Filename -- place filename in Recycle Bin
baFindDrive string StartDrive, string FileName -- finds drive containing FileName
baShell string Operation, string FileName, string Args, string WorkDir, string State -- executes a file
baTempFileName string Prefix -- returns a temporary file name
baMakeShortcutEx string File, string Folder, string Title, string Args, string WorkDir, string Icon, integer IconNumber, integer Hotkey, string State -- creates a Win95/NT Shortcut
baResolveShortcut string Linkname -- returns file that Linkname points to\n\n-- Window Functions
baCloseApp integer WinHandle -- closes application owning window
baSetWindowState integer WinHandle, string State -- sets state of window
baWaitTillActive integer WinHandle -- waits till window becomes the active one
baChildWindowList integer ParentWnd, string Classname, string Caption, integer Match -- returns list of all matching child windows of ParentWnd
baWindowDepth integer WinHandle -- returns z-order depth of specified window
baSetWindowDepth integer WinHandle, integer Depth -- sets WinHandle to z-order Depth
baSendMsg integer WinHandle, integer msg, integer wParam, integer lParam, integer wait -- sends msg to window
baAddSysItems integer WinHandle, integer SysMenu, integer MinBox, integer MaxBox  -- adds system items
baClipWindow integer WinHandle, integer Left, integer Top, integer Right, integer Bottom, integer Border, integer Remove -- clips WinHandle
baSetParent integer WinHandle, integer NewParent -- makes WinHandle a child of NewParent
baRemoveSysItems integer WinHandle, integer SysMenu, integer MinBox, integer MaxBox -- removes system items
baStageHandle -- returns the handle of the Director Stage window
--- Registration Functions---
baSaveRegistration string UserName, integer UserName -- saves your registration information
baGetRegistration -- retrieves your registration information
baFunctions -- retrieves the number of functions you are licensed to use

 
 */

namespace Director {

const char *BawindowsXtra::xlibName = "Bawindows";
const XlibFileDesc BawindowsXtra::fileNames[] = {
	{ "bawindows",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BawindowsXtra::m_new,		 0, 0,	600 },
	{ "baCpuInfo",				BawindowsXtra::m_baCpuInfo,		 0, 0,	600 },
	{ "baFlushIni",				BawindowsXtra::m_baFlushIni,		 0, 0,	600 },
	{ "baDeleteIniEntry",				BawindowsXtra::m_baDeleteIniEntry,		 2, 2,	600 },
	{ "baDeleteIniSection",				BawindowsXtra::m_baDeleteIniSection,		 1, 1,	600 },
	{ "baReadRegString",				BawindowsXtra::m_baReadRegString,		 3, 3,	600 },
	{ "baWriteRegString",				BawindowsXtra::m_baWriteRegString,		 3, 3,	600 },
	{ "baReadRegNumber",				BawindowsXtra::m_baReadRegNumber,		 3, 3,	600 },
	{ "baWriteRegNumber",				BawindowsXtra::m_baWriteRegNumber,		 3, 3,	600 },
	{ "baReadRegBinary",				BawindowsXtra::m_baReadRegBinary,		 3, 3,	600 },
	{ "baWriteRegBinary",				BawindowsXtra::m_baWriteRegBinary,		 3, 3,	600 },
	{ "baReadRegMulti",				BawindowsXtra::m_baReadRegMulti,		 3, 3,	600 },
	{ "baWriteRegMulti",				BawindowsXtra::m_baWriteRegMulti,		 3, 3,	600 },
	{ "baDeleteReg",				BawindowsXtra::m_baDeleteReg,		 2, 2,	600 },
	{ "baRegKeyList",				BawindowsXtra::m_baRegKeyList,		 1, 1,	600 },
	{ "baRegValueList",				BawindowsXtra::m_baRegValueList,		 1, 1,	600 },
	{ "baSoundCard",				BawindowsXtra::m_baSoundCard,		 -1, -1,	600 },
	{ "baFontList",				BawindowsXtra::m_baFontList,		 0, 0,	600 },
	{ "baFontStyleList",				BawindowsXtra::m_baFontStyleList,		 0, 0,	600 },
	{ "baCommandArgs",				BawindowsXtra::m_baCommandArgs,		 -1, -1,	600 },
	{ "baPrevious",				BawindowsXtra::m_baPrevious,		 0, 0,	600 },
	{ "baScreenInfo",				BawindowsXtra::m_baScreenInfo,		 0, 0,	600 },
	{ "baDisableDiskErrors",				BawindowsXtra::m_baDisableDiskErrors,		 0, 0,	600 },
	{ "baDisableKeys",				BawindowsXtra::m_baDisableKeys,		 1, 1,	600 },
	{ "baDisableMouse",				BawindowsXtra::m_baDisableMouse,		 1, 1,	600 },
	{ "baDisableSwitching",				BawindowsXtra::m_baDisableSwitching,		 0, 0,	600 },
	{ "baDisableScreenSaver",				BawindowsXtra::m_baDisableScreenSaver,		 0, 0,	600 },
	{ "baScreenSaverTime",				BawindowsXtra::m_baScreenSaverTime,		 0, 0,	600 },
	{ "baSetPattern",				BawindowsXtra::m_baSetPattern,		 1, 1,	600 },
	{ "baExitWindows",				BawindowsXtra::m_baExitWindows,		 0, 0,	600 },
	{ "baWinHelp",				BawindowsXtra::m_baWinHelp,		 2, 2,	600 },
	{ "baSetCurrentDir",				BawindowsXtra::m_baSetCurrentDir,		 0, 0,	600 },
	{ "baRestrictCursor",				BawindowsXtra::m_baRestrictCursor,		 3, 3,	600 },
	{ "baFreeCursor",				BawindowsXtra::m_baFreeCursor,		 -1, -1,	600 },
	{ "baInstallFont",				BawindowsXtra::m_baInstallFont,		 1, 1,	600 },
	{ "baKeyBeenPressed",				BawindowsXtra::m_baKeyBeenPressed,		 0, 0,	600 },
	{ "baSleep",				BawindowsXtra::m_baSleep,		 0, 0,	600 },
	{ "baSetDisplayEx",				BawindowsXtra::m_baSetDisplayEx,		 5, 5,	600 },
	{ "baEnvironment",				BawindowsXtra::m_baEnvironment,		 0, 0,	600 },
	{ "baSetEnvironment",				BawindowsXtra::m_baSetEnvironment,		 1, 1,	600 },
	{ "baCreatePMGroup",				BawindowsXtra::m_baCreatePMGroup,		 0, 0,	600 },
	{ "baDeletePMGroup",				BawindowsXtra::m_baDeletePMGroup,		 0, 0,	600 },
	{ "baCreatePMIcon",				BawindowsXtra::m_baCreatePMIcon,		 3, 3,	600 },
	{ "baDeletePMIcon",				BawindowsXtra::m_baDeletePMIcon,		 0, 0,	600 },
	{ "baPMGroupList",				BawindowsXtra::m_baPMGroupList,		 -1, -1,	600 },
	{ "baPMIconList",				BawindowsXtra::m_baPMIconList,		 0, 0,	600 },
	{ "baPMSubGroupList",				BawindowsXtra::m_baPMSubGroupList,		 0, 0,	600 },
	{ "baPrinterInfo",				BawindowsXtra::m_baPrinterInfo,		 0, 0,	600 },
	{ "baSetPrinter",				BawindowsXtra::m_baSetPrinter,		 1, 1,	600 },
	{ "baPrintDlg",				BawindowsXtra::m_baPrintDlg,		 0, 0,	600 },
	{ "baPageSetupDlg",				BawindowsXtra::m_baPageSetupDlg,		 0, 0,	600 },
	{ "baRefreshDesktop",				BawindowsXtra::m_baRefreshDesktop,		 0, 0,	600 },
	{ "baFileDate",				BawindowsXtra::m_baFileDate,		 2, 2,	600 },
	{ "baFileDateEx",				BawindowsXtra::m_baFileDateEx,		 3, 3,	600 },
	{ "baSetFileDate",				BawindowsXtra::m_baSetFileDate,		 6, 6,	600 },
	{ "baRecycleFile",				BawindowsXtra::m_baRecycleFile,		 0, 0,	600 },
	{ "baFindDrive",				BawindowsXtra::m_baFindDrive,		 1, 1,	600 },
	{ "baShell",				BawindowsXtra::m_baShell,		 4, 4,	600 },
	{ "baTempFileName",				BawindowsXtra::m_baTempFileName,		 0, 0,	600 },
	{ "baMakeShortcutEx",				BawindowsXtra::m_baMakeShortcutEx,		 8, 8,	600 },
	{ "baResolveShortcut",				BawindowsXtra::m_baResolveShortcut,		 0, 0,	600 },
	{ "baCloseApp",				BawindowsXtra::m_baCloseApp,		 0, 0,	600 },
	{ "baSetWindowState",				BawindowsXtra::m_baSetWindowState,		 1, 1,	600 },
	{ "baWaitTillActive",				BawindowsXtra::m_baWaitTillActive,		 0, 0,	600 },
	{ "baChildWindowList",				BawindowsXtra::m_baChildWindowList,		 3, 3,	600 },
	{ "baWindowDepth",				BawindowsXtra::m_baWindowDepth,		 0, 0,	600 },
	{ "baSetWindowDepth",				BawindowsXtra::m_baSetWindowDepth,		 1, 1,	600 },
	{ "baSendMsg",				BawindowsXtra::m_baSendMsg,		 4, 4,	600 },
	{ "baAddSysItems",				BawindowsXtra::m_baAddSysItems,		 3, 3,	600 },
	{ "baClipWindow",				BawindowsXtra::m_baClipWindow,		 6, 6,	600 },
	{ "baSetParent",				BawindowsXtra::m_baSetParent,		 1, 1,	600 },
	{ "baRemoveSysItems",				BawindowsXtra::m_baRemoveSysItems,		 3, 3,	600 },
	{ "baStageHandle",				BawindowsXtra::m_baStageHandle,		 -1, -1,	600 },
	{ "baSaveRegistration",				BawindowsXtra::m_baSaveRegistration,		 1, 1,	600 },
	{ "baGetRegistration",				BawindowsXtra::m_baGetRegistration,		 -1, -1,	600 },
	{ "baFunctions",				BawindowsXtra::m_baFunctions,		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BawindowsXtraObject::BawindowsXtraObject(ObjectType ObjectType) :Object<BawindowsXtraObject>("Bawindows") {
	_objType = ObjectType;
}

bool BawindowsXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BawindowsXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BawindowsXtra::xlibName);
	warning("BawindowsXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BawindowsXtra::open(ObjectType type, const Common::Path &path) {
    BawindowsXtraObject::initMethods(xlibMethods);
    BawindowsXtraObject *xobj = new BawindowsXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BawindowsXtra::close(ObjectType type) {
    BawindowsXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BawindowsXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BawindowsXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BawindowsXtra::m_baCpuInfo, 0)
XOBJSTUB(BawindowsXtra::m_baFlushIni, 0)
XOBJSTUB(BawindowsXtra::m_baDeleteIniEntry, 0)
XOBJSTUB(BawindowsXtra::m_baDeleteIniSection, 0)
XOBJSTUB(BawindowsXtra::m_baReadRegString, 0)
XOBJSTUB(BawindowsXtra::m_baWriteRegString, 0)
XOBJSTUB(BawindowsXtra::m_baReadRegNumber, 0)
XOBJSTUB(BawindowsXtra::m_baWriteRegNumber, 0)
XOBJSTUB(BawindowsXtra::m_baReadRegBinary, 0)
XOBJSTUB(BawindowsXtra::m_baWriteRegBinary, 0)
XOBJSTUB(BawindowsXtra::m_baReadRegMulti, 0)
XOBJSTUB(BawindowsXtra::m_baWriteRegMulti, 0)
XOBJSTUB(BawindowsXtra::m_baDeleteReg, 0)
XOBJSTUB(BawindowsXtra::m_baRegKeyList, 0)
XOBJSTUB(BawindowsXtra::m_baRegValueList, 0)
XOBJSTUB(BawindowsXtra::m_baSoundCard, 0)
XOBJSTUB(BawindowsXtra::m_baFontList, 0)
XOBJSTUB(BawindowsXtra::m_baFontStyleList, 0)
XOBJSTUB(BawindowsXtra::m_baCommandArgs, 0)
XOBJSTUB(BawindowsXtra::m_baPrevious, 0)
XOBJSTUB(BawindowsXtra::m_baScreenInfo, 0)
XOBJSTUB(BawindowsXtra::m_baDisableDiskErrors, 0)
XOBJSTUB(BawindowsXtra::m_baDisableKeys, 0)
XOBJSTUB(BawindowsXtra::m_baDisableMouse, 0)
XOBJSTUB(BawindowsXtra::m_baDisableSwitching, 0)
XOBJSTUB(BawindowsXtra::m_baDisableScreenSaver, 0)
XOBJSTUB(BawindowsXtra::m_baScreenSaverTime, 0)
XOBJSTUB(BawindowsXtra::m_baSetPattern, 0)
XOBJSTUB(BawindowsXtra::m_baExitWindows, 0)
XOBJSTUB(BawindowsXtra::m_baWinHelp, 0)
XOBJSTUB(BawindowsXtra::m_baSetCurrentDir, 0)
XOBJSTUB(BawindowsXtra::m_baRestrictCursor, 0)
XOBJSTUB(BawindowsXtra::m_baFreeCursor, 0)
XOBJSTUB(BawindowsXtra::m_baInstallFont, 0)
XOBJSTUB(BawindowsXtra::m_baKeyBeenPressed, 0)
XOBJSTUB(BawindowsXtra::m_baSleep, 0)
XOBJSTUB(BawindowsXtra::m_baSetDisplayEx, 0)
XOBJSTUB(BawindowsXtra::m_baEnvironment, 0)
XOBJSTUB(BawindowsXtra::m_baSetEnvironment, 0)
XOBJSTUB(BawindowsXtra::m_baCreatePMGroup, 0)
XOBJSTUB(BawindowsXtra::m_baDeletePMGroup, 0)
XOBJSTUB(BawindowsXtra::m_baCreatePMIcon, 0)
XOBJSTUB(BawindowsXtra::m_baDeletePMIcon, 0)
XOBJSTUB(BawindowsXtra::m_baPMGroupList, 0)
XOBJSTUB(BawindowsXtra::m_baPMIconList, 0)
XOBJSTUB(BawindowsXtra::m_baPMSubGroupList, 0)
XOBJSTUB(BawindowsXtra::m_baPrinterInfo, 0)
XOBJSTUB(BawindowsXtra::m_baSetPrinter, 0)
XOBJSTUB(BawindowsXtra::m_baPrintDlg, 0)
XOBJSTUB(BawindowsXtra::m_baPageSetupDlg, 0)
XOBJSTUB(BawindowsXtra::m_baRefreshDesktop, 0)
XOBJSTUB(BawindowsXtra::m_baFileDate, 0)
XOBJSTUB(BawindowsXtra::m_baFileDateEx, 0)
XOBJSTUB(BawindowsXtra::m_baSetFileDate, 0)
XOBJSTUB(BawindowsXtra::m_baRecycleFile, 0)
XOBJSTUB(BawindowsXtra::m_baFindDrive, 0)
XOBJSTUB(BawindowsXtra::m_baShell, 0)
XOBJSTUB(BawindowsXtra::m_baTempFileName, 0)
XOBJSTUB(BawindowsXtra::m_baMakeShortcutEx, 0)
XOBJSTUB(BawindowsXtra::m_baResolveShortcut, 0)
XOBJSTUB(BawindowsXtra::m_baCloseApp, 0)
XOBJSTUB(BawindowsXtra::m_baSetWindowState, 0)
XOBJSTUB(BawindowsXtra::m_baWaitTillActive, 0)
XOBJSTUB(BawindowsXtra::m_baChildWindowList, 0)
XOBJSTUB(BawindowsXtra::m_baWindowDepth, 0)
XOBJSTUB(BawindowsXtra::m_baSetWindowDepth, 0)
XOBJSTUB(BawindowsXtra::m_baSendMsg, 0)
XOBJSTUB(BawindowsXtra::m_baAddSysItems, 0)
XOBJSTUB(BawindowsXtra::m_baClipWindow, 0)
XOBJSTUB(BawindowsXtra::m_baSetParent, 0)
XOBJSTUB(BawindowsXtra::m_baRemoveSysItems, 0)
XOBJSTUB(BawindowsXtra::m_baStageHandle, 0)
XOBJSTUB(BawindowsXtra::m_baSaveRegistration, 0)
XOBJSTUB(BawindowsXtra::m_baGetRegistration, 0)
XOBJSTUB(BawindowsXtra::m_baFunctions, 0)

}
