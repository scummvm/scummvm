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
#include "backends/cloud/dropbox/dropboxlistdirectoryrequest.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>
#include <common/file.h>

namespace Cloud {
namespace Dropbox {

Common::String DropboxStorage::KEY; //can't use ConfMan there yet, loading it on instance creation/auth
Common::String DropboxStorage::SECRET; //TODO: hide these secrets somehow

static void saveAccessTokenCallback(void *ptr) {
	Common::JSONValue *json = (Common::JSONValue *)ptr;
	if (json) {
		debug("saveAccessTokenCallback:");
		debug("%s", json->stringify(true).c_str());

		Common::JSONObject result = json->asObject();
		if (!result.contains("access_token") || !result.contains("uid")) {
			warning("Bad response, no token/uid passed");
		} else {
			//we suppose that's the first storage
			ConfMan.set("storages_number", "1", "cloud");
			ConfMan.set("current_storage", "1", "cloud");
			ConfMan.set("storage1_type", "Dropbox", "cloud");
			ConfMan.set("storage1_access_token", result.getVal("access_token")->asString(), "cloud");
			ConfMan.set("storage1_user_id", result.getVal("uid")->asString(), "cloud");
			ConfMan.removeKey("dropbox_code", "cloud");
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
	ConfMan.set(keyPrefix + "type", "Dropbox", "cloud");
	ConfMan.set(keyPrefix + "access_token", _token, "cloud");
	ConfMan.set(keyPrefix + "user_id", _uid, "cloud");
}

void DropboxStorage::printFiles(Common::Array<StorageFile> files) {
	debug("files:");
	for (uint32 i = 0; i < files.size(); ++i)
		debug("\t%s", files[i].name().c_str());
}

int32 DropboxStorage::listDirectory(Common::String path, FileArrayCallback outerCallback, bool recursive) {
	return ConnMan.addRequest(new DropboxListDirectoryRequest(_token, path, outerCallback, recursive));
}

Networking::NetworkReadStream *DropboxStorage::streamFile(Common::String path) {
	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("path", new Common::JSONValue(path));
	Common::JSONValue value(jsonRequestParameters);

	Networking::CurlRequest *request = new Networking::CurlRequest(0, "https://content.dropboxapi.com/2/files/download");
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Dropbox-API-Arg: " + Common::JSON::stringify(&value));
	request->addHeader("Content-Type: "); //required to be empty (as we do POST, it's usually app/form-url-encoded)

	return request->execute();
}

int32 DropboxStorage::download(Common::String remotePath, Common::String localPath, BoolCallback callback) {
	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(localPath, true)) {
		warning("DropboxStorage: unable to open file to download into");
		if (callback) (*callback)(false);
		delete f;
		return -1;
	}

	return ConnMan.addRequest(new DownloadRequest(callback, streamFile(remotePath), f));
}

int32 DropboxStorage::syncSaves(BoolCallback callback) {
	//this is not the real syncSaves() implementation	
	//"" is root in Dropbox, not "/"
	//this must create all these directories:
	return download("/remote/test.jpg", "local/a/b/c/d/test.jpg", 0);
}

int32 DropboxStorage::info(StorageInfoCallback outerCallback) {
	Common::BaseCallback<> *innerCallback = new Common::CallbackBridge<DropboxStorage, StorageInfo>(this, &DropboxStorage::infoInnerCallback, outerCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, "https://api.dropboxapi.com/1/account/info");
	request->addHeader("Authorization: Bearer " + _token);
	return ConnMan.addRequest(request);
	//that callback bridge wraps the outerCallback (passed in arguments from user) into innerCallback
	//so, when CurlJsonRequest is finished, it calls the innerCallback
	//innerCallback (which is DropboxStorage::infoInnerCallback in this case) processes the void *ptr
	//and then calls the outerCallback (which wants to receive StorageInfo, not void *)
}

void DropboxStorage::infoInnerCallback(StorageInfoCallback outerCallback, void *jsonPointer) {
	Common::JSONValue *json = (Common::JSONValue *)jsonPointer;
	if (!json) {
		warning("NULL passed instead of JSON");
		delete outerCallback;
		return;
	}

	if (outerCallback) {		
		//Dropbox documentation states there is no errors for this API method
		Common::JSONObject info = json->asObject();
		Common::String uid = Common::String::format("%d", info.getVal("uid")->asNumber());
		Common::String name = info.getVal("display_name")->asString();
		Common::String email = info.getVal("email")->asString();
		Common::JSONObject quota = info.getVal("quota_info")->asObject();
		uint32 quotaNormal = quota.getVal("normal")->asNumber();
		uint32 quotaShared = quota.getVal("shared")->asNumber();
		uint32 quotaAllocated = quota.getVal("quota")->asNumber();		
		(*outerCallback)(StorageInfo(uid, name, email, quotaNormal+quotaShared, quotaAllocated));
		delete outerCallback;
	}
	
	delete json;
}

void DropboxStorage::infoMethodCallback(StorageInfo storageInfo) {
	debug("\nStorage info:");
	debug("User name: %s", storageInfo.name().c_str());
	debug("Email: %s", storageInfo.email().c_str());
	debug("Disk usage: %u/%u", storageInfo.used(), storageInfo.available());
}

DropboxStorage *DropboxStorage::loadFromConfig(Common::String keyPrefix) {
	KEY = ConfMan.get("DROPBOX_KEY", "cloud");
	SECRET = ConfMan.get("DROPBOX_SECRET", "cloud");

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
	url += "&client_id=" + KEY;
	return url;
}

void DropboxStorage::authThroughConsole() {
	if (!ConfMan.hasKey("DROPBOX_KEY", "cloud") || !ConfMan.hasKey("DROPBOX_SECRET", "cloud")) {
		warning("No Dropbox keys available, cannot do auth");
		return;
	}

	KEY = ConfMan.get("DROPBOX_KEY", "cloud");
	SECRET = ConfMan.get("DROPBOX_SECRET", "cloud");

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
	Common::BaseCallback<> *callback = new Common::GlobalFunctionCallback(saveAccessTokenCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, "https://api.dropboxapi.com/1/oauth2/token");
	request->addPostField("code=" + code);
	request->addPostField("grant_type=authorization_code");
	request->addPostField("client_id=" + KEY);
	request->addPostField("client_secret=" + SECRET);
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F");	
	ConnMan.addRequest(request);	
}

} //end of namespace Dropbox
} //end of namespace Cloud
