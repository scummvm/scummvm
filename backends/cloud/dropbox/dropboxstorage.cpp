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
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

Common::String DropboxStorage::KEY; //can't use ConfMan there yet, loading it on instance creation/auth
Common::String DropboxStorage::SECRET; //TODO: hide these secrets somehow

static void printJsonCallback(void *ptr) {
	Common::JSONValue *json = (Common::JSONValue *)ptr;
	if (json) {
		debug("printJsonCallback:");
		debug("%s", json->stringify(true).c_str());
		delete json;
	} else {
		debug("printJsonCallback: got NULL instead of JSON!");
	}
}

static void saveAccessTokenCallback(void *ptr) {
	Common::JSONValue *json = (Common::JSONValue *)ptr;
	if (json) {
		debug("saveAccessTokenCallback:");
		debug("%s", json->stringify(true).c_str());

		Common::JSONObject result = json->asObject();
		if (!result.contains("access_token") || !result.contains("uid")) {
			warning("Bad response, no token/uid passed");
		} else {
			ConfMan.set("current_storage_type", "Dropbox", "cloud");
			ConfMan.set("current_storage_access_token", result.getVal("access_token")->asString(), "cloud");
			ConfMan.set("current_storage_user_id", result.getVal("uid")->asString(), "cloud");
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

void DropboxStorage::listDirectory(Common::String path) {	
}

void DropboxStorage::syncSaves() {
	//not syncing, but already something:
	printInfo();
}

void DropboxStorage::printInfo() {
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(printJsonCallback, "https://api.dropboxapi.com/1/account/info");
	request->addHeader("Authorization: Bearer " + _token);
	ConnMan.addRequest(request);
}

DropboxStorage *DropboxStorage::loadFromConfig() {
	KEY = ConfMan.get("DROPBOX_KEY", "cloud");
	SECRET = ConfMan.get("DROPBOX_SECRET", "cloud");

	if (!ConfMan.hasKey("current_storage_access_token", "cloud")) {
		warning("No access_token found");
		return 0;
	}

	if (!ConfMan.hasKey("current_storage_user_id", "cloud")) {
		warning("No user_id found");
		return 0;
	}

	Common::String accessToken = ConfMan.get("current_storage_access_token", "cloud");
	Common::String userId = ConfMan.get("current_storage_user_id", "cloud");
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
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(saveAccessTokenCallback, "https://api.dropboxapi.com/1/oauth2/token");
	request->addPostField("code=" + code);
	request->addPostField("grant_type=authorization_code");
	request->addPostField("client_id=" + KEY);
	request->addPostField("client_secret=" + SECRET);
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F");	
	ConnMan.addRequest(request);	
}

} //end of namespace Dropbox
} //end of namespace Cloud
