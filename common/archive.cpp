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

#include "common/archive.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/punycode.h"
#include "common/debug.h"

namespace Common {

ArchiveMember::~ArchiveMember() {
}

U32String ArchiveMember::getDisplayName() const {
	return getName();
}

bool ArchiveMember::isInMacArchive() const {
	return false;
}

bool ArchiveMember::isDirectory() const {
	return false;
}

void ArchiveMember::listChildren(ArchiveMemberList &childList, const char *pattern) const {
}

GenericArchiveMember::GenericArchiveMember(const String &pathStr, const Archive &parent)
	: _parent(parent), _path(pathStr, parent.getPathSeparator()) {
}

GenericArchiveMember::GenericArchiveMember(const Path &path, const Archive &parent)
	: _parent(parent), _path(path) {
}

String GenericArchiveMember::getName() const {
	return _path.toString(_parent.getPathSeparator());
}

Path GenericArchiveMember::getPathInArchive() const {
	return _path;
}

String GenericArchiveMember::getFileName() const {
	return _path.getLastComponent().toString(_parent.getPathSeparator());
}

SeekableReadStream *GenericArchiveMember::createReadStream() const {
	return _parent.createReadStreamForMember(_path);
}

SeekableReadStream *GenericArchiveMember::createReadStreamForAltStream(AltStreamType altStreamType) const {
	return _parent.createReadStreamForMemberAltStream(_path, altStreamType);
}

bool GenericArchiveMember::isDirectory() const {
	return _parent.isPathDirectory(_path);
}

void GenericArchiveMember::listChildren(ArchiveMemberList &childList, const char *pattern) const {
	if (!pattern)
		pattern = "*";

	Common::Path searchPath = _path.appendComponent(pattern);

	_parent.listMatchingMembers(childList, searchPath);
}

bool Archive::isPathDirectory(const Path &path) const {
	prepareMaps();
	Common::Path pathNorm = path.normalize();
	return _directoryMap.contains(pathNorm) || _fileMap.contains(pathNorm);
}

int Archive::listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents) const {
	// Get all "names" (TODO: "files" ?)
	ArchiveMemberList allNames;
	listMembers(allNames);

	String patternString = pattern.toString(getPathSeparator());
	int matches = 0;

	char pathSepString[2] = {getPathSeparator(), '\0'};

	const char *wildcardExclusions = matchPathComponents ? NULL : pathSepString;

	ArchiveMemberList::const_iterator it = allNames.begin();
	for (; it != allNames.end(); ++it) {
		// TODO: We match case-insenstivie for now, our API does not define whether that's ok or not though...
		// For our use case case-insensitive is probably what we want to have though.
		if ((*it)->getName().matchString(patternString, true, wildcardExclusions)) {
			list.push_back(*it);
			matches++;
		}
	}

	return matches;
}

SeekableReadStream *Archive::createReadStreamForMemberAltStream(const Path &path, AltStreamType altStreamType) const {
	return nullptr;
}

Common::Error Archive::dumpArchive(const Path &destPath) {
	Common::ArchiveMemberList files;

	listMembers(files);

	byte *data = nullptr;
	uint dataSize = 0;

	for (auto &f : files) {
		Common::Path filePath = f->getPathInArchive().punycodeEncode();
		debug(1, "dumpArchive(): File: %s", filePath.toString().c_str());

		// skip if f represents a directory
		if (filePath.isSeparatorTerminated()) continue;

		Common::SeekableReadStream *stream = f->createReadStream();

		uint32 len = stream->size();
		if (dataSize < len) {
			free(data);
			data = (byte *)malloc(stream->size());
			dataSize = stream->size();
		}

		stream->read(data, len);

		Common::DumpFile out;
		Common::Path outPath = destPath.join(filePath);
		if (!out.open(outPath, true)) {
			return Common::Error(Common::kCreatingFileFailed, "Cannot open/create dump file " + outPath.toString(Common::Path::kNativeSeparator));
		} else {
			uint32 writtenBytes = out.write(data, len);
			if (writtenBytes < len) {
				// Not all data was written
				out.close();
				delete stream;
				free(data);
				return Common::Error(Common::kWritingFailed, "Not enough storage space! Please free up some storage and try again");
			}
			out.flush();
			out.close();
		}

		delete stream;
	}

	free(data);
	return Common::kNoError;
}

char Archive::getPathSeparator() const {
	return '/';
}

