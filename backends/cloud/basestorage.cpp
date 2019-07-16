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

#include "backends/cloud/basestorage.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {

BaseStorage::BaseStorage() {}

BaseStorage::BaseStorage(Common::String token, Common::String refreshToken):
	_token(token), _refreshToken(refreshToken) {}

BaseStorage::~BaseStorage() {}

void BaseStorage::getAccessToken(Common::String code) {
	Networking::JsonCallback callback = new Common::Callback<BaseStorage, Networking::JsonResponse>(this, &BaseStorage::codeFlowComplete);
	Networking::ErrorCallback errorCallback = new Common::Callback<BaseStorage, Networking::ErrorResponse>(this, &BaseStorage::codeFlowFailed);

	Common::String url = Common::String::format("https://cloud.scummvm.org/%s/token/%s", cloudProvider().c_str(), code.c_str());
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, errorCallback, url);

	addRequest(request);
}

void BaseStorage::codeFlowComplete(Networking::JsonResponse response) {
	Common::JSONValue *json = (Common::JSONValue *)response.value;
	if (json == nullptr) {
		debug(9, "BaseStorage::codeFlowComplete: got NULL instead of JSON!");
		CloudMan.removeStorage(this);
		return;
	}

	if (!json->isObject()) {
		debug(9, "BaseStorage::codeFlowComplete: passed JSON is not an object!");
		CloudMan.removeStorage(this);
		delete json;
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!Networking::CurlJsonRequest::jsonContainsAttribute(result, "error", "BaseStorage::codeFlowComplete")) {
		warning("BaseStorage: bad response, no 'error' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		CloudMan.removeStorage(this);
		delete json;
		return;
	}

	if (result.getVal("error")->asBool()) {
		Common::String errorMessage = "{error: true}, message is missing";
		if (Networking::CurlJsonRequest::jsonContainsString(result, "message", "BaseStorage::codeFlowComplete")) {
			errorMessage = result.getVal("message")->asString();
		}
		warning("BaseStorage: response says error occurred: %s", errorMessage.c_str());
		CloudMan.removeStorage(this);
		delete json;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonContainsObject(result, "oauth", "BaseStorage::codeFlowComplete")) {
		warning("BaseStorage: bad response, no 'oauth' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		CloudMan.removeStorage(this);
		delete json;
		return;
	}

	Common::JSONObject oauth = result.getVal("oauth")->asObject();
	bool requiresRefreshToken = needsRefreshToken();
	if (!Networking::CurlJsonRequest::jsonContainsString(oauth, "access_token", "BaseStorage::codeFlowComplete") ||
		!Networking::CurlJsonRequest::jsonContainsString(oauth, "refresh_token", "BaseStorage::codeFlowComplete", !requiresRefreshToken)) {
		warning("BaseStorage: bad response, no 'access_token' or 'refresh_token' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		CloudMan.removeStorage(this);
		delete json;
		return;
	}

	debug(9, "%s", json->stringify(true).c_str()); // TODO: remove when done testing against cloud.scummvm.org
	_token = oauth.getVal("access_token")->asString();
	if (requiresRefreshToken) {
		_refreshToken = oauth.getVal("refresh_token")->asString();
	}
	CloudMan.replaceStorage(this, storageIndex());
	ConfMan.flushToDisk();

	delete json;
}

void BaseStorage::codeFlowFailed(Networking::ErrorResponse error) {
	debug(9, "BaseStorage: code flow failed (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug(9, "%s", error.response.c_str());
	CloudMan.removeStorage(this);
}

void BaseStorage::refreshAccessToken(BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (_refreshToken == "") {
		warning("BaseStorage: no refresh token available to get new access token.");
		if (callback) (*callback)(BoolResponse(nullptr, false));
		return;
	}

	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BaseStorage, BoolResponse, Networking::JsonResponse>(this, &BaseStorage::tokenRefreshed, callback);
	if (errorCallback == nullptr)
		errorCallback = getErrorPrintingCallback();

	Common::String url = Common::String::format("https://cloud.scummvm.org/%s/refresh?code=%s", cloudProvider().c_str(), _refreshToken.c_str());
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, url);
	addRequest(request);
}

void BaseStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	if (json == nullptr) {
		debug(9, "BaseStorage::tokenRefreshed: got NULL instead of JSON!");
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete callback;
		return;
	}

	if (!json->isObject()) {
		debug(9, "BaseStorage::tokenRefreshed: passed JSON is not an object!");
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	Common::JSONObject result = json->asObject();
	if (!Networking::CurlJsonRequest::jsonContainsAttribute(result, "error", "BaseStorage::tokenRefreshed")) {
		warning("BaseStorage: bad response, no 'error' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	if (result.getVal("error")->asBool()) {
		Common::String errorMessage = "{error: true}, message is missing";
		if (Networking::CurlJsonRequest::jsonContainsString(result, "message", "BaseStorage::tokenRefreshed")) {
			errorMessage = result.getVal("message")->asString();
		}
		warning("BaseStorage: response says error occurred: %s", errorMessage.c_str());
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	if (!Networking::CurlJsonRequest::jsonContainsObject(result, "oauth", "BaseStorage::tokenRefreshed")) {
		warning("BaseStorage: bad response, no 'oauth' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	Common::JSONObject oauth = result.getVal("oauth")->asObject();
	bool requiresRefreshToken = !canReuseRefreshToken();
	if (!Networking::CurlJsonRequest::jsonContainsString(oauth, "access_token", "BaseStorage::tokenRefreshed") ||
		!Networking::CurlJsonRequest::jsonContainsString(oauth, "refresh_token", "BaseStorage::tokenRefreshed", !requiresRefreshToken)) {
		warning("BaseStorage: bad response, no 'access_token' or 'refresh_token' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		if (callback)
			(*callback)(BoolResponse(nullptr, false));
		delete json;
		delete callback;
		return;
	}

	debug(9, "%s", json->stringify(true).c_str()); // TODO: remove when done testing against cloud.scummvm.org

	_token = oauth.getVal("access_token")->asString();
	if (requiresRefreshToken) {
		_refreshToken = oauth.getVal("refresh_token")->asString();
	}
	CloudMan.save(); //ask CloudManager to save our new access_token and refresh_token
	if (callback)
		(*callback)(BoolResponse(nullptr, true));
	delete json;
	delete callback;
}

} // End of namespace Cloud
