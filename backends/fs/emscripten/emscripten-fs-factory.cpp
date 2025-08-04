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

#ifdef EMSCRIPTEN

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#include "backends/fs/emscripten/emscripten-fs-factory.h"
#include "backends/fs/emscripten/emscripten-posix-fs.h"
#include "backends/fs/emscripten/http-fs.h"
#include "common/debug.h"
#ifdef USE_CLOUD
#include "backends/fs/emscripten/cloud-fs.h"
#endif

#include <emscripten.h>

EM_ASYNC_JS(void, _initSettings, (const char *pathPtr), {
	try {
		const path = UTF8ToString(pathPtr);
		const settingsPath = path + "/scummvm.ini";
		
		// Mount the filesystem
		FS.mount(IDBFS, { autoPersist: true }, path);
		
		// Sync the filesystem
		await new Promise((resolve, reject) => {
			FS.syncfs(true, (err) => err ? reject(err) : resolve());
		});
		
		// Check if settings file exists and download if needed
		if (!FS.analyzePath(settingsPath).exists) {
			const response = await fetch("scummvm.ini");
			if (response.ok) {
				const text = await response.text();
				FS.writeFile(settingsPath, text);
			}
		}
	} catch (err) {
		console.error("Error initializing files:", err);
		alert("Error initializing files: " + err);
		throw err;
	}
});

EmscriptenFilesystemFactory::EmscriptenFilesystemFactory() {
	_initSettings(getenv("HOME"));
	_httpNodes = new Common::HashMap<Common::String, HTTPFilesystemNode *>();
}

AbstractFSNode *EmscriptenFilesystemFactory::makeCurrentDirectoryFileNode() const {
	// getcwd() defaults to root on emscripten and ScummVM doesn't use setcwd()
	return makeRootFileNode();
}

AbstractFSNode *EmscriptenFilesystemFactory::makeRootFileNode() const {
	return new EmscriptenPOSIXFilesystemNode("/");
}

AbstractFSNode *EmscriptenFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	debug(5, "EmscriptenFilesystemFactory::makeFileNodePath(%s)", path.c_str());
	assert(!path.empty());
	if (path.hasPrefix(DATA_PATH)) {
		if (!_httpNodes->contains(path)) {
			// finding a node by path requires a http request to the server, so we cache the nodes
			_httpNodes->setVal(path, new HTTPFilesystemNode(path));
		}
		return new HTTPFilesystemNode(*(_httpNodes->getVal(path)));
#ifdef USE_CLOUD
	} else if (path.hasPrefix(CLOUD_FS_PATH) && CloudMan.isStorageEnabled()) {
		return new CloudFilesystemNode(path);
#endif
	} else {
		return new EmscriptenPOSIXFilesystemNode(path);
	}
}
#endif
