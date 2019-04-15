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
 */

#if defined(POSIX)

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit		//Needed for IRIX's unistd.h
#define FORBIDDEN_SYMBOL_EXCEPTION_random
#define FORBIDDEN_SYMBOL_EXCEPTION_srandom

#include <unistd.h>

#include "backends/fs/chroot/chroot-fs-factory.h"
#include "backends/fs/chroot/chroot-fs.h"

ChRootFilesystemFactory::ChRootFilesystemFactory(const Common::String &root)
    : _root(root) {
}

AbstractFSNode *ChRootFilesystemFactory::makeRootFileNode() const {
	return new ChRootFilesystemNode(_root, "/");
}

AbstractFSNode *ChRootFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	if (getcwd(buf, MAXPATHLEN) == NULL) {
		return NULL;
	}

	if (Common::String(buf).hasPrefix(_root + Common::String("/"))) {
		return new ChRootFilesystemNode(_root, buf + _root.size());
	}

	return new ChRootFilesystemNode(_root, "/");
}

AbstractFSNode *ChRootFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new ChRootFilesystemNode(_root, path);
}

#endif
