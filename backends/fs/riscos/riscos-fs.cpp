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
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(RISCOS)

#include "backends/platform/sdl/riscos/riscos-utils.h"
#include "backends/fs/riscos/riscos-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/algorithm.h"

// TODO: Replace use of access()
#include <unistd.h>

#include <kernel.h>
#include <swis.h>

bool RISCOSFilesystemNode::exists() const {
	return access(_path.c_str(), F_OK) == 0;
}

bool RISCOSFilesystemNode::isReadable() const {
	return access(_path.c_str(), R_OK) == 0;
}

bool RISCOSFilesystemNode::isWritable() const {
	return access(_path.c_str(), W_OK) == 0;
}

void RISCOSFilesystemNode::setFlags() {
	_kernel_swi_regs regs;
	regs.r[0] = 23;
	regs.r[1] = (int)_nativePath.c_str();
	_kernel_swi(OS_File, &regs, &regs);

	if (regs.r[0] == 0) {
		_isDirectory = false;
		_isValid = false;
	} else if (regs.r[0] == 2) {
		_isDirectory = true;
		_isValid = true;
	} else {
		_isDirectory = false;
		_isValid = true;
	}
}

RISCOSFilesystemNode::RISCOSFilesystemNode(const Common::String &p) {
	_path = p;
	if (p == "/") {
		_nativePath = "";
		_isDirectory = true;
		_isValid = true;
	} else {
		_nativePath = RISCOS_Utils::toRISCOS(p);
		setFlags();
	}
}

AbstractFSNode *RISCOSFilesystemNode::getChild(const Common::String &n) const {
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

bool RISCOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	if (_path == "/") {
		// Special case for the root dir: List all drives
		char fsname[MAXPATHLEN] = "";
		for (int fsNum = 0; fsNum < 256; fsNum += 1) {
			if (fsNum == 33 || fsNum == 46 || fsNum == 53 || fsNum == 99)
				continue;

			_kernel_swi_regs regs;
			regs.r[0] = 33;
			regs.r[1] = fsNum;
			regs.r[2] = (int)fsname;
			regs.r[3] = sizeof(fsname);
			_kernel_swi(OS_FSControl, &regs, &regs);
			if (fsname[0] == 0)
				continue;

			int drives = (fsNum == 193) ? 23 : 9;

			for (int discnum = 0; discnum <= drives; discnum += 1) {
				const Common::String path = Common::String::format("%s::%d.$", fsname, discnum);
				char outpath[MAXPATHLEN] = "";
				regs.r[0] = 37;
				regs.r[1] = (int)path.c_str();
				regs.r[2] = (int)outpath;
				regs.r[3] = 0;
				regs.r[4] = 0;
				regs.r[5] = sizeof(outpath);
				if (_kernel_swi(OS_FSControl, &regs, &regs) != NULL)
					continue;

				RISCOSFilesystemNode *entry = new RISCOSFilesystemNode();
				entry->_nativePath = outpath;
				entry->_path = Common::String('/') + outpath;
				entry->_displayName = outpath;
				entry->setFlags();
				if (entry->_isDirectory)
					myList.push_back(entry);
			}
		}
		return true;
	}

	char file[MAXPATHLEN];
	_kernel_swi_regs regs;
	regs.r[0] = 9;
	regs.r[1] = (int)_nativePath.c_str();
	regs.r[2] = (int)file;
	regs.r[3] = 1;
	regs.r[4] = 0;
	regs.r[5] = sizeof(file);
	regs.r[6] = 0;
	while (regs.r[4] != -1) {
		_kernel_swi(OS_GBPB, &regs, &regs);

		if (regs.r[4] == -1)
			break;

		// Start with a clone of this node, with the correct path set
		RISCOSFilesystemNode entry(*this);
		entry._displayName = file;
		entry._displayName = RISCOS_Utils::toUnix(entry._displayName);
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;
		entry._nativePath = RISCOS_Utils::toRISCOS(entry._path);
		entry.setFlags();
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			(mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		myList.push_back(new RISCOSFilesystemNode(entry));
	}

	return true;
}

AbstractFSNode *RISCOSFilesystemNode::getParent() const {
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

	if (*(end-1) == '/' && end != start + 1)
		end--;

	return makeNode(Common::String(start, end));
}

Common::SeekableReadStream *RISCOSFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *RISCOSFilesystemNode::createWriteStream(bool atomic) {
	return StdioStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);
}

bool RISCOSFilesystemNode::createDirectory() {
	_kernel_swi_regs regs;
	regs.r[0] = 8;
	regs.r[1] = (int)_nativePath.c_str();
	if (_kernel_swi(OS_File, &regs, &regs) == NULL)
		setFlags();

	return _isValid && _isDirectory;
}

namespace Riscos {

bool assureDirectoryExists(const Common::String &dir, const char *prefix) {
	AbstractFSNode *node;

	// Check whether the prefix exists if one is supplied.
	if (prefix) {
		node = new RISCOSFilesystemNode(prefix);
		if (!node->isDirectory()) {
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

		node = new RISCOSFilesystemNode(path);
		if (!node->createDirectory()) {
			if (node->exists()) {
				if (!node->isDirectory()) {
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

} // End of namespace RISCOS

#endif //#if defined(RISCOS)
