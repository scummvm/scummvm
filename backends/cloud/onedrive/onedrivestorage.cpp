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
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>
#include <common/file.h>
#include "common/system.h"
#include "common/cloudmanager.h"

namespace Cloud {
namespace OneDrive {

Common::String OneDriveStorage::KEY; //can't use ConfMan there yet, loading it on instance creation/auth
Common::String OneDriveStorage::SECRET; //TODO: hide these secrets somehow

OneDriveStorage::OneDriveStorage(Common::String accessToken, Common::String userId, Common::String refreshToken):
	_token(accessToken), _uid(userId), _refreshToken(refreshToken) {}

OneDriveStorage::OneDriveStorage(Common::String code) {
	getAccessToken(new Common::Callback<OneDriveStorage, bool>(this, &OneDriveStorage::codeFlowComplete), code);
}

OneDriveStorage::~OneDriveStorage() {}

void OneDriveStorage::getAccessToken(BoolCallback callback, Common::String code) {
	bool codeFlow = (code != "");

	if (!codeFlow && _refreshToken == "") {
		warning("OneDriveStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(false);
		return;
	}

	Common::BaseCallback<> *innerCallback = new Common::CallbackBridge<OneDriveStorage, bool>(this, &OneDriveStorage::tokenRefreshed, callback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, "https://login.live.com/oauth20_token.srf");
	if (codeFlow) {
		request->addPostField("code=" + code);
		request->addPostField("grant_type=authorization_code");
	} else {
		request->addPostField("refresh_token=" + _refreshToken);
		request->addPostField("grant_type=refresh_token");
	}
	request->addPostField("client_id=" + KEY);
	request->addPostField("client_secret=" + SECRET);
	request->addPostField("&redirect_uri=http%3A%2F%2Flocalhost%3A12345%2F");
	ConnMan.addRequest(request);
}

void OneDriveStorage::tokenRefreshed(BoolCallback callback, void *jsonPointer) {
	Common::JSONValue *json = (Common::JSONValue *)jsonPointer;
	if (!json) {
		warning("OneDriveStorage: got NULL instead of JSON");
		if (callback) (*callback)(false);
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!result.contains("access_token") || !result.contains("user_id") || !result.contains("refresh_token")) {
		warning("Bad response, no token or user_id passed");
		debug("%s", json->stringify().c_str());
		if (callback) (*callback)(false);
	} else {
		_token = result.getVal("access_token")->asString();
		_uid = result.getVal("user_id")->asString();
		_refreshToken = result.getVal("refresh_token")->asString();
		g_system->getCloudManager()->save(); //ask CloudManager to save our new refreshToken
		if (callback) (*callback)(true);
	}
	delete json;
}

void OneDriveStorage::codeFlowComplete(bool success) {
	if (!success) {
		warning("OneDriveStorage: failed to get access token through code flow");
		return;
	}

	g_system->getCloudManager()->addStorage(this);
	ConfMan.removeKey("onedrive_code", "cloud");
	debug("Done! You can use OneDrive now! Look:");
	g_system->getCloudManager()->syncSaves();
}

void OneDriveStorage::saveConfig(Common::String keyPrefix) {
	ConfMan.set(keyPrefix + "type", "OneDrive", "cloud");
	ConfMan.set(keyPrefix + "access_token", _token, "cloud");
	ConfMan.set(keyPrefix + "user_id", _uid, "cloud");
	ConfMan.set(keyPrefix + "refresh_token", _refreshToken, "cloud");
}

void OneDriveStorage::printJsonTokenReceived(bool success) {
	if (success) syncSaves(0); //try again
}

void OneDriveStorage::printJson(void *jsonPointer) {
	Common::JSONValue *json = (Common::JSONValue *)jsonPointer;
	if (!json) {
		warning("printJson: NULL");
		return;
	}

	Common::JSONObject result = json->asObject();
	if (result.contains("error")) {
		//Common::JSONObject error = result.getVal("error")->asObject();
		debug("bad token, trying again...");
		getAccessToken(new Common::Callback<OneDriveStorage, bool>(this, &OneDriveStorage::printJsonTokenReceived));
		delete json;
		return;
	}

	debug("%s", json->stringify().c_str());
	delete json;
}

int32 OneDriveStorage::syncSaves(BoolCallback callback) {
	//this is not the real syncSaves() implementation	
	Common::BaseCallback<> *innerCallback = new Common::Callback<OneDriveStorage>(this, &OneDriveStorage::printJson);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, "https://api.onedrive.com/v1.0/drives/");	
	request->addHeader("Authorization: bearer " + _token);
	return ConnMan.addRequest(request);
}

OneDriveStorage *OneDriveStorage::loadFromConfig(Common::String keyPrefix) {
	KEY = ConfMan.get("ONEDRIVE_KEY", "cloud");
	SECRET = ConfMan.get("ONEDRIVE_SECRET", "cloud");

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
	url += "&client_id=" + KEY;
	url += "&scope=onedrive.appfolder%20offline_access"; //TODO
	return url;
}

void OneDriveStorage::authThroughConsole() {
	if (!ConfMan.hasKey("ONEDRIVE_KEY", "cloud") || !ConfMan.hasKey("ONEDRIVE_SECRET", "cloud")) {
		warning("No OneDrive keys available, cannot do auth");
		return;
	}

	KEY = ConfMan.get("ONEDRIVE_KEY", "cloud");
	SECRET = ConfMan.get("ONEDRIVE_SECRET", "cloud");

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

} //end of namespace OneDrive
} //end of namespace Cloud
