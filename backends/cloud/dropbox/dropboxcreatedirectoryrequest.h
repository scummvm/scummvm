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

#ifndef BACKENDS_CLOUD_DROPBOX_DROPBOXCREATEDIRECTORYREQUEST_H
#define BACKENDS_CLOUD_DROPBOX_DROPBOXCREATEDIRECTORYREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Dropbox {

class DropboxStorage;

class DropboxCreateDirectoryRequest: public Networking::Request {
	DropboxStorage *_storage;
	Common::String _path;
	Storage::BoolCallback _boolCallback;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void responseCallback(Networking::JsonResponse response);
	void errorCallback(Networking::ErrorResponse error);
	void finishCreation(bool success);
public:
	DropboxCreateDirectoryRequest(DropboxStorage *storage, Common::String path, Storage::BoolCallback cb, Networking::ErrorCallback ecb);
	virtual ~DropboxCreateDirectoryRequest();

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;
};

} // End of namespace Dropbox
} // End of namespace Cloud

#endif
