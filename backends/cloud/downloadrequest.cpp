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
#include "backends/networking/curl/connectionmanager.h"
#include "common/debug.h"
#include "common/textconsole.h"

namespace Cloud {

DownloadRequest::DownloadRequest(Storage *storage, Storage::BoolCallback callback, Networking::ErrorCallback ecb, Common::String remoteFileId, Common::DumpFile *dumpFile):
	Request(nullptr, ecb), _boolCallback(callback), _localFile(dumpFile), _remoteFileId(remoteFileId), _storage(storage),
	_remoteFileStream(nullptr), _workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DownloadRequest::~DownloadRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _boolCallback;
	delete _localFile;
}

void DownloadRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_remoteFileStream = nullptr;
	//TODO: reopen DumpFile
	_ignoreCallback = false;

	_workingRequest = _storage->streamFileById(
		_remoteFileId,
		new Common::Callback<DownloadRequest, Networking::NetworkReadStreamResponse>(this, &DownloadRequest::streamCallback),
		new Common::Callback<DownloadRequest, Networking::ErrorResponse>(this, &DownloadRequest::streamErrorCallback)
	);
}

void DownloadRequest::streamCallback(Networking::NetworkReadStreamResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	_remoteFileStream = (Networking::NetworkReadStream *)response.value;
}

void DownloadRequest::streamErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void DownloadRequest::handle() {	
	if (!_localFile) {
		warning("DownloadRequest: no file to write");
		finishError(Networking::ErrorResponse(this, false, true, "", -1));
		return;
	}

	if (!_localFile->isOpen()) {
		warning("DownloadRequest: failed to open file to write");
		finishError(Networking::ErrorResponse(this, false, true, "", -1));
		return;
	}

	if (!_remoteFileStream) {
		//waiting for callback
		return;
	}

	const int kBufSize = 640 * 1024; //640 KB is enough to everyone?..
	char buf[kBufSize];
	uint32 readBytes = _remoteFileStream->read(buf, kBufSize);

	if (readBytes != 0)
		if (_localFile->write(buf, readBytes) != readBytes) {
			warning("DownloadRequest: unable to write all received bytes into output file");
			finishError(Networking::ErrorResponse(this, false, true, "", -1));
			return;
		}

	if (_remoteFileStream->eos()) {
		if (_remoteFileStream->httpResponseCode() != 200) {
			warning("HTTP response code is not 200 OK (it's %ld)", _remoteFileStream->httpResponseCode());
			//TODO: do something about it actually			
		}

		finishSuccess(_remoteFileStream->httpResponseCode() == 200);

		_localFile->close(); //yes, I know it's closed automatically in ~DumpFile()		
	}
}

void DownloadRequest::restart() {
	warning("DownloadRequest: can't restart as there are no means to reopen DumpFile");
	finishError(Networking::ErrorResponse(this, false, true, "", -1));
	//start();
}

void DownloadRequest::finishSuccess(bool success) {
	Request::finishSuccess();
	if (_boolCallback) (*_boolCallback)(Storage::BoolResponse(this, success));
}

void DownloadRequest::finishError(Networking::ErrorResponse error) {
	if (_localFile) _localFile->close();
	Request::finishError(error);
}

} // End of namespace Cloud
