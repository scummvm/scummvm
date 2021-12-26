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

//=============================================================================
//
// Platform-independent Directory functions
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_DIRECTORY_H
#define AGS_SHARED_UTIL_DIRECTORY_H

#include "common/fs.h"
#include "common/stack.h"
#include "ags/lib/std/memory.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

extern const char *SAVE_FOLDER_PREFIX;

namespace Directory {

// Creates new directory (if it does not exist)
bool   CreateDirectory(const String &path);
// Makes sure all the sub-directories in the path are created. Parent path is
// not touched, and function must fail if parent path is not accessible.
bool   CreateAllDirectories(const String &parent, const String &sub_dirs);
// Sets current working directory, returns the resulting path
String SetCurrentDirectory(const String &path);
// Gets current working directory
String GetCurrentDirectory();

// Get list of subdirs found in the given directory
bool   GetDirs(const String &dir_path, std::vector<String> &dirs);
// Get list of files found in the given directory
bool   GetFiles(const String &dir_path, std::vector<String> &files);

} // namespace Directory

class FindFile {
private:
	Common::FSNode _folder;
	Common::FSList _files;
	int _index = 0;

private:
	static FindFile Open(const String &path, const String &wildcard,
		bool do_file, bool do_dir);

public:
	FindFile() {}
	~FindFile();
	static FindFile OpenFiles(const String &path, const String &wildcard = "*") {
		return Open(path, wildcard, true, false);
	}
	static FindFile OpenDirs(const String &path, const String &wildcard = "*") {
		return Open(path, wildcard, false, true);
	}
	bool AtEnd() const {
		return (_index >= (int)_files.size());
	}
	String Current() const {
		return AtEnd() ? String() : String(_files[_index].getName().c_str());
	}
	void Close();
	bool Next();
};

class FindFileRecursive {
public:
	FindFileRecursive() {}
	~FindFileRecursive();

	static FindFileRecursive Open(const String &path, const String &wildcard = "*",
		size_t max_level = -1);
	// TODO: directory mode, like in FindFile
	bool AtEnd() const {
		return _ffile.AtEnd();
	}
	String Current() const {
		return _curFile;
	}
	void Close();
	bool Next();

private:
	bool PushDir(const String &sub);
	bool PopDir();

	Common::Stack<FindFile> _fdirs;
	FindFile _fdir; // current find dir iterator
	FindFile _ffile; // current find file iterator
	int _maxLevel = -1; // max nesting level, -1 for unrestricted
	String _fullDir; // full directory path
	String _curDir; // current dir path, relative to the base path
	String _curFile; // current file path with parent dirs
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
