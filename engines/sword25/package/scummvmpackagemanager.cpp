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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/str-array.h"
#include "common/unzip.h"
#include "sword25/package/scummvmpackagemanager.h"

// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "SCUMMVMPACKAGEMANAGER"

namespace Sword25 {

const char PATH_SEPARATOR = '/';

static Common::String normalizePath(const Common::String &path, const Common::String &currentDirectory) {
	Common::String wholePath = (path.size() >= 1 && path[0] == PATH_SEPARATOR) ? path : currentDirectory + PATH_SEPARATOR + path;

	if (wholePath.size() == 0) {
		// The path list has no elements, therefore the root directory is returned
		return Common::String(PATH_SEPARATOR);
	}

	return Common::normalizePath(wholePath, PATH_SEPARATOR);
}

BS_ScummVMPackageManager::BS_ScummVMPackageManager(BS_Kernel *kernelPtr) :
	BS_PackageManager(kernelPtr),
	_currentDirectory(PATH_SEPARATOR),
	_rootFolder(ConfMan.get("path")) {
}

// -----------------------------------------------------------------------------

BS_ScummVMPackageManager::~BS_ScummVMPackageManager() {
}

// -----------------------------------------------------------------------------

BS_Service *BS_ScummVMPackageManager_CreateObject(BS_Kernel *kernelPtr) {
	return new BS_ScummVMPackageManager(kernelPtr);
}

// -----------------------------------------------------------------------------

/**
 * Scans through the archive list for a specified file
 */
Common::ArchiveMemberPtr BS_ScummVMPackageManager::GetArchiveMember(const Common::String &fileName) {
	// Loop through checking each archive
	Common::List<ArchiveEntry *>::iterator i;
	for (i = _archiveList.begin(); i != _archiveList.end(); ++i) {
		if (!fileName.hasPrefix((*i)->_mountPath)) {
			// The mount path has more subfolder depth than the search entry, so skip it
			continue;
		}

		// Look into the archive for the desired file
		Common::Archive *archiveFolder = (*i)->archive;

		// Construct relative path
		Common::String resPath(&fileName.c_str()[(*i)->_mountPath.size()]);

		if (archiveFolder->hasFile(resPath)) {
			return archiveFolder->getMember(resPath);
		}
	}

	return Common::ArchiveMemberPtr();
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::LoadPackage(const Common::String &fileName, const Common::String &mountPosition) {
	Common::Archive *zipFile = Common::makeZipArchive(fileName);
	if (zipFile == NULL) {
		BS_LOG_ERRORLN("Unable to mount file \"%s\" to \"%s\"", fileName.c_str(), mountPosition.c_str());
		return false;
	} else {
		BS_LOGLN("Package '%s' mounted as '%s'.", fileName.c_str(), mountPosition.c_str());
		Common::ArchiveMemberList files;
		zipFile->listMembers(files);
		debug(0, "Capacity %d", files.size());

		_archiveList.push_back(new ArchiveEntry(zipFile, mountPosition));

		return true;
	}
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::LoadDirectoryAsPackage(const Common::String &directoryName, const Common::String &mountPosition) {
	Common::FSNode directory(directoryName);
	Common::Archive *folderArchive = new Common::FSDirectory(directory);
	if (!directory.exists() || (folderArchive == NULL)) {
		BS_LOG_ERRORLN("Unable to mount directory \"%s\" to \"%s\".", directoryName.c_str(), mountPosition.c_str());
		return false;
	} else {
		BS_LOGLN("Directory '%s' mounted as '%s'.", directoryName.c_str(), mountPosition.c_str());
		_archiveList.push_front(new ArchiveEntry(folderArchive, mountPosition));
		return true;
	}
}

// -----------------------------------------------------------------------------

void *BS_ScummVMPackageManager::GetFile(const Common::String &fileName, unsigned int *fileSizePtr) {
	Common::SeekableReadStream *in;
	Common::ArchiveMemberPtr fileNode = GetArchiveMember(normalizePath(fileName, _currentDirectory));
	if (!fileNode)
		return 0;
	if (!(in = fileNode->createReadStream()))
		return 0;

	// If the filesize is desired, then output the size
	if (fileSizePtr)
		*fileSizePtr = in->size();

	if (in->size() > 204800)
		warning("UGLY: UGLY: Sucking >200kb file into memory (%d bytes)", in->size());

	// Read the file
	byte *buffer = new byte[in->size()];
	if (!in->read(buffer, in->size())) return 0;

	return buffer;
}

// -----------------------------------------------------------------------------

Common::String BS_ScummVMPackageManager::GetCurrentDirectory() {
	return _currentDirectory;
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::ChangeDirectory(const Common::String &directory) {
	// Get the path elements for the file
	_currentDirectory = normalizePath(directory, _currentDirectory);
	return true;
}

// -----------------------------------------------------------------------------

Common::String BS_ScummVMPackageManager::GetAbsolutePath(const Common::String &fileName) {
	return normalizePath(fileName, _currentDirectory);
}

// -----------------------------------------------------------------------------

unsigned int BS_ScummVMPackageManager::GetFileSize(const Common::String &fileName) {
	Common::SeekableReadStream *in;
	Common::ArchiveMemberPtr fileNode = GetArchiveMember(normalizePath(fileName, _currentDirectory));
	if (!fileNode)
		return 0;
	if (!(in = fileNode->createReadStream()))
		return 0;

	uint fileSize = in->size();

	return fileSize;
}

// -----------------------------------------------------------------------------

unsigned int BS_ScummVMPackageManager::GetFileType(const Common::String &fileName) {
	warning("STUB: BS_ScummVMPackageManager::GetFileType(%s)", fileName.c_str());

	//return fileNode.isDirectory() ? BS_PackageManager::FT_DIRECTORY : BS_PackageManager::FT_FILE;
	return BS_PackageManager::FT_FILE;
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::FileExists(const Common::String &fileName) {
	Common::ArchiveMemberPtr fileNode = GetArchiveMember(normalizePath(fileName, _currentDirectory));
	return fileNode;
}

// -----------------------------------------------------------------------------
// File find
// -----------------------------------------------------------------------------

class ArchiveFileSearch : public BS_PackageManager::FileSearch {
public:
	// Path must be normalised
	ArchiveFileSearch(BS_PackageManager &packageManager, const Common::StringArray &foundFiles) :
		_packageManager(packageManager),
		_foundFiles(foundFiles),
		_foundFilesIt(_foundFiles.begin()) {
	}

	virtual Common::String GetCurFileName() {
		return *_foundFilesIt;
	}

	virtual unsigned int GetCurFileType() {
		return _packageManager.GetFileType(*_foundFilesIt);
	}

	virtual unsigned int GetCurFileSize() {
		return _packageManager.GetFileSize(*_foundFilesIt);
	}

	virtual bool NextFile() {
		++_foundFilesIt;
		return _foundFilesIt != _foundFiles.end();
	}

	BS_PackageManager                  &_packageManager;
	Common::StringArray                 _foundFiles;
	Common::StringArray::const_iterator _foundFilesIt;
};

// -----------------------------------------------------------------------------

BS_PackageManager::FileSearch *BS_ScummVMPackageManager::CreateSearch(
    const Common::String &filter, const Common::String &path, unsigned int typeFilter) {
#if 0
	Common::String normalizedPath = normalizePath(path, _currentDirectory);

	Common::FSNode folderNode = GetFSNode(path);
	if (!folderNode.exists() || !folderNode.isDirectory()) return NULL;

	Common::Archive *folder = new Common::FSDirectory(folderNode);
	Common::ArchiveMemberList memberList;

	if (folder->listMatchingMembers(memberList, Filter) == 0)
		return NULL;

	// Create a list of the matching names
	Common::StringArray nameList;
	for (Common::ArchiveMemberList::iterator i = memberList.begin(); i != memberList.end(); ++i) {
		nameList.push_back((*i)->getName());
	}

	// Return a ArchiveFileSearch object that encapsulates the name list
	return new ArchiveFileSearch(*this, nameList);
#else
	warning("STUB: BS_ScummVMPackageManager::CreateSearch(%s, %s, %d)", filter.c_str(), path.c_str(), typeFilter);
	Common::StringArray nameList;
	return new ArchiveFileSearch(*this, nameList);
#endif
}

} // End of namespace Sword25

