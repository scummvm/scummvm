/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "common/file.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::unifySeparators(const AnsiString &path) {
	AnsiString newPath = path;

	for (uint32 i = 0; i < newPath.size(); i++) {
		if (newPath[i] == '\\')
			newPath.setChar('/', i);
	}

	return newPath;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::normalizeFileName(const AnsiString &path) {
	AnsiString newPath = unifySeparators(path);
	newPath.toLowercase();
	return newPath;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::combine(const AnsiString &path1, const AnsiString &path2) {
	AnsiString newPath1 = unifySeparators(path1);
	AnsiString newPath2 = unifySeparators(path2);

	if (!StringUtil::endsWith(newPath1, "/", true) && !StringUtil::startsWith(newPath2, "/", true))
		newPath1 += "/";

	return newPath1 + newPath2;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getDirectoryName(const AnsiString &path) {
	AnsiString newPath = unifySeparators(path);

	//size_t pos = newPath.find_last_of(L'/');
	Common::String filename = getFileName(path);
	return Common::String(path.c_str(), path.size() - filename.size());
	//if (pos == AnsiString::npos) return "";
	//else return newPath.substr(0, pos + 1);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getFileName(const AnsiString &path) {
	AnsiString newPath = unifySeparators(path);

	//size_t pos = newPath.find_last_of(L'/'); TODO REMOVE.
	Common::String lastPart = Common::lastPathComponent(newPath, '/');
	if (lastPart[lastPart.size() - 1 ] != '/')
		return lastPart;
	else
		return path;
	//if (pos == AnsiString::npos) return path;
	//else return newPath.substr(pos + 1);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getFileNameWithoutExtension(const AnsiString &path) {
	AnsiString fileName = getFileName(path);

	//size_t pos = fileName.find_last_of('.'); //TODO REMOVE!
	// TODO: Prettify this.
	AnsiString extension = Common::lastPathComponent(fileName, '.');
	for (int i = 0; i < extension.size() + 1; i++) {
		fileName.deleteLastChar();
	}
//	Common::String filename = Common::String(fileName.c_str(), fileName.size() - extension.size() + 1);
	return fileName;
	//if (pos == AnsiString::npos) return fileName;
	//else return fileName.substr(0, pos);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getExtension(const AnsiString &path) {
	AnsiString fileName = getFileName(path);

	//size_t pos = fileName.find_last_of('.');
	return Common::lastPathComponent(path, '.');
	//if (pos == AnsiString::npos) return "";
	//else return fileName.substr(pos);
}


//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getSafeLogFileName() {
	AnsiString logFileName = getUserDirectory();

/*#ifdef __WIN32__
	char moduleName[MAX_PATH_LENGTH];
	::GetModuleFileName(NULL, moduleName, MAX_PATH_LENGTH);

	AnsiString fileName = GetFileNameWithoutExtension(moduleName) + ".log";
	fileName = Combine("/Wintermute Engine/Logs/", fileName);
	logFileName = Combine(logFileName, fileName);

#else*/
	// !PORTME
	logFileName = combine(logFileName, "/Wintermute Engine/wme.log");
//#endif

	createDirectory(getDirectoryName(logFileName));
	return logFileName;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::createDirectory(const AnsiString &path) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::matchesMask(const AnsiString &fileName, const AnsiString &mask) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::fileExists(const AnsiString &fileName) {
	warning("PathUtil::FileExists(%s)", fileName.c_str());

	Common::File stream;

	stream.open(fileName.c_str());
	bool ret = stream.isOpen();
	stream.close();

	return ret;
}


//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::getUserDirectory() {
	warning("PathUtil::GetUserDirectory - stubbed");
	AnsiString userDir = "./";
#if 0
#ifdef __WIN32__
	char buffer[MAX_PATH_LENGTH];
	buffer[0] = '\0';
	LPITEMIDLIST pidl = NULL;
	LPMALLOC pMalloc;
	if (DID_SUCCEED(SHGetMalloc(&pMalloc))) {
		SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
		if (pidl) {
			SHGetPathFromIDList(pidl, buffer);
		}
		pMalloc->Free(pidl);
		userDir = AnsiString(buffer);
	}
#elif __MACOSX__
	FSRef fileRef;
	OSStatus error = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &fileRef);
	if (error == noErr) {
		char buffer[MAX_PATH_LENGTH];
		error = FSRefMakePath(&fileRef, (UInt8 *)buffer, sizeof(buffer));
		if (error == noErr)
			userDir = buffer;

	}
#elif __IPHONEOS__
	char path[MAX_PATH_LENGTH];
	IOS_GetDataDir(path);
	userDir = AnsiString(path);
#endif
#endif // 0
	return userDir;
}

} // end of namespace WinterMute
