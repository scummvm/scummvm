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
#include "director/lingo/xlibs/v/versions.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- Versions XObject, version 1.1, 8/9/96
--Versions
-- Copyright © 1996 Glenn M. Picher, Dirigo Multimedia
-- Email: gpicher@maine.com
-- Web: http://www.maine.com/shops/gpicher
-- Phone: (207)767-8015 (South Portland, Maine, USA)
--
-- Distributors: g/matter, inc.
-- Email: support@gmatter.com
-- Web: http://www.gmatter.com
-- Phone: (415)243-0394 (San Francisco, California USA)
--
-- License granted to use and redistribute for any purpose,
-- as long as copyright and contact information remains intact.
-- Each instance of the XObject will present a copyright alert
-- box once if you use any methods other than the QuickTime version
-- checking functions. The registered version does not present any
-- alert boxes.
--
I mNew           -- Standard creation method
X mDispose       -- Standard dispose method
S mQuickTimeVersion
--  Get string of QuickTime version ('000000.000000.000000.000000' if QTW
--  is not installed). Suitable for string comparisons (<, =, >). Example:
--  Your title includes the QTW v2.1.1.57 installer, and you're running on
--  a machine with QTW v2.0.1.41 already installed.  QTW v. 2.1.1.57 becomes
--  '000002.000001.000001.000057' . Alphabetically, this comes after QTW
--  v. 2.0.1.41 ('000002.000000.000001.000041'). Thus you can conclude
--  that QuickTime needs to be updated to the version supplied with your
--  title. This is a workaround for a bug in MCI 'info qtwvideo version'
--  reporting, which does not always produce valid numeric comparisons. Only
--  reports the 16-bit QuickTime version. Works by initializing QuickTime,
--  so the first use may take much longer than subsequent uses.
--  Number is formatted to be compatible with file version numbers.
--  Note: requires ASK16.EXE and VERS16.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if ASK16.EXE can't load; Returns string with word 1 'Error:'
--  if ASK16.EXE fails. ***Note: version 1.0 of this XObject did not use
--  ASK16.EXE and VERS16.DLL . To avoid occasional crashes when 16-bit Director
--  got confused about the state of QuickTime, this new version keeps
--  Director's QuickTime code isolated from this XObject's QuickTime
--  code. Be sure to add ASK16.EXE and VERS16.DLL to your
--  distributed projects when upgrading to this version of this XObject.
S mWin32QuickTimeVersion
--  Reports the 32-bit QuickTime version. Requires ASK32.EXE and VERS32.DLL
--  in the same directory as this XObject .DLL . These files are
--  distributed with this XObject .DLL file. Returns EMPTY if 32-bit
--  environment is unavailable, or ASK32.EXE can't load; Returns
--  string with word 1 'Error:' if ASK32.EXE fails.
SS mFileVersion, fileName
--  Get string of file version number. Allows checking versions of
--  QuickTime compressor files, even 32-bit versions. Suitable for string
--  comparisons. Example: QTIM32.DLL in the System folder, v. 2.1.1.57 becomes
--  '000002.000001.000001.000057' . The extra digits are the minimum required to
--  represent the maximum 64-bit version number. Can be used with any file that
--  contains a version resource, not just QuickTime files. If file is missing
--  or does not contain version info, result is '000000.000000.000000.000000' .
--  Works whether 32-bit environment is available or not; however,
--  ASK32.EXE and VERS32.DLL are required in the same directory as this
--  XObject .DLL to get Win32 version numbers under Windows NT or 95. These
--  files are distributed with this XObject .DLL file. Further note: Windows
--  NT uses a different system directory for 32-bit .DLLs. See below.
S mWindowsDirectory
--  Returns full path to Windows directory (including trailing '\').
--  Useful for building full path names for use with mFileVersion.
--  Word 1 of the returned string will be 'Error:' in the unlikely
--  event of an error, followed by a description of the error.
--  Note: returns the Win16 answer (see below).
S mWin32WindowsDirectory
--  Returns full path to Windows directory (including trailing '\').
--  Should be the same answer as mWindowsDirectory for all current
--  Windows versions, but this may change in future Win versions.
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
S mSystemDirectory
--  Returns full path to System directory (including trailing '\').
--  Useful for building full path names for use with mFileVersion.
--  Word 1 of the returned string will be 'Error:' in the unlikely
--  event of an error, followed by a description of the error.
--  Note: returns the Win16 answer (see below).
S mWin32SystemDirectory
--  Returns full path to System directory (including trailing '\').
--  Under Windows NT, this is a different answer than mSystemDirectory.
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
S mDOSVersion
--  Returns the DOS version reported to 16-bit apps. No Win32 equivalent.
S mWindowsVersion
--  Returns the Windows version reported to 16-bit apps. This is
--  not the same answer as mWin32Version under Windows 95.
S mWin32Version
--  Returns the Win32 version (a different answer than mWindowsVersion)
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
--  Note the lack of ability to check for Win32-specific DOS version.
S mWin32Platform
--  Returns the Win32 platform ('Win32s on Windows 3.1',
--  'Win32 on Windows 95', or 'Windows NT').
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
S mWin32Build
--  Returns the Win32 build. This is useful because Director requires
--  at least Windows NT v3.51 with Service Pack 4 applied (build 1057).
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
S mWinNTVersion
--  Tells you what verison of Windows NT you're running under-- 'Workstation',
--  'Server', 'Advanced Server', 'Unknown', or 'Error' is there's a problem.
--  Note: requires ASK32.EXE and VERS32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
SS mGetShortFileName, theFile
--  Returns the MS-DOS style filename of a Windows 95 or Windows NT long
--  file name which might contain spaces or other DOS-illegal characters.
--  This method can also accept file names that are already DOS-legal.
--  This method is helpful when an XObject only works with DOS filenames.
--  Note: requires ASK32.EXE and FNAME32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
SS mGetLongFileName, theFile
--  Returns the Windows 95 or Windows NT long file name, which might contain
--  spaces or other DOS-illegal characters, given a DOS short file name.
--  This method can also accept file names that are already long.
--  Note: requires ASK32.EXE and FNAME32.DLL in the same directory as this
--  XObject .DLL (these files are distributed with this XObject).
--  Returns EMPTY if 32-bit environment is unavailable, or ASK32.EXE
--  can't load; Returns string with word 1 'Error:' if ASK32.EXE fails.
--
 */

namespace Director {

const char *VersionsXObj::xlibName = "Versions";
const XlibFileDesc VersionsXObj::fileNames[] = {
	{ "VERSIONS",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				VersionsXObj::m_new,		 0, 0,	400 },
	{ "dispose",				VersionsXObj::m_dispose,		 0, 0,	400 },
	{ "quickTimeVersion",				VersionsXObj::m_quickTimeVersion,		 0, 0,	400 },
	{ "win32QuickTimeVersion",				VersionsXObj::m_win32QuickTimeVersion,		 0, 0,	400 },
	{ "fileVersion",				VersionsXObj::m_fileVersion,		 1, 1,	400 },
	{ "windowsDirectory",				VersionsXObj::m_windowsDirectory,		 0, 0,	400 },
	{ "win32WindowsDirectory",				VersionsXObj::m_win32WindowsDirectory,		 0, 0,	400 },
	{ "systemDirectory",				VersionsXObj::m_systemDirectory,		 0, 0,	400 },
	{ "win32SystemDirectory",				VersionsXObj::m_win32SystemDirectory,		 0, 0,	400 },
	{ "dOSVersion",				VersionsXObj::m_dOSVersion,		 0, 0,	400 },
	{ "windowsVersion",				VersionsXObj::m_windowsVersion,		 0, 0,	400 },
	{ "win32Version",				VersionsXObj::m_win32Version,		 0, 0,	400 },
	{ "win32Platform",				VersionsXObj::m_win32Platform,		 0, 0,	400 },
	{ "win32Build",				VersionsXObj::m_win32Build,		 0, 0,	400 },
	{ "winNTVersion",				VersionsXObj::m_winNTVersion,		 0, 0,	400 },
	{ "getShortFileName",				VersionsXObj::m_getShortFileName,		 1, 1,	400 },
	{ "getLongFileName",				VersionsXObj::m_getLongFileName,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

VersionsXObject::VersionsXObject(ObjectType ObjectType) :Object<VersionsXObject>("Versions") {
	_objType = ObjectType;
}

void VersionsXObj::open(ObjectType type, const Common::Path &path) {
    VersionsXObject::initMethods(xlibMethods);
    VersionsXObject *xobj = new VersionsXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void VersionsXObj::close(ObjectType type) {
    VersionsXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void VersionsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("VersionsXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(VersionsXObj::m_dispose)
XOBJSTUB(VersionsXObj::m_quickTimeVersion, "000003.000000.000000.000000")
XOBJSTUB(VersionsXObj::m_win32QuickTimeVersion, "000003.000000.000000.000000")
XOBJSTUB(VersionsXObj::m_fileVersion, "000003.000000.000000.000000")
XOBJSTUB(VersionsXObj::m_windowsDirectory, "")
XOBJSTUB(VersionsXObj::m_win32WindowsDirectory, "")
XOBJSTUB(VersionsXObj::m_systemDirectory, "")
XOBJSTUB(VersionsXObj::m_win32SystemDirectory, "")
XOBJSTUB(VersionsXObj::m_dOSVersion, "")
XOBJSTUB(VersionsXObj::m_windowsVersion, "")
XOBJSTUB(VersionsXObj::m_win32Version, "")
XOBJSTUB(VersionsXObj::m_win32Platform, "")
XOBJSTUB(VersionsXObj::m_win32Build, "")
XOBJSTUB(VersionsXObj::m_winNTVersion, "")
XOBJSTUB(VersionsXObj::m_getShortFileName, "")
XOBJSTUB(VersionsXObj::m_getLongFileName, "")

}
