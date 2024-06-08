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

#include "backends/networking/curl/sessionrequest.h"
#include "backends/dlc/scummvmcloud.h"
#include "backends/dlc/dlcmanager.h"
#include "backends/dlc/dlcdesc.h"

#include "engines/metaengine.h"

namespace DLC {
namespace ScummVMCloud {

void ScummVMCloud::jsonCallbackGetAllDLCs(const Networking::JsonResponse &response) {
	const Common::JSONValue *json = response.value;
	if (json == nullptr || !json->isObject()) {
		return;
	}
	debug(1, "DLC list JSON response: %s", json->stringify(true).c_str());
	Common::JSONObject result = json->asObject();
	if (result.contains("entries")) {
		Common::JSONArray items = result.getVal("entries")->asArray();
		for (uint32 i = 0; i < items.size(); ++i) {
			if (!Networking::CurlJsonRequest::jsonIsObject(items[i], "ScummVMCloud")) continue;
			Common::JSONObject item = items[i]->asObject();

			DLC::DLCDesc *dlc = new DLC::DLCDesc();
			dlc->id = item.getVal("id")->asString();
			dlc->name = item.getVal("name")->asString();
			dlc->url = item.getVal("url")->asString();
			dlc->platform = item.getVal("platform")->asString();
			dlc->gameid = item.getVal("gameid")->asString();
			dlc->description = item.getVal("description")->asString();
			dlc->language = item.getVal("language")->asString();
			dlc->extra = item.getVal("extra")->asString();
			dlc->engineid = item.getVal("engineid")->asString();
			dlc->guioptions = item.getVal("guioptions")->asString();
			if (item.getVal("size")->isString()) {
				dlc->size = item.getVal("size")->asString().asUint64();
			} else {
				dlc->size = item.getVal("size")->asIntegerNumber();
			}
			dlc->idx = i;
			DLCMan._dlcs.push_back(dlc);
		}
	}
	// send refresh DLC list command to GUI
	DLCMan.refreshDLCList();
}

void ScummVMCloud::errorCallbackGetAllDLCs(const Networking::ErrorResponse &error) {
	warning("JsonRequest Error - getAllDLCs");
}

void ScummVMCloud::getAllDLCs() {
	Common::String url("https://scummvm-dlcs-default-rtdb.firebaseio.com/dlcs.json"); // temporary mock api
	Networking::JsonCallback callback = new Common::Callback<ScummVMCloud, const Networking::JsonResponse &>(this, &ScummVMCloud::jsonCallbackGetAllDLCs);
	Networking::ErrorCallback failureCallback = new Common::Callback<ScummVMCloud, const Networking::ErrorResponse &>(this, &ScummVMCloud::errorCallbackGetAllDLCs);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(
		callback, failureCallback, url);

	request->execute();
}

void ScummVMCloud::downloadFileCallback(const Networking::DataResponse &r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);
	DLCMan._currentDownloadedSize += response->len;
	if (DLCMan._interruptCurrentDownload) {
		_rq->close();
		DLCMan._interruptCurrentDownload = false;
		// delete the download cache (the incomplete .zip)
		Common::Path relativeFilePath = Common::Path(DLCMan._queuedDownloadTasks.front()->id);
		removeCacheFile(relativeFilePath);

		DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kCancelled;
		DLCMan.refreshDLCList();

		// handle next download
		DLCMan._queuedDownloadTasks.pop();
		DLCMan._dlcsInProgress.remove_at(0);
		DLCMan.processDownloadQueue();
	}
	if (response->eos) {
		DLC::DLCDesc *dlc = DLCMan._queuedDownloadTasks.front();
		debug(1, "Downloaded: %s", dlc->name.c_str());

		_rq->close(); // delete request

		Common::Path relativeFilePath = Common::Path(dlc->id);

		// extract the downloaded zip
		Common::String gameDir = Common::punycode_encodefilename(dlc->name);
		Common::Path destPath(ConfMan.getPath("dlcspath").appendComponent(gameDir));
		Common::Error error = extractZip(relativeFilePath, destPath);

		// remove cache (the downloaded .zip)
		removeCacheFile(relativeFilePath);

		if (error.getCode() == Common::kNoError) {
			// add downloaded game entry in scummvm configuration file
			addEntryToConfig(destPath);
			dlc->state = DLCDesc::kDownloaded;
			DLCMan._errorText = "";
		} else {
			// if there is any error in extraction
			dlc->state = DLCDesc::kErrorDownloading;
			DLCMan._errorText = error.getDesc();
		}

		DLCMan.refreshDLCList();

		// handle next download
		DLCMan._queuedDownloadTasks.pop();
		DLCMan._dlcsInProgress.remove_at(0);
		DLCMan.processDownloadQueue();
	}
}

void ScummVMCloud::errorCallback(const Networking::ErrorResponse &error) {
	// error downloading - start next download in queue
	DLCMan._queuedDownloadTasks.front()->state = DLCDesc::kErrorDownloading;
	DLCMan._queuedDownloadTasks.pop();
	DLCMan._dlcsInProgress.remove_at(0);
	DLCMan.processDownloadQueue();
}

void ScummVMCloud::startDownloadAsync(const Common::String &id, const Common::String &url) {
	Common::Path localFile(ConfMan.getPath("dlcspath").appendComponent(id));

	Networking::DataCallback callback = new Common::Callback<ScummVMCloud, const Networking::DataResponse &>(this, &ScummVMCloud::downloadFileCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<ScummVMCloud, const Networking::ErrorResponse &>(this, &ScummVMCloud::errorCallback);
	_rq = new Networking::SessionRequest(url, localFile, callback, failureCallback);

	_rq->start();
}

Common::Error ScummVMCloud::extractZip(const Common::Path &file, const Common::Path &destPath) {
	Common::Archive *dataArchive = nullptr;
	Common::Path dlcPath(ConfMan.getPath("dlcspath"));
	Common::FSNode *fs = new Common::FSNode(dlcPath.join(file));
	Common::Error error = Common::kNoError;
	if (fs->exists()) {
		dataArchive = Common::makeZipArchive(*fs);
		// dataArchive is nullptr if zip file is incomplete
		if (dataArchive != nullptr) {
			error = dataArchive->dumpArchive(destPath);
		} else {
			error = Common::Error(Common::kCreatingFileFailed, DLCMan._queuedDownloadTasks.front()->name + "Archive is broken, please re-download");
		}
	}
	delete fs;
	delete dataArchive;

	return error;
}

void ScummVMCloud::removeCacheFile(const Common::Path &file) {
	Common::Path dlcPath(ConfMan.getPath("dlcspath"));
	Common::Path fileToDelete = dlcPath.join(file);
#if defined(POSIX)
	unlink(fileToDelete.toString(Common::Path::kNativeSeparator).c_str());
#elif defined(WIN32)
	_unlink(fileToDelete.toString(Common::Path::kNativeSeparator).c_str());
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
	DLC::DLCDesc *dlc = DLCMan._queuedDownloadTasks.front();
	Common::String domain = EngineMan.generateUniqueDomain(dlc->gameid);
	ConfMan.addGameDomain(domain);
	ConfMan.set("engineid", dlc->engineid, domain);
	ConfMan.set("gameid", dlc->gameid, domain);
	ConfMan.set("description", dlc->description, domain);
	ConfMan.set("language", dlc->language, domain);
	ConfMan.set("platform", dlc->platform, domain);
	ConfMan.set("path", gamePath.toString(), domain);
	ConfMan.set("extra", dlc->extra, domain);
	ConfMan.set("guioptions", dlc->guioptions, domain);
	ConfMan.set("download", dlc->id, domain);

	// send refresh launcher command to GUI
	DLCMan.refreshLauncherGameList();
}

} // End of namespace ScummVMCloud
} // End of namespace DLC
