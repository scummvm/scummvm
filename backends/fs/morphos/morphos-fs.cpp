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

#if defined(__MORPHOS__)

#include "backends/fs/morphos/morphos-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/debug.h"
#include "common/util.h"

//#define ENTER() /* debug(6, "Enter") */
//#define LEAVE() /* debug(6, "Leave") */

/**
 * Returns the last component of a given path.
 *
 * @param str Common::String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
const char *lastPathComponent(const Common::String &str) {
	int offset = str.size();

	if (offset <= 0) {
		debug(6, "Bad offset");
		return 0;
	}

	const char *p = str.c_str();

	while (offset > 0 && (p[offset-1] == '/' || p[offset-1] == ':'))
		offset--;

	while (offset > 0 && (p[offset-1] != '/' && p[offset-1] != ':'))
		offset--;

	return p + offset;
}

MorphOSFilesystemNode::MorphOSFilesystemNode() {

	_sDisplayName = "Available Disks";
	_bIsValid = true;
	_bIsDirectory = true;
	_sPath = "";
	_pFileLock = 0;
	_nProt = 0; // Protection is ignored for the root volume

}

MorphOSFilesystemNode::MorphOSFilesystemNode(const Common::String &p) {

    int len = 0;
	int offset = p.size();

	assert(offset > 0);

	if (offset <= 0) {
		debug(6, "Bad offset");
		return;
	}

	_sPath = p;
	_sDisplayName = ::lastPathComponent(_sPath);
	_pFileLock = 0;
	_bIsDirectory = false;
	_bIsValid = false;

	struct FileInfoBlock *fib = (struct FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	
	if (fib == NULL) {
	 debug(6,"Failed...");
	 return;	
	}
	
	BPTR pLock = Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
	 if (Examine(pLock, fib) != DOSFALSE) {
	 
	  if (fib->fib_EntryType > 0)
	  {
	   	_bIsDirectory = true;
	   	_pFileLock = DupLock(pLock);
		_bIsValid = (_pFileLock != 0);
	    const char c = _sPath.lastChar();
			if (c != '/' && c != ':')
				_sPath += '/';
	  
	  } else {
	  
	  	_bIsDirectory = false;
		_bIsValid = false;
	  
	  }
	 
	 
	 }
	
	
	 UnLock(pLock);
	}
	
	FreeDosObject(DOS_FIB, fib);

}

MorphOSFilesystemNode::MorphOSFilesystemNode(BPTR pLock, const char *pDisplayName) {

	int bufSize = MAXPATHLEN;
	_pFileLock = 0;

	while (true) {
		char *n = new char[bufSize];
		if (NameFromLock(pLock, (STRPTR)n, bufSize) != DOSFALSE) {
			_sPath = n;
			_sDisplayName = pDisplayName ? pDisplayName : FilePart((STRPTR)n);
			delete[] n;
			break;
		}

		if (IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			debug(6, "IoErr() != ERROR_LINE_TOO_LONG");
			//LEAVE();
			delete[] n;
			return;
		}

		bufSize *= 2;
		delete[] n;
	}

	_bIsValid = false;
	_bIsDirectory = false;


	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	
	if (fib == NULL) {
	 debug(6,"Failed...");
	 return;	
	}

	if (Examine(pLock, fib) != DOSFALSE) {
	 
  		_bIsDirectory = fib->fib_EntryType >0;
	
	  if (_bIsDirectory)
	  {
	   	if (fib->fib_EntryType != ST_ROOT)
	   		_sPath += '/';
	   	_pFileLock = DupLock(pLock);
		_bIsValid = (_pFileLock != NULL);
	  
	  } else {
	  
		_bIsValid = true;
	  
	  }
    }
		
	FreeDosObject(DOS_FIB, fib);

}

// We need the custom copy constructor because of DupLock()
MorphOSFilesystemNode::MorphOSFilesystemNode(const MorphOSFilesystemNode& node)
: AbstractFSNode() {
	//ENTER();
	_sDisplayName = node._sDisplayName;
	_bIsValid = node._bIsValid;
	_bIsDirectory = node._bIsDirectory;
	_sPath = node._sPath;
	_pFileLock = DupLock(node._pFileLock);
	_nProt = node._nProt;
	//LEAVE();
}

MorphOSFilesystemNode::~MorphOSFilesystemNode() {
	//ENTER();
	if (_pFileLock)
		UnLock(_pFileLock);
	//LEAVE();
}

bool MorphOSFilesystemNode::exists() const {
	//ENTER();
	if (_sPath.empty())
		return false;

	bool nodeExists = false;

	// Previously we were trying to examine the node in order
	// to determine if the node exists or not.
	// I don't see the point : once you have been granted a
	// lock on it, it means it exists...
	//
	// =============================  Old code
	// BPTR pLock = IDOS->Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	// if (pLock)
	// {
	// 	if (IDOS->Examine(pLock, fib) != DOSFALSE)
	// 		nodeExists = true;
	// 	IDOS->UnLock(pLock);
	// }
	//
	// IDOS->FreeDosObject(DOS_FIB, fib);
	//
	// =============================  New code
	BPTR pLock = Lock(_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		nodeExists = true;
		UnLock(pLock);
	}

	//LEAVE();
	return nodeExists;
}

AbstractFSNode *MorphOSFilesystemNode::getChild(const Common::String &n) const {
	//ENTER();
	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		return 0;
	}

	Common::String newPath(_sPath);

	if (_sPath.lastChar() != '/')
		newPath += '/';

	newPath += n;

	//LEAVE();
	return new MorphOSFilesystemNode(newPath);
}

bool MorphOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {


	//bool ret = false;

	if (!_bIsValid) {
		debug(6, "Invalid node");
		return false; // Empty list
	}

	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		return false; // Empty list
	}

	if (isRootNode()) {
		debug(6, "Root node");
		myList = listVolumes();
		return true;
	}


	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	
	if (fib == NULL) {
	 debug(6, "Failed to allocate memory for FileInfoBLock");
	 return false;
	}
	
	if (Examine(_pFileLock, fib) != DOSFALSE) {
	
	  while (ExNext(_pFileLock, fib) != DOSFALSE)
	  {
	  
	  	MorphOSFilesystemNode *entry;
	  	Common::String full_path = NULL;
	  	BPTR pLock;
	  	
	  	if ((mode == Common::FSNode::kListAll) || 
	  		(fib->fib_EntryType > 0 && (Common::FSNode::kListDirectoriesOnly == mode)) ||
	  		(fib->fib_EntryType < 0 && (Common::FSNode::kListFilesOnly == mode )))
	  		{
	  		
	  		full_path = _sPath;
	  		full_path += fib->fib_FileName;
	  		pLock = Lock(full_path.c_str(), SHARED_LOCK);
			if (pLock) {
					entry = new MorphOSFilesystemNode( pLock, fib->fib_FileName );
					if (entry) {
						myList.push_back(entry);
					}

					UnLock(pLock);
			}	  
			
			}
			
		}	
	  	if (ERROR_NO_MORE_ENTRIES != IoErr() ) {
			debug(6, "An error occurred during ExamineDir");
			return false;
		}
	}
	
	FreeDosObject(DOS_FIB, fib);
	  
	return true;
		
}

AbstractFSNode *MorphOSFilesystemNode::getParent() const {
	//ENTER();

	if (isRootNode()) {
		debug(6, "Root node");
		//LEAVE();
		return new MorphOSFilesystemNode(*this);
	}

	BPTR pLock = _pFileLock;

	if (!_bIsDirectory) {
		assert(!pLock);
		pLock = Lock((CONST_STRPTR)_sPath.c_str(), SHARED_LOCK);
		assert(pLock);
	}

	MorphOSFilesystemNode *node;

	BPTR parentDir = ParentDir( pLock );
	if (parentDir) {
		node = new MorphOSFilesystemNode(parentDir);
		UnLock(parentDir);
	} else
		node = new MorphOSFilesystemNode();

	if (!_bIsDirectory) {
		UnLock(pLock);
	}

	//LEAVE();

	return node;
}


AbstractFSList MorphOSFilesystemNode::listVolumes() const {

	AbstractFSList myList;
	DosList *dosList;
	const ULONG kLockFlags = LDF_READ | LDF_VOLUMES;
	char buffer[MAXPATHLEN];

	dosList = LockDosList(kLockFlags);
	
	if (dosList == NULL) {
		debug(6, "Cannot lock the DOS list");
		return myList;
	}

	dosList = NextDosEntry(dosList, LDF_VOLUMES);
	
	while (dosList) {
	
		if (dosList->dol_Type == DLT_VOLUME &&
			dosList->dol_Name &&
			dosList->dol_Task) {

			MorphOSFilesystemNode *entry;
			

			//CopyStringBSTRToC(dosList->dol_Name, buffer, MAXPATHLEN);

			// Volume name + '\0'
			//char *volName = new char [strlen(buffer) + 1];
			CONST_STRPTR volName = (CONST_STRPTR)BADDR(dosList->dol_Name)+1;
			CONST_STRPTR devName = (CONST_STRPTR)((struct Task *)dosList->dol_Task->mp_SigTask)->tc_Node.ln_Name;
			BPTR volumeLock;

			strcpy(buffer, volName);
			strcat(buffer, ":");

			volumeLock = Lock(buffer, SHARED_LOCK);
			if (volumeLock) {

				sprintf(buffer, "%s (%s)", volName, devName);

				entry = new MorphOSFilesystemNode(volumeLock, buffer);
				if (entry) {
					myList.push_back(entry);
				}

				UnLock(volumeLock);
			}
		}
		dosList = NextDosEntry(dosList, LDF_VOLUMES);
	}

	UnLockDosList(kLockFlags);

	return myList;
}

Common::SeekableReadStream *MorphOSFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), false);
}

Common::WriteStream *MorphOSFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}

bool MorphOSFilesystemNode::create(bool isDirectoryFlag) {
	error("Not supported");
	return false;
}

#endif //defined(__MORPHOS__)
