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

#include "common/system.h"
#include "common/punycode.h"
#include "common/textconsole.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs-factory.h"

namespace Common {

// File-in-directory archive member that captures relative path
class FSDirectoryFile : public ArchiveMember {
public:
	FSDirectoryFile(const Common::Path &pathInDirectory, const FSNode &fsNode);

	SeekableReadStream *createReadStream() const override;
	String getName() const override;
	Path getPathInArchive() const override;
	String getFileName() const override;
	U32String getDisplayName() const override;

private:
	Common::Path _pathInDirectory;
	FSNode _fsNode;
};

FSDirectoryFile::FSDirectoryFile(const Common::Path &pathInDirectory, const FSNode &fsNode) : _pathInDirectory(pathInDirectory), _fsNode(fsNode) {
}

SeekableReadStream *FSDirectoryFile::createReadStream() const {
	return _fsNode.createReadStream();
}

String FSDirectoryFile::getName() const {
	return _fsNode.getName();
}

Path FSDirectoryFile::getPathInArchive() const {
	return _pathInDirectory;
}

String FSDirectoryFile::getFileName() const {
	return _fsNode.getName();
}

U32String FSDirectoryFile::getDisplayName() const {
	return _fsNode.getDisplayName();
}


FSNode::FSNode() {
}

FSNode::FSNode(AbstractFSNode *realNode)
	: _realNode(realNode) {
}

FSNode::FSNode(const Path &p) {
	assert(g_system);
	FilesystemFactory *factory = g_system->getFilesystemFactory();
	AbstractFSNode *tmp = nullptr;

	if (p.empty() || p == Path("."))
		tmp = factory->makeCurrentDirectoryFileNode();
	else
		tmp = factory->makeFileNodePath(p.toString());
	_realNode = SharedPtr<AbstractFSNode>(tmp);
}

bool FSNode::operator<(const FSNode& node) const {
	// Directories come before files, i.e., are "lower".
	if (isDirectory() != node.isDirectory())
		return isDirectory();

	// If both nodes are of the same type (two files or two dirs),
	// then sort by name, ignoring case.
	return getName().compareToIgnoreCase(node.getName()) < 0;
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

U32String FSNode::getDisplayName() const {
	assert(_realNode);
	return _realNode->getDisplayName();
}

String FSNode::getName() const {
	assert(_realNode);
	// We transparently decode any punycode-named files
	return punycode_decodefilename(_realNode->getName());
}

String FSNode::getFileName() const {
	return getName();
}

Common::Path FSNode::getPathInArchive() const {
	return getName();
}

String FSNode::getRealName() const {
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

SeekableWriteStream *FSNode::createWriteStream() const {
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

FSDirectory::FSDirectory(const Path &prefix, const FSNode &node, int depth, bool flat,
						 bool ignoreClashes, bool includeDirectories)
  : _node(node), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
	_includeDirectories(includeDirectories) {

	setPrefix(prefix);
}

FSDirectory::FSDirectory(const Path &name, int depth, bool flat, bool ignoreClashes, bool includeDirectories)
  : _node(name), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
	_includeDirectories(includeDirectories) {
}

FSDirectory::FSDirectory(const Path &prefix, const Path &name, int depth, bool flat,
						 bool ignoreClashes, bool includeDirectories)
  : _node(name), _cached(false), _depth(depth), _flat(flat), _ignoreClashes(ignoreClashes),
	_includeDirectories(includeDirectories) {

	setPrefix(prefix);
}

FSDirectory::~FSDirectory() {
}

void FSDirectory::setPrefix(const Path &prefix) {
	_prefix = prefix;
}

FSNode FSDirectory::getFSNode() const {
	return _node;
}

FSNode *FSDirectory::lookupCache(NodeCache &cache, const Path &name) const {
	// make caching as lazy as possible
	if (!name.empty()) {
		ensureCached();

		if (cache.contains(name))
			return &cache[name];
	}

	return nullptr;
}

bool FSDirectory::hasFile(const Path &path) const {
	if (path.toString().empty() || !_node.isDirectory())
		return false;

	FSNode *node = lookupCache(_fileCache, path);
	return node && node->exists();
}

const ArchiveMemberPtr FSDirectory::getMember(const Path &path) const {
	if (path.toString().empty() || !_node.isDirectory())
		return ArchiveMemberPtr();

	FSNode *node = lookupCache(_fileCache, path);

	if (!node || !node->exists()) {
		warning("FSDirectory::getMember: '%s' does not exist", Common::toPrintable(path.toString()).c_str());
		return ArchiveMemberPtr();
	} else if (node->isDirectory()) {
		warning("FSDirectory::getMember: '%s' is a directory", Common::toPrintable(path.toString()).c_str());
		return ArchiveMemberPtr();
	}

	return ArchiveMemberPtr(new FSDirectoryFile(path, *node));
}

SeekableReadStream *FSDirectory::createReadStreamForMember(const Path &path) const {
	if (path.toString().empty() || !_node.isDirectory())
		return nullptr;

	FSNode *node = lookupCache(_fileCache, path);
	if (!node)
		return nullptr;
	SeekableReadStream *stream = node->createReadStream();
	if (!stream)
		warning("FSDirectory::createReadStreamForMember: Can't create stream for file '%s'", Common::toPrintable(path.toString()).c_str());

	return stream;
}

FSDirectory *FSDirectory::getSubDirectory(const Path &name, int depth, bool flat, bool ignoreClashes) {
	return getSubDirectory(Path(), name, depth, flat, ignoreClashes);
}

FSDirectory *FSDirectory::getSubDirectory(const Path &prefix, const Path &name, int depth,
		bool flat, bool ignoreClashes) {
	if (name.toString().empty() || !_node.isDirectory())
		return nullptr;

	FSNode *node = lookupCache(_subDirCache, name);
	if (!node)
		return nullptr;

	return new FSDirectory(prefix, *node, depth, flat, ignoreClashes);
}

void FSDirectory::cacheDirectoryRecursive(FSNode node, int depth, const Path& prefix) const {
	if (depth <= 0)
		return;

	FSList list;
	node.getChildren(list, FSNode::kListAll);

	FSList::iterator it = list.begin();
	for ( ; it != list.end(); ++it) {
		Path name = prefix.appendComponent(it->getRealName());

		// since the hashmap is case insensitive, we need to check for clashes when caching
		if (it->isDirectory()) {
			if (!_flat && _subDirCache.contains(name)) {
				// Always warn in this case as it's when there are 2 directories at the same place with different case
				// That means a problem in user installation as lookups are always done case insensitive
				warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring sub-directory '%s'",
				        Common::toPrintable(name.toString('/')).c_str());
			} else {
				if (_subDirCache.contains(name)) {
					if (!_ignoreClashes) {
						warning("FSDirectory::cacheDirectory: name clash when building subDirCache with subdirectory '%s'",
						        Common::toPrintable(name.toString('/')).c_str());
					}
				}
				cacheDirectoryRecursive(*it, depth - 1, _flat ? prefix : name);
				_subDirCache[name] = *it;
			}
		} else {
			if (_fileCache.contains(name)) {
				if (!_ignoreClashes) {
					warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring file '%s'",
					        Common::toPrintable(name.toString('/')).c_str());
				}
			} else {
				_fileCache[name] = *it;

#ifdef MACOSX
				// On Mac, check for native resource fork
				String rsrcName = it->getPath() + "/..namedfork/rsrc";
				FSNode rsrc = FSNode(rsrcName);

				Path cacheName = prefix.join(it->getRealName() + "/..namedfork/rsrc");

				if (rsrc.exists()) {
					_fileCache[cacheName] = rsrc;
				}
#endif
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

int FSDirectory::listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents) const {
	if (!_node.isDirectory())
		return 0;

	// Cache dir data
	ensureCached();

	int matches = 0;
	for (NodeCache::const_iterator it = _fileCache.begin(); it != _fileCache.end(); ++it) {
		if (it->_key.matchPattern(pattern)) {
			list.push_back(ArchiveMemberPtr(new FSDirectoryFile(it->_key, it->_value)));
			matches++;
		}
	}
	if (_includeDirectories) {
		for (NodeCache::const_iterator it = _subDirCache.begin(); it != _subDirCache.end(); ++it) {
			if (it->_key.matchPattern(pattern)) {
				list.push_back(ArchiveMemberPtr(new FSDirectoryFile(it->_key, it->_value)));
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
		list.push_back(ArchiveMemberPtr(new FSDirectoryFile(it->_key, it->_value)));
		++files;
	}

	if (_includeDirectories) {
		for (NodeCache::const_iterator it = _subDirCache.begin(); it != _subDirCache.end(); ++it) {
			list.push_back(ArchiveMemberPtr(new FSDirectoryFile(it->_key, it->_value)));
			++files;
		}
	}

	return files;
}


} // End of namespace Common