bool Archive::getChildren(const Common::Path &path, Common::Array<Common::String> &list, ListMode mode, bool hidden) const {
	list.clear();
	prepareMaps();
	Common::Path pathNorm = path.normalize();
	if (!_fileMap.contains(pathNorm) && !_directoryMap.contains(pathNorm))
		return false;
	if (mode == kListDirectoriesOnly || mode == kListAll)
		for (SubfileSet::iterator its = _directoryMap[pathNorm].begin(); its != _directoryMap[pathNorm].end(); its++)
		  if (hidden || its->_key.firstChar() != '.')
				list.push_back(its->_key);
	if (mode == kListFilesOnly || mode == kListAll)
		for (SubfileSet::iterator its = _fileMap[pathNorm].begin(); its != _fileMap[pathNorm].end(); its++)
			if (hidden || its->_key.firstChar() != '.')
				list.push_back(its->_key);
	return true;
}

void Archive::prepareMaps() const {
	if (_mapsAreReady)
		return;

	/* In order to avoid call-loop we need to set this variable before calling isDirectory on any members as the
	   default implementation uses maps.
	 */
	_mapsAreReady = true;

	ArchiveMemberList list;
	listMembers(list);

	for (ArchiveMemberList::iterator it = list.begin(); it != list.end(); it++) {
		Common::Path cur = (*it)->getPathInArchive().normalize();
		if (!(*it)->isDirectory()) {
			Common::Path parent = cur.getParent().normalize();
			Common::String fname = cur.baseName();
			_fileMap[parent][fname] = true;
			cur = parent;
		}

		while (!cur.empty()) {
			Common::Path parent = cur.getParent().normalize();
			Common::String dname = cur.baseName();
			_directoryMap[parent][dname] = true;
			cur = parent;
		}
	}

	for (AllfileMap::iterator itd = _directoryMap.begin();
	     itd != _directoryMap.end(); itd++) {
		for (SubfileSet::iterator its = itd->_value.begin(); its != itd->_value.end(); its++) {
			_fileMap[itd->_key].erase(its->_key);
		}
	}
}

SeekableReadStream *MemcachingCaseInsensitiveArchive::createReadStreamForMember(const Path &path) const {
	return createReadStreamForMemberImpl(path, false, Common::AltStreamType::Invalid);
}

SeekableReadStream *MemcachingCaseInsensitiveArchive::createReadStreamForMemberAltStream(const Path &path, Common::AltStreamType altStreamType) const {
	// There is no situation where an invalid alt stream should be returning anything unless the implementation
	// of readContentsForPathAltStream is broken, and attempting that will break the cache keying since we used Invalid
	// for keying the primary stream.
	if (altStreamType == Common::AltStreamType::Invalid)
		return nullptr;

	return createReadStreamForMemberImpl(path, true, altStreamType);
}

SeekableReadStream *MemcachingCaseInsensitiveArchive::createReadStreamForMemberImpl(const Path &path, bool isAltStream, Common::AltStreamType altStreamType) const {
	CacheKey cacheKey;
	cacheKey.path = translatePath(path);
	cacheKey.altStreamType = isAltStream ? altStreamType : AltStreamType::Invalid;

	bool isNew = false;
	if (!_cache.contains(cacheKey)) {
		SharedArchiveContents readResult = isAltStream ? readContentsForPathAltStream(cacheKey.path, altStreamType) : readContentsForPath(cacheKey.path);
		if (readResult._bypass)
			return readResult._bypass;
		_cache[cacheKey] = readResult;
		isNew = true;
	}

	SharedArchiveContents* entry = &_cache[cacheKey];

	// Errors and missing files. Just return nullptr,
	// no need to create stream.
	if (entry->isFileMissing())
		return nullptr;

	// Check whether the entry is still valid as WeakPtr might have expired.
	if (!entry->makeStrong()) {
		// If it's expired, recreate the entry.
		SharedArchiveContents readResult = isAltStream ? readContentsForPathAltStream(cacheKey.path, altStreamType) : readContentsForPath(cacheKey.path);
		if (readResult._bypass)
			return readResult._bypass;
		_cache[cacheKey] = readResult;
		entry = &_cache[cacheKey];
		isNew = true;
	}

	// It's possible that recreation failed in case of e.g. network
	// share going offline.
	if (entry->isFileMissing())
		return nullptr;

	// Now we have a valid contents reference. Make stream for it.
	Common::MemoryReadStream *memStream = new Common::MemoryReadStream(entry->getContents(), entry->getSize());

	// If the entry was just created and it's too big for strong caching,
	// mark the copy in cache as weak
	if (isNew && entry->getSize() > _maxStronglyCachedSize) {
		entry->makeWeak();
	}

	return memStream;
}

SharedArchiveContents MemcachingCaseInsensitiveArchive::readContentsForPathAltStream(const Path &translatedPath, AltStreamType altStreamType) const {
	return SharedArchiveContents();
}

MemcachingCaseInsensitiveArchive::CacheKey::CacheKey() : altStreamType(AltStreamType::Invalid) {
}

