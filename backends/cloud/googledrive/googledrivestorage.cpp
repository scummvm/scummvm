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
#include <curl/curl.h>

namespace Cloud {
namespace GoogleDrive {

char *GoogleDriveStorage::KEY = nullptr; //can't use CloudConfig there yet, loading it on instance creation/auth
char *GoogleDriveStorage::SECRET = nullptr; //TODO: hide these secrets somehow

void GoogleDriveStorage::loadKeyAndSecret() {
	Common::String k = ConfMan.get("GOOGLE_DRIVE_KEY", ConfMan.kCloudDomain);
	KEY = new char[k.size() + 1];
	memcpy(KEY, k.c_str(), k.size());
	KEY[k.size()] = 0;

	k = ConfMan.get("GOOGLE_DRIVE_SECRET", ConfMan.kCloudDomain);
	SECRET = new char[k.size() + 1];
	memcpy(SECRET, k.c_str(), k.size());
	SECRET[k.size()] = 0;
}

GoogleDriveStorage::GoogleDriveStorage(Common::String accessToken, Common::String refreshToken):
	_token(accessToken), _refreshToken(refreshToken) {}

GoogleDriveStorage::GoogleDriveStorage(Common::String code) {
	getAccessToken(
	    new Common::Callback<GoogleDriveStorage, BoolResponse>(this, &GoogleDriveStorage::codeFlowComplete),
	    new Common::Callback<GoogleDriveStorage, Networking::ErrorResponse>(this, &GoogleDriveStorage::codeFlowFailed),
	    code
	);
}

GoogleDriveStorage::~GoogleDriveStorage() {}

void GoogleDriveStorage::getAccessToken(BoolCallback callback, Networking::ErrorCallback errorCallback, Common::String code) {
	if (!KEY || !SECRET) loadKeyAndSecret();
	bool codeFlow = (code != "");

	if (!codeFlow && _refreshToken == "") {
		warning("GoogleDriveStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<GoogleDriveStorage, BoolResponse, Networking::JsonResponse>(this, &GoogleDriveStorage::tokenRefreshed, callback);
	if (errorCallback == nullptr) errorCallback = getErrorPrintingCallback();
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, "https://accounts.google.com/o/oauth2/token"); //TODO
	if (codeFlow) {
		request->addPostField("code=" + code);
		request->addPostField("grant_type=authorization_code");
	} else {
		request->addPostField("refresh_token=" + _refreshToken);
		request->addPostField("grant_type=refresh_token");
	}
	request->addPostField("client_id=" + Common::String(KEY));
	request->addPostField("client_secret=" + Common::String(SECRET));
	if (Cloud::CloudManager::couldUseLocalServer()) {
		request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345");
	} else {
		request->addPostField("&redirect_uri=https%3A%2F%2Fwww.scummvm.org/c/code");
	}
	addRequest(request);
}

void GoogleDriveStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("GoogleDriveStorage: got NULL instead of JSON");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!result.contains("access_token")) {
		warning("Bad response, no token passed");
		debug("%s", json->stringify().c_str());
		if (callback) (*callback)(BoolResponse(nullptr, false));
	} else {
		_token = result.getVal("access_token")->asString();
		if (!result.contains("refresh_token"))
			warning("No refresh_token passed");
		else
			_refreshToken = result.getVal("refresh_token")->asString();
		CloudMan.save(); //ask CloudManager to save our new refreshToken
		if (callback) (*callback)(BoolResponse(nullptr, true));
	}
	delete json;
}

void GoogleDriveStorage::codeFlowComplete(BoolResponse response) {
	if (!response.value) {
		warning("GoogleDriveStorage: failed to get access token through code flow");
		CloudMan.removeStorage(this);
		return;
	}

	ConfMan.removeKey("googledrive_code", ConfMan.kCloudDomain);
	CloudMan.replaceStorage(this, kStorageGoogleDriveId);
	ConfMan.flushToDisk();
}

void GoogleDriveStorage::codeFlowFailed(Networking::ErrorResponse error) {
	debug("Google Drive's code flow failed (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug("%s", error.response.c_str());
	CloudMan.removeStorage(this);
}

void GoogleDriveStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "access_token", _token, ConfMan.kCloudDomain);
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, ConfMan.kCloudDomain);
}

