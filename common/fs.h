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

#ifndef COMMON_FS_H
#define COMMON_FS_H

#include "common/array.h"
#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/ustr.h"

class AbstractFSNode;

namespace Common {

/**
 * @defgroup common_fs File system
 * @ingroup common
 *
 * @brief API for operations on the file system.
 *
 * @{
 */

class FSNode;
class FSDirectory;
class SeekableReadStream;
class WriteStream;
class SeekableWriteStream;

/**
 * List of multiple file system nodes. For example, the contents of a given directory.
 * This is a subclass instead of just a typedef so that forward declarations
 * of it can be used in other places.
 */
class FSList : public Array<FSNode> {};

/**
 * FSNode, short for "File System Node", provides an abstraction for file
 * paths, allowing for portable file system browsing. This means, for example,
 * that multiple or single roots have to be supported (compare Unix with a
 * single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it is left to them whether '/', '\', or ':' is the path separator)
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 */
class FSNode : public ArchiveMember {
private:
	friend class ::AbstractFSNode;
	friend class FSDirectory;
	SharedPtr<AbstractFSNode>	_realNode;
	/**
	 * Construct an FSNode from a backend's AbstractFSNode implementation.
	 *
	 * @param realNode Pointer to a heap allocated instance. FSNode will take
	 *                 ownership of the pointer.
	 */
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
	 * Create a new pathless FSNode. Since there is no path associated
	 * with this node, path-related operations (i.e. exists(), isDirectory(),
	 * getPath()) will always return false or raise an assertion.
	 */
	FSNode();

	/**
	 * Create a new FSNode referring to the specified path. This is
	 * the counterpart to the path() method.
	 *
	 * If the path is empty or equals ".", then a node representing the "current
	 * directory" will be created. If that is not possible (since e.g. the
	 * operating system does not support the concept), some other directory is
	 * used (usually the root directory).
	 */
	explicit FSNode(const Path &path);
	~FSNode();

	/**
	 * Compare the name of this node to the name of another. Directories
	 * go before normal files.
	 */
	bool operator<(const FSNode& node) const;

	/**
	 * Indicate whether the object referred by this node exists in the file system or not.
	 *
	 * @return True if the node exists, false otherwise.
	 */
	bool exists() const;

	/**
	 * Create a new node referring to a child node of the current node, which
	 * must be a directory node (otherwise an invalid node is returned).
	 * If a child matching the name exists, a normal node for it is returned.
	 * If no child with the name exists, a node for it is still returned,
	 * but exists() will return 'false' for it. This node can however be used
	 * to create a new file using the createWriteStream() method.
	 *
	 * @todo If createWriteStream() (or a hypothetical future mkdir() method) is used,
	 *       this should affect what exists/isDirectory/isReadable/isWritable return
	 *       for existing nodes. However, this is not the case for many existing
	 *       FSNode implementations. Either fix those, or document that FSNodes
	 *       can become 'stale'.
	 *
	 * @param name	Name of a child of this directory.
	 * @return The node referring to the child with the given name.
	 */
	FSNode getChild(const String &name) const;

	/**
	 * Return a list of all child nodes of this directory node. If called on a node
	 * that does not represent a directory, false is returned.
	 *
	 * @return True if successful, false otherwise (e.g. when the directory does not exist).
	 */
	bool getChildren(FSList &fslist, ListMode mode = kListDirectoriesOnly, bool hidden = true) const;

	/**
	 * Return a human-readable string for this node, usable for display (e.g.
	 * in the GUI code). Do *not* rely on it being usable for anything else,
	 * like constructing paths.
	 *
	 * @return The display name.
	 */
	U32String getDisplayName() const override;

	/**
	 * Return a string representation of the name of the file. This can be
	 * used e.g. by detection code that relies on matching the name of a given
	 * file. However, it is *not* suitable for use with fopen / File::open, nor
	 * should it be archived.
	 *
	 * @return The file name.
	 */
	String getName() const override;

	/**
	 * Return a string representation of the name of the file.
	 *
	 * @return The file name.
	 */
	String getFileName() const override;

	/**
	 * Return a string representation of the name of the file.  Since FSNode
	 * parents are always the parent FSDirectory, this will not return a full
	 * path, only the file name.
	 *
	 * @return The file name.
	 */
	Path getPathInArchive() const override;

	/**
	 * Return a string representation of the name of the file, without any
	 * Punycode transformation. This can be used e.g. by detection code that
	 * relies on matching the name of a given file. However, it is *not*
	 * suitable for use with fopen / File::open, nor should it be archived.
	 *
	 * @return The file name.
	 */
	virtual String getRealName() const;

