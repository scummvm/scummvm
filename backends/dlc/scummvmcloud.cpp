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

#include "backends/dlc/scummvmcloud.h"
#include "backends/dlc/dlcmanager.h"
#include "common/config-manager.h"

namespace DLC {
namespace ScummVMCloud {

void ScummVMCloud::getAllDLCs(Common::Array<DLCDesc*> &dlcs) {
    dlcs.push_back(new DLCDesc{"Beneath a Steel Sky - Freeware CD Version", "bass_cd", 100, 0, DLCDesc::kAvailable});
    dlcs.push_back(new DLCDesc{"Beneath a Steel Sky - Freeware Floppy Version", "bass_floppy", 230, 1, DLCDesc::kAvailable});
}

void ScummVMCloud::downloadFileCallback(Networking::DataResponse r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);
	if (response->eos) {
		warning("downloaded");
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

void ScummVMCloud::startDownloadAsync(Common::String &id) {
	Common::String url = Common::String::format("https://downloads.scummvm.org/frs/extras/Beneath%%20a%%20Steel%%20Sky/bass-cd-1.2.zip");
	Common::String localFile = "";

	Networking::SessionRequest *rq = session.get(url, localFile,
		new Common::Callback<ScummVMCloud, Networking::DataResponse>(this, &ScummVMCloud::downloadFileCallback),
		new Common::Callback<ScummVMCloud, Networking::ErrorResponse>(this, &ScummVMCloud::errorCallback));

	rq->start();
}

} // End of namespace ScummVMCloud
} // End of namespace DLC
