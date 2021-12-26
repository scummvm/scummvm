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

#ifndef BACKENDS_NETWORKING_CURL_SESSIONREQUEST_H
#define BACKENDS_NETWORKING_CURL_SESSIONREQUEST_H

#include "backends/networking/curl/curlrequest.h"
#include "common/memstream.h"

namespace Common {
class DumpFile;
class JSONValue;
}

namespace Networking {

#define CURL_SESSION_REQUEST_BUFFER_SIZE 512 * 1024

struct SessionFileResponse {
	byte *buffer;
	uint32 len;
	bool eos;
};

/**
 * @brief Class for reading file and storing locally
 *
 * @return Returns SessionFileResponse in the callback
 */

class SessionRequest: public CurlRequest {
protected:
	Common::MemoryWriteStreamDynamic _contentsStream;
	byte *_buffer;
	char *_text;
	bool _started, _complete, _success;
	bool _binary;
	Common::DumpFile *_localFile;
	SessionFileResponse _response;

	bool reuseStream();

	/** Prepares raw bytes from _contentsStream. */
	char *getPreparedContents();

	virtual void finishError(ErrorResponse error, RequestState state = PAUSED);
	virtual void finishSuccess();
	void openLocalFile(Common::String localFile);

public:
	SessionRequest(Common::String url, Common::String localFile, DataCallback cb = nullptr, ErrorCallback ecb = nullptr, bool binary = false);
	virtual ~SessionRequest();

	void start();
	void startAndWait();

	void reuse(Common::String url, Common::String localFile, DataCallback cb = nullptr, ErrorCallback ecb = nullptr, bool binary = false);

	virtual void handle();
	virtual void restart();

	/** This request DOES NOT delete automatically after calling callbacks. It gets PAUSED, and in order to make it FINISHED (i.e. delete), this method MUST be called. */
	void close();

	/**
	 * @brief Closes the current request and removes any unfinished files
	 *
	 */
	void abortRequest();

	bool complete();
	bool success();

	char *text();
	Common::JSONValue *json();

	byte *getData() { return _contentsStream.getData(); }
	uint32 getSize() { return _contentsStream.size(); }

};

} // End of namespace Networking

#endif
