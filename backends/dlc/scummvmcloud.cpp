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
#include "common/punycode.h"
#include "common/config-manager.h"
#include "common/formats/json.h"

#include "gui/gui-manager.h"

#include "backends/dlc/scummvmcloud.h"
#include "backends/dlc/dlcmanager.h"

#include "engines/metaengine.h"

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
			Common::String platform = item.getVal("platform")->asString();
			Common::String gameid = item.getVal("gameid")->asString();
			Common::String description = item.getVal("description")->asString();
			Common::String language = item.getVal("language")->asString();
			Common::String extra = item.getVal("extra")->asString();
			Common::String engineid = item.getVal("engineid")->asString();
			Common::String guioptions = item.getVal("guioptions")->asString();
			uint32 size;
			if (item.getVal("size")->isString()) {
				size = item.getVal("size")->asString().asUint64();
			} else {
				size = item.getVal("size")->asIntegerNumber();
			}
			DLCMan._dlcs.push_back(new DLCDesc{name, id, url, platform, gameid, description, language, extra, engineid, guioptions, size, i, DLCDesc::kAvailable});
		}
	}
	// send refresh DLC list command to GUI
	DLCMan.refreshDLCList();
}

void ScummVMCloud::errorCallbackGetAllDLCs(Networking::ErrorResponse error) {
	warning("JsonRequest Error - getAllDLCs");
}

void ScummVMCloud::getAllDLCs() {
	Common::String url("https://scummvm-dlcs-default-rtdb.firebaseio.com/dlcs.json"); // temporary mock api
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(
		new Common::Callback<ScummVMCloud, Networking::JsonResponse>(this, &ScummVMCloud::jsonCallbackGetAllDLCs), 
		new Common::Callback<ScummVMCloud, Networking::ErrorResponse>(this, &ScummVMCloud::errorCallbackGetAllDLCs), 
		url);

	request->execute();
}

void ScummVMCloud::downloadFileCallback(Networking::DataResponse r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);
	DLCMan._currentDownloadedSize += response->len;
	if (DLCMan._interruptCurrentDownload) {
		_rq->close();
		DLCMan._interruptCurrentDownload = false;
		// delete the download cache (the incomplete .zip)
		Common::Path relativeFilePath = Common::Path(DLCMan._queuedDownloadTasks.front()->id);
		removeCacheFile(relativeFilePath);
		// handle next download
		DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kDownloaded;
		DLCMan._queuedDownloadTasks.pop();
		DLCMan._dlcsInProgress.remove_at(0);
		DLCMan.processDownloadQueue();
	}
	if (response->eos) {
		warning("downloaded");
		_rq->close(); // delete request
		Common::Path relativeFilePath = Common::Path(DLCMan._queuedDownloadTasks.front()->id);
		// extract the downloaded zip
		Common::String gameDir = Common::punycode_encodefilename(DLCMan._queuedDownloadTasks.front()->name);
		Common::Path destPath = Common::Path(ConfMan.get("dlcspath")).appendComponent(gameDir);
		extractZip(relativeFilePath, destPath);
		// remove cache (the downloaded .zip)
		removeCacheFile(relativeFilePath);
		// add downloaded game entry in scummvm configuration file
		addEntryToConfig(destPath);
		// handle next download
		DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kDownloaded;
		DLCMan.refreshDLCList();
		DLCMan._queuedDownloadTasks.pop();
		DLCMan._dlcsInProgress.remove_at(0);
		DLCMan.processDownloadQueue();
	}
}

void ScummVMCloud::errorCallback(Networking::ErrorResponse error) {
	// error downloading - start next download in queue
	DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kErrorDownloading;
	DLCMan._queuedDownloadTasks.pop();
	DLCMan._dlcsInProgress.remove_at(0);
	DLCMan.processDownloadQueue();
}

void ScummVMCloud::startDownloadAsync(const Common::String &id, const Common::String &url) {
	Common::String localFile = normalizePath(ConfMan.get("dlcspath") + "/" + id, '/');

	_rq = new Networking::SessionRequest(url, localFile,
		new Common::Callback<ScummVMCloud, Networking::DataResponse>(this, &ScummVMCloud::downloadFileCallback),
		new Common::Callback<ScummVMCloud, Networking::ErrorResponse>(this, &ScummVMCloud::errorCallback));

	_rq->start();
}

void ScummVMCloud::extractZip(const Common::Path &file, const Common::Path &destPath) {
	Common::Archive *dataArchive = nullptr;
	Common::Path dlcPath = Common::Path(ConfMan.get("dlcspath"));
	Common::FSNode *fs = new Common::FSNode(dlcPath.join(file));
	if (fs->exists()) {
		dataArchive = Common::makeZipArchive(*fs);
		// dataArchive is nullptr if zip file is incomplete
		if (dataArchive != nullptr) {
			dataArchive->dumpArchive(destPath.toString());
			DLCMan._errorText = "";
		} else {
			DLCMan._errorText = "ERROR: " + DLCMan._queuedDownloadTasks.front()->name + ": Archive is broken, please re-download!";
		}
	}
	delete fs;
	delete dataArchive;
}

void ScummVMCloud::removeCacheFile(const Common::Path &file) {
	Common::Path dlcPath = Common::Path(ConfMan.get("dlcspath"));
	Common::Path fileToDelete = dlcPath.join(file);
#if defined(POSIX)
	unlink(fileToDelete.toString().c_str());
#elif defined(WIN32)
	_unlink(fileToDelete.toString().c_str());
#else
	warning("ScummVMCloud::removeCacheFile(): Removing is unimplemented");
#endif
}

void ScummVMCloud::addEntryToConfig(Common::Path gamePath) {
	Common::FSNode dir(gamePath);
	Common::FSList fsnodes;
	if (!dir.getChildren(fsnodes, Common::FSNode::kListAll)) {
		warning("ScummVMCloud::addEntryToConfig(): Game directory does not exists");
		return;
	}
	if (fsnodes.size() == 1 && fsnodes[0].isDirectory()) {
		// if extraction process created a new folder inside gamePath, set gamePath to that directory
		gamePath = gamePath.appendComponent(fsnodes[0].getFileName());
	}
	// add a new entry in scummvm config file
	Common::String domain = EngineMan.generateUniqueDomain(DLCMan._queuedDownloadTasks.front()->gameid);
	ConfMan.addGameDomain(domain);
	ConfMan.set("engineid", DLCMan._queuedDownloadTasks.front()->engineid, domain);
	ConfMan.set("gameid", DLCMan._queuedDownloadTasks.front()->gameid, domain);
	ConfMan.set("description", DLCMan._queuedDownloadTasks.front()->description, domain);
	ConfMan.set("language", DLCMan._queuedDownloadTasks.front()->language, domain);
	ConfMan.set("platform", DLCMan._queuedDownloadTasks.front()->platform, domain);
	ConfMan.set("path", gamePath.toString(), domain);
	ConfMan.set("extra", DLCMan._queuedDownloadTasks.front()->extra, domain);
	ConfMan.set("guioptions", DLCMan._queuedDownloadTasks.front()->guioptions, domain);
	ConfMan.set("download", DLCMan._queuedDownloadTasks.front()->id, domain);

	// send refresh launcher command to GUI
	DLCMan.refreshLauncherGameList();
}

} // End of namespace ScummVMCloud
} // End of namespace DLC