	/**
	 * Return a path representation of the file that is suitable for
	 * archiving (i.e. writing to the config file).
	 * What a 'path' is differs greatly from system to
	 * system.
	 *
	 * @return The 'path' represented by this file system node.
	 */
	Path getPath() const;

	/**
	 * Get the parent node of this node. If this node has no parent node,
	 * then it returns a duplicate of this node.
	 */
	FSNode getParent() const;

	/**
	 * Indicate whether the node refers to a directory or not.
	 *
	 * @todo Currently we assume that a node that is not a directory
	 * automatically is a file (ignoring things like symlinks or pipes).
	 * That might actually be OK... but we could still add an isFile method.
	 * Or even replace isDirectory by a getType() method that can return values like
	 * kDirNodeType, kFileNodeType, kInvalidNodeType.
	 */
	bool isDirectory() const override;

	/**
	 * Adds the immediate children of this FSNode to a list, optionally matching a pattern.
	 * Has no effect if this FSNode is not a directory.
	 */
	void listChildren(Common::ArchiveMemberList &childList, const char *pattern = nullptr) const override;

	/**
	 * Indicate whether the object referred by this node can be read from or not.
	 *
	 * If the node refers to a directory, readability implies being able to read
	 * and list the directory entries.
	 *
	 * If the node refers to a file, readability implies being able to read the
	 * contents of the file.
	 *
	 * @return True if the object can be read, false otherwise.
	 */
	bool isReadable() const;

	/**
	 * Indicate whether the object referred by this node can be written to or not.
	 *
	 * If the node refers to a directory, writability implies being able to modify
	 * the directory entry (i.e. rename the directory, remove it, or write files inside of it).
	 *
	 * If the node refers to a file, writability implies being able to write data
	 * to the file.
	 *
	 * @return True if the object can be written to, false otherwise.
	 */
	bool isWritable() const;

	/**
	 * Create a SeekableReadStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * @return Pointer to the stream object, nullptr in case of a failure.
	 */
	SeekableReadStream *createReadStream() const override;

	/**
	 * Create a SeekableReadStream instance corresponding to an alternate stream
	 * of the file referred by this node. This assumes that the node actually
	 * refers to a readable file and the alternate stream exists.  If either is
	 * not the case, nullptr is returned.
	 *
	 * @return Pointer to the stream object, nullptr in case of a failure.
	 */
	SeekableReadStream *createReadStreamForAltStream(AltStreamType altStreamType) const override;

	/**
	 * Create a WriteStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * When an atomic write stream is requested, the backend will write
	 * the data in a temporary file before moving it to its final destination.
	 *
	 * @param atomic Request for an atomic file write when closing.
	 *
	 * @return Pointer to the stream object, 0 in case of a failure.
	 */
	SeekableWriteStream *createWriteStream(bool atomic = true) const;

	/**
	 * Create a directory referred by this node. This assumes that this
	 * node refers to a non-existing directory. If this is not the case,
	 * false is returned.
	 *
	 * @return True if the directory was created, false otherwise.
	 */
	bool createDirectory() const;
};

/**
 * FSDirectory models a directory tree from the file system and allows users
 * to access it through the Archive interface. Searching is case-insensitive,
 * since the intended goal is to support retrieval of game data.
 *
 * FSDirectory can represent a single directory, or a tree with specified depth,
 * depending on the value passed to the 'depth' parameter in the constructors.
 * In the default mode, file names are cached with their relative path,
 * with elements separated by slashes, e.g.:
 * @code
 * c:\my\data\file.ext
 * @endcode
 * would be cached as 'data/file.ext' if FSDirectory was created on 'c:/my' with
 * depth > 1. If depth was 1, then the 'data' subdirectory would have been
 * ignored, instead.
 * Again, only SLASHES are used as separators independently from the
 * underlying file system.
 *
 * Relative paths can be specified when calling matching functions like createReadStreamForMember(),
 * hasFile(), listMatchingMembers(), and listMembers(). See the function-specific
 * documentation for more information.
 *
 * If the 'flat' argument to the constructor is true, files in subdirectories
 * are cached without the relative path, so in the following example:
 * @code
 * c:\my\data\file.ext
 * @endcode
 * would be cached as 'file.ext'.
 *
 * When the 'ignoreClashes' argument to the constructor is true, name clashes are
 * expected by the engine. It means that files that clash should be identical and
 * getSubDirectory should not be used on clashing directories. This flag is useful
 * in flat mode when there are directories with the same name at different places in the
 * tree whose name is not relevant for the engine code.
 *
 * Client code can customize the cache by using constructors with the 'prefix'
 * parameter. In this case, the prefix is prepended to each entry in the cache,
 * and effectively treated as a 'virtual' parent subdirectory. FSDirectory adds
 * a trailing slash to the prefix if needed. Following on with the previous example
 * and using 'your' as a prefix, the cache entry would have been 'your/data/file.ext'.
 * This is done both in non-flat and flat mode.
 *
 */
class FSDirectory : public Archive {
	FSNode _node;
	int _depth;
	bool _flat;
	bool _ignoreClashes;
	bool _includeDirectories;

