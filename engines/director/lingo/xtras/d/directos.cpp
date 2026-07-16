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
#include "director/lingo/xtras/d/directos.h"

/**************************************************
 *
 * USED IN:
 * Winnie the Pooh Kindergarten
 * tokimemotypegs
 *
 **************************************************/

/*
-- xtra DirectOS
new object me
--
-- DirectOS Xtra(TM) version 2.01
--
-- Developed by Omri Kalinsky and Tomer Berda, DirectXtras Inc.
-- Copyright 1997-1999.
--
-- General Methods
--
* dosRegister integer SerialNumber
* dosGetLastError
--
-- System Methods
--
* dosGetVersion
* dosSetSystemDate string NewDate
* dosSetSystemTime string NewTime
* dosIsSoundCardInstalled
* dosLaunchApp string Application, string State, integer Wait
* dosGetDisplay
* dosSetDisplay integer Width, integer Height, integer Depth, string Mode
* dosSetCursorPos integer X, integer Y
* dosMouseClick string Type
* dosGetDoubleClickTime
* dosGetKeyState integer KeyCode
* dosMessageBox string Text, string Caption, string Buttons, string Icon, string DefaultButton
* dosGetPrinter string Info
* dosExitSystem string Action
* dosEncryptText string Text, string Key
* dosDecryptText string Text, string Key
--
-- File Methods
--
* dosFindApp string FileType
* dosGetTempFileName
* dosSelectFile string Operation, string StartFolder, string FileName, any FileTypes, string Message
* dosGetSpecialFolder string Folder
* dosGetCurrentFolder
* dosSetCurrentFolder string PathName
* dosGetDriveInfo string Drive, string Info
* dosIsFileExist string FileName
* dosGetFileAttribute string FileName, string Attribute
* dosSetFileAttribute string FileName, string Attribute, integer Value
* dosGetFileSize string FileName
* dosGetFileDate string FileName, string Info
* dosGetFileTime string FileName, string Info
* dosGetFileVersion string FileName
* dosOpenFile string FileName, string State, integer Wait
* dosPrintFile string FileName
* dosMoveFile string SourceFileName, string TargetFileName
* dosCopyFile string SourceFileName, string TargetFileName
* dosDeleteFile string FileName
* dosTrashFile string FileName
* dosIsFolderExist string PathName
* dosGetFilesInFolder string PathName
* dosGetFoldersInFolder string PathName
* dosCreateFolder string PathName
* dosDeleteFolder string PathName
* dosCreateLink string LinkAtName, string LinkToName, string Args, string WorkingFolder, string IconLocation
* dosResolveLink string FileName
* dosReadINI string FileName, string SectionName, string KeyName
* dosWriteINI string FileName, string SectionName, string KeyName, string Data
* dosDeleteINI string FileName, string SectionName, string KeyName
--
-- Windowing Methods
--
* dosGetTopWindowIDs
* dosGetAppWindowID
* dosGetParentWindowID integer WindowID
* dosGetChildWindowIDs integer WindowID
* dosGetFrontWindowID
* dosSetFrontWindow integer WindowID
* dosIsWindowExist integer WindowID
* dosGetWindowName integer WindowID
* dosSetWindowName integer WindowID, string NewName
* dosSetWindowState integer WindowID, string State
* dosCloseWindow integer WindowID
* dosGetWindowRect integer WindowID
* dosSetWindowRect integer WindowID, integer Left, integer Top, integer Right, integer Bottom
--
-- Windows Only Methods
--
* dosReadReg string Key, string SubKey, string ValueName
* dosWriteReg string Key, string SubKey, string ValueName, any Value
* dosDeleteReg string Key, string SubKey, string ValueName
* dosKeyListReg string Key, string SubKey
* dosValueListReg string Key, string SubKey
* dosSetPattern any Pattern
* dosSetWallpaper string FileName
* dosGetScreenSaver
* dosSetScreenSaver string FileName
* dosGetScreenSaverTimeOut
* dosSetScreenSaverTimeOut integer Seconds
* dosGetScreenSaverActive
* dosSetScreenSaverActive integer Active
* dosClipCursor integer Left, integer Top, integer Right, integer Bottom
* dosReleaseCursor
* dosDisableSystemKeys integer State
* dosSetPrinter string Info, any Setting
* dosGetCommandLine
* dosGetShortFileName string FileName
--
-- MacOS Only Methods
--
* dosGetFileType string FileName
* dosGetFileCreator string FileName
 */

