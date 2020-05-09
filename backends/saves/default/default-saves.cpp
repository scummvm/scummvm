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

// This define lets us use the system function remove() on Symbian, which
// is disabled by default due to a macro conflict.
// See backends/platform/symbian/src/portdefs.h .
#define SYMBIAN_USE_SYSTEM_REMOVE

#include "common/scummsys.h"

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#endif
#include "common/file.h"
#include "common/system.h"

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/default/default-saves.h"

#include "common/savefile.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/zlib.h"

#include <errno.h>	// for removeSavefile()

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
const char *DefaultSaveFileManager::TIMESTAMPS_FILENAME = "timestamps";
#endif

DefaultSaveFileManager::DefaultSaveFileManager() {
}

DefaultSaveFileManager::DefaultSaveFileManager(const Common::String &defaultSavepath) {
	ConfMan.registerDefault("savepath", defaultSavepath);
}


void DefaultSaveFileManager::checkPath(const Common::FSNode &dir) {
	clearError();
	if (!dir.exists()) {
		if (!dir.createDirectory()) {
			setError(Common::kPathDoesNotExist, "Failed to create directory '"+dir.getPath()+"'");
		}
	} else if (!dir.isDirectory()) {
		setError(Common::kPathNotDirectory, "The savepath '"+dir.getPath()+"' is not a directory");
	}
}

void DefaultSaveFileManager::updateSavefilesList(Common::StringArray &lockedFiles) {
	//make it refresh the cache next time it lists the saves
	_cachedDirectory = "";

	//remember the locked files list because some of these files don't exist yet
	_lockedFiles = lockedFiles;
}

Common::StringArray DefaultSaveFileManager::listSavefiles(const Common::String &pattern) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return Common::StringArray();

	Common::HashMap<Common::String, bool> locked;
	for (Common::StringArray::const_iterator i = _lockedFiles.begin(), end = _lockedFiles.end(); i != end; ++i) {
		locked[*i] = true;
	}

	Common::StringArray results;
	for (SaveFileCache::const_iterator file = _saveFileCache.begin(), end = _saveFileCache.end(); file != end; ++file) {
		if (!locked.contains(file->_key) && file->_key.matchString(pattern, true)) {
			results.push_back(file->_key);
		}
	}
	return results;
}

Common::InSaveFile *DefaultSaveFileManager::openRawFile(const Common::String &filename) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return nullptr;

	SaveFileCache::const_iterator file = _saveFileCache.find(filename);
	if (file == _saveFileCache.end()) {
		return nullptr;
	} else {
		// Open the file for loading.
		Common::SeekableReadStream *sf = file->_value.createReadStream();
		return sf;
	}
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const Common::String &filename) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return nullptr;

	for (Common::StringArray::const_iterator i = _lockedFiles.begin(), end = _lockedFiles.end(); i != end; ++i) {
		if (filename == *i) {
			return nullptr; //file is locked, no loading available
		}
	}

	SaveFileCache::const_iterator file = _saveFileCache.find(filename);
	if (file == _saveFileCache.end()) {
		return nullptr;
	} else {
		// Open the file for loading.
		Common::SeekableReadStream *sf = file->_value.createReadStream();
		return Common::wrapCompressedReadStream(sf);
	}
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	// Assure the savefile name cache is up-to-date.
	const Common::String savePathName = getSavePath();
	assureCached(savePathName);
	if (getError().getCode() != Common::kNoError)
		return nullptr;

	for (Common::StringArray::const_iterator i = _lockedFiles.begin(), end = _lockedFiles.end(); i != end; ++i) {
		if (filename == *i) {
			return nullptr; //file is locked, no saving available
		}
	}

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	// Update file's timestamp
	Common::HashMap<Common::String, uint32> timestamps = loadTimestamps();
	timestamps[filename] = INVALID_TIMESTAMP;
	saveTimestamps(timestamps);
#endif

	// Obtain node.
	SaveFileCache::const_iterator file = _saveFileCache.find(filename);
	Common::FSNode fileNode;

	// If the file did not exist before, we add it to the cache.
	if (file == _saveFileCache.end()) {
		const Common::FSNode savePath(savePathName);
		fileNode = savePath.getChild(filename);
	} else {
		fileNode = file->_value;
	}

	// Open the file for saving.
	Common::WriteStream *const sf = fileNode.createWriteStream();
	if (!sf)
		return nullptr;
	Common::OutSaveFile *const result = new Common::OutSaveFile(compress ? Common::wrapCompressedWriteStream(sf) : sf);

	// Add file to cache now that it exists.
	_saveFileCache[filename] = Common::FSNode(fileNode.getPath());

	return result;
}

bool DefaultSaveFileManager::removeSavefile(const Common::String &filename) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return false;

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	// Update file's timestamp
	Common::HashMap<Common::String, uint32> timestamps = loadTimestamps();
	Common::HashMap<Common::String, uint32>::iterator it = timestamps.find(filename);
	if (it != timestamps.end()) {
		timestamps.erase(it);
		saveTimestamps(timestamps);
	}
#endif

	// Obtain node if exists.
	SaveFileCache::const_iterator file = _saveFileCache.find(filename);
	if (file == _saveFileCache.end()) {
		return false;
	} else {
		const Common::FSNode fileNode = file->_value;
		// Remove from cache, this invalidates the 'file' iterator.
		_saveFileCache.erase(file);
		file = _saveFileCache.end();

		// FIXME: remove does not exist on all systems. If your port fails to
		// compile because of this, please let us know (scummvm-devel).
		// There is a nicely portable workaround, too: Make this method overloadable.
		if (remove(fileNode.getPath().c_str()) != 0) {
			if (errno == EACCES)
				setError(Common::kWritePermissionDenied, "Search or write permission denied: "+fileNode.getName());

			if (errno == ENOENT)
				setError(Common::kPathDoesNotExist, "removeSavefile: '"+fileNode.getName()+"' does not exist or path is invalid");

			return false;
		} else {
			return true;
		}
	}
}

