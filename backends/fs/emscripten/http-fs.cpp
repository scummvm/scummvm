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
// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#ifdef EMSCRIPTEN

#include "backends/fs/emscripten/http-fs.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/posix/posix-iostream.h"
#include "common/debug.h"
#include "common/formats/json.h"
#include <emscripten.h>

HTTPFilesystemNode::HTTPFilesystemNode(const Common::String &path, const Common::String &displayName, const Common::String &baseUrl, bool isValid, bool isDirectory, int size) : _path(path), _displayName(displayName), _url(baseUrl), _isValid(isValid), _isDirectory(isDirectory), _size(size) {
	debug(5, "HTTPFilesystemNode::HTTPFilesystemNode(%s, %s)", path.c_str(), baseUrl.c_str());
	assert(path.size() > 0);
	assert(isDirectory || size >= 0 || !isValid);
	_children = new AbstractFSList();
}

HTTPFilesystemNode::HTTPFilesystemNode(const Common::String &p) : _path(p), _isValid(false), _isDirectory(false), _size(-1) {
	debug(5, "HTTPFilesystemNode::HTTPFilesystemNode(%s)", p.c_str());
	assert(p.size() > 0);
	_children = new AbstractFSList();

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');
	if (_path == DATA_PATH) { // need special case for handling the root of the http-filesystem
		_isDirectory = true;
		_isValid = true;
		_url = _path;
	} else { // we need to peek in the parent folder to see if the node exists and if it's a directory
		AbstractFSNode *parent = getParent();
		AbstractFSList tmp = AbstractFSList();
		parent->getChildren(tmp, Common::FSNode::kListAll, true);
		for (AbstractFSList::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			AbstractFSNode *child = *i;
			if (child->getPath() == _path) {
				_isDirectory = child->isDirectory();
				_isValid = true;
				_url = ((HTTPFilesystemNode *)child)->_url;
				_size = ((HTTPFilesystemNode *)child)->_size;
				break;
			}
		}
	}
	assert(_isDirectory || _size >= 0 || !_isValid);
	debug(5, "HTTPFilesystemNode::HTTPFilesystemNode(%s) -  %s isValid %s isDirectory %s", p.c_str(), _url.c_str(), _isValid ? "True" : "false", _isDirectory ? "True" : "false");
}

AbstractFSNode *HTTPFilesystemNode::getChild(const Common::String &n) const {
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

EM_ASYNC_JS(char *, _httpFsFetchIndex, (const char *url), {
	globalThis['httpFsIndexCache'] = globalThis['httpFsIndexCache'] || {};
	returnString = "";
	url = UTF8ToString(url);
	console.debug("Downloading %s", url);
	if (globalThis['httpFsIndexCache'][url]) {
		console.debug("Cache hit for %s", url);
		returnString = globalThis['httpFsIndexCache'][url];
	} else {
		try {
			const response = await fetch(url);
			if (response.ok) {
				returnString = await response.text();
				globalThis['httpFsIndexCache'][url] = returnString;
			}
		} catch (error) {
			console.error(error);
		}
	}
	var size = lengthBytesUTF8(returnString) + 1;
	var ret = Module._malloc(size);
	stringToUTF8Array(returnString, HEAP8, ret, size);
	return ret;
});

EM_ASYNC_JS(bool, _httpFsFetchFile, (const char *url, byte *dataPtr, int dataSize), {
	returnBytes = new Uint8Array();
	url = UTF8ToString(url);
	console.debug("Downloading %s", url);
	try {
		const response = await fetch(url);
		if (response.ok) {
			returnBytes = await response.bytes();
			if (returnBytes.length == dataSize) {
				Module.writeArrayToMemory(returnBytes, dataPtr);
				return true;
			}
		} else {
			console.error("HTTPFilesystemNode::_httpFsFetchFile: %s", response.statusText);
		}
	} catch (error) {
		console.error(error);
	}

	return false;
});

bool HTTPFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	if (!_isValid) {
		return false;
	}
	assert(_isDirectory);
	if (_children->size() == 0) {
		// if we don't have a children list yet, we need to fetch it from the server
		debug(5, "HTTPFilesystemNode::getChildren Fetching Children: %s at %s", _path.c_str(), _url.c_str());
		Common::String url = _url + "/index.json";
		char *response = _httpFsFetchIndex(url.c_str());
		if (strcmp(response, "") == 0) {
			return false;
		}

		Common::JSONObject jsonObj = Common::JSON::parse(response)->asObject();
		// add dummy element so we know that we fetched the list
		_children->push_back(new HTTPFilesystemNode(_path, ".", _url, false, false, 0));
		for (typename Common::HashMap<Common::String, Common::JSONValue *>::iterator i = jsonObj.begin(); i != jsonObj.end(); ++i) {
			Common::String name = i->_key;
			bool isDir = false;
			int size = -1;
			Common::String baseUrl = _url + "/" + name;

			if (i->_value->isObject()) {
				isDir = true;
				if (i->_value->asObject().contains("baseUrl")) {
					debug(5, "HTTPFilesystemNode::directoryListedCallback - Directory with baseUrl %s", name.c_str());
					baseUrl = i->_value->asObject()["baseUrl"]->asString();
				}
			} else if (i->_value->isIntegerNumber()) {
				size = i->_value->asIntegerNumber();
			}
			HTTPFilesystemNode *file_node = new HTTPFilesystemNode(_path + "/" + name, name, baseUrl, true, isDir, size);
			_children->push_back(file_node);
		}
	}
	for (AbstractFSList::iterator i = (*_children).begin(); i != (*_children).end(); ++i) {
		HTTPFilesystemNode *node = (HTTPFilesystemNode *)*i;

		if (node->_isValid && (mode == Common::FSNode::kListAll ||
							   (mode == Common::FSNode::kListFilesOnly && !node->_isDirectory) ||
							   (mode == Common::FSNode::kListDirectoriesOnly && node->_isDirectory))) {
			// we need to copy node here as FSNode will take ownership of the pointer and destroy it after use
			HTTPFilesystemNode *file_node = new HTTPFilesystemNode(*node);
			myList.push_back(file_node);
		} else {
			debug(5, "HTTPFilesystemNode::getChildren - skipping %s", node->_path.c_str());
		}
	}
	return true;
}

