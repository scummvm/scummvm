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

#ifndef BACKENDS_CLOUD_ID_IDLISTDIRECTORYREQUEST_H
#define BACKENDS_CLOUD_ID_IDLISTDIRECTORYREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace Id {

class IdStorage;

class IdListDirectoryRequest: public Networking::Request {
	Common::String _requestedPath;
	bool _requestedRecursive;
	IdStorage *_storage;
	Storage::ListDirectoryCallback _listDirectoryCallback;
	Common::Array<StorageFile> _files;
	Common::Array<StorageFile> _directoriesQueue;
	StorageFile _currentDirectory;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void idResolvedCallback(const Storage::UploadResponse &response);
	void idResolveErrorCallback(const Networking::ErrorResponse &error);
	void listNextDirectory();
	void listedDirectoryCallback(const Storage::FileArrayResponse &response);
	void listedDirectoryErrorCallback(const Networking::ErrorResponse &error);
	void finishListing(const Common::Array<StorageFile> &files);
public:
	IdListDirectoryRequest(IdStorage *storage, const Common::String &path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive = false);
	~IdListDirectoryRequest() override;

	void handle() override;
	void restart() override;
	Common::String date() const override;
};

} // End of namespace Id
} // End of namespace Cloud

#endif
