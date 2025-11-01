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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDAPI_H
#define DIRECTOR_LINGO_XTRAS_B_BUDAPI_H

namespace Director {

class BudAPIXtraObject : public Object<BudAPIXtraObject> {
public:
	BudAPIXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BudAPIXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_baVersion(int nargs);
void m_baSysFolder(int nargs);
void m_baCpuInfo(int nargs);
void m_baDiskInfo(int nargs);
void m_baMemoryInfo(int nargs);
void m_baFindApp(int nargs);
void m_baReadIni(int nargs);
void m_baWriteIni(int nargs);
void m_baFlushIni(int nargs);
void m_baReadRegString(int nargs);
void m_baWriteRegString(int nargs);
void m_baReadRegNumber(int nargs);
void m_baWriteRegNumber(int nargs);
void m_baDeleteReg(int nargs);
void m_baRegKeyList(int nargs);
void m_baRegValueList(int nargs);
void m_baSoundCard(int nargs);
void m_baFontInstalled(int nargs);
void m_baFontList(int nargs);
void m_baFontStyleList(int nargs);
void m_baCommandArgs(int nargs);
void m_baPrevious(int nargs);
void m_baScreenInfo(int nargs);
void m_baDisableDiskErrors(int nargs);
void m_baDisableKeys(int nargs);
void m_baDisableMouse(int nargs);
void m_baDisableSwitching(int nargs);
void m_baDisableScreenSaver(int nargs);
void m_baScreenSaverTime(int nargs);
void m_baSetScreenSaver(int nargs);
void m_baSetWallpaper(int nargs);
void m_baSetPattern(int nargs);
void m_baSetDisplay(int nargs);
void m_baExitWindows(int nargs);
void m_baRunProgram(int nargs);
void m_baWinHelp(int nargs);
void m_baMsgBox(int nargs);
void m_baHideTaskBar(int nargs);
void m_baSetCurrentDir(int nargs);
void m_baCopyText(int nargs);
void m_baPasteText(int nargs);
void m_baEncryptText(int nargs);
void m_baDecryptText(int nargs);
void m_baPlaceCursor(int nargs);
void m_baRestrictCursor(int nargs);
void m_baFreeCursor(int nargs);
void m_baSetVolume(int nargs);
void m_baGetVolume(int nargs);
void m_baInstallFont(int nargs);
void m_baKeyIsDown(int nargs);
void m_baKeyBeenPressed(int nargs);
void m_baSleep(int nargs);
void m_baCreatePMGroup(int nargs);
void m_baDeletePMGroup(int nargs);
void m_baCreatePMIcon(int nargs);
void m_baDeletePMIcon(int nargs);
void m_baPMGroupList(int nargs);
void m_baPMIconList(int nargs);
void m_baPMSubGroupList(int nargs);
void m_baSystemTime(int nargs);
void m_baSetSystemTime(int nargs);
void m_baPrinterInfo(int nargs);
void m_baSetPrinter(int nargs);
void m_baRefreshDesktop(int nargs);
void m_baFileAge(int nargs);
void m_baFileExists(int nargs);
void m_baFolderExists(int nargs);
void m_baCreateFolder(int nargs);
void m_baDeleteFolder(int nargs);
void m_baRenameFile(int nargs);
void m_baDeleteFile(int nargs);
void m_baDeleteXFiles(int nargs);
void m_baXDelete(int nargs);
void m_baFileDate(int nargs);
void m_baFileSize(int nargs);
void m_baFileAttributes(int nargs);
void m_baSetFileAttributes(int nargs);
void m_baRecycleFile(int nargs);
void m_baCopyFile(int nargs);
void m_baCopyXFiles(int nargs);
void m_baXCopy(int nargs);
void m_baFileList(int nargs);
void m_baFolderList(int nargs);
void m_baFindFirstFile(int nargs);
void m_baFindNextFile(int nargs);
void m_baFindClose(int nargs);
void m_baGetFilename(int nargs);
void m_baGetFolder(int nargs);
void m_baFileVersion(int nargs);
void m_baEncryptFile(int nargs);
void m_baFindDrive(int nargs);
void m_baOpenFile(int nargs);
void m_baOpenURL(int nargs);
void m_baPrintFile(int nargs);
void m_baShell(int nargs);
void m_baShortFileName(int nargs);
void m_baTempFileName(int nargs);
void m_baMakeShortcut(int nargs);
void m_baMakeShortcutEx(int nargs);
void m_baResolveShortcut(int nargs);
void m_baWindowInfo(int nargs);
void m_baFindWindow(int nargs);
void m_baActiveWindow(int nargs);
void m_baCloseWindow(int nargs);
void m_baCloseApp(int nargs);
void m_baSetWindowState(int nargs);
void m_baActivateWindow(int nargs);
void m_baSetWindowTitle(int nargs);
void m_baMoveWindow(int nargs);
void m_baWindowToFront(int nargs);
void m_baWindowToBack(int nargs);
void m_baGetWindow(int nargs);
void m_baWaitTillActive(int nargs);
void m_baWaitForWindow(int nargs);
void m_baNextActiveWindow(int nargs);
void m_baWindowExists(int nargs);
void m_baWindowList(int nargs);
void m_baChildWindowList(int nargs);
void m_baWindowDepth(int nargs);
void m_baSetWindowDepth(int nargs);
void m_baSendKeys(int nargs);
void m_baSendMsg(int nargs);
void m_baAddSysItems(int nargs);
void m_baRemoveSysItems(int nargs);
void m_baWinHandle(int nargs);
void m_baStageHandle(int nargs);
void m_baAbout(int nargs);
void m_baRegister(int nargs);
void m_baSaveRegistration(int nargs);
void m_baGetRegistration(int nargs);
void m_baFunctions(int nargs);

} // End of namespace BudAPIXtra

} // End of namespace Director

#endif
