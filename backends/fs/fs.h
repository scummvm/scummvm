/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * NOTE 1: currently ONLY the directory listing mode is used; that is, we only are
 * interested in listing directories. Thus, for now implementation only have to
 * list directories.
 *
 * NOTE 2: Backends still have to provide a way to extract a path from a FSIntern
 *
 * You may ask now: "isn't this cheating? Why do we go through all this when we use
 * a path in the end anyway?!?".
 * Well, for once as long as we don't provide our own file open/read/write API, we
 * still have to use fopen(). Since all our targets already support fopen(), it should
 * be possible to get a fopen() compatible string for any file system node.
 *
 * Secondly, with this abstraction layer, we still avoid a lot of complications based on
 * differences in FS roots, different path separators, or even systems with no real
 * paths (Mac OS 9, which doesn't even have the notion of a "current director").
 * And if we ever want to support devices with no FS in the classical sense (Palm...),
 * we can build upon this.
 */
 
/* 
 * TODO - Instead of starting with getRoot(), we should rather add a getDefaultDir()
 * call that on Unix might return the current dir or the users home dir...
 * i.e. the root dir is usually not the best starting point for browsing.
 */

#include "common/list.h"
#include "common/str.h"

class FSList;

/**
 * File system node.
 */
class FilesystemNode {
protected:
	typedef Common::String String;

public:

	/**
	 * Flag to tell listDir() which kind of files to list.
	 */
	typedef enum {
		kListFilesOnly = 1,
		kListDirectoriesOnly = 2,
		kListAll = 3
	} ListMode;

	/**
	 * Returns a special node representing the FS root. The starting point for
	 * any file system browsing.
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	static FilesystemNode *getRoot();

	/*
	 * Construct a node based on a path; the path is in the same format as it
	 * would be for calls to fopen().
	 *
	 * I.e. getNodeForPath(oldNode.path()) should create a new node identical to oldNode.
	 */
//	static FilesystemNode *getNodeForPath(const String &path);

	virtual ~FilesystemNode() {}

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
	virtual FSList *listDir(ListMode mode = kListDirectoriesOnly) const = 0;

	/**
	 * The parent node of this directory.
	 * The parent of the root is the root itself
	 */
	virtual FilesystemNode *parent() const = 0;

	/**
	 * Return a clone of this node allocated with new().
	 */
	virtual FilesystemNode *clone() const = 0;
	
	/**
	 * Compare the name of this node to the name of another.
	 */
	virtual bool operator< (const FilesystemNode& node) const
	{
		return displayName() < node.displayName();
	}
};


/**
 * Sorted list of multiple file system nodes. E.g. the contents of a given directory.
 */
class FSList : Common::List<FilesystemNode *> {
public:
	class ConstIterator {
		friend class FSList;
		FilesystemNode **_data;
		ConstIterator(FilesystemNode **data) : _data(data) { }
	public:
		const FilesystemNode &operator *() const { return **_data; }
        const FilesystemNode *operator->() const { return *_data; }
        bool operator !=(const ConstIterator &iter) const { return _data != iter._data; }
		void operator ++() { ++_data; }
	};

	~FSList() {
		for (int i = 0; i < _size; i++)
			delete _data[i];
	}

	void push_back(const FilesystemNode &element) {
		ensureCapacity(_size + 1);
		// Determine where to insert the item.
		// TODO this is inefficient, should use binary search instead
		int i = 0;
		while (i < _size && *_data[i] < element)
			i++;
		if (i < _size)
			memmove(&_data[i + 1], &_data[i], (_size - i) * sizeof(FilesystemNode *));
		_data[i] = element.clone();
		_size++;
	}
	
	const FilesystemNode& operator [](int idx) const {
		assert(idx >= 0 && idx < _size);
		return *_data[idx];
	}

	int size() const	{ return _size; }

	ConstIterator	begin() const {
		return ConstIterator(_data);
	}

	ConstIterator	end() const {
		return ConstIterator(_data + _size);
	}

};

#endif
