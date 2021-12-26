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

#ifndef BACKENDS_CLOUD_DROPBOX_DROPBOXINFOREQUEST_H
#define BACKENDS_CLOUD_DROPBOX_DROPBOXINFOREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Dropbox {

class DropboxStorage;

class DropboxInfoRequest: public Networking::Request {
	DropboxStorage *_storage;
	Common::String _uid, _name, _email;
	Storage::StorageInfoCallback _infoCallback;
	Request *_workingRequest;
	bool _ignoreCallback;

	void start();
	void userResponseCallback(Networking::JsonResponse response);
	void quotaResponseCallback(Networking::JsonResponse response);
	void errorCallback(Networking::ErrorResponse error);
	void finishInfo(StorageInfo info);
public:
	DropboxInfoRequest(DropboxStorage *storage, Storage::StorageInfoCallback cb, Networking::ErrorCallback ecb);
	virtual ~DropboxInfoRequest();

	virtual void handle();
	virtual void restart();
};

} // End of namespace Dropbox
} // End of namespace Cloud

#endif
