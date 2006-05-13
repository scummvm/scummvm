/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifdef WIN32

#include "common/stdafx.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

/*
 * Implementation of the ScummVM file system API based on Windows API.
 */

class WindowsFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
	String _path;

public:
	WindowsFilesystemNode();
	WindowsFilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;

private:
	static char *toAscii(TCHAR *x);
	static const TCHAR* toUnicode(const char *x);
	static void addFile (AbstractFSList &list, ListMode mode, const char *base, WIN32_FIND_DATA* find_data);
};


static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '\\') {
		--cur;
	}

	return cur + 1;
}

char* WindowsFilesystemNode::toAscii(TCHAR *x) {

#ifndef UNICODE
	return (char*)x;
#else
	static char asciiString[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, x, _tcslen(x) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	return asciiString;
#endif
}

const TCHAR* WindowsFilesystemNode::toUnicode(const char *x) {
#ifndef UNICODE
	return (const TCHAR *)x;
#else
	static TCHAR unicodeString[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, x, strlen(x) + 1, unicodeString, sizeof(unicodeString) / sizeof(TCHAR));
	return unicodeString;
#endif
}

void WindowsFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, WIN32_FIND_DATA* find_data) {
	WindowsFilesystemNode entry;
	char *asciiName = toAscii(find_data->cFileName);
	bool isDirectory;

	// Skip local directory (.) and parent (..)
	if (!strcmp(asciiName, ".") || !strcmp(asciiName, ".."))
		return;

	isDirectory = (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false);

	if ((!isDirectory && mode == FilesystemNode::kListDirectoriesOnly) ||
		(isDirectory && mode == FilesystemNode::kListFilesOnly))
		return;

	entry._isDirectory = isDirectory;
	entry._displayName = asciiName;
	entry._path = base;
	entry._path += asciiName;
	if (entry._isDirectory)
		entry._path += "\\";
	entry._isValid = true;
	entry._isPseudoRoot = false;

	list.push_back(new WindowsFilesystemNode(entry));
}

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return new WindowsFilesystemNode((const char *)".");
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new WindowsFilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new WindowsFilesystemNode(path);
}

WindowsFilesystemNode::WindowsFilesystemNode() {
	_isDirectory = true;
#ifndef _WIN32_WCE
	// Create a virtual root directory for standard Windows system
	_isValid = false;
	_path = "";
	_isPseudoRoot = true;
#else
	_displayName = "Root";
	// No need to create a pseudo root directory on Windows CE
	_isValid = true;
	_path = "\\";
	_isPseudoRoot = false;
#endif
}

WindowsFilesystemNode::WindowsFilesystemNode(const String &p) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path);

	// Check whether it is a directory, and whether the file actually exists
	DWORD fileAttribs = GetFileAttributes(toUnicode(_path.c_str()));

	if (fileAttribs == INVALID_FILE_ATTRIBUTES) {
		_isValid = false;
		_isDirectory = false;
	} else {
		_isValid = true;
		_isDirectory = ((fileAttribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
	}
	_isPseudoRoot = false;
}

bool WindowsFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);

	if (_isPseudoRoot) {
#ifndef _WIN32_WCE
		// Drives enumeration
		TCHAR drive_buffer[100];
		GetLogicalDriveStrings(sizeof(drive_buffer) / sizeof(TCHAR), drive_buffer);

		for (TCHAR *current_drive = drive_buffer; *current_drive;
			current_drive += _tcslen(current_drive) + 1) {
				WindowsFilesystemNode entry;
				char drive_name[2];

				drive_name[0] = toAscii(current_drive)[0];
				drive_name[1] = '\0';
				entry._displayName = drive_name;
				entry._isDirectory = true;
				entry._isValid = true;
				entry._isPseudoRoot = false;
				entry._path = toAscii(current_drive);
				myList.push_back(new WindowsFilesystemNode(entry));
		}
#endif
	}
	else {
		// Files enumeration
		WIN32_FIND_DATA desc;
		HANDLE handle;
		char searchPath[MAX_PATH + 10];

		sprintf(searchPath, "%s*", _path.c_str());

		handle = FindFirstFile(toUnicode(searchPath), &desc);
		if (handle == INVALID_HANDLE_VALUE)
			return false;
		addFile(myList, mode, _path.c_str(), &desc);
		while (FindNextFile(handle, &desc))
			addFile(myList, mode, _path.c_str(), &desc);

		FindClose(handle);
	}

	return true;
}

AbstractFilesystemNode *WindowsFilesystemNode::parent() const {
	assert(_isValid || _isPseudoRoot);
	if (_isPseudoRoot)
		return 0;
	WindowsFilesystemNode *p = new WindowsFilesystemNode();
	if (_path.size() > 3) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p = new WindowsFilesystemNode();
		p->_path = String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
		p->_isPseudoRoot = false;
	}
	return p;
}

AbstractFilesystemNode *WindowsFilesystemNode::child(const String &name) const {
	assert(_isDirectory);
	String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += name;

	// Check whether the directory actually exists
	DWORD fileAttribs = GetFileAttributes(toUnicode(newPath.c_str()));
	if (fileAttribs == INVALID_FILE_ATTRIBUTES)
		return 0;

	WindowsFilesystemNode *p = new WindowsFilesystemNode(newPath);
	return p;
}

#endif // WIN32
