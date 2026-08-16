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

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_strstr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit // Needed for IRIX's unistd.h

#include <libretro.h>
#include <file/file_path.h>
#include <retro_dirent.h>
#include <streams/file_stream.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backends/platform/libretro/include/libretro-fs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "common/algorithm.h"
#include "common/array.h"
#include "common/stream.h"
#include "common/config-manager.h"

static const char *kLibRetroAuthorizedRootPath = "libretro-authorized:///";
static const char *kLibRetroAuthorizedRootLabel = "Authorized storage";

struct LibRetroAuthorizedLocation {
	Common::String path;
	Common::String label;
};

static Common::Array<LibRetroAuthorizedLocation> s_libretroAuthorizedLocations;

static Common::String libretroFsStripTrailingSlash(Common::String path) {
	while (path.size() > 1 && path.lastChar() == '/')
		path.erase(path.size() - 1);

	return path;
}

static bool libretroFsSamePath(const Common::String &a, const Common::String &b) {
	return libretroFsStripTrailingSlash(a).equals(libretroFsStripTrailingSlash(b));
}

static bool libretroFsIsAuthorizedRoot(const Common::String &path) {
	Common::String normalizedPath = libretroFsStripTrailingSlash(path);

	for (uint i = 0; i < s_libretroAuthorizedLocations.size(); ++i) {
		if (normalizedPath.equals(libretroFsStripTrailingSlash(s_libretroAuthorizedLocations[i].path)))
			return true;
	}

	return false;
}

static bool libretroFsIsInsideAuthorizedLocation(const Common::String &path) {
	Common::String normalizedPath = libretroFsStripTrailingSlash(path);

	for (uint i = 0; i < s_libretroAuthorizedLocations.size(); ++i) {
		Common::String root = libretroFsStripTrailingSlash(s_libretroAuthorizedLocations[i].path);

		if (normalizedPath.equals(root))
			return true;

		if (normalizedPath.hasPrefix(root) && normalizedPath.size() > root.size() && normalizedPath[root.size()] == '/')
			return true;
	}

	return false;
}

static Common::String libretroFsPosixDefaultDir();

static bool libretroFsHasUriScheme(const Common::String &path) {
	return strstr(path.c_str(), "://") != nullptr;
}

static Common::String libretroFsPercentDecode(const Common::String &s) {
	Common::String out;
	for (uint i = 0; i < s.size(); ++i) {
		if (s[i] == '%' && i + 2 < s.size()) {
			char h1 = s[i + 1], h2 = s[i + 2];
			int v1 = (h1 >= '0' && h1 <= '9') ? h1 - '0' : (h1 >= 'A' && h1 <= 'F') ? h1 - 'A' + 10 : (h1 >= 'a' && h1 <= 'f') ? h1 - 'a' + 10 : -1;
			int v2 = (h2 >= '0' && h2 <= '9') ? h2 - '0' : (h2 >= 'A' && h2 <= 'F') ? h2 - 'A' + 10 : (h2 >= 'a' && h2 <= 'f') ? h2 - 'a' + 10 : -1;
			if (v1 >= 0 && v2 >= 0) {
				out += (char)((v1 << 4) | v2);
				i += 2;
				continue;
			}
		}
		out += s[i];
	}
	return out;
}

static Common::String libretroFsSafDisplayName(const Common::String &path) {
	Common::String decoded(path);
	// SAF paths are double-encoded (e.g. %252F). Decode up to twice to reveal real separators.
	for (int pass = 0; pass < 2; ++pass) {
		if (decoded.contains('%'))
			decoded = libretroFsPercentDecode(decoded);
		else
			break;
	}

	while (decoded.size() > 1 && decoded.lastChar() == '/')
		decoded.erase(decoded.size() - 1);

	// Take the part after the last '/'.
	size_t slash = decoded.findLastOf('/');
	if (slash != Common::String::npos)
		decoded = decoded.substr(slash + 1);

	// Take the part after the last ':' (e.g. 'primary:Games' -> 'Games').
	size_t colon = decoded.findLastOf(':');
	if (colon != Common::String::npos)
		decoded = decoded.substr(colon + 1);

	return decoded.empty() ? path : decoded;
}

