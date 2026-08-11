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

#if defined(WIN32)

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/windows/windows-fs.h"
#include "backends/fs/stdiostream.h"

bool WindowsFilesystemNode::exists() const {
	// Check whether the file actually exists
	return (GetFileAttributes(charToTchar(_path.c_str())) != INVALID_FILE_ATTRIBUTES);
}

bool WindowsFilesystemNode::isReadable() const {
	// Since all files are always readable and it is not possible to give
	// write-only permission, this is equivalent to ::exists().
	return (GetFileAttributes(charToTchar(_path.c_str())) != INVALID_FILE_ATTRIBUTES);
}

bool WindowsFilesystemNode::isWritable() const {
	// Check whether the file exists and it can be written.
	DWORD fileAttribs = GetFileAttributes(charToTchar(_path.c_str()));
	return ((fileAttribs != INVALID_FILE_ATTRIBUTES) && (!(fileAttribs & FILE_ATTRIBUTE_READONLY)));
}

void WindowsFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, bool hidden, WIN32_FIND_DATA* find_data) {
	// Skip local directory (.) and parent (..)
	if (!_tcscmp(find_data->cFileName, TEXT(".")) ||
		!_tcscmp(find_data->cFileName, TEXT("..")))
		return;

	// Skip hidden files if asked
	if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !hidden)
		return;

	bool isDirectory = ((find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

	if ((!isDirectory && mode == Common::FSNode::kListDirectoriesOnly) ||
		(isDirectory && mode == Common::FSNode::kListFilesOnly))
		return;

	const char *fileName = tcharToChar(find_data->cFileName);

	WindowsFilesystemNode entry;
	entry._isDirectory = isDirectory;
	entry._displayName = fileName;
	entry._path = base;
	entry._path += fileName;
	if (entry._isDirectory)
		entry._path += "\\";
	entry._isValid = true;
	entry._isPseudoRoot = false;

	list.push_back(new WindowsFilesystemNode(entry));
}

const char* WindowsFilesystemNode::tcharToChar(const TCHAR *str) {
#ifndef UNICODE
	return str;
#else
	static char multiByteString[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, str, _tcslen(str) + 1, multiByteString, MAX_PATH, nullptr, nullptr);
	return multiByteString;
#endif
}

const TCHAR* WindowsFilesystemNode::charToTchar(const char *str) {
#ifndef UNICODE
	return str;
#else
	static wchar_t wideCharString[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, str, strlen(str) + 1, wideCharString, MAX_PATH);
	return wideCharString;
#endif
}

WindowsFilesystemNode::WindowsFilesystemNode() {
	// Create a virtual root directory for standard Windows system
	_isDirectory = true;
	_isValid = false;
	_path = "";
	_isPseudoRoot = true;
}

WindowsFilesystemNode::WindowsFilesystemNode(const Common::String &p, const bool currentDir) {
	if (currentDir) {
		TCHAR path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, path);
		_path = tcharToChar(path);
	} else {
		assert(p.size() > 0);
		_path = p;
	}

	_displayName = lastPathComponent(_path, '\\');

	setFlags();

	_isPseudoRoot = false;
}

void WindowsFilesystemNode::setFlags() {
	// Check whether it is a directory, and whether the file actually exists
	DWORD fileAttribs = GetFileAttributes(charToTchar(_path.c_str()));

	if (fileAttribs == INVALID_FILE_ATTRIBUTES) {
		_isDirectory = false;
		_isValid = false;
	} else {
		_isDirectory = ((fileAttribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
		_isValid = true;
		// Add a trailing slash, if necessary.
		if (_isDirectory && _path.lastChar() != '\\') {
			_path += '\\';
		}
	}
}

AbstractFSNode *WindowsFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	return new WindowsFilesystemNode(newPath, false);
}

bool WindowsFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	if (_isPseudoRoot) {
		// Drives enumeration
		TCHAR drive_buffer[100];
		GetLogicalDriveStrings(sizeof(drive_buffer) / sizeof(TCHAR), drive_buffer);

		for (TCHAR *current_drive = drive_buffer; *current_drive;
			current_drive += _tcslen(current_drive) + 1) {
				WindowsFilesystemNode entry;
				char drive_name[2];

				drive_name[0] = tcharToChar(current_drive)[0];
				drive_name[1] = '\0';
				entry._displayName = drive_name;
				entry._isDirectory = true;
				entry._isValid = true;
				entry._isPseudoRoot = false;
				entry._path = tcharToChar(current_drive);
				myList.push_back(new WindowsFilesystemNode(entry));
		}
	} else {
		// Files enumeration
		WIN32_FIND_DATA desc;
		HANDLE handle;
		char searchPath[MAX_PATH + 10];

		Common::sprintf_s(searchPath, "%s*", _path.c_str());

		handle = FindFirstFile(charToTchar(searchPath), &desc);

		if (handle == INVALID_HANDLE_VALUE)
			return false;

		addFile(myList, mode, _path.c_str(), hidden, &desc);

		while (FindNextFile(handle, &desc))
			addFile(myList, mode, _path.c_str(), hidden, &desc);

		FindClose(handle);
	}

	return true;
}

AbstractFSNode *WindowsFilesystemNode::getParent() const {
	assert(_isValid || _isPseudoRoot);

	if (_isPseudoRoot)
		return nullptr;

	WindowsFilesystemNode *p;
	if (_path.size() > 3) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path, '\\');

		p = new WindowsFilesystemNode();
		p->_path = Common::String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path, '\\');
		p->_isPseudoRoot = false;
	} else {
		// pseudo root
		p = new WindowsFilesystemNode();
	}

	return p;
}

Common::SeekableReadStream *WindowsFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *WindowsFilesystemNode::createWriteStream(bool atomic) {
	return StdioStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);
}

bool WindowsFilesystemNode::createDirectory() {
	if (CreateDirectory(charToTchar(_path.c_str()), nullptr) != 0)
		setFlags();

	return _isValid && _isDirectory;
}

#endif //#ifdef WIN32
