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

#include "backends/cloud/box/boxuploadrequest.h"
#include "backends/cloud/box/boxstorage.h"
#include "backends/cloud/box/boxtokenrefresher.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace Box {

#define BOX_API_FILES "https://upload.box.com/api/2.0/files"

BoxUploadRequest::BoxUploadRequest(BoxStorage *storage, Common::String path, Common::String localPath, Storage::UploadCallback callback, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _storage(storage), _savePath(path), _localPath(localPath), _uploadCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

BoxUploadRequest::~BoxUploadRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _uploadCallback;
}

void BoxUploadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_resolvedId = ""; //used to update file contents
	_parentId = ""; //used to create file within parent directory
	_ignoreCallback = false;

	resolveId();
}

void BoxUploadRequest::resolveId() {
	//check whether such file already exists
	Storage::UploadCallback innerCallback = new Common::Callback<BoxUploadRequest, Storage::UploadResponse>(this, &BoxUploadRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<BoxUploadRequest, Networking::ErrorResponse>(this, &BoxUploadRequest::idResolveFailedCallback);
	_workingRequest = _storage->resolveFileId(_savePath, innerCallback, innerErrorCallback);
}

void BoxUploadRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	_resolvedId = response.value.id();
	upload();
}

void BoxUploadRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//not resolved => error or no such file
	if (error.response.contains("no such file found in its parent directory")) {
		//parent's id after the '\n'
		Common::String parentId = error.response;
		for (uint32 i = 0; i < parentId.size(); ++i)
			if (parentId[i] == '\n') {
				parentId.erase(0, i + 1);
				break;
			}

		_parentId = parentId;
		upload();
		return;
	}

	finishError(error);
}

void BoxUploadRequest::upload() {
	Common::String name = _savePath;
	for (uint32 i = name.size(); i > 0; --i) {
		if (name[i - 1] == '/' || name[i - 1] == '\\') {
			name.erase(0, i);
			break;
		}
	}

	Common::String url = BOX_API_FILES;
	if (_resolvedId != "")
		url += "/" + _resolvedId;
	url += "/content";
	Networking::JsonCallback callback = new Common::Callback<BoxUploadRequest, Networking::JsonResponse>(this, &BoxUploadRequest::uploadedCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<BoxUploadRequest, Networking::ErrorResponse>(this, &BoxUploadRequest::notUploadedCallback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());

	Common::JSONObject jsonRequestParameters;
	if (_resolvedId == "") {
		Common::JSONObject parentObject;
		parentObject.setVal("id", new Common::JSONValue(_parentId));
		jsonRequestParameters.setVal("parent", new Common::JSONValue(parentObject));
		jsonRequestParameters.setVal("name", new Common::JSONValue(name));
	}

	Common::JSONValue value(jsonRequestParameters);
	request->addFormField("attributes", Common::JSON::stringify(&value));
	request->addFormFile("file", _localPath);

	_workingRequest = ConnMan.addRequest(request);
}

void BoxUploadRequest::uploadedCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	Networking::ErrorResponse error(this, false, true, "", -1);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq) {
		const Networking::NetworkReadStream *stream = rq->getNetworkReadStream();
		if (stream) {
			long code = stream->httpResponseCode();
			error.httpResponseCode = code;
		}
	}

	if (error.httpResponseCode != 200 && error.httpResponseCode != 201)
		warning("BoxUploadRequest: looks like an error (bad HTTP code)");

	//check JSON and show warnings if it's malformed
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

	Common::JSONObject object = json->asObject();
	if (Networking::CurlJsonRequest::jsonContainsArray(object, "entries", "BoxUploadRequest")) {
		Common::JSONArray entries = object.getVal("entries")->asArray();
		if (entries.size() == 0) {
			warning("BoxUploadRequest: 'entries' found, but it's empty");
		} else if (!Networking::CurlJsonRequest::jsonIsObject(entries[0], "BoxUploadRequest")) {
			warning("BoxUploadRequest: 'entries' first item is not an object");
		} else {
			Common::JSONObject item = entries[0]->asObject();

			if (Networking::CurlJsonRequest::jsonContainsString(item, "id", "BoxUploadRequest") &&
				Networking::CurlJsonRequest::jsonContainsString(item, "name", "BoxUploadRequest") &&
				Networking::CurlJsonRequest::jsonContainsString(item, "type", "BoxUploadRequest") &&
				Networking::CurlJsonRequest::jsonContainsString(item, "modified_at", "BoxUploadRequest") &&
				Networking::CurlJsonRequest::jsonContainsStringOrIntegerNumber(item, "size", "BoxUploadRequest")) {

				//finished
				Common::String id = item.getVal("id")->asString();
				Common::String name = item.getVal("name")->asString();
				bool isDirectory = (item.getVal("type")->asString() == "folder");
				uint32 size;
				if (item.getVal("size")->isString()) {
					size = item.getVal("size")->asString().asUint64();
				} else {
					size = item.getVal("size")->asIntegerNumber();
				}
				uint32 timestamp = ISO8601::convertToTimestamp(item.getVal("modified_at")->asString());

				finishUpload(StorageFile(id, _savePath, name, size, timestamp, isDirectory));
				delete json;
				return;
			}
		}
	}

	//TODO: check errors
	/*
	if (object.contains("error")) {
		warning("Box returned error: %s", json->stringify(true).c_str());
		delete json;
		error.response = json->stringify(true);
		finishError(error);
		return;
	}
	*/

	warning("BoxUploadRequest: no file info to return");
	finishUpload(StorageFile(_savePath, 0, 0, false));

	delete json;
}

void BoxUploadRequest::notUploadedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void BoxUploadRequest::handle() {}

void BoxUploadRequest::restart() { start(); }

void BoxUploadRequest::finishUpload(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback)
		(*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace Box
} // End of namespace Cloud
