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


#include "backends/dlc/android/playstore.h"
#include "backends/dlc/dlcmanager.h"
#include "backends/dlc/scummvmcloud.h"
#include "backends/dlc/android/playstore.h"
#include "common/system.h"

namespace Common {

DECLARE_SINGLETON(DLC::DLCManager);

}

namespace DLC {

DLCManager::DLCManager() {
	// _store = g_system->getDLCStore();
	// TODO: Handle creation through getDLCStore()
	_store = new DLC::ScummVMCloud::ScummVMCloud();
}

void DLCManager::init() {
	DLCManager::getAllDLCs(_dlcs);
}

void DLCManager::getAllDLCs(Common::Array<DLCDesc*> &dlcs) {
	_store->getAllDLCs(dlcs);
}

void DLCManager::addDownload(uint32 idx) {
	_dlcs[idx]->state = DLCDesc::kInProgress;
	_queuedDownloadTasks.push(_dlcs[idx]);
	if (!_isDLCDownloading) {
		// if queue is not already processing
		DLCManager::processDownloadQueue();
	}
}

void DLCManager::processDownloadQueue() {
	_isDLCDownloading = true;
	if (!_queuedDownloadTasks.empty()) {
		if (_queuedDownloadTasks.front()->state == DLCDesc::kInProgress) {
			_currentDownloadingDLC = _queuedDownloadTasks.front()->id;
			DLCManager::startDownloadAsync(_queuedDownloadTasks.front()->id, _queuedDownloadTasks.front()->url);
		} else {
			// state is already cancelled/downloaded -> skip download
			_queuedDownloadTasks.pop();
			// process next download in the queue
			processDownloadQueue();
		}
	} else {
		// no more download in queue
		_isDLCDownloading = false;
		_currentDownloadingDLC = "";
	}
}

void DLCManager::startDownloadAsync(const Common::String &id, const Common::String &url) {
	_store->startDownloadAsync(id, url);
}

bool DLCManager::cancelDownload(uint32 idx) {
	if (_currentDownloadingDLC == _dlcs[idx]->id) {
		// if already downloading, interrupt startDownloadAsync
	} else {
		// if not started, skip it in processDownload()
		_dlcs[idx]->state = DLCDesc::kCancelled;
	}
	return true;
}

Common::String DLCManager::getCurrentDownloadingDLC() {
	return _currentDownloadingDLC;
}

int DLCManager::getDLCIdxFromId(const Common::String &id) {
	for (int i = 0; i < _dlcs.size(); ++i) {
		if (_dlcs[i]->id == id) return i;
	}
	return -1;
}

} // End of namespace DLC
