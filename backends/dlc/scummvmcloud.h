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

#ifndef BACKENDS_DLC_SCUMMVMCLOUD_H
#define BACKENDS_DLC_SCUMMVMCLOUD_H

#include "common/error.h"
#include "common/queue.h"

#include "backends/dlc/store.h"
#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Networking {
class SessionRequest;
}

namespace DLC {
namespace ScummVMCloud {

class ScummVMCloud : public DLC::Store {

	Networking::SessionRequest *_rq;

public:	
	ScummVMCloud() {}
	virtual ~ScummVMCloud() {}

	virtual void getAllDLCs() override;

	virtual void startDownloadAsync(const Common::String &id, const Common::String &url) override;

	virtual void removeCacheFile(const Common::Path &file) override;

	// extracts the provided zip in the provided destination path
	Common::Error extractZip(const Common::Path &file, const Common::Path &destPath);

	void addEntryToConfig(Common::Path gamePath);

	// callback functions
	void jsonCallbackGetAllDLCs(const Networking::JsonResponse &response);

	void errorCallbackGetAllDLCs(const Networking::ErrorResponse &error);

	void downloadFileCallback(const Networking::DataResponse &response);

	void errorCallback(const Networking::ErrorResponse &error);
};

} // End of namespace ScummVMCloud
} // End of namespace DLC

#endif
