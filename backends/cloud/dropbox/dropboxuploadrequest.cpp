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
#include "common/debug.h"

namespace Cloud {
namespace Dropbox {

DropboxUploadRequest::DropboxUploadRequest(Common::String token, Common::String path, Common::SeekableReadStream *contents, Storage::UploadCallback callback):
	Networking::Request(0), _token(token), _savePath(path), _contentsStream(contents), _uploadCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DropboxUploadRequest::~DropboxUploadRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _contentsStream;
	delete _uploadCallback;
}


void DropboxUploadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	if (!_contentsStream->seek(0)) {
		warning("DropboxUploadRequest: cannot restart because stream couldn't seek(0)");
		finish();
	}
	_ignoreCallback = false;

	uploadNextPart();
}

void DropboxUploadRequest::uploadNextPart() {	
	const uint32 UPLOAD_PER_ONE_REQUEST = 10 * 1024 * 1024;
	
	Common::String url = "https://content.dropboxapi.com/2/files/upload_session/";
	Common::JSONObject jsonRequestParameters;

	if (_contentsStream->pos() == 0 || _sessionId == "") {
		if (_contentsStream->size() <= UPLOAD_PER_ONE_REQUEST) {
			url = "https://content.dropboxapi.com/2/files/upload";
			jsonRequestParameters.setVal("path", new Common::JSONValue(_savePath));
			jsonRequestParameters.setVal("mode", new Common::JSONValue("overwrite"));
			jsonRequestParameters.setVal("autorename", new Common::JSONValue(false));
			jsonRequestParameters.setVal("mute", new Common::JSONValue(false));
		} else {
			url += "start";
			jsonRequestParameters.setVal("close", new Common::JSONValue(false));
		}
	} else {
		if (_contentsStream->size() - _contentsStream->pos() <= UPLOAD_PER_ONE_REQUEST) {
			url += "finish";			
			Common::JSONObject jsonCursor, jsonCommit;			
			jsonCursor.setVal("session_id", new Common::JSONValue(_sessionId));
			jsonCursor.setVal("offset", new Common::JSONValue(_contentsStream->pos()));
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
			jsonCursor.setVal("offset", new Common::JSONValue(_contentsStream->pos()));
			jsonRequestParameters.setVal("cursor", new Common::JSONValue(jsonCursor));
			jsonRequestParameters.setVal("close", new Common::JSONValue(false));			
		}
	}
	
	Common::JSONValue value(jsonRequestParameters);
	Networking::JsonCallback innerCallback = new Common::Callback<DropboxUploadRequest, Networking::JsonResponse>(this, &DropboxUploadRequest::partUploadedCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, url);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/octet-stream");	
	request->addHeader("Dropbox-API-Arg: " + Common::JSON::stringify(&value));	

	byte *buffer = new byte[UPLOAD_PER_ONE_REQUEST];
	uint32 size = _contentsStream->read(buffer, UPLOAD_PER_ONE_REQUEST);
	request->setBuffer(buffer, size);
	
	_workingRequest = ConnMan.addRequest(request);
}

void DropboxUploadRequest::partUploadedCallback(Networking::JsonResponse pair) {
	if (_ignoreCallback) return;
	_workingRequest = nullptr;

	UploadStatus status;
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)pair.request;
	if (rq && rq->getNetworkReadStream())
		status.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	Common::JSONValue *json = pair.value;
	if (json) {
		bool needsFinishRequest = false;

		if (json->isObject()) {
			Common::JSONObject response = json->asObject();

			//debug("%s", json->stringify(true).c_str());

			if (response.contains("error") || response.contains("error_summary")) {
				warning("Dropbox returned error: %s", response.getVal("error_summary")->asString().c_str());
				delete json;
				status.failed = true;
				status.response = json->stringify(true);
				finishUpload(status);
				return;
			}

			if (response.contains("server_modified")) {
				//finished
				Common::String path = response.getVal("path_lower")->asString();				
				uint32 size = response.getVal("size")->asIntegerNumber();
				uint32 timestamp = ISO8601::convertToTimestamp(response.getVal("server_modified")->asString());				
				status.file = StorageFile(path, size, timestamp, false);
				finishUpload(status);
				return;
			}

			if (_sessionId == "") {
				if (response.contains("session_id"))
					_sessionId = response.getVal("session_id")->asString();
				else
					warning("no session_id found in Dropbox's response");
				needsFinishRequest = true;
			}
		}

		if (!needsFinishRequest && (_contentsStream->eos() || _contentsStream->pos() >= _contentsStream->size() - 1)) {			
			if (status.file.name() == "") {
				status.file = StorageFile(_savePath, 0, 0, false);
				warning("no file info to put into status");
			}
			finishUpload(status);
		} else {
			uploadNextPart();
		}
	} else {
		warning("null, not json");
		status.failed = true;
		finishUpload(status);
	}

	delete json;
}

void DropboxUploadRequest::handle() {}

void DropboxUploadRequest::restart() { start(); }

void DropboxUploadRequest::finish() {
	UploadStatus status;
	status.interrupted = true;
	finishUpload(status);
}

void DropboxUploadRequest::finishUpload(UploadStatus status) {
	Request::finish();
	if (_uploadCallback) (*_uploadCallback)(Storage::UploadResponse(this, status));
}

} // End of namespace Dropbox
} // End of namespace Cloud