Common::String GoogleDriveStorage::name() const {
	return "Google Drive";
}

void GoogleDriveStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	Common::JSONObject info = json->asObject();

	Common::String uid, name, email;
	uint64 quotaUsed = 0, quotaAllocated = 0;

	if (info.contains("user") && info.getVal("user")->isObject()) {
		//"me":true, "kind":"drive#user","photoLink": "",
		//"displayName":"Alexander Tkachev","emailAddress":"alexander@tkachov.ru","permissionId":""
		Common::JSONObject user = info.getVal("user")->asObject();
		uid = user.getVal("permissionId")->asString(); //not sure it's user's id, but who cares anyway?
		name = user.getVal("displayName")->asString();
		email = user.getVal("emailAddress")->asString();
	}

	if (info.contains("storageQuota") && info.getVal("storageQuota")->isObject()) {
		//"usageInDrive":"6332462","limit":"18253611008","usage":"6332462","usageInDriveTrash":"0"
		Common::JSONObject storageQuota = info.getVal("storageQuota")->asObject();
		Common::String usage = storageQuota.getVal("usage")->asString();
		Common::String limit = storageQuota.getVal("limit")->asString();
		quotaUsed = usage.asUint64();
		quotaAllocated = limit.asUint64();
	}

	CloudMan.setStorageUsername(kStorageGoogleDriveId, email);

	if (outerCallback) {
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, name, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

void GoogleDriveStorage::createDirectoryInnerCallback(BoolCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {
		Common::JSONObject info = json->asObject();
		(*outerCallback)(BoolResponse(nullptr, info.contains("id")));
		delete outerCallback;
	}

	delete json;
}

Networking::Request *GoogleDriveStorage::listDirectoryById(Common::String id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	if (!callback) callback = new Common::Callback<GoogleDriveStorage, FileArrayResponse>(this, &GoogleDriveStorage::printFiles);
	return addRequest(new GoogleDriveListDirectoryByIdRequest(this, id, callback, errorCallback));
}

Networking::Request *GoogleDriveStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new GoogleDriveUploadRequest(this, path, contents, callback, errorCallback));
}

Networking::Request *GoogleDriveStorage::streamFileById(Common::String id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	if (callback) {
		Common::String url = "https://www.googleapis.com/drive/v3/files/" + ConnMan.urlEncode(id) + "?alt=media";
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
	debug(9, "\nuser info:");
	debug(9, "\tname: %s", response.value.name().c_str());
	debug(9, "\temail: %s", response.value.email().c_str());
	debug(9, "\tdisk usage: %lu/%lu", response.value.used(), response.value.available());
}

Networking::Request *GoogleDriveStorage::createDirectoryWithParentId(Common::String parentId, Common::String name, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::String url = "https://www.googleapis.com/drive/v3/files";
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<GoogleDriveStorage, BoolResponse, Networking::JsonResponse>(this, &GoogleDriveStorage::createDirectoryInnerCallback, callback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + accessToken());
	request->addHeader("Content-Type: application/json");

	Common::JSONArray parentsArray;
	parentsArray.push_back(new Common::JSONValue(parentId));

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("mimeType", new Common::JSONValue("application/vnd.google-apps.folder"));
	jsonRequestParameters.setVal("name", new Common::JSONValue(name));
	jsonRequestParameters.setVal("parents", new Common::JSONValue(parentsArray));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	return addRequest(request);
}

Networking::Request *GoogleDriveStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	if (!callback) callback = new Common::Callback<GoogleDriveStorage, StorageInfoResponse>(this, &GoogleDriveStorage::printInfo);
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<GoogleDriveStorage, StorageInfoResponse, Networking::JsonResponse>(this, &GoogleDriveStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(this, innerCallback, errorCallback, "https://www.googleapis.com/drive/v3/about?fields=storageQuota,user");
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
}

Common::String GoogleDriveStorage::savesDirectoryPath() { return "scummvm/saves/"; }

GoogleDriveStorage *GoogleDriveStorage::loadFromConfig(Common::String keyPrefix) {
	loadKeyAndSecret();

	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("No access_token found");
		return nullptr;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain)) {
		warning("No refresh_token found");
		return nullptr;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	return new GoogleDriveStorage(accessToken, refreshToken);
}

Common::String GoogleDriveStorage::getRootDirectoryId() {
	return "root";
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
