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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <curl/curl.h>
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"

namespace Networking {

CurlJsonRequest::CurlJsonRequest(JsonCallback cb, ErrorCallback ecb, Common::String url) :
	CurlRequest(nullptr, ecb, url), _jsonCallback(cb), _contentsStream(DisposeAfterUse::YES),
	_buffer(new byte[CURL_JSON_REQUEST_BUFFER_SIZE]) {}

CurlJsonRequest::~CurlJsonRequest() {
	delete _jsonCallback;
	delete[] _buffer;
}

char *CurlJsonRequest::getPreparedContents() {
	//write one more byte in the end
	byte zero[1] = {0};
	_contentsStream.write(zero, 1);

	//replace all "bad" bytes with '.' character
	byte *result = _contentsStream.getData();
	uint32 size = _contentsStream.size();
	for (uint32 i = 0; i < size; ++i) {
		if (result[i] == '\n')
				result[i] = ' '; //yeah, kinda stupid
		else if (result[i] < 0x20 || result[i] > 0x7f)
			result[i] = '.';
	}

	//make it zero-terminated string
	result[size - 1] = '\0';

	return (char *)result;
}

void CurlJsonRequest::handle() {
	if (!_stream) _stream = makeStream();

	if (_stream) {
		uint32 readBytes = _stream->read(_buffer, CURL_JSON_REQUEST_BUFFER_SIZE);
		if (readBytes != 0)
			if (_contentsStream.write(_buffer, readBytes) != readBytes)
				warning("CurlJsonRequest: unable to write all the bytes into MemoryWriteStreamDynamic");

		if (_stream->eos()) {
			char *contents = getPreparedContents();
			Common::JSONValue *json = Common::JSON::parse(contents);
			if (json) {
				finishJson(json); //it's JSON even if's not 200 OK? That's fine!..
			} else {
				if (_stream->httpResponseCode() == 200) //no JSON, but 200 OK? That's fine!..
					finishJson(nullptr);
				else
					finishError(ErrorResponse(this, false, true, contents, _stream->httpResponseCode()));
			}
		}
	}
}

void CurlJsonRequest::restart() {
	if (_stream)
		delete _stream;
	_stream = nullptr;
	_contentsStream = Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	//with no stream available next handle() will create another one
}

void CurlJsonRequest::finishJson(Common::JSONValue *json) {
	Request::finishSuccess();
	if (_jsonCallback)
		(*_jsonCallback)(JsonResponse(this, json)); //potential memory leak, free it in your callbacks!
	else
		delete json;
}

bool CurlJsonRequest::jsonIsObject(Common::JSONValue *item, const char *warningPrefix) {
	if (item == nullptr) {
		warning("%s: passed item is NULL", warningPrefix);
		return false;
	}

	if (item->isObject()) return true;

	warning("%s: passed item is not an object", warningPrefix);
	debug(9, "%s", item->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsObject(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	if (item.getVal(key)->isObject()) return true;

	warning("%s: passed item's \"%s\" attribute is not an object", warningPrefix, key);
	debug(9, "%s", item.getVal(key)->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsString(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	if (item.getVal(key)->isString()) return true;

	warning("%s: passed item's \"%s\" attribute is not a string", warningPrefix, key);
	debug(9, "%s", item.getVal(key)->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsIntegerNumber(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	if (item.getVal(key)->isIntegerNumber()) return true;

	warning("%s: passed item's \"%s\" attribute is not an integer", warningPrefix, key);
	debug(9, "%s", item.getVal(key)->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsArray(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	if (item.getVal(key)->isArray()) return true;

	warning("%s: passed item's \"%s\" attribute is not an array", warningPrefix, key);
	debug(9, "%s", item.getVal(key)->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsStringOrIntegerNumber(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	if (item.getVal(key)->isString() || item.getVal(key)->isIntegerNumber()) return true;

	warning("%s: passed item's \"%s\" attribute is neither a string or an integer", warningPrefix, key);
	debug(9, "%s", item.getVal(key)->stringify(true).c_str());
	return false;
}

bool CurlJsonRequest::jsonContainsAttribute(Common::JSONObject &item, const char *key, const char *warningPrefix, bool isOptional) {
	if (!item.contains(key)) {
		if (isOptional) {
			return true;
		}

		warning("%s: passed item misses the \"%s\" attribute", warningPrefix, key);
		return false;
	}

	return true;
}

} // End of namespace Networking
