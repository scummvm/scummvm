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

#ifdef __PSP__

#include "engines/engine.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/psp/psp-stream.h"

#include <sys/stat.h>
#include <unistd.h>

#include <pspkernel.h>

#define	ROOT_PATH	"ms0:/"

//#define __PSP_PRINT_TO_FILE__
//#define __PSP_DEBUG_FUNCS__ /* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */
#include "backends/platform/psp/trace.h"

/**
 * Implementation of the ScummVM file system API based on PSPSDK API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class PSPFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a PSPFilesystemNode with the root node as path.
	 */
	PSPFilesystemNode();

	/**
	 * Creates a PSPFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param verify true if the isValid and isDirectory flags should be verified during the construction.
	 */
	PSPFilesystemNode(const Common::String &p, bool verify = true);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
};

PSPFilesystemNode::PSPFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = ROOT_PATH;
}

PSPFilesystemNode::PSPFilesystemNode(const Common::String &p, bool verify) {
	DEBUG_ENTER_FUNC();
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path, '/');
	_isValid = true;
	_isDirectory = true;

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	if (verify) {
		struct stat st;
		if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
			PSP_DEBUG_PRINT_FUNC("Suspended\n");
		_isValid = (0 == stat(_path.c_str(), &st));
		PowerMan.endCriticalSection();
		_isDirectory = S_ISDIR(st.st_mode);
	}
	DEBUG_EXIT_FUNC();
}

bool PSPFilesystemNode::exists() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), F_OK);
	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return (ret == 0);
}

bool PSPFilesystemNode::isReadable() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), R_OK);
	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return (ret == 0);
}

bool PSPFilesystemNode::isWritable() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), W_OK);
	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return ret == 0;
}


AbstractFSNode *PSPFilesystemNode::getChild(const Common::String &n) const {
	DEBUG_ENTER_FUNC();
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	PSP_DEBUG_PRINT_FUNC("child [%s]\n", newPath.c_str());

	AbstractFSNode *node = new PSPFilesystemNode(newPath, true);

	DEBUG_EXIT_FUNC();
	return node;
}

bool PSPFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	DEBUG_ENTER_FUNC();
	assert(_isDirectory);

	//TODO: honor the hidden flag

	bool ret = true;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("Current path[%s]\n", _path.c_str());

	int dfd  = sceIoDopen(_path.c_str());
	if (dfd > 0) {
		SceIoDirent dir;
		memset(&dir, 0, sizeof(dir));

		while (sceIoDread(dfd, &dir) > 0) {
			// Skip 'invisible files
			if (dir.d_name[0] == '.')
				continue;

			PSPFilesystemNode entry;

			entry._isValid = true;
			entry._displayName = dir.d_name;

			Common::String newPath(_path);
			if (newPath.lastChar() != '/')
				newPath += '/';
			newPath += dir.d_name;

			entry._path = newPath;
			entry._isDirectory = dir.d_stat.st_attr & FIO_SO_IFDIR;

			PSP_DEBUG_PRINT_FUNC("Child[%s], %s\n", entry._path.c_str(), entry._isDirectory ? "dir" : "file");

			// Honor the chosen mode
			if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			        (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
				continue;

			myList.push_back(new PSPFilesystemNode(entry));
		}

		sceIoDclose(dfd);
		ret = true;
	} else { // dfd <= 0
		ret = false;
	}

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return ret;
}

AbstractFSNode *PSPFilesystemNode::getParent() const {
	DEBUG_ENTER_FUNC();
	if (_path == ROOT_PATH)
		return 0;

	PSP_DEBUG_PRINT_FUNC("current[%s]\n", _path.c_str());

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	AbstractFSNode *node = new PSPFilesystemNode(Common::String(start, end - start), false);
	DEBUG_EXIT_FUNC();
	return node;
}

Common::SeekableReadStream *PSPFilesystemNode::createReadStream() {
	return PSPIoStream::makeFromPath(getPath(), false);
}

Common::WriteStream *PSPFilesystemNode::createWriteStream() {
	return PSPIoStream::makeFromPath(getPath(), true);
}

#endif //#ifdef __PSP__
