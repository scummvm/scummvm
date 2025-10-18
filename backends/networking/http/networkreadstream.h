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

#ifndef BACKENDS_NETWORKING_HTTP_NETWORKREADSTREAM_H
#define BACKENDS_NETWORKING_HTTP_NETWORKREADSTREAM_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/str.h"
#include "common/stream.h"

namespace Networking {
typedef Common::Array<Common::String> RequestHeaders;

// Simple interface for platform-specific NetworkReadStream implementations
class NetworkReadStream : public Common::ReadStream {
protected:
	Common::MemoryReadWriteStream _backingStream;
	bool _keepAlive;
	long _keepAliveIdle, _keepAliveInterval;
	bool _eos, _requestComplete;
	const byte *_sendingContentsBuffer;
	uint32 _sendingContentsSize;
	uint32 _sendingContentsPos;
	Common::String _responseHeaders;
	uint64 _progressDownloaded, _progressTotal;

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

	NetworkReadStream(bool keepAlive, long keepAliveIdle, long keepAliveInterval)
		: _backingStream(DisposeAfterUse::YES), _eos(false), _requestComplete(false), _sendingContentsBuffer(nullptr),
		  _sendingContentsSize(0), _sendingContentsPos(0), _progressDownloaded(0), _progressTotal(0),
		  _keepAlive(keepAlive), _keepAliveIdle(keepAliveIdle), _keepAliveInterval(keepAliveInterval) {
	}

public:
	/* Implementation-defined Constructors */

	/** Send <postFields>, using POST by default. */
	static NetworkReadStream *make(const char *url, RequestHeaders *headersList, const Common::String &postFields,
			bool uploading = false, bool usingPatch = false,
			bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);

	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	static NetworkReadStream *make(const char *url, RequestHeaders *headersList,
			const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles,
			bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);

	/** Send <buffer>, using POST by default. */
	static NetworkReadStream *make(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize,
			bool uploading = false, bool usingPatch = false, bool post = true,
			bool keepAlive = false, long keepAliveIdle = 120, long keepAliveInterval = 60);

	/** Send <postFields>, using POST by default. */
	virtual bool reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading = false, bool usingPatch = false) = 0;
	/** Send <formFields>, <formFiles>, using POST multipart/form. */
	virtual bool reuse(
		const char *url, RequestHeaders *headersList,
		const Common::HashMap<Common::String, Common::String> &formFields,
		const Common::HashMap<Common::String, Common::Path> &formFiles) = 0;
	/** Send <buffer>, using POST by default. */
	virtual bool reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading = false, bool usingPatch = false, bool post = true) = 0;
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
	bool eos() const override { return _eos; }

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
	uint32 read(void *dataPtr, uint32 dataSize) override;

	/**
	 * Returns HTTP response code from inner CURL handle.
	 * It returns -1 to indicate there is no inner handle.
	 *
	 * @note This method should be called when eos() == true.
	 */
	virtual long httpResponseCode() const = 0;

	/**
	 * Return current location URL from inner CURL handle.
	 * "" is returned to indicate there is no inner handle.
	 *
	 * @note This method should be called when eos() == true.
	 */
	virtual Common::String currentLocation() const = 0;

	/**
	 * Return response headers as HashMap. All header names in
	 * it are lowercase.
	 *
	 * @note This method should be called when eos() == true.
	 */
	virtual Common::HashMap<Common::String, Common::String> responseHeadersMap() const = 0;

	/** Returns a number in range [0, 1], where 1 is "complete". */
	double getProgress() const;

	/** Used in curl progress callback to pass current downloaded/total values. */
	void setProgress(uint64 downloaded, uint64 total);

	bool keepAlive() const { return _keepAlive; }

	virtual bool hasError() const = 0;
	virtual const char *getError() const = 0;
};

} // End of namespace Networking

#endif
