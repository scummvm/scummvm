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
#include <unistd.h>
#include "asyncfio.h"
#include "fileio.h"
#include "systemps2.h"
#include "ps2debug.h"

#define DEFAULT_MODE (FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IROTH | FIO_S_IWOTH)

FILE *ps2_fopen(const char *fname, const char *mode);
int ps2_fclose(FILE *stream);
// extern int fileXioChdir(const char* pathname);

#include <fileXio_rpc.h>


extern AsyncFio fio;
extern OSystem_PS2 *g_systemPs2;

/**
 * Implementation of the ScummVM file system API based on the Ps2SDK.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class Ps2FilesystemNode : public AbstractFilesystemNode {

friend class Ps2FilesystemFactory;

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
	Ps2FilesystemNode(const String &path, bool verify);
	
	/**
	 * Copy constructor.
	 */
	Ps2FilesystemNode(const Ps2FilesystemNode *node);
	
	virtual bool exists() const {
		int fd;

		dbg_printf("Q: Does %s exist ?\n", _path.c_str());

		if (_path[4] != ':') { // don't bother for relative path...
		                       //  ...they always fail on PS2!
			dbg_printf("A: nope -> skip relative path\n");
			return false;
		}

		if (_path[0] == 'h') { // bypass host
			dbg_printf("A: nope -> skip host\n");
			return false;
		}

		if ((fd = fio.open(_path.c_str(), O_RDONLY)) >= 0) { /* it's a file */
			fio.close(fd);
			dbg_printf("A: yep -> file\n");
			return true;
		}

		   /* romeo's black magic */
		fd = fio.open(_path.c_str(), O_RDONLY, DEFAULT_MODE | FIO_S_IFDIR); 
		if (fd == -EISDIR) {
			dbg_printf("A: yep -> dir\n");
			return true;
		}
		/* NOTE: it cannot be a file cause it would have been caught by
		         the previous test, so no need to close it ;-)          */

		dbg_printf("A: nope\n");
		return false;
	}

	virtual String getDisplayName() const { return _displayName; }
	virtual String getName() const { return _displayName; }
	virtual String getPath() const { return _path; }
	virtual bool isDirectory() const {

		dbg_printf("Q: Is %s a dir?\n", _path.c_str());

#if 0
		int fd;

		if (_path.empty()) {
			printf("A: yep -> top level\n");
			return true; // top level
		}

		if (_path.lastChar() == ':' ||
			(_path.lastChar() == '/' && _path[_path.size()-2] == ':')) {
			printf("A: yep -> device\n");
			return true; // device
		}

		fd = fio.open(_path.c_str(), O_RDONLY, DEFAULT_MODE | FIO_S_IFDIR);
		if (fd == -EISDIR) {
			printf("A: yep\n");
			return true;
		}

		if (fd >= 0)
			fio.close(fd);

		printf("A: nope\n");
		return false;
#else
		if (_isDirectory)
			dbg_printf("A: yep -> _isDirectory == 1\n");
		else
			dbg_printf("A: nope -> _isDirectory == 0\n");

		return _isDirectory;
#endif
	}

	virtual bool isReadable() const {
		int fd;
		if ((fd = fio.open(_path.c_str(), O_RDONLY)) >= 0) {
			fio.close(fd);
			return true;
		}
		return false;
	}

	virtual bool isWritable() const {
		int fd;
		if ((fd =  fio.open(_path.c_str(), O_WRONLY)) >= 0) {
			fio.close(fd);
			return true;
		}
		return false;
	}

	virtual AbstractFilesystemNode *clone() const { return new Ps2FilesystemNode(this); }
	virtual AbstractFilesystemNode *getChild(const String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFilesystemNode *getParent() const;
};

/**
 * Returns the last component of a given path.
 * 
 * @param str String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
const char *lastPathComponent(const Common::String &str) {
	if (str.empty())
		return "";

	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur >= start && *cur != '/' && *cur != ':') {
		--cur;
	}

	dbg_printf("romeo : lastPathComponent = %s\n", cur + 1);

	return cur + 1;
}

Ps2FilesystemNode::Ps2FilesystemNode() {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "PlayStation 2";
	_path = "";
}

Ps2FilesystemNode::Ps2FilesystemNode(const String &path) {

	Ps2FilesystemNode(path, true);
/*
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
*/
}

Ps2FilesystemNode::Ps2FilesystemNode(const String &path, bool verify) {
	_path = path;
	_isDirectory = true;

	if (strcmp(path.c_str(), "") == 0) {
		_isRoot = true;
		_displayName = String("PlayStation 2");
		verify = false; /* root is always a dir*/
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
			verify = false; /* devices are always dir */
		}
	}

	if (verify && !_isRoot) {
		int medium, fd;

		if (strncmp(path.c_str(), "pfs0", 4) == 0)
			medium = 0;
		else if (strncmp(path.c_str(), "cdfs", 4) == 0)
			medium = 1;
		else if (strncmp(path.c_str(), "mass", 4) == 0)
			medium = 2;

		switch(medium) {

		case 0: /* hd */
		dbg_printf("hd >  ");
		fd = fio.open(_path.c_str(), O_RDONLY, DEFAULT_MODE | FIO_S_IFDIR);

		if (fd == -EISDIR) {
			dbg_printf(" romeo : new node [ %s ] is a dir\n", path.c_str());
		}
		else if (fd >=0) {
			_isDirectory = false;
			dbg_printf(" romeo : new node [ %s ] is -not- a dir (%d)\n",
				path.c_str(), fd);
			fio.close(fd);
		}
		else
			dbg_printf(" romeo : new node [ %s ] is -not- (%d)\n",
				 path.c_str(), fd);

		break;

		case 1: /* cd */
		dbg_printf("cd >  ");
		fd = fio.open(_path.c_str(), O_RDONLY, DEFAULT_MODE | FIO_S_IFDIR);

		if (fd == -ENOENT) {
			dbg_printf(" romeo : new node [ %s ] is a dir\n", path.c_str());
		}
		else if (fd >=0) {
			_isDirectory = false;
			dbg_printf(" romeo : new node [ %s ] is -not- a dir (%d)\n",
				path.c_str(), fd);
			fio.close(fd);
		}
		else
			dbg_printf(" romeo : new node [ %s ] is -not- (%d)\n",
				path.c_str(), fd);

		break;

		case 2: /* usb */
		dbg_printf("usb >  ");
		// fd = fio.open(_path.c_str(), O_RDONLY, DEFAULT_MODE | FIO_S_IFDIR);
#if 1
		fd = fio.open(_path.c_str(), O_RDONLY);
		if (fd == -EISDIR) {
			dbg_printf(" romeo : new node [ %s ] is a dir\n", path.c_str());
		}
		else if (fd > 0)  {
			_isDirectory = false;
			dbg_printf(" romeo : new node [ %s ] is a -not- a dir (%d)\n",
				path.c_str(), fd);
			fio.close(fd);
		}
		else {
			dbg_printf(" romeo : new node [ %s ] is a -not-\n",	path.c_str());
			_isDirectory = false;
		}

#else

		fd = fio.chdir(path.c_str());

		dbg_printf(" %d ", fd);

		if (fd == -48) {
			dbg_printf(" romeo : new node [ %s ] is a dir\n", path.c_str());
			// fio.close(fd);
			// chdir("");
		}
		else {
			_isDirectory = false;
			dbg_printf(" romeo : new node [ %s ] is a -not- a dir (%d)\n",
				path.c_str(), fd);
		}
#endif

		break;
		} /* switch(medium) */

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
