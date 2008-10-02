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
#include "backends/fs/stdiostream.h"
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "backends/platform/ps2/asyncfio.h"
#include "backends/platform/ps2/fileio.h"
#include "backends/platform/ps2/systemps2.h"
#include "backends/platform/ps2/ps2debug.h"

#define DEFAULT_MODE (FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IROTH | FIO_S_IWOTH)

extern AsyncFio fio;
extern OSystem_PS2 *g_systemPs2;

/**
 * Implementation of the ScummVM file system API based on the Ps2SDK.
 * 
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class Ps2FilesystemNode : public AbstractFSNode {

friend class Ps2FilesystemFactory;

protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isRoot;

private:
	char *getDeviceDescription(const char *path) const;
	bool getDirectoryFlag(const char *path);

public:
	/**
	 * Creates a PS2FilesystemNode with the root node as path.
	 */
	Ps2FilesystemNode();
	
	/**
	 * Creates a PS2FilesystemNode for a given path.
	 * 
	 * @param path Common::String with the path the new node should point to.
	 */
	Ps2FilesystemNode(const Common::String &path);
	Ps2FilesystemNode(const Common::String &path, bool verify);
	
	/**
	 * Copy constructor.
	 */
	Ps2FilesystemNode(const Ps2FilesystemNode *node);

	virtual bool exists(void) const;

	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }

	virtual bool isDirectory() const {
		return _isDirectory;
	}

	virtual bool isReadable() const {
		return exists();
	}

	virtual bool isWritable() const {
		// The only writable device on the ps2 is the memory card
		return false;
	}

	virtual AbstractFSNode *clone() const { return new Ps2FilesystemNode(this); }
	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *openForReading();
	virtual Common::WriteStream *openForWriting();
};

Ps2FilesystemNode::Ps2FilesystemNode() {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "PlayStation 2";
	_path = "";
}

Ps2FilesystemNode::Ps2FilesystemNode(const Common::String &path) {
	_path = path;
	_isDirectory = true;
	if (strcmp(path.c_str(), "") == 0) {
		_isRoot = true;
		_displayName = Common::String("PlayStation 2");
	} else {
		_isRoot = false;
		const char *dsplName = NULL, *pos = path.c_str();
		while (*pos)
			if (*pos++ == '/')
				dsplName = pos;
		if (dsplName)
			_displayName = Common::String(dsplName);
		else
			_displayName = getDeviceDescription(path.c_str());
	}
}

Ps2FilesystemNode::Ps2FilesystemNode(const Common::String &path, bool verify) {
	_path = path;

	if (strcmp(path.c_str(), "") == 0) {
		_isRoot = true; /* root is always a dir*/
		_displayName = Common::String("PlayStation 2");
		_isDirectory = true;
	} else {
		_isRoot = false;
		const char *dsplName = NULL, *pos = path.c_str();
		while (*pos)
			if (*pos++ == '/')
				dsplName = pos;

		if (dsplName) {
			_displayName = Common::String(dsplName);
			if (verify)
				_isDirectory = getDirectoryFlag(path.c_str());
			else
				_isDirectory = false;
		} else {
			_displayName = getDeviceDescription(path.c_str());
			_isDirectory = true; /* devices are always dir */
		}
	}
}

Ps2FilesystemNode::Ps2FilesystemNode(const Ps2FilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
	_isRoot = node->_isRoot;
}

bool Ps2FilesystemNode::exists(void) const {
	
	dbg_printf("Ps2FilesystemNode::exists: path \"%s\": ", _path.c_str());

	if (_path[4] != ':') { // don't bother for relative path... they always fail on PS2!
		dbg_printf("NO, relative path\n");
		return false;
	}

	if (_path[0] == 'h') { // bypass host
		dbg_printf("NO, host device ignored\n");
		return false;
	}

	int fd = fio.open(_path.c_str(), O_RDONLY);
	if (fd == -EISDIR) {
		dbg_printf("YES, directory\n");
		return true;
	} else if (fd >= 0) {
		dbg_printf("YES, file\n");
		fio.close(fd);
		return true;
	}

	printf("NO, not found\n");
	return false;
}

bool Ps2FilesystemNode::getDirectoryFlag(const char *path) {
	if (strncmp(path, "host:", 5) == 0)
		return true;	// Can't get listings from host: right now

	int fd = fio.open(_path.c_str(), O_RDONLY);

	if (fd == -EISDIR) {
		dbg_printf(" romeo : new node [ %s ] is a dir\n", path);
		return true;
	} else if (fd >=0) {
		dbg_printf(" romeo : new node [ %s ] is -not- a dir (%d)\n", path, fd);
		fio.close(fd);
	} else
		dbg_printf(" romeo : new node [ %s ] is -not- (%d)\n", path, fd);

	return false;
}

AbstractFSNode *Ps2FilesystemNode::getChild(const Common::String &n) const {
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
		dirEntry._displayName = getDeviceDescription(dirEntry._path.c_str());
		list.push_back(new Ps2FilesystemNode(&dirEntry));

		if (g_systemPs2->hddPresent()) {
			dirEntry._path = "pfs0:";
			dirEntry._displayName = getDeviceDescription(dirEntry._path.c_str());
			list.push_back(new Ps2FilesystemNode(&dirEntry));
		}

		if (g_systemPs2->usbMassPresent()) {
			dirEntry._path = "mass:";
			dirEntry._displayName = getDeviceDescription(dirEntry._path.c_str());
			list.push_back(new Ps2FilesystemNode(&dirEntry));
		}
		return true;
	} else {
		char listDir[256];
		int fd;
		
		if (_path.lastChar() == '/' /* || _path.lastChar() == ':'*/)
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
				if (((mode == Common::FSNode::kListDirectoriesOnly) && (dirent.stat.mode & FIO_S_IFDIR)) ||
					((mode == Common::FSNode::kListFilesOnly) && !(dirent.stat.mode & FIO_S_IFDIR)) ||
					(mode == Common::FSNode::kListAll)) {

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

AbstractFSNode *Ps2FilesystemNode::getParent() const {
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
		return new Ps2FilesystemNode(Common::String(_path.c_str(), slash - _path.c_str()));
	else
		return new Ps2FilesystemNode();
}


char *Ps2FilesystemNode::getDeviceDescription(const char *path) const {
	if (strncmp(path, "cdfs", 4) == 0)
		return "DVD Drive";
	else if (strncmp(path, "mass", 4) == 0)
		return "USB Mass Storage";
	else
		return "Harddisk";	
}

Common::SeekableReadStream *Ps2FilesystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *Ps2FilesystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}
