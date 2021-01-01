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
 */

#ifndef COMMON_ARCHIVE_H
#define COMMON_ARCHIVE_H

#include "common/str.h"
#include "common/list.h"
#include "common/ptr.h"
#include "common/singleton.h"

namespace Common {

/**
 * @defgroup common_arch Archive
 * @ingroup common
 *
 * @brief  The Archive module allows for managing the members of arbitrary containers in a uniform
 * fashion. 
 * It also supports looking up by names and file names, opening a file, and returning a usable input stream.
 * @{
 */

class FSNode;
class SeekableReadStream;


/**
 * The ArchiveMember class is an abstract interface to represent elements inside
 * implementations of an archive.
 *
 * Archive subclasses must provide their own implementation of ArchiveMember,
 * and use it when serving calls to @ref Archive::listMembers and @ref Archive::listMatchingMembers.
 * Alternatively, you can use the @ref GenericArchiveMember.
 */
class ArchiveMember {
public:
	virtual ~ArchiveMember() { }
	virtual SeekableReadStream *createReadStream() const = 0; /*!< Create a read stream. */
	virtual String getName() const = 0; /*!< Get the name of the archive member. */
	virtual String getDisplayName() const { return getName(); } /*!< Get the display name of the archive member. */
};

typedef SharedPtr<ArchiveMember> ArchiveMemberPtr; /*!< Shared pointer to an archive member. */
typedef List<ArchiveMemberPtr> ArchiveMemberList; /*!< List of archive members. */

/**
 * Compare two archive member operators @p a and @p b and return which of them is higher.
 */
struct ArchiveMemberListComparator {
	bool operator()(const ArchiveMemberPtr &a, const ArchiveMemberPtr &b) {
		return a->getName() < b->getName();
	}
};

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
	const Archive *_parent;
	const String _name;
public:
	GenericArchiveMember(const String &name, const Archive *parent); /*!< Create a generic archive member that belongs to the @p parent archive. */
	String getName() const; /*!< Get the name of a generic archive member. */
	SeekableReadStream *createReadStream() const; /*!< Create a read stream. */
};


/**
 * The Archive class allows for managing the members of arbitrary containers in a uniform
 * fashion, allowing lookup by (file) names.
 * It also supports opening a file and returning a usable input stream.
 */
class Archive {
public:
	virtual ~Archive() { }

	/**
	 * Check if a member with the given @p name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	virtual bool hasFile(const String &name) const = 0;

	/**
	 * Add all members of the Archive matching the specified pattern to the list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return The number of members added to list.
	 */
	virtual int listMatchingMembers(ArchiveMemberList &list, const String &pattern) const;

	/**
	 * Add all members of the Archive to the list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return The number of names added to list.
	 */
	virtual int listMembers(ArchiveMemberList &list) const = 0;

	/**
	 * Return an ArchiveMember representation of the given file.
	 */
	virtual const ArchiveMemberPtr getMember(const String &name) const = 0;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 *
	 * @return The newly created input stream.
	 */
	virtual SeekableReadStream *createReadStreamForMember(const String &name) const = 0;
};


/**
 * The SearchSet class enables access to a group of Archives through the Archive interface.
 *
 * Its intended usage is a situation in which there are no name clashes among names in the
 * contained Archives, hence the simplistic policy of always looking for the first
 * match. SearchSet does guarantee that searches are performed in DESCENDING
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
	typedef List<Node> ArchiveNodeList;
	ArchiveNodeList _list;

	ArchiveNodeList::iterator find(const String &name);
	ArchiveNodeList::const_iterator find(const String &name) const;

	void insert(const Node& node); //!< Add an archive while keeping the list sorted by descending priority.

	bool _ignoreClashes;

public:
	SearchSet() : _ignoreClashes(false) { }
	virtual ~SearchSet() { clear(); }

	/**
	 * Add a new archive to the searchable set.
	 */
	void add(const String& name, Archive *arch, int priority = 0, bool autoFree = true);

	/**
	 * Create and add an FSDirectory by name.
	 */
	void addDirectory(const String &name, const String &directory, int priority = 0, int depth = 1, bool flat = false);

	/**
	 * Create and add an FSDirectory by FSNode.
	 */
	void addDirectory(const String &name, const FSNode &directory, int priority = 0, int depth = 1, bool flat = false);

	/**
	 * Create and add a subdirectory by name (caseless).
	 *
	 * It is also possible to add subdirectories of subdirectories (of any depth) with this function.
	 * The path seperator for this case is SLASH for all systems.
	 *
	 * Example:
	 *
	 *   "game/itedata"
	 *
	 * In this example, the code first tries to search for all directories matching
	 * "game" (case insensitive) in the path "directory" first and search through all
	 * of the matches for "itedata" (case insensitive too).
	 *
	 * Note that it will add all matches found!
	 *
	 * Even though this method is currently implemented via addSubDirectoriesMatching, it is not safe
	 * to assume that this method is using anything other than a simple case insensitive compare.
	 * Thus, do not use any tokens like '*' or '?' in the "caselessName" parameter of this function.
	 */
	void addSubDirectoryMatching(const FSNode &directory, const String &caselessName, int priority = 0, int depth = 1, bool flat = false) {
		addSubDirectoriesMatching(directory, caselessName, true, priority, depth, flat);
	}

	/**
	 * Create and add subdirectories by pattern.
	 *
	 * It is also possible to add subdirectories of subdirectories (of any depth) with this function.
	 * The path seperator for this case is SLASH for all systems.
	 *
	 * Example:
	 *
	 *   "game/itedata"
	 *
	 * In this example, the code first tries to search for all directories matching
	 * "game" in the path "directory" first and search through all of the matches for
	 * "itedata". If "ingoreCase" is set to true, the code does a case insensitive
	 * match, otherwise it is doing a case sensitive match.
	 *
	 * This method also works with tokens. For a list of available tokens,
	 * see @ref Common::matchString.
	 */
	void addSubDirectoriesMatching(const FSNode &directory, String origPattern, bool ignoreCase, int priority = 0, int depth = 1, bool flat = false);

	/**
	 * Remove an archive from the searchable set.
	 */
	void remove(const String& name);

	/**
	 * Check if a given archive name is already present.
	 */
	bool hasArchive(const String &name) const;

	/**
	 * Empty the searchable set.
	 */
	virtual void clear();

	/**
	 * Change the order of searches.
	 */
	void setPriority(const String& name, int priority);

	virtual bool hasFile(const String &name) const;
	virtual int listMatchingMembers(ArchiveMemberList &list, const String &pattern) const;
	virtual int listMembers(ArchiveMemberList &list) const;

	virtual const ArchiveMemberPtr getMember(const String &name) const;

	/**
	 * Implement createReadStreamForMember from the Archive base class. The current policy is
	 * opening the first file encountered that matches the name.
	 */
	virtual SeekableReadStream *createReadStreamForMember(const String &name) const;

	/**
	 * Ignore clashes when adding directories. For more details, see the corresponding parameter
	 * in @ref FSDirectory documentation.
	 */
	void setIgnoreClashes(bool ignoreClashes) { _ignoreClashes = ignoreClashes; }
};


class SearchManager : public Singleton<SearchManager>, public SearchSet {
public:

	/**
	 * Reset the Search Manager to the default list of search paths (system
	 * specific dirs + current dir).
	 */
	virtual void clear();

private:
	friend class Singleton<SingletonBaseType>;
	SearchManager();
};

/** Shortcut for accessing the Search Manager. */
#define SearchMan		Common::SearchManager::instance()

/** @} */

} // namespace Common

#endif
