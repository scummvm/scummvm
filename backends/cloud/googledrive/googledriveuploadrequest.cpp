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

#include "backends/cloud/googledrive/googledriveuploadrequest.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"
#include "googledrivetokenrefresher.h"

namespace Cloud {
namespace GoogleDrive {

#define GOOGLEDRIVE_API_FILES "https://www.googleapis.com/upload/drive/v3/files"

GoogleDriveUploadRequest::GoogleDriveUploadRequest(GoogleDriveStorage *storage, Common::String path, Common::SeekableReadStream *contents, Storage::UploadCallback callback, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _storage(storage), _savePath(path), _contentsStream(contents), _uploadCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveUploadRequest::~GoogleDriveUploadRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _contentsStream;
	delete _uploadCallback;
}

void GoogleDriveUploadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	if (_contentsStream == nullptr || !_contentsStream->seek(0)) {
		warning("GoogleDriveUploadRequest: cannot restart because stream couldn't seek(0)");
		finishError(Networking::ErrorResponse(this, false, true, "GoogleDriveUploadRequest::start: couldn't restart because failed to seek(0)", -1));
		return;
	}
	_resolvedId = ""; //used to update file contents
	_parentId = ""; //used to create file within parent directory
	_serverReceivedBytes = 0;
	_ignoreCallback = false;

	resolveId();
}

void GoogleDriveUploadRequest::resolveId() {
	//check whether such file already exists
	Storage::UploadCallback innerCallback = new Common::Callback<GoogleDriveUploadRequest, Storage::UploadResponse>(this, &GoogleDriveUploadRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<GoogleDriveUploadRequest, Networking::ErrorResponse>(this, &GoogleDriveUploadRequest::idResolveFailedCallback);
	_workingRequest = _storage->resolveFileId(_savePath, innerCallback, innerErrorCallback);
}

void GoogleDriveUploadRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	_resolvedId = response.value.id();
	startUpload();
}

void GoogleDriveUploadRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

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
		startUpload();
		return;
	}

	finishError(error);
}

void GoogleDriveUploadRequest::startUpload() {
	Common::String name = _savePath;
	for (uint32 i = name.size(); i > 0; --i) {
		if (name[i - 1] == '/' || name[i - 1] == '\\') {
			name.erase(0, i);
			break;
		}
	}

	Common::String url = GOOGLEDRIVE_API_FILES;
	if (_resolvedId != "")
		url += "/" + ConnMan.urlEncode(_resolvedId);
	url += "?uploadType=resumable&fields=id,mimeType,modifiedTime,name,size";
	Networking::JsonCallback callback = new Common::Callback<GoogleDriveUploadRequest, Networking::JsonResponse>(this, &GoogleDriveUploadRequest::startUploadCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveUploadRequest, Networking::ErrorResponse>(this, &GoogleDriveUploadRequest::startUploadErrorCallback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	request->addHeader("Content-Type: application/json");
	if (_resolvedId != "")
		request->usePatch();

	Common::JSONObject jsonRequestParameters;
	if (_resolvedId != "") {
		jsonRequestParameters.setVal("id", new Common::JSONValue(_resolvedId));
	} else {
		Common::JSONArray parentsArray;
		parentsArray.push_back(new Common::JSONValue(_parentId));
		jsonRequestParameters.setVal("parents", new Common::JSONValue(parentsArray));
	}
	jsonRequestParameters.setVal("name", new Common::JSONValue(name));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	_workingRequest = ConnMan.addRequest(request);
}

void GoogleDriveUploadRequest::startUploadCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Networking::ErrorResponse error(this, false, true, "GoogleDriveUploadRequest::startUploadCallback", -1);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq) {
		const Networking::NetworkReadStream *stream = rq->getNetworkReadStream();
		if (stream) {
			long code = stream->httpResponseCode();
			if (code == 200) {
				Common::HashMap<Common::String, Common::String> headers = stream->responseHeadersMap();
				if (headers.contains("location")) {
					_uploadUrl = headers["location"];
					uploadNextPart();
					return;
				} else {
					error.response += ": response must provide Location header, but it's not there";
				}
			} else {
				error.response += ": response is not 200 OK";
			}

			error.httpResponseCode = code;
		} else {
			error.response += ": missing response stream [improbable]";
		}
	} else {
		error.response += ": missing request object [improbable]";
	}

	Common::JSONValue *json = response.value;
	delete json;

	finishError(error);
}

