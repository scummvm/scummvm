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
#include <curl/curl.h>

namespace Cloud {
namespace OneDrive {

char *OneDriveStorage::KEY; //can't use ConfMan there yet, loading it on instance creation/auth
char *OneDriveStorage::SECRET; //TODO: hide these secrets somehow

void OneDriveStorage::loadKeyAndSecret() {
	Common::String k = ConfMan.get("ONEDRIVE_KEY", "cloud");
	KEY = new char[k.size() + 1];
	memcpy(KEY, k.c_str(), k.size());
	KEY[k.size()] = 0;

	k = ConfMan.get("ONEDRIVE_SECRET", "cloud");
	SECRET = new char[k.size() + 1];
	memcpy(SECRET, k.c_str(), k.size());
	SECRET[k.size()] = 0;
}

OneDriveStorage::OneDriveStorage(Common::String accessToken, Common::String userId, Common::String refreshToken):
	_token(accessToken), _uid(userId), _refreshToken(refreshToken) {}

OneDriveStorage::OneDriveStorage(Common::String code) {
	getAccessToken(new Common::Callback<OneDriveStorage, BoolResponse>(this, &OneDriveStorage::codeFlowComplete), code);
}

OneDriveStorage::~OneDriveStorage() {}

void OneDriveStorage::getAccessToken(BoolCallback callback, Common::String code) {
	bool codeFlow = (code != "");

	if (!codeFlow && _refreshToken == "") {
		warning("OneDriveStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, BoolResponse, Networking::JsonResponse>(this, &OneDriveStorage::tokenRefreshed, callback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, getErrorPrintingCallback(), "https://login.live.com/oauth20_token.srf"); //TODO
	if (codeFlow) {
		request->addPostField("code=" + code);
		request->addPostField("grant_type=authorization_code");
	} else {
		request->addPostField("refresh_token=" + _refreshToken);
		request->addPostField("grant_type=refresh_token");
	}
	request->addPostField("client_id=" + Common::String(KEY));
	request->addPostField("client_secret=" + Common::String(SECRET));
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F");
	addRequest(request);
}

void OneDriveStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("OneDriveStorage: got NULL instead of JSON");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!result.contains("access_token") || !result.contains("user_id") || !result.contains("refresh_token")) {
		warning("Bad response, no token or user_id passed");
		debug("%s", json->stringify().c_str());
		if (callback) (*callback)(BoolResponse(nullptr, false));
	} else {
		_token = result.getVal("access_token")->asString();
		_uid = result.getVal("user_id")->asString();
		_refreshToken = result.getVal("refresh_token")->asString();
		CloudMan.save(); //ask CloudManager to save our new refreshToken
		if (callback) (*callback)(BoolResponse(nullptr, true));
	}
	delete json;
}

void OneDriveStorage::codeFlowComplete(BoolResponse response) {
	if (!response.value) {
		warning("OneDriveStorage: failed to get access token through code flow");
		return;
	}

	ConfMan.removeKey("onedrive_code", "cloud");
	CloudMan.replaceStorage(this, kStorageOneDriveId);
	ConfMan.flushToDisk();
	debug("Done! You can use OneDrive now! Look:");
	CloudMan.syncSaves();
}

void OneDriveStorage::saveConfig(Common::String keyPrefix) {	
	ConfMan.set(keyPrefix + "access_token", _token, "cloud");
	ConfMan.set(keyPrefix + "user_id", _uid, "cloud");
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, "cloud");
}

Common::String OneDriveStorage::name() const {
	return "OneDrive";
}

void OneDriveStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {
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

		CloudMan.setStorageUsedSpace(kStorageOneDriveId, quotaUsed); //TODO that's not ScummVM's actually
		CloudMan.setStorageUsername(kStorageOneDriveId, email);
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, name, email, quotaUsed, quotaAllocated)));
		delete outerCallback;
	}

	delete json;
}

void OneDriveStorage::printJson(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("printJson: NULL");
		return;
	}

	debug("%s", json->stringify().c_str());
	delete json;
}

void OneDriveStorage::fileInfoCallback(Networking::NetworkReadStreamCallback outerCallback, Networking::JsonResponse response) {
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

Networking::Request *OneDriveStorage::listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	return addRequest(new OneDriveListDirectoryRequest(this, path, callback, errorCallback, recursive));
}

