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

#include "common/system.h"
#include "common/textconsole.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs-factory.h"

namespace Common {

FSNode::FSNode() {
}

FSNode::FSNode(AbstractFSNode *realNode)
	: _realNode(realNode) {
}

FSNode::FSNode(const String &p) {
	assert(g_system);
	FilesystemFactory *factory = g_system->getFilesystemFactory();
	AbstractFSNode *tmp = nullptr;

	if (p.empty() || p == ".")
		tmp = factory->makeCurrentDirectoryFileNode();
	else
		tmp = factory->makeFileNodePath(p);
	_realNode = SharedPtr<AbstractFSNode>(tmp);
}

bool FSNode::operator<(const FSNode& node) const {
	// Directories come before files, i.e., are "lower".
	if (isDirectory() != node.isDirectory())
		return isDirectory();

	// If both nodes are of the same type (two files or two dirs),
	// then sort by name, ignoring case.
	return getDisplayName().compareToIgnoreCase(node.getDisplayName()) < 0;
}

bool FSNode::exists() const {
	return _realNode && _realNode->exists();
}

FSNode FSNode::getChild(const String &n) const {
	// If this node is invalid or not a directory, return an invalid node
	if (_realNode == nullptr || !_realNode->isDirectory())
		return FSNode();

	AbstractFSNode *node = _realNode->getChild(n);
	return FSNode(node);
}

bool FSNode::getChildren(FSList &fslist, ListMode mode, bool hidden) const {
	if (!_realNode || !_realNode->isDirectory())
		return false;

	AbstractFSList tmp;

	if (!_realNode->getChildren(tmp, mode, hidden))
		return false;

	fslist.clear();
	for (AbstractFSList::iterator i = tmp.begin(); i != tmp.end(); ++i) {
		fslist.push_back(FSNode(*i));
	}

	return true;
}

String FSNode::getDisplayName() const {
	assert(_realNode);
	return _realNode->getDisplayName();
}

String FSNode::getName() const {
	assert(_realNode);
	return _realNode->getName();
}

FSNode FSNode::getParent() const {
	if (_realNode == nullptr)
		return *this;

	AbstractFSNode *node = _realNode->getParent();
	if (node == nullptr) {
		return *this;
	} else {
		return FSNode(node);
	}
}

String FSNode::getPath() const {
	assert(_realNode);
	return _realNode->getPath();
}

bool FSNode::isDirectory() const {
	return _realNode && _realNode->isDirectory();
}

bool FSNode::isReadable() const {
	return _realNode && _realNode->isReadable();
}

bool FSNode::isWritable() const {
	return _realNode && _realNode->isWritable();
}

SeekableReadStream *FSNode::createReadStream() const {
	if (_realNode == nullptr)
		return nullptr;

	if (!_realNode->exists()) {
		warning("FSNode::createReadStream: '%s' does not exist", getName().c_str());
		return nullptr;
	} else if (_realNode->isDirectory()) {
		warning("FSNode::createReadStream: '%s' is a directory", getName().c_str());
		return nullptr;
	}

	return _realNode->createReadStream();
}

WriteStream *FSNode::createWriteStream() const {
	if (_realNode == nullptr)
		return nullptr;

	if (_realNode->isDirectory()) {
		warning("FSNode::createWriteStream: '%s' is a directory", getName().c_str());
		return nullptr;
	}

	return _realNode->createWriteStream();
}

bool FSNode::createDirectory() const {
	if (_realNode == nullptr)
		return false;

	if (_realNode->exists()) {
		if (_realNode->isDirectory()) {
			warning("FSNode::createDirectory: '%s' already exists", getName().c_str());
		} else {
			warning("FSNode::createDirectory: '%s' is a file", getName().c_str());
		}
		return false;
	}

	return _realNode->createDirectory();
}

FSDirectory::FSDirectory(const FSNode &node, int depth, bool flat, bool ignoreClashes, bool includeDirectories)
  : _node(node), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
	_includeDirectories(includeDirectories) {
}

FSDirectory::FSDirectory(const String &prefix, const FSNode &node, int depth, bool flat,
                         bool ignoreClashes, bool includeDirectories)
  : _node(node), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
    _includeDirectories(includeDirectories) {

	setPrefix(prefix);
}

FSDirectory::FSDirectory(const String &name, int depth, bool flat, bool ignoreClashes, bool includeDirectories)
  : _node(name), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
    _includeDirectories(includeDirectories) {
}

FSDirectory::FSDirectory(const String &prefix, const String &name, int depth, bool flat,
                         bool ignoreClashes, bool includeDirectories)
  : _node(name), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
    _includeDirectories(includeDirectories) {

	setPrefix(prefix);
}

FSDirectory::~FSDirectory() {
}

void FSDirectory::setPrefix(const String &prefix) {
	_prefix = prefix;

	if (!_prefix.empty() && !_prefix.hasSuffix("/"))
		_prefix += "/";
}

FSNode FSDirectory::getFSNode() const {
	return _node;
}

FSNode *FSDirectory::lookupCache(NodeCache &cache, const String &name) const {
	// make caching as lazy as possible
	if (!name.empty()) {
		ensureCached();

		if (cache.contains(name))
			return &cache[name];
	}

	return nullptr;
}

bool FSDirectory::hasFile(const String &name) const {
	if (name.empty() || !_node.isDirectory())
		return false;

	FSNode *node = lookupCache(_fileCache, name);
	return node && node->exists();
}

const ArchiveMemberPtr FSDirectory::getMember(const String &name) const {
	if (name.empty() || !_node.isDirectory())
		return ArchiveMemberPtr();

	FSNode *node = lookupCache(_fileCache, name);

	if (!node || !node->exists()) {
		warning("FSDirectory::getMember: '%s' does not exist", name.c_str());
		return ArchiveMemberPtr();
	} else if (node->isDirectory()) {
		warning("FSDirectory::getMember: '%s' is a directory", name.c_str());
		return ArchiveMemberPtr();
	}

	return ArchiveMemberPtr(new FSNode(*node));
}

SeekableReadStream *FSDirectory::createReadStreamForMember(const String &name) const {
	if (name.empty() || !_node.isDirectory())
		return nullptr;

	FSNode *node = lookupCache(_fileCache, name);
	if (!node)
		return nullptr;
	SeekableReadStream *stream = node->createReadStream();
	if (!stream)
		warning("FSDirectory::createReadStreamForMember: Can't create stream for file '%s'", name.c_str());

	return stream;
}

FSDirectory *FSDirectory::getSubDirectory(const String &name, int depth, bool flat, bool ignoreClashes) {
	return getSubDirectory(String(), name, depth, flat, ignoreClashes);
}

FSDirectory *FSDirectory::getSubDirectory(const String &prefix, const String &name, int depth,
        bool flat, bool ignoreClashes) {
	if (name.empty() || !_node.isDirectory())
		return nullptr;

	FSNode *node = lookupCache(_subDirCache, name);
	if (!node)
		return nullptr;

	return new FSDirectory(prefix, *node, depth, flat, ignoreClashes);
}

void FSDirectory::cacheDirectoryRecursive(FSNode node, int depth, const String& prefix) const {
	if (depth <= 0)
		return;

	FSList list;
	node.getChildren(list, FSNode::kListAll);

	FSList::iterator it = list.begin();
	for ( ; it != list.end(); ++it) {
		String name = prefix + it->getName();

		// don't touch name as it might be used for warning messages
		String lowercaseName = name;
		lowercaseName.toLowercase();

		// since the hashmap is case insensitive, we need to check for clashes when caching
		if (it->isDirectory()) {
			if (!_flat && _subDirCache.contains(lowercaseName)) {
				// Always warn in this case as it's when there are 2 directories at the same place with different case
				// That means a problem in user installation as lookups are always done case insensitive
				warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring sub-directory '%s'",
				        name.c_str());
			} else {
				if (_subDirCache.contains(lowercaseName)) {
					if (!_ignoreClashes) {
						warning("FSDirectory::cacheDirectory: name clash when building subDirCache with subdirectory '%s'",
						        name.c_str());
					}
				}
				cacheDirectoryRecursive(*it, depth - 1, _flat ? prefix : lowercaseName + "/");
				_subDirCache[lowercaseName] = *it;
			}
		} else {
			if (_fileCache.contains(lowercaseName)) {
				if (!_ignoreClashes) {
					warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring file '%s'",
					        name.c_str());
				}
			} else {
				_fileCache[lowercaseName] = *it;
			}
		}
	}

}

