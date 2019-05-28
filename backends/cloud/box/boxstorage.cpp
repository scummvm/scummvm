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
#include "backends/cloud/box/boxstorage.h"
#include "backends/cloud/box/boxlistdirectorybyidrequest.h"
#include "backends/cloud/box/boxtokenrefresher.h"
#include "backends/cloud/box/boxuploadrequest.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"

#ifdef ENABLE_RELEASE
#include "dists/clouds/cloud_keys.h"
#endif

namespace Cloud {
namespace Box {

#define BOX_OAUTH2_TOKEN "https://api.box.com/oauth2/token"
#define BOX_API_FOLDERS "https://api.box.com/2.0/folders"
#define BOX_API_FILES_CONTENT "https://api.box.com/2.0/files/%s/content"
#define BOX_API_USERS_ME "https://api.box.com/2.0/users/me"

char *BoxStorage::KEY = nullptr; //can't use CloudConfig there yet, loading it on instance creation/auth
char *BoxStorage::SECRET = nullptr;

void BoxStorage::loadKeyAndSecret() {
#ifdef ENABLE_RELEASE
	KEY = RELEASE_BOX_KEY;
	SECRET = RELEASE_BOX_SECRET;
#else
	Common::String k = ConfMan.get("BOX_KEY", ConfMan.kCloudDomain);
	KEY = new char[k.size() + 1];
	memcpy(KEY, k.c_str(), k.size());
	KEY[k.size()] = 0;

	k = ConfMan.get("BOX_SECRET", ConfMan.kCloudDomain);
	SECRET = new char[k.size() + 1];
	memcpy(SECRET, k.c_str(), k.size());
	SECRET[k.size()] = 0;
#endif
}

BoxStorage::BoxStorage(Common::String token, Common::String refreshToken):
	_token(token), _refreshToken(refreshToken) {}

BoxStorage::BoxStorage(Common::String code) {
	getAccessToken(
		new Common::Callback<BoxStorage, BoolResponse>(this, &BoxStorage::codeFlowComplete),
		new Common::Callback<BoxStorage, Networking::ErrorResponse>(this, &BoxStorage::codeFlowFailed),
		code
	);
}

BoxStorage::~BoxStorage() {}

void BoxStorage::getAccessToken(BoolCallback callback, Networking::ErrorCallback errorCallback, Common::String code) {
	if (!KEY || !SECRET)
		loadKeyAndSecret();
	bool codeFlow = (code != "");

	if (!codeFlow && _refreshToken == "") {
		warning("BoxStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, BoolResponse, Networking::JsonResponse>(this, &BoxStorage::tokenRefreshed, callback);
	if (errorCallback == nullptr)
		errorCallback = getErrorPrintingCallback();

	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, BOX_OAUTH2_TOKEN);
	if (codeFlow) {
		request->addPostField("grant_type=authorization_code");
		request->addPostField("code=" + code);
	} else {
		request->addPostField("grant_type=refresh_token");
		request->addPostField("refresh_token=" + _refreshToken);
	}
	request->addPostField("client_id=" + Common::String(KEY));
	request->addPostField("client_secret=" + Common::String(SECRET));
	/*
	if (Cloud::CloudManager::couldUseLocalServer()) {
	    request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345");
	} else {
	    request->addPostField("&redirect_uri=https%3A%2F%2Fwww.scummvm.org/c/code");
	}
	*/
	addRequest(request);
}

void BoxStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("BoxStorage: got NULL instead of JSON");
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete callback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "BoxStorage")) {
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!Networking::CurlJsonRequest::jsonContainsString(result, "access_token", "BoxStorage") ||
		!Networking::CurlJsonRequest::jsonContainsString(result, "refresh_token", "BoxStorage")) {
		warning("BoxStorage: bad response, no token passed");
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

void BoxStorage::codeFlowComplete(BoolResponse response) {
	if (!response.value) {
		warning("BoxStorage: failed to get access token through code flow");
		CloudMan.removeStorage(this);
		return;
	}

	CloudMan.replaceStorage(this, kStorageBoxId);
	ConfMan.flushToDisk();
}

void BoxStorage::codeFlowFailed(Networking::ErrorResponse error) {
	debug(9, "BoxStorage: code flow failed (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug(9, "%s", error.response.c_str());
	CloudMan.removeStorage(this);
}

void BoxStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "access_token", _token, ConfMan.kCloudDomain);
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, ConfMan.kCloudDomain);
}

Common::String BoxStorage::name() const {
	return "Box";
}

void BoxStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("BoxStorage::infoInnerCallback: NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonIsObject(json, "BoxStorage::infoInnerCallback")) {
		delete json;
		delete outerCallback;
		return;
	}

	Common::JSONObject jsonInfo = json->asObject();

	Common::String uid, displayName, email;
	uint64 quotaUsed = 0, quotaAllocated = 0;

	// can check that "type": "user"
	// there is also "max_upload_size", "phone" and "avatar_url"

	if (Networking::CurlJsonRequest::jsonContainsString(jsonInfo, "id", "BoxStorage::infoInnerCallback"))
		uid = jsonInfo.getVal("id")->asString();

	if (Networking::CurlJsonRequest::jsonContainsString(jsonInfo, "name", "BoxStorage::infoInnerCallback"))
		displayName = jsonInfo.getVal("name")->asString();

	if (Networking::CurlJsonRequest::jsonContainsString(jsonInfo, "login", "BoxStorage::infoInnerCallback"))
		email = jsonInfo.getVal("login")->asString();

	if (Networking::CurlJsonRequest::jsonContainsIntegerNumber(jsonInfo, "space_amount", "BoxStorage::infoInnerCallback"))
		quotaAllocated = jsonInfo.getVal("space_amount")->asIntegerNumber();

	if (Networking::CurlJsonRequest::jsonContainsIntegerNumber(jsonInfo, "space_used", "BoxStorage::infoInnerCallback"))
		quotaUsed = jsonInfo.getVal("space_used")->asIntegerNumber();

	Common::String username = email;
	if (username == "") username = displayName;
	if (username == "") username = uid;
	CloudMan.setStorageUsername(kStorageBoxId, username);

	if (outerCallback) {
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, displayName, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

Networking::Request *BoxStorage::listDirectoryById(Common::String id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	if (!callback)
		callback = getPrintFilesCallback();
	return addRequest(new BoxListDirectoryByIdRequest(this, id, callback, errorCallback));
}

void BoxStorage::createDirectoryInnerCallback(BoolCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("BoxStorage::createDirectoryInnerCallback: NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {
		if (Networking::CurlJsonRequest::jsonIsObject(json, "BoxStorage::createDirectoryInnerCallback")) {
			Common::JSONObject jsonInfo = json->asObject();
			(*outerCallback)(BoolResponse(nullptr, jsonInfo.contains("id")));
		} else {
			(*outerCallback)(BoolResponse(nullptr, false));
		}
		delete outerCallback;
	}

	delete json;
}

Networking::Request *BoxStorage::createDirectoryWithParentId(Common::String parentId, Common::String directoryName, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();

	Common::String url = BOX_API_FOLDERS;
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, BoolResponse, Networking::JsonResponse>(this, &BoxStorage::createDirectoryInnerCallback, callback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + accessToken());
	request->addHeader("Content-Type: application/json");

	Common::JSONObject parentObject;
	parentObject.setVal("id", new Common::JSONValue(parentId));

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("name", new Common::JSONValue(directoryName));
	jsonRequestParameters.setVal("parent", new Common::JSONValue(parentObject));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	return addRequest(request);
}

Networking::Request *BoxStorage::upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	return addRequest(new BoxUploadRequest(this, remotePath, localPath, callback, errorCallback));
}

Networking::Request *BoxStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	warning("BoxStorage::upload(ReadStream) not implemented");
	if (errorCallback)
		(*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "BoxStorage::upload(ReadStream) not implemented", -1));
	delete callback;
	delete errorCallback;
	return nullptr;
}

bool BoxStorage::uploadStreamSupported() {
	return false;
}

Networking::Request *BoxStorage::streamFileById(Common::String id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	if (callback) {
		Common::String url = Common::String::format(BOX_API_FILES_CONTENT, id.c_str());
		Common::String header = "Authorization: Bearer " + _token;
		curl_slist *headersList = curl_slist_append(nullptr, header.c_str());
		Networking::NetworkReadStream *stream = new Networking::NetworkReadStream(url.c_str(), headersList, "");
		(*callback)(Networking::NetworkReadStreamResponse(nullptr, stream));
	}
	delete callback;
	delete errorCallback;
	return nullptr;
}

Networking::Request *BoxStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, StorageInfoResponse, Networking::JsonResponse>(this, &BoxStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(this, innerCallback, errorCallback, BOX_API_USERS_ME);
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
}

Common::String BoxStorage::savesDirectoryPath() { return "scummvm/saves/"; }

BoxStorage *BoxStorage::loadFromConfig(Common::String keyPrefix) {
	loadKeyAndSecret();

	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("BoxStorage: no access_token found");
		return nullptr;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain)) {
		warning("BoxStorage: no refresh_token found");
		return nullptr;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	return new BoxStorage(accessToken, refreshToken);
}

Common::String BoxStorage::getRootDirectoryId() {
	return "0";
}

} // End of namespace Box
} // End of namespace Cloud
