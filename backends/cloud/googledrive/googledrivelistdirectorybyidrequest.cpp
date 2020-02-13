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

#include "backends/cloud/googledrive/googledrivelistdirectorybyidrequest.h"
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

#define GOOGLEDRIVE_API_FILES "https://www.googleapis.com/drive/v3/files?spaces=drive&fields=files%28id,mimeType,modifiedTime,name,size%29,nextPageToken&orderBy=folder,name"
//files(id,mimeType,modifiedTime,name,size),nextPageToken

GoogleDriveListDirectoryByIdRequest::GoogleDriveListDirectoryByIdRequest(GoogleDriveStorage *storage, Common::String id, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _requestedId(id), _storage(storage), _listDirectoryCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveListDirectoryByIdRequest::~GoogleDriveListDirectoryByIdRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _listDirectoryCallback;
}

void GoogleDriveListDirectoryByIdRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_files.clear();
	_ignoreCallback = false;

	makeRequest("");
}

void GoogleDriveListDirectoryByIdRequest::makeRequest(Common::String pageToken) {
	Common::String url = GOOGLEDRIVE_API_FILES;
	if (pageToken != "")
		url += "&pageToken=" + pageToken;
	url += "&q=%27" + _requestedId + "%27+in+parents";

	Networking::JsonCallback callback = new Common::Callback<GoogleDriveListDirectoryByIdRequest, Networking::JsonResponse>(this, &GoogleDriveListDirectoryByIdRequest::responseCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveListDirectoryByIdRequest, Networking::ErrorResponse>(this, &GoogleDriveListDirectoryByIdRequest::errorCallback);
	Networking::CurlJsonRequest *request = new GoogleDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	_workingRequest = ConnMan.addRequest(request);
}

void GoogleDriveListDirectoryByIdRequest::responseCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete response.value;
		return;
	}
	if (response.request)
		_date = response.request->date();

	Networking::ErrorResponse error(this, "GoogleDriveListDirectoryByIdRequest::responseCallback");
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq && rq->getNetworkReadStream())
		error.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	Common::JSONValue *json = response.value;
	if (json) {
		Common::JSONObject responseObject = json->asObject();

		///debug("%s", json->stringify(true).c_str());

		if (responseObject.contains("error") || responseObject.contains("error_summary")) {
			warning("GoogleDrive returned error: %s", responseObject.getVal("error_summary")->asString().c_str());
			error.failed = true;
			error.response = json->stringify();
			finishError(error);
			delete json;
			return;
		}

		//TODO: check that ALL keys exist AND HAVE RIGHT TYPE to avoid segfaults

		if (responseObject.contains("files") && responseObject.getVal("files")->isArray()) {
			Common::JSONArray items = responseObject.getVal("files")->asArray();
			for (uint32 i = 0; i < items.size(); ++i) {
				Common::JSONObject item = items[i]->asObject();
				Common::String id = item.getVal("id")->asString();
				Common::String name = item.getVal("name")->asString();
				bool isDirectory = (item.getVal("mimeType")->asString() == "application/vnd.google-apps.folder");
				uint32 size = 0, timestamp = 0;
				if (item.contains("size") && item.getVal("size")->isString())
					size = item.getVal("size")->asString().asUint64();
				if (item.contains("modifiedTime") && item.getVal("modifiedTime")->isString())
					timestamp = ISO8601::convertToTimestamp(item.getVal("modifiedTime")->asString());

				//as we list directory by id, we can't determine full path for the file, so we leave it empty
				_files.push_back(StorageFile(id, "", name, size, timestamp, isDirectory));
			}
		}

		bool hasMore = (responseObject.contains("nextPageToken"));

		if (hasMore) {
			Common::String token = responseObject.getVal("nextPageToken")->asString();
			makeRequest(token);
		} else {
			finishListing(_files);
		}
	} else {
		warning("null, not json");
		error.failed = true;
		finishError(error);
	}

	delete json;
}

void GoogleDriveListDirectoryByIdRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void GoogleDriveListDirectoryByIdRequest::handle() {}

void GoogleDriveListDirectoryByIdRequest::restart() { start(); }

Common::String GoogleDriveListDirectoryByIdRequest::date() const { return _date; }

void GoogleDriveListDirectoryByIdRequest::finishListing(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback)
		(*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
