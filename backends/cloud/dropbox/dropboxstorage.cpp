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

#include <curl/curl.h>
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/cloud/dropbox/dropboxcreatedirectoryrequest.h"
#include "backends/cloud/dropbox/dropboxinforequest.h"
#include "backends/cloud/dropbox/dropboxlistdirectoryrequest.h"
#include "backends/cloud/dropbox/dropboxuploadrequest.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

#define DROPBOX_API_FILES_DOWNLOAD "https://content.dropboxapi.com/2/files/download"

DropboxStorage::DropboxStorage(Common::String accessToken, bool enabled): BaseStorage(accessToken, "", enabled) {}

DropboxStorage::DropboxStorage(Common::String code, Networking::ErrorCallback cb): BaseStorage() {
	getAccessToken(code, cb);
}

DropboxStorage::~DropboxStorage() {}

Common::String DropboxStorage::cloudProvider() { return "dropbox"; }

uint32 DropboxStorage::storageIndex() { return kStorageDropboxId; }

bool DropboxStorage::needsRefreshToken() { return false; }

bool DropboxStorage::canReuseRefreshToken() { return false; }

void DropboxStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "access_token", _token, ConfMan.kCloudDomain);
	saveIsEnabledFlag(keyPrefix);
}

Common::String DropboxStorage::name() const {
	return "Dropbox";
}

Networking::Request *DropboxStorage::listDirectory(Common::String path, ListDirectoryCallback outerCallback, Networking::ErrorCallback errorCallback, bool recursive) {
	return addRequest(new DropboxListDirectoryRequest(_token, path, outerCallback, errorCallback, recursive));
}

Networking::Request *DropboxStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new DropboxUploadRequest(_token, path, contents, callback, errorCallback));
}

Networking::Request *DropboxStorage::streamFileById(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("path", new Common::JSONValue(path));
	Common::JSONValue value(jsonRequestParameters);

	Networking::CurlRequest *request = new Networking::CurlRequest(nullptr, nullptr, DROPBOX_API_FILES_DOWNLOAD); //TODO: is it OK to pass no callbacks?
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Dropbox-API-Arg: " + Common::JSON::stringify(&value));
	request->addHeader("Content-Type: "); //required to be empty (as we do POST, it's usually app/form-url-encoded)

	Networking::NetworkReadStreamResponse response = request->execute();
	if (callback)
		(*callback)(response);
	return response.request; // no leak here, response.request == request
}

Networking::Request *DropboxStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	return addRequest(new DropboxCreateDirectoryRequest(_token, path, callback, errorCallback));
}

Networking::Request *DropboxStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	return addRequest(new DropboxInfoRequest(_token, callback, errorCallback));
}

Common::String DropboxStorage::savesDirectoryPath() { return "/saves/"; }

DropboxStorage *DropboxStorage::loadFromConfig(Common::String keyPrefix) {
	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("DropboxStorage: no access_token found");
		return nullptr;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);	
	return new DropboxStorage(accessToken, loadIsEnabledFlag(keyPrefix));
}

void DropboxStorage::removeFromConfig(Common::String keyPrefix) {
	ConfMan.removeKey(keyPrefix + "access_token", ConfMan.kCloudDomain);
	removeIsEnabledFlag(keyPrefix);
}

} // End of namespace Dropbox
} // End of namespace Cloud
