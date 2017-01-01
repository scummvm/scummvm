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

#include "backends/cloud/id/idstreamfilerequest.h"
#include "backends/cloud/id/idstorage.h"

namespace Cloud {
namespace Id {

IdStreamFileRequest::IdStreamFileRequest(IdStorage *storage, Common::String path, Networking::NetworkReadStreamCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _requestedFile(path), _storage(storage), _streamCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

IdStreamFileRequest::~IdStreamFileRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _streamCallback;
}

void IdStreamFileRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_ignoreCallback = false;

	//find file's id
	Storage::UploadCallback innerCallback = new Common::Callback<IdStreamFileRequest, Storage::UploadResponse>(this, &IdStreamFileRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdStreamFileRequest, Networking::ErrorResponse>(this, &IdStreamFileRequest::idResolveFailedCallback);
	_workingRequest = _storage->resolveFileId(_requestedFile, innerCallback, innerErrorCallback);
}

void IdStreamFileRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Networking::NetworkReadStreamCallback innerCallback = new Common::Callback<IdStreamFileRequest, Networking::NetworkReadStreamResponse>(this, &IdStreamFileRequest::streamFileCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdStreamFileRequest, Networking::ErrorResponse>(this, &IdStreamFileRequest::streamFileErrorCallback);
	_workingRequest = _storage->streamFileById(response.value.id(), innerCallback, innerErrorCallback);
}

void IdStreamFileRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void IdStreamFileRequest::streamFileCallback(Networking::NetworkReadStreamResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishStream(response.value);
}

void IdStreamFileRequest::streamFileErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void IdStreamFileRequest::handle() {}

void IdStreamFileRequest::restart() { start(); }

void IdStreamFileRequest::finishStream(Networking::NetworkReadStream *stream) {
	Request::finishSuccess();
	if (_streamCallback)
		(*_streamCallback)(Networking::NetworkReadStreamResponse(this, stream));
}

} // End of namespace Id
} // End of namespace Cloud
