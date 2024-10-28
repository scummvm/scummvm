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

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_exit // Needed for IRIX's unistd.h

#include <file/file_path.h>
#include <retro_dirent.h>
#include <retro_stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "backends/platform/libretro/include/libretro-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/algorithm.h"

void LibRetroFilesystemNode::setFlags() {
	const char *fspath = _path.c_str();

	_isValid = path_is_valid(fspath);
	_isDirectory = path_is_directory(fspath);
	_isReadable = access(fspath, R_OK) == 0;
	_isWritable = access(_path.c_str(), W_OK) == 0;
}

LibRetroFilesystemNode::LibRetroFilesystemNode(const Common::String &p) {
	assert(p.size() > 0);

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/") || p.hasPrefix("~\\")) {
		Common::String homeDir = getHomeDir();
		if (homeDir.empty())
			homeDir = ".";

		// Skip over the tilda.  We know that p contains at least
		// two chars, so this is safe:
		_path = homeDir + (p.c_str() + 1);

	} else
		_path = p;

	char portable_path[_path.size()+1];
	strcpy(portable_path,_path.c_str());
	pathname_make_slashes_portable(portable_path);

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(Common::String(portable_path), '/');
	_displayName = Common::lastPathComponent(_path, '/');

	setFlags();
}

AbstractFSNode *LibRetroFilesystemNode::getChild(const Common::String &n) const {
	assert(!_path.empty());
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	// We assume here that _path is already normalized (hence don't bother to call
	//  Common::normalizePath on the final path).
	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return makeNode(newPath);
}

bool LibRetroFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	struct RDIR *dirp = retro_opendir(_path.c_str());

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((retro_readdir(dirp))) {
		const char *d_name = retro_dirent_get_name(dirp);

		// Skip 'invisible' files if necessary
		if (d_name[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((d_name[0] == '.' && d_name[1] == 0) || (d_name[0] == '.' && d_name[1] == '.')) {
			continue;
		}

		// Start with a clone of this node, with the correct path set
		LibRetroFilesystemNode entry(*this);
		entry._displayName = d_name;
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;

		entry._isValid = true;
		entry._isDirectory = retro_dirent_is_dir(dirp, entry._path.c_str());

		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) || (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		myList.push_back(new LibRetroFilesystemNode(entry));
	}
	retro_closedir(dirp);

	return true;
}

AbstractFSNode *LibRetroFilesystemNode::getParent() const {
	if (_path == "/")
		return 0; // The filesystem root has no parent

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end - 1) != '/')
		end--;

	if (end == start) {
		return 0;
	}

	AbstractFSNode * parent = makeNode(Common::String(start, end));

	if (parent->isDirectory() == false)
		return 0;

	return parent;
}

Common::SeekableReadStream *LibRetroFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *LibRetroFilesystemNode::createWriteStream(bool atomic) {
	return StdioStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);
}

bool LibRetroFilesystemNode::createDirectory() {
	if (path_mkdir(_path.c_str()))
		setFlags();

	return _isValid && _isDirectory;
}

namespace Posix {

bool assureDirectoryExists(const Common::String &dir, const char *prefix) {
	// Check whether the prefix exists if one is supplied.
	if (prefix) {
		if (!path_is_valid(prefix)) {
			return false;
		} else if (!path_is_directory(prefix)) {
			return false;
		}
	}

	// Obtain absolute path.
	Common::String path;
	if (prefix) {
		path = prefix;
		path += '/';
		path += dir;
	} else {
		path = dir;
	}

	path = Common::normalizePath(path, '/');

	const Common::String::iterator end = path.end();
	Common::String::iterator cur = path.begin();
	if (*cur == '/')
		++cur;

	do {
		if (cur + 1 != end) {
			if (*cur != '/') {
				continue;
			}

			// It is kind of ugly and against the purpose of Common::String to
			// insert 0s inside, but this is just for a local string and
			// simplifies the code a lot.
			*cur = '\0';
		}

		if (!path_mkdir(path.c_str())) {
			if (errno == EEXIST) {
				if (!path_is_valid(path.c_str())) {
					return false;
				} else if (!path_is_directory(path.c_str())) {
					return false;
				}
			} else {
				return false;
			}
		}

		*cur = '/';
	} while (cur++ != end);

	return true;
}

} // End of namespace Posix

Common::String LibRetroFilesystemNode::getHomeDir(void) {
	Common::String path;
#if defined(__WIN32)
	const char *c_homeDriveDir = getenv("HOMEDRIVE");
	const char *c_homePathDir = getenv("HOMEPATH");
	char c_homeDir[strlen(c_homeDriveDir) + strlen(c_homePathDir) + 1] = {0};
	strcat(strcat(c_homeDir, c_homeDriveDir), c_homePathDir);
#else
	const char *c_homeDir = getenv("HOME");
#endif
	if (c_homeDir && *c_homeDir)
		path = c_homeDir;

	return path;
}
