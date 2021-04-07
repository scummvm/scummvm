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
 */

#if defined(WIN32)

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/windows/windows-fs.h"
#include "backends/fs/stdiostream.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

// F_OK, R_OK and W_OK are not defined under MSVC, so we define them here
// For more information on the modes used by MSVC, check:
// http://msdn2.microsoft.com/en-us/library/1w06ktdy(VS.80).aspx
#ifndef F_OK
#define F_OK 0
#endif

#ifndef R_OK
#define R_OK 4
#endif

#ifndef W_OK
#define W_OK 2
#endif

bool WindowsFilesystemNode::exists() const {
	return _waccess(Win32::asciiToWinUnicode(_path.c_str()), F_OK) == 0;
}

bool WindowsFilesystemNode::isReadable() const {
	return _waccess(Win32::asciiToWinUnicode(_path.c_str()), R_OK) == 0;
}

bool WindowsFilesystemNode::isWritable() const {
	return _waccess(Win32::asciiToWinUnicode(_path.c_str()), W_OK) == 0;
}

void WindowsFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, bool hidden, LPWIN32_FIND_DATAW find_data) {
	WindowsFilesystemNode entry;
	char *asciiName = Win32::winUnicodeToAscii(find_data->cFileName);
	bool isDirectory;

	// Skip local directory (.) and parent (..)
	if (!strcmp(asciiName, ".") || !strcmp(asciiName, ".."))
		return;

	// Skip hidden files if asked
	if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !hidden)
		return;

	isDirectory = (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false);

	if ((!isDirectory && mode == Common::FSNode::kListDirectoriesOnly) ||
		(isDirectory && mode == Common::FSNode::kListFilesOnly))
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


WindowsFilesystemNode::WindowsFilesystemNode() {
	// Create a virtual root directory for standard Windows system
	_isDirectory = true;
	_isValid = false;
	_path = "";
	_isPseudoRoot = true;
}

WindowsFilesystemNode::WindowsFilesystemNode(const Common::String &p, const bool currentDir) {
	if (currentDir) {
		char path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, path);
		_path = path;
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
	DWORD fileAttribs = GetFileAttributesW(Win32::asciiToWinUnicode(_path.c_str()));

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
		wchar_t drive_buffer[100];
		GetLogicalDriveStringsW(sizeof(drive_buffer) / sizeof(wchar_t), drive_buffer);

		for (wchar_t *current_drive = drive_buffer; *current_drive;
			current_drive += wcsnlen(current_drive, MAX_PATH) + 1) {
				WindowsFilesystemNode entry;
				char drive_name[2];

				drive_name[0] = Win32::winUnicodeToAscii(current_drive)[0];
				drive_name[1] = '\0';
				entry._displayName = drive_name;
				entry._isDirectory = true;
				entry._isValid = true;
				entry._isPseudoRoot = false;
				entry._path = Win32::winUnicodeToAscii(current_drive);
				myList.push_back(new WindowsFilesystemNode(entry));
		}
	} else {
		// Files enumeration
		WIN32_FIND_DATAW desc;
		HANDLE handle;
		char searchPath[MAX_PATH + 10];

		sprintf(searchPath, "%s*", _path.c_str());

		handle = FindFirstFileW(Win32::asciiToWinUnicode(searchPath), &desc);

		if (handle == INVALID_HANDLE_VALUE)
			return false;

		addFile(myList, mode, _path.c_str(), hidden, &desc);

		while (FindNextFileW(handle, &desc))
			addFile(myList, mode, _path.c_str(), hidden, &desc);

		FindClose(handle);
	}

	return true;
}

AbstractFSNode *WindowsFilesystemNode::getParent() const {
	assert(_isValid || _isPseudoRoot);

	if (_isPseudoRoot)
		return 0;

	WindowsFilesystemNode *p = new WindowsFilesystemNode();
	if (_path.size() > 3) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path, '\\');

		p = new WindowsFilesystemNode();
		p->_path = Common::String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path, '\\');
		p->_isPseudoRoot = false;
	}

	return p;
}

Common::SeekableReadStream *WindowsFilesystemNode::createReadStream() {
	return makeFromPath(getPath(), false);
}

Common::WriteStream *WindowsFilesystemNode::createWriteStream() {
	return makeFromPath(getPath(), true);
}

//TODO fix StdioStream::makeFromPath, use it instead of this, and remove this
StdioStream *WindowsFilesystemNode::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = _wfopen(Win32::asciiToWinUnicode(path.c_str()), writeMode ? L"wb" : L"rb");
	if (handle)
		return new StdioStream(handle);
	return 0;
}

bool WindowsFilesystemNode::createDirectory() {
	if (CreateDirectoryW(Win32::asciiToWinUnicode(_path.c_str()), NULL) != 0)
		setFlags();

	return _isValid && _isDirectory;
}

#endif //#ifdef WIN32
