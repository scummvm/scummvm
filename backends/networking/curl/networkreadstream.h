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

typedef void CURL;
struct curl_slist;

namespace Networking {

class NetworkReadStream: public Common::MemoryReadWriteStream {	
	CURL *_easy;
	bool _eos, _requestComplete;

public:	
	NetworkReadStream(const char *url, curl_slist *headersList, Common::String postFields);
	NetworkReadStream(const char *url, curl_slist *headersList, byte *buffer, uint32 bufferSize, bool post = true);
	virtual ~NetworkReadStream();

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
	 * @param dataPtr	pointer to a buffer into which the data is read
	 * @param dataSize	number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize);

	/**
	 * This method is called by ConnectionManager to indicate
	 * that transfer is finished.
	 *
	 * @note It's called on failure too.
	 */
	void finished();

	/**
	 * Returns HTTP response code from inner CURL handle.
	 * It returns -1 to indicate there is no inner handle.
	 *
	 * @note This method should be called when eos() == true.
	 */
	long httpResponseCode() const;
};

} // End of namespace Networking

#endif