void GoogleDriveUploadRequest::startUploadErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void GoogleDriveUploadRequest::uploadNextPart() {
	const uint32 UPLOAD_PER_ONE_REQUEST = 10 * 1024 * 1024;
	Common::String url = _uploadUrl;

	Networking::JsonCallback callback = new Common::Callback<GoogleDriveUploadRequest, Networking::JsonResponse>(this, &GoogleDriveUploadRequest::partUploadedCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveUploadRequest, Networking::ErrorResponse>(this, &GoogleDriveUploadRequest::partUploadedErrorCallback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	request->usePut();

	uint32 oldPos = _contentsStream->pos();
	if (oldPos != _serverReceivedBytes) {
		if (!_contentsStream->seek(_serverReceivedBytes)) {
			warning("GoogleDriveUploadRequest: cannot upload because stream couldn't seek(%lu)", _serverReceivedBytes);
			finishError(Networking::ErrorResponse(this, false, true, "GoogleDriveUploadRequest::uploadNextPart: seek() didn't work", -1));
			return;
		}
		oldPos = _serverReceivedBytes;
	}

	byte *buffer = new byte[UPLOAD_PER_ONE_REQUEST];
	uint32 size = _contentsStream->read(buffer, UPLOAD_PER_ONE_REQUEST);
	if (size != 0)
		request->setBuffer(buffer, size);

	if (_uploadUrl != "") {
		if (_contentsStream->pos() == 0)
			request->addHeader(Common::String::format("Content-Length: 0"));
		else
			request->addHeader(Common::String::format("Content-Range: bytes %u-%u/%u", oldPos, _contentsStream->pos() - 1, _contentsStream->size()));
	}

	_workingRequest = ConnMan.addRequest(request);
}

bool GoogleDriveUploadRequest::handleHttp308(const Networking::NetworkReadStream *stream) {
	//308 Resume Incomplete, with Range: X-Y header
	if (!stream)
		return false;
	if (stream->httpResponseCode() != 308)
		return false; //seriously

	Common::HashMap<Common::String, Common::String> headers = stream->responseHeadersMap();
	if (headers.contains("range")) {
		Common::String range = headers["range"];
		for (int rangeTry = 0; rangeTry < 2; ++rangeTry) {
			const char *needle = (rangeTry == 0 ? "0-" : "bytes=0-"); //if it lost the first part, I refuse to talk with it
			uint32 needleLength = (rangeTry == 0 ? 2 : 8);

			if (range.hasPrefix(needle)) {
				range.erase(0, needleLength);
				_serverReceivedBytes = range.asUint64() + 1;
				uploadNextPart();
				return true;
			}
		}
	}

	return false;
}

void GoogleDriveUploadRequest::partUploadedCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Networking::ErrorResponse error(this, false, true, "", -1);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq) {
		const Networking::NetworkReadStream *stream = rq->getNetworkReadStream();
		if (stream) {
			long code = stream->httpResponseCode();
			error.httpResponseCode = code;
			if (code == 308 && handleHttp308(stream)) {
				delete (Common::JSONValue *)response.value;
				return;
			}
		}
	}

	Common::JSONValue *json = response.value;
	if (json == nullptr) {
		error.response = "Failed to parse JSON, null passed!";
		finishError(error);
		return;
	}

	if (json->isObject()) {
		Common::JSONObject object = json->asObject();

		if (object.contains("error")) {
			warning("GoogleDrive returned error: %s", json->stringify(true).c_str());
			error.response = json->stringify(true);
			finishError(error);
			delete json;
			return;
		}

		if (Networking::CurlJsonRequest::jsonContainsString(object, "id", "GoogleDriveUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsString(object, "name", "GoogleDriveUploadRequest") &&
			Networking::CurlJsonRequest::jsonContainsString(object, "mimeType", "GoogleDriveUploadRequest")) {
			//finished
			Common::String id = object.getVal("id")->asString();
			Common::String name = object.getVal("name")->asString();
			bool isDirectory = (object.getVal("mimeType")->asString() == "application/vnd.google-apps.folder");
			uint32 size = 0, timestamp = 0;
			if (Networking::CurlJsonRequest::jsonContainsString(object, "size", "GoogleDriveUploadRequest", true))
				size = object.getVal("size")->asString().asUint64();
			if (Networking::CurlJsonRequest::jsonContainsString(object, "modifiedTime", "GoogleDriveUploadRequest", true))
				timestamp = ISO8601::convertToTimestamp(object.getVal("modifiedTime")->asString());

			finishUpload(StorageFile(id, _savePath, name, size, timestamp, isDirectory));
			return;
		}
	}

	if (_contentsStream->eos() || _contentsStream->pos() >= _contentsStream->size() - 1) {
		warning("GoogleDriveUploadRequest: no file info to return");
		finishUpload(StorageFile(_savePath, 0, 0, false));
	} else {
		uploadNextPart();
	}

	delete json;
}

void GoogleDriveUploadRequest::partUploadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)error.request;
	if (rq) {
		const Networking::NetworkReadStream *stream = rq->getNetworkReadStream();
		if (stream) {
			long code = stream->httpResponseCode();
			if (code == 308 && handleHttp308(stream)) {
				return;
			}
		}
	}

	finishError(error);
}

void GoogleDriveUploadRequest::handle() {}

void GoogleDriveUploadRequest::restart() { start(); }

void GoogleDriveUploadRequest::finishUpload(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback)
		(*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
