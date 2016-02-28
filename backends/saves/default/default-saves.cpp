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

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/default/default-saves.h"

#include "common/savefile.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/zlib.h"

#ifndef _WIN32_WCE
#include <errno.h>	// for removeSavefile()
#endif

DefaultSaveFileManager::DefaultSaveFileManager() {
}

DefaultSaveFileManager::DefaultSaveFileManager(const Common::String &defaultSavepath) {
	ConfMan.registerDefault("savepath", defaultSavepath);
}


void DefaultSaveFileManager::checkPath(const Common::FSNode &dir) {
	clearError();
	if (!dir.exists()) {
		setError(Common::kPathDoesNotExist, "The savepath '"+dir.getPath()+"' does not exist");
	} else if (!dir.isDirectory()) {
		setError(Common::kPathNotDirectory, "The savepath '"+dir.getPath()+"' is not a directory");
	}
}

Common::StringArray DefaultSaveFileManager::listSavefiles(const Common::String &pattern) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return Common::StringArray();

	Common::StringArray results;
	for (SaveFileCache::const_iterator file = _saveFileCache.begin(), end = _saveFileCache.end(); file != end; ++file) {
		if (file->_key.matchString(pattern, true)) {
			results.push_back(file->_key);
		}
	}

	return results;
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const Common::String &filename) {
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
		return Common::wrapCompressedReadStream(sf);
	}
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	// Assure the savefile name cache is up-to-date.
	const Common::String savePathName = getSavePath();
	assureCached(savePathName);
	if (getError().getCode() != Common::kNoError)
		return nullptr;

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
	Common::OutSaveFile *const result = compress ? Common::wrapCompressedWriteStream(sf) : sf;

	// Add file to cache now that it exists.
	_saveFileCache[filename] = Common::FSNode(fileNode.getPath());

	return result;
}

bool DefaultSaveFileManager::removeSavefile(const Common::String &filename) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return false;

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
#ifndef _WIN32_WCE
			if (errno == EACCES)
				setError(Common::kWritePermissionDenied, "Search or write permission denied: "+fileNode.getName());

			if (errno == ENOENT)
				setError(Common::kPathDoesNotExist, "removeSavefile: '"+fileNode.getName()+"' does not exist or path is invalid");
#endif
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

#ifdef _WIN32_WCE
	if (dir.empty())
		dir = ConfMan.get("path");
#endif

	return dir;
}

void DefaultSaveFileManager::assureCached(const Common::String &savePathName) {
	// Check that path exists and is usable.
	checkPath(Common::FSNode(savePathName));

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

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