namespace Director {

const char *DirectOSXtra::xlibName = "DirectOS";
const XlibFileDesc DirectOSXtra::fileNames[] = {
	{ "directos",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DirectOSXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "dosRegister", DirectOSXtra::m_dosRegister, 1, 1, 500, HBLTIN },
	{ "dosGetLastError", DirectOSXtra::m_dosGetLastError, 0, 0, 500, HBLTIN },
	{ "dosGetVersion", DirectOSXtra::m_dosGetVersion, 0, 0, 500, HBLTIN },
	{ "dosSetSystemDate", DirectOSXtra::m_dosSetSystemDate, 1, 1, 500, HBLTIN },
	{ "dosSetSystemTime", DirectOSXtra::m_dosSetSystemTime, 1, 1, 500, HBLTIN },
	{ "dosIsSoundCardInstalled", DirectOSXtra::m_dosIsSoundCardInstalled, 0, 0, 500, HBLTIN },
	{ "dosLaunchApp", DirectOSXtra::m_dosLaunchApp, 3, 3, 500, HBLTIN },
	{ "dosGetDisplay", DirectOSXtra::m_dosGetDisplay, 0, 0, 500, HBLTIN },
	{ "dosSetDisplay", DirectOSXtra::m_dosSetDisplay, 4, 4, 500, HBLTIN },
	{ "dosSetCursorPos", DirectOSXtra::m_dosSetCursorPos, 2, 2, 500, HBLTIN },
	{ "dosMouseClick", DirectOSXtra::m_dosMouseClick, 1, 1, 500, HBLTIN },
	{ "dosGetDoubleClickTime", DirectOSXtra::m_dosGetDoubleClickTime, 0, 0, 500, HBLTIN },
	{ "dosGetKeyState", DirectOSXtra::m_dosGetKeyState, 1, 1, 500, HBLTIN },
	{ "dosMessageBox", DirectOSXtra::m_dosMessageBox, 5, 5, 500, HBLTIN },
	{ "dosGetPrinter", DirectOSXtra::m_dosGetPrinter, 1, 1, 500, HBLTIN },
	{ "dosExitSystem", DirectOSXtra::m_dosExitSystem, 1, 1, 500, HBLTIN },
	{ "dosEncryptText", DirectOSXtra::m_dosEncryptText, 2, 2, 500, HBLTIN },
	{ "dosDecryptText", DirectOSXtra::m_dosDecryptText, 2, 2, 500, HBLTIN },
	{ "dosFindApp", DirectOSXtra::m_dosFindApp, 1, 1, 500, HBLTIN },
	{ "dosGetTempFileName", DirectOSXtra::m_dosGetTempFileName, 0, 0, 500, HBLTIN },
	{ "dosSelectFile", DirectOSXtra::m_dosSelectFile, 5, 5, 500, HBLTIN },
	{ "dosGetSpecialFolder", DirectOSXtra::m_dosGetSpecialFolder, 1, 1, 500, HBLTIN },
	{ "dosGetCurrentFolder", DirectOSXtra::m_dosGetCurrentFolder, 0, 0, 500, HBLTIN },
	{ "dosSetCurrentFolder", DirectOSXtra::m_dosSetCurrentFolder, 1, 1, 500, HBLTIN },
	{ "dosGetDriveInfo", DirectOSXtra::m_dosGetDriveInfo, 2, 2, 500, HBLTIN },
	{ "dosIsFileExist", DirectOSXtra::m_dosIsFileExist, 1, 1, 500, HBLTIN },
	{ "dosGetFileAttribute", DirectOSXtra::m_dosGetFileAttribute, 2, 2, 500, HBLTIN },
	{ "dosSetFileAttribute", DirectOSXtra::m_dosSetFileAttribute, 3, 3, 500, HBLTIN },
	{ "dosGetFileSize", DirectOSXtra::m_dosGetFileSize, 1, 1, 500, HBLTIN },
	{ "dosGetFileDate", DirectOSXtra::m_dosGetFileDate, 2, 2, 500, HBLTIN },
	{ "dosGetFileTime", DirectOSXtra::m_dosGetFileTime, 2, 2, 500, HBLTIN },
	{ "dosGetFileVersion", DirectOSXtra::m_dosGetFileVersion, 1, 1, 500, HBLTIN },
	{ "dosOpenFile", DirectOSXtra::m_dosOpenFile, 3, 3, 500, HBLTIN },
	{ "dosPrintFile", DirectOSXtra::m_dosPrintFile, 1, 1, 500, HBLTIN },
	{ "dosMoveFile", DirectOSXtra::m_dosMoveFile, 2, 2, 500, HBLTIN },
	{ "dosCopyFile", DirectOSXtra::m_dosCopyFile, 2, 2, 500, HBLTIN },
	{ "dosDeleteFile", DirectOSXtra::m_dosDeleteFile, 1, 1, 500, HBLTIN },
	{ "dosTrashFile", DirectOSXtra::m_dosTrashFile, 1, 1, 500, HBLTIN },
	{ "dosIsFolderExist", DirectOSXtra::m_dosIsFolderExist, 1, 1, 500, HBLTIN },
	{ "dosGetFilesInFolder", DirectOSXtra::m_dosGetFilesInFolder, 1, 1, 500, HBLTIN },
	{ "dosGetFoldersInFolder", DirectOSXtra::m_dosGetFoldersInFolder, 1, 1, 500, HBLTIN },
	{ "dosCreateFolder", DirectOSXtra::m_dosCreateFolder, 1, 1, 500, HBLTIN },
	{ "dosDeleteFolder", DirectOSXtra::m_dosDeleteFolder, 1, 1, 500, HBLTIN },
	{ "dosCreateLink", DirectOSXtra::m_dosCreateLink, 5, 5, 500, HBLTIN },
	{ "dosResolveLink", DirectOSXtra::m_dosResolveLink, 1, 1, 500, HBLTIN },
	{ "dosReadINI", DirectOSXtra::m_dosReadINI, 3, 3, 500, HBLTIN },
	{ "dosWriteINI", DirectOSXtra::m_dosWriteINI, 4, 4, 500, HBLTIN },
	{ "dosDeleteINI", DirectOSXtra::m_dosDeleteINI, 3, 3, 500, HBLTIN },
	{ "dosGetTopWindowIDs", DirectOSXtra::m_dosGetTopWindowIDs, 0, 0, 500, HBLTIN },
	{ "dosGetAppWindowID", DirectOSXtra::m_dosGetAppWindowID, 0, 0, 500, HBLTIN },
	{ "dosGetParentWindowID", DirectOSXtra::m_dosGetParentWindowID, 1, 1, 500, HBLTIN },
	{ "dosGetChildWindowIDs", DirectOSXtra::m_dosGetChildWindowIDs, 1, 1, 500, HBLTIN },
	{ "dosGetFrontWindowID", DirectOSXtra::m_dosGetFrontWindowID, 0, 0, 500, HBLTIN },
	{ "dosSetFrontWindow", DirectOSXtra::m_dosSetFrontWindow, 1, 1, 500, HBLTIN },
	{ "dosIsWindowExist", DirectOSXtra::m_dosIsWindowExist, 1, 1, 500, HBLTIN },
	{ "dosGetWindowName", DirectOSXtra::m_dosGetWindowName, 1, 1, 500, HBLTIN },
	{ "dosSetWindowName", DirectOSXtra::m_dosSetWindowName, 2, 2, 500, HBLTIN },
	{ "dosSetWindowState", DirectOSXtra::m_dosSetWindowState, 2, 2, 500, HBLTIN },
	{ "dosCloseWindow", DirectOSXtra::m_dosCloseWindow, 1, 1, 500, HBLTIN },
	{ "dosGetWindowRect", DirectOSXtra::m_dosGetWindowRect, 1, 1, 500, HBLTIN },
	{ "dosSetWindowRect", DirectOSXtra::m_dosSetWindowRect, 5, 5, 500, HBLTIN },
	{ "dosReadReg", DirectOSXtra::m_dosReadReg, 3, 3, 500, HBLTIN },
	{ "dosWriteReg", DirectOSXtra::m_dosWriteReg, 4, 4, 500, HBLTIN },
	{ "dosDeleteReg", DirectOSXtra::m_dosDeleteReg, 3, 3, 500, HBLTIN },
	{ "dosKeyListReg", DirectOSXtra::m_dosKeyListReg, 2, 2, 500, HBLTIN },
	{ "dosValueListReg", DirectOSXtra::m_dosValueListReg, 2, 2, 500, HBLTIN },
	{ "dosSetPattern", DirectOSXtra::m_dosSetPattern, 1, 1, 500, HBLTIN },
	{ "dosSetWallpaper", DirectOSXtra::m_dosSetWallpaper, 1, 1, 500, HBLTIN },
	{ "dosGetScreenSaver", DirectOSXtra::m_dosGetScreenSaver, 0, 0, 500, HBLTIN },
	{ "dosSetScreenSaver", DirectOSXtra::m_dosSetScreenSaver, 1, 1, 500, HBLTIN },
	{ "dosGetScreenSaverTimeOut", DirectOSXtra::m_dosGetScreenSaverTimeOut, 0, 0, 500, HBLTIN },
	{ "dosSetScreenSaverTimeOut", DirectOSXtra::m_dosSetScreenSaverTimeOut, 1, 1, 500, HBLTIN },
	{ "dosGetScreenSaverActive", DirectOSXtra::m_dosGetScreenSaverActive, 0, 0, 500, HBLTIN },
	{ "dosSetScreenSaverActive", DirectOSXtra::m_dosSetScreenSaverActive, 1, 1, 500, HBLTIN },
	{ "dosClipCursor", DirectOSXtra::m_dosClipCursor, 4, 4, 500, HBLTIN },
	{ "dosReleaseCursor", DirectOSXtra::m_dosReleaseCursor, 0, 0, 500, HBLTIN },
	{ "dosDisableSystemKeys", DirectOSXtra::m_dosDisableSystemKeys, 1, 1, 500, HBLTIN },
	{ "dosSetPrinter", DirectOSXtra::m_dosSetPrinter, 2, 2, 500, HBLTIN },
	{ "dosGetCommandLine", DirectOSXtra::m_dosGetCommandLine, 0, 0, 500, HBLTIN },
	{ "dosGetShortFileName", DirectOSXtra::m_dosGetShortFileName, 1, 1, 500, HBLTIN },
	{ "dosGetFileType", DirectOSXtra::m_dosGetFileType, 1, 1, 500, HBLTIN },
	{ "dosGetFileCreator", DirectOSXtra::m_dosGetFileCreator, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirectOSXtraObject::DirectOSXtraObject(ObjectType ObjectType) :Object<DirectOSXtraObject>("DirectOS") {
	_objType = ObjectType;
}

bool DirectOSXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirectOSXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirectOSXtra::xlibName);
	warning("DirectOSXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirectOSXtra::open(ObjectType type, const Common::Path &path) {
    DirectOSXtraObject::initMethods(xlibMethods);
    DirectOSXtraObject *xobj = new DirectOSXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DirectOSXtra::close(ObjectType type) {
    DirectOSXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DirectOSXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirectOSXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DirectOSXtra::m_dosRegister, 0)
XOBJSTUB(DirectOSXtra::m_dosGetLastError, 0)
XOBJSTUB(DirectOSXtra::m_dosGetVersion, 0)
XOBJSTUB(DirectOSXtra::m_dosSetSystemDate, 0)
XOBJSTUB(DirectOSXtra::m_dosSetSystemTime, 0)
XOBJSTUB(DirectOSXtra::m_dosIsSoundCardInstalled, 0)
XOBJSTUB(DirectOSXtra::m_dosLaunchApp, 0)
XOBJSTUB(DirectOSXtra::m_dosGetDisplay, 0)
XOBJSTUB(DirectOSXtra::m_dosSetDisplay, 0)
XOBJSTUB(DirectOSXtra::m_dosSetCursorPos, 0)
XOBJSTUB(DirectOSXtra::m_dosMouseClick, 0)
XOBJSTUB(DirectOSXtra::m_dosGetDoubleClickTime, 0)
XOBJSTUB(DirectOSXtra::m_dosGetKeyState, 0)
XOBJSTUB(DirectOSXtra::m_dosMessageBox, 0)
XOBJSTUB(DirectOSXtra::m_dosGetPrinter, 0)
XOBJSTUB(DirectOSXtra::m_dosExitSystem, 0)
XOBJSTUB(DirectOSXtra::m_dosEncryptText, 0)
XOBJSTUB(DirectOSXtra::m_dosDecryptText, 0)
XOBJSTUB(DirectOSXtra::m_dosFindApp, 0)
XOBJSTUB(DirectOSXtra::m_dosGetTempFileName, 0)
XOBJSTUB(DirectOSXtra::m_dosSelectFile, 0)
XOBJSTUB(DirectOSXtra::m_dosGetSpecialFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosGetCurrentFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosSetCurrentFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosGetDriveInfo, 0)
XOBJSTUB(DirectOSXtra::m_dosIsFileExist, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileAttribute, 0)
XOBJSTUB(DirectOSXtra::m_dosSetFileAttribute, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileSize, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileDate, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileTime, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileVersion, 0)
XOBJSTUB(DirectOSXtra::m_dosOpenFile, 0)
XOBJSTUB(DirectOSXtra::m_dosPrintFile, 0)
XOBJSTUB(DirectOSXtra::m_dosMoveFile, 0)
XOBJSTUB(DirectOSXtra::m_dosCopyFile, 0)
XOBJSTUB(DirectOSXtra::m_dosDeleteFile, 0)
XOBJSTUB(DirectOSXtra::m_dosTrashFile, 0)
XOBJSTUB(DirectOSXtra::m_dosIsFolderExist, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFilesInFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFoldersInFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosCreateFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosDeleteFolder, 0)
XOBJSTUB(DirectOSXtra::m_dosCreateLink, 0)
XOBJSTUB(DirectOSXtra::m_dosResolveLink, 0)
XOBJSTUB(DirectOSXtra::m_dosReadINI, 0)
XOBJSTUB(DirectOSXtra::m_dosWriteINI, 0)
XOBJSTUB(DirectOSXtra::m_dosDeleteINI, 0)
XOBJSTUB(DirectOSXtra::m_dosGetTopWindowIDs, 0)
XOBJSTUB(DirectOSXtra::m_dosGetAppWindowID, 0)
XOBJSTUB(DirectOSXtra::m_dosGetParentWindowID, 0)
XOBJSTUB(DirectOSXtra::m_dosGetChildWindowIDs, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFrontWindowID, 0)
XOBJSTUB(DirectOSXtra::m_dosSetFrontWindow, 0)
XOBJSTUB(DirectOSXtra::m_dosIsWindowExist, 0)
XOBJSTUB(DirectOSXtra::m_dosGetWindowName, 0)
XOBJSTUB(DirectOSXtra::m_dosSetWindowName, 0)
XOBJSTUB(DirectOSXtra::m_dosSetWindowState, 0)
XOBJSTUB(DirectOSXtra::m_dosCloseWindow, 0)
XOBJSTUB(DirectOSXtra::m_dosGetWindowRect, 0)
XOBJSTUB(DirectOSXtra::m_dosSetWindowRect, 0)
XOBJSTUB(DirectOSXtra::m_dosReadReg, 0)
XOBJSTUB(DirectOSXtra::m_dosWriteReg, 0)
XOBJSTUB(DirectOSXtra::m_dosDeleteReg, 0)
XOBJSTUB(DirectOSXtra::m_dosKeyListReg, 0)
XOBJSTUB(DirectOSXtra::m_dosValueListReg, 0)
XOBJSTUB(DirectOSXtra::m_dosSetPattern, 0)
XOBJSTUB(DirectOSXtra::m_dosSetWallpaper, 0)
XOBJSTUB(DirectOSXtra::m_dosGetScreenSaver, 0)
XOBJSTUB(DirectOSXtra::m_dosSetScreenSaver, 0)
XOBJSTUB(DirectOSXtra::m_dosGetScreenSaverTimeOut, 0)
XOBJSTUB(DirectOSXtra::m_dosSetScreenSaverTimeOut, 0)
XOBJSTUB(DirectOSXtra::m_dosGetScreenSaverActive, 0)
XOBJSTUB(DirectOSXtra::m_dosSetScreenSaverActive, 0)
XOBJSTUB(DirectOSXtra::m_dosClipCursor, 0)
XOBJSTUB(DirectOSXtra::m_dosReleaseCursor, 0)
XOBJSTUB(DirectOSXtra::m_dosDisableSystemKeys, 0)
XOBJSTUB(DirectOSXtra::m_dosSetPrinter, 0)
XOBJSTUB(DirectOSXtra::m_dosGetCommandLine, 0)
XOBJSTUB(DirectOSXtra::m_dosGetShortFileName, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileType, 0)
XOBJSTUB(DirectOSXtra::m_dosGetFileCreator, 0)

}
