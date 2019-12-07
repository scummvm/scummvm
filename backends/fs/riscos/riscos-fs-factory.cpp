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

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(RISCOS)

#include "backends/fs/riscos/riscos-fs-factory.h"
#include "backends/fs/riscos/riscos-fs.h"

#include <unistd.h>

AbstractFSNode *RISCOSFilesystemFactory::makeRootFileNode() const {
	return new RISCOSFilesystemNode("/");
}

AbstractFSNode *RISCOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	return getcwd(buf, MAXPATHLEN) ? new RISCOSFilesystemNode(buf) : NULL;
}

AbstractFSNode *RISCOSFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new RISCOSFilesystemNode(path);
}
#endif
