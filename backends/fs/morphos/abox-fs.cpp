/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#if defined(__MORPHOS__)

#include <proto/dos.h>

#include <stdio.h>

#include "base/engine.h"
#include "../fs.h"

/*
 * Implementation of the ScummVM file system API based on the MorphOS A-Box API.
 */

class ABoxFilesystemNode : public AbstractFilesystemNode {
	protected:
		BPTR _lock;
		String _displayName;
		bool _isDirectory;
		bool _isValid;
		String _path;

	public:
		ABoxFilesystemNode();
		ABoxFilesystemNode(BPTR lock, CONST_STRPTR display_name = NULL);
		ABoxFilesystemNode(const ABoxFilesystemNode *node);
		~ABoxFilesystemNode();

		virtual String displayName() const { return _displayName; }
		virtual bool isValid() const { return _isValid; }
		virtual bool isDirectory() const { return _isDirectory; }
		virtual String path() const { return _path; }

		virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const;
		static  FSList listRoot();
		virtual AbstractFilesystemNode *parent() const;
		virtual AbstractFilesystemNode *clone() const { return new ABoxFilesystemNode(this); }
};


AbstractFilesystemNode *FilesystemNode::getRoot()
{
	return new ABoxFilesystemNode();
}

ABoxFilesystemNode::ABoxFilesystemNode()
{
	_displayName = "Mounted Volumes";
	_isValid = true;
	_isDirectory = true;
	_path = "";
	_lock = NULL;
}

ABoxFilesystemNode::ABoxFilesystemNode(BPTR lock, CONST_STRPTR display_name)
{
	int bufsize = 256;

	_lock = NULL;
	for (;;)
	{
		char name[bufsize];
		if (NameFromLock(lock, name, bufsize) != DOSFALSE)
		{
			_path = name;
			_displayName = display_name ? display_name : FilePart(name);
			break;
		}
		if (IoErr() != ERROR_LINE_TOO_LONG)
		{
			_isValid = false;
			warning("Error while retrieving path name: %d", IoErr());
			return;
		}
		bufsize *= 2;
	}

	_isValid = false;

	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	if (fib == NULL)
	{
		warning("Failed to allocate memory for FileInfoBlock");
		return;
	}

	if (Examine(lock, fib) != DOSFALSE)
	{
		_isDirectory = fib->fib_EntryType > 0;
		if (_isDirectory)
		{
			if (fib->fib_EntryType != ST_ROOT)
				_path += "/";
			_lock = DupLock(lock);
			_isValid = (_lock != NULL);
		}
		else
			_isValid = true;
	}
	FreeDosObject(DOS_FIB, fib);
}

ABoxFilesystemNode::ABoxFilesystemNode(const ABoxFilesystemNode *node)
{
	_displayName = node->_displayName;
	_isValid = node->_isValid;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
	_lock = DupLock(node->_lock);
}

ABoxFilesystemNode::~ABoxFilesystemNode()
{
	if (_lock)
	{
		UnLock(_lock);
		_lock = NULL;
	}
}

FSList ABoxFilesystemNode::listDir(ListMode mode) const
{
	FSList myList;

	if (!_isValid)
		error("listDir() called on invalid node");

	if (!_isDirectory)
		error("listDir() called on file node");

	if (_lock == NULL)
	{
		/* This is the root node */
		return listRoot();
	}

	/* "Normal" file system directory */
	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);

	if (fib == NULL)
	{
		warning("Failed to allocate memory for FileInfoBlock");
		return myList;
	}

	if (Examine(_lock, fib) != DOSFALSE)
	{
		while (ExNext(_lock, fib) != DOSFALSE)
		{
			ABoxFilesystemNode *entry;
			String full_path;
			BPTR lock;

			if ((fib->fib_EntryType > 0 && (mode & kListDirectoriesOnly)) ||
				 (fib->fib_EntryType < 0 && (mode & kListFilesOnly)))
			{
				full_path = _path;
				full_path += fib->fib_FileName;
				lock = Lock(full_path.c_str(), SHARED_LOCK);
				if (lock)
				{
					entry = new ABoxFilesystemNode(lock);
					if (entry)
					{
						if (entry->isValid())
							myList.push_back(wrap(entry));
						else
							delete entry;
					}
					UnLock(lock);
				}
			}
		}

		if (IoErr() != ERROR_NO_MORE_ENTRIES)
			warning("Error while reading directory: %d", IoErr());
	}

	FreeDosObject(DOS_FIB, fib);

	return myList;
}

AbstractFilesystemNode *ABoxFilesystemNode::parent() const
{
	AbstractFilesystemNode *node = NULL;

	if (!_isDirectory)
		error("parent() called on file node");

	if (_lock == NULL) {
		/* Parent of the root is the root itself */
		node = 0;
	} else {
		BPTR parent_lock = ParentDir(_lock);
		if (parent_lock) {
			node = new ABoxFilesystemNode(parent_lock);
			UnLock(parent_lock);
		} else
			node = new ABoxFilesystemNode();
	}

	return node;
}

FSList ABoxFilesystemNode::listRoot()
{
	FSList myList;
	DosList *dosList;
	CONST ULONG lockDosListFlags = LDF_READ | LDF_VOLUMES;
	char name[256];

	dosList = LockDosList(lockDosListFlags);
	if (dosList == NULL)
	{
		warning("Could not lock dos list");
		return myList;
	}

	dosList = NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList)
	{
		if (dosList->dol_Type == DLT_VOLUME &&  // Should always be true, but ...
			 dosList->dol_Name &&                                   // Same here
			 dosList->dol_Task                                              // Will be NULL if volume is removed from drive but still in use by some program
			)
		{
			ABoxFilesystemNode *entry;
			CONST_STRPTR volume_name = (CONST_STRPTR)BADDR(dosList->dol_Name)+1;
			CONST_STRPTR device_name = (CONST_STRPTR)((struct Task *)dosList->dol_Task->mp_SigTask)->tc_Node.ln_Name;
			BPTR volume_lock;

			strcpy(name, volume_name);
			strcat(name, ":");
			volume_lock = Lock(name, SHARED_LOCK);
			if (volume_lock)
			{
				sprintf(name, "%s (%s)", volume_name, device_name);
				entry = new ABoxFilesystemNode(volume_lock, name);
				if (entry)
				{
					if (entry->isValid())
						myList.push_back(wrap(entry));
					else
						delete entry;
				}
				UnLock(volume_lock);
			}
		}
		dosList = NextDosEntry(dosList, LDF_VOLUMES);
	}

	UnLockDosList(lockDosListFlags);

	return myList;
}

#endif // defined(__MORPHOS__)


