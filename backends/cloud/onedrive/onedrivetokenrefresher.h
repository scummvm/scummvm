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

#ifndef BACKENDS_CLOUD_ONEDRIVE_ONEDRIVETOKENREFRESHER_H
#define BACKENDS_CLOUD_ONEDRIVE_ONEDRIVETOKENREFRESHER_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace OneDrive {

class OneDriveStorage;

class OneDriveTokenRefresher: public Networking::CurlJsonRequest {
	OneDriveStorage *_parentStorage;
	Common::Array<Common::String> _headers;

	void tokenRefreshed(const Storage::BoolResponse &response);

	void finishJson(const Common::JSONValue *json) override;
	void finishError(const Networking::ErrorResponse &error, Networking::RequestState state = Networking::FINISHED) override;
	void finishErrorIrrecoverable(const Networking::ErrorResponse &error, Networking::RequestState state = Networking::FINISHED);

public:
	OneDriveTokenRefresher(OneDriveStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url);
	~OneDriveTokenRefresher() override;

	void setHeaders(const Common::Array<Common::String> &headers) override;
	void addHeader(const Common::String &header) override;
};

} // End of namespace OneDrive
} // End of namespace Cloud

#endif
