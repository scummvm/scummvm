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

#include "common/str.h"

#include "backends/fs/fs.h"

/**
 * Abstract file system node. Private subclasses implement the actual
 * functionality.
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
	 * This method is a rather ugly hack which is used internally by the
	 * actual node implementions to wrap up raw nodes inside FilesystemNode
	 * objects. We probably want to get rid of this eventually and replace it
	 * with a cleaner / more elegant solution, but for now it works.
	 * @note This takes over ownership of node. Do not delete it yourself,
	 *       else you'll get ugly crashes. You've been warned!
	 */
	static FilesystemNode wrap(AbstractFilesystemNode *node);

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
	virtual FSList listDir(ListMode mode) const = 0;


	/* TODO:
	bool exists();

	bool isDirectory();
	bool isFile();

	bool isReadable();
	bool isWriteable();
	*/
};


#endif
