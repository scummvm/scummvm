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

#ifndef BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVELISTDIRECTORYBYIDREQUEST_H
#define BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVELISTDIRECTORYBYIDREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace GoogleDrive {

class GoogleDriveStorage;

class GoogleDriveListDirectoryByIdRequest: public Networking::Request {
	Common::String _requestedId;
	GoogleDriveStorage *_storage;

	Storage::ListDirectoryCallback _listDirectoryCallback;
	Common::Array<StorageFile> _files;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void makeRequest(Common::String pageToken);
	void responseCallback(Networking::JsonResponse response);
	void errorCallback(Networking::ErrorResponse error);
	void finishListing(Common::Array<StorageFile> &files);
public:
	GoogleDriveListDirectoryByIdRequest(GoogleDriveStorage *storage, Common::String id, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb);
	virtual ~GoogleDriveListDirectoryByIdRequest();

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;
};

} // End of namespace GoogleDrive
} // End of namespace Cloud

#endif
