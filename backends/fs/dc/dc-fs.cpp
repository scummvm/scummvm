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

#if defined(__DC__)

#include "common/stdafx.h"
#include "backends/fs/abstract-fs.h"

#include <ronin/cdfs.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Implementation of the ScummVM file system API based on POSIX.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class RoninCDFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a RoninCDFilesystemNode with the root node as path.
	 */
	RoninCDFilesystemNode();
	
	/**
	 * Creates a RoninCDFilesystemNode for a given path.
	 * 
	 * @param path String with the path the new node should point to.
	 * @param verify true if the isValid and isDirectory flags should be verified during the construction.
	 */
	RoninCDFilesystemNode(const String &path, bool verify);

	virtual bool exists() const { return true; }		//FIXME: this is just a stub
	virtual String getDisplayName() const { return _displayName; }
	virtual String getName() const { return _displayName; }
	virtual String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isValid() const { return _isValid; }
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub

	virtual AbstractFilesystemNode *getChild(const String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *getParent() const;
};

/**
 * Returns the last component of a given path.
 * 
 * Examples:
 * 			/foo/bar.txt would return /bar.txt
 * 			/foo/bar/    would return /bar/
 *  
 * @param str String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur >= start && *cur != '/') {
		--cur;
	}

	return cur + 1;
}

RoninCDFilesystemNode::RoninCDFilesystemNode() {
	// The root dir.
	_path = "/";
	_displayName = _path;
	_isValid = true;
	_isDirectory = true;
}

RoninCDFilesystemNode::RoninCDFilesystemNode(const String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path);
	_isValid = true;
	_isDirectory = true;

	if (verify) {
		int fd;

		if((fd = open(_path.c_str(), O_RDONLY)) >= 0) {
			close(fd);
			_isDirectory = false;
		}
		else if((fd = open(_path.c_str(), O_DIR|O_RDONLY)) >= 0) {
			close(fd);
		}
		else {
			_isValid = false;
		}
	}
}

AbstractFilesystemNode *RoninCDFilesystemNode::getChild(const String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	
	String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new RoninCDFilesystemNode(newPath, true);
}

bool RoninCDFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);
	
	DIR *dirp = opendir(_path.c_str());
	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		RoninCDFilesystemNode entry(newPath, false);

		entry._isDirectory = dp->d_size < 0;

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
			
		myList.push_back(new RoninCDFilesystemNode(entry));
	}
	closedir(dirp);
	
	return true;
}

AbstractFilesystemNode *RoninCDFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	return new RoninCDFilesystemNode(String(start, end - start), false);
}

#endif // defined(__DC__)
