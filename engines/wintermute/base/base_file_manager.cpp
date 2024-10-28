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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/file/base_disk_file.h"
#include "engines/wintermute/base/file/base_savefile_manager_file.h"
#include "engines/wintermute/base/file/base_save_thumb_file.h"
#include "engines/wintermute/base/file/base_package.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/wintermute.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/tokenizer.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"
#include "common/compression/unzip.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
BaseFileManager::BaseFileManager(Common::Language lang, bool detectionMode) {
	_detectionMode = detectionMode;
	_language = lang;
	_resources = nullptr;
	initResources();
	initPaths();
	registerPackages();
}

//////////////////////////////////////////////////////////////////////
BaseFileManager::~BaseFileManager() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::cleanup() {
	// delete registered paths
	_packagePaths.clear();

	// close open files
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		delete _openFiles[i];
	}
	_openFiles.clear();

	// delete packages
	_packages.clear();

	// get rid of the resources:
	delete _resources;
	_resources = NULL;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
byte *BaseFileManager::readWholeFile(const Common::String &filename, uint32 *size, bool mustExist) {
	byte *buffer = nullptr;

	Common::SeekableReadStream *file = openFile(filename);
	if (!file) {
		if (mustExist) {
			debugC(kWintermuteDebugFileAccess, "Error opening file '%s'", filename.c_str());
		}
		return nullptr;
	}

	buffer = new byte[file->size() + 1];
	if (buffer == nullptr) {
		debugC(kWintermuteDebugFileAccess, "Error allocating buffer for file '%s' (%d bytes)", filename.c_str(), (int)file->size() + 1);
		closeFile(file);
		return nullptr;
	}

	if (file->read(buffer, (uint32)file->size()) != (uint32)file->size()) {
		debugC(kWintermuteDebugFileAccess, "Error reading file '%s'", filename.c_str());
		closeFile(file);
		delete[] buffer;
		return nullptr;
	};

	buffer[file->size()] = '\0';
	if (size != nullptr) {
		*size = file->size();
	}
	closeFile(file);

	return buffer;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::addPath(TPathType type, const Common::FSNode &path) {
	if (!path.exists()) {
		return STATUS_FAILED;
	}

	switch (type) {
	case PATH_SINGLE:
	default:
		//  _singlePaths.push_back(path);
		error("TODO: Allow adding single-paths");
		break;
	case PATH_PACKAGE:
		_packagePaths.push_back(path);
		break;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::reloadPaths() {
	// delete registered paths
	//_singlePaths.clear();
	_packagePaths.clear();

	return initPaths();
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::initPaths() {
	// Removed: Config-based file-path choice.

	// package files paths
	const Common::FSNode gameData(ConfMan.getPath("path"));
	addPath(PATH_PACKAGE, gameData);

	Common::FSNode dataSubFolder = gameData.getChild("data");
	if (dataSubFolder.exists()) {
		addPath(PATH_PACKAGE, dataSubFolder);
	}
	Common::FSNode languageSubFolder = gameData.getChild("language");
	if (languageSubFolder.exists()) {
		addPath(PATH_PACKAGE, languageSubFolder);
	}
	// Also add languages/ for Reversion1.
	languageSubFolder = gameData.getChild("languages");
	if (languageSubFolder.exists()) {
		addPath(PATH_PACKAGE, languageSubFolder);
	}
	return STATUS_OK;
}

bool BaseFileManager::registerPackages(const Common::FSList &fslist) {
	for (Common::FSList::const_iterator it = fslist.begin(); it != fslist.end(); ++it) {
		debugC(kWintermuteDebugFileAccess, "Adding %s", it->getName().c_str());
		if (it->getName().contains(".dcp")) {
			if (registerPackage(*it, it->getName())) {
				addPath(PATH_PACKAGE, *it);
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::registerPackages() {
	debugC(kWintermuteDebugFileAccess, "Scanning packages");

	// We need game flags to perform some game-specific hacks.
	uint32 flags = BaseEngine::instance().getFlags();

	// Register without using SearchMan, as otherwise the FSNode-based lookup in openPackage will fail
	// and that has to be like that to support the detection-scheme.
	Common::FSList files;
	for (Common::FSList::const_iterator it = _packagePaths.begin(); it != _packagePaths.end(); ++it) {
		debugC(kWintermuteDebugFileAccess, "Should register folder: %s %s", it->getPath().toString(Common::Path::kNativeSeparator).c_str(), it->getName().c_str());
		if (!it->getChildren(files, Common::FSNode::kListFilesOnly)) {
			warning("getChildren() failed for path: %s", it->getName().c_str());
		}
		for (Common::FSList::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
			if (!fileIt)
				continue;

			// To prevent any case sensitivity issues we make the filename
			// all lowercase here. This makes the code slightly prettier
			// than the equivalent of using equalsIgnoreCase.
			Common::String fileName = fileIt->getName();
			fileName.toLowercase();
			bool searchSignature = false;

			if (!fileName.hasSuffix(".dcp") && !fileName.hasSuffix(".exe")) {
				continue;
			}
			if (fileName.hasSuffix(".exe")) {
				searchSignature = true;
			}

			// Again, make the parent's name all lowercase to avoid any case
			// issues.
			Common::String parentName = it->getName();
			parentName.toLowercase();

			// Avoid registering all the resolutions for SD/HD games
			if (flags & GF_IGNORE_HD_FILES && fileName.hasSuffix("_hd.dcp")) {
				continue;
			} else if (flags & GF_IGNORE_SD_FILES && fileName.hasSuffix("_sd.dcp")) {
				continue;
			}

			// Avoid registering all the language files
			// TODO: Select based on the gameDesc.
			if (_language != Common::UNK_LANG) {
				// English
				if (fileName == "english.dcp" || fileName == "xlanguage_en.dcp" || fileName == "english_language_pack.dcp") {
					if (_language != Common::EN_ANY) {
						continue;
					}
				// Chinese
				} else if (fileName == "chinese.dcp" || fileName == "xlanguage_nz.dcp" || fileName == "chinese_language_pack.dcp") {
					if (_language != Common::ZH_ANY) {
						continue;
					}
				// Simplified Chinese
				} else if (fileName == "xlanguage_zh_s.dcp") {
					if (_language != Common::ZH_CHN) {
						continue;
					}
				// Traditional Chinese
				} else if (fileName == "xlanguage_zh_t.dcp") {
					if (_language != Common::ZH_TWN) {
						continue;
					}
				// Czech
				} else if (fileName == "czech.dcp" || fileName == "xlanguage_cz.dcp" || fileName == "czech_language_pack.dcp") {
					if (_language != Common::CS_CZE) {
						continue;
					}
				// French
				} else if (fileName == "french.dcp" || fileName == "xlanguage_fr.dcp" || fileName == "french_language_pack.dcp") {
					if (_language != Common::FR_FRA) {
						continue;
					}
				// German
				} else if (fileName == "german.dcp" || fileName == "xlanguage_de.dcp" || fileName == "german_language_pack.dcp") {
					if (_language != Common::DE_DEU) {
						continue;
					}
				// Italian
				} else if (fileName == "italian.dcp" || fileName == "xlanguage_it.dcp" || fileName == "italian_language_pack.dcp") {
					if (_language != Common::IT_ITA) {
						continue;
					}
				// Latvian
				} else if (fileName == "latvian.dcp" || fileName == "xlanguage_lv.dcp" || fileName == "latvian_language_pack.dcp") {
					if (_language != Common::LV_LVA) {
						continue;
					}
				// Persian
				} else if (fileName == "persian.dcp" || fileName == "xlanguage_fa.dcp" || fileName == "persian_language_pack.dcp") {
					if (_language != Common::FA_IRN) {
						continue;
					}
				// Polish
				} else if (fileName == "polish.dcp" || fileName == "xlanguage_pl.dcp" || fileName == "polish_language_pack.dcp") {
					if (_language != Common::PL_POL) {
						continue;
					}
				// Portuguese
				} else if (fileName == "portuguese.dcp" || fileName == "xlanguage_pt.dcp" || fileName == "portuguese_language_pack.dcp") {
					if (_language != Common::PT_BRA) {
						continue;
					}
				// Russian
				} else if (fileName == "russian.dcp" || fileName == "xlanguage_ru.dcp" || fileName == "russian_language_pack.dcp") {
					if (_language != Common::RU_RUS) {
						continue;
					}
				// Serbian
				} else if (fileName == "serbian.dcp" || fileName == "xlanguage_sr.dcp" || fileName == "serbian_language_pack.dcp") {
					if (_language != Common::SR_SRB) {
						continue;
					}
				// Spanish
				} else if (fileName == "spanish.dcp" || fileName == "xlanguage_es.dcp" || fileName == "spanish_language_pack.dcp") {
					if (_language != Common::ES_ESP) {
						continue;
					}
				// generic
				} else if (fileName.hasPrefix("xlanguage_")) {
					warning("Unknown language package: %s", fileName.c_str());
					continue;
				}
			}
			debugC(kWintermuteDebugFileAccess, "Registering %s %s", fileIt->getPath().toString(Common::Path::kNativeSeparator).c_str(), fileIt->getName().c_str());
			registerPackage((*fileIt), fileName, searchSignature);
		}
	}

//	debugC(kWintermuteDebugFileAccess, "  Registered %d files in %d package(s)", _files.size(), _packages.size());

	return STATUS_OK;
}

bool BaseFileManager::registerPackage(Common::FSNode file, const Common::String &filename, bool searchSignature) {
	PackageSet *pack = new PackageSet(file, filename, searchSignature);
	_packages.add(filename, pack, pack->getPriority() , true);
	_versions[filename] = pack->getVersion();

	return STATUS_OK;
}

void BaseFileManager::initResources() {
	_resources = Common::makeZipArchive("wintermute.zip");
	if (!_resources && !_detectionMode) { // Wintermute.zip is unavailable during detection
		error("Couldn't load wintermute.zip");
	}
	if (_resources) {
		assert(_resources->hasFile("syste_font.bmp"));
		assert(_resources->hasFile("invalid.bmp"));
		assert(_resources->hasFile("invalid_debug.bmp"));
	}
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openPkgFile(const Common::String &filename) {
	Common::String upcName = filename;
	upcName.toUppercase();
	Common::SeekableReadStream *file = nullptr;

	// correct slashes
	for (uint32 i = 0; i < upcName.size(); i++) {
		if (upcName[(int32)i] == '/') {
			upcName.setChar('\\', (uint32)i);
		}
	}
	Common::ArchiveMemberPtr entry = _packages.getMember(Common::Path(upcName, '\\'));
	if (!entry) {
		return nullptr;
	}
	file = entry->createReadStream();
	return file;
}

//////////////////////////////////////////////////////////////////////////
uint32 BaseFileManager::getPackageVersion(const Common::String &filename) {
	Common::HashMap<Common::String, uint32>::iterator it = _versions.find(filename);
	if (it != _versions.end()) {
		return it->_value;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::hasFile(const Common::String &filename) {
	Common::String backslashPath(filename);
	for (uint32 i = 0; i < backslashPath.size(); i++) {
		if (backslashPath[(int32)i] == '/') {
			backslashPath.setChar('\\', (uint32)i);
		}
	}
	Common::Path path(backslashPath, '\\');

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		BasePersistenceManager pm(BaseEngine::instance().getGameTargetName());
		if (filename.size() <= 9) {
			return false;
		}
		int slot = atoi(filename.c_str() + 9);
		return pm.getSaveExists(slot);
	}
	if (sfmFileExists(filename)) {
		return true;
	}
	if (diskFileExists(filename)) {
		return true;
	}
	if (_packages.hasFile(path)) {
		return true;    // We don't bother checking if the file can actually be opened, something bigger is wrong if that is the case.
	}
	if (!_detectionMode && _resources->hasFile(path)) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
int BaseFileManager::listMatchingPackageMembers(Common::ArchiveMemberList &list, const Common::String &pattern) {
	return _packages.listMatchingMembers(list, Common::Path(pattern));
}

//////////////////////////////////////////////////////////////////////////
int BaseFileManager::listMatchingFiles(Common::StringArray &list, const Common::String &pattern) {
	list = sfmFileList(pattern);

	Common::ArchiveMemberList files;
	listMatchingDiskFileMembers(files, pattern);
	for (Common::ArchiveMemberList::const_iterator it = files.begin(); it != files.end(); ++it) {
		list.push_back((*it)->getName());
	}

	return list.size();
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFile(const Common::String &filename, bool absPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0) {
		return nullptr;
	}
	debugC(kWintermuteDebugFileAccess, "Open file %s", filename.c_str());

	Common::SeekableReadStream *file = openFileRaw(filename);
	if (file && keepTrackOf) {
		_openFiles.push_back(file);
	}
	return file;
}


//////////////////////////////////////////////////////////////////////////
Common::WriteStream *BaseFileManager::openFileForWrite(const Common::String &filename) {
	if (strcmp(filename.c_str(), "") == 0) {
		return nullptr;
	}
	debugC(kWintermuteDebugFileAccess, "Open file %s for write", filename.c_str());

	return openFileForWriteRaw(filename);
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::closeFile(Common::SeekableReadStream *File) {
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		if (_openFiles[i] == File) {
			delete _openFiles[i];
			_openFiles.remove_at(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFileRaw(const Common::String &filename) {
	Common::SeekableReadStream *ret = nullptr;

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		if (!BaseEngine::instance().getGameRef()) {
			error("Attempt to load filename: %s without BaseEngine-object, this is unsupported", filename.c_str());
		}
		BaseSaveThumbFile *saveThumbFile = new BaseSaveThumbFile();
		if (DID_SUCCEED(saveThumbFile->open(filename))) {
			ret = saveThumbFile->getMemStream();
		}
		delete saveThumbFile;
		return ret;
	}

	ret = openSfmFile(filename);
	if (ret) {
		return ret;
	}

	ret = openDiskFile(filename);
	if (ret) {
		return ret;
	}

	ret = openPkgFile(filename);
	if (ret) {
		return ret;
	}

	if (!_detectionMode) {
		ret = _resources->createReadStreamForMember(Common::Path(filename));
	}
	if (ret) {
		return ret;
	}

	debugC(kWintermuteDebugFileAccess ,"BFileManager::OpenFileRaw - Failed to open %s", filename.c_str());
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
Common::WriteStream *BaseFileManager::openFileForWriteRaw(const Common::String &filename) {
	Common::WriteStream *ret = nullptr;

	ret = openSfmFileForWrite(filename);
	if (ret) {
		return ret;
	}

	debugC(kWintermuteDebugFileAccess ,"BFileManager::OpenFileRaw - Failed to open %s", filename.c_str());
	return nullptr;
}

BaseFileManager *BaseFileManager::getEngineInstance() {
	if (BaseEngine::instance().getFileManager()) {
		return BaseEngine::instance().getFileManager();
	}
	return nullptr;
}

} // End of namespace Wintermute
