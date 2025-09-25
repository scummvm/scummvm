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
#include "director/lingo/xtras/masterapp.h"

/**************************************************
 *
 * USED IN:
 * puppetmotel
 *
 **************************************************/

/*
-- xtra MasterApp
-- MasterApp Xtra
-- Copyright © 1996-1998 Glenn M. Picher, Dirigo Multimedia
--
-- Published and supported by:
--
--   updateStage
--   1341 Mass Ave., Suite 124
--   Arlington, MA USA 02174
--   Web: http://www.updatestage.com
--   Email: sales@updatestage.com, support@updatestage.com
--   Voice: 781-641-6043
--   Fax: 781-641-7068
--   Email/phone/fax support hours: 10AM - 6PM US EST
--
--
-- Developed by:
--
--   Glenn M. Picher
--   Dirigo Multimedia
--   50 Market St., Suite 1A-338
--   South Portland, ME USA 04106
--   Web: http://www.maine.com/shops/gpicher
--   Email: gpicher@maine.com
--   Voice: 207-767-8015
--   Fax: 207-767-1018
--
-- Registered 32-bit version, April 22, 1998, v1.0.0.37
--
new object me
-- Note: since the MasterApp Xtra only uses global handlers, there is no need
-- to create a child object. Just use the handlers listed below.
--
+ register object xtraReference, string serialNumber
-- Note: you only get one chance per Director session to enter this correctly.
-- There will be a beep and a five second delay if you get the serial number wrong.
-- Subsequent attempts to register will have no effect, until restarting Director.
--
-- Note: check the return values of all handlers for a string with word 1 'Error:' !
--
-- TASK FUNCTIONS
-- --------------
* mappGetTaskList
-- Returns a string listing of all currently running tasks, one per line,
-- with the following info on each line, separated by commas:
--   taskName, taskID, parentTaskID, instanceID, moduleID
-- Note: the .x32 and Mac versions use the same numbers for taskID, instanceID,
-- and moduleID. Under Windows NT with the .x32 version, parentTaskID is always 0.
-- Note that Win32 sometimes reuses taskIDs; verify the taskName is still the same!
* mappGetTaskIDs
-- Get taskIDs for all running tasks, separated by ' '. Use 'word' to examine results.
* mappTaskName integer taskID
* mappTaskParent integer taskID
* mappTaskInstance integer taskID
* mappTaskModule integer taskID
* mappTaskEvents integer taskID --Applies to Win 3.1 only; on Win95/NT, always returns '0'.
* mappGetTaskWindowList integer taskID
-- Gets a listing of all windows belonging to this taskID, in the form:
--   windowName, windowID, parentWindowID, hasChildren, windowType
* mappGetTaskWindowIDs integer taskID
* mappGetWindowTask integer windowID
* mappRudeQuitTask integer taskID, integer exitValue
-- Rude way of quitting a program. Using mappCloseWindow is better! Returns
-- 0 if the program's already quit, -4 if it's still running. It's not unusual
-- to see -4 as a result. On Win32, -4 is *always* the result.
* mappTaskIsRunning integer taskID
* mappFeedTimeSlice integer taskID
-- Note: on Win32, the taskID is ignored; has same effect as a generic time slice.
* mappFeedGenericTimeSlice
* mappHostAppTask
-- Returns the taskID for this Xtra's host application.
* mappDirectorTask
* mappAuthorwareTask
-- Synonyms for mappHostAppTask.
* mappGetTaskOrder
-- Reports taskIDs with visible windows, in front-to-back order. Note that Windows
-- Explorer usually shows up first, due to the Windows task bar.
--
--
-- THREAD FUNCTIONS (WIN32 ONLY)
-- -----------------------------
* mappGetThreadList
-- .x32 version only; gets a listing of all threads, in the form:
--   threadID, parentTaskID
-- Primarily useful with mappGetThreadWindowList and 16-bit apps on NT.
* mappGetTaskThreads integer taskID
-- .x32 version only; gets a list of all threadIDs for this taskID, separated by ' '.
* mappGetThreadWindowList integer threadID
-- .x32 version only; gets a list of all windows for this threadID, in the form:
--   windowName, windowID, parentWindowID, hasChildren, windowType
* mappGetThreadWindowIDs integer threadID
-- .x32 version only; reports only the thread's windowIDs, separated by spaces.
--
--
-- MODULE FUNCTIONS
-- ----------------
* mappGetModuleList
-- Returns a string listing of all currently running modules, one per line,
-- with the following info on each line, separated by commas:
--   moduleName, moduleID, moduleFileName
-- Note: the .x32 and Mac versions use the same numbers for taskID, instanceID,
-- and moduleID. Under Windows NT with the .x32 version, moduleFileName does not
-- include the full path or the filename extension. Note that Win32 sometimes
-- reuses moduleIDs; verify the moduleName is still the same!
* mappGetModuleIDs
-- Returns a string of moduleIDs, separated by spaces.
* mappModuleTask integer moduleID
* mappModuleFilename integer moduleID
-- Note: on Windows NT, the answer does not include a full path or extension!
* mappHostAppFilename
* mappDirectorFilename
* mappAuthorwareFilename
-- Where is this program's executable file located? These are all synonyms.
* mappModuleName integer moduleID
-- Note: on Win32, this is a synonym for mappTaskName.
* mappModuleIsRunning integer moduleID
-- Is the indicated moduleID still running?
* mappUnloadModule integer moduleID
-- Note: on Win32, each task gets its own address space and thus unloading
-- a module doesn't affect other tasks; so on Win32 and the Mac, this method is a
-- synonym for mappRudeQuitTask().
* mappHostAppModule
* mappDirectorModule
* mappAuthorwareModule
-- Returns moduleID for the current program. All are synonyms.
--
--
-- WINDOW FUNCTIONS
-- ----------------
* mappGetParentWindowList
-- Gets a list of all parent windows, in the form:
--   windowName, windowID, parentWindowID, hasChildren, windowType
-- These are reported in front-to-back screen draw order, though not all of
-- the reported windows may be visible or on-screen. Note the Windows 95/NT
-- taskbar, or tooltip windows, are typically the first windows listed.
* mappGetParentWindowIDs
-- Gets all parent windowIDs, separated by spaces.
* mappGetChildWindowList integer windowID
-- Gets a list of all child windows of the supplied window, in the form:
--   windowName, windowID, parentWindowID, hasChildren, windowType
* mappGetChildWindowIDs integer windowID
-- Gets a string of all windowIDs owned by the windowID, separated by ' '.
* mappWindowParent integer windowID
* mappGetWindowParent integer windowID
-- Gets parent windowID of the supplied windowID. These are synonyms.
* mappGetHighestWindowParent integer windowID
* mappWindowName integer windowID
* mappGetWindowName integer windowID
-- Gets window name. These are synonyms.
* mappSetWindowName integer windowID, string newName
* mappWindowHasChildren integer windowID
* mappWindowType integer windowID
* mappGetWindowType integer windowID
* mappWindowIsVisible integer windowID
* mappFindWindow any windowName, any windowType
-- Get a windowID for the first parent window matching the windowName and windowType.
-- For windowName, supply a string to check a windowName, or 0 for all windowNames.
-- For windowType, supply a string to check a windowType, or 0 for all windowTypes.
-- Returns a string for an error, or an integer with the first found windowID.
-- Example: see if Windows Paint accessory is running with syntax like this:
--   set alreadyRunning = integerP(mappFindWindow(0,"MSPaintApp"))
* mappGetActiveWindow
-- Gets the currently active top level parent windowID. Applies only to the host app,
-- not other apps. Generally the same as mappHostAppMainWindow(), but can be used with
-- mappSetActiveWindow() to determine the current state.
* mappSetActiveWindow integer windowID
-- Sets the active window to the supplied top level parent windowID . Use with care-- can
-- take over mouse and keyboard input! Returns the previously active windowID, or 0 if no
-- window in the host app was previously active.
* mappGetForegroundWindow
-- Win32 only method! Returns the windowID that the user is currently working with.
-- Can be easier to use than mappGetParentWindowList() or mappGetParentWindowIDs(),
-- since it doesn't return floating Windows Explorer task bar windowIDs, etc.
* mappSetForegroundWindow integer windowID
-- Win32 only method! Changes front to back draw order and gives the window's task higher
-- priority for processor time slices. Similar to mappWindowToFront, but more immediate.
-- Returns TRUE or FALSE for success or failure.
* mappGetKeyboardInputWindow
-- What windowID is currently getting keyboard events?
* mappSetKeyboardInputWindow integer windowID
-- Set the windowID to receive keyboard events. Supply 0 if you want keyboard input to be
-- ignored. Returns the windowID previously receiving keyboard events.
* mappCaptureMouseInput integer windowID
-- Directs mouse input to the specified window. For use with the host app's windows.
-- Returns the previously set input window. The supplied windowID must already be
-- frontmost. This allows a window to react to mouse movement outside its rectangle.
-- This doesn't prevent other windows from being activated by mouse clicks.
* mappReleaseMouseInput
-- Frees mouse input to be received normally. Reverses effect of mappCaptureMouseInput().
-- No return value.
* mappGetParentWindowTopChild integer windowID
-- Returns the topmost child window of the supplied parent windowID. May return a 'floating
-- palette' window, such as Director's 'Control Panel' window. Supply 0 if you want
-- the top window on the screen-- this can tell you if the Windows taskbar is showing, if
-- the windowType of the returned top window is 'Shell_TrayWnd'.
* mappGetDesktopWindow
-- Gets a windowID for the desktop, the screen 'parent' of every parent window.
* mappGetWindowOutsideRect integer windowID
* mappGetWindowInsideRect integer windowID
* mappSetWindowOutsideRect integer windowID, integer l, integer t, integer b, integer r
* mappSetWindowRect integer windowID, integer l, integer t, integer b, integer r
* mappCloseWindow integer windowID
-- Closes a window. This is the normal way to quit an application. Returns
-- -1 (bad windowID), 0 (windows has quit), or -2 (windows hasn't quit yet).
-- Unlike the XObject and .x16 version, the .x32 version waits up to 3 seconds
-- for the window to quit.
* mappWindowExists integer windowID
* mappWindowToFront integer windowID
-- Brings the window to the front of the screen.
* mappWindowToBack integer windowID
-- Sends the window to the back.
* mappHideWindow integer windowID
* mappShowWindow integer windowID
-- Controls the appearance of the window.
* mappMinimizeWindow integer windowID
* mappRestoreWindow integer windowID
* mappMaximizeWindow integer windowID
* mappDirectorStageWindow
-- Returns the windowID of the stage. Director-only; doesn't work in Authorware.
* mappHostAppMainWindow
-- Returns the main windowID for this Xtra's host application.
-- In the case of Director, this is the parent of the parent of the stage window.
* mappDirectorMainWindow
* mappAuthorwareMainWindow
-- Synonyms for mappHostAppMainWindow.
* mappKeepOnTop integer windowID
-- Window is displayed on top of all other windows, even when other app's
-- windows are activated. Useful for making floating control panels.
* mappDontKeepOnTop integer windowID
-- Reverses the results of mappKeepOnTop().
--
--
-- LAUNCH FUNCTIONS
-- ----------------
* mappLaunch string theApp, string theCommandLineArguments
-- Launch the program indicated by the full pathname 'theApp', possibly with command line
-- arguments (supply EMPTY if there are no command line arguments). Returns an instanceID
-- or a moduleID (which is the same number as a taskID with the .x32 and Mac versions).
-- With the .x16 version, use mappInstanceTask() or mappModuleTask() to get a taskID.
-- Note that Win32 sometimes reuses taskIDs, so verify the taskName is still what
-- you expect; see mappGetTaskList() and mappTaskName().
* mappLaunchButDontActivate string theApp, string theCommandLineArguments
* mappLaunchHidden string theApp, string theCommandLineArguments
* mappLaunchMinimized string theApp, string theCommandLineArguments
* mappLaunchMaximized string theApp, string theCommandLineArguments
-- Similar to mappLaunch, but with different initial appearance. Not every application
-- respects the requested appearance-- testing is required.
--
--
-- LAUNCH UTILITIES
-- ----------------
* mappLocateExecutable string theFile
-- Locate the executable file registered to handle the document file indicated
-- by the full pathname 'theFile'. Alternative syntax: supply a file name with no path
-- and the DOS 'PATH' environment variable will be searched for the application.
-- Note that mappGetWindowsRegistry() can also be used to find apps, even if they do
-- not register any document types.
* mappGetShortFileName string theFile
* mappGetLongFileName string theFile
-- Converts between MS-DOS 8.3 compliant short file names and long file names. Some
-- programs require, or work more reliably with, short file names.
* mappOpenDocument string theDocument
-- Opens the document with the application registered to open that type of document.
-- Supply a full pathname to the document. Returns 'OK', or a string with word 1 'Error:'.
-- This is a difference from the XObject version, which returns an instanceID or moduleID;
-- however, the 32-bit Windows API does not support this! Use mappGetTaskList(), etc. to
-- identify the launched applications, or use mappLaunch() with the doc as the command line.
* mappPrintDocument string theDocument
-- Opens the document with the application registered to open that type of document
-- and prints one copy to the default printer. Same return values as mappOpenDocument.
* mappInstanceTask integer instanceID
* mappInstanceIsRunning integer instanceID
* mappUnloadInstance integer instanceID
-- Note: on Win32, each instance gets its own address space and thus unloading
-- an instance doesn't affect other tasks; so on Win32 and the Mac, this method is a
-- synonym for mappRudeQuitTask().
--
--
-- INPUT SIMULATION
-- ----------------
* mappFakeMouseClick integer windowID, integer x, integer y
* mappFakeMouseClickWait integer windowID, integer x, integer y
* mappFakeCharacter integer windowID, string theChar
* mappFakeCharacterWait integer windowID, string theChar
* mappFakeCharCode integer windowID, integer charNum, integer codeNum
* mappFakeCharCodeWait integer windowID, integer charNum, integer codeNum
-- Simulates a keypress of a special key in the window. Examples:
-- Tab key = 9,15 ; Return key = 13,28; Escape key = 27,1;
-- Ctrl-x (cut) = 24,45; Ctrl-c (copy) = 3,46; Ctrl-v (paste) =
-- 22,47; for others, use a keyboard sniffer. See documentation.
* mappShowMenu integer windowID
* mappShowMenuWait integer windowID
* mappAnyWindowAtPoint integer x, integer y
-- What windowID is under point(x,y)? Use screen coordinates.
* mappChildWindowAtPoint integer windowID, integer x, integer y
-- What child window of the supplied parent windowID is under point(x,y)?
-- Use screen coordinates. This can provide a windowID *within* a windowID
-- returned by mappAnyWindowAtPoint().
--
--
-- WINDOWS REGISTRY FUNCTIONS
-- --------------------------
* mappGetWindowsRegistry string theBase, string theKey, string theValue
-- Gets the value theValue of registry key theKey, a subkey of key theBase.
-- For the .x16 version, 'theBase' can only be 'HKEY_CLASSES_ROOT' and theValue
-- must be EMPTY (because that's just how the 16-bit registry works). For the .x32
-- version, theBase can also be 'HKEY_CURRENT_USER', 'HKEY_LOCAL_MACHINE' or
-- 'HKEY_USERS'. Note that theKey can specify multiple keys in the registry,
-- separated by '\' characters. Suppy EMPTY for theValue if you want the '(Default)'
-- value for theKey (the 16-bit registry only knows about the '(Default') key).
-- See the read me info, and experiment with REGEDIT.EXE, to learn more!
-- Returns a string, an integer, or binary data in the form of a list of bytes (as
-- integers from 0 to 255), depending on what type of entry is present in the registry.
* mappSetWindowsRegistry string theBase, string theKey, string theValue, any theData
-- Sets a registry entry with new data. The key and its parent keys are created if needed.
-- Supply a string, an integer, or binary data in the form of a list of bytes as integers
-- from 0 to 255 (depending on what sort of entry you are making). The entry type will be
-- changed if needed. Supply EMPTY for theValue to set the '(Default)' value (the default
-- 16-bit registry value), which must be a string value. Note: while you can use all the
-- keys for 'theBase' listed under mappGetWindowsRegistry(), it's inadvisable to create
-- settings under 'HKEY_USERS'. NOTE: Be *careful* with this method!
-- It can damage your registry data and prevent your computer from working right.
-- Please back up your registry data before doing development work and testing.
* mappGetWindowsRegistryEntries string theBase, string theKey
-- Returns a string with all entries (subkeys and values) available under the
-- supplied key; one entry per line. Subkeys are listed first, then values; but
-- the order in which subkeys and values are reported may vary from call to call.
-- The default 16-bit value is not included in the list of values; it's always available.
* mappGetWindowsRegistryKeys string theBase, string theKey
-- Returns a string with all subkeys available under the
-- supplied key; one entry per line.
* mappGetWindowsRegistryValues string theBase, string theKey
-- Returns a string with all values available under the
-- supplied key; one entry per line. Does not include 16-bit default value.
* mappGetWindowsRegistryEntryType string theBase, string theKey, string theValue
-- Returns the type of data in the indicated registry entry: 'string', 'integer',
-- 'bytelist', 'unsupported' or 'subkey'.
* mappDeleteWindowsRegistry string theBase, string theKey, string theValue
-- Erases the registry key. On Windows 95, this also deletes any subkeys.
-- On Windows NT, this won't work if subkeys exist, so use mappGetWindowsRegistryKeys()
-- to detect and delete any subkeys first. NOTE: Be *careful* with this method!
-- It can damage your registry data and prevent your computer from working right.
-- Please back up your registry data before doing development work and testing.
-- Note: supply 'EMPTY' for 'theValue' for 16-bit registry values.
--
--
-- MISCELLANEOUS WINDOWS-ONLY FUNCTIONS
-- ------------------------------------
* mappGetDefaultPrinter
-- What printer is set as the default printer?
* mappSetDefaultPrinter string thePrinter
-- Set this printer to be the default printer. Supply the exact results of either
-- mappGetDefaultPrinter() or one of the lines returned by mappGetInstalledPrinters().
* mappGetInstalledPrinters
-- What are the installed printers which can be used with mappSetDefaultPrinter?
* mappWindowsFlavor
-- Which flavor of Windows is running? Returns 'win31', 'win95' or 'winNT'.
-- Obviously the .x32 version never reports 'win31'.

 */

