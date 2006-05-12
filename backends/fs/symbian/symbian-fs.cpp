/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005-2006 The ScummVM project
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

#if defined (__SYMBIAN32__) 
#include "common/stdafx.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

#include <dirent.h>
#include <eikenv.h>
#include <f32file.h>
#include <bautils.h>

/*
 * Implementation of the ScummVM file system API based on POSIX.
 */

class SymbianFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	bool _isPseudoRoot;

public:
	SymbianFilesystemNode(bool aIsRoot);
	SymbianFilesystemNode(const String &path);
	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;
};


static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '\\') {
		--cur;
	}

	return cur + 1;
}

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return AbstractFilesystemNode::getRoot();
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new SymbianFilesystemNode(true);
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new SymbianFilesystemNode(path);
}

SymbianFilesystemNode::SymbianFilesystemNode(bool aIsRoot) {
	_path = "";
	_isValid = true;
	_isDirectory = true;
	_isPseudoRoot = aIsRoot;
	_displayName = "Root";

}

SymbianFilesystemNode::SymbianFilesystemNode(const String &path) {
	if (path.size() == 0)
		_isPseudoRoot = true;
	else
		_isPseudoRoot = false;

	_path = path;
	const char *dsplName = NULL, *pos = path.c_str();
	// FIXME -- why is this code scanning for a slash '/' when the rest of
	// the code in this file uses backslashes '\' ?
	// TODO: Use lastPathComponent here.
	while (*pos)
		if (*pos++ == '/')
			dsplName = pos;
	_displayName = String(dsplName);
	_isValid = true;
	_isDirectory = true;
}

bool SymbianFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);

	if (_isPseudoRoot) {
		// Drives enumeration
		RFs fs = CEikonEnv::Static()->FsSession();
		TInt driveNumber;
		TChar driveLetter;
		TUint driveLetterValue;
		TVolumeInfo volumeInfo;
		TBuf8<30> driveLabel8;
		TBuf8<30> driveString8;
		
		for (driveNumber=EDriveA; driveNumber<=EDriveZ; driveNumber++) {
			TInt err = fs.Volume(volumeInfo, driveNumber);
			if (err != KErrNone)
				continue; 
			if(fs.DriveToChar(driveNumber,driveLetter) != KErrNone)
				continue;

			driveLetterValue = driveLetter;

			if(volumeInfo.iName.Length() > 0) {				
				driveLabel8.Copy(volumeInfo.iName); // 16 to 8bit des // enabling this line alone gives KERN-EXEC 3 with non-optimized GCC? WHY? grrr
				driveString8.Format(_L8("Drive %c: (%S)"), driveLetterValue, &driveLabel8);
			} else {
				driveString8.Format(_L8("Drive %c:"), driveLetterValue);
			}
				
			char path[10];
			sprintf(path,"%c:\\", driveNumber+'A');
			
			SymbianFilesystemNode entry(false);		
			entry._displayName = (char*)driveString8.PtrZ(); // drive_name
			entry._isDirectory = true;
			entry._isValid = true;
			entry._isPseudoRoot = false;
			entry._path = path;
			myList.push_back(new SymbianFilesystemNode(entry));
		}
	} else {
		TPtrC8 ptr((const unsigned char*)_path.c_str(),_path.size());
		TFileName fname;
		fname.Copy(ptr);
		TBuf8<256>nameBuf;
		CDir* dirPtr;
		if(CEikonEnv::Static()->FsSession().GetDir(fname,KEntryAttNormal|KEntryAttDir,0,dirPtr)==KErrNone) {
			CleanupStack::PushL(dirPtr);
			TInt cnt=dirPtr->Count();
			for(TInt loop=0;loop<cnt;loop++) {
				TEntry fileentry=(*dirPtr)[loop];
				nameBuf.Copy(fileentry.iName);
				SymbianFilesystemNode entry(false);
				entry._isPseudoRoot = false;
				
				entry._displayName =(char*)nameBuf.PtrZ();
				entry._path = _path;
				entry._path +=(char*)nameBuf.PtrZ();
				entry._isDirectory = fileentry.IsDir();
				
				// Honor the chosen mode
				if ((mode == FilesystemNode::kListFilesOnly && entry._isDirectory) ||
					(mode == FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
					continue;
				
				if (entry._isDirectory)
					entry._path += "\\";
				myList.push_back(new SymbianFilesystemNode(entry));
			}
			CleanupStack::PopAndDestroy(dirPtr);
		}

	}
	return true;
}

AbstractFilesystemNode *SymbianFilesystemNode::parent() const {
	SymbianFilesystemNode *p =NULL;

	// Root node is its own parent. Still we can't just return this
	// as the GUI code will call delete on the old node.
	if (!_isPseudoRoot && _path.size() > 3) {
		p=new SymbianFilesystemNode(false);
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p->_path = String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
	}
	else
	{
		p=new SymbianFilesystemNode(true);
	}
	return p;
}

AbstractFilesystemNode *SymbianFilesystemNode::child(const String &name) const {
	assert(_isDirectory);
	String newPath(_path);

	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += name;

	TPtrC8 ptr((const unsigned char*) newPath.c_str(), newPath.size());
	TFileName fname;
	fname.Copy(ptr);
	TBool isFolder = EFalse;
	BaflUtils::IsFolder(CEikonEnv::Static()->FsSession(), fname, isFolder);
	if(!isFolder)
		return 0;

	SymbianFilesystemNode *p = new SymbianFilesystemNode(newPath);
	return p;
}

#endif // defined(__SYMBIAN32__)
