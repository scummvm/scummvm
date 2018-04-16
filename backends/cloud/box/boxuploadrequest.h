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

#ifndef BACKENDS_CLOUD_BOX_BOXUPLOADREQUEST_H
#define BACKENDS_CLOUD_BOX_BOXUPLOADREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace Box {
class BoxStorage;

class BoxUploadRequest: public Networking::Request {
	BoxStorage *_storage;
	Common::String _savePath, _localPath;
	Storage::UploadCallback _uploadCallback;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _resolvedId, _parentId;

	void start();
	void resolveId();
	void idResolvedCallback(Storage::UploadResponse response);
	void idResolveFailedCallback(Networking::ErrorResponse error);
	void upload();
	void uploadedCallback(Networking::JsonResponse response);
	void notUploadedCallback(Networking::ErrorResponse error);
	void finishUpload(StorageFile status);

public:
	BoxUploadRequest(BoxStorage *storage, Common::String path, Common::String localPath, Storage::UploadCallback callback, Networking::ErrorCallback ecb);
	virtual ~BoxUploadRequest();

	virtual void handle();
	virtual void restart();
};

} // End of namespace Box
} // End of namespace Cloud

#endif
