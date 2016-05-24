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

#include "backends/cloud/downloadrequest.h"
#include "common/debug.h"
#include "common/textconsole.h"

namespace Cloud {

DownloadRequest::DownloadRequest(Storage::BoolCallback callback, Networking::NetworkReadStream *stream):
	Request(0), _boolCallback(callback), _stream(stream) {}

bool DownloadRequest::handle() {
	if (!_stream) {
		warning("DownloadRequest: no stream to read");
		return true;
	}

	const int kBufSize = 16 * 1024;
	char buf[kBufSize];
	uint32 readBytes = _stream->read(buf, kBufSize);

	//TODO: save into file
	/*
	if (readBytes != 0)
		if (_outputStream.write(buf, readBytes) != readBytes)
			warning("DropboxDownloadRequest: unable to write all received bytes into output stream");
	*/

	buf[readBytes] = 0;
	debug("%s", buf); //TODO: remove

	if (_stream->eos()) {
		if (_stream->httpResponseCode() != 200) {
			warning("HTTP response code is not 200 OK (it's %ld)", _stream->httpResponseCode());
			//TODO: do something about it actually			
		}

		if (_boolCallback) (*_boolCallback)(_stream->httpResponseCode() == 200);

		//TODO: close file stream
		return true;
	}

	return false;
}

} //end of namespace Cloud
