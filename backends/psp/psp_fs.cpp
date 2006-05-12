/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 * Copyright (C) 2005 Joost Peters PSP Backend
 * Copyright (C) 2005 Thomas Mayer PSP Backend
 * Copyright (C) 2005 Paolo Costabel PSP Backend
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

#ifdef __PSP__
#include "base/engine.h"

#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Implementation of the ScummVM file system API based on PSPSDK API.
 */

class PSPFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
	String _path;
	
public:
	PSPFilesystemNode();
	PSPFilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
};

AbstractFilesystemNode *FilesystemNode::getRoot() {
	return new PSPFilesystemNode();
}

PSPFilesystemNode::PSPFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = "ms0:/";
	_isPseudoRoot = true;
}

PSPFilesystemNode::PSPFilesystemNode(const Common::String &p)
{
	_displayName = p;
	_isValid = true;
	_isDirectory = true;
	_path = p;
}


AbstractFilesystemNode *FilesystemNode::getNodeForPath(const String &path) 
{
	return new PSPFilesystemNode(path);
}


bool PSPFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);

	int dfd;
	
    dfd = sceIoDopen(_path.c_str());
	if (dfd > 0) {
        SceIoDirent dir;	   
        memset(&dir, 0, sizeof(dir));
	   
        while (sceIoDread(dfd, &dir) > 0) {
            // Skip 'invisible files
            if (dir.d_name[0] == '.') 
                continue;
               
            PSPFilesystemNode entry;
            
            entry._isValid = true;
            entry._isPseudoRoot = false;
            entry._displayName = dir.d_name;
            entry._path = _path;
            entry._path += dir.d_name;
            entry._isDirectory = dir.d_stat.st_attr & FIO_SO_IFDIR;
            if (entry._isDirectory)
                entry._path += "/";
            
            // Honor the chosen mode
            if ((mode == kListFilesOnly && entry._isDirectory) ||
                (mode == kListDirectoriesOnly && !entry._isDirectory))
                    continue;
            
            myList.push_back(new PSPFilesystemNode(entry));
        }
        sceIoDclose(dfd);
        return true;
	} else {
		return false;
	}
}

const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '/') {
		--cur;
	}

	return cur + 1;
}

AbstractFilesystemNode *PSPFilesystemNode::parent() const {
	assert(_isValid || _isPseudoRoot);
	if (_isPseudoRoot)
		return 0;
	PSPFilesystemNode *p = new PSPFilesystemNode();
	if (_path.size() > 5) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p->_path = String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
		p->_isPseudoRoot = false;
	}
	return p;
}

#endif // PSP
