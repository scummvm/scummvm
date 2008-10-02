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

#ifndef COMMON_FS_H
#define COMMON_FS_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"

class AbstractFSNode;

namespace Common {

class FSNode;
class SeekableReadStream;
class WriteStream;

/**
 * List of multiple file system nodes. E.g. the contents of a given directory.
 * This is subclass instead of just a typedef so that we can use forward
 * declarations of it in other places.
 */
class FSList : public Common::Array<FSNode> {};

/**
 * FSNode, short for "File System Node", provides an abstraction for file
 * paths, allowing for portable file system browsing. This means for example,
 * that multiple or single roots have to be supported (compare Unix with a
 * single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it's left to them whether / or \ or : is the path separator :-);
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 */
class FSNode {
private:
	Common::SharedPtr<AbstractFSNode>	_realNode;
	FSNode(AbstractFSNode *realNode);

public:
	/**
	 * Flag to tell listDir() which kind of files to list.
	 */
	enum ListMode {
		kListFilesOnly = 1,
		kListDirectoriesOnly = 2,
		kListAll = 3
	};

	/**
	 * Create a new pathless FSNode. Since there's no path associated
	 * with this node, path-related operations (i.e. exists(), isDirectory(),
	 * getPath()) will always return false or raise an assertion.
	 */
	FSNode();

	/**
	 * Create a new FSNode referring to the specified path. This is
	 * the counterpart to the path() method.
	 *
	 * If path is empty or equals ".", then a node representing the "current
	 * directory" will be created. If that is not possible (since e.g. the
	 * operating system doesn't support the concept), some other directory is
	 * used (usually the root directory).
	 */
	explicit FSNode(const Common::String &path);

	virtual ~FSNode() {}

	/**
	 * Compare the name of this node to the name of another. Directories
	 * go before normal files.
	 */
	bool operator<(const FSNode& node) const;

	/**
	 * Indicates whether the object referred by this node exists in the filesystem or not.
	 *
	 * @return bool true if the node exists, false otherwise.
	 */
	virtual bool exists() const;

	/**
	 * Create a new node referring to a child node of the current node, which
	 * must be a directory node (otherwise an invalid node is returned).
	 * If a child matching the name exists, a normal node for it is returned.
	 * If no child with the name exists, a node for it is still returned,
	 * but exists() will return 'false' for it. This node can however be used
	 * to create a new file using the openForWriting() method.
	 *
	 * @todo If openForWriting() (or a hypothetical future mkdir() method) is used,
	 *       this should affect what exists/isDirectory/isReadable/isWritable return
	 *       for existing nodes. However, this is not the case for many existing
	 *       FSNode implementations. Either fix those, or document that FSNodes
	 *       can become 'stale'...
	 *
	 * @param name	the name of a child of this directory
	 * @return the node referring to the child with the given name
	 */
	FSNode getChild(const Common::String &name) const;

	/**
	 * Return a list of all child nodes of this directory node. If called on a node
	 * that does not represent a directory, false is returned.
	 *
	 * @return true if successful, false otherwise (e.g. when the directory does not exist).
	 */
	virtual bool getChildren(FSList &fslist, ListMode mode = kListDirectoriesOnly, bool hidden = false) const;

	/**
	 * Return a human readable string for this node, usable for display (e.g.
	 * in the GUI code). Do *not* rely on it being usable for anything else,
	 * like constructing paths!
	 *
	 * @return the display name
	 */
	virtual Common::String getDisplayName() const;

	/**
	 * Return a string representation of the name of the file. This is can be
	 * used e.g. by detection code that relies on matching the name of a given
	 * file. But it is *not* suitable for use with fopen / File::open, nor
	 * should it be archived.
	 *
	 * @return the file name
	 */
	virtual Common::String getName() const;

	/**
	 * Return a string representation of the file which is suitable for
	 * archiving (i.e. writing to the config file). This will usually be a
	 * 'path' (hence the name of the method), but can be anything that meets
	 * the above criterions. What a 'path' is differs greatly from system to
	 * system anyway.
	 *
	 * @note Do not assume that this string contains (back)slashes or any
	 *       other kind of 'path separators'.
	 *
	 * @return the 'path' represented by this filesystem node
	 */
	virtual Common::String getPath() const;

	/**
	 * Get the parent node of this node. If this node has no parent node,
	 * then it returns a duplicate of this node.
	 */
	FSNode getParent() const;

	/**
	 * Indicates whether the node refers to a directory or not.
	 *
	 * @todo Currently we assume that a node that is not a directory
	 * automatically is a file (ignoring things like symlinks or pipes).
	 * That might actually be OK... but we could still add an isFile method.
	 * Or even replace isDirectory by a getType() method that can return values like
	 * kDirNodeType, kFileNodeType, kInvalidNodeType.
	 */
	virtual bool isDirectory() const;

	/**
	 * Indicates whether the object referred by this node can be read from or not.
	 *
	 * If the node refers to a directory, readability implies being able to read
	 * and list the directory entries.
	 *
	 * If the node refers to a file, readability implies being able to read the
	 * contents of the file.
	 *
	 * @return true if the object can be read, false otherwise.
	 */
	virtual bool isReadable() const;

	/**
	 * Indicates whether the object referred by this node can be written to or not.
	 *
	 * If the node refers to a directory, writability implies being able to modify
	 * the directory entry (i.e. rename the directory, remove it or write files inside of it).
	 *
	 * If the node refers to a file, writability implies being able to write data
	 * to the file.
	 *
	 * @return true if the object can be written to, false otherwise.
	 */
	virtual bool isWritable() const;

	/**
	 * Searches recursively for files matching the specified pattern inside this directory and
	 * all its subdirectories. It is safe to call this method for non-directories, in this case
	 * it will just return false.
	 *
	 * The files in each directory are scanned first. Other than that, a depth first search
	 * is performed.
	 *
	 * @param results List to put the matches in.
	 * @param pattern Pattern of the files to look for.
	 * @param hidden Whether to search hidden files or not.
	 * @param exhaustive Whether to continue searching after one match has been found.
	 * @param depth How many levels to search through (-1 = search all subdirs, 0 = only the current one)
	 *
	 * @return true if matches could be found, false otherwise.
	 */
	virtual bool lookupFile(FSList &results, const Common::String &pattern, bool hidden, bool exhaustive, int depth = -1) const;
	
	
	/**
	 * Creates a SeekableReadStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * @return pointer to the stream object, 0 in case of a failure
	 */
	virtual Common::SeekableReadStream *openForReading() const;

	/**
	 * Creates a WriteStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * @return pointer to the stream object, 0 in case of a failure
	 */
	virtual Common::WriteStream *openForWriting() const;
};

} // End of namespace Common

#endif //COMMON_FS_H
