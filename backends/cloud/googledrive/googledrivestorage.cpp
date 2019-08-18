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
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/googledrive/googledrivetokenrefresher.h"
#include "backends/cloud/googledrive/googledrivelistdirectorybyidrequest.h"
#include "backends/cloud/googledrive/googledriveuploadrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include "common/debug.h"

namespace Cloud {
namespace GoogleDrive {

#define GOOGLEDRIVE_API_FILES_ALT_MEDIA "https://www.googleapis.com/drive/v3/files/%s?alt=media"
#define GOOGLEDRIVE_API_FILES "https://www.googleapis.com/drive/v3/files"
#define GOOGLEDRIVE_API_ABOUT "https://www.googleapis.com/drive/v3/about?fields=storageQuota,user"

GoogleDriveStorage::GoogleDriveStorage(Common::String token, Common::String refreshToken, bool enabled):
	IdStorage(token, refreshToken, enabled) {}

GoogleDriveStorage::GoogleDriveStorage(Common::String code, Networking::ErrorCallback cb) {
	getAccessToken(code, cb);
}

GoogleDriveStorage::~GoogleDriveStorage() {}

Common::String GoogleDriveStorage::cloudProvider() { return "gdrive"; }

uint32 GoogleDriveStorage::storageIndex() { return kStorageGoogleDriveId; }

bool GoogleDriveStorage::needsRefreshToken() { return true; }

bool GoogleDriveStorage::canReuseRefreshToken() { return true; }

void GoogleDriveStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "access_token", _token, ConfMan.kCloudDomain);
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, ConfMan.kCloudDomain);
	saveIsEnabledFlag(keyPrefix);
}

Common::String GoogleDriveStorage::name() const {
	return "Google Drive";
}

void GoogleDriveStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("GoogleDriveStorage::infoInnerCallback: NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "GoogleDriveStorage::infoInnerCallback")) {
		delete json;
		delete outerCallback;
		return;
	}

	Common::JSONObject jsonInfo = json->asObject();

	Common::String uid, displayName, email;
	uint64 quotaUsed = 0, quotaAllocated = 0;

	if (Networking::CurlJsonRequest::jsonContainsAttribute(jsonInfo, "user", "GoogleDriveStorage::infoInnerCallback") &&
		Networking::CurlJsonRequest::jsonIsObject(jsonInfo.getVal("user"), "GoogleDriveStorage::infoInnerCallback")) {
		//"me":true, "kind":"drive#user","photoLink": "",
		//"displayName":"Alexander Tkachev","emailAddress":"alexander@tkachov.ru","permissionId":""
		Common::JSONObject user = jsonInfo.getVal("user")->asObject();
		if (Networking::CurlJsonRequest::jsonContainsString(user, "permissionId", "GoogleDriveStorage::infoInnerCallback"))
			uid = user.getVal("permissionId")->asString(); //not sure it's user's id, but who cares anyway?
		if (Networking::CurlJsonRequest::jsonContainsString(user, "displayName", "GoogleDriveStorage::infoInnerCallback"))
			displayName = user.getVal("displayName")->asString();
		if (Networking::CurlJsonRequest::jsonContainsString(user, "emailAddress", "GoogleDriveStorage::infoInnerCallback"))
			email = user.getVal("emailAddress")->asString();
	}

	if (Networking::CurlJsonRequest::jsonContainsAttribute(jsonInfo, "storageQuota", "GoogleDriveStorage::infoInnerCallback") &&
		Networking::CurlJsonRequest::jsonIsObject(jsonInfo.getVal("storageQuota"), "GoogleDriveStorage::infoInnerCallback")) {
		//"usageInDrive":"6332462","limit":"18253611008","usage":"6332462","usageInDriveTrash":"0"
		Common::JSONObject storageQuota = jsonInfo.getVal("storageQuota")->asObject();

		if (Networking::CurlJsonRequest::jsonContainsString(storageQuota, "usage", "GoogleDriveStorage::infoInnerCallback")) {
			Common::String usage = storageQuota.getVal("usage")->asString();
			quotaUsed = usage.asUint64();
		}

		if (Networking::CurlJsonRequest::jsonContainsString(storageQuota, "limit", "GoogleDriveStorage::infoInnerCallback")) {
			Common::String limit = storageQuota.getVal("limit")->asString();
			quotaAllocated = limit.asUint64();
		}
	}

	CloudMan.setStorageUsername(kStorageGoogleDriveId, email);

	if (outerCallback) {
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, displayName, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

void GoogleDriveStorage::createDirectoryInnerCallback(BoolCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("GoogleDriveStorage::createDirectoryInnerCallback: NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {
		if (Networking::CurlJsonRequest::jsonIsObject(json, "GoogleDriveStorage::createDirectoryInnerCallback")) {
			Common::JSONObject jsonInfo = json->asObject();
			(*outerCallback)(BoolResponse(nullptr, jsonInfo.contains("id")));
		} else {
			(*outerCallback)(BoolResponse(nullptr, false));
		}
		delete outerCallback;
	}

	delete json;
}

Networking::Request *GoogleDriveStorage::listDirectoryById(Common::String id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	if (!callback)
		callback = new Common::Callback<GoogleDriveStorage, FileArrayResponse>(this, &GoogleDriveStorage::printFiles);
	return addRequest(new GoogleDriveListDirectoryByIdRequest(this, id, callback, errorCallback));
}

Networking::Request *GoogleDriveStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new GoogleDriveUploadRequest(this, path, contents, callback, errorCallback));
}

