/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(POSIX)
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink

#include <unistd.h>
#endif

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/file.h"
#include "backends/dlc/scummvmcloud.h"
#include "backends/dlc/dlcmanager.h"
#include "common/config-manager.h"
#include "common/formats/json.h"

namespace DLC {
namespace ScummVMCloud {

void ScummVMCloud::jsonCallbackGetAllDLCs(Networking::JsonResponse response) {
	Common::JSONValue *json = (Common::JSONValue *)response.value;
	if (json == nullptr || !json->isObject()) {
		return;
	}
	// warning("%s", json->stringify(true).c_str());
	Common::JSONObject result = json->asObject();
	if (result.contains("entries")) {
		Common::JSONArray items = result.getVal("entries")->asArray();
		for (uint32 i = 0; i < items.size(); ++i) {
			if (!Networking::CurlJsonRequest::jsonIsObject(items[i], "ScummVMCloud")) continue;
			Common::JSONObject item = items[i]->asObject();
			Common::String id = item.getVal("id")->asString();
			Common::String name = item.getVal("name")->asString();
			Common::String url = item.getVal("url")->asString();
			uint32 size;
			if (item.getVal("size")->isString()) {
				size = item.getVal("size")->asString().asUint64();
			} else {
				size = item.getVal("size")->asIntegerNumber();
			}
			DLCMan._dlcs.push_back(new DLCDesc{name, id, url, size, i, DLCDesc::kAvailable});
		}
	}
	// send refresh DLC list command to GUI
	DLCMan.refreshDLCList();
}

void ScummVMCloud::errorCallbackGetAllDLCs(Networking::ErrorResponse error) {
	warning("JsonRequest Error - getAllDLCs");
}

void ScummVMCloud::getAllDLCs() {
	Common::String url("https://mocki.io/v1/0d86064d-1c04-41c8-a7b0-7e7e044b9b58"); // temporary mock api
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(
		new Common::Callback<ScummVMCloud, Networking::JsonResponse>(this, &ScummVMCloud::jsonCallbackGetAllDLCs), 
		new Common::Callback<ScummVMCloud, Networking::ErrorResponse>(this, &ScummVMCloud::errorCallbackGetAllDLCs), 
		url);

	request->execute();
}

void ScummVMCloud::downloadFileCallback(Networking::DataResponse r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);
	if (response->eos) {
		warning("downloaded");
		_rq->close(); // delete request
		Common::Path relativeFilePath = Common::Path(DLCMan._queuedDownloadTasks.front()->id);
		// extract the downloaded zip
		extractZip(relativeFilePath);
		// remove cache (the downloaded .zip)
		removeCacheFile(relativeFilePath);
		// handle next download
		DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kDownloaded;
		DLCMan._queuedDownloadTasks.pop();
		DLCMan.processDownloadQueue();
	}
}

void ScummVMCloud::errorCallback(Networking::ErrorResponse error) {
	// error downloading - start next download in queue
	DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kErrorDownloading;
	DLCMan._queuedDownloadTasks.pop();
	DLCMan.processDownloadQueue();
}

void ScummVMCloud::startDownloadAsync(const Common::String &id, const Common::String &url) {
	// TODO: Change it to dlcpath
	Common::String localFile = normalizePath(ConfMan.get("iconspath") + "/" + id, '/');

	_rq = new Networking::SessionRequest(url, localFile,
		new Common::Callback<ScummVMCloud, Networking::DataResponse>(this, &ScummVMCloud::downloadFileCallback),
		new Common::Callback<ScummVMCloud, Networking::ErrorResponse>(this, &ScummVMCloud::errorCallback));

	_rq->start();
}

void ScummVMCloud::extractZip(Common::Path file) {
	Common::Archive *dataArchive = nullptr;
	Common::Path dlcPath = Common::Path(ConfMan.get("iconspath"));
	Common::FSNode *fs = new Common::FSNode(dlcPath.join(file));
	if (fs->exists()) {
		dataArchive = Common::makeZipArchive(*fs);
		// dataArchive is nullptr if zip file is incomplete
		if (dataArchive != nullptr) {
			dataArchive->dumpArchive(dlcPath.toString());
		}
	}
	delete fs;
	delete dataArchive;
}

void ScummVMCloud::removeCacheFile(Common::Path file) {
	Common::Path dlcPath = Common::Path(ConfMan.get("iconspath"));
	Common::Path fileToDelete = dlcPath.join(file);
#if defined(POSIX)
	unlink(fileToDelete.toString().c_str());
#elif defined(WIN32)
	_unlink(fileToDelete.toString().c_str());
#else
	warning("ScummVMCloud::removeCacheFile(): Removing is unimplemented");
#endif
}

} // End of namespace ScummVMCloud
} // End of namespace DLC
