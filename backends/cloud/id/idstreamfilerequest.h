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

#ifndef BACKENDS_CLOUD_ID_IDSTREAMFILEREQUEST_H
#define BACKENDS_CLOUD_ID_IDSTREAMFILEREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace Id {

class IdStorage;

class IdStreamFileRequest: public Networking::Request {
	Common::String _requestedFile;
	IdStorage *_storage;
	Networking::NetworkReadStreamCallback _streamCallback;
	Request *_workingRequest;
	bool _ignoreCallback;

	void start();
	void idResolvedCallback(const Storage::UploadResponse &response);
	void idResolveFailedCallback(const Networking::ErrorResponse &error);
	void streamFileCallback(const Networking::NetworkReadStreamResponse &response);
	void streamFileErrorCallback(const Networking::ErrorResponse &error);
	void finishStream(Networking::NetworkReadStream *stream);
public:
	IdStreamFileRequest(IdStorage *storage, const Common::String &path, Networking::NetworkReadStreamCallback cb, Networking::ErrorCallback ecb);
	~IdStreamFileRequest() override;

	void handle() override;
	void restart() override;
};

} // End of namespace Id
} // End of namespace Cloud

#endif
