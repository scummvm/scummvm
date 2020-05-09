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

#ifndef BACKENDS_NETWORKING_CURL_POSTREQUEST_H
#define BACKENDS_NETWORKING_CURL_POSTREQUEST_H

#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Networking {

class PostRequest: public Networking::Request {
	Common::String _url;
	Networking::JSONValueCallback _jsonCallback;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	byte *_postData;
	int _postLen;
	Common::JSONValue *_jsonData;

	Common::String _contentType;

	void responseCallback(Networking::JsonResponse response);
	void errorCallback(Networking::ErrorResponse error);

public:
	PostRequest(Common::String url, Networking::JSONValueCallback cb, Networking::ErrorCallback ecb);
	virtual ~PostRequest();

	void start();

	void setPostData(byte *postData, int postLen);
	void setJSONData(Common::JSONValue *jsonData);
	void setContentType(Common::String type) { _contentType = type; }

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;
};

} // End of namespace Networking

#endif
