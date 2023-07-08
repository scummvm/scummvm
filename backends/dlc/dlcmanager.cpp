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
#include "common/system.h"

namespace Common {

DECLARE_SINGLETON(DLC::DLCManager);

}

namespace DLC {

DLCManager::DLCManager() {
	_store = g_system->getDLCStore();
}

void DLCManager::init() {
	getAllDLCs(_dlcs);
}

void DLCManager::getAllDLCs(Common::Array<DLC*>& dlcs) {
	// if distribution store's API available call it else hardcode
}

void DLCManager::addDownload(uint32 idx) {
	_dlcs[idx]->state = kInProgress;
	_queuedDownloadTasks.push(_dlcs[idx]);
	if (!_isDLCDownloading) {
		DLCManager::processDownload();
	}
}

void DLCManager::processDownload() {
	if (!_queuedDownloadTasks.empty()) {
		if (_dlcs[_queuedDownloadTasks.front()->idx]->state == kInProgress) {
			_isDLCDownloading = true;
			_currentDownloadingDLC = _queuedDownloadTasks.front()->id;
			DLCManager::startDownloadAsync(_queuedDownloadTasks.front()->id, &DLCManager::cb);
		} else {
			// state is already cancelled/downloaded -> skip download
			_queuedDownloadTasks.pop();
			// process next download in the queue
			processDownload();
		}
	} else {
		_isDLCDownloading = false;
	}
}

void DLCManager::cb() {
	// if finished successfully
	_dlcs[_queuedDownloadTasks.front()->idx]->state = kDownloaded;
	_queuedDownloadTasks.pop();
	// process next download in the queue
	DLCManager::processDownload();
}

bool DLCManager::cancelDownload(uint32 idx) {
	if (_currentDownloadingDLC == _dlcs[idx]->id) {
		// if already downloading, interrupt startDownloadAsync
	} else {
		// if not started, skip it in processDownload()
		_dlcs[idx]->state = kCancelled;
	}
	return true;
}

Common::String DLCManager::getCurrentDownloadingDLC() {
	return _currentDownloadingDLC;
}

} // End of namespace DLC
