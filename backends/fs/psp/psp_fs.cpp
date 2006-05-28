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

#include <sys/stat.h>
#include <unistd.h>

#define	ROOT_PATH	"ms0:/"


/*
 * Implementation of the ScummVM file system API based on PSPSDK API.
 */

class PSPFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	
public:
	PSPFilesystemNode();
	PSPFilesystemNode(const Common::String &p, bool verify);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;
};

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return AbstractFilesystemNode::getRoot();
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new PSPFilesystemNode();
}

PSPFilesystemNode::PSPFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = ROOT_PATH;
}

PSPFilesystemNode::PSPFilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);
        
	_path = p;
	_displayName = _path;
	_isValid = true;
	_isDirectory = true;

	if (verify) {
		struct stat st; 
		_isValid = (0 == stat(_path.c_str(), &st));
		_isDirectory = S_ISDIR(st.st_mode);
	}       
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new PSPFilesystemNode(path, true);
}


bool PSPFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);

	int dfd  = sceIoDopen(_path.c_str());
	if (dfd > 0) {
		SceIoDirent dir;	   
		memset(&dir, 0, sizeof(dir));
	   
		while (sceIoDread(dfd, &dir) > 0) {
			// Skip 'invisible files
			if (dir.d_name[0] == '.') 
				continue;
               
			PSPFilesystemNode entry;
            
			entry._isValid = true;
			entry._displayName = dir.d_name;
			entry._path = _path;
			entry._path += dir.d_name;
			entry._isDirectory = dir.d_stat.st_attr & FIO_SO_IFDIR;
			
			if (entry._isDirectory)
				entry._path += "/";
            
			// Honor the chosen mode
			if ((mode == FilesystemNode::kListFilesOnly && entry._isDirectory) ||
			   (mode == FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
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
	assert(_isValid);
	
	if (_path == ROOT_PATH)
		return 0;
	
	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);
	
	PSPFilesystemNode *p = new PSPFilesystemNode(String(start, end - start), false);
	
	return p;
}

AbstractFilesystemNode *PSPFilesystemNode::child(const String &name) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += name;
	PSPFilesystemNode *p = new PSPFilesystemNode(newPath, true);

	return p;
}

#endif // PSP