Networking::Request *OneDriveStorage::upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	return addRequest(new OneDriveUploadRequest(this, path, contents, callback, errorCallback));
}

Networking::Request *OneDriveStorage::streamFileById(Common::String path, Networking::NetworkReadStreamCallback outerCallback, Networking::ErrorCallback errorCallback) {
	Common::String url = "https://api.onedrive.com/v1.0/drive/special/approot:/" + path;
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, Networking::NetworkReadStreamResponse, Networking::JsonResponse>(this, &OneDriveStorage::fileInfoCallback, outerCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(this, innerCallback, errorCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
}

void OneDriveStorage::fileDownloaded(BoolResponse response) {
	if (response.value) debug("file downloaded!");
	else debug("download failed!");
}

void OneDriveStorage::printFiles(FileArrayResponse response) {
	debug("files:");
	Common::Array<StorageFile> &files = response.value;
	for (uint32 i = 0; i < files.size(); ++i)
		debug("\t%s", files[i].path().c_str());
}

void OneDriveStorage::printBool(BoolResponse response) {
	debug("bool: %s", response.value ? "true" : "false");
}

void OneDriveStorage::printFile(UploadResponse response) {
	debug("\nuploaded file info:");
	debug("\tpath: %s", response.value.path().c_str());
	debug("\tsize: %u", response.value.size());
	debug("\ttimestamp: %u", response.value.timestamp());
}

Networking::Request *OneDriveStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	return addRequest(new OneDriveCreateDirectoryRequest(this, path, callback, errorCallback));
}

Networking::Request *OneDriveStorage::info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<OneDriveStorage, StorageInfoResponse, Networking::JsonResponse>(this, &OneDriveStorage::infoInnerCallback, callback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(this, innerCallback, errorCallback, "https://api.onedrive.com/v1.0/drive/special/approot");
	request->addHeader("Authorization: bearer " + _token);
	return addRequest(request);
}

Common::String OneDriveStorage::savesDirectoryPath() { return "saves/"; }

OneDriveStorage *OneDriveStorage::loadFromConfig(Common::String keyPrefix) {
	loadKeyAndSecret();

	if (!ConfMan.hasKey(keyPrefix + "access_token", "cloud")) {
		warning("No access_token found");
		return 0;
	}

	if (!ConfMan.hasKey(keyPrefix + "user_id", "cloud")) {
		warning("No user_id found");
		return 0;
	}

	if (!ConfMan.hasKey(keyPrefix + "refresh_token", "cloud")) {
		warning("No refresh_token found");
		return 0;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", "cloud");
	Common::String userId = ConfMan.get(keyPrefix + "user_id", "cloud");
	Common::String refreshToken = ConfMan.get(keyPrefix + "refresh_token", "cloud");
	return new OneDriveStorage(accessToken, userId, refreshToken);
}

Common::String OneDriveStorage::getAuthLink() {
	Common::String url = "https://login.live.com/oauth20_authorize.srf";
	url += "?response_type=code";
	url += "&redirect_uri=http://localhost:12345/"; //that's for copy-pasting
	//url += "&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F"; //that's "http://localhost:12345/" for automatic opening
	url += "&client_id="; url += KEY;
	url += "&scope=onedrive.appfolder%20offline_access"; //TODO
	return url;
}

void OneDriveStorage::authThroughConsole() {
	if (!ConfMan.hasKey("ONEDRIVE_KEY", "cloud") || !ConfMan.hasKey("ONEDRIVE_SECRET", "cloud")) {
		warning("No OneDrive keys available, cannot do auth");
		return;
	}

	loadKeyAndSecret();

	if (ConfMan.hasKey("onedrive_code", "cloud")) {
		//phase 2: get access_token using specified code
		new OneDriveStorage(ConfMan.get("onedrive_code", "cloud"));
		return;
	}

	debug("Navigate to this URL and press \"Allow\":");
	debug("%s\n", getAuthLink().c_str());
	debug("Then, add onedrive_code key in [cloud] section of configuration file. You should copy the <code> value from URL and put it as value for that key.\n");
	debug("Navigate to this URL to get more information on ScummVM's configuration files:");
	debug("http://wiki.scummvm.org/index.php/User_Manual/Configuring_ScummVM#Using_the_configuration_file_to_configure_ScummVM\n");	
}

} // End of namespace OneDrive
} // End of namespace Cloud
