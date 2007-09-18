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

#include "backends/fs/abstract-fs.h"
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "asyncfio.h"
#include "systemps2.h"

extern AsyncFio fio;
extern OSystem_PS2 *g_systemPs2;

/**
 * Implementation of the ScummVM file system API based on the Ps2SDK.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class Ps2FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	String _path;
	bool _isDirectory;
	bool _isRoot;

public:
	/**
	 * Creates a PS2FilesystemNode with the root node as path.
	 */
	Ps2FilesystemNode();
	
	/**
	 * Creates a PS2FilesystemNode for a given path.
	 * 
	 * @param path String with the path the new node should point to.
	 */
	Ps2FilesystemNode(const String &path);
	
	/**
	 * Copy constructor.
	 */
	Ps2FilesystemNode(const Ps2FilesystemNode *node);

	virtual bool exists() const { return true; }		//FIXME: this is just a stub
	virtual String getDisplayName() const { return _displayName; }
	virtual String getName() const { return _displayName; }
	virtual String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isValid() const { return !_isRoot; }
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub

	virtual AbstractFilesystemNode *clone() const { return new Ps2FilesystemNode(this); }
	virtual AbstractFilesystemNode *getChild(const String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFilesystemNode *getParent() const;
};

Ps2FilesystemNode::Ps2FilesystemNode() {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "PlayStation 2";
	_path = "";
}

Ps2FilesystemNode::Ps2FilesystemNode(const String &path) {
	_path = path;
	_isDirectory = true;
	if (strcmp(path.c_str(), "") == 0) {
		_isRoot = true;
		_displayName = String("PlayStation 2");
	} else {
		_isRoot = false;
		const char *dsplName = NULL, *pos = path.c_str();
		while (*pos)
			if (*pos++ == '/')
				dsplName = pos;
		if (dsplName)
			_displayName = String(dsplName);
		else {
			if (strncmp(path.c_str(), "cdfs", 4) == 0)
				_displayName = "DVD Drive";
			else if (strncmp(path.c_str(), "mass", 4) == 0)
				_displayName = "USB Mass Storage";
			else
				_displayName = "Harddisk";
		}
	}
}

Ps2FilesystemNode::Ps2FilesystemNode(const Ps2FilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
	_isRoot = node->_isRoot;
}

AbstractFilesystemNode *Ps2FilesystemNode::getChild(const String &n) const {
	if (!_isDirectory)
		return NULL;

	char listDir[256];
	sprintf(listDir, "%s/", _path.c_str());
	int fd = fio.dopen(listDir);
	
	if (fd >= 0) {
		iox_dirent_t dirent;
		
		while (fio.dread(fd, &dirent) > 0) {
			if (strcmp(n.c_str(), dirent.name) == 0) {
				Ps2FilesystemNode *dirEntry = new Ps2FilesystemNode();

				dirEntry->_isDirectory = (bool)(dirent.stat.mode & FIO_S_IFDIR);
				dirEntry->_isRoot = false;

				dirEntry->_path = _path;
				dirEntry->_path += "/";
				dirEntry->_path += dirent.name;

				dirEntry->_displayName = dirent.name;

				fio.dclose(fd);
				return dirEntry;
			}
		}
		fio.dclose(fd);
	}
	
	return NULL;
}

bool Ps2FilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	//TODO: honor the hidden flag
	
	if (!_isDirectory)
		return false;

	if (_isRoot) {
		Ps2FilesystemNode dirEntry;
		dirEntry._isDirectory = true;
		dirEntry._isRoot = false;
		dirEntry._path = "cdfs:";
		dirEntry._displayName = "DVD Drive";
		list.push_back(new Ps2FilesystemNode(&dirEntry));

		if (g_systemPs2->hddPresent()) {
			dirEntry._path = "pfs0:";
			dirEntry._displayName = "Harddisk";
			list.push_back(new Ps2FilesystemNode(&dirEntry));
		}

		if (g_systemPs2->usbMassPresent()) {
			dirEntry._path = "mass:";
			dirEntry._displayName = "USB Mass Storage";
			list.push_back(new Ps2FilesystemNode(&dirEntry));
		}
		return true;
	} else {
		char listDir[256];
		int fd;
		
		if (_path.lastChar() == '/')
			fd = fio.dopen(_path.c_str());
		else {
			sprintf(listDir, "%s/", _path.c_str());
			fd = fio.dopen(listDir);
		}
		
		if (fd >= 0) {
			iox_dirent_t dirent;
			Ps2FilesystemNode dirEntry;
			int dreadRes;
			while ((dreadRes = fio.dread(fd, &dirent)) > 0) {
				if (dirent.name[0] == '.')
					continue; // ignore '.' and '..'
				if (((mode == FilesystemNode::kListDirectoriesOnly) && (dirent.stat.mode & FIO_S_IFDIR)) ||
					((mode == FilesystemNode::kListFilesOnly) && !(dirent.stat.mode & FIO_S_IFDIR)) ||
					(mode == FilesystemNode::kListAll)) {

					dirEntry._isDirectory = (bool)(dirent.stat.mode & FIO_S_IFDIR);
					dirEntry._isRoot = false;

					dirEntry._path = _path;
					if (_path.lastChar() != '/')
						dirEntry._path += "/";
					dirEntry._path += dirent.name;

					dirEntry._displayName = dirent.name;

					list.push_back(new Ps2FilesystemNode(&dirEntry));
				}
			}
			fio.dclose(fd);
			return true;
		} else
			return false;
	}
}

AbstractFilesystemNode *Ps2FilesystemNode::getParent() const {
	if (_isRoot)
		return new Ps2FilesystemNode(this);

	const char *slash = NULL;
	const char *cnt = _path.c_str();

	while (*cnt) {
		if (*cnt == '/')
			slash = cnt;
		cnt++;
	}

	if (slash)
		return new Ps2FilesystemNode(String(_path.c_str(), slash - _path.c_str()));
	else
		return new Ps2FilesystemNode();
}
