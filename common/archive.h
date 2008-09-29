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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_ARCHIVES_H
#define COMMON_ARCHIVES_H

#include "common/fs.h"
#include "common/str.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/ptr.h"
#include "common/singleton.h"
#include "common/stream.h"

namespace Common {

/**
 * FilePtr is a convenient way to keep track of a SeekableReadStream without
 * having to worry about releasing its memory.
 */
typedef SharedPtr<SeekableReadStream> FilePtr;

/**
 * Archive allows searches of (file)names into an arbitrary container.
 * It also supports opening a file and returning an usable input stream.
 */
class Archive {
public:
	virtual ~Archive() { }

	/**
	 * Check if a name is present in the Archive. Patterns are not allowed,
	 * as this is meant to be a quick File::exists() replacement.
	 */
	virtual bool hasFile(const String &name) = 0;

	/**
	 * Add all the names present in the Archive which match pattern to
	 * list. Returned names can be used as parameters to openFile.
	 * Must not remove elements from the list.
	 *
	 * @return the number of names added to list
	 */
	virtual int matchPattern(StringList &list, const String &pattern);

	/**
	 * Add all the names present in the Archive to list. Returned
	 * names can be used as parameters to openFile.
	 * Must not remove elements from the list.
	 *
	 * @return the number of names added to list
	 */
	virtual int getAllNames(StringList &list) = 0;

	/**
	 * Create a stream bound to a file in the archive.
	 * @return the newly created input stream
	 */
	virtual SeekableReadStream *openFile(const String &name) = 0;
};


typedef SharedPtr<Archive> ArchivePtr;


/**
 * FSDirectory models a directory tree from the filesystem and allows users
 * to access it through the Archive interface. FSDirectory can represent a
 * single directory, or a tree with specified depth, rooted in a 'base'
 * directory.
 * Searching is case-insensitive, as the main intended goal is supporting
 * retrieval of game data. First case-insensitive match is returned when
 * searching, thus making FSDirectory heavily dependant on the underlying
 * FilesystemNode implementation.
 */
class FSDirectory : public Archive {
	FilesystemNode	_node;

	// Caches are case insensitive, clashes are dealt with when creating
	// Key is stored in lowercase.
	typedef HashMap<String, FilesystemNode, IgnoreCase_Hash, IgnoreCase_EqualTo> NodeCache;
	NodeCache	_fileCache, _subDirCache;

	// look for a match
	FilesystemNode lookupCache(NodeCache &cache, const String &name);

	// cache management
	void cacheDirectoryRecursive(FilesystemNode node, int depth, const String& prefix);
	bool _cached;
	int	_depth;

public:
	/**
	 * Create a FSDirectory representing a tree with the specified depth. Will result in an
	 * unbound FSDirectory if name is not found on the filesystem or is not a directory.
	 */
	FSDirectory(const String &name, int depth = 1);

	/**
	 * Create a FSDirectory representing a tree with the specified depth. Will result in an
	 * unbound FSDirectory if node does not exist or is not a directory.
	 */
	FSDirectory(const FilesystemNode &node, int depth = 1);

	virtual ~FSDirectory();

	/**
	 * This return the underlying FSNode of the FSDirectory.
	 */
	FilesystemNode getFSNode() const;

	/**
	 * Create a new FSDirectory pointing to a sub directory of the instance.
	 * @return a new FSDirectory instance
	 */
	FSDirectory *getSubDirectory(const String &name);

	virtual bool hasFile(const String &name);
	virtual int matchPattern(StringList &list, const String &pattern);
	virtual int getAllNames(StringList &list);
	virtual SeekableReadStream *openFile(const String &name);
};


/**
 * SearchSet enables access to a group of Archives through the Archive interface.
 * Its intended usage is a situation in which there are no name clashes among names in the
 * contained Archives, hence the simplistic policy of always looking for the first
 * match. SearchSet *DOES* guarantee that searches are performed in *DESCENDING*
 * priority order. In case of conflicting priorities, insertion order prevails.
 */
class SearchSet : public Archive {
	struct Node {
		int			_priority;
		String		_name;
		ArchivePtr	_arc;
		Node(int priority, const String &name, ArchivePtr arc)
			: _priority(priority), _name(name), _arc(arc) {
		}
	};
	typedef List<Node> ArchiveList;
	ArchiveList _list;

	ArchiveList::iterator find(const String &name) const;

	// Add an archive keeping the list sorted by ascending priorities.
	void insert(const Node& node);

public:
	/**
	 * Add a new archive to the searchable set.
	 */
	void add(const String& name, ArchivePtr archive, int priority = 0);

	/**
	 * Remove an archive from the searchable set.
	 */
	void remove(const String& name);

	/**
	 * Check if a given archive name is already present.
	 */
	bool hasArchive(const String &name) const;

	/**
     * Empties the searchable set.
     */
	virtual void clear();

	/**
     * Change the order of searches.
     */
	void setPriority(const String& name, int priority);

	virtual bool hasFile(const String &name);
	virtual int matchPattern(StringList &list, const String &pattern);
	virtual int getAllNames(StringList &list);

	/**
	 * Implements openFile from Archive base class. The current policy is
	 * opening the first file encountered that matches the name.
	 */
	virtual SeekableReadStream *openFile(const String &name);
};


class SearchManager : public Singleton<SearchManager>, public SearchSet {
public:
	SearchManager();

	/**
	 * Add an existing Archive. This is meant to support searching in system-specific
	 * archives, namely the MACOSX/IPHONE bundles.
	 */
	void addArchive(const String &name, ArchivePtr archive, int priority = 0);

	/**
	 * Create and add a FSDirectory by name
	 */
	void addDirectory(const String &name, const String &directory, int priority = 0);

	/**
	 * Create and add a FSDirectory and its subdirectories by name
	 */
	void addDirectoryRecursive(const String &name, const String &directory, int depth = 4, int priority = 0);

	/**
	 * TODO
	 */
	virtual void clear();
};

/** Shortcut for accessing the search manager. */
#define SearchMan		Common::SearchManager::instance()

} // namespace Common

#endif
