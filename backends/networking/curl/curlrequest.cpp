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
#include "backends/networking/curl/curlrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/textconsole.h"

namespace Networking {

CurlRequest::CurlRequest(DataCallback cb, ErrorCallback ecb, Common::String url):
	Request(cb, ecb), _url(url), _stream(nullptr), _headersList(nullptr), _bytesBuffer(nullptr),
	_bytesBufferSize(0), _uploading(false), _usingPatch(false), _keepAlive(false), _keepAliveIdle(120), _keepAliveInterval(60) {}

CurlRequest::~CurlRequest() {
	delete _stream;
	delete _bytesBuffer;
}

NetworkReadStream *CurlRequest::makeStream() {
	if (_bytesBuffer)
		return new NetworkReadStream(_url.c_str(), _headersList, _bytesBuffer, _bytesBufferSize, _uploading, _usingPatch, true, _keepAlive, _keepAliveIdle, _keepAliveInterval);
	if (!_formFields.empty() || !_formFiles.empty())
		return new NetworkReadStream(_url.c_str(), _headersList, _formFields, _formFiles, _keepAlive, _keepAliveIdle, _keepAliveInterval);
	return new NetworkReadStream(_url.c_str(), _headersList, _postFields, _uploading, _usingPatch, _keepAlive, _keepAliveIdle, _keepAliveInterval);
}

void CurlRequest::handle() {
	if (!_stream) _stream = makeStream();

	if (_stream && _stream->eos()) {
		if (_stream->httpResponseCode() != 200) {
			warning("CurlRequest: HTTP response code is not 200 OK (it's %ld)", _stream->httpResponseCode());
			ErrorResponse error(this, false, true, "HTTP response code is not 200 OK", _stream->httpResponseCode());
			finishError(error);
			return;
		}

		finishSuccess(); //note that this Request doesn't call its callback on success (that's because it has nothing to return)
	}
}

void CurlRequest::restart() {
	if (_stream)
		delete _stream;
	_stream = nullptr;
	//with no stream available next handle() will create another one
}

Common::String CurlRequest::date() const {
	if (_stream) {
		Common::HashMap<Common::String, Common::String> headers = _stream->responseHeadersMap();
		if (headers.contains("date"))
			return headers["date"];
	}
	return "";
}

void CurlRequest::setHeaders(Common::Array<Common::String> &headers) {
	curl_slist_free_all(_headersList);
	_headersList = nullptr;
	for (uint32 i = 0; i < headers.size(); ++i)
		addHeader(headers[i]);
}

void CurlRequest::addHeader(Common::String header) {
	_headersList = curl_slist_append(_headersList, header.c_str());
}

void CurlRequest::addPostField(Common::String keyValuePair) {
	if (_bytesBuffer)
		warning("CurlRequest: added POST fields would be ignored, because there is buffer present");

	if (!_formFields.empty() || !_formFiles.empty())
		warning("CurlRequest: added POST fields would be ignored, because there are form fields/files present");

	if (_postFields == "")
		_postFields = keyValuePair;
	else
		_postFields += "&" + keyValuePair;
}

void CurlRequest::addFormField(Common::String name, Common::String value) {
	if (_bytesBuffer)
		warning("CurlRequest: added POST form fields would be ignored, because there is buffer present");

	if (_formFields.contains(name))
		warning("CurlRequest: form field '%s' already had a value", name.c_str());

	_formFields[name] = value;
}

void CurlRequest::addFormFile(Common::String name, Common::String filename) {
	if (_bytesBuffer)
		warning("CurlRequest: added POST form files would be ignored, because there is buffer present");

	if (_formFields.contains(name))
		warning("CurlRequest: form file field '%s' already had a value", name.c_str());

	_formFiles[name] = filename;
}

void CurlRequest::setBuffer(byte *buffer, uint32 size) {
	if (_postFields != "")
		warning("CurlRequest: added POST fields would be ignored, because buffer added");

	if (_bytesBuffer)
		delete _bytesBuffer;

	_bytesBuffer = buffer;
	_bytesBufferSize = size;
}

void CurlRequest::usePut() { _uploading = true; }

void CurlRequest::usePatch() { _usingPatch = true; }

void CurlRequest::connectionKeepAlive(long idle, long interval) {
	_keepAlive = true;
	_keepAliveIdle = idle;
	_keepAliveInterval = interval;
}

void CurlRequest::connectionClose() {
	_keepAlive = false;
}

NetworkReadStreamResponse CurlRequest::execute() {
	if (!_stream) {
		_stream = makeStream();
		ConnMan.addRequest(this);
	}

	return NetworkReadStreamResponse(this, _stream);
}

const NetworkReadStream *CurlRequest::getNetworkReadStream() const { return _stream; }

void CurlRequest::wait(int spinlockDelay) {
	while (state() == Networking::PROCESSING) {
		g_system->delayMillis(spinlockDelay);
	}
}

} // End of namespace Networking
