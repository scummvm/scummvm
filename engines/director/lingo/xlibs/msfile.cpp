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

#include "common/savefile.h"
#include "common/file.h"
#include "gui/filebrowser-dialog.h"
#include "gui/browser.h"
#include "director/director.h"
#include "director/picture.h"
#include "director/types.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/msfile.h"

/*************************************
 *
 * USED IN:
 * The Simpsons Cartoon Studio
 *
 *************************************/

/*
"I      mNew                      --Instantiates XObject"	1028:0023	string	56
"X      mDispose                  --Removes XObject Instance"	1028:005b	string	60
"S      mName                     --Returns XObject Name"	1028:0097	string	56
"SS     mGetFullDirList, dospath  --Returns MS-DOS Full Directory List (use *.*)"	1028:00cf	string	80
"SS     mGetFileList, dospath     --Returns MS-DOS File List (use *.*)"	1028:011f	string	70
"SS     mGetDirList, dospath      --Returns MS-DOS Directory List (use *.*)"	1028:0165	string	75
"II     mGetDiskFree, drive       --Returns Free disk space, drive is 0 for current, 1 for A, ..."	1028:01b0	string	97
"SS     mGetVolLabel, drive       --Returns Drive Label (use d:\\*.*)"	1028:0211	string	68
"IS     mCreateDir, dospath       --Creates an MS-DOS Directory"	1028:0255	string	63
"IS     mRemoveDir, dospath       --Removes an MS-DOS Directory"	1028:0294	string	63
"S      mGetCurrDir               --Returns Current Working Directory"	1028:02d3	string	69
"IS     mSetCurrDir, dospath      --Sets Current Working Directory"	1028:0318	string	66
"IS     mRemoveFile, dospath      --Deletes File named by dospath"	1028:035a	string	65
"ISS    mRenameFile, oldpath ,newpath      --Renames File named by oldpath"	1028:039b	string	74
"ISS    mCopyFile, srcpath, destpath       --Copys srcpath to destpath, overwrites"	1028:03e5	string	82
"ISSI   mCopyFiles, srcpath, destpath, incsubdir   --Copys srcpath to destpath, overwrites"	1028:0437	string	90
"ISSSS  mSetAppINI, app, key, string, file --Sets app.ini"	1028:0491	string	57
"SSSS   mGetAppINI, app, key, file         --Gets app.ini"	1028:04ca	string	57
"S      mGetWindowsDirectory               --Gets Windows directory path"	1028:0503	string	72
"I      mGetWinVer                         --Gets Windows major version number"	1028:054b	string	78
"I      mRestartWindows                    --Gets Windows major version number"	1028:0599	string	78
"SSSS   mOpenFileDlg, title, defspec, file --Displays Director Open File Dialog Box"	1028:05e7	string	83
"SSSS   mSaveFileDlg, title, defspec, file --Displays Director Save File Dialog Box"	1028:063a	string	83
"SSSS   mGetOFileName, deffile, filter, defext --Displays Open File Dialog Box"	1028:068d	string	78
"SSSS   mGetSFileName, deffile, filter, defext --Displays Save File Dialog Box"	1028:06db	string	78
"SSSS   mDisplayDlg, title, message, defspec --Displays Dialog Box"	1028:0729	string	66
"ISSSS  mProgMgrAdd, group, command, title, icon  --\r\r"	1028:076b	string	57
*/

