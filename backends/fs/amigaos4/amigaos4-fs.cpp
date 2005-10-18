/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project, contribution by Hans-Jörg Frieden and Juha Niemimäki
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
 * $Header$
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

#include "util.h"

#include "base/engine.h"
#include "backends/fs/fs.h"

#define ENTER() /* debug(6, "Enter\n") */
#define LEAVE() /* debug(6, "Leave\n") */


const uint32 ExAllBufferSize = 40960;

class AmigaOSFilesystemNode : public AbstractFilesystemNode {
	protected:
		BPTR _pFileLock;
		String _sDisplayName;
		bool _bIsDirectory;
		bool _bIsValid;
		String _sPath;

	public:
		AmigaOSFilesystemNode();
		AmigaOSFilesystemNode(const AmigaOSFilesystemNode *pNode);
		AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName = 0);
		AmigaOSFilesystemNode(const String &p);

		~AmigaOSFilesystemNode();

		virtual String displayName() const { return _sDisplayName; };
		virtual bool isValid() const { return _bIsValid; };
		virtual bool isDirectory() const { return _bIsDirectory; };
		virtual String path() const { return _sPath; };

		virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const;
		virtual FSList listVolumes(void) const;
		virtual AbstractFilesystemNode *parent() const;
		virtual AbstractFilesystemNode *clone() const { return new AmigaOSFilesystemNode(this); };
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

	// Check whether it is a directory, and whether the file actually exists

	struct FileInfoBlock *fib = (struct FileInfoBlock *)IDOS->AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		debug(6, "fib == 0\n");
		LEAVE();
		return;
	}

	BPTR pLock = IDOS->Lock( (char *)_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		if (IDOS->Examine(pLock, fib) != DOSFALSE) {
			if (fib->fib_EntryType > 0)
				_bIsDirectory = true;
			else
				_bIsDirectory = false;

			if (_bIsDirectory) {
				if (fib->fib_EntryType != ST_ROOT)
					_sPath += "/";

				_pFileLock = IDOS->DupLock(pLock);
				_bIsValid = (_pFileLock != 0);
			}
			else _bIsValid = true;
		}
	}

	IDOS->FreeDosObject(DOS_FIB, fib);
	LEAVE();
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName) {
	ENTER();
	int bufsize = 256;
	_pFileLock = 0;

	while (1) {
		char *name = new char[bufsize];
		if (IDOS->NameFromLock(pLock, name, bufsize) != DOSFALSE) {
			_sPath = name;
			_sDisplayName = pDisplayName ? pDisplayName : IDOS->FilePart(name);
			delete name;
			break;
		}

		if (IDOS->IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			debug(6, "Error\n");
			LEAVE();
			delete name;
			return;
		}
		bufsize *= 2;
		delete name;
	}

	_bIsValid =	false;

	struct FileInfoBlock *fib = (struct	FileInfoBlock *)IDOS->AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		debug(6, "fib == 0\n");
		LEAVE();
		return;
	}

	if (IDOS->Examine(pLock, fib) != DOSFALSE) {
		if (fib->fib_EntryType > 0)
			_bIsDirectory = true;
		else
			_bIsDirectory = false;

		if (_bIsDirectory) {
			if (fib->fib_EntryType != ST_ROOT)
				_sPath += "/";

			_pFileLock = IDOS->DupLock(pLock);
			_bIsValid = (_pFileLock != 0);
		}
		else _bIsValid = true;
	}

	IDOS->FreeDosObject(DOS_FIB, fib);
	LEAVE();
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(const AmigaOSFilesystemNode *node) {
	ENTER();
	_sDisplayName = node->_sDisplayName;
	_bIsValid = node->_bIsValid;
	_bIsDirectory = node->_bIsDirectory;
	_sPath = node->_sPath;
	_pFileLock = IDOS->DupLock(node->_pFileLock);
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

	if (!_bIsValid) {
		debug(6, "Invalid node\n");
		LEAVE();
		//return 0;
	}

	if (!_bIsDirectory) {
		debug(6, "Not a directory\n");
		LEAVE();
		//return 0;
	}

	if (_pFileLock == 0) {
		debug(6, "Root node\n");
		LEAVE();
		return listVolumes();
	}

	//FSList *myList = new FSList();
	FSList myList;

	struct ExAllControl *eac;
	struct ExAllData *data, *ead;
	BOOL bExMore;

	eac = (struct ExAllControl *)IDOS->AllocDosObject(DOS_EXALLCONTROL, 0);
	if (eac) {
		data = (struct ExAllData *)IExec->AllocVec(ExAllBufferSize, MEMF_ANY);
		if (data) {
			eac->eac_LastKey = 0;
			do {
				bExMore = IDOS->ExAll(_pFileLock, data,	ExAllBufferSize,
					ED_TYPE, eac);

				LONG error = IDOS->IoErr();
				if (!bExMore && error != ERROR_NO_MORE_ENTRIES)
					break;

				if (eac->eac_Entries ==	0)
					continue;

				ead	= data;
				do {
					AmigaOSFilesystemNode *entry;
					String full_path;
					BPTR lock;

					if ((ead->ed_Type > 0 && (mode & kListDirectoriesOnly)) ||
						(ead->ed_Type < 0 && (mode & kListFilesOnly))) {
						full_path = _sPath;
						full_path += (char*)ead->ed_Name;
						lock = IDOS->Lock((char *)full_path.c_str(), SHARED_LOCK);
						if (lock) {
							entry = new AmigaOSFilesystemNode(lock, (char *)ead->ed_Name);
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
	AmigaOSFilesystemNode *node;

	if (!_bIsDirectory) {
		debug(6, "No directory\n");
		LEAVE();
		return 0;
	}

	if (_pFileLock == 0) {
		debug(6, "Root node\n");
		LEAVE();
		return clone();
	}

	BPTR parent = IDOS->ParentDir(_pFileLock);
	if (parent) {
		node = new AmigaOSFilesystemNode(parent);
		IDOS->UnLock(parent);
	}
	else
		node = new AmigaOSFilesystemNode();

	LEAVE();
	return node;
}

FSList AmigaOSFilesystemNode::listVolumes(void)	const {
	ENTER();
	//FSList *myList = new FSList();
	FSList myList;

	struct DosList *dosList;

	const uint32 lockFlags = LDF_READ | LDF_VOLUMES;
	char name[256];

	dosList = IDOS->LockDosList(lockFlags);
	if (!dosList) {
		debug(6, "Cannot lock dos list\n");
		LEAVE();
		return myList;
	}


	dosList = IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList) {
		if (dosList->dol_Type == DLT_VOLUME &&
			dosList->dol_Name &&
			dosList->dol_Task) {
			AmigaOSFilesystemNode *entry;
			const char *volname = (const char *)BADDR(dosList->dol_Name)+1;
			const char *devname = (const char *)((struct Task *)dosList->dol_Task->mp_SigTask)->tc_Node.ln_Name;

			strcpy(name, volname);
			strcat(name, ":");

			BPTR volume_lock = IDOS->Lock(name,	SHARED_LOCK);
			if (volume_lock) {
				sprintf(name, "%s (%s)", volname, devname);
				entry = new AmigaOSFilesystemNode(volume_lock, name);
				if (entry) {
					if (entry->isValid())
						myList.push_back(wrap(entry));
					else
						delete entry;
				}
				IDOS->UnLock(volume_lock);
			}
		}
		dosList	= IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	}

	IDOS->UnLockDosList(lockFlags);

	LEAVE();
	return myList;
}

#endif
