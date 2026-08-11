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

#include "backends/networking/http/httprequest.h"
#include "backends/networking/http/connectionmanager.h"
#include "backends/networking/http/networkreadstream.h"
#include "common/textconsole.h"

namespace Networking {

HttpRequest::HttpRequest(DataCallback cb, ErrorCallback ecb, const Common::String &url):
	Request(cb, ecb), _url(url), _stream(nullptr), _bytesBuffer(nullptr),
	_bytesBufferSize(0), _uploading(false), _usingPatch(false), _keepAlive(false), _keepAliveIdle(120), _keepAliveInterval(60) {}

HttpRequest::~HttpRequest() {
	delete _stream;
	delete[] _bytesBuffer;
}

NetworkReadStream *HttpRequest::makeStream() {
	if (_bytesBuffer)
		return NetworkReadStream::make(_url.c_str(), &_headersList, _bytesBuffer, _bytesBufferSize, _uploading, _usingPatch, true, _keepAlive, _keepAliveIdle, _keepAliveInterval);
	if (!_formFields.empty() || !_formFiles.empty())
		return NetworkReadStream::make(_url.c_str(), &_headersList, _formFields, _formFiles, _keepAlive, _keepAliveIdle, _keepAliveInterval);
	return NetworkReadStream::make(_url.c_str(), &_headersList, _postFields, _uploading, _usingPatch, _keepAlive, _keepAliveIdle, _keepAliveInterval);
}

void HttpRequest::handle() {
	if (!_stream) _stream = makeStream();

	if (_stream && _stream->eos()) {
		if (_stream->httpResponseCode() != 200) {
			warning("HttpRequest: HTTP response code is not 200 OK (it's %ld)", _stream->httpResponseCode());
			ErrorResponse error(this, false, true, "HTTP response code is not 200 OK", _stream->httpResponseCode());
			finishError(error);
			return;
		}

		finishSuccess(); //note that this Request doesn't call its callback on success (that's because it has nothing to return)
	}
}

void HttpRequest::restart() {
	if (_stream)
		delete _stream;
	_stream = nullptr;
	//with no stream available next handle() will create another one
}

Common::String HttpRequest::date() const {
	if (_stream) {
		Common::HashMap<Common::String, Common::String> headers = _stream->responseHeadersMap();
		if (headers.contains("date"))
			return headers["date"];
	}
	return "";
}

void HttpRequest::setHeaders(const Common::Array<Common::String> &headers) {
	_headersList = headers;
}

void HttpRequest::addHeader(const Common::String &header) {
	_headersList.push_back(header);
}

void HttpRequest::addPostField(const Common::String &keyValuePair) {
	if (_bytesBuffer)
		warning("HttpRequest: added POST fields would be ignored, because there is buffer present");

	if (!_formFields.empty() || !_formFiles.empty())
		warning("HttpRequest: added POST fields would be ignored, because there are form fields/files present");

	if (_postFields == "")
		_postFields = keyValuePair;
	else
		_postFields += "&" + keyValuePair;
}

void HttpRequest::addFormField(const Common::String &name, const Common::String &value) {
	if (_bytesBuffer)
		warning("HttpRequest: added POST form fields would be ignored, because there is buffer present");

	if (_formFields.contains(name))
		warning("HttpRequest: form field '%s' already had a value", name.c_str());

	_formFields[name] = value;
}

void HttpRequest::addFormFile(const Common::String &name, const Common::Path &filename) {
	if (_bytesBuffer)
		warning("HttpRequest: added POST form files would be ignored, because there is buffer present");

	if (_formFields.contains(name))
		warning("HttpRequest: form file field '%s' already had a value", name.c_str());

	_formFiles[name] = filename;
}

void HttpRequest::setBuffer(byte *buffer, uint32 size) {
	if (_postFields != "")
		warning("HttpRequest: added POST fields would be ignored, because buffer added");

	if (_bytesBuffer)
		delete[] _bytesBuffer;

	_bytesBuffer = buffer;
	_bytesBufferSize = size;
}

void HttpRequest::usePut() { _uploading = true; }

void HttpRequest::usePatch() { _usingPatch = true; }

void HttpRequest::connectionKeepAlive(long idle, long interval) {
	_keepAlive = true;
	_keepAliveIdle = idle;
	_keepAliveInterval = interval;
}

void HttpRequest::connectionClose() {
	_keepAlive = false;
}

NetworkReadStreamResponse HttpRequest::execute() {
	if (!_stream) {
		_stream = makeStream();
		ConnMan.addRequest(this);
	}

	return NetworkReadStreamResponse(this, _stream);
}

const NetworkReadStream *HttpRequest::getNetworkReadStream() const { return _stream; }

void HttpRequest::wait(int spinlockDelay) {
	while (state() == Networking::PROCESSING) {
		g_system->delayMillis(spinlockDelay);
	}
}

} // End of namespace Networking
