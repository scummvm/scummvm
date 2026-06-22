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

#ifndef DIRECTOR_LINGO_XTRAS_B_BAWINDOWS_H
#define DIRECTOR_LINGO_XTRAS_B_BAWINDOWS_H

namespace Director {

class BawindowsXtraObject : public Object<BawindowsXtraObject> {
public:
	BawindowsXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BawindowsXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_baCpuInfo(int nargs);
void m_baFlushIni(int nargs);
void m_baDeleteIniEntry(int nargs);
void m_baDeleteIniSection(int nargs);
void m_baReadRegString(int nargs);
void m_baWriteRegString(int nargs);
void m_baReadRegNumber(int nargs);
void m_baWriteRegNumber(int nargs);
void m_baReadRegBinary(int nargs);
void m_baWriteRegBinary(int nargs);
void m_baReadRegMulti(int nargs);
void m_baWriteRegMulti(int nargs);
void m_baDeleteReg(int nargs);
void m_baRegKeyList(int nargs);
void m_baRegValueList(int nargs);
void m_baSoundCard(int nargs);
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
void m_baSetPattern(int nargs);
void m_baExitWindows(int nargs);
void m_baWinHelp(int nargs);
void m_baSetCurrentDir(int nargs);
void m_baRestrictCursor(int nargs);
void m_baFreeCursor(int nargs);
void m_baInstallFont(int nargs);
void m_baKeyBeenPressed(int nargs);
void m_baSleep(int nargs);
void m_baSetDisplayEx(int nargs);
void m_baEnvironment(int nargs);
void m_baSetEnvironment(int nargs);
void m_baCreatePMGroup(int nargs);
void m_baDeletePMGroup(int nargs);
void m_baCreatePMIcon(int nargs);
void m_baDeletePMIcon(int nargs);
void m_baPMGroupList(int nargs);
void m_baPMIconList(int nargs);
void m_baPMSubGroupList(int nargs);
void m_baPrinterInfo(int nargs);
void m_baSetPrinter(int nargs);
void m_baPrintDlg(int nargs);
void m_baPageSetupDlg(int nargs);
void m_baRefreshDesktop(int nargs);
void m_baFileDate(int nargs);
void m_baFileDateEx(int nargs);
void m_baSetFileDate(int nargs);
void m_baRecycleFile(int nargs);
void m_baFindDrive(int nargs);
void m_baShell(int nargs);
void m_baTempFileName(int nargs);
void m_baMakeShortcutEx(int nargs);
void m_baResolveShortcut(int nargs);
void m_baCloseApp(int nargs);
void m_baSetWindowState(int nargs);
void m_baWaitTillActive(int nargs);
void m_baChildWindowList(int nargs);
void m_baWindowDepth(int nargs);
void m_baSetWindowDepth(int nargs);
void m_baSendMsg(int nargs);
void m_baAddSysItems(int nargs);
void m_baClipWindow(int nargs);
void m_baSetParent(int nargs);
void m_baRemoveSysItems(int nargs);
void m_baStageHandle(int nargs);
void m_baSaveRegistration(int nargs);
void m_baGetRegistration(int nargs);
void m_baFunctions(int nargs);

} // End of namespace BawindowsXtra

} // End of namespace Director

#endif
