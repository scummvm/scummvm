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

#ifndef BACKENDS_NETWORKING_HTTP_CURL_NETWORKREADSTREAMCURL_H
#define BACKENDS_NETWORKING_HTTP_CURL_NETWORKREADSTREAMCURL_H

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/http/networkreadstream.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/str.h"
#include "common/stream.h"

#include <curl/curl.h>

namespace Networking {

class NetworkReadStreamCurl : public NetworkReadStream {
private:
	CURL *_easy;
	struct curl_slist *_headersSlist;
	char *_errorBuffer;
	CURLcode _errorCode;
	byte *_bufferCopy; // To use with old curl version where CURLOPT_COPYPOSTFIELDS is not available
	void initCurl(const char *url, RequestHeaders *headersList);
	bool reuseCurl(const char *url, RequestHeaders *headersList);
	static struct curl_slist *requestHeadersToSlist(const RequestHeaders *headersList);

	static size_t curlDataCallback(char *d, size_t n, size_t l, void *p);
	static size_t curlReadDataCallback(char *d, size_t n, size_t l, void *p);
	static size_t curlHeadersCallback(char *d, size_t n, size_t l, void *p);
	static int curlProgressCallbackOlder(void *p, double dltotal, double dlnow, double ultotal, double ulnow);

	// CURL-specific methods
	CURL *getEasyHandle() const { return _easy; }
	void resetStream();
	void setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post);
	void setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles);

public:
	NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	~NetworkReadStreamCurl();
	void finished(CURLcode errorCode);

	/** Send <postFields>, using POST by default. */
	bool reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading = false, bool usingPatch = false) override;
	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	bool reuse(
		const char *url, RequestHeaders *headersList,
		const Common::HashMap<Common::String, Common::String> &formFields,
		const Common::HashMap<Common::String, Common::Path> &formFiles) override;
	/** Send <buffer>, using POST by default. */
	bool reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = true) override;

	long httpResponseCode() const override;
	Common::String currentLocation() const override;
	/**
	 * Return response headers as HashMap. All header names in
	 * it are lowercase.
	 *
	 * @note This method should be called when eos() == true.
	 */
	Common::HashMap<Common::String, Common::String> responseHeadersMap() const override;

	bool hasError() const override;
	const char *getError() const override;
};

} // End of namespace Networking

#endif