void FSDirectory::ensureCached() const  {
	if (_cached)
		return;
	cacheDirectoryRecursive(_node, _depth, _prefix);
	_cached = true;
}

int FSDirectory::listMatchingMembers(ArchiveMemberList &list, const String &pattern) const {
	if (!_node.isDirectory())
		return 0;

	// Cache dir data
	ensureCached();

	// need to match lowercase key, since all entries in our file cache are
	// stored as lowercase.
	String lowercasePattern(pattern);
	lowercasePattern.toLowercase();

	int matches = 0;
	for (NodeCache::const_iterator it = _fileCache.begin(); it != _fileCache.end(); ++it) {
		if (it->_key.matchString(lowercasePattern, false, true)) {
			list.push_back(ArchiveMemberPtr(new FSNode(it->_value)));
			matches++;
		}
	}
	if (_includeDirectories) {
		for (NodeCache::const_iterator it = _subDirCache.begin(); it != _subDirCache.end(); ++it) {
			if (it->_key.matchString(lowercasePattern, false, true)) {
				list.push_back(ArchiveMemberPtr(new FSNode(it->_value)));
				matches++;
			}
		}
	}

	return matches;
}

int FSDirectory::listMembers(ArchiveMemberList &list) const {
	if (!_node.isDirectory())
		return 0;

	// Cache dir data
	ensureCached();

	int files = 0;
	for (NodeCache::const_iterator it = _fileCache.begin(); it != _fileCache.end(); ++it) {
		list.push_back(ArchiveMemberPtr(new FSNode(it->_value)));
		++files;
	}

	if (_includeDirectories) {
		for (NodeCache::const_iterator it = _subDirCache.begin(); it != _subDirCache.end(); ++it) {
			list.push_back(ArchiveMemberPtr(new FSNode(it->_value)));
			++files;
		}
	}

	return files;
}


} // End of namespace Common
