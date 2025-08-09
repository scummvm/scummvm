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

#include "backends/fs/emscripten/cloud-fs.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/posix/posix-iostream.h"

#include "common/system.h"

Common::HashMap<Common::String, AbstractFSList> CloudFilesystemNode::_cloudFolders = Common::HashMap<Common::String, AbstractFSList>();

CloudFilesystemNode::CloudFilesystemNode(const Common::String &p) : _isDirectory(false), _isValid(false), _path(p), _storageFileId(nullptr) {
	debug(5, "CloudFilesystemNode::CloudFilesystemNode(Common::String %s)", p.c_str());
	assert(p.size() > 0);

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');
	if (_path == CLOUD_FS_PATH) { // need special case for handling the root of the cloud-filesystem
		_displayName = "[" + Common::lastPathComponent(_path, '/') + "]";
		_isDirectory = true;
		_isValid = true;
		return;
	} else { // we need to peek in the parent folder to see if file exists and if it's a directory
		AbstractFSNode *parent = getParent();
		AbstractFSList tmp = AbstractFSList();
		parent->getChildren(tmp, Common::FSNode::kListAll, true);
		for (AbstractFSList::iterator i = tmp.begin(); i != tmp.end(); ++i) {
			CloudFilesystemNode *child = (CloudFilesystemNode *)*i;
			if (child->getPath() == _path) {
				_isDirectory = child->isDirectory();
				_isValid = true;
				_storageFileId = child->_storageFileId;
				break;
			}
		}

		return;
	}
}

AbstractFSNode *CloudFilesystemNode::getChild(const Common::String &n) const {
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

void CloudFilesystemNode::directoryListedCallback(const Cloud::Storage::ListDirectoryResponse &response) {
	debug(5, "CloudFilesystemNode::directoryListedCallback %s", _path.c_str());
	Common::Array<Cloud::StorageFile> storageFiles = response.value;
	AbstractFSList *dirList = new AbstractFSList();

	for (Common::Array<Cloud::StorageFile>::iterator i = storageFiles.begin(); i != storageFiles.end(); ++i) {
		Cloud::StorageFile storageFile = *i;
		CloudFilesystemNode *file_node = new CloudFilesystemNode();
		file_node->_isDirectory = storageFile.isDirectory();
		file_node->_path = _path + "/" + storageFile.name();
		file_node->_isValid = true;
		file_node->_displayName = "" + storageFile.name();
		file_node->_storageFileId = storageFile.id();
		dirList->push_back(file_node);
	}
	_cloudFolders[_path] = *dirList;
}

void CloudFilesystemNode::directoryListedErrorCallback(const Networking::ErrorResponse &_error) {
	// _workingRequest = nullptr; // TODO: HANDLE THIS SOMEWHERE
	error("Response %ld: %s", _error.httpResponseCode, _error.response.c_str());
}

void CloudFilesystemNode::fileDownloadedCallback(const Cloud::Storage::BoolResponse &response) {
	// _workingRequest = nullptr; // TODO: HANDLE THIS SOMEWHERE
	debug(5, "CloudFilesystemNode::fileDownloadedCallback %s", _path.c_str());
}

void CloudFilesystemNode::fileDownloadedErrorCallback(const Networking::ErrorResponse &_error) {
	// _workingRequest = nullptr; // TODO: HANDLE THIS SOMEWHERE
	error("Response %ld: %s", _error.httpResponseCode, _error.response.c_str());
}

bool CloudFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	if (!_cloudFolders.contains(_path)) {
		debug(5, "CloudFilesystemNode::getChildren Fetching Children: %s", _path.c_str());
		Common::String _cloud_path = _path.substr(sizeof(CLOUD_FS_PATH), _path.size() - sizeof(CLOUD_FS_PATH));

		CloudMan.listDirectory(
			_cloud_path,
			new Common::Callback<CloudFilesystemNode, const Cloud::Storage::ListDirectoryResponse &>((CloudFilesystemNode *)this, &CloudFilesystemNode::directoryListedCallback),
			new Common::Callback<CloudFilesystemNode, const Networking::ErrorResponse &>((CloudFilesystemNode *)this, &CloudFilesystemNode::directoryListedErrorCallback),
			false);

		while (!_cloudFolders.contains(_path)) {
			g_system->delayMillis(10);
		}
		debug(5, "CloudFilesystemNode::getChildren %s size %u", _path.c_str(), _cloudFolders[_path].size());
	}

	for (AbstractFSList::iterator i = _cloudFolders[_path].begin(); i != _cloudFolders[_path].end(); ++i) {
		// TODO: Respect mode and hidden getChildren parameters
		CloudFilesystemNode *node = (CloudFilesystemNode *)*i;
		// we need to copy node here as FSNode will take ownership of the pointer and destroy it after use
		myList.push_back(new CloudFilesystemNode(*node));
	}
	return true;
}

AbstractFSNode *CloudFilesystemNode::getParent() const {
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

Common::SeekableReadStream *CloudFilesystemNode::createReadStream() {
	debug(5, "CloudFilesystemNode::createReadStream() %s", _path.c_str());
	Common::String fsCachePath = Common::normalizePath("/.cache/" + _path, '/');
	POSIXFilesystemNode *cacheFile = new POSIXFilesystemNode(fsCachePath);
	if (!cacheFile->exists()) {
		Cloud::Storage *_storage = CloudMan.getCurrentStorage();
		Networking::Request *_workingRequest = _storage->downloadById(
			_storageFileId,
			Common::Path(fsCachePath),
			new Common::Callback<CloudFilesystemNode, const Cloud::Storage::BoolResponse &>(this, &CloudFilesystemNode::fileDownloadedCallback),
			new Common::Callback<CloudFilesystemNode, const Networking::ErrorResponse &>(this, &CloudFilesystemNode::fileDownloadedErrorCallback));
		while (_workingRequest->state() != Networking::RequestState::FINISHED) {
			g_system->delayMillis(10);
		}
		debug(5, "CloudFilesystemNode::createReadStream() file written %s", fsCachePath.c_str());
	}
	return PosixIoStream::makeFromPath(fsCachePath, StdioStream::WriteMode_Read);
}

Common::SeekableWriteStream *CloudFilesystemNode::createWriteStream(bool atomic) {
	return 0;
}

bool CloudFilesystemNode::createDirectory() {
	return false;
}
bool CloudFilesystemNode::exists() const {
	return _isValid;
}

bool CloudFilesystemNode::isReadable() const {
	return exists();
}

bool CloudFilesystemNode::isWritable() const {
	return false;
}

#endif // #if defined(EMSCRIPTEN)
