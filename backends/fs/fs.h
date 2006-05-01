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
 * $URL$
 * $Id$
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
 * This is subclass instead of just a typedef so that we can use forward
 * declarations of it in other places.
 */
class FSList : public Common::Array<FilesystemNode> {};


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
	 * The child node with the given name. If no child with this name
	 * exists, returns 0. Will never be called on a node which is not
	 * a directory node.
	 */
	virtual AbstractFilesystemNode *child(const String &name) const = 0;

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
	 * Return a human readable string for this node, usable for display (e.g.
	 * in the GUI code). Do *not* rely on it being usable for anything else,
	 * like constructing paths!
	 * @return the display name
	 */
	virtual String displayName() const = 0;

	/**
	 * Is this node valid? Returns true if the file/directory pointed
	 * to by this node exists, false otherwise.
	 *
	 * @todo Maybe rename this to exists() ? Or maybe even distinguish between
	 * the two? E.g. a path may be non-existant but valid, while another might
	 * be completely invalid). But do we ever need to make that distinction?
	 */
	virtual bool isValid() const = 0;

	/**
	 * Is this node pointing to a directory?
	 * @todo Currently we assume that a valid node that is not a directory
	 * automatically is a file (ignoring things like symlinks). That might
	 * actually be OK... but we could still add an isFile method. Or even replace
	 * isValid and isDirectory by a getType() method that can return values like
	 * kDirNodeType, kFileNodeType, kInvalidNodeType.
	 */
	virtual bool isDirectory() const = 0;

	/**
	 * Return a string representation of the file which can be passed to fopen(),
	 * and is suitable for archiving (i.e. writing to the config file).
	 * This will usually be a 'path' (hence the name of the method), but can
	 * be anything that fulfilly the above criterions.
	 */
	virtual String path() const = 0;

	/**
	 * Return a list of child nodes of this directory node. If called
	 * on a node that does not represent a directory, an error is triggered.
	 * @todo Rename this to listChildren.
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

	FilesystemNode(AbstractFilesystemNode *realNode);

	/**
	 * Returns a special node representing the FS root. The starting point for
	 * any file system browsing.
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	static AbstractFilesystemNode *getRoot();

	/**
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
	/**
	 * Create a new FilesystemNode refering to the specified path. This is
	 * the counterpart to the path() method.
	 */
	FilesystemNode(const String &path);


	FilesystemNode();
	FilesystemNode(const FilesystemNode &node);
	~FilesystemNode();

	FilesystemNode &operator  =(const FilesystemNode &node);

	/**
	 * Get the parent node of this node. If this node has no parent node,
	 * then it returns a duplicate of this node.
	 */
	FilesystemNode getParent() const;

	/**
	 * Fetch a child node of this node, with the given name. Only valid for
	 * directory nodes (an assertion is triggered otherwise). If no no child
	 * node with the given name exists, an invalid node is returned.
	 */
	FilesystemNode getChild(const String &name) const;

	virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const;
	virtual String displayName() const;
	virtual bool isValid() const;
	virtual bool isDirectory() const;
	virtual String path() const;

protected:
	void decRefCount();

	virtual AbstractFilesystemNode *parent() const { return 0; }
	virtual AbstractFilesystemNode *child(const String &name) const { return 0; }
};


#endif
