/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project, contribution by Hans-Jörg Frieden and Juha Niemimäki
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
 * $URL$
 * $Id$
 */

#if defined(__amigaos4__)
#ifdef __USE_INLINE__
#undef __USE_INLINE__
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>

#ifndef USE_NEWLIB
#include <strings.h>
#endif

#include <common/stdafx.h>

#include "common/util.h"

#include "base/engine.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

#define ENTER() /* debug(6, "Enter") */
#define LEAVE() /* debug(6, "Leave") */


const uint32 kExAllBufferSize = 40960; // TODO: is this okay for sure?

class AmigaOSFilesystemNode : public AbstractFilesystemNode {
	protected:
		BPTR _pFileLock;
		String _sDisplayName;
		bool _bIsDirectory;
		bool _bIsValid;
		String _sPath;

	public:
		AmigaOSFilesystemNode();
		AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName = 0);
		AmigaOSFilesystemNode(const String &p);

	    // Note: Copy constructor is needed because it duplicates the file lock
		AmigaOSFilesystemNode(const AmigaOSFilesystemNode &node);
		
		virtual ~AmigaOSFilesystemNode();

		virtual String displayName() const { return _sDisplayName; };
		virtual bool isValid() const { return _bIsValid; };
		virtual bool isDirectory() const { return _bIsDirectory; };
		virtual String path() const { return _sPath; };

		virtual FSList listDir(ListMode mode) const;
		virtual FSList listVolumes(void) const;
		virtual AbstractFilesystemNode *parent() const;
		virtual AbstractFilesystemNode *child(const String &name) const;
};

AbstractFilesystemNode *FilesystemNode::getRoot() {
	return new AmigaOSFilesystemNode();
}

AbstractFilesystemNode *FilesystemNode::getNodeForPath(const String	&path) {
	return new AmigaOSFilesystemNode(path);
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode() {
	ENTER();
	_sDisplayName = "Available Disks";
	_bIsValid = true;
	_bIsDirectory = true;
	_sPath = "";
	_pFileLock = 0;
	LEAVE();
}


AmigaOSFilesystemNode::AmigaOSFilesystemNode(const String &p) {
	ENTER();

	int len = 0, offset = p.size();

	assert(offset > 0);

	_sPath = p;

	// Extract last	component from path
	const char *str = p.c_str();

	while (offset > 0 && (str[offset-1] == '/' || str[offset-1] == ':'))
		offset--;

	while (offset > 0 && (str[offset-1] != '/' && str[offset-1] != ':')) {
		len++;
		offset--;
	}

	_sDisplayName = String(str + offset, len);
	_pFileLock = 0;

	struct FileInfoBlock *fib = (struct FileInfoBlock *)IDOS->AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		debug(6, "FileInfoBlock is NULL");
		LEAVE();
		return;
	}

	// Check whether the node exists and if it is a directory
	BPTR pLock = IDOS->Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		if (IDOS->Examine(pLock, fib) != DOSFALSE) {
			if (FIB_IS_DRAWER(fib)) {
				_bIsDirectory = true;
				_pFileLock = IDOS->DupLock(pLock);
				_bIsValid = (_pFileLock != 0);

				// Add a trailing slash if it is needed
				const char c = _sPath.lastChar();
				if (c != '/' && c != ':')
					_sPath += '/';

			}
			else {
				_bIsDirectory = false;
				_bIsValid = true;
			}			 
		}
	}

	IDOS->FreeDosObject(DOS_FIB, fib);
	LEAVE();
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName) {
	ENTER();
	int bufSize = MAXPATHLEN;
	_pFileLock = 0;

	while (1) {
		char *name = new char[bufSize];
		if (IDOS->NameFromLock(pLock, (STRPTR)name, bufSize) != DOSFALSE) {
			_sPath = name;
			_sDisplayName = pDisplayName ? pDisplayName : IDOS->FilePart((STRPTR)name);
			delete [] name;
			break;
		}

		if (IDOS->IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			debug(6, "IoErr() != ERROR_LINE_TOO_LONG");
			LEAVE();
			delete [] name;
			return;
		}
		bufSize *= 2;
		delete [] name;
	}

	_bIsValid =	false;

	struct FileInfoBlock *fib = (struct	FileInfoBlock *)IDOS->AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		debug(6, "FileInfoBlock is NULL");
		LEAVE();
		return;
	}

	if (IDOS->Examine(pLock, fib) != DOSFALSE) {
		if (FIB_IS_DRAWER(fib)) {
			_bIsDirectory = true;
			_pFileLock = IDOS->DupLock(pLock);
			_bIsValid = _pFileLock != 0;

			const char c = _sPath.lastChar();
			if (c != '/' && c != ':')
				_sPath += '/';
		}
		else {
			_bIsDirectory = false;
			_bIsValid = true;
		}		 
	}

	IDOS->FreeDosObject(DOS_FIB, fib);
	LEAVE();
}

