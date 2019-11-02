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

#ifndef BACKENDS_NETWORKING_CURL_CURLREQUEST_H
#define BACKENDS_NETWORKING_CURL_CURLREQUEST_H

#include "backends/networking/curl/request.h"
#include "common/str.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

struct curl_slist;

namespace Networking {

class NetworkReadStream;

typedef Response<NetworkReadStream *> NetworkReadStreamResponse;
typedef Common::BaseCallback<NetworkReadStreamResponse> *NetworkReadStreamCallback;

class CurlRequest: public Request {
protected:
	Common::String _url;
	NetworkReadStream *_stream;
	curl_slist *_headersList;
	Common::String _postFields;
	Common::HashMap<Common::String, Common::String> _formFields;
	Common::HashMap<Common::String, Common::String> _formFiles;
	byte *_bytesBuffer;
	uint32 _bytesBufferSize;
	bool _uploading; //using PUT method
	bool _usingPatch; //using PATCH method
	bool _keepAlive;
	long _keepAliveIdle, _keepAliveInterval;

	virtual NetworkReadStream *makeStream();

public:
	CurlRequest(DataCallback cb, ErrorCallback ecb, Common::String url);
	virtual ~CurlRequest();

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;

	/** Replaces all headers with the passed array of headers. */
	virtual void setHeaders(Common::Array<Common::String> &headers);

	/** Adds a header into headers list. */
	virtual void addHeader(Common::String header);

	/** Adds a post field (key=value pair). */
	virtual void addPostField(Common::String field);

	/** Adds a form/multipart field (name, value). */
	virtual void addFormField(Common::String name, Common::String value);

	/** Adds a form/multipart file (field name, file name). */
	virtual void addFormFile(Common::String name, Common::String filename);

	/** Sets bytes buffer. */
	virtual void setBuffer(byte *buffer, uint32 size);

	/** Remembers to use PUT method when it would create NetworkReadStream. */
	virtual void usePut();

	/** Remembers to use PATCH method when it would create NetworkReadStream. */
	virtual void usePatch();

	/** Remembers to use Connection: keep-alive or close. */
	virtual void connectionKeepAlive(long idle = 120, long interval = 60);
	virtual void connectionClose();

	/**
	 * Starts this Request with ConnMan.
	 * @return its NetworkReadStream in NetworkReadStreamResponse.
	 */
	virtual NetworkReadStreamResponse execute();

	/** Returns Request's NetworkReadStream. */
	const NetworkReadStream *getNetworkReadStream() const;

	/** Waits for Request to be processed. Should be called after Request is put into ConnMan. */
	void wait(int spinlockDelay = 5);
};

} // End of namespace Networking

#endif
