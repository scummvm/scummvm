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
const char NAVIGATION_CHARACTER = '.';

// -----------------------------------------------------------------------------
// Support functions and classes
// -----------------------------------------------------------------------------

static Common::String RemoveRedundantPathSeparators(const Common::String &Path) {
	Common::String Result;

	// Iterate over all the chracters of the input path
	Common::String::const_iterator It = Path.begin();
	while (It != Path.end()) {
		if (*It == PATH_SEPARATOR) {
			// Directory separater found

			// Skip over directory separator(s)
			while (It != Path.end() && *It == PATH_SEPARATOR) ++It;

			// Unless it's the end of the path, add the separator
			if (It != Path.end()) Result += PATH_SEPARATOR;
		} else {
			// Normal characters are copied over unchanged
			Result += *It;
			++It;
		}
	}

	return Result;
}

// ---------------------------------------------------------------------

static PathElementArray SeparatePath(const Common::String &Path, const Common::String &CurrentDirectory) {
	// Determine whether the path is absolute (begins with /) or relative, in which case it's added
	// to the current directory
	Common::String wholePath = (Path.size() >= 1 && Path[0] == PATH_SEPARATOR) ? "" : CurrentDirectory + PATH_SEPARATOR;

	// Add in the provided path
	wholePath += RemoveRedundantPathSeparators(Path);

	// Parse the path and divide into it's components. This ensures that occurences of ".." and "."
	// are handled correctly.
	PathElementArray pathElements;
	size_t separatorPos = 0;
	while (separatorPos < wholePath.size()) {
		// Find next directory separator
		const char *p = strchr(wholePath.c_str() + separatorPos + 1, PATH_SEPARATOR);
		size_t nextseparatorPos = (p == NULL) ? wholePath.size() : p - wholePath.c_str();

		// Calculate the beginning and end of the path element
		Common::String::const_iterator elementBegin = wholePath.begin() + separatorPos + 1;
		Common::String::const_iterator elementEnd = wholePath.begin() + nextseparatorPos;

		if (elementEnd - elementBegin == 2 &&
			elementBegin[0] == NAVIGATION_CHARACTER &&
			elementBegin[1] == NAVIGATION_CHARACTER) {
			// element is "..", therefore the previous path element should be removed
			if (pathElements.size()) pathElements.pop_back();
		} else if (elementEnd - elementBegin == 1 &&
			elementBegin[0] == NAVIGATION_CHARACTER) {
			// element is ".", so we do nothing
		} else {
			// Normal elements get added to the list
			pathElements.push_back(PathElement(wholePath.begin() + separatorPos + 1, wholePath.begin() + nextseparatorPos));
		}

		separatorPos = nextseparatorPos;
	}

	return pathElements;
}

static Common::String NormalizePath(const Common::String &Path, const Common::String &CurrentDirectory) {
	// Get the path elements for the file
	 PathElementArray pathElements = SeparatePath(Path, CurrentDirectory);

	if (pathElements.size()) {
		// The individual path elements are fitted together, separated by a directory
		// separator. The resulting string is returned as a result
		Common::String Result;

		PathElementArray::const_iterator It = pathElements.begin();
		while (It != pathElements.end()) {
			Result += PATH_SEPARATOR;
			Result += Common::String(It->GetBegin(), It->GetEnd());
			++It;
		}

		return Result;
	} else {
		// The path list has no elements, therefore the root directory is returned
		return Common::String(PATH_SEPARATOR);
	}
}

// -----------------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------------

BS_ScummVMPackageManager::BS_ScummVMPackageManager(BS_Kernel *KernelPtr) :
		BS_PackageManager(KernelPtr),
		_currentDirectory(PATH_SEPARATOR),
		_rootFolder(ConfMan.get("path")) {
}

// -----------------------------------------------------------------------------

BS_ScummVMPackageManager::~BS_ScummVMPackageManager() {
}

// -----------------------------------------------------------------------------

BS_Service *BS_ScummVMPackageManager_CreateObject(BS_Kernel *KernelPtr) { 
	return new BS_ScummVMPackageManager(KernelPtr); 
}

// -----------------------------------------------------------------------------

/**
 * Scans through the archive list for a specified file
 */
