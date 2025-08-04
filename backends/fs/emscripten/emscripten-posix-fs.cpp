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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#include <stdio.h>

#include "backends/fs/emscripten/emscripten-fs-factory.h"
#include "backends/fs/emscripten/emscripten-posix-fs.h"
#include "backends/fs/emscripten/http-fs.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/posix/posix-iostream.h"
#include "common/system.h"

#ifdef USE_CLOUD
#include "backends/cloud/cloudmanager.h"
#include "backends/fs/emscripten/cloud-fs.h"
#endif

AbstractFSNode *EmscriptenPOSIXFilesystemNode::makeNode(const Common::String &path) const {
	return g_system->getFilesystemFactory()->makeFileNodePath(path);
}

EmscriptenPOSIXFilesystemNode::EmscriptenPOSIXFilesystemNode(const Common::String &path) : POSIXFilesystemNode(path) {}
EmscriptenPOSIXFilesystemNode::EmscriptenPOSIXFilesystemNode() : POSIXFilesystemNode() {}

bool EmscriptenPOSIXFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {

	if (_path == "/") {
		HTTPFilesystemNode *data_entry = new HTTPFilesystemNode(DATA_PATH);
		myList.push_back(data_entry);

#ifdef USE_CLOUD
		if (CloudMan.isStorageEnabled()) {
			CloudFilesystemNode *cloud_entry = new CloudFilesystemNode(CLOUD_FS_PATH);
			myList.push_back(cloud_entry);
		}
#endif
	}
	return POSIXFilesystemNode::getChildren(myList, mode, hidden);
}