	Path	_prefix; // string that is prepended to each cache item key
	void setPrefix(const Path &prefix);

	// Caches are case insensitive, clashes are dealt with when creating
	// Key is stored in lowercase.
	typedef HashMap<Path, FSNode, Path::IgnoreCaseAndMac_Hash, Path::IgnoreCaseAndMac_EqualTo> NodeCache;
	mutable NodeCache	_fileCache, _subDirCache;
	mutable bool _cached;

	// look for a match
	FSNode *lookupCache(NodeCache &cache, const Path &name) const;

	// cache management
	void cacheDirectoryRecursive(FSNode node, int depth, const Path& prefix) const;

	// fill cache if not already cached
	void ensureCached() const;

public:
	/**
	 * Create a FSDirectory representing a tree with the specified depth. Will result in an
	 * unbound FSDirectory if name is not found in the file system or if the node is not a
	 * valid directory.
	 */
	FSDirectory(const Path &name, int depth = 1, bool flat = false,
	            bool ignoreClashes = false, bool includeDirectories = false);
	/**
	 * @overload
	 */
	FSDirectory(const FSNode &node, int depth = 1, bool flat = false,
	            bool ignoreClashes = false, bool includeDirectories = false);

	/**
	 * Create a FSDirectory representing a tree with the specified depth. The parameter
	 * prefix is prepended to the keys in the cache. See @ref FSDirectory.
	 */
	FSDirectory(const Path &prefix, const Path &name, int depth = 1,
	            bool flat = false, bool ignoreClashes = false, bool includeDirectories = false);
	/**
	 * @overload
	 */
	FSDirectory(const Path &prefix, const FSNode &node, int depth = 1,
	            bool flat = false, bool ignoreClashes = false, bool includeDirectories = false);

	virtual ~FSDirectory();

	/**
	 * Return the underlying FSNode of the FSDirectory.
	 */
	FSNode getFSNode() const;

	/**
	 * Create a new FSDirectory pointing to a subdirectory of the instance.
	 * @return A new FSDirectory instance.
	 */
	FSDirectory *getSubDirectory(const Path &name, int depth = 1, bool flat = false,
	                             bool ignoreClashes = false);
	/**
	 * Create a new FSDirectory pointing to a subdirectory of the instance. See FSDirectory
	 * for an explanation of the prefix parameter.
	 * @return A new FSDirectory instance.
	 */
	FSDirectory *getSubDirectory(const Path &prefix, const Path &name, int depth = 1,
	                             bool flat = false, bool ignoreClashes = false);

	/**
	 * Check for the existence of a file in the cache. A full match of relative path and file name
	 * is needed for success.
	 */
	bool hasFile(const Path &path) const override;

	/**
	 * Check if a specified subpath is a directory.
	 */
	bool isPathDirectory(const Path &path) const override;

	/**
	 * Return a list of matching file names. Pattern can use GLOB wildcards.
	 */
	int listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents = false) const override;

	/**
	 * Return a list of all the files in the cache.
	 */
	int listMembers(ArchiveMemberList &list) const override;

	/**
	 * Get an ArchiveMember representation of the specified file. A full match of relative
	 * path and file name is needed for success.
	 */
	const ArchiveMemberPtr getMember(const Path &path) const override;

	/**
	 * Open the specified file. A full match of relative path and file name is needed
	 * for success.
	 */
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;

	/**
	 * Open an alternate stream for a specified file. A full match of relative path and file name is needed
	 * for success.
	 */
	SeekableReadStream *createReadStreamForMemberAltStream(const Path &path, AltStreamType altStreamType) const override;
};

/** @} */

} // End of namespace Common

#endif //COMMON_FS_H