Common::String DefaultSaveFileManager::getSavePath() const {

	Common::String dir;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath");

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (dir == "None") {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath");
	}

	return dir;
}

void DefaultSaveFileManager::assureCached(const Common::String &savePathName) {
	// Check that path exists and is usable.
	checkPath(Common::FSNode(savePathName));

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	Common::Array<Common::String> files = CloudMan.getSyncingFiles(); //returns empty array if not syncing
	if (!files.empty()) updateSavefilesList(files); //makes this cache invalid
	else _lockedFiles = files;
#endif

	if (_cachedDirectory == savePathName) {
		return;
	}

	_saveFileCache.clear();
	_cachedDirectory.clear();

	if (getError().getCode() != Common::kNoError) {
		warning("DefaultSaveFileManager::assureCached: Can not cache path '%s': '%s'", savePathName.c_str(), getErrorDesc().c_str());
		return;
	}

	// FSNode can cache its members, thus create it after checkPath to reflect
	// actual file system state.
	const Common::FSNode savePath(savePathName);

	Common::FSList children;
	if (!savePath.getChildren(children, Common::FSNode::kListFilesOnly)) {
		return;
	}

	// Build the savefile name cache.
	for (Common::FSList::const_iterator file = children.begin(), end = children.end(); file != end; ++file) {
		if (_saveFileCache.contains(file->getName())) {
			warning("DefaultSaveFileManager::assureCached: Name clash when building cache, ignoring file '%s'", file->getName().c_str());
		} else {
			_saveFileCache[file->getName()] = *file;
		}
	}

	// Only now store that we cached 'savePathName' to indicate we successfully
	// cached the directory.
	_cachedDirectory = savePathName;
}

#if defined(USE_CLOUD) && defined(USE_LIBCURL)

Common::HashMap<Common::String, uint32> DefaultSaveFileManager::loadTimestamps() {
	Common::HashMap<Common::String, uint32> timestamps;

	//refresh the files list
	Common::Array<Common::String> files;
	g_system->getSavefileManager()->updateSavefilesList(files);

	//start with listing all the files in saves/ directory and setting invalid timestamp to them
	Common::StringArray localFiles = g_system->getSavefileManager()->listSavefiles("*");
	for (uint32 i = 0; i < localFiles.size(); ++i)
		timestamps[localFiles[i]] = INVALID_TIMESTAMP;

	//now actually load timestamps from file
	Common::InSaveFile *file = g_system->getSavefileManager()->openRawFile(TIMESTAMPS_FILENAME);
	if (!file) {
		warning("DefaultSaveFileManager: failed to open '%s' file to load timestamps", TIMESTAMPS_FILENAME);
		return timestamps;
	}

	while (!file->eos()) {
		//read filename into buffer (reading until the first ' ')
		Common::String buffer;
		while (!file->eos()) {
			byte b = file->readByte();
			if (b == ' ') break;
			buffer += (char)b;
		}

		//read timestamp info buffer (reading until ' ' or some line ending char)
		Common::String filename = buffer;
		while (true) {
			bool lineEnded = false;
			buffer = "";
			while (!file->eos()) {
				byte b = file->readByte();
				if (b == ' ' || b == '\n' || b == '\r') {
					lineEnded = (b == '\n');
					break;
				}
				buffer += (char)b;
			}

			if (buffer == "" && file->eos()) break;
			if (!lineEnded) filename += " " + buffer;
			else break;
		}

		//parse timestamp
		uint32 timestamp = buffer.asUint64();
		if (buffer == "" || timestamp == 0) break;
		if (timestamps.contains(filename))
			timestamps[filename] = timestamp;
	}

	delete file;
	return timestamps;
}

void DefaultSaveFileManager::saveTimestamps(Common::HashMap<Common::String, uint32> &timestamps) {
	Common::DumpFile f;
	Common::String filename = concatWithSavesPath(TIMESTAMPS_FILENAME);
	if (!f.open(filename, true)) {
		warning("DefaultSaveFileManager: failed to open '%s' file to save timestamps", filename.c_str());
		return;
	}

	for (Common::HashMap<Common::String, uint32>::iterator i = timestamps.begin(); i != timestamps.end(); ++i) {
		uint32 v = i->_value;
		if (v < 1) v = 1; // 0 timestamp is treated as EOF up there, so we should never save zeros

		Common::String data = i->_key + Common::String::format(" %u\n", v);
		if (f.write(data.c_str(), data.size()) != data.size()) {
			warning("DefaultSaveFileManager: failed to write timestamps data into '%s'", filename.c_str());
			return;
		}
	}

	f.flush();
	f.finalize();
	f.close();
}

#endif // ifdef USE_LIBCURL

Common::String DefaultSaveFileManager::concatWithSavesPath(Common::String name) {
	DefaultSaveFileManager *manager = dynamic_cast<DefaultSaveFileManager *>(g_system->getSavefileManager());
	Common::String path = (manager ? manager->getSavePath() : ConfMan.get("savepath"));
	if (path.size() > 0 && (path.lastChar() == '/' || path.lastChar() == '\\'))
		return path + name;

	//simple heuristic to determine which path separator to use
	int backslashes = 0;
	for (uint32 i = 0; i < path.size(); ++i)
		if (path[i] == '/') --backslashes;
		else if (path[i] == '\\') ++backslashes;

	if (backslashes > 0) return path + '\\' + name;
	return path + '/' + name;
}

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
