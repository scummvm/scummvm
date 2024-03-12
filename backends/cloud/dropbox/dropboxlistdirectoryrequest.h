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

#ifndef BACKENDS_CLOUD_DROPBOX_DROPBOXLISTDIRECTORYREQUEST_H
#define BACKENDS_CLOUD_DROPBOX_DROPBOXLISTDIRECTORYREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Dropbox {

class DropboxStorage;

class DropboxListDirectoryRequest: public Networking::Request {
	Common::String _requestedPath;
	bool _requestedRecursive;

	Storage::ListDirectoryCallback _listDirectoryCallback;
	DropboxStorage *_storage;
	Common::Array<StorageFile> _files;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void responseCallback(const Networking::JsonResponse &response);
	void errorCallback(const Networking::ErrorResponse &error);
	void finishListing(const Common::Array<StorageFile> &files);
public:
	DropboxListDirectoryRequest(DropboxStorage *storage, const Common::String &path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive = false);
	~DropboxListDirectoryRequest() override;

	void handle() override;
	void restart() override;
	Common::String date() const override;
};

} // End of namespace Dropbox
} // End of namespace Cloud

#endif
