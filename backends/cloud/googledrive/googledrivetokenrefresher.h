/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVETOKENREFRESHER_H
#define BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVETOKENREFRESHER_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace GoogleDrive {

class GoogleDriveStorage;

class GoogleDriveTokenRefresher: public Networking::CurlJsonRequest {
	GoogleDriveStorage *_parentStorage;
	Common::Array<Common::String> _headers;

	void tokenRefreshed(Storage::BoolResponse response);

	virtual void finishJson(Common::JSONValue *json);
public:
	GoogleDriveTokenRefresher(GoogleDriveStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url);
	virtual ~GoogleDriveTokenRefresher();

	virtual void setHeaders(Common::Array<Common::String> &headers);
	virtual void addHeader(Common::String header);
};

} // End of namespace GoogleDrive
} // End of namespace Cloud

#endif