static Common::String libretroFsUriDisplayName(const Common::String &path) {
	Common::String trimmed(path);

	while (trimmed.size() > 1 && trimmed.lastChar() == '/')
		trimmed.erase(trimmed.size() - 1);

	Common::String name = Common::lastPathComponent(trimmed, '/');
	return name.empty() ? trimmed : name;
}

static int libretroFsWhenceToVfs(int whence) {
	switch (whence) {
	case SEEK_SET:
		return RETRO_VFS_SEEK_POSITION_START;
	case SEEK_CUR:
		return RETRO_VFS_SEEK_POSITION_CURRENT;
	case SEEK_END:
		return RETRO_VFS_SEEK_POSITION_END;
	default:
		return whence;
	}
}

class LibRetroFileReadStream final : public Common::SeekableReadStream {
public:
	explicit LibRetroFileReadStream(RFILE *file)
		: _file(file), _size(file ? filestream_get_size(file) : -1),
		  _eos(false), _err(false) {
	}

	~LibRetroFileReadStream() override {
		close();
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		if (!_file || !dataPtr) {
			_err = true;
			return 0;
		}

		if (dataSize == 0)
			return 0;

		int64_t ret = filestream_read(_file, dataPtr, dataSize);

		if (ret < 0) {
			_err = true;
			return 0;
		}

		// Emulate fread()/feof() behaviour closely enough for ScummVM:
		// EOF becomes observable after a read cannot satisfy the requested size.
		if ((uint32)ret < dataSize)
			_eos = true;

		return (uint32)ret;
	}

	bool eos() const override {
		return _eos;
	}

	bool err() const override {
		return _err;
	}

	void clearErr() override {
		_eos = false;
		_err = false;
	}

	int64 pos() const override {
		if (!_file)
			return -1;

		int64_t ret = filestream_tell(_file);
		return ret < 0 ? -1 : ret;
	}

	int64 size() const override {
		if (!_file)
			return -1;

		return _size >= 0 ? _size : filestream_get_size(_file);
	}

	bool seek(int64 offs, int whence = SEEK_SET) override {
		if (!_file) {
			_err = true;
			return false;
		}

		int64_t ret = filestream_seek(_file, offs, libretroFsWhenceToVfs(whence));

		if (ret < 0) {
			_err = true;
			return false;
		}

		// ScummVM's stream contract says a successful seek clears EOF.
		_eos = false;
		return true;
	}

private:
	void close() {
		if (_file) {
			filestream_close(_file);
			_file = nullptr;
		}
	}

	RFILE *_file;
	int64 _size;
	bool _eos;
	bool _err;
};

class LibRetroFileWriteStream final : public Common::SeekableWriteStream {
public:
	explicit LibRetroFileWriteStream(RFILE *file) : _file(file), _err(false) {
	}

	~LibRetroFileWriteStream() override {
		close();
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		if (!_file || !dataPtr) {
			_err = true;
			return 0;
		}

		if (dataSize == 0)
			return 0;

		int64_t ret = filestream_write(_file, dataPtr, dataSize);

		if (ret < 0) {
			_err = true;
			return 0;
		}

		if ((uint32)ret < dataSize)
			_err = true;

		return (uint32)ret;
	}

	bool flush() override {
		if (!_file) {
			_err = true;
			return false;
		}

		if (filestream_flush(_file) != 0) {
			_err = true;
			return false;
		}

		return true;
	}

	bool err() const override {
		return _err;
	}

	void clearErr() override {
		_err = false;
	}

	void finalize() override {
		flush();
	}

	int64 pos() const override {
		if (!_file)
			return -1;

		int64_t ret = filestream_tell(_file);
		return ret < 0 ? -1 : ret;
	}

