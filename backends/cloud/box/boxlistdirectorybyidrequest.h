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

#ifndef BACKENDS_CLOUD_BOX_BOXLISTDIRECTORYBYIDREQUEST_H
#define BACKENDS_CLOUD_BOX_BOXLISTDIRECTORYBYIDREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace Box {

class BoxStorage;

class BoxListDirectoryByIdRequest: public Networking::Request {
	Common::String _requestedId;
	BoxStorage *_storage;

	Storage::ListDirectoryCallback _listDirectoryCallback;
	Common::Array<StorageFile> _files;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _date;

	void start();
	void makeRequest(uint32 offset);
	void responseCallback(Networking::JsonResponse response);
	void errorCallback(Networking::ErrorResponse error);
	void finishListing(Common::Array<StorageFile> &files);
public:
	BoxListDirectoryByIdRequest(BoxStorage *storage, Common::String id, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb);
	virtual ~BoxListDirectoryByIdRequest();

	virtual void handle();
	virtual void restart();
	virtual Common::String date() const;
};

} // End of namespace Box
} // End of namespace Cloud

#endif
