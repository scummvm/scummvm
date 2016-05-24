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

DownloadRequest::DownloadRequest(Storage::BoolCallback callback, Networking::NetworkReadStream *stream, Common::DumpFile *dumpFile):
	Request(0), _boolCallback(callback), _remoteFileStream(stream), _localFile(dumpFile) {}

bool DownloadRequest::handle() {
	if (!_remoteFileStream) {
		warning("DownloadRequest: no stream to read");
		return true;
	}

	if (!_localFile) {
		warning("DownloadRequest: no file to write");
		return true;
	}

	if (!_localFile->isOpen()) {
		warning("DownloadRequest: failed to open file to write");
		return true;
	}

	const int kBufSize = 16 * 1024;
	char buf[kBufSize];
	uint32 readBytes = _remoteFileStream->read(buf, kBufSize);

	if (readBytes != 0)
		if (_localFile->write(buf, readBytes) != readBytes) {
			warning("DownloadRequest: unable to write all received bytes into output file");
			if (_boolCallback) (*_boolCallback)(false);
			return true;
		}

	if (_remoteFileStream->eos()) {
		if (_remoteFileStream->httpResponseCode() != 200) {
			warning("HTTP response code is not 200 OK (it's %ld)", _remoteFileStream->httpResponseCode());
			//TODO: do something about it actually			
		}

		if (_boolCallback) (*_boolCallback)(_remoteFileStream->httpResponseCode() == 200);

		_localFile->close(); //yes, I know it's closed automatically in ~DumpFile()
		return true;
	}

	return false;
}

} //end of namespace Cloud
