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

ScummVMPackageManager::ScummVMPackageManager(BS_Kernel *kernelPtr) :
	PackageManager(kernelPtr),
	_currentDirectory(PATH_SEPARATOR),
	_rootFolder(ConfMan.get("path")) {
}

ScummVMPackageManager::~ScummVMPackageManager() {
}

BS_Service *ScummVMPackageManager_CreateObject(BS_Kernel *kernelPtr) {
	return new ScummVMPackageManager(kernelPtr);
}

/**
 * Scans through the archive list for a specified file
 */
Common::ArchiveMemberPtr ScummVMPackageManager::GetArchiveMember(const Common::String &fileName) {
	// Loop through checking each archive
	Common::List<ArchiveEntry *>::iterator i;
	for (i = _archiveList.begin(); i != _archiveList.end(); ++i) {
		if (!fileName.hasPrefix((*i)->_mountPath)) {
			// The mount path is in different subtree. Skipping
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

bool ScummVMPackageManager::LoadPackage(const Common::String &fileName, const Common::String &mountPosition) {
	Common::Archive *zipFile = Common::makeZipArchive(fileName);
	if (zipFile == NULL) {
		BS_LOG_ERRORLN("Unable to mount file \"%s\" to \"%s\"", fileName.c_str(), mountPosition.c_str());
		return false;
	} else {
		BS_LOGLN("Package '%s' mounted as '%s'.", fileName.c_str(), mountPosition.c_str());
		Common::ArchiveMemberList files;
		zipFile->listMembers(files);
		debug(0, "Capacity %d", files.size());

		for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); ++it)
			debug(3, "%s", (*it)->getName().c_str());

		_archiveList.push_back(new ArchiveEntry(zipFile, mountPosition));

		return true;
	}
}

bool ScummVMPackageManager::LoadDirectoryAsPackage(const Common::String &directoryName, const Common::String &mountPosition) {
	Common::FSNode directory(directoryName);
	Common::Archive *folderArchive = new Common::FSDirectory(directory, 6);
	if (!directory.exists() || (folderArchive == NULL)) {
		BS_LOG_ERRORLN("Unable to mount directory \"%s\" to \"%s\".", directoryName.c_str(), mountPosition.c_str());
		return false;
	} else {
		BS_LOGLN("Directory '%s' mounted as '%s'.", directoryName.c_str(), mountPosition.c_str());

		Common::ArchiveMemberList files;
		folderArchive->listMembers(files);
		debug(0, "Capacity %d", files.size());

		_archiveList.push_front(new ArchiveEntry(folderArchive, mountPosition));

		return true;
	}
}

byte *ScummVMPackageManager::GetFile(const Common::String &fileName, unsigned int *fileSizePtr) {
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

Common::String ScummVMPackageManager::GetCurrentDirectory() {
	return _currentDirectory;
}

bool ScummVMPackageManager::ChangeDirectory(const Common::String &directory) {
	// Get the path elements for the file
	_currentDirectory = normalizePath(directory, _currentDirectory);
	return true;
}

Common::String ScummVMPackageManager::GetAbsolutePath(const Common::String &fileName) {
	return normalizePath(fileName, _currentDirectory);
}

unsigned int ScummVMPackageManager::GetFileSize(const Common::String &fileName) {
	Common::SeekableReadStream *in;
	Common::ArchiveMemberPtr fileNode = GetArchiveMember(normalizePath(fileName, _currentDirectory));
	if (!fileNode)
		return 0;
	if (!(in = fileNode->createReadStream()))
		return 0;

	uint fileSize = in->size();

	return fileSize;
}

unsigned int ScummVMPackageManager::GetFileType(const Common::String &fileName) {
	warning("STUB: BS_ScummVMPackageManager::GetFileType(%s)", fileName.c_str());

	//return fileNode.isDirectory() ? BS_PackageManager::FT_DIRECTORY : BS_PackageManager::FT_FILE;
	return PackageManager::FT_FILE;
}

bool ScummVMPackageManager::FileExists(const Common::String &fileName) {
	Common::ArchiveMemberPtr fileNode = GetArchiveMember(normalizePath(fileName, _currentDirectory));
	return fileNode;
}

int ScummVMPackageManager::doSearch(Common::ArchiveMemberList &list, const Common::String &filter, const Common::String &path, unsigned int typeFilter) {
	Common::String normalizedFilter = normalizePath(filter, _currentDirectory);
	int num = 0;

	if (path.size() > 0)
		warning("STUB: BS_ScummVMPackageManager::doSearch(<%s>, <%s>, %d)", filter.c_str(), path.c_str(), typeFilter);

	// Loop through checking each archive
	Common::List<ArchiveEntry *>::iterator i;
	for (i = _archiveList.begin(); i != _archiveList.end(); ++i) {
		Common::ArchiveMemberList memberList;

		if (!normalizedFilter.hasPrefix((*i)->_mountPath)) {
			// The mount path is in different subtree. Skipping
			continue;
		}

		// Construct relative path
		Common::String resFilter(&normalizedFilter.c_str()[(*i)->_mountPath.size()]);

		if ((*i)->archive->listMatchingMembers(memberList, resFilter) == 0)
			continue;

		// Create a list of the matching names
		for (Common::ArchiveMemberList::iterator it = memberList.begin(); it != memberList.end(); ++it) {
			if (((typeFilter & PackageManager::FT_DIRECTORY) && (*it)->getName().hasSuffix("/")) ||
				((typeFilter & PackageManager::FT_FILE) && !(*it)->getName().hasSuffix("/"))) {
				list.push_back(*it);
				num++;
			}
		}
	}

	return num;
}

} // End of namespace Sword25

