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

#include "backends/cloud/dropbox/dropboxuploadrequest.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

#define DROPBOX_API_FILES_UPLOAD "https://content.dropboxapi.com/2/files/upload"
#define DROPBOX_API_FILES_UPLOAD_SESSION "https://content.dropboxapi.com/2/files/upload_session/"

DropboxUploadRequest::DropboxUploadRequest(Common::String token, Common::String path, Common::SeekableReadStream *contents, Storage::UploadCallback callback, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _token(token), _savePath(path), _contentsStream(contents), _uploadCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DropboxUploadRequest::~DropboxUploadRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _contentsStream;
	delete _uploadCallback;
}

void DropboxUploadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	if (!_contentsStream) {
		warning("DropboxUploadRequest: cannot start because stream is invalid");
		finishError(Networking::ErrorResponse(this, false, true, "DropboxUploadRequest::start: cannot start because stream is invalid", -1));
		return;
	}
	if (!_contentsStream->seek(0)) {
		warning("DropboxUploadRequest: cannot restart because stream couldn't seek(0)");
		finishError(Networking::ErrorResponse(this, false, true, "DropboxUploadRequest::start: cannot restart because stream couldn't seek(0)", -1));
		return;
	}
	_ignoreCallback = false;

	uploadNextPart();
}

void DropboxUploadRequest::uploadNextPart() {
	const uint32 UPLOAD_PER_ONE_REQUEST = 10 * 1024 * 1024;

	Common::String url = DROPBOX_API_FILES_UPLOAD_SESSION;
	Common::JSONObject jsonRequestParameters;

	if (_contentsStream->pos() == 0 || _sessionId == "") {
		if ((uint32)_contentsStream->size() <= UPLOAD_PER_ONE_REQUEST) {
			url = DROPBOX_API_FILES_UPLOAD;
			jsonRequestParameters.setVal("path", new Common::JSONValue(_savePath));
			jsonRequestParameters.setVal("mode", new Common::JSONValue("overwrite"));
			jsonRequestParameters.setVal("autorename", new Common::JSONValue(false));
			jsonRequestParameters.setVal("mute", new Common::JSONValue(false));
		} else {
			url += "start";
			jsonRequestParameters.setVal("close", new Common::JSONValue(false));
		}
	} else {
		if ((uint32)(_contentsStream->size() - _contentsStream->pos()) <= UPLOAD_PER_ONE_REQUEST) {
			url += "finish";
			Common::JSONObject jsonCursor, jsonCommit;
			jsonCursor.setVal("session_id", new Common::JSONValue(_sessionId));
			jsonCursor.setVal("offset", new Common::JSONValue((long long int)_contentsStream->pos()));
			jsonCommit.setVal("path", new Common::JSONValue(_savePath));
			jsonCommit.setVal("mode", new Common::JSONValue("overwrite"));
			jsonCommit.setVal("autorename", new Common::JSONValue(false));
			jsonCommit.setVal("mute", new Common::JSONValue(false));
			jsonRequestParameters.setVal("cursor", new Common::JSONValue(jsonCursor));
			jsonRequestParameters.setVal("commit", new Common::JSONValue(jsonCommit));
		} else {
			url += "append_v2";
			Common::JSONObject jsonCursor;
			jsonCursor.setVal("session_id", new Common::JSONValue(_sessionId));
			jsonCursor.setVal("offset", new Common::JSONValue((long long int)_contentsStream->pos()));
			jsonRequestParameters.setVal("cursor", new Common::JSONValue(jsonCursor));
			jsonRequestParameters.setVal("close", new Common::JSONValue(false));
		}
	}

	Common::JSONValue value(jsonRequestParameters);
	Networking::JsonCallback callback = new Common::Callback<DropboxUploadRequest, Networking::JsonResponse>(this, &DropboxUploadRequest::partUploadedCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<DropboxUploadRequest, Networking::ErrorResponse>(this, &DropboxUploadRequest::partUploadedErrorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(callback, failureCallback, url);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/octet-stream");
	request->addHeader("Dropbox-API-Arg: " + Common::JSON::stringify(&value));

	byte *buffer = new byte[UPLOAD_PER_ONE_REQUEST];
	uint32 size = _contentsStream->read(buffer, UPLOAD_PER_ONE_REQUEST);
	request->setBuffer(buffer, size);

	_workingRequest = ConnMan.addRequest(request);
}

void DropboxUploadRequest::partUploadedCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Networking::ErrorResponse error(this, false, true, "", -1);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq && rq->getNetworkReadStream())
		error.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	Common::JSONValue *json = response.value;
	if (json == nullptr) {
		error.response = "Failed to parse JSON, null passed!";
		finishError(error);
		return;
	}

	bool needsFinishRequest = false;

	if (json->isObject()) {
		Common::JSONObject object = json->asObject();

		//debug(9, "%s", json->stringify(true).c_str());

		if (object.contains("error") || object.contains("error_summary")) {
			if (Networking::CurlJsonRequest::jsonContainsString(object, "error_summary", "DropboxUploadRequest")) {
				warning("Dropbox returned error: %s", object.getVal("error_summary")->asString().c_str());
			}
			error.response = json->stringify(true);
			finishError(error);
			delete json;
			return;
		}

		if (Networking::CurlJsonRequest::jsonContainsString(object, "path_lower", "DropboxUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsString(object, "server_modified", "DropboxUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsIntegerNumber(object, "size", "DropboxUploadRequest")) {
			//finished
			Common::String path = object.getVal("path_lower")->asString();
			uint32 size = object.getVal("size")->asIntegerNumber();
			uint32 timestamp = ISO8601::convertToTimestamp(object.getVal("server_modified")->asString());
			finishUpload(StorageFile(path, size, timestamp, false));
			return;
		}

		if (_sessionId == "") {
			if (Networking::CurlJsonRequest::jsonContainsString(object, "session_id", "DropboxUploadRequest"))
				_sessionId = object.getVal("session_id")->asString();
			needsFinishRequest = true;
		}
	}

	if (!needsFinishRequest && (_contentsStream->eos() || _contentsStream->pos() >= _contentsStream->size() - 1)) {
		warning("DropboxUploadRequest: no file info to return");
		finishUpload(StorageFile(_savePath, 0, 0, false));
	} else {
		uploadNextPart();
	}

	delete json;
}

void DropboxUploadRequest::partUploadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void DropboxUploadRequest::handle() {}

void DropboxUploadRequest::restart() { start(); }

void DropboxUploadRequest::finishUpload(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback)
		(*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace Dropbox
} // End of namespace Cloud