	int64 size() const override {
		if (!_file)
			return -1;

		return filestream_get_size(_file);
	}

	bool seek(int64 offs, int whence = SEEK_SET) override {
		if (!_file) {
			_err = true;
			return false;
		}

		if (filestream_seek(_file, offs, libretroFsWhenceToVfs(whence)) < 0) {
			_err = true;
			return false;
		}

		return true;
	}

private:
	void close() {
		if (_file) {
			flush();
			filestream_close(_file);
			_file = nullptr;
		}
	}

	RFILE *_file;
	bool _err;
};

void LibRetroFilesystemNode::setFlags() {
	const char *fspath = _path.c_str();

	// Keep all filesystem state queries on libretro-common VFS.
	// This is important on Android/SAF and also keeps POSIX paths consistent
	// with the same VFS path used later by filestream_open().
	_isValid = path_is_valid(fspath);
	_isDirectory = path_is_directory(fspath);

	// libretro-common exposes stat-style validity/directory queries here, but
	// no portable readability/writability probes. Treat valid paths as readable
	// for ScummVM's FSNode purposes and let createReadStream()/createWriteStream()
	// be the definitive open check.
	_isReadable = _isValid;
	_isWritable = _isValid;
}

LibRetroFilesystemNode::LibRetroFilesystemNode(const Common::String &p) {
	assert(p.size() > 0);

	if (p.equals(kLibRetroAuthorizedRootPath)) {
		_path = p;
		_displayName = kLibRetroAuthorizedRootLabel;
		_isValid = true;
		_isDirectory = true;
		_isReadable = true;
		_isWritable = false;
		return;
	}

	if (libretroFsHasUriScheme(p)) {
		_path = p;
		_displayName = libretroFsUriDisplayName(_path);
		setFlags();
		return;
	}

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/") || p.hasPrefix("~\\")) {
		Common::String homeDir = getHomeDir();
		if (homeDir.empty())
			homeDir = ".";

		// Skip over the tilda.  We know that p contains at least
		// two chars, so this is safe:
		_path = homeDir + (p.c_str() + 1);

	} else
		_path = p;

	char portable_path[_path.size() + 1];
	strcpy(portable_path, _path.c_str());
	pathname_make_slashes_portable(portable_path);

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(Common::String(portable_path), '/');
	_displayName = Common::lastPathComponent(_path, '/');
	setFlags();
}

AbstractFSNode *LibRetroFilesystemNode::getChild(const Common::String &n) const {
	assert(!_path.empty());
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	// We assume here that _path is already normalized (hence don't bother to call
	//  Common::normalizePath on the final path).
	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return makeNode(newPath);
}

