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

#ifndef BACKENDS_CLOUD_ONEDRIVE_ONEDRIVETOKENREFRESHER_H
#define BACKENDS_CLOUD_ONEDRIVE_ONEDRIVETOKENREFRESHER_H

#include "backends/cloud/storage.h"
#include "common/callback.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace OneDrive {

class OneDriveStorage;

class OneDriveTokenRefresher: public Networking::CurlJsonRequest {
	OneDriveStorage *_parentStorage;
	Common::Array<Common::String> _headers;
	Networking::CurlJsonRequest *_innerRequest;
	Networking::JsonCallback _jsonCallback;
	int32 _retryId;
	bool _started;

	void innerRequestCallback(Networking::RequestJsonPair pair);
	void tokenRefreshed(Storage::RequestBoolPair pair);
public:	
	OneDriveTokenRefresher(OneDriveStorage *parent, Networking::JsonCallback callback, const char *url);
	virtual ~OneDriveTokenRefresher();

	virtual void handle();
	virtual void restart();

	virtual void setHeaders(Common::Array<Common::String> &headers) { _headers = headers; }
	virtual void addHeader(Common::String header) { _headers.push_back(header); }
	virtual void addPostField(Common::String field) { _innerRequest->addPostField(field); }
	virtual Networking::NetworkReadStream *execute();
};

} //end of namespace OneDrive
} //end of namespace Cloud

#endif
