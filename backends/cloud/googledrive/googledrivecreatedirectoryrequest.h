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

#ifndef BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVECREATEDIRECTORYREQUEST_H
#define BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVECREATEDIRECTORYREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace GoogleDrive {

class GoogleDriveStorage;

class GoogleDriveCreateDirectoryRequest: public Networking::Request {
	Common::String _requestedParentPath;
	Common::String _requestedDirectoryName;
	GoogleDriveStorage *_storage;
	Storage::BoolCallback _boolCallback;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void createdBaseDirectoryCallback(Storage::BoolResponse response);
	void createdBaseDirectoryErrorCallback(Networking::ErrorResponse error);
	void resolveId();
	void idResolvedCallback(Storage::UploadResponse response);
	void idResolveFailedCallback(Networking::ErrorResponse error);
	void createdDirectoryCallback(Storage::BoolResponse response);
	void createdDirectoryErrorCallback(Networking::ErrorResponse error);
	void finishSuccess(bool success);
public:
	GoogleDriveCreateDirectoryRequest(GoogleDriveStorage *storage, Common::String parentPath, Common::String directoryName, Storage::BoolCallback cb, Networking::ErrorCallback ecb);
	virtual ~GoogleDriveCreateDirectoryRequest();

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;
};

} // End of namespace GoogleDrive
} // End of namespace Cloud

#endif