bool MemcachingCaseInsensitiveArchive::CacheKey_EqualTo::operator()(const CacheKey &x, const CacheKey &y) const {
	return (x.altStreamType == y.altStreamType) && x.path.equalsIgnoreCase(y.path);
}

uint MemcachingCaseInsensitiveArchive::CacheKey_Hash::operator()(const CacheKey &x) const {
	return static_cast<uint>(x.path.hashIgnoreCase() * 1000003u) ^ static_cast<uint>(x.altStreamType);
}

SearchSet::ArchiveNodeList::iterator SearchSet::find(const String &name) {
	ArchiveNodeList::iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_name == name)
			break;
	}
	return it;
}

SearchSet::ArchiveNodeList::const_iterator SearchSet::find(const String &name) const {
	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_name == name)
			break;
	}
	return it;
}

/*
	Keep the nodes sorted according to descending priorities.
	In case two or node nodes have the same priority, insertion
	order prevails.
*/
void SearchSet::insert(const Node &node) {
	ArchiveNodeList::iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_priority < node._priority)
			break;
	}
	_list.insert(it, node);
}

void SearchSet::add(const String &name, Archive *archive, int priority, bool autoFree) {
	if (find(name) == _list.end()) {
		Node node(priority, name, archive, autoFree);
		insert(node);
	} else {
		if (autoFree)
			delete archive;
		warning("SearchSet::add: archive '%s' already present", name.c_str());
	}

}

void SearchSet::addDirectory(const String &name, const Path &directory, int priority, int depth, bool flat) {
	FSNode dir(directory);
	addDirectory(name, dir, priority, depth, flat);
}

void SearchSet::addDirectory(const String &name, const FSNode &dir, int priority, int depth, bool flat) {
	if (!dir.exists() || !dir.isDirectory())
		return;

	add(name, new FSDirectory(dir, depth, flat, _ignoreClashes), priority);
}

void SearchSet::addDirectory(const Path &directory, int priority, int depth, bool flat) {
	addDirectory(directory.toString(), directory, priority, depth, flat);
}
void SearchSet::addDirectory(const FSNode &directory, int priority, int depth, bool flat) {
	addDirectory(directory.getPath().toString(), directory, priority, depth, flat);
}

void SearchSet::addSubDirectoriesMatching(const FSNode &directory, String origPattern, bool ignoreCase, int priority, int depth, bool flat) {
	FSList subDirs;
	if (!directory.getChildren(subDirs))
		return;

	String nextPattern, pattern;
	String::const_iterator sep = Common::find(origPattern.begin(), origPattern.end(), '/');
	if (sep != origPattern.end()) {
		pattern = String(origPattern.begin(), sep);

		++sep;
		if (sep != origPattern.end())
			nextPattern = String(sep, origPattern.end());
	} else {
		pattern = origPattern;
	}

	// TODO: The code we have for displaying all matches, which vary only in case, might
	// be a bit overhead, but as long as we want to display all useful information to the
	// user we will need to keep track of all directory names added so far. We might
	// want to reconsider this though.
	typedef HashMap<String, bool, IgnoreCase_Hash, IgnoreCase_EqualTo> MatchList;
	MatchList multipleMatches;
	MatchList::iterator matchIter;

	for (FSList::const_iterator i = subDirs.begin(); i != subDirs.end(); ++i) {
		String name = i->getName();

		if (matchString(name.c_str(), pattern.c_str(), ignoreCase)) {
			matchIter = multipleMatches.find(name);
			if (matchIter == multipleMatches.end()) {
				multipleMatches[name] = true;
			} else {
				if (matchIter->_value) {
					warning("Clash in case for match of pattern \"%s\" found in directory \"%s\": \"%s\"", pattern.c_str(),
							directory.getPath().toString(Common::Path::kNativeSeparator).c_str(), matchIter->_key.c_str());
					matchIter->_value = false;
				}

				warning("Clash in case for match of pattern \"%s\" found in directory \"%s\": \"%s\"", pattern.c_str(),
						directory.getPath().toString(Common::Path::kNativeSeparator).c_str(), name.c_str());
			}

			if (nextPattern.empty())
				addDirectory(name, *i, priority, depth, flat);
			else
				addSubDirectoriesMatching(*i, nextPattern, ignoreCase, priority, depth, flat);
		}
	}
}

void SearchSet::remove(const String &name) {
	ArchiveNodeList::iterator it = find(name);
	if (it != _list.end()) {
		if (it->_autoFree)
			delete it->_arc;
		_list.erase(it);
	}
}

bool SearchSet::hasArchive(const String &name) const {
	return (find(name) != _list.end());
}

Archive *SearchSet::getArchive(const String &name) const {
	auto arch = find(name);

	if (arch == _list.end())
		return nullptr;

	return arch->_arc;
}

