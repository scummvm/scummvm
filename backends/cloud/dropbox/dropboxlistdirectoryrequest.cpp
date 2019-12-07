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

#include "backends/cloud/dropbox/dropboxlistdirectoryrequest.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"
#include "common/debug.h"

namespace Cloud {
namespace Dropbox {

#define DROPBOX_API_LIST_FOLDER "https://api.dropboxapi.com/2/files/list_folder"
#define DROPBOX_API_LIST_FOLDER_CONTINUE "https://api.dropboxapi.com/2/files/list_folder/continue"

DropboxListDirectoryRequest::DropboxListDirectoryRequest(Common::String token, Common::String path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb), _requestedPath(path), _requestedRecursive(recursive), _listDirectoryCallback(cb),
	_token(token), _workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DropboxListDirectoryRequest::~DropboxListDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _listDirectoryCallback;
}

void DropboxListDirectoryRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_files.clear();
	_ignoreCallback = false;

	Networking::JsonCallback callback = new Common::Callback<DropboxListDirectoryRequest, Networking::JsonResponse>(this, &DropboxListDirectoryRequest::responseCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<DropboxListDirectoryRequest, Networking::ErrorResponse>(this, &DropboxListDirectoryRequest::errorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, failureCallback, DROPBOX_API_LIST_FOLDER);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/json");

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("path", new Common::JSONValue(_requestedPath));
	jsonRequestParameters.setVal("recursive", new Common::JSONValue(_requestedRecursive));
	jsonRequestParameters.setVal("include_media_info", new Common::JSONValue(false));
	jsonRequestParameters.setVal("include_deleted", new Common::JSONValue(false));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	_workingRequest = ConnMan.addRequest(request);
}

void DropboxListDirectoryRequest::responseCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;

	if (_ignoreCallback) {
		delete response.value;
		return;
	}

	if (response.request)
		_date = response.request->date();

	Networking::ErrorResponse error(this, "DropboxListDirectoryRequest::responseCallback: unknown error");
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq && rq->getNetworkReadStream())
		error.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	Common::JSONValue *json = response.value;
	if (json == nullptr) {
		error.response = "Failed to parse JSON, null passed!";
		finishError(error);
		return;
	}

	if (!json->isObject()) {
		error.response = "Passed JSON is not an object!";
		finishError(error);
		delete json;
		return;
	}

	Common::JSONObject responseObject = json->asObject();

	if (responseObject.contains("error") || responseObject.contains("error_summary")) {
		if (responseObject.contains("error_summary") && responseObject.getVal("error_summary")->isString()) {
			warning("Dropbox returned error: %s", responseObject.getVal("error_summary")->asString().c_str());
		}
		error.failed = true;
		error.response = json->stringify();
		finishError(error);
		delete json;
		return;
	}

	//check that ALL keys exist AND HAVE RIGHT TYPE to avoid segfaults
	if (responseObject.contains("entries")) {
		if (!responseObject.getVal("entries")->isArray()) {
			error.response = Common::String::format(
				"\"entries\" found, but that's not an array!\n%s",
				responseObject.getVal("entries")->stringify(true).c_str()
				);
			finishError(error);
			delete json;
			return;
		}

		Common::JSONArray items = responseObject.getVal("entries")->asArray();
		for (uint32 i = 0; i < items.size(); ++i) {
			if (!Networking::CurlJsonRequest::jsonIsObject(items[i], "DropboxListDirectoryRequest"))
				continue;

			Common::JSONObject item = items[i]->asObject();

			if (!Networking::CurlJsonRequest::jsonContainsString(item, "path_lower", "DropboxListDirectoryRequest"))
				continue;
			if (!Networking::CurlJsonRequest::jsonContainsString(item, ".tag", "DropboxListDirectoryRequest"))
				continue;

			Common::String path = item.getVal("path_lower")->asString();
			bool isDirectory = (item.getVal(".tag")->asString() == "folder");
			uint32 size = 0, timestamp = 0;
			if (!isDirectory) {
				if (!Networking::CurlJsonRequest::jsonContainsString(item, "server_modified", "DropboxListDirectoryRequest"))
					continue;
				if (!Networking::CurlJsonRequest::jsonContainsIntegerNumber(item, "size", "DropboxListDirectoryRequest"))
					continue;

				size = item.getVal("size")->asIntegerNumber();
				timestamp = ISO8601::convertToTimestamp(item.getVal("server_modified")->asString());
			}
			_files.push_back(StorageFile(path, size, timestamp, isDirectory));
		}
	}

	bool hasMore = false;
	if (responseObject.contains("has_more")) {
		if (!responseObject.getVal("has_more")->isBool()) {
			warning("DropboxListDirectoryRequest: \"has_more\" is not a boolean");
			debug(9, "%s", responseObject.getVal("has_more")->stringify(true).c_str());
			error.response = "\"has_more\" is not a boolean!";
			finishError(error);
			delete json;
			return;
		}

		hasMore = responseObject.getVal("has_more")->asBool();
	}

	if (hasMore) {
		if (!Networking::CurlJsonRequest::jsonContainsString(responseObject, "cursor", "DropboxListDirectoryRequest")) {
			error.response = "\"has_more\" found, but \"cursor\" is not (or it's not a string)!";
			finishError(error);
			delete json;
			return;
		}

		Networking::JsonCallback callback = new Common::Callback<DropboxListDirectoryRequest, Networking::JsonResponse>(this, &DropboxListDirectoryRequest::responseCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<DropboxListDirectoryRequest, Networking::ErrorResponse>(this, &DropboxListDirectoryRequest::errorCallback);
		Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, failureCallback, DROPBOX_API_LIST_FOLDER_CONTINUE);
		request->addHeader("Authorization: Bearer " + _token);
		request->addHeader("Content-Type: application/json");

		Common::JSONObject jsonRequestParameters;
		jsonRequestParameters.setVal("cursor", new Common::JSONValue(responseObject.getVal("cursor")->asString()));

		Common::JSONValue value(jsonRequestParameters);
		request->addPostField(Common::JSON::stringify(&value));

		_workingRequest = ConnMan.addRequest(request);
	} else {
		finishListing(_files);
	}

	delete json;
}

void DropboxListDirectoryRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void DropboxListDirectoryRequest::handle() {}

void DropboxListDirectoryRequest::restart() { start(); }

Common::String DropboxListDirectoryRequest::date() const { return _date; }

void DropboxListDirectoryRequest::finishListing(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback)
		(*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace Dropbox
} // End of namespace Cloud
