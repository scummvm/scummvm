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

#ifdef __LIBRETRO__

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit // Needed for IRIX's unistd.h

#include "backends/platform/libretro/include/libretro-fs-factory.h"
#include "backends/platform/libretro/include/libretro-fs.h"

AbstractFSNode *LibRetroFilesystemFactory::makeRootFileNode() const {
	return new LibRetroFilesystemNode("/");
}

AbstractFSNode *LibRetroFilesystemFactory::makeCurrentDirectoryFileNode() const {
#ifdef PLAYSTATION3
	return new LibRetroFilesystemNode("/");
#else
	char *cwd = getcwd(NULL, 0);
	AbstractFSNode *node = cwd ? new LibRetroFilesystemNode(Common::String(cwd)) : NULL;
	if (cwd)
		free(cwd);
	return node;
#endif
}

AbstractFSNode *LibRetroFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new LibRetroFilesystemNode(path);
}
#endif
