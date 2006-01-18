/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 */

#ifndef FS_H
#define FS_H

/*
 * The API described in this header is meant to allow for file system browsing in a
 * portable fashions. To this ends, multiple or single roots have to be supported
 * (compare Unix with a single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it's left to them whether / or \ or : is the path separator :-);
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 *
 * NOTE: Backends still have to provide a way to extract a path from a FSIntern
 *
 * You may ask now: "isn't this cheating? Why do we go through all this when we use
 * a path in the end anyway?!?".
 * Well, for once as long as we don't provide our own file open/read/write API, we
 * still have to use fopen(). Since all our targets already support fopen(), it should
 * be possible to get a fopen() compatible string for any file system node.
 *
 * Secondly, with this abstraction layer, we still avoid a lot of complications based on
 * differences in FS roots, different path separators, or even systems with no real
 * paths (MacOS 9 doesn't even have the notion of a "current directory").
 * And if we ever want to support devices with no FS in the classical sense (Palm...),
 * we can build upon this.
 */

/*
 * TODO - Instead of starting with getRoot(), we should rather add a getDefaultDir()
 * call that on Unix might return the current dir or the users home dir...
 * i.e. the root dir is usually not the best starting point for browsing.
 */

#include "common/array.h"
#include "common/str.h"

class FilesystemNode;


/**
 * List of multiple file system nodes. E.g. the contents of a given directory.
 */
class FSList : public Common::Array<FilesystemNode> {
public:
	void sort();
};


/**
 * File system node.
 */
class AbstractFilesystemNode {
protected:
	friend class FilesystemNode;
	typedef Common::String String;

	/**
	 * The parent node of this directory.
	 * The parent of the root is the root itself.
	 */
	virtual AbstractFilesystemNode *parent() const = 0;

	/**
	 * This method is a rather ugly hack which is used internally by the
	 * actual node implementions to wrap up raw nodes inside FilesystemNode
	 * objects. We probably want to get rid of this eventually and replace it
	 * with a cleaner / more elegant solution, but for now it works.
	 * @note This takes over ownership of node. Do not delete it yourself,
	 *       else you'll get ugly crashes. You've been warned!
	 */
	static FilesystemNode wrap(AbstractFilesystemNode *node);

public:

	/**
	 * Flag to tell listDir() which kind of files to list.
	 */
	typedef enum {
		kListFilesOnly = 1,
		kListDirectoriesOnly = 2,
		kListAll = 3
	} ListMode;

	virtual ~AbstractFilesystemNode() {}

	/**
	 * Return display name, used by e.g. the GUI to present the file in the file browser.
	 * @return the display name
	 */
	virtual String displayName() const = 0;

	/**
	 * Is this node valid (i.e. referring to an actual FS object)?
	 */
	virtual bool isValid() const = 0;

	/**
	 * Is this node a directory or not?
	 */
	virtual bool isDirectory() const = 0;

	/**
	 * A path representation suitable for use with fopen()
	 */
	virtual String path() const = 0;

	/**
	 * List the content of this directory node.
	 * If this node is not a directory, throw an exception or call error().
	 */
	virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const = 0;

	/**
	 * Compare the name of this node to the name of another. Directories
	 * go before normal files.
	 */
	virtual bool operator< (const AbstractFilesystemNode& node) const
	{
		if (isDirectory() && !node.isDirectory())
			return true;
		if (!isDirectory() && node.isDirectory())
			return false;
		return scumm_stricmp(displayName().c_str(), node.displayName().c_str()) < 0;
	}


	/* TODO:
	bool exists();

	bool isDirectory();
	bool isFile();

	bool isReadable();
	bool isWriteable();
	*/
};

class FilesystemNode : public AbstractFilesystemNode {
	friend class AbstractFilesystemNode;

	typedef Common::String String;
private:
	AbstractFilesystemNode *_realNode;
	int *_refCount;

	/**
	 * Returns a special node representing the FS root. The starting point for
	 * any file system browsing.
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	static AbstractFilesystemNode *getRoot();

	/*
	 * Construct a node based on a path; the path is in the same format as it
	 * would be for calls to fopen().
	 *
	 * I.e. getNodeForPath(oldNode.path()) should create a new node identical to oldNode.
	 *
	 * @TODO: This is of course a place where non-portable code easily will sneak
	 *        in, because the format of the path used here is not well-defined.
	 *        So we really should reconsider this API and try to come up with
	 *        something which is more portable but still flexible enough for our
	 *        purposes.
	 */
	static AbstractFilesystemNode *getNodeForPath(const String &path);


public:
	FilesystemNode();
	FilesystemNode(const FilesystemNode &node);
	FilesystemNode(const String &path);
	~FilesystemNode();

	FilesystemNode &operator  =(const FilesystemNode &node);

	FilesystemNode getParent() const;


	virtual String displayName() const { return _realNode->displayName(); }
	virtual bool isValid() const { return _realNode->isValid(); }
	virtual bool isDirectory() const { return _realNode->isDirectory(); }
	virtual String path() const { return _realNode->path(); }

	virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const { return _realNode->listDir(mode); }

protected:
	void decRefCount();

	virtual AbstractFilesystemNode *parent() const { return 0; }
};


#endif