void SearchSet::clear() {
	for (ArchiveNodeList::iterator i = _list.begin(); i != _list.end(); ++i) {
		if (i->_autoFree)
			delete i->_arc;
	}

	_list.clear();
}

void SearchSet::setPriority(const String &name, int priority) {
	ArchiveNodeList::iterator it = find(name);
	if (it == _list.end()) {
		warning("SearchSet::setPriority: archive '%s' is not present", name.c_str());
		return;
	}

	if (priority == it->_priority)
		return;

	Node node(*it);
	_list.erase(it);
	node._priority = priority;
	insert(node);
}

bool SearchSet::hasFile(const Path &path) const {
	if (path.empty())
		return false;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_arc->hasFile(path))
			return true;
	}

	return false;
}

bool SearchSet::isPathDirectory(const Path &path) const {
	if (path.empty())
		return false;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_arc->isPathDirectory(path)) {
			// See if an earlier archive contains the same path as a non-directory file.
			// If this is the case, then we want to return false here because getMember will return
			// that file.  This is a bit faster than hasFile for each archive first.
			while (it != _list.begin()) {
				--it;
				if (it->_arc->hasFile(path))
					return false;
			}

			return true;
		}
	}

	return false;
}

bool SearchSet::getChildren(const Common::Path &path, Common::Array<Common::String> &list, ListMode mode, bool hidden) const {
	bool hasAny = false;
	ArchiveNodeList::const_iterator it = _list.begin();
	list.clear();
	for (; it != _list.end(); ++it) {
		Common::Array<Common::String> tmpList;
		if (it->_arc->getChildren(path, tmpList, mode, hidden)) {
			list.push_back(tmpList);
			hasAny = true;
		}
	}

	return hasAny;
}

int SearchSet::listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents) const {
	int matches = 0;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it)
		matches += it->_arc->listMatchingMembers(list, pattern, matchPathComponents);

	return matches;
}

int SearchSet::listMatchingMembers(ArchiveMemberDetailsList &list, const Path &pattern, bool matchPathComponents) const {
	int matches = 0;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		List<ArchiveMemberPtr> matchingMembers;
		matches += it->_arc->listMatchingMembers(matchingMembers, pattern, matchPathComponents);
		for (ArchiveMemberPtr &member : matchingMembers)
			list.push_back(ArchiveMemberDetails(member, it->_name));
	}

	return matches;
}

int SearchSet::listMembers(ArchiveMemberList &list) const {
	int matches = 0;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it)
		matches += it->_arc->listMembers(list);

	return matches;
}

const ArchiveMemberPtr SearchSet::getMember(const Path &path, Archive **container) const {
	if (path.empty())
		return ArchiveMemberPtr();

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_arc->hasFile(path)) {
			if (container) {
				*container = it->_arc;
			}
			return it->_arc->getMember(path);
		}
	}

	return ArchiveMemberPtr();
}

const ArchiveMemberPtr SearchSet::getMember(const Path &path) const {
	return getMember(path, nullptr);
}

SeekableReadStream *SearchSet::createReadStreamForMember(const Path &path) const {
	if (path.empty())
		return nullptr;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		SeekableReadStream *stream = it->_arc->createReadStreamForMember(path);
		if (stream)
			return stream;
	}

	return nullptr;
}

SeekableReadStream *SearchSet::createReadStreamForMemberAltStream(const Path &path, AltStreamType altStreamType) const {
	if (path.empty())
		return nullptr;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		SeekableReadStream *stream = it->_arc->createReadStreamForMemberAltStream(path, altStreamType);
		if (stream)
			return stream;
	}

	return nullptr;
}

SeekableReadStream *SearchSet::createReadStreamForMemberNext(const Path &path, const Archive *starting) const {
	if (path.empty())
		return nullptr;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it)
		if (it->_arc == starting) {
			++it;
			break;
		}
	for (; it != _list.end(); ++it) {
		SeekableReadStream *stream = it->_arc->createReadStreamForMember(path);
		if (stream)
			return stream;
	}

	return nullptr;
}

SearchManager::SearchManager() {
	clear(); // Force a reset
}

void SearchManager::clear() {
	SearchSet::clear();

	// Always keep system specific archives in the SearchManager.
	// But we give them a lower priority than the default priority (which is 0),
	// so that archives added by client code are searched first.
	if (g_system)
		g_system->addSysArchivesToSearchSet(*this, -1);

#ifndef __ANDROID__
	// Add the current dir as a very last resort.
	// See also bug #3984.
	// But don't do this for Android platform, since it may lead to crashes
	addDirectory(".", ".", -2);
#endif
}

DECLARE_SINGLETON(SearchManager);

} // namespace Common
