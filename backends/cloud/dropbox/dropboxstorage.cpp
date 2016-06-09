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

#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/cloud/dropbox/dropboxcreatedirectoryrequest.h"
#include "backends/cloud/dropbox/dropboxlistdirectoryrequest.h"
#include "backends/cloud/dropbox/dropboxuploadrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

char *DropboxStorage::KEY; //can't use ConfMan there yet, loading it on instance creation/auth
char *DropboxStorage::SECRET; //TODO: hide these secrets somehow

void DropboxStorage::loadKeyAndSecret() {
	Common::String k = ConfMan.get("DROPBOX_KEY", "cloud");
	KEY = new char[k.size() + 1];
	memcpy(KEY, k.c_str(), k.size());
	KEY[k.size()] = 0;

	k = ConfMan.get("DROPBOX_SECRET", "cloud");
	SECRET = new char[k.size() + 1];
	memcpy(SECRET, k.c_str(), k.size());
	SECRET[k.size()] = 0;
}

static void saveAccessTokenCallback(Networking::JsonResponse pair) {
	Common::JSONValue *json = (Common::JSONValue *)pair.value;
	if (json) {
		debug("saveAccessTokenCallback:");
		debug("%s", json->stringify(true).c_str());

		Common::JSONObject result = json->asObject();
		if (!result.contains("access_token") || !result.contains("uid")) {
			warning("Bad response, no token/uid passed");
		} else {
			//we suppose that's the first storage
			//TODO: update it to use CloudMan.replaceStorage()			
			ConfMan.set("current_storage", "1", "cloud");
			ConfMan.set("storage_Dropbox_type", "Dropbox", "cloud");
			ConfMan.set("storage_Dropbox_access_token", result.getVal("access_token")->asString(), "cloud");
			ConfMan.set("storage_Dropbox_user_id", result.getVal("uid")->asString(), "cloud");
			ConfMan.removeKey("dropbox_code", "cloud");
			ConfMan.flushToDisk();
			debug("Now please restart ScummVM to apply the changes.");
		}

		delete json;
	} else {
		debug("saveAccessTokenCallback: got NULL instead of JSON!");
	}
}

DropboxStorage::DropboxStorage(Common::String accessToken, Common::String userId): _token(accessToken), _uid(userId) {
	curl_global_init(CURL_GLOBAL_ALL);
}

DropboxStorage::~DropboxStorage() {
	curl_global_cleanup();
}

void DropboxStorage::saveConfig(Common::String keyPrefix) {	
	ConfMan.set(keyPrefix + "access_token", _token, "cloud");
	ConfMan.set(keyPrefix + "user_id", _uid, "cloud");
}

Common::String DropboxStorage::name() const {
	return "Dropbox";
}

void DropboxStorage::printFiles(FileArrayResponse response) {
	debug("files:");
	Common::Array<StorageFile> &files = response.value;
	for (uint32 i = 0; i < files.size(); ++i)
		debug("\t%s", files[i].name().c_str());
}

void DropboxStorage::printBool(BoolResponse response) {
	debug("bool: %s", (response.value?"true":"false"));
}

void DropboxStorage::printStorageFile(UploadResponse response) {	
	debug("\nuploaded file info:");
	debug("\tpath: %s", response.value.path().c_str());
	debug("\tsize: %u", response.value.size());
	debug("\ttimestamp: %u", response.value.timestamp());
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

	Networking::CurlRequest *request = new Networking::CurlRequest(nullptr, nullptr, "https://content.dropboxapi.com/2/files/download"); //TODO: is it right?
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Dropbox-API-Arg: " + Common::JSON::stringify(&value));
	request->addHeader("Content-Type: "); //required to be empty (as we do POST, it's usually app/form-url-encoded)

	Networking::NetworkReadStreamResponse response = request->execute();
	if (callback) (*callback)(response);
	return response.request;
}

Networking::Request *DropboxStorage::createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	return addRequest(new DropboxCreateDirectoryRequest(_token, path, callback, errorCallback));
}

