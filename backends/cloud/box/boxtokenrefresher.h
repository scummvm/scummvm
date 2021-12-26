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

#ifndef BACKENDS_CLOUD_BOX_BOXTOKENREFRESHER_H
#define BACKENDS_CLOUD_BOX_BOXTOKENREFRESHER_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Box {

class BoxStorage;

class BoxTokenRefresher: public Networking::CurlJsonRequest {
	BoxStorage *_parentStorage;
	Common::Array<Common::String> _headers;

	void tokenRefreshed(Storage::BoolResponse response);

	virtual void finishJson(Common::JSONValue *json);
	virtual void finishError(Networking::ErrorResponse error, Networking::RequestState state = Networking::FINISHED);
public:
	BoxTokenRefresher(BoxStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url);
	virtual ~BoxTokenRefresher();

	virtual void setHeaders(Common::Array<Common::String> &headers);
	virtual void addHeader(Common::String header);
};

} // End of namespace Box
} // End of namespace Cloud

#endif
