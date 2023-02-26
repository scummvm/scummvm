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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <curl/curl.h>
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "backends/networking/curl/sessionrequest.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/formats/json.h"

namespace Networking {

SessionRequest::SessionRequest(Common::String url, Common::String localFile, DataCallback cb, ErrorCallback ecb, bool binary):
	CurlRequest(cb, ecb, url), _contentsStream(DisposeAfterUse::YES),
	_buffer(new byte[CURL_SESSION_REQUEST_BUFFER_SIZE]), _text(nullptr), _localFile(nullptr),
	_started(false), _complete(false), _success(false), _binary(binary) {

	openLocalFile(localFile);

	// automatically go under ConnMan control so nobody would be able to leak the memory
	// but, we don't need it to be working just yet
	_state = PAUSED;
	ConnMan.addRequest(this);
}

SessionRequest::~SessionRequest() {
	delete[] _buffer;
}

void SessionRequest::openLocalFile(Common::String localFile) {
	if (localFile.empty())
		return;

	_localFile = new Common::DumpFile();
	if (!_localFile->open(localFile, true)) {
		warning("SessionRequestFile: unable to open file to download into");
		ErrorResponse error(this, false, true, "SessionRequestFile: unable to open file to download into", -1);
		finishError(error);
		delete _localFile;
		_localFile = nullptr;
		return;
	}

	debug(5, "SessionRequest: opened localfile %s", localFile.c_str());

	_binary = true; // Enforce binary
}

bool SessionRequest::reuseStream() {
	if (!_stream) {
		return false;
	}

	if (_bytesBuffer)
		return _stream->reuse(_url.c_str(), _headersList, _bytesBuffer, _bytesBufferSize, _uploading, _usingPatch, true);

	if (!_formFields.empty() || !_formFiles.empty())
		return _stream->reuse(_url.c_str(), _headersList, _formFields, _formFiles);

	return _stream->reuse(_url.c_str(), _headersList, _postFields, _uploading, _usingPatch);
}

char *SessionRequest::getPreparedContents() {
	//write one more byte in the end
	byte zero[1] = {0};
	_contentsStream.write(zero, 1);

	//replace all "bad" bytes with '.' character
	byte *result = _contentsStream.getData();
	uint32 size = _contentsStream.size();

	//make it zero-terminated string
	result[size - 1] = '\0';

	return (char *)result;
}

void SessionRequest::finishError(ErrorResponse error, RequestState state) {
	_complete = true;
	_success = false;
	CurlRequest::finishError(error, PAUSED);
}

void SessionRequest::finishSuccess() {
	_state = PAUSED;
	_complete = true;
	_success = true;

	if (_localFile) {
		_localFile->close();
		delete _localFile;
		_localFile = nullptr;
	}

	if (_callback) {	// If localfile is present, contentStream is empty, so it is fine
		_response.buffer = _contentsStream.getData();
		_response.len = _contentsStream.size();
		_response.eos = true;

		(*_callback)(DataResponse(this, &_response));
	}
}

void SessionRequest::start() {
	if (_state != PAUSED || _started) {
		warning("Can't start() SessionRequest as it is already started");
		return;
	}

	_state = PROCESSING;
	_started = true;
}

void SessionRequest::startAndWait() {
	start();
	wait();
}

void SessionRequest::reuse(Common::String url, Common::String localFile, DataCallback cb, ErrorCallback ecb, bool binary) {
	_url = url;

	delete _callback;
	delete _errorCallback;
	_callback = cb;
	_errorCallback = ecb;

	_binary = binary;

	openLocalFile(localFile);

	restart();
}

void SessionRequest::handle() {
	if (!_stream) _stream = makeStream();

	if (_stream) {
		if (_stream->httpResponseCode() != 200 && _stream->httpResponseCode() != 0) {
			warning("SessionRequest: HTTP response code is not 200 OK (it's %ld)", _stream->httpResponseCode());
			ErrorResponse error(this, false, true, "HTTP response code is not 200 OK", _stream->httpResponseCode());
			finishError(error);
			return;
		}
		uint32 readBytes = _stream->read(_buffer, CURL_SESSION_REQUEST_BUFFER_SIZE);
		if (readBytes != 0) {
			if (!_localFile) {
				if (_contentsStream.write(_buffer, readBytes) != readBytes)
					warning("SessionRequest: unable to write all the bytes into MemoryWriteStreamDynamic");
			} else {
				_response.buffer = _buffer;
				_response.len = readBytes;
				_response.eos = _stream->eos();

				if (_localFile->write(_buffer, readBytes) != readBytes) {
					warning("DownloadRequest: unable to write all received bytes into output file");
					finishError(Networking::ErrorResponse(this, false, true, "DownloadRequest::handle: failed to write all bytes into a file", -1));
					return;
				}

				if (_callback)
					(*_callback)(DataResponse(this, &_response));
			}
		}

		if (_stream->eos()) {
			if (_stream->hasError()) {
				ErrorResponse error(this, false, true, Common::String::format("TLS stream response code is not CURLE_OK OK: %s", _stream->getError()), _stream->getErrorCode());
				finishError(error);
				return;
			}

			finishSuccess();
		}
	}
}

void SessionRequest::restart() {
	if (_stream) {
		bool deleteStream = true;
		if (_keepAlive && reuseStream()) {
			deleteStream = false;
		}

		if (deleteStream) {
			delete _stream;
			_stream = nullptr;
		}
	}

	_contentsStream = Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	_text = nullptr;
	_complete = false;
	_success = false;
	_started = false;
	//with no stream available next handle() will create another one
}

void SessionRequest::close() {
	_state = FINISHED;
}

void SessionRequest::abortRequest() {
	if (_localFile) {
		_localFile->close();
		delete _localFile;
		_localFile = nullptr;

		// TODO we need to remove file, but there is no API
	}
	_state = FINISHED;
}

bool SessionRequest::complete() {
	return _complete;
}

bool SessionRequest::success() {
	return _success;
}

char *SessionRequest::text() {
	if (_binary || _localFile)
		return nullptr;

	if (_text == nullptr)
		_text = getPreparedContents();
	return _text;
}

Common::JSONValue *SessionRequest::json() {
	if (_binary)
		error("SessionRequest::json() is called for binary stream");
	if (_localFile)
		error("SessionRequest::json() is called for localFile stream");
	return Common::JSON::parse(text());
}

} // End of namespace Networking
