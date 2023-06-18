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

namespace Common {

GenericArchiveMember::GenericArchiveMember(const String &name, const Archive *parent)
	: _parent(parent), _name(name) {
}

String GenericArchiveMember::getName() const {
	return _name;
}

SeekableReadStream *GenericArchiveMember::createReadStream() const {
	return _parent->createReadStreamForMember(_name);
}


int Archive::listMatchingMembers(ArchiveMemberList &list, const Path &pattern, bool matchPathComponents) const {
	// Get all "names" (TODO: "files" ?)
	ArchiveMemberList allNames;
	listMembers(allNames);

	String patternString = pattern.toString();
	int matches = 0;
	const char *wildcardExclusions = matchPathComponents ? NULL : "/";

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

void Archive::dumpArchive(String destPath) {
	Common::ArchiveMemberList files;

	listMembers(files);

	byte *data = nullptr;
	uint dataSize = 0;

	for (auto &f : files) {
		Common::String filename = Common::punycode_encodefilename(f->getName());
		warning("File: %s", filename.c_str());

		Common::SeekableReadStream *stream = f->createReadStream();

		uint32 len = stream->size();
		if (dataSize < len) {
			free(data);
			data = (byte *)malloc(stream->size());
			dataSize = stream->size();
		}

		stream->read(data, len);

		Common::DumpFile out;
		Common::String outname = destPath + filename;
		if (!out.open(outname, true)) {
			warning("Archive::dumpArchive(): Can not open dump file %s", outname.c_str());
		} else {
			out.write(data, len);
			out.flush();
			out.close();
		}

		delete stream;
	}

	free(data);
}

bool DefaultListableCaseInsensitiveArchive::hasDirectory(const Common::Path &path) const {
	prepareMaps();
	Common::String pathStr = path.toString(_separator);
	if (pathStr.lastChar() == _separator)
		pathStr.deleteLastChar();
	return _directoryMap.contains(pathStr) || _fileMap.contains(pathStr);
}

bool DefaultListableCaseInsensitiveArchive::getChildren(const Common::Path &path, Common::Array<Common::String> &list, ListMode mode, bool hidden) const {
	list.clear();
	prepareMaps();
	Common::String pathStr = path.toString(_separator);
	if (pathStr.lastChar() == _separator)
		pathStr.deleteLastChar();
	if (!_fileMap.contains(pathStr) && !_directoryMap.contains(pathStr))
		return false;
	if (mode == kListDirectoriesOnly || mode == kListAll)
		for (SubfileSet::iterator its = _directoryMap[pathStr].begin(); its != _directoryMap[pathStr].end(); its++)
		  if (hidden || its->_key.firstChar() != '.')
				list.push_back(its->_key);
	if (mode == kListFilesOnly || mode == kListAll)
		for (SubfileSet::iterator its = _fileMap[pathStr].begin(); its != _fileMap[pathStr].end(); its++)
			if (hidden || its->_key.firstChar() != '.')
				list.push_back(its->_key);
	return true;
}

void DefaultListableCaseInsensitiveArchive::prepareMaps() const {
	if (_mapsAreReady)
		return;
	ArchiveMemberList list;
	listMembers(list);

	for (ArchiveMemberList::iterator it = list.begin(); it != list.end(); it++) {
		Common::String name = normalizePath((*it)->getName(), _separator);
		size_t lastSep = name.rfind(_separator);
		if (lastSep == Common::String::npos) {
			_fileMap[""][name] = true;
			continue;
		}
		if (lastSep != name.size() - 1) {
			Common::String parent = name.substr(0, lastSep);
			Common::String fname = name.substr(lastSep + 1);
			_fileMap[parent][fname] = true;
		}

		int prevSep = -1;
		for (uint i = 0; i <= lastSep; i++)
			if (name[i] == _separator) {
				Common::String parent = prevSep < 0 ? "" : name.substr(0, prevSep);
				Common::String dname = name.substr(prevSep + 1, i - (prevSep + 1));
				_directoryMap[parent][dname] = true;
				prevSep = i;
			}
	}

	for (AllfileMap::iterator itd = _directoryMap.begin();
	     itd != _directoryMap.end(); itd++) {
		for (SubfileSet::iterator its = itd->_value.begin(); its != itd->_value.end(); its++) {
			_fileMap[itd->_key].erase(its->_key);
		}
	}

	_mapsAreReady = true;
}

SeekableReadStream *MemcachingCaseInsensitiveArchive::createReadStreamForMember(const Path &path) const {
	String translated = translatePath(path);
	bool isNew = false;
	if (!_cache.contains(translated)) {
		SharedArchiveContents readResult = readContentsForPath(translated);
		if (readResult._bypass)
			return readResult._bypass;
		_cache[translated] = readResult;
		isNew = true;
	}

	SharedArchiveContents* entry = &_cache[translated];

	// Errors and missing files. Just return nullptr,
	// no need to create stream.
	if (entry->isFileMissing())
		return nullptr;

	// Check whether the entry is still valid as WeakPtr might have expired.
	if (!entry->makeStrong()) {
		// If it's expired, recreate the entry.
		SharedArchiveContents readResult = readContentsForPath(translated);
		if (readResult._bypass)
			return readResult._bypass;
		_cache[translated] = readResult;
		entry = &_cache[translated];
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

void SearchSet::addDirectory(const String &name, const String &directory, int priority, int depth, bool flat) {
	FSNode dir(directory);
	addDirectory(name, dir, priority, depth, flat);
}

void SearchSet::addDirectory(const String &name, const FSNode &dir, int priority, int depth, bool flat) {
	if (!dir.exists() || !dir.isDirectory())
		return;

	add(name, new FSDirectory(dir, depth, flat, _ignoreClashes), priority);
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
					warning("Clash in case for match of pattern \"%s\" found in directory \"%s\": \"%s\"", pattern.c_str(), directory.getPath().c_str(), matchIter->_key.c_str());
					matchIter->_value = false;
				}

				warning("Clash in case for match of pattern \"%s\" found in directory \"%s\": \"%s\"", pattern.c_str(), directory.getPath().c_str(), name.c_str());
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

bool SearchSet::hasDirectory(const Common::Path &path) const {
	if (path.empty())
		return true;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_arc->hasDirectory(path))
			return true;
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

int SearchSet::listMembers(ArchiveMemberList &list) const {
	int matches = 0;

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it)
		matches += it->_arc->listMembers(list);

	return matches;
}

const ArchiveMemberPtr SearchSet::getMember(const Path &path) const {
	if (path.empty())
		return ArchiveMemberPtr();

	ArchiveNodeList::const_iterator it = _list.begin();
	for (; it != _list.end(); ++it) {
		if (it->_arc->hasFile(path))
			return it->_arc->getMember(path);
	}

	return ArchiveMemberPtr();
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
