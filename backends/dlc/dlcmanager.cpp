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

#include "common/system.h"

#include "backends/dlc/dlcmanager.h"
#include "backends/dlc/dlcdesc.h"
#include "backends/dlc/scummvmcloud.h"

#include "gui/dlcsdialog.h"
#include "gui/launcher.h"

namespace Common {

DECLARE_SINGLETON(DLC::DLCManager);

}

namespace DLC {

DLCManager::DLCManager() : CommandSender(nullptr) {
	_store = g_system->getDLCStore();
}

DLCManager::~DLCManager() {
	for (uint32 i = 0; i < _dlcs.size(); ++i) {
		delete _dlcs[i];
	}
}

void DLCManager::init() {}

void DLCManager::getAllDLCs() {
	_store->getAllDLCs();
}

void DLCManager::refreshDLCList() {
	sendCommand(GUI::kRefreshDLCList, 0);
}

void DLCManager::refreshLauncherGameList() {
	if (_launcher) {
		_launcher->rebuild();
	}
}

void DLCManager::setLauncher(GUI::LauncherDialog *launcher) {
	_launcher = launcher;
}

void DLCManager::addDownload(uint32 idx) {
	if (_dlcs[idx]->state == DLCDesc::kInProgress) {
		// if DLC is already in queue, don't add again
		return;
	}
	_dlcs[idx]->state = DLCDesc::kInProgress;
	_queuedDownloadTasks.push(_dlcs[idx]);
	_dlcsInProgress.push_back(_dlcs[idx]);
	if (!_isDLCDownloading) {
		// if queue is not already processing
		DLCManager::processDownloadQueue();
	}
}

void DLCManager::processDownloadQueue() {
	_currentDownloadedSize = 0;
	_isDLCDownloading = true;
	if (!_queuedDownloadTasks.empty()) {
		if (_queuedDownloadTasks.front()->state == DLCDesc::kInProgress) {
			_currentDownloadingDLC = _queuedDownloadTasks.front()->id;
			DLCManager::startDownloadAsync(_queuedDownloadTasks.front()->id, _queuedDownloadTasks.front()->url);
		} else {
			// state is already cancelled/downloaded -> skip download
			_queuedDownloadTasks.pop();
			_dlcsInProgress.remove_at(0);
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
	if (_queuedDownloadTasks.front()->idx == idx) {
		// if already downloading, interrupt startDownloadAsync
		_interruptCurrentDownload = true;
	} else {
		// if not started, skip it in processDownload()
		_dlcs[idx]->state = DLCDesc::kCancelled;
		DLCMan.refreshDLCList();
	}
	return true;
}

Common::String DLCManager::getCurrentDownloadingDLC() const {
	return _currentDownloadingDLC;
}

uint DLCManager::getDLCIdxFromId(const Common::String &id) const {
	for (uint32 i = 0; i < _dlcs.size(); ++i) {
		if (_dlcs[i]->id == id) return i;
	}
	return -1;
}

} // End of namespace DLC
