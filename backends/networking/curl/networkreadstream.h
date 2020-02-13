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

#ifndef BACKENDS_NETWORKING_CURL_NETWORKREADSTREAM_H
#define BACKENDS_NETWORKING_CURL_NETWORKREADSTREAM_H

#include "common/memstream.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

typedef void CURL;
struct curl_slist;

namespace Networking {

class NetworkReadStream: public Common::ReadStream {
	CURL *_easy;
	Common::MemoryReadWriteStream _backingStream;
	bool _keepAlive;
	long _keepAliveIdle, _keepAliveInterval;
	bool _eos, _requestComplete;
	char *_errorBuffer;
	const byte *_sendingContentsBuffer;
	uint32 _sendingContentsSize;
	uint32 _sendingContentsPos;
	byte *_bufferCopy; // To use with old curl version where CURLOPT_COPYPOSTFIELDS is not available
	Common::String _responseHeaders;
	uint64 _progressDownloaded, _progressTotal;

	void resetStream();
	void initCurl(const char *url, curl_slist *headersList);
	bool reuseCurl(const char *url, curl_slist *headersList);
	void setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post);
	void setupFormMultipart(Common::HashMap<Common::String, Common::String> formFields, Common::HashMap<Common::String, Common::String> formFiles);

	/**
	 * Fills the passed buffer with _sendingContentsBuffer contents.
	 * It works similarly to read(), expect it's not for reading
	 * Stream's contents, but for sending our own data to the server.
	 *
	 * @returns how many bytes were actually read (filled in)
	 */
	uint32 fillWithSendingContents(char *bufferToFill, uint32 maxSize);

	/**
	* Remembers headers returned to CURL in server's response.
	*
	* @returns how many bytes were actually read
	*/
	uint32 addResponseHeaders(char *buffer, uint32 bufferSize);

	static size_t curlDataCallback(char *d, size_t n, size_t l, void *p);
	static size_t curlReadDataCallback(char *d, size_t n, size_t l, void *p);
	static size_t curlHeadersCallback(char *d, size_t n, size_t l, void *p);
	static int curlProgressCallbackOlder(void *p, double dltotal, double dlnow, double ultotal, double ulnow);
public:
	/** Send <postFields>, using POST by default. */
	NetworkReadStream(const char *url, curl_slist *headersList, Common::String postFields, bool uploading = false, bool usingPatch = false, bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);
	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	NetworkReadStream(
	    const char *url, curl_slist *headersList,
	    Common::HashMap<Common::String, Common::String> formFields,
	    Common::HashMap<Common::String, Common::String> formFiles,
		bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);
	/** Send <buffer>, using POST by default. */
	NetworkReadStream(const char *url, curl_slist *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = true, bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);
	virtual ~NetworkReadStream();

	/** Send <postFields>, using POST by default. */
	bool reuse(const char *url, curl_slist *headersList, Common::String postFields, bool uploading = false, bool usingPatch = false);
	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	bool reuse(
		const char *url, curl_slist *headersList,
		Common::HashMap<Common::String, Common::String> formFields,
		Common::HashMap<Common::String, Common::String> formFiles);
	/** Send <buffer>, using POST by default. */
	bool reuse(const char *url, curl_slist *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = true);

	/**
	 * Returns true if a read failed because the stream end has been reached.
	 * This flag is cleared by clearErr().
	 * For a SeekableReadStream, it is also cleared by a successful seek.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C feof(). In particular, in a stream
	 * with N bytes, reading exactly N bytes from the start should *not*
	 * set eos; only reading *beyond* the available data should set it.
	 */
	virtual bool eos() const;

	/**
	 * Read data from the stream. Subclasses must implement this
	 * method; all other read methods are implemented using it.
	 *
	 * @note The semantics of any implementation of this method are
	 * supposed to match those of ISO C fread(), in particular where
	 * it concerns setting error and end of file/stream flags.
	 *
	 * @param dataPtr   pointer to a buffer into which the data is read
	 * @param dataSize  number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize);

	/**
	 * This method is called by ConnectionManager to indicate
	 * that transfer is finished.
	 *
	 * @note It's called on failure too.
	 */
	void finished(uint32 errorCode);

	/**
	 * Returns HTTP response code from inner CURL handle.
	 * It returns -1 to indicate there is no inner handle.
	 *
	 * @note This method should be called when eos() == true.
	 */
	long httpResponseCode() const;

	/**
	* Return current location URL from inner CURL handle.
	* "" is returned to indicate there is no inner handle.
	*
	* @note This method should be called when eos() == true.
	*/
	Common::String currentLocation() const;

	/**
	* Return response headers.
	*
	* @note This method should be called when eos() == true.
	*/
	Common::String responseHeaders() const;

	/**
	* Return response headers as HashMap. All header names in
	* it are lowercase.
	*
	* @note This method should be called when eos() == true.
	*/
	Common::HashMap<Common::String, Common::String> responseHeadersMap() const;

	/** Returns a number in range [0, 1], where 1 is "complete". */
	double getProgress() const;

	/** Used in curl progress callback to pass current downloaded/total values. */
	void setProgress(uint64 downloaded, uint64 total);

	bool keepAlive() const { return _keepAlive; }
};

} // End of namespace Networking

#endif
