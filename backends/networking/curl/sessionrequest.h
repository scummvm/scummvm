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

#ifndef BACKENDS_NETWORKING_CURL_SESSIONREQUEST_H
#define BACKENDS_NETWORKING_CURL_SESSIONREQUEST_H

#include "backends/networking/curl/curlrequest.h"
#include "common/memstream.h"
#include "common/json.h"

namespace Networking {

#define CURL_SESSION_REQUEST_BUFFER_SIZE 512 * 1024

class SessionRequest: public CurlRequest {
protected:
	Common::MemoryWriteStreamDynamic _contentsStream;
	byte *_buffer;
	char *_text;
	bool _started, _complete, _success;

	bool reuseStream();

	/** Prepares raw bytes from _contentsStream. */
	char *getPreparedContents();

	virtual void finishError(ErrorResponse error);
	virtual void finishSuccess();

public:
	SessionRequest(Common::String url, DataCallback cb = nullptr, ErrorCallback ecb = nullptr);
	virtual ~SessionRequest();

	void start();
	void startAndWait();

	void reuse(Common::String url, DataCallback cb = nullptr, ErrorCallback ecb = nullptr);

	virtual void handle();
	virtual void restart();

	/** This request DOES NOT delete automatically after calling callbacks. It gets PAUSED, and in order to make it FINISHED (i.e. delete), this method MUST be called. */
	void close();	

	bool complete();
	bool success();

	char *text();
	Common::JSONValue *json();
};

} // End of namespace Networking

#endif
