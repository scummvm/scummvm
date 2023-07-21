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

#ifndef BACKENDS_DLC_ScummVMCloud_ScummVMCloud_H
#define BACKENDS_DLC_ScummVMCloud_ScummVMCloud_H

#include "backends/dlc/store.h"
#include "backends/dlc/dlcdesc.h"
#include "backends/networking/curl/session.h"
#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "common/queue.h"

namespace DLC {
namespace ScummVMCloud {

class ScummVMCloud: public DLC::Store {

Networking::Session session;

public:	
	ScummVMCloud() {}
	virtual ~ScummVMCloud() {}

	virtual void init() override {}

	virtual void requestInfo() override {}

	virtual void getDownloadState() override {}

	virtual void requestDownload() override {}

	virtual void getBytesDownloaded() override {}

	virtual void cancelDownload() override {}

	virtual void getAllDLCs(Common::Array<DLCDesc*> &dlcs) override;

	virtual void startDownloadAsync(const Common::String &id, const Common::String &url) override;

	virtual void removeCacheFile(Common::Path file) override;

	void extractZip(Common::Path file);

	// callback functions
	void jsonCallbackGetAllDLCs(Networking::JsonResponse response);

	void errorCallbackGetAllDLCs(Networking::ErrorResponse error);

	void downloadFileCallback(Networking::DataResponse response);

	void errorCallback(Networking::ErrorResponse error);
};

} // End of namespace ScummVMCloud
} // End of namespace DLC

#endif