namespace Director {

const char *MasterAppXtra::xlibName = "MasterApp";
const XlibFileDesc MasterAppXtra::fileNames[] = {
	{ "masterapp",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MasterAppXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mappGetTaskList", MasterAppXtra::m_mappGetTaskList, 0, 0, 500, HBLTIN },
	{ "mappGetTaskIDs", MasterAppXtra::m_mappGetTaskIDs, 0, 0, 500, HBLTIN },
	{ "mappTaskName", MasterAppXtra::m_mappTaskName, 1, 1, 500, HBLTIN },
	{ "mappTaskParent", MasterAppXtra::m_mappTaskParent, 1, 1, 500, HBLTIN },
	{ "mappTaskInstance", MasterAppXtra::m_mappTaskInstance, 1, 1, 500, HBLTIN },
	{ "mappTaskModule", MasterAppXtra::m_mappTaskModule, 1, 1, 500, HBLTIN },
	{ "mappTaskEvents", MasterAppXtra::m_mappTaskEvents, 1, 1, 500, HBLTIN },
	{ "mappGetTaskWindowList", MasterAppXtra::m_mappGetTaskWindowList, 1, 1, 500, HBLTIN },
	{ "mappGetTaskWindowIDs", MasterAppXtra::m_mappGetTaskWindowIDs, 1, 1, 500, HBLTIN },
	{ "mappGetWindowTask", MasterAppXtra::m_mappGetWindowTask, 1, 1, 500, HBLTIN },
	{ "mappRudeQuitTask", MasterAppXtra::m_mappRudeQuitTask, 2, 2, 500, HBLTIN },
	{ "mappTaskIsRunning", MasterAppXtra::m_mappTaskIsRunning, 1, 1, 500, HBLTIN },
	{ "mappFeedTimeSlice", MasterAppXtra::m_mappFeedTimeSlice, 1, 1, 500, HBLTIN },
	{ "mappFeedGenericTimeSlice", MasterAppXtra::m_mappFeedGenericTimeSlice, 0, 0, 500, HBLTIN },
	{ "mappHostAppTask", MasterAppXtra::m_mappHostAppTask, 0, 0, 500, HBLTIN },
	{ "mappDirectorTask", MasterAppXtra::m_mappDirectorTask, 0, 0, 500, HBLTIN },
	{ "mappAuthorwareTask", MasterAppXtra::m_mappAuthorwareTask, 0, 0, 500, HBLTIN },
	{ "mappGetTaskOrder", MasterAppXtra::m_mappGetTaskOrder, 0, 0, 500, HBLTIN },
	{ "mappGetThreadList", MasterAppXtra::m_mappGetThreadList, 0, 0, 500, HBLTIN },
	{ "mappGetTaskThreads", MasterAppXtra::m_mappGetTaskThreads, 1, 1, 500, HBLTIN },
	{ "mappGetThreadWindowList", MasterAppXtra::m_mappGetThreadWindowList, 1, 1, 500, HBLTIN },
	{ "mappGetThreadWindowIDs", MasterAppXtra::m_mappGetThreadWindowIDs, 1, 1, 500, HBLTIN },
	{ "mappGetModuleList", MasterAppXtra::m_mappGetModuleList, 0, 0, 500, HBLTIN },
	{ "mappGetModuleIDs", MasterAppXtra::m_mappGetModuleIDs, 0, 0, 500, HBLTIN },
	{ "mappModuleTask", MasterAppXtra::m_mappModuleTask, 1, 1, 500, HBLTIN },
	{ "mappModuleFilename", MasterAppXtra::m_mappModuleFilename, 1, 1, 500, HBLTIN },
	{ "mappHostAppFilename", MasterAppXtra::m_mappHostAppFilename, 0, 0, 500, HBLTIN },
	{ "mappDirectorFilename", MasterAppXtra::m_mappDirectorFilename, 0, 0, 500, HBLTIN },
	{ "mappAuthorwareFilename", MasterAppXtra::m_mappAuthorwareFilename, 0, 0, 500, HBLTIN },
	{ "mappModuleName", MasterAppXtra::m_mappModuleName, 1, 1, 500, HBLTIN },
	{ "mappModuleIsRunning", MasterAppXtra::m_mappModuleIsRunning, 1, 1, 500, HBLTIN },
	{ "mappUnloadModule", MasterAppXtra::m_mappUnloadModule, 1, 1, 500, HBLTIN },
	{ "mappHostAppModule", MasterAppXtra::m_mappHostAppModule, 0, 0, 500, HBLTIN },
	{ "mappDirectorModule", MasterAppXtra::m_mappDirectorModule, 0, 0, 500, HBLTIN },
	{ "mappAuthorwareModule", MasterAppXtra::m_mappAuthorwareModule, 0, 0, 500, HBLTIN },
	{ "mappGetParentWindowList", MasterAppXtra::m_mappGetParentWindowList, 0, 0, 500, HBLTIN },
	{ "mappGetParentWindowIDs", MasterAppXtra::m_mappGetParentWindowIDs, 0, 0, 500, HBLTIN },
	{ "mappGetChildWindowList", MasterAppXtra::m_mappGetChildWindowList, 1, 1, 500, HBLTIN },
	{ "mappGetChildWindowIDs", MasterAppXtra::m_mappGetChildWindowIDs, 1, 1, 500, HBLTIN },
	{ "mappWindowParent", MasterAppXtra::m_mappWindowParent, 1, 1, 500, HBLTIN },
	{ "mappGetWindowParent", MasterAppXtra::m_mappGetWindowParent, 1, 1, 500, HBLTIN },
	{ "mappGetHighestWindowParent", MasterAppXtra::m_mappGetHighestWindowParent, 1, 1, 500, HBLTIN },
	{ "mappWindowName", MasterAppXtra::m_mappWindowName, 1, 1, 500, HBLTIN },
	{ "mappGetWindowName", MasterAppXtra::m_mappGetWindowName, 1, 1, 500, HBLTIN },
	{ "mappSetWindowName", MasterAppXtra::m_mappSetWindowName, 2, 2, 500, HBLTIN },
	{ "mappWindowHasChildren", MasterAppXtra::m_mappWindowHasChildren, 1, 1, 500, HBLTIN },
	{ "mappWindowType", MasterAppXtra::m_mappWindowType, 1, 1, 500, HBLTIN },
	{ "mappGetWindowType", MasterAppXtra::m_mappGetWindowType, 1, 1, 500, HBLTIN },
	{ "mappWindowIsVisible", MasterAppXtra::m_mappWindowIsVisible, 1, 1, 500, HBLTIN },
	{ "mappFindWindow", MasterAppXtra::m_mappFindWindow, 2, 2, 500, HBLTIN },
	{ "mappGetActiveWindow", MasterAppXtra::m_mappGetActiveWindow, 0, 0, 500, HBLTIN },
	{ "mappSetActiveWindow", MasterAppXtra::m_mappSetActiveWindow, 1, 1, 500, HBLTIN },
	{ "mappGetForegroundWindow", MasterAppXtra::m_mappGetForegroundWindow, 0, 0, 500, HBLTIN },
	{ "mappSetForegroundWindow", MasterAppXtra::m_mappSetForegroundWindow, 1, 1, 500, HBLTIN },
	{ "mappGetKeyboardInputWindow", MasterAppXtra::m_mappGetKeyboardInputWindow, 0, 0, 500, HBLTIN },
	{ "mappSetKeyboardInputWindow", MasterAppXtra::m_mappSetKeyboardInputWindow, 1, 1, 500, HBLTIN },
	{ "mappCaptureMouseInput", MasterAppXtra::m_mappCaptureMouseInput, 1, 1, 500, HBLTIN },
	{ "mappReleaseMouseInput", MasterAppXtra::m_mappReleaseMouseInput, 0, 0, 500, HBLTIN },
	{ "mappGetParentWindowTopChild", MasterAppXtra::m_mappGetParentWindowTopChild, 1, 1, 500, HBLTIN },
	{ "mappGetDesktopWindow", MasterAppXtra::m_mappGetDesktopWindow, 0, 0, 500, HBLTIN },
	{ "mappGetWindowOutsideRect", MasterAppXtra::m_mappGetWindowOutsideRect, 1, 1, 500, HBLTIN },
	{ "mappGetWindowInsideRect", MasterAppXtra::m_mappGetWindowInsideRect, 1, 1, 500, HBLTIN },
	{ "mappSetWindowOutsideRect", MasterAppXtra::m_mappSetWindowOutsideRect, 5, 5, 500, HBLTIN },
	{ "mappSetWindowRect", MasterAppXtra::m_mappSetWindowRect, 5, 5, 500, HBLTIN },
	{ "mappCloseWindow", MasterAppXtra::m_mappCloseWindow, 1, 1, 500, HBLTIN },
	{ "mappWindowExists", MasterAppXtra::m_mappWindowExists, 1, 1, 500, HBLTIN },
	{ "mappWindowToFront", MasterAppXtra::m_mappWindowToFront, 1, 1, 500, HBLTIN },
	{ "mappWindowToBack", MasterAppXtra::m_mappWindowToBack, 1, 1, 500, HBLTIN },
	{ "mappHideWindow", MasterAppXtra::m_mappHideWindow, 1, 1, 500, HBLTIN },
	{ "mappShowWindow", MasterAppXtra::m_mappShowWindow, 1, 1, 500, HBLTIN },
	{ "mappMinimizeWindow", MasterAppXtra::m_mappMinimizeWindow, 1, 1, 500, HBLTIN },
	{ "mappRestoreWindow", MasterAppXtra::m_mappRestoreWindow, 1, 1, 500, HBLTIN },
	{ "mappMaximizeWindow", MasterAppXtra::m_mappMaximizeWindow, 1, 1, 500, HBLTIN },
	{ "mappDirectorStageWindow", MasterAppXtra::m_mappDirectorStageWindow, 0, 0, 500, HBLTIN },
	{ "mappHostAppMainWindow", MasterAppXtra::m_mappHostAppMainWindow, 0, 0, 500, HBLTIN },
	{ "mappDirectorMainWindow", MasterAppXtra::m_mappDirectorMainWindow, 0, 0, 500, HBLTIN },
	{ "mappAuthorwareMainWindow", MasterAppXtra::m_mappAuthorwareMainWindow, 0, 0, 500, HBLTIN },
	{ "mappKeepOnTop", MasterAppXtra::m_mappKeepOnTop, 1, 1, 500, HBLTIN },
	{ "mappDontKeepOnTop", MasterAppXtra::m_mappDontKeepOnTop, 1, 1, 500, HBLTIN },
	{ "mappLaunch", MasterAppXtra::m_mappLaunch, 2, 2, 500, HBLTIN },
	{ "mappLaunchButDontActivate", MasterAppXtra::m_mappLaunchButDontActivate, 2, 2, 500, HBLTIN },
	{ "mappLaunchHidden", MasterAppXtra::m_mappLaunchHidden, 2, 2, 500, HBLTIN },
	{ "mappLaunchMinimized", MasterAppXtra::m_mappLaunchMinimized, 2, 2, 500, HBLTIN },
	{ "mappLaunchMaximized", MasterAppXtra::m_mappLaunchMaximized, 2, 2, 500, HBLTIN },
	{ "mappLocateExecutable", MasterAppXtra::m_mappLocateExecutable, 1, 1, 500, HBLTIN },
	{ "mappGetShortFileName", MasterAppXtra::m_mappGetShortFileName, 1, 1, 500, HBLTIN },
	{ "mappGetLongFileName", MasterAppXtra::m_mappGetLongFileName, 1, 1, 500, HBLTIN },
	{ "mappOpenDocument", MasterAppXtra::m_mappOpenDocument, 1, 1, 500, HBLTIN },
	{ "mappPrintDocument", MasterAppXtra::m_mappPrintDocument, 1, 1, 500, HBLTIN },
	{ "mappInstanceTask", MasterAppXtra::m_mappInstanceTask, 1, 1, 500, HBLTIN },
	{ "mappInstanceIsRunning", MasterAppXtra::m_mappInstanceIsRunning, 1, 1, 500, HBLTIN },
	{ "mappUnloadInstance", MasterAppXtra::m_mappUnloadInstance, 1, 1, 500, HBLTIN },
	{ "mappFakeMouseClick", MasterAppXtra::m_mappFakeMouseClick, 3, 3, 500, HBLTIN },
	{ "mappFakeMouseClickWait", MasterAppXtra::m_mappFakeMouseClickWait, 3, 3, 500, HBLTIN },
	{ "mappFakeCharacter", MasterAppXtra::m_mappFakeCharacter, 2, 2, 500, HBLTIN },
	{ "mappFakeCharacterWait", MasterAppXtra::m_mappFakeCharacterWait, 2, 2, 500, HBLTIN },
	{ "mappFakeCharCode", MasterAppXtra::m_mappFakeCharCode, 3, 3, 500, HBLTIN },
	{ "mappFakeCharCodeWait", MasterAppXtra::m_mappFakeCharCodeWait, 3, 3, 500, HBLTIN },
	{ "mappShowMenu", MasterAppXtra::m_mappShowMenu, 1, 1, 500, HBLTIN },
	{ "mappShowMenuWait", MasterAppXtra::m_mappShowMenuWait, 1, 1, 500, HBLTIN },
	{ "mappAnyWindowAtPoint", MasterAppXtra::m_mappAnyWindowAtPoint, 2, 2, 500, HBLTIN },
	{ "mappChildWindowAtPoint", MasterAppXtra::m_mappChildWindowAtPoint, 3, 3, 500, HBLTIN },
	{ "mappGetWindowsRegistry", MasterAppXtra::m_mappGetWindowsRegistry, 3, 3, 500, HBLTIN },
	{ "mappSetWindowsRegistry", MasterAppXtra::m_mappSetWindowsRegistry, 4, 4, 500, HBLTIN },
	{ "mappGetWindowsRegistryEntries", MasterAppXtra::m_mappGetWindowsRegistryEntries, 2, 2, 500, HBLTIN },
	{ "mappGetWindowsRegistryKeys", MasterAppXtra::m_mappGetWindowsRegistryKeys, 2, 2, 500, HBLTIN },
	{ "mappGetWindowsRegistryValues", MasterAppXtra::m_mappGetWindowsRegistryValues, 2, 2, 500, HBLTIN },
	{ "mappGetWindowsRegistryEntryType", MasterAppXtra::m_mappGetWindowsRegistryEntryType, 3, 3, 500, HBLTIN },
	{ "mappDeleteWindowsRegistry", MasterAppXtra::m_mappDeleteWindowsRegistry, 3, 3, 500, HBLTIN },
	{ "mappGetDefaultPrinter", MasterAppXtra::m_mappGetDefaultPrinter, 0, 0, 500, HBLTIN },
	{ "mappSetDefaultPrinter", MasterAppXtra::m_mappSetDefaultPrinter, 1, 1, 500, HBLTIN },
	{ "mappGetInstalledPrinters", MasterAppXtra::m_mappGetInstalledPrinters, 0, 0, 500, HBLTIN },
	{ "mappWindowsFlavor", MasterAppXtra::m_mappWindowsFlavor, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MasterAppXtraObject::MasterAppXtraObject(ObjectType ObjectType) :Object<MasterAppXtraObject>("MasterApp") {
	_objType = ObjectType;
}

bool MasterAppXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MasterAppXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MasterAppXtra::xlibName);
	warning("MasterAppXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MasterAppXtra::open(ObjectType type, const Common::Path &path) {
    MasterAppXtraObject::initMethods(xlibMethods);
    MasterAppXtraObject *xobj = new MasterAppXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MasterAppXtra::close(ObjectType type) {
    MasterAppXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MasterAppXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MasterAppXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MasterAppXtra::m_register, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskName, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskParent, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskInstance, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskModule, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskEvents, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskWindowList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskWindowIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowTask, 0)
XOBJSTUB(MasterAppXtra::m_mappRudeQuitTask, 0)
XOBJSTUB(MasterAppXtra::m_mappTaskIsRunning, 0)
XOBJSTUB(MasterAppXtra::m_mappFeedTimeSlice, 0)
XOBJSTUB(MasterAppXtra::m_mappFeedGenericTimeSlice, 0)
XOBJSTUB(MasterAppXtra::m_mappHostAppTask, 0)
XOBJSTUB(MasterAppXtra::m_mappDirectorTask, 0)
XOBJSTUB(MasterAppXtra::m_mappAuthorwareTask, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskOrder, 0)
XOBJSTUB(MasterAppXtra::m_mappGetThreadList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetTaskThreads, 0)
XOBJSTUB(MasterAppXtra::m_mappGetThreadWindowList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetThreadWindowIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappGetModuleList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetModuleIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappModuleTask, 0)
XOBJSTUB(MasterAppXtra::m_mappModuleFilename, 0)
XOBJSTUB(MasterAppXtra::m_mappHostAppFilename, 0)
XOBJSTUB(MasterAppXtra::m_mappDirectorFilename, 0)
XOBJSTUB(MasterAppXtra::m_mappAuthorwareFilename, 0)
XOBJSTUB(MasterAppXtra::m_mappModuleName, 0)
XOBJSTUB(MasterAppXtra::m_mappModuleIsRunning, 0)
XOBJSTUB(MasterAppXtra::m_mappUnloadModule, 0)
XOBJSTUB(MasterAppXtra::m_mappHostAppModule, 0)
XOBJSTUB(MasterAppXtra::m_mappDirectorModule, 0)
XOBJSTUB(MasterAppXtra::m_mappAuthorwareModule, 0)
XOBJSTUB(MasterAppXtra::m_mappGetParentWindowList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetParentWindowIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappGetChildWindowList, 0)
XOBJSTUB(MasterAppXtra::m_mappGetChildWindowIDs, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowParent, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowParent, 0)
XOBJSTUB(MasterAppXtra::m_mappGetHighestWindowParent, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowName, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowName, 0)
XOBJSTUB(MasterAppXtra::m_mappSetWindowName, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowHasChildren, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowType, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowType, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowIsVisible, 0)
XOBJSTUB(MasterAppXtra::m_mappFindWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappGetActiveWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappSetActiveWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappGetForegroundWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappSetForegroundWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappGetKeyboardInputWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappSetKeyboardInputWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappCaptureMouseInput, 0)
XOBJSTUB(MasterAppXtra::m_mappReleaseMouseInput, 0)
XOBJSTUB(MasterAppXtra::m_mappGetParentWindowTopChild, 0)
XOBJSTUB(MasterAppXtra::m_mappGetDesktopWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowOutsideRect, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowInsideRect, 0)
XOBJSTUB(MasterAppXtra::m_mappSetWindowOutsideRect, 0)
XOBJSTUB(MasterAppXtra::m_mappSetWindowRect, 0)
XOBJSTUB(MasterAppXtra::m_mappCloseWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowExists, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowToFront, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowToBack, 0)
XOBJSTUB(MasterAppXtra::m_mappHideWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappShowWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappMinimizeWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappRestoreWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappMaximizeWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappDirectorStageWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappHostAppMainWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappDirectorMainWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappAuthorwareMainWindow, 0)
XOBJSTUB(MasterAppXtra::m_mappKeepOnTop, 0)
XOBJSTUB(MasterAppXtra::m_mappDontKeepOnTop, 0)
XOBJSTUB(MasterAppXtra::m_mappLaunch, 0)
XOBJSTUB(MasterAppXtra::m_mappLaunchButDontActivate, 0)
XOBJSTUB(MasterAppXtra::m_mappLaunchHidden, 0)
XOBJSTUB(MasterAppXtra::m_mappLaunchMinimized, 0)
XOBJSTUB(MasterAppXtra::m_mappLaunchMaximized, 0)
XOBJSTUB(MasterAppXtra::m_mappLocateExecutable, 0)
XOBJSTUB(MasterAppXtra::m_mappGetShortFileName, 0)
XOBJSTUB(MasterAppXtra::m_mappGetLongFileName, 0)
XOBJSTUB(MasterAppXtra::m_mappOpenDocument, 0)
XOBJSTUB(MasterAppXtra::m_mappPrintDocument, 0)
XOBJSTUB(MasterAppXtra::m_mappInstanceTask, 0)
XOBJSTUB(MasterAppXtra::m_mappInstanceIsRunning, 0)
XOBJSTUB(MasterAppXtra::m_mappUnloadInstance, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeMouseClick, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeMouseClickWait, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeCharacter, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeCharacterWait, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeCharCode, 0)
XOBJSTUB(MasterAppXtra::m_mappFakeCharCodeWait, 0)
XOBJSTUB(MasterAppXtra::m_mappShowMenu, 0)
XOBJSTUB(MasterAppXtra::m_mappShowMenuWait, 0)
XOBJSTUB(MasterAppXtra::m_mappAnyWindowAtPoint, 0)
XOBJSTUB(MasterAppXtra::m_mappChildWindowAtPoint, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowsRegistry, 0)
XOBJSTUB(MasterAppXtra::m_mappSetWindowsRegistry, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowsRegistryEntries, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowsRegistryKeys, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowsRegistryValues, 0)
XOBJSTUB(MasterAppXtra::m_mappGetWindowsRegistryEntryType, 0)
XOBJSTUB(MasterAppXtra::m_mappDeleteWindowsRegistry, 0)
XOBJSTUB(MasterAppXtra::m_mappGetDefaultPrinter, 0)
XOBJSTUB(MasterAppXtra::m_mappSetDefaultPrinter, 0)
XOBJSTUB(MasterAppXtra::m_mappGetInstalledPrinters, 0)
XOBJSTUB(MasterAppXtra::m_mappWindowsFlavor, 0)

}
