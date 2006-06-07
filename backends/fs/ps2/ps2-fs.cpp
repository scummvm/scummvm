/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
#include "backends/fs/fs.h"
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "asyncfio.h"
#include "systemps2.h"

extern AsyncFio fio;
extern OSystem_PS2 *g_systemPs2;

class Ps2FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isRoot;
	String _path;

public:
	Ps2FilesystemNode(void);
	Ps2FilesystemNode(const Ps2FilesystemNode *node);
	Ps2FilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return !_isRoot; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	//virtual FSList listDir(ListMode) const;
	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new Ps2FilesystemNode(this); }
	virtual AbstractFilesystemNode *child(const String &name) const;
};

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return AbstractFilesystemNode::getRoot();
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot(void) {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new Ps2FilesystemNode(path);
}

Ps2FilesystemNode::Ps2FilesystemNode(void) {
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

bool Ps2FilesystemNode::listDir(AbstractFSList &list, ListMode mode) const {
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

AbstractFilesystemNode *Ps2FilesystemNode::parent() const {
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

AbstractFilesystemNode *Ps2FilesystemNode::child(const String &name) const {
	if (!_isDirectory)
		return NULL;

	char listDir[256];
	sprintf(listDir, "%s/", _path.c_str());
	int fd = fio.dopen(listDir);
	
	if (fd >= 0) {
		iox_dirent_t dirent;

		while (fio.dread(fd, &dirent) > 0) {
			if (strcmp(name.c_str(), dirent.name) == 0) {
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
