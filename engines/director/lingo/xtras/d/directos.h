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

#ifndef DIRECTOR_LINGO_XTRAS_D_DIRECTOS_H
#define DIRECTOR_LINGO_XTRAS_D_DIRECTOS_H

namespace Director {

class DirectOSXtraObject : public Object<DirectOSXtraObject> {
public:
	DirectOSXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace DirectOSXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dosRegister(int nargs);
void m_dosGetLastError(int nargs);
void m_dosGetVersion(int nargs);
void m_dosSetSystemDate(int nargs);
void m_dosSetSystemTime(int nargs);
void m_dosIsSoundCardInstalled(int nargs);
void m_dosLaunchApp(int nargs);
void m_dosGetDisplay(int nargs);
void m_dosSetDisplay(int nargs);
void m_dosSetCursorPos(int nargs);
void m_dosMouseClick(int nargs);
void m_dosGetDoubleClickTime(int nargs);
void m_dosGetKeyState(int nargs);
void m_dosMessageBox(int nargs);
void m_dosGetPrinter(int nargs);
void m_dosExitSystem(int nargs);
void m_dosEncryptText(int nargs);
void m_dosDecryptText(int nargs);
void m_dosFindApp(int nargs);
void m_dosGetTempFileName(int nargs);
void m_dosSelectFile(int nargs);
void m_dosGetSpecialFolder(int nargs);
void m_dosGetCurrentFolder(int nargs);
void m_dosSetCurrentFolder(int nargs);
void m_dosGetDriveInfo(int nargs);
void m_dosIsFileExist(int nargs);
void m_dosGetFileAttribute(int nargs);
void m_dosSetFileAttribute(int nargs);
void m_dosGetFileSize(int nargs);
void m_dosGetFileDate(int nargs);
void m_dosGetFileTime(int nargs);
void m_dosGetFileVersion(int nargs);
void m_dosOpenFile(int nargs);
void m_dosPrintFile(int nargs);
void m_dosMoveFile(int nargs);
void m_dosCopyFile(int nargs);
void m_dosDeleteFile(int nargs);
void m_dosTrashFile(int nargs);
void m_dosIsFolderExist(int nargs);
void m_dosGetFilesInFolder(int nargs);
void m_dosGetFoldersInFolder(int nargs);
void m_dosCreateFolder(int nargs);
void m_dosDeleteFolder(int nargs);
void m_dosCreateLink(int nargs);
void m_dosResolveLink(int nargs);
void m_dosReadINI(int nargs);
void m_dosWriteINI(int nargs);
void m_dosDeleteINI(int nargs);
void m_dosGetTopWindowIDs(int nargs);
void m_dosGetAppWindowID(int nargs);
void m_dosGetParentWindowID(int nargs);
void m_dosGetChildWindowIDs(int nargs);
void m_dosGetFrontWindowID(int nargs);
void m_dosSetFrontWindow(int nargs);
void m_dosIsWindowExist(int nargs);
void m_dosGetWindowName(int nargs);
void m_dosSetWindowName(int nargs);
void m_dosSetWindowState(int nargs);
void m_dosCloseWindow(int nargs);
void m_dosGetWindowRect(int nargs);
void m_dosSetWindowRect(int nargs);
void m_dosReadReg(int nargs);
void m_dosWriteReg(int nargs);
void m_dosDeleteReg(int nargs);
void m_dosKeyListReg(int nargs);
void m_dosValueListReg(int nargs);
void m_dosSetPattern(int nargs);
void m_dosSetWallpaper(int nargs);
void m_dosGetScreenSaver(int nargs);
void m_dosSetScreenSaver(int nargs);
void m_dosGetScreenSaverTimeOut(int nargs);
void m_dosSetScreenSaverTimeOut(int nargs);
void m_dosGetScreenSaverActive(int nargs);
void m_dosSetScreenSaverActive(int nargs);
void m_dosClipCursor(int nargs);
void m_dosReleaseCursor(int nargs);
void m_dosDisableSystemKeys(int nargs);
void m_dosSetPrinter(int nargs);
void m_dosGetCommandLine(int nargs);
void m_dosGetShortFileName(int nargs);
void m_dosGetFileType(int nargs);
void m_dosGetFileCreator(int nargs);

} // End of namespace DirectOSXtra

} // End of namespace Director

#endif
