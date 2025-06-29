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

#if defined(PSP2)

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir

#include "backends/fs/psp2/psp2-fs.h"
#include "backends/fs/psp2/psp2-stream.h"
#include "common/bufferedstream.h"
#include "common/debug.h"
#include "engines/engine.h"

#include <sys/stat.h>
#include <unistd.h>

#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>


PSP2FilesystemNode::PSP2FilesystemNode(const Common::String &path, const Config &config, bool verify) :
        _isPseudoRoot(false),
	    _config(config) {

    _path = path;
    _displayName = lastPathComponent(_path, '/');

    if (isDrive(path)) {
        _isDirectory = true;
        _isValid = true;

        if (!_path.hasSuffix("/")) {
			_path += "/";
		}

        debug(8, "path [%s]", _path.c_str());

        if (verify) {
            struct stat st;
            if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
                debug(8, "Suspended");
            _isValid = (0 == stat(_path.c_str(), &st));
            PowerMan.endCriticalSection();
            _isDirectory = S_ISDIR(st.st_mode);
        }
    }
}

PSP2FilesystemNode::PSP2FilesystemNode(const Config &config) :
		_isPseudoRoot(true),
		_config(config) {
	_isDirectory = true;
	_isValid = false;
}

bool PSP2FilesystemNode::exists() const {
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		debug(8, "Suspended");	// Make sure to block in case of suspend

	debug(8, "path [%s]", _path.c_str());

	ret = access(_path.c_str(), F_OK);
	PowerMan.endCriticalSection();

	return (ret == 0);
}

bool PSP2FilesystemNode::isReadable() const {
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		debug(8, "Suspended");	// Make sure to block in case of suspend

	debug(8, "path [%s]", _path.c_str());

	ret = access(_path.c_str(), R_OK);
	PowerMan.endCriticalSection();

	return (ret == 0);
}

bool PSP2FilesystemNode::isWritable() const {
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		debug(8, "Suspended");	// Make sure to block in case of suspend

	debug(8, "path [%s]", _path.c_str());

	ret = access(_path.c_str(), W_OK);
	PowerMan.endCriticalSection();

	return ret == 0;
}


AbstractFSNode *PSP2FilesystemNode::getChild(const Common::String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	debug(8, "child [%s]", newPath.c_str());

	AbstractFSNode *node = new PSP2FilesystemNode(newPath, _config, true);

	return node;
}

bool PSP2FilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	//TODO: honor the hidden flag

	bool ret = true;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		debug(8, "Suspended");	// Make sure to block in case of suspend

	debug(8, "Current path[%s]", _path.c_str());

	int dfd  = sceIoDopen(_path.c_str());
	if (dfd > 0) {
		SceIoDirent dir;
		memset(&dir, 0, sizeof(dir));

		while (sceIoDread(dfd, &dir) > 0) {
			// Skip 'invisible files
			if (dir.d_name[0] == '.')
				continue;

			PSP2FilesystemNode entry(_config);

			entry._isValid = true;
			entry._displayName = dir.d_name;

			Common::String newPath(_path);
			if (newPath.lastChar() != '/')
				newPath += '/';
			newPath += dir.d_name;

			entry._path = newPath;
			entry._isDirectory = dir.d_stat.st_attr & SCE_SO_IFDIR;

			debug(8, "Child[%s], %s", entry._path.c_str(), entry._isDirectory ? "dir" : "file");

			// Honor the chosen mode
			if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			        (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
				continue;

			myList.push_back(new PSP2FilesystemNode(entry));
		}

		sceIoDclose(dfd);
		ret = true;
	} else { // dfd <= 0
		ret = false;
	}

	PowerMan.endCriticalSection();

	return ret;
}

AbstractFSNode *PSP2FilesystemNode::getParent() const {
	if (_isPseudoRoot) {
        return nullptr;
    }

    if (isDrive(_path)) {
        return makeNode();
    }

	debug(8, "current[%s]", _path.c_str());

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	AbstractFSNode *node = new PSP2FilesystemNode(Common::String(start, end - start), _config, false);

	return node;
}

Common::SeekableReadStream *PSP2FilesystemNode::createReadStream() {
	const uint32 READ_BUFFER_SIZE = 1024;

	Common::SeekableReadStream *stream = Psp2IoStream::makeFromPath(getPath(), false);

	return Common::wrapBufferedSeekableReadStream(stream, READ_BUFFER_SIZE, DisposeAfterUse::YES);
}

Common::SeekableWriteStream *PSP2FilesystemNode::createWriteStream(bool atomic) {
	const uint32 WRITE_BUFFER_SIZE = 1024;

	// TODO: Add atomic support if possible
	Common::SeekableWriteStream *stream = Psp2IoStream::makeFromPath(getPath(), true);

	return Common::wrapBufferedWriteStream(stream, WRITE_BUFFER_SIZE);
}

bool PSP2FilesystemNode::createDirectory() {
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		debug(8, "Suspended");	// Make sure to block in case of suspend

	debug(8, "path [%s]", _path.c_str());

	if (sceIoMkdir(_path.c_str(), 0777) == 0) {
		struct stat st;
		_isValid = (0 == stat(_path.c_str(), &st));
		_isDirectory = S_ISDIR(st.st_mode);
	}

	PowerMan.endCriticalSection();

	return _isValid && _isDirectory;
}

bool PSP2FilesystemNode::isDrive(const Common::String &path) const {
	Common::String normalizedPath = Common::normalizePath(path, '/');
	return _config.isDrive(normalizedPath);
}

#endif //#ifdef PSP2