Common::FSNode BS_ScummVMPackageManager::GetFSNode(const Common::String &FileName) {
	// Get the path elements for the file
	PathElementArray pathElements = SeparatePath(FileName, _currentDirectory);

	// Loop through checking each archive
	Common::List<ArchiveEntry>::iterator i;
	for (i = _archiveList.begin(); i != _archiveList.end(); ++i) {
		if (i->MountPath.size() > pathElements.size())
			// The mount path has more subfolder depth than the search entry, so skip it
			continue;

		// Check the path against that of the archive
		PathElementArray::iterator iPath = pathElements.begin();
		PathElementArray::iterator iEntry = i->MountPath.begin();

		for (; iEntry != i->MountPath.end(); ++iEntry, ++iPath) {
			if (Common::String(iPath->GetBegin(), iPath->GetEnd()) == 
				Common::String(iEntry->GetBegin(), iEntry->GetEnd())) 
				break;
		}
		
		if (iEntry == i->MountPath.end()) {
			// Look into the archive for the desired file
//			Common::Archive *archiveFolder = i->Archive;

			// TODO: Loop through any folders in the archive
			for (; iPath != pathElements.end(); ++iPath) {

			}			

			// Return the found node
			return Common::FSNode();
		}
	}

	 return Common::FSNode();
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::LoadPackage(const Common::String &FileName, const Common::String &MountPosition) {
	// Get the path elements for the file
	 PathElementArray pathElements = SeparatePath(MountPosition, _currentDirectory);

	Common::Archive *zipFile = Common::makeZipArchive(FileName);
	if (zipFile == NULL) {
		BS_LOG_ERRORLN("Unable to mount file \"%s\" to \"%s\"", FileName.c_str(), MountPosition.c_str());
		return false;
	} else {
		BS_LOGLN("Package '%s' mounted as '%s'.", FileName.c_str(), MountPosition.c_str());
		_archiveList.push_back(ArchiveEntry(zipFile, pathElements));
		return true;
	}
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::LoadDirectoryAsPackage(const Common::String &DirectoryName, const Common::String &MountPosition) {
	// Get the path elements for the file
	 PathElementArray pathElements = SeparatePath(MountPosition, _currentDirectory);

	Common::FSNode directory(DirectoryName);
	Common::Archive *folderArchive = new Common::FSDirectory(directory);
	if (!directory.exists() || (folderArchive == NULL)) {
		BS_LOG_ERRORLN("Unable to mount directory \"%s\" to \"%s\".", DirectoryName.c_str(), MountPosition.c_str());
		return false;
	} else {	
		BS_LOGLN("Directory '%s' mounted as '%s'.", DirectoryName.c_str(), MountPosition.c_str());
		_archiveList.push_front(ArchiveEntry(folderArchive, pathElements));
		return true;
	}
}

// -----------------------------------------------------------------------------

void *BS_ScummVMPackageManager::GetFile(const Common::String &FileName, unsigned int *FileSizePtr) {
	Common::File f;
	Common::FSNode fileNode = GetFSNode(FileName);
	if (!fileNode.exists()) return 0;
	if (!f.open(fileNode)) return 0;
	
	// If the filesize is desired, then output the size
	if (FileSizePtr) *FileSizePtr = f.size();

	// Read the file
	byte *buffer = new byte[f.size()];
	if (!f.read(buffer, f.size())) return 0;
		
	f.close();
	return buffer;
}

// -----------------------------------------------------------------------------

Common::String BS_ScummVMPackageManager::GetCurrentDirectory() {
	return _currentDirectory;
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::ChangeDirectory(const Common::String &Directory) {
	// Get the path elements for the file
	_currentDirectory = NormalizePath(Directory, _currentDirectory);
	return true;
}

// -----------------------------------------------------------------------------

Common::String BS_ScummVMPackageManager::GetAbsolutePath(const Common::String &FileName) {
	return NormalizePath(FileName, _currentDirectory);
}

// -----------------------------------------------------------------------------

unsigned int BS_ScummVMPackageManager::GetFileSize(const Common::String &FileName) {
	Common::File f;
	Common::FSNode fileNode = GetFSNode(FileName);
	if (!fileNode.exists()) return 0;
	if (!f.open(fileNode)) return 0;

	uint32 fileSize = f.size();
	f.close();
	return fileSize;
}

// -----------------------------------------------------------------------------

unsigned int BS_ScummVMPackageManager::GetFileType(const Common::String &FileName) {
	Common::File f;
	Common::FSNode fileNode = GetFSNode(FileName);
	if (!fileNode.exists()) return 0;

	return fileNode.isDirectory() ? BS_PackageManager::FT_DIRECTORY : BS_PackageManager::FT_FILE;
}

// -----------------------------------------------------------------------------

bool BS_ScummVMPackageManager::FileExists(const Common::String &FileName) {
	Common::FSNode fileNode = GetFSNode(FileName);
	return fileNode.exists();
}

// -----------------------------------------------------------------------------
// File find
// -----------------------------------------------------------------------------

class ArchiveFileSearch : public BS_PackageManager::FileSearch {
public:
	// Path must be normalised
	ArchiveFileSearch(BS_PackageManager &PackageManager, const Common::StringArray &FoundFiles) :
		_packageManager(PackageManager),
		_foundFiles(FoundFiles),
		_foundFilesIt(_foundFiles.begin()) {
	}

	virtual Common::String GetCurFileName()	{
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

	BS_PackageManager &					_packageManager;
	Common::StringArray					_foundFiles;
	Common::StringArray::const_iterator	_foundFilesIt;
};

// -----------------------------------------------------------------------------

BS_PackageManager::FileSearch *BS_ScummVMPackageManager::CreateSearch(
		const Common::String &Filter, const Common::String &Path, unsigned int TypeFilter) {
	Common::String NormalizedPath = NormalizePath(Path, _currentDirectory);

	Common::FSNode folderNode = GetFSNode(Path);
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
}

} // End of namespace Sword25

