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
 * ArchiveMember is an abstract interface to represent elements inside
 * implementations of Archive.
 *
 * Archive subclasses must provide their own implementation of ArchiveMember,
 * and use it when serving calls to listMembers() and listMatchingMembers().
 * Alternatively, the GenericArchiveMember below can be used.
 */
class ArchiveMember {
public:
	virtual ~ArchiveMember() { }
	virtual String getName() const = 0;
	virtual SeekableReadStream *open() = 0;
};

typedef SharedPtr<ArchiveMember> ArchiveMemberPtr;
typedef List<ArchiveMemberPtr> ArchiveMemberList;

class Archive;

/**
 * Simple ArchiveMember implementation which allows
 * creation of ArchiveMember compatible objects via
 * a simple Archive and name pair.
 *
 * Note that GenericArchiveMember objects will not
 * be working anymore after the 'parent' object
 * is destroyed.
 */
class GenericArchiveMember : public ArchiveMember {
	Archive *_parent;
	String _name;
public:
	GenericArchiveMember(String name, Archive *parent);
	String getName() const;
	SeekableReadStream *open();
};


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
	virtual int listMatchingMembers(ArchiveMemberList &list, const String &pattern);

	/**
	 * Add all the names present in the Archive to list. Returned
	 * names can be used as parameters to openFile.
	 * Must not remove elements from the list.
	 *
	 * @return the number of names added to list
	 */
	virtual int listMembers(ArchiveMemberList &list) = 0;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	virtual ArchiveMemberPtr getMember(const String &name) = 0;

	/**
	 * Create a stream bound to a file in the archive.
	 * @return the newly created input stream
	 */
	virtual SeekableReadStream *openFile(const String &name) = 0;
};


/**
 * FSDirectory models a directory tree from the filesystem and allows users
 * to access it through the Archive interface. Searching is case-insensitive,
 * since the intended goal is supporting retrieval of game data.
 *
 * FSDirectory can represent a single directory, or a tree with specified depth,
 * depending on the value passed to the 'depth' parameter in the constructors.
 * Filenames are cached with their relative path, with elements separated by
 * backslashes, e.g.:
 *
 * c:\my\data\file.ext
 *
 * would be cached as 'data/file.ext' if FSDirectory was created on 'c:/my' with
 * depth > 1. If depth was 1, then the 'data' subdirectory would have been
 * ignored, instead.
 * Again, only BACKSLASHES are used as separators independently from the
 * underlying file system.
 *
 * Relative paths can be specified when calling matching functions like openFile(),
 * hasFile(), listMatchingMembers() and listMembers(). Please see the function
 * specific comments for more information.
 *
 * Client code can customize cache by using the constructors with the 'prefix'
 * parameter. In this case, the prefix is prepended to each entry in the cache,
 * and effectively treated as a 'virtual' parent subdirectory. FSDirectory adds
 * a trailing backslash to prefix if needed. Following on with the previous example
 * and using 'your' as prefix, the cache entry would have been 'your/data/file.ext'.
 *
 */
class FSDirectory : public Archive {
	FSNode	_node;

	// Caches are case insensitive, clashes are dealt with when creating
	// Key is stored in lowercase.
	typedef HashMap<String, FSNode, IgnoreCase_Hash, IgnoreCase_EqualTo> NodeCache;
	NodeCache	_fileCache, _subDirCache;
	Common::String	_prefix;	// string that is prepended to each cache item key
	void setPrefix(const String &prefix);

	// look for a match
	FSNode lookupCache(NodeCache &cache, const String &name);

	// cache management
	void cacheDirectoryRecursive(FSNode node, int depth, const String& prefix);
	bool _cached;
	int	_depth;

public:
	/**
	 * Create a FSDirectory representing a tree with the specified depth. Will result in an
	 * unbound FSDirectory if name is not found on the filesystem or if the node is not a
	 * valid directory.
	 */
	FSDirectory(const String &name, int depth = 1);
	FSDirectory(const FSNode &node, int depth = 1);

	/**
	 * Create a FSDirectory representing a tree with the specified depth. The parameter
	 * prefix is prepended to the keys in the cache. See class comment.
	 */
	FSDirectory(const String &prefix, const String &name, int depth = 1);
	FSDirectory(const String &prefix, const FSNode &node, int depth = 1);

	virtual ~FSDirectory();

	/**
	 * This return the underlying FSNode of the FSDirectory.
	 */
	FSNode getFSNode() const;

	/**
	 * Create a new FSDirectory pointing to a sub directory of the instance. See class comment
	 * for an explanation of the prefix parameter.
	 * @return a new FSDirectory instance
	 */
	FSDirectory *getSubDirectory(const String &name, int depth = 1);
	FSDirectory *getSubDirectory(const String &prefix, const String &name, int depth = 1);

	/**
	 * Checks for existence in the cache. A full match of relative path and filename is needed
	 * for success.
	 */
	virtual bool hasFile(const String &name);

	/**
	 * Returns a list of matching file names. Pattern can use GLOB wildcards.
	 */
	virtual int listMatchingMembers(ArchiveMemberList &list, const String &pattern);

	/**
	 * Returns a list of all the files in the cache.
	 */
	virtual int listMembers(ArchiveMemberList &list);

	/**
	 * Get a ArchiveMember representation of the specified file. A full match of relative
	 * path and filename is needed for success.
	 */
	virtual ArchiveMemberPtr getMember(const String &name);

	/**
	 * Open the specified file. A full match of relative path and filename is needed
	 * for success.
	 */
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
		int		_priority;
		String	_name;
		Archive	*_arc;
		bool	_autoFree;
		Node(int priority, const String &name, Archive *arc, bool autoFree)
			: _priority(priority), _name(name), _arc(arc), _autoFree(autoFree) {
		}
	};
	typedef List<Node> ArchiveList;
	ArchiveList _list;

	ArchiveList::iterator find(const String &name) const;

	// Add an archive keeping the list sorted by ascending priorities.
	void insert(const Node& node);

public:
	virtual ~SearchSet() { clear(); }

	/**
	 * Add a new archive to the searchable set.
	 */
	void add(const String& name, Archive *arch, int priority = 0, bool autoFree = true);

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
	virtual int listMatchingMembers(ArchiveMemberList &list, const String &pattern);
	virtual int listMembers(ArchiveMemberList &list);

	virtual ArchiveMemberPtr getMember(const String &name);

	/**
	 * Implements openFile from Archive base class. The current policy is
	 * opening the first file encountered that matches the name.
	 */
	virtual SeekableReadStream *openFile(const String &name);
};


class SearchManager : public Singleton<SearchManager>, public SearchSet {
public:

	/**
	 * Create and add a FSDirectory by name
	 */
	void addDirectory(const String &name, const String &directory, int priority = 0, int depth = 1);

	/**
	 * Create and add a FSDirectory by FSNode
	 */
	void addDirectory(const String &name, const FSNode &directory, int priority = 0, int depth = 1);

	/**
	 * Resets the search manager to the default list of search paths (system
	 * specific dirs + current dir).
	 */
	virtual void clear();

private:
	friend class Common::Singleton<SingletonBaseType>;
	SearchManager();
};

/** Shortcut for accessing the search manager. */
#define SearchMan		Common::SearchManager::instance()

} // namespace Common

#endif
