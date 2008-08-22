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
 * $URL$
 * $Id$
 */

#if defined(UNIX)

#include "backends/fs/posix/posix-fs.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

/**
 * Returns the last component of a given path.
 *
 * Examples:
 *			/foo/bar.txt would return /bar.txt
 *			/foo/bar/    would return /bar/
 *
 * @param str String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
const char *lastPathComponent(const Common::String &str) {
	if(str.empty())
		return "";

	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur >= start && *cur != '/') {
		--cur;
	}

	return cur + 1;
}

void POSIXFilesystemNode::setFlags() {
	struct stat st;

	_isValid = (0 == stat(_path.c_str(), &st));
	_isDirectory = _isValid ? S_ISDIR(st.st_mode) : false;
}

POSIXFilesystemNode::POSIXFilesystemNode() {
	// The root dir.
	_path = "/";
	_displayName = _path;
	_isValid = true;
	_isDirectory = true;
}

POSIXFilesystemNode::POSIXFilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/")) {
		const char *home = getenv("HOME");
		if (home != NULL && strlen(home) < MAXPATHLEN) {
			_path = home;
			// Skip over the tilda.  We know that p contains at least
			// two chars, so this is safe:
			_path += p.c_str() + 1;
		}
	} else {
		_path = p;
	}

	_displayName = lastPathComponent(_path);

	if (verify) {
		setFlags();
	}
}

AbstractFilesystemNode *POSIXFilesystemNode::getChild(const Common::String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new POSIXFilesystemNode(newPath, true);
}

bool POSIXFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	DIR *dirp = opendir(_path.c_str());
	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files if necessary
		if (dp->d_name[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((dp->d_name[0] == '.' && dp->d_name[1] == 0) || (dp->d_name[0] == '.' && dp->d_name[1] == '.')) {
			continue;
		}

		Common::String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		POSIXFilesystemNode entry(newPath, false);

#if defined(SYSTEM_NOT_SUPPORTING_D_TYPE)
		/* TODO: d_type is not part of POSIX, so it might not be supported
		 * on some of our targets. For those systems where it isn't supported,
		 * add this #elif case, which tries to use stat() instead.
		 *
		 * The d_type method is used to avoid costly recurrent stat() calls in big
		 * directories.
		 */
		entry.setFlags();
#else
		if (dp->d_type == DT_UNKNOWN) {
			// Fall back to stat()
			entry.setFlags();
		} else {
			entry._isValid = (dp->d_type == DT_DIR) || (dp->d_type == DT_REG) || (dp->d_type == DT_LNK);
			if (dp->d_type == DT_LNK) {
				struct stat st;
				if (stat(entry._path.c_str(), &st) == 0)
					entry._isDirectory = S_ISDIR(st.st_mode);
				else
					entry._isDirectory = false;
			} else {
				entry._isDirectory = (dp->d_type == DT_DIR);
			}
		}
#endif

		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == FilesystemNode::kListFilesOnly && entry._isDirectory) ||
			(mode == FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "/";

		myList.push_back(new POSIXFilesystemNode(entry));
	}
	closedir(dirp);

	return true;
}

AbstractFilesystemNode *POSIXFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	return new POSIXFilesystemNode(Common::String(start, end - start), true);
}

#endif //#if defined(UNIX)