bool LibRetroFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	if (_path.equals(kLibRetroAuthorizedRootPath)) {
		if (mode == Common::FSNode::kListFilesOnly)
			return true;

		for (uint i = 0; i < s_libretroAuthorizedLocations.size(); ++i) {
			LibRetroFilesystemNode *node = new LibRetroFilesystemNode(s_libretroAuthorizedLocations[i].path);

			if (!node->isDirectory()) {
				delete node;
				continue;
			}

			if (!s_libretroAuthorizedLocations[i].label.empty())
				node->_displayName = s_libretroAuthorizedLocations[i].label;

			myList.push_back(node);
		}

		return true;
	}

	struct RDIR *dirp = retro_opendir(_path.c_str());

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((retro_readdir(dirp))) {
		const char *d_name = retro_dirent_get_name(dirp);

		// Skip 'invisible' files if necessary
		if (d_name[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((d_name[0] == '.' && d_name[1] == 0) || (d_name[0] == '.' && d_name[1] == '.')) {
			continue;
		}

		// Start with a clone of this node, with the correct path set
		LibRetroFilesystemNode entry(*this);
		entry._displayName = d_name;
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;

		entry._isValid = true;
		entry._isDirectory = retro_dirent_is_dir(dirp, entry._path.c_str());
		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) || (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		myList.push_back(new LibRetroFilesystemNode(entry));
	}
	retro_closedir(dirp);

	return true;
}

AbstractFSNode *LibRetroFilesystemNode::getParent() const {
	if (_path == "/" || _path.equals(kLibRetroAuthorizedRootPath))
		return 0; // The filesystem root has no parent

	if (hasAuthorizedLocations() && libretroFsIsAuthorizedRoot(_path))
		return makeNode(kLibRetroAuthorizedRootPath);

	Common::String parentPath(_path);

	if (libretroFsHasUriScheme(parentPath)) {
		const char *pathStr = parentPath.c_str();
		const char *scheme = strstr(pathStr, "://");
		const uint schemeRootLen = scheme ? (uint)(scheme - pathStr) + 3 : 0;

		// For hierarchical URI paths, keep the authority root intact.
		// Example:
		//   smb://server/share/game -> smb://server/share/
		//   smb://server/share/     -> smb://server/
		//   smb://server/           -> /
		uint uriRootLen = schemeRootLen;
		if (schemeRootLen > 0) {
			const char *authorityEnd = strchr(pathStr + schemeRootLen, '/');
			if (authorityEnd)
				uriRootLen = (uint)(authorityEnd - pathStr) + 1;
			else
				uriRootLen = parentPath.size();
		}

		while (parentPath.size() > uriRootLen && parentPath.lastChar() == '/')
			parentPath.erase(parentPath.size() - 1);

		if (parentPath.size() <= uriRootLen) {
			if (hasAuthorizedLocations() && (parentPath.hasPrefix("saf://") || libretroFsIsInsideAuthorizedLocation(_path)))
				return makeNode(kLibRetroAuthorizedRootPath);

			return makeNode("/");
		}

		size_t pos = parentPath.findLastOf('/');
		if (pos == Common::String::npos || pos + 1 <= uriRootLen)
			parentPath = parentPath.substr(0, uriRootLen);
		else
			parentPath = parentPath.substr(0, pos + 1);

		AbstractFSNode *parent = makeNode(parentPath);

		if (parent && parent->isDirectory() == false) {
			delete parent;
			return 0;
		}

		return parent;
	}

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end - 1) != '/')
		end--;

	if (end == start) {
		return 0;
	}

	Common::String posixParentPath(start, end);

	AbstractFSNode *parent = makeNode(posixParentPath);

	if (parent->isDirectory() == false) {
		delete parent;
		return 0;
	}

	return parent;
}

