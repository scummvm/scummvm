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

#ifndef DIRECTOR_LINGO_XTRAS_MASTERAPP_H
#define DIRECTOR_LINGO_XTRAS_MASTERAPP_H

namespace Director {

class MasterAppXtraObject : public Object<MasterAppXtraObject> {
public:
	MasterAppXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace MasterAppXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_register(int nargs);
void m_mappGetTaskList(int nargs);
void m_mappGetTaskIDs(int nargs);
void m_mappTaskName(int nargs);
void m_mappTaskParent(int nargs);
void m_mappTaskInstance(int nargs);
void m_mappTaskModule(int nargs);
void m_mappTaskEvents(int nargs);
void m_mappGetTaskWindowList(int nargs);
void m_mappGetTaskWindowIDs(int nargs);
void m_mappGetWindowTask(int nargs);
void m_mappRudeQuitTask(int nargs);
void m_mappTaskIsRunning(int nargs);
void m_mappFeedTimeSlice(int nargs);
void m_mappFeedGenericTimeSlice(int nargs);
void m_mappHostAppTask(int nargs);
void m_mappDirectorTask(int nargs);
void m_mappAuthorwareTask(int nargs);
void m_mappGetTaskOrder(int nargs);
void m_mappGetThreadList(int nargs);
void m_mappGetTaskThreads(int nargs);
void m_mappGetThreadWindowList(int nargs);
void m_mappGetThreadWindowIDs(int nargs);
void m_mappGetModuleList(int nargs);
void m_mappGetModuleIDs(int nargs);
void m_mappModuleTask(int nargs);
void m_mappModuleFilename(int nargs);
void m_mappHostAppFilename(int nargs);
void m_mappDirectorFilename(int nargs);
void m_mappAuthorwareFilename(int nargs);
void m_mappModuleName(int nargs);
void m_mappModuleIsRunning(int nargs);
void m_mappUnloadModule(int nargs);
void m_mappHostAppModule(int nargs);
void m_mappDirectorModule(int nargs);
void m_mappAuthorwareModule(int nargs);
void m_mappGetParentWindowList(int nargs);
void m_mappGetParentWindowIDs(int nargs);
void m_mappGetChildWindowList(int nargs);
void m_mappGetChildWindowIDs(int nargs);
void m_mappWindowParent(int nargs);
void m_mappGetWindowParent(int nargs);
void m_mappGetHighestWindowParent(int nargs);
void m_mappWindowName(int nargs);
void m_mappGetWindowName(int nargs);
void m_mappSetWindowName(int nargs);
void m_mappWindowHasChildren(int nargs);
void m_mappWindowType(int nargs);
void m_mappGetWindowType(int nargs);
void m_mappWindowIsVisible(int nargs);
void m_mappFindWindow(int nargs);
void m_mappGetActiveWindow(int nargs);
void m_mappSetActiveWindow(int nargs);
void m_mappGetForegroundWindow(int nargs);
void m_mappSetForegroundWindow(int nargs);
void m_mappGetKeyboardInputWindow(int nargs);
void m_mappSetKeyboardInputWindow(int nargs);
void m_mappCaptureMouseInput(int nargs);
void m_mappReleaseMouseInput(int nargs);
void m_mappGetParentWindowTopChild(int nargs);
void m_mappGetDesktopWindow(int nargs);
void m_mappGetWindowOutsideRect(int nargs);
void m_mappGetWindowInsideRect(int nargs);
void m_mappSetWindowOutsideRect(int nargs);
void m_mappSetWindowRect(int nargs);
void m_mappCloseWindow(int nargs);
void m_mappWindowExists(int nargs);
void m_mappWindowToFront(int nargs);
void m_mappWindowToBack(int nargs);
void m_mappHideWindow(int nargs);
void m_mappShowWindow(int nargs);
void m_mappMinimizeWindow(int nargs);
void m_mappRestoreWindow(int nargs);
void m_mappMaximizeWindow(int nargs);
void m_mappDirectorStageWindow(int nargs);
void m_mappHostAppMainWindow(int nargs);
void m_mappDirectorMainWindow(int nargs);
void m_mappAuthorwareMainWindow(int nargs);
void m_mappKeepOnTop(int nargs);
void m_mappDontKeepOnTop(int nargs);
void m_mappLaunch(int nargs);
void m_mappLaunchButDontActivate(int nargs);
void m_mappLaunchHidden(int nargs);
void m_mappLaunchMinimized(int nargs);
void m_mappLaunchMaximized(int nargs);
void m_mappLocateExecutable(int nargs);
void m_mappGetShortFileName(int nargs);
void m_mappGetLongFileName(int nargs);
void m_mappOpenDocument(int nargs);
void m_mappPrintDocument(int nargs);
void m_mappInstanceTask(int nargs);
void m_mappInstanceIsRunning(int nargs);
void m_mappUnloadInstance(int nargs);
void m_mappFakeMouseClick(int nargs);
void m_mappFakeMouseClickWait(int nargs);
void m_mappFakeCharacter(int nargs);
void m_mappFakeCharacterWait(int nargs);
void m_mappFakeCharCode(int nargs);
void m_mappFakeCharCodeWait(int nargs);
void m_mappShowMenu(int nargs);
void m_mappShowMenuWait(int nargs);
void m_mappAnyWindowAtPoint(int nargs);
void m_mappChildWindowAtPoint(int nargs);
void m_mappGetWindowsRegistry(int nargs);
void m_mappSetWindowsRegistry(int nargs);
void m_mappGetWindowsRegistryEntries(int nargs);
void m_mappGetWindowsRegistryKeys(int nargs);
void m_mappGetWindowsRegistryValues(int nargs);
void m_mappGetWindowsRegistryEntryType(int nargs);
void m_mappDeleteWindowsRegistry(int nargs);
void m_mappGetDefaultPrinter(int nargs);
void m_mappSetDefaultPrinter(int nargs);
void m_mappGetInstalledPrinters(int nargs);
void m_mappWindowsFlavor(int nargs);

} // End of namespace MasterAppXtra

} // End of namespace Director

#endif
