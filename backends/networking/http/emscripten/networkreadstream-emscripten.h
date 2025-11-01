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

#ifndef BACKENDS_NETWORKING_HTTP_EMSCRIPTEN_NETWORKREADSTREAMEMSCRIPTEN_H
#define BACKENDS_NETWORKING_HTTP_EMSCRIPTEN_NETWORKREADSTREAMEMSCRIPTEN_H

#ifdef EMSCRIPTEN

#include "backends/networking/http/networkreadstream.h"
#include <emscripten/fetch.h>

namespace Networking {

class NetworkReadStream; // Forward declaration

class NetworkReadStreamEmscripten : public NetworkReadStream {
private:
	emscripten_fetch_attr_t *_emscripten_fetch_attr;
	emscripten_fetch_t *_emscripten_fetch;
	const char *_emscripten_fetch_url = nullptr;
	char **_emscripten_request_headers;
	bool _success;
	char *_errorBuffer;

	void resetStream();
	void setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post);
	void setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles);
public:
	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	~NetworkReadStreamEmscripten() override;
	void initEmscripten(const char *url, RequestHeaders *headersList);

	// NetworkReadStream interface
	bool reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading = false, bool usingPatch = false) override { return false; }                                                 // no reuse for Emscripten
	bool reuse(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) override { return false; } // no reuse for Emscripten
	bool reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = false) override { return false; }                         // no reuse for Emscripten

	bool hasError() const override;
	const char *getError() const override;

	long httpResponseCode() const override;
	Common::String currentLocation() const override;
	Common::HashMap<Common::String, Common::String> responseHeadersMap() const override;

	uint32 read(void *dataPtr, uint32 dataSize) override;

	// Static callback functions
	static void emscriptenOnSuccess(emscripten_fetch_t *fetch);
	static void emscriptenOnError(emscripten_fetch_t *fetch);
	static void emscriptenOnProgress(emscripten_fetch_t *fetch);
	static void emscriptenOnReadyStateChange(emscripten_fetch_t *fetch);
	void emscriptenDownloadFinished(bool success);
};

} // End of namespace Networking

#endif // EMSCRIPTEN

#endif