namespace Director {

const char *const MSFile::xlibName = "MSFile";
const XlibFileDesc MSFile::fileNames[] = {
	{"MSFile", nullptr},
	{"MSFILE16", nullptr},
	{nullptr, nullptr},
};

static const MethodProto xlibMethods[] = {
	// XObject
	{"new",                 MSFile::m_new,                 0, 0, 400},
	{"dispose",             MSFile::m_dispose,             0, 0, 400},
	{"name",                MSFile::m_name,                0, 1, 400},
	{"getFullDirList",      MSFile::m_getFullDirList,      1, 1, 400},
	{"getFileList",         MSFile::m_getFileList,         1, 1, 400},
	{"getDirList",          MSFile::m_getDirList,          1, 1, 400},
	{"getDiskFree",         MSFile::m_getDiskFree,         1, 1, 400},
	{"getVolLabel",         MSFile::m_getVolLabel,         1, 1, 400},
	{"createDir",           MSFile::m_createDir,           1, 1, 400},
	{"removeDir",           MSFile::m_removeDir,           1, 1, 400},
	{"getCurrDir",          MSFile::m_getCurrDir,          0, 1, 400},
	{"setCurrDir",          MSFile::m_setCurrDir,          1, 1, 400},
	{"removeFile",          MSFile::m_removeFile,          1, 1, 400},
	{"renameFile",          MSFile::m_renameFile,          2, 1, 400},
	{"copyFile",            MSFile::m_copyFile,            2, 1, 400},
	{"copyFiles",           MSFile::m_copyFiles,           3, 1, 400},
	{"setAppINI",           MSFile::m_setAppINI,           4, 1, 400},
	{"getAppINI",           MSFile::m_getAppINI,           3, 1, 400},
	{"getWindowsDirectory", MSFile::m_getWindowsDirectory, 0, 1, 400},
	{"getWinVer",           MSFile::m_getWinVer,           0, 1, 400},
	{"restartWindows",      MSFile::m_restartWindows,      0 ,1, 400},
	{"openFileDlg",         MSFile::m_openFileDlg,         3, 1, 400},
	{"saveFileDlg",         MSFile::m_saveFileDlg,         3, 1, 400},
	{"getOFileName",        MSFile::m_getOFileName,        3, 1, 400},
	{"getSFileName",        MSFile::m_getSFileName,        3, 1, 400},
	{"displayDlg",          MSFile::m_displayDlg,          3, 1, 400},
	{"progMgrAdd",          MSFile::m_progMgrAdd,          4, 1, 400},
	{nullptr, nullptr, 0, 0, 0}};

static const BuiltinProto xlibBuiltins[] = {
	{nullptr, nullptr, 0, 0, 0, VOIDSYM}};

MSFileObject::MSFileObject(ObjectType objType) : Object<MSFileObject>("MSFile") {
	_objType = objType;
	_lastError = msErrorNone;
}

MSFileObject::MSFileObject(const MSFileObject &obj) : Object<MSFileObject>(obj) {
	_objType = obj.getObjType();
	_lastError = msErrorNone;
}

MSFileObject::~MSFileObject() {
}

void MSFileObject::dispose() {
	_disposed = true;
}

bool MSFileObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MSFileObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MSFile::xlibName);
	warning("MSFile::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MSFile::m_new(int nargs) {
	Datum result = g_lingo->_state->me;
	g_lingo->push(result);
}

XOBJSTUB(MSFile::m_dispose, 0)
XOBJSTUB(MSFile::m_name, "")
XOBJSTUB(MSFile::m_getFullDirList, "")
XOBJSTUB(MSFile::m_getFileList, "")
XOBJSTUB(MSFile::m_getDirList, "")

void MSFile::m_getDiskFree(int nargs) {
	// We don't care about the drive letter. We will always just return 500 MB.
	g_lingo->pop();

	// 500 MB in KB
	g_lingo->push(Datum(500 * 1024 * 1024));
}

void MSFile::m_getVolLabel(int nargs) {
	Datum d = g_lingo->pop();

	Common::String volLabel;

	const char *gameId = g_director->getGameId();
	if (strncmp(gameId, "simpsonsstudio", 14) == 0 || strncmp(gameId, "simpsonsplayer", 14) == 0) {
		volLabel = "Simpsons";
	} else {
		warning("MSFile::m_getVolLabel(): Unsupported gameid '%s'", gameId);
	}

	g_lingo->push(volLabel);
}

XOBJSTUB(MSFile::m_createDir, 0)
XOBJSTUB(MSFile::m_removeDir, 0)
XOBJSTUB(MSFile::m_getCurrDir, "")
XOBJSTUB(MSFile::m_setCurrDir, 0)
XOBJSTUB(MSFile::m_removeFile, 0)
XOBJSTUB(MSFile::m_renameFile, 0)

Common::Path MSFile::resolveSourceFilePath(const Common::String& srcParam) {
	return findPath(srcParam);
}

