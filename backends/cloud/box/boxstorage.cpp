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

#include "backends/cloud/box/boxstorage.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>
#include "common/config-manager.h"

namespace Cloud {
namespace Box {

char *BoxStorage::KEY = nullptr; //can't use CloudConfig there yet, loading it on instance creation/auth
char *BoxStorage::SECRET = nullptr; //TODO: hide these secrets somehow

void BoxStorage::loadKeyAndSecret() {
	Common::String k = ConfMan.get("BOX_KEY", ConfMan.kCloudDomain);
	KEY = new char[k.size() + 1];
	memcpy(KEY, k.c_str(), k.size());
	KEY[k.size()] = 0;

	k = ConfMan.get("BOX_SECRET", ConfMan.kCloudDomain);
	SECRET = new char[k.size() + 1];
	memcpy(SECRET, k.c_str(), k.size());
	SECRET[k.size()] = 0;
}

BoxStorage::BoxStorage(Common::String accessToken, Common::String refreshToken):
	_token(accessToken), _refreshToken(refreshToken) {}

BoxStorage::BoxStorage(Common::String code) {
	getAccessToken(new Common::Callback<BoxStorage, BoolResponse>(this, &BoxStorage::codeFlowComplete), code);
}

BoxStorage::~BoxStorage() {}

void BoxStorage::getAccessToken(BoolCallback callback, Common::String code) {
	if (!KEY || !SECRET) loadKeyAndSecret();
	bool codeFlow = (code != "");

	if (!codeFlow && _refreshToken == "") {
		warning("BoxStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, BoolResponse, Networking::JsonResponse>(this, &BoxStorage::tokenRefreshed, callback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, getErrorPrintingCallback(), "https://api.box.com/oauth2/token");
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
#ifdef USE_SDL_NET
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345");
#else
	request->addPostField("&redirect_uri=https%3A%2F%2Fwww.scummvm.org/c/code");
#endif
	*/
	addRequest(request);
}

void BoxStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("BoxStorage: got NULL instead of JSON");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!result.contains("access_token") || !result.contains("refresh_token")) {
		warning("Bad response, no token passed");
		debug("%s", json->stringify().c_str());
		if (callback) (*callback)(BoolResponse(nullptr, false));
	} else {
		_token = result.getVal("access_token")->asString();
		_refreshToken = result.getVal("refresh_token")->asString();
		CloudMan.save(); //ask CloudManager to save our new refreshToken
		if (callback) (*callback)(BoolResponse(nullptr, true));
	}
	delete json;
}

void BoxStorage::codeFlowComplete(BoolResponse response) {
	if (!response.value) {
		warning("BoxStorage: failed to get access token through code flow");
		return;
	}

	CloudMan.replaceStorage(this, kStorageBoxId);
	ConfMan.flushToDisk();
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
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}
	
	Common::JSONObject info = json->asObject();

	Common::String uid, name, email;
	uint64 quotaUsed = 0, quotaAllocated = 26843545600L; // 25 GB, because I actually don't know any way to find out the real one

	if (info.contains("createdBy") && info.getVal("createdBy")->isObject()) {
		Common::JSONObject createdBy = info.getVal("createdBy")->asObject();
		if (createdBy.contains("user") && createdBy.getVal("user")->isObject()) {
			Common::JSONObject user = createdBy.getVal("user")->asObject();
			uid = user.getVal("id")->asString();
			name = user.getVal("displayName")->asString();
		}
	}

	if (info.contains("size") && info.getVal("size")->isIntegerNumber()) {
		quotaUsed = info.getVal("size")->asIntegerNumber();
	}

	Common::String username = email;
	if (username == "") username = name;
	if (username == "") username = uid;
	CloudMan.setStorageUsername(kStorageBoxId, username);

