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

#if defined(POSIX) || defined(PLAYSTATION3)

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit		//Needed for IRIX's unistd.h
#define FORBIDDEN_SYMBOL_EXCEPTION_random
#define FORBIDDEN_SYMBOL_EXCEPTION_srandom

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"

#include <unistd.h>

AbstractFSNode *POSIXFilesystemFactory::makeRootFileNode() const {
	return new POSIXFilesystemNode("/");
}

AbstractFSNode *POSIXFilesystemFactory::makeCurrentDirectoryFileNode() const {
#if defined(__ANDROID__)
	// Keep this here if we still want to maintain support for the Android SDL port, since this affects that too
	//
	// For Android it does not make sense to have "." in Search Manager as a current directory file node, so we skip it here
	// Otherwise this can potentially lead to a crash since, in Android getcwd() returns the root path "/"
	// and when SearchMan is used (eg. SearchSet::createReadStreamForMember) and it tries to search root path (and calls POSIXFilesystemNode::getChildren())
	// then a JNI call is made (JNI::getAllStorageLocations()) which leads to a crash if done from the wrong context -- and is also useless overhead as a call in that case.
	// This fixes the error case: Loading "Beneath A Steel Sky" with Adlib or FluidSynth audio once, exiting to Launcher via in-game ScummVM menu and re-launching the game.
	// Don't return NULL here, since it causes crash with other engines (eg. Blade Runner)
	// Returning '.' here will cause POSIXFilesystemNode::getChildren() to ignore it
	//
	// We also skip adding the '.' directory to SearchManager (in archive.cpp, SearchManager::clear())
	char buf[MAXPATHLEN] = {'.', '\0'};
	return new POSIXFilesystemNode(buf);
#else
	char buf[MAXPATHLEN];
	return getcwd(buf, MAXPATHLEN) ? new POSIXFilesystemNode(buf) : NULL;
#endif
}

AbstractFSNode *POSIXFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new POSIXFilesystemNode(path);
}
#endif
