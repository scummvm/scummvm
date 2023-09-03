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

#include "backends/networking/sdl_net/handlers/filesbasehandler.h"
#include "backends/saves/default/default-saves.h"
#include "common/config-manager.h"
#include "common/system.h"

namespace Networking {

FilesBaseHandler::FilesBaseHandler() {}

FilesBaseHandler::~FilesBaseHandler() {}

Common::String FilesBaseHandler::parentPath(const Common::String &path) {
	Common::String result = path;
	if (result.size() && (result.lastChar() == '/' || result.lastChar() == '\\')) result.deleteLastChar();
	if (!result.empty()) {
		for (int i = result.size() - 1; i >= 0; --i)
			if (i == 0 || result[i] == '/' || result[i] == '\\') {
				result.erase(i);
				break;
			}
	}
	if (result.size() && result.lastChar() != '/' && result.lastChar() != '\\')
		result += '/';
	return result;
}

bool FilesBaseHandler::urlToPath(Common::String &url, Common::Path &path, Common::String &baseUrl, Common::Path &basePath, bool isDirectory) {
	// <url> is not empty, but could lack the trailing slash
	if (isDirectory && url.lastChar() != '/')
		url += '/';

	if (url.hasPrefix("/root") && ConfMan.hasKey("rootpath", "cloud")) {
		baseUrl = "/root/";
		basePath = ConfMan.getPath("rootpath", "cloud");
		url.erase(0, 5);
		if (url.size() && url[0] == '/')
			url.deleteChar(0); // if that was "/root/ab/c", it becomes "/ab/c", but we need "ab/c"
		path = basePath.join(url, '/');
		return true;
	}

	if (url.hasPrefix("/saves")) {
		baseUrl = "/saves/";

		// determine savepath (prefix to remove)
#ifdef USE_LIBCURL
		DefaultSaveFileManager *manager = dynamic_cast<DefaultSaveFileManager *>(g_system->getSavefileManager());
		basePath = (manager ? manager->concatWithSavesPath("") : ConfMan.getPath("savepath"));
#else
		basePath = ConfMan.getPath("savepath");
#endif
		url.erase(0, 6);
		if (url.size() && url[0] == '/')
			url.deleteChar(0);
		path = basePath.join(url, '/');
		return true;
	}

	return false;
}

} // End of namespace Networking
