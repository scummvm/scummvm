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

#if defined(PALMOS_MODE)

#include "PalmVersion.h"
#include "globals.h"

#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"

/**
 * Implementation of the ScummVM file system API based on PalmOS VFS API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class PalmOSFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;

public:
	/**
	 * Creates a PalmOSFilesystemNode with the root node as path.
	 */
	PalmOSFilesystemNode();

	/**
	 * Creates a POSIXFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	PalmOSFilesystemNode(const Common::String &p);

	virtual bool exists() const { return _isValid; }
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *openForReading();
	virtual Common::WriteStream *openForWriting();

private:
	/**
	 * Adds a single WindowsFilesystemNode to a given list.
	 * This method is used by getChildren() to populate the directory entries list.
	 *
	 * @param list List to put the file entry node in.
	 * @param mode Mode to use while adding the file entry to the list.
	 * @param base Common::String with the directory being listed.
	 * @param find_data Describes a file that the FindFirstFile, FindFirstFileEx, or FindNextFile functions find.
	 */
	static void addFile(AbstractFSList &list, ListMode mode, const Char *base, FileInfoType* find_data);
};

void PalmOSFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, FileInfoType* find_data) {
	PalmOSFilesystemNode entry;
	bool isDir;

	isDir = (find_data->attributes & vfsFileAttrDirectory);

	if ((!isDir && mode == Common::FSNode::kListDirectoriesOnly) ||
		(isDir && mode == Common::FSNode::kListFilesOnly))
		return;

	entry._isDirectory = isDir;
	entry._displayName = find_data->nameP;
	entry._path = base;
	entry._path += find_data->nameP;

	if (entry._isDirectory)
		entry._path += "/";

	entry._isValid = true;
	entry._isPseudoRoot = false;

	list.push_back(new PalmOSFilesystemNode(entry));
}

PalmOSFilesystemNode::PalmOSFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = "/";
	_isPseudoRoot = false;
}

PalmOSFilesystemNode::PalmOSFilesystemNode(const Common::String &p) {
	_path = p;
	_displayName = lastPathComponent(_path, '/');

	UInt32 attr;
	FileRef handle;
	Err error = VFSFileOpen(gVars->VFS.volRefNum, _path.c_str(), vfsModeRead, &handle);
	if (!error) {
		error = VFSFileGetAttributes(handle, &attr);
		VFSFileClose(handle);
	}

	if (error) {
		_isValid = false;
		_isDirectory = false;

	} else {
		_isValid = true;
		_isDirectory = (attr & vfsFileAttrDirectory);
	}
	_isPseudoRoot = false;
}

AbstractFSNode *PalmOSFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	FileRef handle;
	UInt32 attr;
	Err error = VFSFileOpen(gVars->VFS.volRefNum, newPath.c_str(), vfsModeRead, &handle);
	if (error)
		return 0;

	error = VFSFileGetAttributes(handle, &attr);
	VFSFileClose(handle);

	if (error || !(attr & vfsFileAttrDirectory))
		return 0;

	return new PalmOSFilesystemNode(newPath);
}

bool PalmOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	//TODO: honor the hidden flag

	Err error;
	Char nameP[256];
	FileInfoType desc;
	FileRef handle;
	UInt32 dirIterator = expIteratorStart;

	desc.nameP = nameP;
	desc.nameBufLen = 256;
	error = VFSFileOpen(gVars->VFS.volRefNum, _path.c_str(), vfsModeRead, &handle);

	if (error)
		return false;

	while (dirIterator != expIteratorStop) {
		error = VFSDirEntryEnumerate(handle, &dirIterator, &desc);
		if (!error) {
			addFile(myList, mode, _path.c_str(), &desc);
		}
	}

	VFSFileClose(handle);

	return true;
}

AbstractFSNode *PalmOSFilesystemNode::getParent() const {
	PalmOSFilesystemNode *p = 0;

	if (!_isPseudoRoot) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path, '/');

		p = new PalmOSFilesystemNode();
		p->_path = Common::String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path, '/');
		p->_isPseudoRoot =(p->_path == "/");
	}

	return p;
}

Common::SeekableReadStream *PalmOSFilesystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *PalmOSFilesystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif // PALMOS_MODE
