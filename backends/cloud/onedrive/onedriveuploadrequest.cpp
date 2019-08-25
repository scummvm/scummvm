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

#include "backends/cloud/onedrive/onedriveuploadrequest.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"
#include "onedrivetokenrefresher.h"

namespace Cloud {
namespace OneDrive {

#define ONEDRIVE_API_SPECIAL_APPROOT_UPLOAD "https://graph.microsoft.com/v1.0/drive/special/approot:/%s:/upload.createSession"
#define ONEDRIVE_API_SPECIAL_APPROOT_CONTENT "https://graph.microsoft.com/v1.0/drive/special/approot:/%s:/content"

OneDriveUploadRequest::OneDriveUploadRequest(OneDriveStorage *storage, Common::String path, Common::SeekableReadStream *contents, Storage::UploadCallback callback, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _storage(storage), _savePath(path), _contentsStream(contents), _uploadCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

OneDriveUploadRequest::~OneDriveUploadRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _contentsStream;
	delete _uploadCallback;
}

void OneDriveUploadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	if (_contentsStream == nullptr) {
		warning("OneDriveUploadRequest: cannot restart because no stream given");
		finishError(Networking::ErrorResponse(this, false, true, "OneDriveUploadRequest::start: can't restart, because no stream given", -1));
		return;
	}
	if (!_contentsStream->seek(0)) {
		warning("OneDriveUploadRequest: cannot restart because stream couldn't seek(0)");
		finishError(Networking::ErrorResponse(this, false, true, "OneDriveUploadRequest::start: can't restart, because seek(0) didn't work", -1));
		return;
	}
	_ignoreCallback = false;

	uploadNextPart();
}

void OneDriveUploadRequest::uploadNextPart() {
	const uint32 UPLOAD_PER_ONE_REQUEST = 10 * 1024 * 1024;

	if (_uploadUrl == "" && (uint32)_contentsStream->size() > UPLOAD_PER_ONE_REQUEST) {
		Common::String url = Common::String::format(ONEDRIVE_API_SPECIAL_APPROOT_UPLOAD, ConnMan.urlEncode(_savePath).c_str()); //folder must exist
		Networking::JsonCallback callback = new Common::Callback<OneDriveUploadRequest, Networking::JsonResponse>(this, &OneDriveUploadRequest::partUploadedCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<OneDriveUploadRequest, Networking::ErrorResponse>(this, &OneDriveUploadRequest::partUploadedErrorCallback);
		Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
		request->addHeader("Authorization: Bearer " + _storage->accessToken());
		request->setBuffer(new byte[1], 0); //use POST
		_workingRequest = ConnMan.addRequest(request);
		return;
	}

	Common::String url;
	if (_uploadUrl == "") {
		url = Common::String::format(ONEDRIVE_API_SPECIAL_APPROOT_CONTENT, ConnMan.urlEncode(_savePath).c_str());
	} else {
		url = _uploadUrl;
	}

	Networking::JsonCallback callback = new Common::Callback<OneDriveUploadRequest, Networking::JsonResponse>(this, &OneDriveUploadRequest::partUploadedCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<OneDriveUploadRequest, Networking::ErrorResponse>(this, &OneDriveUploadRequest::partUploadedErrorCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	request->usePut();

	uint32 oldPos = _contentsStream->pos();

	byte *buffer = new byte[UPLOAD_PER_ONE_REQUEST];
	uint32 size = _contentsStream->read(buffer, UPLOAD_PER_ONE_REQUEST);
	request->setBuffer(buffer, size);

	if (_uploadUrl != "") {
		request->addHeader(Common::String::format("Content-Range: bytes %u-%u/%u", oldPos, _contentsStream->pos() - 1, _contentsStream->size()));
	} else if (_contentsStream->size() == 0) {
		warning("\"Sorry, OneDrive can't upload empty files\"");
		finishUpload(StorageFile(_savePath, 0, 0, false));
		delete request;
		return;
	}

	_workingRequest = ConnMan.addRequest(request);
}

void OneDriveUploadRequest::partUploadedCallback(Networking::JsonResponse response) {
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

	if (json->isObject()) {
		Common::JSONObject object = json->asObject();

		if (object.contains("error")) {
			warning("OneDriveUploadRequest: error: %s", json->stringify(true).c_str());
			error.response = json->stringify(true);
			finishError(error);
			delete json;
			return;
		}

		if (Networking::CurlJsonRequest::jsonContainsString(object, "id", "OneDriveUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsString(object, "name", "OneDriveUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsIntegerNumber(object, "size", "OneDriveUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsString(object, "lastModifiedDateTime", "OneDriveUploadRequest")) {
			//finished
			Common::String path = _savePath;
			uint32 size = object.getVal("size")->asIntegerNumber();
			uint32 timestamp = ISO8601::convertToTimestamp(object.getVal("lastModifiedDateTime")->asString());
			finishUpload(StorageFile(path, size, timestamp, false));
			return;
		}

		if (_uploadUrl == "") {
			if (Networking::CurlJsonRequest::jsonContainsString(object, "uploadUrl", "OneDriveUploadRequest"))
				_uploadUrl = object.getVal("uploadUrl")->asString();
		}
	}

	if (_contentsStream->eos() || _contentsStream->pos() >= _contentsStream->size() - 1) {
		warning("OneDriveUploadRequest: no file info to return");
		finishUpload(StorageFile(_savePath, 0, 0, false));
	} else {
		uploadNextPart();
	}

	delete json;
}

void OneDriveUploadRequest::partUploadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void OneDriveUploadRequest::handle() {}

void OneDriveUploadRequest::restart() { start(); }

void OneDriveUploadRequest::finishUpload(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback)
		(*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace OneDrive
} // End of namespace Cloud
