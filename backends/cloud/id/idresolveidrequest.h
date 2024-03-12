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

#ifndef BACKENDS_CLOUD_ID_IDRESOLVEIDREQUEST_H
#define BACKENDS_CLOUD_ID_IDRESOLVEIDREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace Id {

class IdStorage;

class IdResolveIdRequest: public Networking::Request {
	Common::String _requestedPath;
	IdStorage *_storage;
	Storage::UploadCallback _uploadCallback;
	Common::String _currentDirectory;
	Common::String _currentDirectoryId;
	Request *_workingRequest;
	bool _ignoreCallback;

	void start();
	void listNextDirectory(const StorageFile &fileToReturn);
	void listedDirectoryCallback(const Storage::FileArrayResponse &response);
	void listedDirectoryErrorCallback(const Networking::ErrorResponse &error);
	void finishFile(const StorageFile &file);
public:
	IdResolveIdRequest(IdStorage *storage, const Common::String &path, Storage::UploadCallback cb, Networking::ErrorCallback ecb, bool recursive = false); //TODO: why upload?
	~IdResolveIdRequest() override;

	void handle() override;
	void restart() override;
};

} // End of namespace Id
} // End of namespace Cloud

#endif
