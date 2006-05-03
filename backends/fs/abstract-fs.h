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

#ifndef BACKENDS_ABSTRACT_FS_H
#define BACKENDS_ABSTRACT_FS_H

#include "common/array.h"
#include "common/str.h"

#include "backends/fs/fs.h"

class AbstractFilesystemNode;

typedef Common::Array<AbstractFilesystemNode *>	AbstractFSList;

/**
 * Abstract file system node. Private subclasses implement the actual
 * functionality.
 *
 * Most of the methods correspond directly to methods in class FilesystemNode,
 * so if they are not documented here, look there for more information about
 * the semantics.
 */
class AbstractFilesystemNode {
protected:
	friend class FilesystemNode;
	typedef Common::String String;
	typedef FilesystemNode::ListMode ListMode;

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
	virtual ~AbstractFilesystemNode() {}

	virtual String displayName() const = 0;
	virtual bool isDirectory() const = 0;
	virtual String path() const = 0;
	virtual bool listDir(AbstractFSList &list, ListMode mode) const = 0;


	/* TODO:
	bool exists();

	bool isDirectory();
	bool isFile();

	bool isReadable();
	bool isWriteable();
	*/
};


#endif
