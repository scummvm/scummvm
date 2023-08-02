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

#ifndef DLC_DLCMANAGER_H
#define DLC_DLCMANAGER_H

#include "common/str.h"
#include "common/str-array.h"
#include "common/queue.h"
#include "common/singleton.h"
#include "backends/dlc/store.h"
#include "backends/dlc/dlcdesc.h"
#include "backends/networking/curl/request.h"
#include "gui/object.h"
#include "gui/launcher.h"

namespace DLC {

class DLCManager : public Common::Singleton<DLCManager>, public GUI::CommandSender {

	Store *_store;
	GUI::LauncherDialog *_launcher;

	bool _isDLCDownloading = false;
	Common::String _currentDownloadingDLC;

public:
	bool _fetchDLCs = false;
	bool _interruptCurrentDownload = false;
	uint32 _currentDownloadedSize;
	Common::Array<DLCDesc*> _dlcs;
	Common::Array<DLCDesc*> _dlcsInProgress;
	Common::Queue<DLCDesc*> _queuedDownloadTasks;
	
	DLCManager();
	virtual ~DLCManager() {}

	void init();

	// Runs only once
	void getAllDLCs();

	void refreshDLCList();

	void refreshLauncherGameList();

	void setLauncher(GUI::LauncherDialog *launcher);

	// Add download task to queue, runs on click download button, 
	void addDownload(uint32 idx);

	bool cancelDownload(uint32 idx);

	void processDownloadQueue();

	// Returns the % download progress of current downloading game
	uint32 downloadProgress();

	Common::String getCurrentDownloadingDLC();

	int getDLCIdxFromId(const Common::String &id);

	void startDownloadAsync(const Common::String &id, const Common::String &url);

	void errorCallback(Networking::ErrorResponse error);

	void downloadFileCallback(Networking::DataResponse r);
};

#define DLCMan        DLC::DLCManager::instance()

} // End of namespace DLC


#endif