Networking::Request *GoogleDriveStorage::streamFileById(Common::String id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	if (callback) {
		Common::String url = Common::String::format(GOOGLEDRIVE_API_FILES_ALT_MEDIA, ConnMan.urlEncode(id).c_str());
		Common::String header = "Authorization: Bearer " + _token;
		curl_slist *headersList = curl_slist_append(nullptr, header.c_str());
		Networking::NetworkReadStream *stream = new Networking::NetworkReadStream(url.c_str(), headersList, "");
		(*callback)(Networking::NetworkReadStreamResponse(nullptr, stream));
	}
	delete callback;
	delete errorCallback;
	return nullptr;
}

void GoogleDriveStorage::printInfo(StorageInfoResponse response) {
	debug(9, "\nGoogleDriveStorage: user info:");
	debug(9, "\tname: %s", response.value.name().c_str());
	debug(9, "\temail: %s", response.value.email().c_str());
	debug(9, "\tdisk usage: %lu/%lu", response.value.used(), response.value.available());
}

Networking::Request *GoogleDriveStorage::createDirectoryWithParentId(Common::String parentId, Common::String directoryName, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();

	Common::String url = GOOGLEDRIVE_API_FILES;
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<GoogleDriveStorage, BoolResponse, Networking::JsonResponse>(this, &GoogleDriveStorage::createDirectoryInnerCallback, callback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + accessToken());
	request->addHeader("Content-Type: application/json");

	Common::JSONArray parentsArray;
	parentsArray.push_back(new Common::JSONValue(parentId));

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("mimeType", new Common::JSONValue("application/vnd.google-apps.folder"));
	jsonRequestParameters.setVal("name", new Common::JSONValue(directoryName));
	jsonRequestParameters.setVal("parents", new Common::JSONValue(parentsArray));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	return addRequest(request);
}

Networking::Request *GoogleDriveStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	if (!callback)
		callback = new Common::Callback<GoogleDriveStorage, StorageInfoResponse>(this, &GoogleDriveStorage::printInfo);
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<GoogleDriveStorage, StorageInfoResponse, Networking::JsonResponse>(this, &GoogleDriveStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(this, innerCallback, errorCallback, GOOGLEDRIVE_API_ABOUT);
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
}

Common::String GoogleDriveStorage::savesDirectoryPath() { return "scummvm/saves/"; }

GoogleDriveStorage *GoogleDriveStorage::loadFromConfig(Common::String keyPrefix) {
	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("GoogleDriveStorage: no access_token found");
		return nullptr;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain)) {
		warning("GoogleDriveStorage: no refresh_token found");
		return nullptr;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	return new GoogleDriveStorage(accessToken, refreshToken, loadIsEnabledFlag(keyPrefix));
}

void GoogleDriveStorage::removeFromConfig(Common::String keyPrefix) {
	ConfMan.removeKey(keyPrefix + "access_token", ConfMan.kCloudDomain);
	ConfMan.removeKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	removeIsEnabledFlag(keyPrefix);
}

Common::String GoogleDriveStorage::getRootDirectoryId() {
	return "root";
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
