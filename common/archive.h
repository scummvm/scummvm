/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_ARCHIVE_H
#define COMMON_ARCHIVE_H

#include "common/str.h"
#include "common/list.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
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
	virtual U32String getDisplayName() const { return getName(); } /*!< Get the display name of the archive member. */
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
	virtual bool hasFile(const Path &path) const = 0;

	/**
	 * Add all members of the Archive matching the specified pattern to the list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @param matchPathComponents if set, then whole string will be matched, otherwise (default),
	 *                            path separator ('/') does not match with wildcards
	 *
	 * @return The number of members added to list.
	 */
	virtual int listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents = false) const;

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
	virtual const ArchiveMemberPtr getMember(const Path &path) const = 0;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 *
	 * @return The newly created input stream.
	 */
	virtual SeekableReadStream *createReadStreamForMember(const Path &path) const = 0;

	/**
	 * For most archives: same as previous. For SearchSet see SearchSet
	 * documentation.
	 */
	virtual SeekableReadStream *createReadStreamForMemberNext(const Path &path, const Archive *starting) const {
		return createReadStreamForMember(path);
	}

	/**
	 * Dump all files from the archive to the given directory
	 */
	void dumpArchive(String destPath);
};

class MemcachingCaseInsensitiveArchive;

// This is a shareable reference to a file contents stored in memory.
// It can be in 2 states: strong when it holds a strong reference in
// the sense of SharedPtr. Another state is weak when it only helds
// WeakPtr and thus may expire. Also strong reference is held by
// Returned memory stream. Hence once no memory streams and no
// strong referenceas are remaining, the block is freed.
class SharedArchiveContents {
public:
	SharedArchiveContents(byte *contents, uint32 contentSize) :
		_strongRef(contents, ArrayDeleter<byte>()), _weakRef(_strongRef),
		_contentSize(contentSize), _missingFile(false), _bypass(nullptr) {}
	SharedArchiveContents() : _strongRef(nullptr), _weakRef(nullptr), _contentSize(0), _missingFile(true), _bypass(nullptr) {}
	static SharedArchiveContents bypass(SeekableReadStream *stream) {
		return SharedArchiveContents(stream);
	}

private:
	SharedArchiveContents(SeekableReadStream *stream) : _strongRef(nullptr), _weakRef(nullptr), _contentSize(0), _missingFile(false), _bypass(stream) {}

	bool isFileMissing() const { return _missingFile; }
	SharedPtr<byte> getContents() const { return _strongRef; }
	uint32 getSize() const { return _contentSize; }

	bool makeStrong() {
		if (_strongRef || _contentSize == 0 || _missingFile)
			return true;
		_strongRef = SharedPtr<byte>(_weakRef);
		if (_strongRef)
			return true;
		return false;
	}

	void makeWeak() {
		// No need to make weak if we have no contents
		if (_contentSize == 0)
			return;
		_strongRef = nullptr;
	}

	SharedPtr<byte> _strongRef;
	WeakPtr<byte> _weakRef;
	uint32 _contentSize;
	bool _missingFile;
	SeekableReadStream *_bypass;

	friend class MemcachingCaseInsensitiveArchive;
};

/**
 * An archive that caches the resulting contents.
 */
class MemcachingCaseInsensitiveArchive : public Archive {
public:
	MemcachingCaseInsensitiveArchive(uint32 maxStronglyCachedSize = 512) : _maxStronglyCachedSize(maxStronglyCachedSize) {}
	SeekableReadStream *createReadStreamForMember(const Path &path) const;

	virtual String translatePath(const Path &path) const {
		// Most of users of this class implement DOS-like archives.
		// Others override this method.
		return normalizePath(path.toString('\\'), '\\');
	}

	virtual SharedArchiveContents readContentsForPath(const String& translatedPath) const = 0;

private:
	mutable HashMap<String, SharedArchiveContents, IgnoreCase_Hash, IgnoreCase_EqualTo> _cache;
	uint32 _maxStronglyCachedSize;
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
	 * The path separator for this case is SLASH for all systems.
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
	 * The path separator for this case is SLASH for all systems.
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
	 * Looks up an archive in the searchable set.
	 */
	Archive *getArchive(const String &name) const;

	/**
	 * Empty the searchable set.
	 */
	virtual void clear();

	/**
	 * Change the order of searches.
	 */
	void setPriority(const String& name, int priority);

	bool hasFile(const Path &path) const override;
	int listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents = false) const override;
	int listMembers(ArchiveMemberList &list) const override;

	const ArchiveMemberPtr getMember(const Path &path) const override;

	/**
	 * Implement createReadStreamForMember from the Archive base class. The current policy is
	 * opening the first file encountered that matches the name.
	 */
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;

	/**
	 * Similar to above but exclude matches from archives before starting and starting itself.
	 */
	SeekableReadStream *createReadStreamForMemberNext(const Path &path, const Archive *starting) const override;

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