Networking::Request *DropboxStorage::info(StorageInfoCallback outerCallback, Networking::ErrorCallback errorCallback) {
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<DropboxStorage, StorageInfoResponse, Networking::JsonResponse>(this, &DropboxStorage::infoInnerCallback, outerCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, "https://api.dropboxapi.com/1/account/info");
	request->addHeader("Authorization: Bearer " + _token);
	return addRequest(request);
	//that callback bridge wraps the outerCallback (passed in arguments from user) into innerCallback
	//so, when CurlJsonRequest is finished, it calls the innerCallback
	//innerCallback (which is DropboxStorage::infoInnerCallback in this case) processes the void *ptr
	//and then calls the outerCallback (which wants to receive StorageInfo, not void *)
}

Common::String DropboxStorage::savesDirectoryPath() { return "/saves/"; }

void DropboxStorage::infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (!json) {
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {		
		//Dropbox documentation states there is no errors for this API method
		Common::JSONObject info = json->asObject();
		Common::String uid = Common::String::format("%d", (int)info.getVal("uid")->asIntegerNumber());
		Common::String name = info.getVal("display_name")->asString();
		Common::String email = info.getVal("email")->asString();
		Common::JSONObject quota = info.getVal("quota_info")->asObject();
		uint64 quotaNormal = quota.getVal("normal")->asIntegerNumber();
		uint64 quotaShared = quota.getVal("shared")->asIntegerNumber();
		uint64 quotaAllocated = quota.getVal("quota")->asIntegerNumber();
		(*outerCallback)(StorageInfoResponse(nullptr, StorageInfo(uid, name, email, quotaNormal+quotaShared, quotaAllocated)));
		delete outerCallback;
	}
	
	delete json;
}

void DropboxStorage::infoMethodCallback(StorageInfoResponse response) {
	debug("\nStorage info:");
	debug("User name: %s", response.value.name().c_str());
	debug("Email: %s", response.value.email().c_str());
	debug("Disk usage: %u/%u", (uint32)response.value.used(), (uint32)response.value.available());
}

DropboxStorage *DropboxStorage::loadFromConfig(Common::String keyPrefix) {
	loadKeyAndSecret();

	if (!ConfMan.hasKey(keyPrefix + "access_token", "cloud")) {
		warning("No access_token found");
		return 0;
	}

	if (!ConfMan.hasKey(keyPrefix + "user_id", "cloud")) {
		warning("No user_id found");
		return 0;
	}

	Common::String accessToken = ConfMan.get(keyPrefix + "access_token", "cloud");
	Common::String userId = ConfMan.get(keyPrefix + "user_id", "cloud");
	return new DropboxStorage(accessToken, userId);
}

Common::String DropboxStorage::getAuthLink() {
	Common::String url = "https://www.dropbox.com/1/oauth2/authorize";
	url += "?response_type=code";
	url += "&redirect_uri=http://localhost:12345/"; //that's for copy-pasting
	//url += "&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F"; //that's "http://localhost:12345/" for automatic opening
	url += "&client_id="; url += KEY;
	return url;
}

void DropboxStorage::authThroughConsole() {
	if (!ConfMan.hasKey("DROPBOX_KEY", "cloud") || !ConfMan.hasKey("DROPBOX_SECRET", "cloud")) {
		warning("No Dropbox keys available, cannot do auth");
		return;
	}

	loadKeyAndSecret();

	if (ConfMan.hasKey("dropbox_code", "cloud")) {
		//phase 2: get access_token using specified code
		getAccessToken(ConfMan.get("dropbox_code", "cloud"));
		return;
	}

	debug("Navigate to this URL and press \"Allow\":");
	debug("%s\n", getAuthLink().c_str());
	debug("Then, add dropbox_code key in [cloud] section of configuration file. You should copy the <code> value from URL and put it as value for that key.\n");
	debug("Navigate to this URL to get more information on ScummVM's configuration files:");
	debug("http://wiki.scummvm.org/index.php/User_Manual/Configuring_ScummVM#Using_the_configuration_file_to_configure_ScummVM\n");	
}

void DropboxStorage::getAccessToken(Common::String code) {
	Networking::JsonCallback callback = new Common::GlobalFunctionCallback<Networking::JsonResponse>(saveAccessTokenCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, nullptr, "https://api.dropboxapi.com/1/oauth2/token"); //TODO
	request->addPostField("code=" + code);
	request->addPostField("grant_type=authorization_code");
	request->addPostField("client_id=" + Common::String(KEY));
	request->addPostField("client_secret=" + Common::String(SECRET));
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F");	
	ConnMan.addRequest(request);	
}

} // End of namespace Dropbox
} // End of namespace Cloud
