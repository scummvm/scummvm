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

extern "C" {
// External JS functions for HTTP fetching
extern void NetworkReadStreamEmscripten_init(void);
extern int NetworkReadStreamEmscripten_fetch(const char *method, const char *url,
											 char **requestHeaders,
											 const char *requestData, size_t requestDataSize,
											 char **formFields = nullptr,
											 char **formFiles = nullptr);
extern void NetworkReadStreamEmscripten_close(int fetchId);

// Getter functions for fetch properties
extern char *NetworkReadStreamEmscripten_getDataPtr(int fetchId);
extern char *NetworkReadStreamEmscripten_getErr(int fetchId);
extern uint32 NetworkReadStreamEmscripten_getNumBytes(int fetchId);
extern char **NetworkReadStreamEmscripten_responseHeadersArray(int fetchId);
extern unsigned short NetworkReadStreamEmscripten_status(int fetchId);
extern uint32 NetworkReadStreamEmscripten_totalBytes(int fetchId);
extern bool NetworkReadStreamEmscripten_completed(int fetchId);
extern bool NetworkReadStreamEmscripten_hasError(int fetchId);
}

namespace Networking {

class NetworkReadStreamEmscripten : public NetworkReadStream {
private:
	int _fetchId; // Fetch ID instead of pointer
	Common::String _url;
	RequestHeaders *_headersList;
	uint32 _readPos; // Current read position in the JS buffer

	// Helper methods
	static char **buildHeadersArray(const RequestHeaders *headersList);
	static char **buildFormFieldsArray(const Common::HashMap<Common::String, Common::String> &formFields);
	static char **buildFormFilesArray(const Common::HashMap<Common::String, Common::Path> &formFiles);
	static void cleanupStringArray(char **array);

	void resetStream();
	void setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post);
	void setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles);

public:
	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval);

	~NetworkReadStreamEmscripten() override;
	void initFetch();

	// NetworkReadStream interface
	bool reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading = false, bool usingPatch = false) override { return false; }                                                 // no reuse for Emscripten
	bool reuse(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) override { return false; } // no reuse for Emscripten
	bool reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = false) override { return false; }                         // no reuse for Emscripten

	bool hasError() const override;
	const char *getError() const override;
	double getProgress() const override;
	long httpResponseCode() const override;
	Common::String currentLocation() const override;
	Common::HashMap<Common::String, Common::String> responseHeadersMap() const override;

	uint32 read(void *dataPtr, uint32 dataSize) override;
	bool eos() const override;
};

} // End of namespace Networking

#endif // EMSCRIPTEN

#endif
