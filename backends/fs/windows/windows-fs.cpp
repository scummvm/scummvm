/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// TODO - how about we #define WINDOWS for all our windows targets?
#if defined(_MSC_VER) || defined(__MINGW32__)

#include "../fs.h"


// TODO - this file is just a dummy place holder for the person who wants to implement
// the FS backend for windows. I don't even gurantee it compiles, but it should as a 
// starting place for the implementor at least.
// Look at posix-fs.cpp to see how it is done there, at fs.h for documentation of the
// API. I know that Exult as in files/listfiles.cc implementations for something similar
// for at least Unix, Windows, BeOS, MorphOS and MacOS, if you want to see how it is done.


/*
 * Implementation of the ScummVM file system API based on Windows API.
 */

class WindowsFilesystemNode : public FilesystemNode {
protected:
	// TODO - decide what members to offer. The following are used in the POSIX
	// implementations, but feel free to scrap all of them and use a complete
	// different inner working scheme
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	
public:
	WindowsFilesystemNode();
	WindowsFilesystemNode(const String &path);
	WindowsFilesystemNode(const WindowsFilesystemNode *node);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList *listDir() const;
	virtual FilesystemNode *parent() const;
	virtual FilesystemNode *clone() const { return new WindowsFilesystemNode(this); }
};


FilesystemNode *FilesystemNode::getRoot() {
	// TODO - return a node with the "root". I guess this will be a fake
	// dummy node in which you insert elements for all the drives (C:, D:, ...)
	return new WindowsFilesystemNode();
}

WindowsFilesystemNode::WindowsFilesystemNode() {
	// TODO - default constructor; in the POSIX implementations this just
	// produces the root node, but use as you like....
}

WindowsFilesystemNode::WindowsFilesystemNode(const WindowsFilesystemNode *node) {
	// TODO - copy constructor
}

FSList *WindowsFilesystemNode::listDir() const {
	assert(_isDirectory);

	struct dirent *dp;
	FSList *myList = new FSList();
	
	// TODO - fill myList here

	return myList;
}

FilesystemNode *WindowsFilesystemNode::parent() const {
	// TODO - return a node representing the parent node of this one.
	// For the root node, you may return 'this'.
	return 0;
}

#endif // defined(UNIX)
