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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "backends/fs/kolibrios/kolibrios-fs.h"
#include "backends/fs/posix/posix-iostream.h"
#include "common/algorithm.h"
#include "common/debug.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ksys.h>

#include "common/pack-start.h"	// START STRUCT PACKING

struct kol_readdir_result_entry {
	ksys_file_info_t bdfe;
	char fileName[520];
} PACKED_STRUCT;

struct kol_readdir_result {
	ksys_dir_entry_header_t header;
	kol_readdir_result_entry entries[0];
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

namespace {

// opendir/readdir are buggy. And they encourage using syscalls. Whatever
kol_readdir_result *kol_readdir(const char *path, uint32 max_blocks) {
	ksys_file_status_t ret_result;
	int result_buffer_size = sizeof (ksys_dir_entry_header_t)
		+ max_blocks * sizeof (kol_readdir_result_entry);
	kol_readdir_result *result_buffer = (kol_readdir_result *) malloc (result_buffer_size);
	if (!result_buffer)
		return nullptr;
	memset(result_buffer, 0, result_buffer_size);

	ret_result = _ksys_file_read_dir(path, 0, KSYS_FILE_UTF8, max_blocks, result_buffer);

	// KSYS_FS_ERR_SUCCESS (0) is returned for normal dirs, KSYS_FS_ERR_EOF (6) for virtual directories
	if (ret_result.status != 0 && ret_result.status != KSYS_FS_ERR_EOF) {
		free (result_buffer);
		return nullptr;
	}

	return result_buffer;
}

kol_readdir_result *kol_readdir(const char *path) {
	kol_readdir_result *res = kol_readdir(path, 2);

	if (!res)
		return nullptr;

	uint32 tot_blocks = res->header.files;
	free(res);

	return kol_readdir(path, tot_blocks);
}

bool getFileAttrs(Common::String path, uint32& attrs) {
	ksys_file_info_t info;

	memset(&info, 0, sizeof(info));
	info.attr = KSYS_FILE_ATTR_DIR;

	if(_ksys_file_info(path.c_str(), &info)) {
		attrs = 0;
		return false;
	}

	attrs = info.attr;
	return true;
}

} // namespace

bool KolibriOSFilesystemNode::exists() const {
	uint32 attrs;
	return getFileAttrs(_path, attrs);
}

bool KolibriOSFilesystemNode::isReadable() const {
	uint32 attrs;
	return getFileAttrs(_path, attrs);
}

bool KolibriOSFilesystemNode::isWritable() const {
	uint32 attrs;
	return getFileAttrs(_path, attrs) && !(attrs & 0x01);
}

void KolibriOSFilesystemNode::setFlags() {
	_isValid = getFileAttrs(_path, _attributes);
}

KolibriOSFilesystemNode::KolibriOSFilesystemNode(const Common::String &p) {
	assert(p.size() > 0);

	_path = p;

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');

	setFlags();
}

AbstractFSNode *KolibriOSFilesystemNode::getChild(const Common::String &n) const {
	assert(!_path.empty());
	assert(isDirectory());

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

bool KolibriOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	kol_readdir_result *res = kol_readdir(_path.c_str());

	if (!res)
		return false;

	for (int i = 0; i < (int) res->header.blocks; i++)
	{
		// Skip 'invisible' files if necessary
		if (res->entries[i].fileName[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((res->entries[i].fileName[0] == '.' && res->entries[i].fileName[1] == 0)
		    || (res->entries[i].fileName[0] == '.' && res->entries[i].fileName[1] == '.' && res->entries[i].fileName[2] == 0)) {
			continue;
		}

		// Start with a clone of this node, with the correct path set
		KolibriOSFilesystemNode entry(*this);
		entry._displayName = res->entries[i].fileName;
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;

		entry._isValid = true;
		entry._attributes = res->entries[i].bdfe.attr;

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry.isDirectory()) ||
		    (mode == Common::FSNode::kListDirectoriesOnly && !entry.isDirectory()))
			continue;

		myList.push_back(new KolibriOSFilesystemNode(entry));

	}

	free(res);

	return true;
}

AbstractFSNode *KolibriOSFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;	// The filesystem root has no parent

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end-1) != '/')
		end--;

	if (end == start) {
		// This only happens if we were called with a relative path, for which
		// there simply is no parent.
		// TODO: We could also resolve this by assuming that the parent is the
		//       current working directory, and returning a node referring to that.
		return 0;
	}

	return makeNode(Common::String(start, end));
}

Common::SeekableReadStream *KolibriOSFilesystemNode::createReadStream() {
	return PosixIoStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *KolibriOSFilesystemNode::createWriteStream(bool atomic) {
	return PosixIoStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);
}

bool KolibriOSFilesystemNode::createDirectory() {
	if (mkdir(_path.c_str(), 0755) == 0)
		setFlags();

	return isDirectory();
}

namespace KolibriOS {

bool assureDirectoryExists(const Common::String &dir, const char *prefix) {

	// Check whether the prefix exists if one is supplied.
	if (prefix) {
		uint32 attrs;
		if (!getFileAttrs(prefix, attrs) || !KolibriOSFilesystemNode::isDirectory(attrs)) {
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
	if (cur[0] == '/' && cur[1] >= 1 && cur[1] <= 3 && cur[2] == '/')
		cur += 2;
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

		if (mkdir(path.c_str(), 0755) != 0) {
			uint32 attrs;
			if (!getFileAttrs(path, attrs) || !KolibriOSFilesystemNode::isDirectory(attrs))
				return false;
		}

		*cur = '/';
	} while (cur++ != end);

	return true;
}

} // End of namespace KolibriOS