AbstractFSNode *HTTPFilesystemNode::getParent() const {
	if (_path == "/")
		return 0; // The filesystem root has no parent

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end - 1) != '/')
		end--;

	if (end == start) {
		// This only happens if we were called with a relative path, for which
		// there simply is no parent.
		// TODO: We could also resolve this by assuming that the parent is the
		//       current working directory, and returning a node referring to that.
		return 0;
	}

	Common::String _parent_path = Common::normalizePath(Common::String(start, end), '/');
	FilesystemFactory *factory = g_system->getFilesystemFactory();
	return factory->makeFileNodePath(_parent_path);
}

Common::SeekableReadStream *HTTPFilesystemNode::createReadStream() {
	debug(5, "*HTTPFilesystemNode::createReadStream() %s (size %d) ", _path.c_str(), _size);
	Common::String fsCachePath = Common::normalizePath("/.cache/" + _path, '/');
	POSIXFilesystemNode *cacheFile = new POSIXFilesystemNode(fsCachePath);
	// todo: this should not be cached on the filesystem, but in memory
	// and support range requests
	// port https://github.com/emscripten-core/emscripten/blob/main/src/lib/libwasmfs_fetch.js over
	if (!cacheFile->exists() && _size > 0) {
		byte *buffer = new byte[_size];
		bool success = _httpFsFetchFile(_url.c_str(), buffer, _size);
		if (success) {
			Common::DumpFile *_localFile = new Common::DumpFile();
			if (!_localFile->open(Common::Path(fsCachePath), true)) {
				error("Storage: unable to open file to download into");
				return 0;
			}
			debug(5, "HTTPFilesystemNode::createReadStream() file downloaded %s", _path.c_str());
			_localFile->write(buffer, _size);
			_localFile->close();
			free(buffer);
		} else {
			warning("Storage: unable to download file %s", _url.c_str());
			free(buffer);
			return 0;
		}
	} else if (_size == 0) {
		debug(5, "HTTPFilesystemNode::createReadStream() file empty %s", _path.c_str());
		Common::DumpFile *_localFile = new Common::DumpFile();
		if (!_localFile->open(Common::Path(fsCachePath), true)) {
			warning("Storage: unable to open file to download into");
			return 0;
		}
		_localFile->close();
	}

	return PosixIoStream::makeFromPath(fsCachePath, StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *HTTPFilesystemNode::createWriteStream(bool atomic) {
	return 0;
}

bool HTTPFilesystemNode::createDirectory() {
	return false;
}
bool HTTPFilesystemNode::exists() const {
	return _isValid;
}

bool HTTPFilesystemNode::isReadable() const {
	return exists();
}

bool HTTPFilesystemNode::isWritable() const {
	return false;
}

#endif // #if defined(EMSCRIPTEN)
