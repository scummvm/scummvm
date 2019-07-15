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
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/onedrive/onedrivecreatedirectoryrequest.h"
#include "backends/cloud/onedrive/onedrivetokenrefresher.h"
#include "backends/cloud/onedrive/onedrivelistdirectoryrequest.h"
#include "backends/cloud/onedrive/onedriveuploadrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {
namespace OneDrive {

#define ONEDRIVE_API_SPECIAL_APPROOT_ID "https://api.onedrive.com/v1.0/drive/special/approot:/"
#define ONEDRIVE_API_SPECIAL_APPROOT "https://api.onedrive.com/v1.0/drive/special/approot"

OneDriveStorage::OneDriveStorage(Common::String token,  Common::String refreshToken):
	BaseStorage(token, refreshToken) {}

OneDriveStorage::OneDriveStorage(Common::String code) {
	getAccessToken(code);
}

OneDriveStorage::~OneDriveStorage() {}

Common::String OneDriveStorage::cloudProvider() { return "onedrive"; }

uint32 OneDriveStorage::storageIndex() { return kStorageOneDriveId; }

void OneDriveStorage::refreshAccessToken(BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (_refreshToken == "") {
		warning("OneDriveStorage: no refresh token available to get new access token.");
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, BoolResponse, Networking::JsonResponse>(this, &OneDriveStorage::tokenRefreshed, callback);
	if (errorCallback == nullptr)
		errorCallback = getErrorPrintingCallback();

	Common::String url = "https://cloud.scummvm.org/onedrive/refresh/" + _refreshToken; // TODO: subject to change
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, url);
	addRequest(request);
}

void OneDriveStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("OneDriveStorage: got NULL instead of JSON");
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete callback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "OneDriveStorage")) {
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!Networking::CurlJsonRequest::jsonContainsString(result, "access_token", "OneDriveStorage") ||
		!Networking::CurlJsonRequest::jsonContainsString(result, "user_id", "OneDriveStorage") ||
		!Networking::CurlJsonRequest::jsonContainsString(result, "refresh_token", "OneDriveStorage")) {
		warning("OneDriveStorage: bad response, no token or user_id passed");
		debug(9, "%s", json->stringify().c_str());
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
	} else {
		_token = result.getVal("access_token")->asString();
		_refreshToken = result.getVal("refresh_token")->asString();
		CloudMan.save(); //ask CloudManager to save our new refreshToken
		if (callback)
			(*callback)(BoolResponse(nullptr, true));
	}
	delete json;
	delete callback;
}

void OneDriveStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "access_token", _token, ConfMan.kCloudDomain);
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, ConfMan.kCloudDomain);
}

Common::String OneDriveStorage::name() const {
	return "OneDrive";
}

void OneDriveStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("OneDriveStorage::infoInnerCallback: NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "OneDriveStorage::infoInnerCallback")) {
		delete json;
		delete outerCallback;
		return;
	}

	Common::JSONObject jsonInfo = json->asObject();

	Common::String uid, displayName, email;
	uint64 quotaUsed = 0, quotaAllocated = 26843545600LL; // 25 GB, because I actually don't know any way to find out the real one

	if (Networking::CurlJsonRequest::jsonContainsObject(jsonInfo, "createdBy", "OneDriveStorage::infoInnerCallback")) {
		Common::JSONObject createdBy = jsonInfo.getVal("createdBy")->asObject();
		if (Networking::CurlJsonRequest::jsonContainsObject(createdBy, "user", "OneDriveStorage::infoInnerCallback")) {
			Common::JSONObject user = createdBy.getVal("user")->asObject();
			if (Networking::CurlJsonRequest::jsonContainsString(user, "id", "OneDriveStorage::infoInnerCallback"))
				uid = user.getVal("id")->asString();
			if (Networking::CurlJsonRequest::jsonContainsString(user, "displayName", "OneDriveStorage::infoInnerCallback"))
				displayName = user.getVal("displayName")->asString();
		}
	}

	if (Networking::CurlJsonRequest::jsonContainsIntegerNumber(jsonInfo, "size", "OneDriveStorage::infoInnerCallback")) {
		quotaUsed = jsonInfo.getVal("size")->asIntegerNumber();
	}

	Common::String username = email;
	if (username == "")
		username = displayName;
	if (username == "")
		username = uid;
	CloudMan.setStorageUsername(kStorageOneDriveId, username);

	if (outerCallback) {
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, displayName, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

void OneDriveStorage::fileInfoCallback(Networking::NetworkReadStreamCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("OneDriveStorage::fileInfoCallback: NULL passed instead of JSON");
		if (outerCallback)
			(*outerCallback)(Networking::NetworkReadStreamResponse(response.request, nullptr));
		delete outerCallback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "OneDriveStorage::fileInfoCallback")) {
		if (outerCallback)
			(*outerCallback)(Networking::NetworkReadStreamResponse(response.request, nullptr));
		delete json;
		delete outerCallback;
		return;
	}

	Common::JSONObject result = response.value->asObject();
	if (!Networking::CurlJsonRequest::jsonContainsString(result, "@content.downloadUrl", "OneDriveStorage::fileInfoCallback")) {
		warning("OneDriveStorage: downloadUrl not found in passed JSON");
		debug(9, "%s", response.value->stringify().c_str());
		if (outerCallback)
			(*outerCallback)(Networking::NetworkReadStreamResponse(response.request, nullptr));
		delete json;
		delete outerCallback;
		return;
	}

	const char *url = result.getVal("@content.downloadUrl")->asString().c_str();
	if (outerCallback)
		(*outerCallback)(Networking::NetworkReadStreamResponse(
			response.request,
			new Networking::NetworkReadStream(url, nullptr, "")
		));

	delete json;
	delete outerCallback;
}

Networking::Request *OneDriveStorage::listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	return addRequest(new OneDriveListDirectoryRequest(this, path, callback, errorCallback, recursive));
}

Networking::Request *OneDriveStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new OneDriveUploadRequest(this, path, contents, callback, errorCallback));
}

Networking::Request *OneDriveStorage::streamFileById(Common::String path, Networking::NetworkReadStreamCallback outerCallback, Networking::ErrorCallback errorCallback) {
	Common::String url = ONEDRIVE_API_SPECIAL_APPROOT_ID + ConnMan.urlEncode(path);
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, Networking::NetworkReadStreamResponse, Networking::JsonResponse>(this, &OneDriveStorage::fileInfoCallback, outerCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
}

Networking::Request *OneDriveStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	return addRequest(new OneDriveCreateDirectoryRequest(this, path, callback, errorCallback));
}

Networking::Request *OneDriveStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, StorageInfoResponse, Networking::JsonResponse>(this, &OneDriveStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(this, innerCallback, errorCallback, ONEDRIVE_API_SPECIAL_APPROOT);
	request->addHeader("Authorization: bearer " + _token);
	return addRequest(request);
}

Common::String OneDriveStorage::savesDirectoryPath() { return "saves/"; }

OneDriveStorage *OneDriveStorage::loadFromConfig(Common::String keyPrefix) {
	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("OneDriveStorage: no access_token found");
		return nullptr;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain)) {
		warning("OneDriveStorage: no refresh_token found");
		return nullptr;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	return new OneDriveStorage(accessToken, refreshToken);
}

} // End of namespace OneDrive
} // End of namespace Cloud
