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
 * $URL$
 * $Id$
 */

#if defined (__SYMBIAN32__)
#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"
#include "backends/platform/symbian/src/SymbianOS.h"

#include <dirent.h>
#include <eikenv.h>
#include <f32file.h>
#include <bautils.h>

/**
 * Implementation of the ScummVM file system API based on POSIX.
 *
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class SymbianFilesystemNode : public AbstractFilesystemNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;

public:
	/**
	 * Creates a SymbianFilesystemNode with the root node as path.
	 *
	 * @param aIsRoot true if the node will be a pseudo root, false otherwise.
	 */
	SymbianFilesystemNode(bool aIsRoot);

	/**
	 * Creates a SymbianFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	SymbianFilesystemNode(const Common::String &path);

	virtual bool exists() const {
		TFileName fname;
		TPtrC8 ptr((const unsigned char*)_path.c_str(),_path.size());
		fname.Copy(ptr);
		TBool fileExists = BaflUtils::FileExists(static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession(), fname);
		return fileExists;
	}
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return access(_path.c_str(), R_OK) == 0; }	//FIXME: this is just a stub
	virtual bool isWritable() const { return access(_path.c_str(), W_OK) == 0; }	//FIXME: this is just a stub

	virtual AbstractFilesystemNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFilesystemNode *getParent() const;

	virtual Common::SeekableReadStream *openForReading();
	virtual Common::WriteStream *openForWriting();
};

/**
 * Fixes the path by changing all slashes to backslashes.
 *
 * @param path Common::String with the path to be fixed.
 */
static void fixFilePath(Common::String& aPath){
	TInt len = aPath.size();

	for (TInt index = 0; index < len; index++) {
		if (aPath[index] == '/') {
			aPath.setChar('\\', index);
		}
	}
}

SymbianFilesystemNode::SymbianFilesystemNode(bool aIsRoot) {
	_path = "";
	_isValid = true;
	_isDirectory = true;
	_isPseudoRoot = aIsRoot;
	_displayName = "Root";

}

SymbianFilesystemNode::SymbianFilesystemNode(const Common::String &path) {
	if (path.size() == 0)
		_isPseudoRoot = true;
	else
		_isPseudoRoot = false;

	_path = path;

	fixFilePath(_path);

	_displayName = lastPathComponent(_path, '\\');

	TEntry fileAttribs;
	TFileName fname;
	TPtrC8 ptr((const unsigned char*)_path.c_str(),_path.size());
	fname.Copy(ptr);

	if (static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession().Entry(fname, fileAttribs) == KErrNone) {
		_isValid = true;
		_isDirectory = fileAttribs.IsDir();
	} else {
		_isValid = false;
		_isDirectory = false;
	}
}

AbstractFilesystemNode *SymbianFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);
	Common::String newPath(_path);

	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	TPtrC8 ptr((const unsigned char*) newPath.c_str(), newPath.size());
	TFileName fname;
	fname.Copy(ptr);
	TBool isFolder = EFalse;
	BaflUtils::IsFolder(static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession(), fname, isFolder);
	if (!isFolder)
		return 0;

	return new SymbianFilesystemNode(newPath);
}

bool SymbianFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	//TODO: honor the hidden flag

	if (_isPseudoRoot) {
		// Drives enumeration
		RFs& fs = static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession();
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
			if (fs.DriveToChar(driveNumber,driveLetter) != KErrNone)
				continue;

			driveLetterValue = driveLetter;

			if (volumeInfo.iName.Length() > 0) {
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
		if (static_cast<OSystem_SDL_Symbian*>(g_system)->FsSession().GetDir(fname,KEntryAttNormal|KEntryAttDir,0,dirPtr)==KErrNone) {
			CleanupStack::PushL(dirPtr);
			TInt cnt=dirPtr->Count();
			for (TInt loop=0;loop<cnt;loop++) {
				TEntry fileentry=(*dirPtr)[loop];
				nameBuf.Copy(fileentry.iName);
				SymbianFilesystemNode entry(false);
				entry._isPseudoRoot = false;

				entry._displayName =(char*)nameBuf.PtrZ();
				entry._path = _path;
				entry._path +=(char*)nameBuf.PtrZ();
				entry._isDirectory = fileentry.IsDir();

				// Honor the chosen mode
				if ((mode == Common::FilesystemNode::kListFilesOnly && entry._isDirectory) ||
					(mode == Common::FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
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

AbstractFilesystemNode *SymbianFilesystemNode::getParent() const {
	SymbianFilesystemNode *p =NULL;

	// Root node is its own parent. Still we can't just return this
	// as the GUI code will call delete on the old node.
	if (!_isPseudoRoot && _path.size() > 3) {
		p = new SymbianFilesystemNode(false);
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path, '\\');

		p->_path = Common::String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path, '\\');
	}
	else
	{
		p = new SymbianFilesystemNode(true);
	}

	return p;
}

Common::SeekableReadStream *SymbianFilesystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *SymbianFilesystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#if defined (__SYMBIAN32__)