// We need the custom copy constructor because of DupLock()
AmigaOSFilesystemNode::AmigaOSFilesystemNode(const AmigaOSFilesystemNode& node) {
	ENTER();
	_sDisplayName = node._sDisplayName;
	_bIsValid = node._bIsValid;
	_bIsDirectory = node._bIsDirectory;
	_sPath = node._sPath;
	_pFileLock = IDOS->DupLock(node._pFileLock);
	LEAVE();
}

AmigaOSFilesystemNode::~AmigaOSFilesystemNode() {
	ENTER();
	if (_pFileLock)
		IDOS->UnLock(_pFileLock);
	LEAVE();
}

FSList AmigaOSFilesystemNode::listDir(ListMode mode) const {
	ENTER();

	FSList myList;

	if (!_bIsValid) {
		debug(6, "Invalid node");
		LEAVE();
		return myList; // Empty list
	}

	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		LEAVE();
		return myList; // Empty list
	}

	if (_pFileLock == 0) {
		debug(6, "Root node");
		LEAVE();
		return listVolumes();
	}

	struct ExAllControl *eac = (struct ExAllControl *)IDOS->AllocDosObject(DOS_EXALLCONTROL, 0);
	if (eac) {
		struct ExAllData *data = (struct ExAllData *)IExec->AllocVec(kExAllBufferSize, MEMF_ANY);
		if (data) {
			BOOL bExMore;
			eac->eac_LastKey = 0;
			do {
				// Examine directory
				bExMore = IDOS->ExAll(_pFileLock, data,	kExAllBufferSize, ED_TYPE, eac);

				LONG error = IDOS->IoErr();
				if (!bExMore && error != ERROR_NO_MORE_ENTRIES)
					break; // Abnormal failure

				if (eac->eac_Entries ==	0)
					continue; // Normal failure, no entries

				struct ExAllData *ead = data;
				do {
					if ((mode == FilesystemNode::kListAll) ||
						(EAD_IS_DRAWER(ead) && (mode == FilesystemNode::kListDirectoriesOnly)) ||
						(EAD_IS_FILE(ead) && (mode == FilesystemNode::kListFilesOnly))) {
						String full_path = _sPath;
						full_path += (char*)ead->ed_Name;

						BPTR lock = IDOS->Lock((STRPTR)full_path.c_str(), SHARED_LOCK);
						if (lock) {
							AmigaOSFilesystemNode *entry = new AmigaOSFilesystemNode(lock, (char *)ead->ed_Name);
							if (entry) {
								if (entry->isValid())
								    myList.push_back(wrap(entry));
								else
									delete entry;
							}
							IDOS->UnLock(lock);
						}
					}
					ead = ead->ed_Next;
				} while (ead);
			} while (bExMore);

			IExec->FreeVec(data);
		}

		IDOS->FreeDosObject(DOS_EXALLCONTROL, eac);
	}

	LEAVE();
	return myList;
}

AbstractFilesystemNode *AmigaOSFilesystemNode::parent() const {
	ENTER();

	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		LEAVE();
		return 0;
	}

	if (_pFileLock == 0) {
		debug(6, "Root node");
		LEAVE();
		return new AmigaOSFilesystemNode(*this);
	}

	AmigaOSFilesystemNode *node;

	BPTR parentDir = IDOS->ParentDir( _pFileLock );
	if (parentDir) {
		node = new AmigaOSFilesystemNode(parentDir);
		IDOS->UnLock(parentDir);
	}
	else
		node = new AmigaOSFilesystemNode();

	LEAVE();
	return node;
}

AbstractFilesystemNode *AmigaOSFilesystemNode::child(const String &name) const {
	assert(_bIsDirectory);
	String newPath(_sPath);
	if (_sPath.lastChar() != '/')
		newPath += '/';
	newPath += name;
	return new AmigaOSFilesystemNode(newPath);
}

FSList AmigaOSFilesystemNode::listVolumes(void)	const {
	ENTER();

	FSList myList;

	const uint32 kLockFlags = LDF_READ | LDF_VOLUMES;
	char name[MAXPATHLEN];

	struct DosList *dosList = IDOS->LockDosList(kLockFlags);
	if (!dosList) {
		debug(6, "Cannot lock the DOS list");
		LEAVE();
		return myList;
	}


	dosList = IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList) {
		if (dosList->dol_Type == DLT_VOLUME &&
			dosList->dol_Name &&
			dosList->dol_Task) {
			const char *volName = (const char *)BADDR(dosList->dol_Name)+1;
			const char *devName = (const char *)((struct Task *)dosList->dol_Task->mp_SigTask)->tc_Node.ln_Name;

			strcpy(name, volName);
			strcat(name, ":");

			BPTR volumeLock = IDOS->Lock((STRPTR)name, SHARED_LOCK);
			if (volumeLock) {
				sprintf(name, "%s (%s)", volName, devName);
				AmigaOSFilesystemNode *entry = new AmigaOSFilesystemNode(volumeLock, name);
				if (entry) {
					if (entry->isValid())
						myList.push_back(wrap(entry));
					else
						delete entry;
				}
				IDOS->UnLock(volumeLock);
			}
		}
		dosList	= IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	}

	IDOS->UnLockDosList(kLockFlags);

	LEAVE();
	return myList;
}

#endif
