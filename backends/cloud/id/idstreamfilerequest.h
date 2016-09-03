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
	void idResolvedCallback(Storage::UploadResponse response);
	void idResolveFailedCallback(Networking::ErrorResponse error);
	void streamFileCallback(Networking::NetworkReadStreamResponse response);
	void streamFileErrorCallback(Networking::ErrorResponse error);
	void finishStream(Networking::NetworkReadStream *stream);
public:
	IdStreamFileRequest(IdStorage *storage, Common::String path, Networking::NetworkReadStreamCallback cb, Networking::ErrorCallback ecb);
	virtual ~IdStreamFileRequest();

	virtual void handle();
	virtual void restart();
};

} // End of namespace Id
} // End of namespace Cloud

#endif
