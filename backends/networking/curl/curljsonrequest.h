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

#ifndef BACKENDS_NETWORKING_CURL_CURLJSONREQUEST_H
#define BACKENDS_NETWORKING_CURL_CURLJSONREQUEST_H

#include "backends/networking/curl/curlrequest.h"
#include "common/memstream.h"
#include "common/json.h"

namespace Networking {

typedef Response<Common::JSONValue *> JsonResponse;
typedef Common::BaseCallback<JsonResponse> *JsonCallback;
typedef Common::BaseCallback<Common::JSONValue *> *JSONValueCallback;

#define CURL_JSON_REQUEST_BUFFER_SIZE 512 * 1024

class CurlJsonRequest: public CurlRequest {
protected:
	JsonCallback _jsonCallback;
	Common::MemoryWriteStreamDynamic _contentsStream;
	byte *_buffer;

	/** Prepares raw bytes from _contentsStream to be parsed with Common::JSON::parse(). */
	char *getPreparedContents();

	/** Sets FINISHED state and passes the JSONValue * into user's callback in JsonResponse. */
	virtual void finishJson(Common::JSONValue *json);

public:
	CurlJsonRequest(JsonCallback cb, ErrorCallback ecb, Common::String url);
	virtual ~CurlJsonRequest();

	virtual void handle();
	virtual void restart();

	static bool jsonIsObject(Common::JSONValue *item, const char *warningPrefix);
	static bool jsonContainsObject(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
	static bool jsonContainsString(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
	static bool jsonContainsIntegerNumber(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
	static bool jsonContainsArray(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
	static bool jsonContainsStringOrIntegerNumber(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
	static bool jsonContainsAttribute(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional = false);
};

} // End of namespace Networking

#endif
