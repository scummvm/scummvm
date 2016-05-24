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
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

DropboxListDirectoryRequest::DropboxListDirectoryRequest(Common::String token, Common::String path, Storage::FileArrayCallback cb, bool recursive):
	Networking::Request(0), _filesCallback(cb), _token(token), _complete(false) {	
	Common::BaseCallback<> *innerCallback = new Common::Callback<DropboxListDirectoryRequest>(this, &DropboxListDirectoryRequest::responseCallback);//new Common::GlobalFunctionCallback(printJson); //okay
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, "https://api.dropboxapi.com/2/files/list_folder");
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/json");

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("path", new Common::JSONValue(path));
	jsonRequestParameters.setVal("recursive", new Common::JSONValue(recursive));
	jsonRequestParameters.setVal("include_media_info", new Common::JSONValue(false));
	jsonRequestParameters.setVal("include_deleted", new Common::JSONValue(false));

	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	ConnMan.addRequest(request);
}

void DropboxListDirectoryRequest::responseCallback(void *jsonPtr) {
	Common::JSONValue *json = (Common::JSONValue *)jsonPtr;
	if (json) {
		Common::JSONObject response = json->asObject();
		
		if (response.contains("error") || response.contains("error_summary")) {
			warning("Dropbox returned error: %s", response.getVal("error_summary")->asString().c_str());
			_complete = true;
			delete json;
			return;
		}

		//TODO: check that all keys exist to avoid segfaults
		//TODO: get more files in the folder to check "has_more" case

		Common::JSONArray items = response.getVal("entries")->asArray();
		for (uint32 i = 0; i < items.size(); ++i) {
			Common::JSONObject item = items[i]->asObject();
			Common::String path = item.getVal("path_lower")->asString();			
			bool isDirectory = (item.getVal(".tag")->asString() == "folder");
			uint32 size = 0, timestamp = 0;
			if (!isDirectory) {
				size = item.getVal("size")->asNumber();
				timestamp = ISO8601::convertToTimestamp(item.getVal("server_modified")->asString());
			}
			_files.push_back(StorageFile(path, size, timestamp, isDirectory));
		}

		bool hasMore = response.getVal("has_more")->asBool();

		if (hasMore) {
			Common::BaseCallback<> *innerCallback = new Common::Callback<DropboxListDirectoryRequest>(this, &DropboxListDirectoryRequest::responseCallback);
			Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, "https://api.dropboxapi.com/2/files/list_folder/continue");
			request->addHeader("Authorization: Bearer " + _token);
			request->addHeader("Content-Type: application/json");

			Common::JSONObject jsonRequestParameters;
			jsonRequestParameters.setVal("cursor", new Common::JSONValue(response.getVal("cursor")->asString()));

			Common::JSONValue value(jsonRequestParameters);
			request->addPostField(Common::JSON::stringify(&value));

			ConnMan.addRequest(request);
		} else {
			_complete = true;
		}		
	} else {
		warning("null, not json");
		_complete = true;
	}

	delete json;
}

bool DropboxListDirectoryRequest::handle() {
	if (_complete && _filesCallback) {		
		(*_filesCallback)(_files);
	}

	return _complete;
}


} //end of namespace Dropbox
} //end of namespace Cloud