	if (outerCallback) {
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, name, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

void BoxStorage::printJson(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("printJson: NULL");
		return;
	}

	debug("%s", json->stringify().c_str());
	delete json;
}

void BoxStorage::fileInfoCallback(Networking::NetworkReadStreamCallback outerCallback, Networking::JsonResponse response) {
	if (!response.value) {
		warning("fileInfoCallback: NULL");
		if (outerCallback) (*outerCallback)(Networking::NetworkReadStreamResponse(response.request, 0));
		return;
	}

	Common::JSONObject result = response.value->asObject();
	if (result.contains("@content.downloadUrl")) {
		const char *url = result.getVal("@content.downloadUrl")->asString().c_str();
		if (outerCallback)
			(*outerCallback)(Networking::NetworkReadStreamResponse(
				response.request,
				new Networking::NetworkReadStream(url, 0, "")
			));
	} else {
		warning("downloadUrl not found in passed JSON");
		debug("%s", response.value->stringify().c_str());
		if (outerCallback) (*outerCallback)(Networking::NetworkReadStreamResponse(response.request, 0));
	}
	delete response.value;
}

Networking::Request *BoxStorage::listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	//return addRequest(new BoxListDirectoryRequest(this, path, callback, errorCallback, recursive));
	return nullptr; //TODO
}

Networking::Request *BoxStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	//return addRequest(new BoxUploadRequest(this, path, contents, callback, errorCallback));
	return nullptr; //TODO
}

Networking::Request *BoxStorage::streamFileById(Common::String path, Networking::NetworkReadStreamCallback outerCallback, Networking::ErrorCallback errorCallback) {
	/*
	Common::String url = "https://api.Box.com/v1.0/drive/special/approot:/" + ConnMan.urlEncode(path);
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, Networking::NetworkReadStreamResponse, Networking::JsonResponse>(this, &BoxStorage::fileInfoCallback, outerCallback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
	*/
	return nullptr; //TODO
}

void BoxStorage::fileDownloaded(BoolResponse response) {
	if (response.value) debug("file downloaded!");
	else debug("download failed!");
}

void BoxStorage::printFiles(FileArrayResponse response) {
	debug("files:");
	Common::Array<StorageFile> &files = response.value;
	for (uint32 i = 0; i < files.size(); ++i)
		debug("\t%s", files[i].path().c_str());
}

void BoxStorage::printBool(BoolResponse response) {
	debug("bool: %s", response.value ? "true" : "false");
}

void BoxStorage::printFile(UploadResponse response) {
	debug("\nuploaded file info:");
	debug("\tpath: %s", response.value.path().c_str());
	debug("\tsize: %u", response.value.size());
	debug("\ttimestamp: %u", response.value.timestamp());
}

Networking::Request *BoxStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	//return addRequest(new BoxCreateDirectoryRequest(this, path, callback, errorCallback));
	return nullptr; //TODO
}

Networking::Request *BoxStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	/*
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BoxStorage, StorageInfoResponse, Networking::JsonResponse>(this, &BoxStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(this, innerCallback, errorCallback, "https://api.Box.com/v1.0/drive/special/approot");
	request->addHeader("Authorization: bearer " + _token);
	return addRequest(request);
	*/
	return nullptr; //TODO
}

Common::String BoxStorage::savesDirectoryPath() { return "saves/"; }

BoxStorage *BoxStorage::loadFromConfig(Common::String keyPrefix) {
	loadKeyAndSecret();

	if (!ConfMan.hasKey(keyPrefix + "access_token", ConfMan.kCloudDomain)) {
		warning("No access_token found");
		return 0;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", ConfMan.kCloudDomain)) {
		warning("No refresh_token found");
		return 0;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", ConfMan.kCloudDomain);
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", ConfMan.kCloudDomain);
	return new BoxStorage(accessToken, refreshToken);
}

Common::String BoxStorage::getAuthLink() {
	// now we only specify short "scummvm.org/c/bx" with actual redirect to the auth page
	return "";
}

} // End of namespace Box
} // End of namespace Cloud