Common::SeekableReadStream *MSFile::openSourceStream(const Common::Path &srcFilePath, const Common::String &srcParam, char dirSeparator) {
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	Common::String prefix = savePrefix();

	// The source file may be a save file in the ScummVM save folder.
	Common::String saveFileName = lastPathComponent(srcParam, dirSeparator);

	// Modify the source file name to match the name of the potential save file
	if (!saveFileName.hasSuffixIgnoreCase(".txt")) {
		saveFileName += ".txt";
	}

	if (!saveFileName.hasPrefixIgnoreCase(prefix)) {
		saveFileName = prefix + saveFileName;
	}

	// Try to open the save file
	Common::SeekableReadStream *srcReadStream = saves->openForLoading(saveFileName);

	// If the save file was not found then we will try opening the file directly
	if (!srcReadStream) {
		// We have a relative path so we can open it using Common::File
		Common::File *srcFile = new Common::File();
		if (srcFilePath.empty() || !srcFile->open(srcFilePath)) {
			warning("MSFile::m_copyFile(): Error opening file %s", srcFilePath.toString(Common::Path::kNativeSeparator).c_str());
			delete srcFile;
			return nullptr;
		}
		srcReadStream = srcFile;
	}

	return srcReadStream;
}

Common::SeekableWriteStream *MSFile::openDestinationStream(Common::String &destFileName) {
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	Common::String prefix = savePrefix();

	Common::OutSaveFile *destFile = nullptr;

	if (!destFileName.hasSuffixIgnoreCase(".txt")) {
		destFileName += ".txt";
	}

	if (!destFileName.hasPrefixIgnoreCase(prefix)) {
		destFileName = prefix + destFileName;
	}

	destFile = saves->openForSaving(destFileName, false);

	return destFile;
}

bool MSFile::copyStream(Common::SeekableReadStream* srcStream, Common::SeekableWriteStream* destStream) {
	if (srcStream && destStream) {
		destStream->writeStream(srcStream);
		destStream->finalize();
		bool success = !destStream->err();
		delete destStream;
		return success;
	}

	return false;
}

bool MSFile::copyGameFile(const Common::String& srcParam, const Common::String& destParam) {
	char dirSeparator = g_director->_dirSeparator;
	Common::Path srcFilePath = resolveSourceFilePath(srcParam);
	Common::String destFileName = Common::lastPathComponent(destParam, dirSeparator);

	Common::SeekableReadStream *srcStream = openSourceStream(srcFilePath, srcParam, dirSeparator);
	Common::SeekableWriteStream *destStream = openDestinationStream(destFileName);

	return copyStream(srcStream, destStream);
}

void MSFile::m_copyFile(int nargs) {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	// The source and destination file parameters are absolute paths from the games perspective but do not match the paths for ScummVM
	Common::String srcParam = d1.asString();
	Common::String destParam = d2.asString();

	bool success = copyGameFile(srcParam, destParam);

	g_lingo->push(Datum(success ? 0 : 1)); // Push 0 for success, 1 for failure
}

XOBJSTUB(MSFile::m_copyFiles, 0)
XOBJSTUB(MSFile::m_setAppINI, 0)
XOBJSTUB(MSFile::m_getAppINI, "")
XOBJSTUB(MSFile::m_getWindowsDirectory, "")
XOBJSTUB(MSFile::m_getWinVer, 0)
XOBJSTUB(MSFile::m_restartWindows, 0)
XOBJSTUB(MSFile::m_openFileDlg, "")
XOBJSTUB(MSFile::m_saveFileDlg, "")

void MSFile::m_getOFileName(int nargs) {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String defFile = d1.asString();
	Common::String filter = d2.asString();
	Common::String defExt = d3.asString();

	g_lingo->push(getFileNameFromModal(false, defFile, filter, defExt.c_str()));
}

XOBJSTUB(MSFile::m_getSFileName, "")
XOBJSTUB(MSFile::m_displayDlg, "")
XOBJSTUB(MSFile::m_progMgrAdd, 0)

void MSFile::open(ObjectType type, const Common::Path &path) {
	MSFileObject::initMethods(xlibMethods);
	MSFileObject *xobj = new MSFileObject(type);
	if (g_director->getVersion() >= 500)
		g_lingo->_openXtras.push_back(xlibName);
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void MSFile::close(ObjectType type) {
	g_lingo->_globalvars[xlibName] = Datum();
}
} // End of namespace Director