Common::SeekableReadStream *LibRetroFilesystemNode::createReadStream() {
	RFILE *file = filestream_open(getPath().c_str(), RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (!file)
		return nullptr;
	return new LibRetroFileReadStream(file);
}

Common::SeekableWriteStream *LibRetroFilesystemNode::createWriteStream(bool atomic) {
	(void)atomic;
	RFILE *file = filestream_open(getPath().c_str(), RETRO_VFS_FILE_ACCESS_WRITE, RETRO_VFS_FILE_ACCESS_HINT_NONE);

	if (!file)
		return nullptr;

	return new LibRetroFileWriteStream(file);
}

bool LibRetroFilesystemNode::createDirectory() {
	if (path_mkdir(_path.c_str()))
		setFlags();

	return _isValid && _isDirectory;
}

namespace Posix {

bool assureDirectoryExists(const Common::String &dir, const char *prefix) {
	// Check whether the prefix exists if one is supplied.
	if (prefix) {
		if (!path_is_valid(prefix)) {
			return false;
		} else if (!path_is_directory(prefix)) {
			return false;
		}
	}

	// Obtain absolute path.
	Common::String path;
	if (prefix) {
		path = prefix;
		path += '/';
		path += dir;
	} else {
		path = dir;
	}

	path = Common::normalizePath(path, '/');

	const Common::String::iterator end = path.end();
	Common::String::iterator cur = path.begin();
	if (*cur == '/')
		++cur;

	do {
		if (cur + 1 != end) {
			if (*cur != '/') {
				continue;
			}

			// It is kind of ugly and against the purpose of Common::String to
			// insert 0s inside, but this is just for a local string and
			// simplifies the code a lot.
			*cur = '\0';
		}

		if (!path_mkdir(path.c_str())) {
			if (errno == EEXIST) {
				if (!path_is_valid(path.c_str())) {
					return false;
				} else if (!path_is_directory(path.c_str())) {
					return false;
				}
			} else {
				return false;
			}
		}

		*cur = '/';
	} while (cur++ != end);

	return true;
}

} // End of namespace Posix

Common::String LibRetroFilesystemNode::getHomeDir(void) {
	Common::String path;
	const char *home = nullptr;

#ifdef _WIN32
	const char *drv = getenv("HOMEDRIVE");
	const char *pth = getenv("HOMEPATH");
	if (drv && *drv && pth && *pth) {
		Common::String s = Common::String(drv);
		s += pth;
		return s;
	}
#else
	home = getenv("HOME");
#endif

	if (home && *home)
		path = home;

	return path;
}

Common::String LibRetroFilesystemNode::getAuthorizedRootPath(void) {
	return Common::String(kLibRetroAuthorizedRootPath);
}

void LibRetroFilesystemNode::clearAuthorizedLocations(void) {
	s_libretroAuthorizedLocations.clear();
}

void LibRetroFilesystemNode::addAuthorizedLocation(const Common::String &path, const Common::String &label) {
	if (path.empty())
		return;

	for (uint i = 0; i < s_libretroAuthorizedLocations.size(); ++i) {
		if (libretroFsSamePath(s_libretroAuthorizedLocations[i].path, path))
			return;
	}

	LibRetroFilesystemNode node(path);
	if (!node.isDirectory())
		return;

	LibRetroAuthorizedLocation location;
	location.path = path;

	// The frontend may pass a generic label (e.g. "Removable storage") for all
	// SAF trees; derive a readable name from the path instead.
	(void)label;
	location.label = libretroFsSafDisplayName(path);

	s_libretroAuthorizedLocations.push_back(location);
}

bool LibRetroFilesystemNode::hasAuthorizedLocations(void) {
	return !s_libretroAuthorizedLocations.empty();
}

static Common::String libretroFsPosixDefaultDir() {
	const char *browserStartDir = retro_get_file_browser_start_dir();
	if (browserStartDir && *browserStartDir) {
		Common::String path(browserStartDir);
		if (LibRetroFilesystemNode(path).isDirectory())
			return path;
	}

	Common::String homeDir(LibRetroFilesystemNode::getHomeDir());

	if (!homeDir.empty() && LibRetroFilesystemNode(homeDir).isDirectory())
		return homeDir;

	const char *systemDir = retro_get_system_dir();
	if (systemDir && *systemDir) {
		Common::String path(systemDir);
		if (LibRetroFilesystemNode(path).isDirectory())
			return path;
	}

	const char *saveDir = retro_get_save_dir();
	if (saveDir && *saveDir) {
		Common::String path(saveDir);
		if (LibRetroFilesystemNode(path).isDirectory())
			return path;
	}

	return Common::String("/");
}

bool LibRetroFilesystemNode::useAuthorizedRoot(void) {
	if (!hasAuthorizedLocations())
		return false;

	return retro_setting_get_browsing_mode_authorized();
}

bool LibRetroFilesystemNode::isBrowserLastPathCompatible(const Common::String &path) {
	if (path.empty())
		return false;

	bool isAuthorizedPath = path.equals(kLibRetroAuthorizedRootPath) || libretroFsIsInsideAuthorizedLocation(path);

	if (useAuthorizedRoot())
		return isAuthorizedPath;

	return !isAuthorizedPath;
}

Common::String LibRetroFilesystemNode::getDefaultDir(void) {
	if (useAuthorizedRoot())
		return getAuthorizedRootPath();

	return libretroFsPosixDefaultDir();
}
