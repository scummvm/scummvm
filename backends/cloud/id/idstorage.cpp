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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/cloud/id/idstorage.h"
#include "backends/cloud/id/idcreatedirectoryrequest.h"
#include "backends/cloud/id/iddownloadrequest.h"
#include "backends/cloud/id/idlistdirectoryrequest.h"
#include "backends/cloud/id/idresolveidrequest.h"
#include "backends/cloud/id/idstreamfilerequest.h"
#include "common/debug.h"

namespace Cloud {
namespace Id {

IdStorage::IdStorage() {}

IdStorage::IdStorage(Common::String token, Common::String refreshToken, bool enabled):
	BaseStorage(token, refreshToken, enabled) {}

IdStorage::~IdStorage() {}

void IdStorage::printFiles(FileArrayResponse response) {
	debug(9, "IdStorage: files:");
	Common::Array<StorageFile> &files = response.value;
	for (uint32 i = 0; i < files.size(); ++i) {
		debug(9, "\t%s%s", files[i].name().c_str(), files[i].isDirectory() ? " (directory)" : "");
		debug(9, "\t%s", files[i].path().c_str());
		debug(9, "\t%s", files[i].id().c_str());
		debug(9, " ");
	}
}

void IdStorage::printBool(BoolResponse response) {
	debug(9, "IdStorage: bool: %s", response.value ? "true" : "false");
}

void IdStorage::printFile(UploadResponse response) {
	debug(9, "\nIdStorage: uploaded file info:");
	debug(9, "\tid: %s", response.value.path().c_str());
	debug(9, "\tname: %s", response.value.name().c_str());
	debug(9, "\tsize: %u", response.value.size());
	debug(9, "\ttimestamp: %u", response.value.timestamp());
}

Storage::ListDirectoryCallback IdStorage::getPrintFilesCallback() {
	return new Common::Callback<IdStorage, FileArrayResponse>(this, &IdStorage::printFiles);
}

Networking::Request *IdStorage::resolveFileId(Common::String path, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	if (!callback)
		callback = new Common::Callback<IdStorage, UploadResponse>(this, &IdStorage::printFile);
	return addRequest(new IdResolveIdRequest(this, path, callback, errorCallback));
}

Networking::Request *IdStorage::listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	if (!callback)
		callback = new Common::Callback<IdStorage, FileArrayResponse>(this, &IdStorage::printFiles);
	return addRequest(new IdListDirectoryRequest(this, path, callback, errorCallback, recursive));
}

Networking::Request *IdStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	if (!callback)
		callback = new Common::Callback<IdStorage, BoolResponse>(this, &IdStorage::printBool);

	//find out the parent path and directory name
	Common::String parentPath = "", directoryName = path;
	for (uint32 i = path.size(); i > 0; --i) {
		if (path[i - 1] == '/' || path[i - 1] == '\\') {
			parentPath = path;
			parentPath.erase(i - 1);
			directoryName.erase(0, i);
			break;
		}
	}

	return addRequest(new IdCreateDirectoryRequest(this, parentPath, directoryName, callback, errorCallback));
}

Networking::Request *IdStorage::streamFile(Common::String path, Networking::NetworkReadStreamCallback outerCallback, Networking::ErrorCallback errorCallback) {
	return addRequest(new IdStreamFileRequest(this, path, outerCallback, errorCallback));
}

Networking::Request *IdStorage::download(Common::String remotePath, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new IdDownloadRequest(this, remotePath, localPath, callback, errorCallback));
}

} // End of namespace Id
} // End of namespace Cloud
