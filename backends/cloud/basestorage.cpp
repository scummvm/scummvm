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

BaseStorage::BaseStorage(Common::String token, Common::String refreshToken, bool enabled):
	_token(token), _refreshToken(refreshToken) {
	_isEnabled = enabled; 
}

BaseStorage::~BaseStorage() {}

void BaseStorage::getAccessToken(Common::String code, Networking::ErrorCallback callback) {
	Networking::JsonCallback innerCallback = new Common::CallbackBridge<BaseStorage, Networking::ErrorResponse, Networking::JsonResponse>(this, &BaseStorage::codeFlowComplete, callback);
	Networking::ErrorCallback errorCallback = new Common::CallbackBridge<BaseStorage, Networking::ErrorResponse, Networking::ErrorResponse>(this, &BaseStorage::codeFlowFailed, callback);

	Common::String url = Common::String::format("https://cloud.scummvm.org/%s/token/%s", cloudProvider().c_str(), code.c_str());
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, url);

	addRequest(request);
}

void BaseStorage::codeFlowComplete(Networking::ErrorCallback callback, Networking::JsonResponse response) {
	bool success = true;
	Common::String callbackMessage = "OK";

	Common::JSONValue *json = (Common::JSONValue *)response.value;
	if (json == nullptr) {
		debug(9, "BaseStorage::codeFlowComplete: got NULL instead of JSON!");
		success = false;
	}

	if (success && !json->isObject()) {
		debug(9, "BaseStorage::codeFlowComplete: passed JSON is not an object!");
		success = false;
	}

	Common::JSONObject result;
	if (success) {
		result = json->asObject();
		if (!Networking::CurlJsonRequest::jsonContainsAttribute(result, "error", "BaseStorage::codeFlowComplete")) {
			warning("BaseStorage: bad response, no 'error' attribute passed");
			debug(9, "%s", json->stringify(true).c_str());
			success = false;
		}
	}

	if (success && result.getVal("error")->asBool()) {
		Common::String errorMessage = "{error: true}, message is missing";
		if (Networking::CurlJsonRequest::jsonContainsString(result, "message", "BaseStorage::codeFlowComplete")) {
			errorMessage = result.getVal("message")->asString();
		}
		warning("BaseStorage: response says error occurred: %s", errorMessage.c_str());
		success = false;
		callbackMessage = errorMessage;
	}

	if (success && !Networking::CurlJsonRequest::jsonContainsObject(result, "oauth", "BaseStorage::codeFlowComplete")) {
		warning("BaseStorage: bad response, no 'oauth' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		success = false;
	}

	Common::JSONObject oauth;
	bool requiresRefreshToken = needsRefreshToken();
	if (success) {
		oauth = result.getVal("oauth")->asObject();
		if (!Networking::CurlJsonRequest::jsonContainsString(oauth, "access_token", "BaseStorage::codeFlowComplete") ||
			!Networking::CurlJsonRequest::jsonContainsString(oauth, "refresh_token", "BaseStorage::codeFlowComplete", !requiresRefreshToken)) {
			warning("BaseStorage: bad response, no 'access_token' or 'refresh_token' attribute passed");
			debug(9, "%s", json->stringify(true).c_str());
			success = false;
		}
	}

	if (success) {
		debug(9, "%s", json->stringify(true).c_str()); // TODO: remove when done testing against cloud.scummvm.org
		_token = oauth.getVal("access_token")->asString();
		if (requiresRefreshToken) {
			_refreshToken = oauth.getVal("refresh_token")->asString();
		}
		CloudMan.replaceStorage(this, storageIndex());
		ConfMan.flushToDisk();
	}

	if (!success)
		CloudMan.removeStorage(this);
	if (callback)
		(*callback)(Networking::ErrorResponse(nullptr, false, !success, callbackMessage, -1));
	delete json;
	delete callback;
}

void BaseStorage::codeFlowFailed(Networking::ErrorCallback callback, Networking::ErrorResponse error) {
	debug(9, "BaseStorage: code flow failed (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug(9, "%s", error.response.c_str());
	CloudMan.removeStorage(this);

	if (callback)
		(*callback)(error);
	delete callback;
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

	Common::String url = Common::String::format("https://cloud.scummvm.org/%s/refresh", cloudProvider().c_str());
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorCallback, url);
	request->addHeader("X-ScummVM-Refresh-Token: " + _refreshToken);
	addRequest(request);
}

void BaseStorage::tokenRefreshed(BoolCallback callback, Networking::JsonResponse response) {
	bool success = true;

	Common::JSONValue *json = response.value;
	if (json == nullptr) {
		debug(9, "BaseStorage::tokenRefreshed: got NULL instead of JSON!");
		success = false;
	}

	if (success && !json->isObject()) {
		debug(9, "BaseStorage::tokenRefreshed: passed JSON is not an object!");
		success = false;
	}

	Common::JSONObject result;
	if (success) {
		result = json->asObject();
		if (!Networking::CurlJsonRequest::jsonContainsAttribute(result, "error", "BaseStorage::tokenRefreshed")) {
			warning("BaseStorage: bad response, no 'error' attribute passed");
			debug(9, "%s", json->stringify(true).c_str());
			success = false;
		}
	}

	if (success && result.getVal("error")->asBool()) {
		Common::String errorMessage = "{error: true}, message is missing";
		if (Networking::CurlJsonRequest::jsonContainsString(result, "message", "BaseStorage::tokenRefreshed")) {
			errorMessage = result.getVal("message")->asString();
		}
		warning("BaseStorage: response says error occurred: %s", errorMessage.c_str());
		success = false;
	}

	if (success && !Networking::CurlJsonRequest::jsonContainsObject(result, "oauth", "BaseStorage::tokenRefreshed")) {
		warning("BaseStorage: bad response, no 'oauth' attribute passed");
		debug(9, "%s", json->stringify(true).c_str());
		success = false;
	}

	Common::JSONObject oauth;
	bool requiresRefreshToken = !canReuseRefreshToken();
	if (success) {
		oauth = result.getVal("oauth")->asObject();		
		if (!Networking::CurlJsonRequest::jsonContainsString(oauth, "access_token", "BaseStorage::tokenRefreshed") ||
			!Networking::CurlJsonRequest::jsonContainsString(oauth, "refresh_token", "BaseStorage::tokenRefreshed", !requiresRefreshToken)) {
			warning("BaseStorage: bad response, no 'access_token' or 'refresh_token' attribute passed");
			debug(9, "%s", json->stringify(true).c_str());
			success = false;
		}
	}

	if (success) {
		debug(9, "%s", json->stringify(true).c_str()); // TODO: remove when done testing against cloud.scummvm.org

		_token = oauth.getVal("access_token")->asString();
		if (requiresRefreshToken) {
			_refreshToken = oauth.getVal("refresh_token")->asString();
		}
		CloudMan.save(); //ask CloudManager to save our new access_token and refresh_token
	}

	if (callback)
		(*callback)(BoolResponse(nullptr, success));
	delete json;
	delete callback;
}

void BaseStorage::saveIsEnabledFlag(const Common::String &keyPrefix) const {
	ConfMan.set(keyPrefix + "enabled", _isEnabled ? "true" : "false", ConfMan.kCloudDomain);
}

bool BaseStorage::loadIsEnabledFlag(const Common::String &keyPrefix) {
	if (!ConfMan.hasKey(keyPrefix + "enabled", ConfMan.kCloudDomain))
		return false;

	Common::String enabled = ConfMan.get(keyPrefix + "enabled", ConfMan.kCloudDomain);
	return (enabled == "true");
}

void BaseStorage::removeIsEnabledFlag(const Common::String &keyPrefix) {
	ConfMan.removeKey(keyPrefix + "enabled", ConfMan.kCloudDomain);
}

} // End of namespace Cloud
