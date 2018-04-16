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

#include "backends/networking/sdl_net/handlers/filesbasehandler.h"
#include "backends/saves/default/default-saves.h"
#include "common/config-manager.h"
#include "common/system.h"

namespace Networking {

FilesBaseHandler::FilesBaseHandler() {}

FilesBaseHandler::~FilesBaseHandler() {}

Common::String FilesBaseHandler::parentPath(Common::String path) {
	if (path.size() && (path.lastChar() == '/' || path.lastChar() == '\\')) path.deleteLastChar();
	if (!path.empty()) {
		for (int i = path.size() - 1; i >= 0; --i)
			if (i == 0 || path[i] == '/' || path[i] == '\\') {
				path.erase(i);
				break;
			}
	}
	if (path.size() && path.lastChar() != '/' && path.lastChar() != '\\')
		path += '/';
	return path;
}

bool FilesBaseHandler::transformPath(Common::String &path, Common::String &prefixToRemove, Common::String &prefixToAdd, bool isDirectory) {
	// <path> is not empty, but could lack the trailing slash
	if (isDirectory && path.lastChar() != '/' && path.lastChar() != '\\')
		path += '/';

	if (path.hasPrefix("/root") && ConfMan.hasKey("rootpath", "cloud")) {
		prefixToAdd = "/root/";
		prefixToRemove = ConfMan.get("rootpath", "cloud");
		if (prefixToRemove.size() && prefixToRemove.lastChar() != '/' && prefixToRemove.lastChar() != '\\')
			prefixToRemove += '/';
		if (prefixToRemove == "/") prefixToRemove = "";
		path.erase(0, 5);
		if (path.size() && (path[0] == '/' || path[0] == '\\'))
			path.deleteChar(0); // if that was "/root/ab/c", it becomes "/ab/c", but we need "ab/c"
		path = prefixToRemove + path;
		if (path == "")
			path = "/"; // absolute root is '/'
		return true;
	}

	if (path.hasPrefix("/saves")) {
		prefixToAdd = "/saves/";

		// determine savepath (prefix to remove)
#ifdef USE_LIBCURL
		DefaultSaveFileManager *manager = dynamic_cast<DefaultSaveFileManager *>(g_system->getSavefileManager());
		prefixToRemove = (manager ? manager->concatWithSavesPath("") : ConfMan.get("savepath"));
#else
		prefixToRemove = ConfMan.get("savepath");
#endif
		if (prefixToRemove.size() && prefixToRemove.lastChar() != '/' && prefixToRemove.lastChar() != '\\')
			prefixToRemove += '/';

		path.erase(0, 6);
		if (path.size() && (path[0] == '/' || path[0] == '\\'))
			path.deleteChar(0);
		path = prefixToRemove + path;
		return true;
	}

	return false;
}

} // End of namespace Networking
