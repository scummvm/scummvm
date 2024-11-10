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

#if defined(__amigaos4__)

#include "backends/fs/amigaos/amigaos-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/debug.h"
#include "common/util.h"

/**
 * Returns the last component of a given path
 *
 * @param str Common::String containing the path
 * @return Pointer to the first char of the last component inside str
 */
const char *lastPathComponent(const Common::String &str) {
	int pathOffset = str.size();

	if (pathOffset <= 0) {
		debug(6, "lastPathComponent() failed -> Bad offset (Empty path received)!");
		return 0;
	}

	const char *p = str.c_str();

	while (pathOffset > 0 && (p[pathOffset - 1] == '/' || p[pathOffset - 1] == ':'))
		pathOffset--;
	while (pathOffset > 0 && (p[pathOffset - 1] != '/' && p[pathOffset - 1] != ':'))
		pathOffset--;
	return p + pathOffset;
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode() {
	_sDisplayName = "Available HDDs/Partitions";
	_bIsValid = true;
	_bIsDirectory = true;
	_sPath = "";
	_pFileLock = 0;
	// Protection is ignored for the root volume
	_nProt = 0;
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(const Common::String &p) {
	int nodeOffset = p.size();

	if (nodeOffset <= 0) {
		debug(6, "AmigaOSFileSystemNode() failed -> Bad offset (No Path received)!");
		return;
	}

	_sPath = p;
	_sDisplayName = ::lastPathComponent(_sPath);
	_pFileLock = 0;
	_bIsDirectory = false;
	_bIsValid = false;

	// Check whether the node exists and if it's a directory
	struct ExamineData * pExd = IDOS->ExamineObjectTags(EX_StringNameInput,_sPath.c_str(),TAG_END);
	if (pExd) {
		_nProt = pExd->Protection;
		if (EXD_IS_DIRECTORY(pExd)) {
			_bIsDirectory = true;
			_pFileLock = IDOS->Lock((CONST_STRPTR)_sPath.c_str(), SHARED_LOCK);
			_bIsValid = (_pFileLock != 0);

			// Add a trailing slash, if needed
			if (_sPath.lastChar() != '/' && _sPath.lastChar() != ':')
				_sPath += '/';
		} else {
			_bIsValid = true;
		}
		IDOS->FreeDosObject(DOS_EXAMINEDATA, pExd);
	}
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName) {
	int bufSize = MAXPATHLEN;
	_pFileLock = 0;

	while (true) {
		char *n = new char[bufSize];
		if (IDOS->NameFromLock(pLock, (STRPTR)n, bufSize) != DOSFALSE) {
			_sPath = n;
			_sDisplayName = pDisplayName ? pDisplayName : IDOS->FilePart((STRPTR)n);
			delete[] n;
			break;
		}

		if (IDOS->IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			debug(6, "IDOS->IoErr() failed -> MAXPATHLEN exceeded!");
			delete[] n;
			return;
		}

		bufSize *= 2;
		delete[] n;
	}

	_bIsDirectory = false;
	_bIsValid = false;

	// Check whether the node exists and if it's a directory
	struct ExamineData * pExd = IDOS->ExamineObjectTags(EX_FileLockInput,pLock,TAG_END);
	if (pExd) {
		_nProt = pExd->Protection;
		if (EXD_IS_DIRECTORY(pExd)) {
			_bIsDirectory = true;
			_pFileLock = IDOS->DupLock(pLock);
			_bIsValid = _pFileLock != 0;
			// Add a trailing slash, if needed
			if (_sPath.lastChar() != '/' && _sPath.lastChar() != ':')
				_sPath += '/';
		} else {
			_bIsValid = true;
		}

		IDOS->FreeDosObject(DOS_EXAMINEDATA, pExd);
	} else {
		debug(6, "IDOS->ExamineObjectTags() failed -> Not a directory (or it doesn't exist)!");
	}
}

// We need the custom copy constructor because of DupLock()
AmigaOSFilesystemNode::AmigaOSFilesystemNode(const AmigaOSFilesystemNode& node)
: AbstractFSNode() {
	_sDisplayName = node._sDisplayName;
	_bIsValid = node._bIsValid;
	_bIsDirectory = node._bIsDirectory;
	_sPath = node._sPath;
	_pFileLock = IDOS->DupLock(node._pFileLock);
	_nProt = node._nProt;
}

AmigaOSFilesystemNode::~AmigaOSFilesystemNode() {
	if (_pFileLock)
		IDOS->UnLock(_pFileLock);
}

bool AmigaOSFilesystemNode::exists() const {
	if (_sPath.empty())
		return false;
	bool nodeExists = false;

	BPTR pLock = IDOS->Lock(_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		nodeExists = true;
		IDOS->UnLock(pLock);
	}

	return nodeExists;
}

AbstractFSNode *AmigaOSFilesystemNode::getChild(const Common::String &n) const {
	if (!_bIsDirectory) {
		debug(6, "AmigaOSFileSystemNode::getChild() failed -> Not a directory!");
		return 0;
	}

	Common::String newPath(_sPath);
	if (_sPath.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new AmigaOSFilesystemNode(newPath);
}

bool AmigaOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	if (!_bIsValid) {
		debug(6, "AmigaOSFileSystemNode::getChildren() failed -> Invalid node (Not a Volume)!");
		return false;
	}

	if (!_bIsDirectory) {
		debug(6, "AmigaOSFileSystemNode::getChildren() failed -> Invalid node (Not a directory)!");
		return false;
	}

	if (isRootNode()) {
		myList = listVolumes();
		return true;
	}

	APTR context = IDOS->ObtainDirContextTags(	EX_FileLockInput,	_pFileLock,
							EX_DoCurrentDir,	TRUE, /* for softlinks */
							EX_DataFields,	(EXF_NAME|EXF_LINK|EXF_TYPE),
							TAG_END);
	if (context) {
		// No need to free the value after usage, everything will be dealt with
		// by the DirContext release
		struct ExamineData * pExd = NULL;

		AmigaOSFilesystemNode *entry;
		while ((pExd = IDOS->ExamineDir(context))) {
			if ((EXD_IS_FILE(pExd) && (Common::FSNode::kListFilesOnly == mode))
			|| (EXD_IS_DIRECTORY(pExd) && (Common::FSNode::kListDirectoriesOnly == mode))
			|| Common::FSNode::kListAll == mode
			)
			{
				BPTR pLock = IDOS->Lock(pExd->Name, SHARED_LOCK);
				if (pLock) {
					entry = new AmigaOSFilesystemNode(pLock, pExd->Name);
					if (entry) {
						myList.push_back(entry);
					}

					IDOS->UnLock(pLock);
				}
			}
		}

		if (ERROR_NO_MORE_ENTRIES != IDOS->IoErr()) {
			debug(6, "IDOS->ExamineDir() failed -> End of list exceeded!");
			return false;
		}

		IDOS->ReleaseDirContext(context);
		return true;

	} else {
		debug(6, "IDOS->ObtainDirContext() failed!");
		return false;
	}
}

AbstractFSNode *AmigaOSFilesystemNode::getParent() const {
	if (isRootNode()) {
		return new AmigaOSFilesystemNode(*this);
	}

	BPTR pLock = _pFileLock;
	if (!_bIsDirectory) {
		assert(!pLock);
		pLock = IDOS->Lock((CONST_STRPTR)_sPath.c_str(), SHARED_LOCK);
		assert(pLock);
	}

	AmigaOSFilesystemNode *node;

	BPTR parentDir = IDOS->ParentDir(pLock);
	if (parentDir) {
		node = new AmigaOSFilesystemNode(parentDir);
		IDOS->UnLock(parentDir);
	} else
		node = new AmigaOSFilesystemNode();

	if (!_bIsDirectory) {
		IDOS->UnLock(pLock);
	}

	return node;
}

bool AmigaOSFilesystemNode::isReadable() const {
	if (!_bIsValid)
		return false;

	// Regular RWED protection flags are low-active or inverted,
	// thus the negation. Moreover, a pseudo root filesystem is
	// always readable, whatever the protection says
	bool readable = !(_nProt & EXDF_OTR_READ) || isRootNode();

	return readable;
}

bool AmigaOSFilesystemNode::isWritable() const {
	if (!_bIsValid)
		return false;

	// Regular RWED protection flags are low-active or inverted,
	// thus the negation. Moreover, a pseudo root filesystem is
	// never writable (due of it's pseudo nature), whatever the protection says
	bool writable = !(_nProt & EXDF_OTR_WRITE) && !isRootNode();

	return writable;
}

AbstractFSList AmigaOSFilesystemNode::listVolumes() const {
	AbstractFSList myList;

	const uint32 kLockFlags = LDF_READ | LDF_VOLUMES;
	char buffer[MAXPATHLEN];

	struct DosList *dosList = IDOS->LockDosList(kLockFlags);
	if (!dosList) {
		debug(6, "IDOS->LockDOSList() failed! -> No Volumes found!");
		return myList;
	}

	dosList = IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList) {
		if (dosList->dol_Type == DLT_VOLUME &&
			dosList->dol_Name && dosList->dol_Port) {
			// Copy name to buffer
			IDOS->CopyStringBSTRToC(dosList->dol_Name, buffer, MAXPATHLEN);

			// Volume name + '\0'
			size_t volNameSize = strlen(buffer) + 1;
			char *volName = new char [volNameSize];
			Common::strcpy_s(volName, volNameSize, buffer);
			Common::strcat_s(buffer, ":");

			BPTR volumeLock = IDOS->Lock((STRPTR)buffer, SHARED_LOCK);
			if (volumeLock) {
				char *devName = new char [MAXPATHLEN];

				// Find device name
				IDOS->DevNameFromLock(volumeLock, devName, MAXPATHLEN, DN_DEVICEONLY);
				snprintf(buffer, MAXPATHLEN, "%s (%s)", volName, devName);
				delete[] devName;

				AmigaOSFilesystemNode *entry = new AmigaOSFilesystemNode(volumeLock, buffer);
				if (entry) {
					myList.push_back(entry);
				}

				IDOS->UnLock(volumeLock);
			}

			delete[] volName;
		}
		dosList = IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	}

	IDOS->UnLockDosList(kLockFlags);
	return myList;
}

Common::SeekableReadStream *AmigaOSFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);
}


Common::SeekableWriteStream *AmigaOSFilesystemNode::createWriteStream(bool atomic) {
	return StdioStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);
}

bool AmigaOSFilesystemNode::createDirectory() {
	Common::String createPath = _sPath;
	if (createPath.lastChar() == '/') {
		createPath = createPath.substr(0, createPath.size() - 1);
	}

	BPTR dirLock = IDOS->CreateDir(createPath.c_str());
	if (dirLock) {
		IDOS->UnLock(dirLock);
		_bIsValid = true;
		_bIsDirectory = true;
	} else {
		debug(6, "AmigaOSFilesystemNode::createDirectory() failed -> Directory '%s' could not be created!", createPath.c_str());
	}

	return _bIsValid && _bIsDirectory;
}

#endif
