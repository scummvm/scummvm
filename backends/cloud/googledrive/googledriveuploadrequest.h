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

#ifndef BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVEUPLOADREQUEST_H
#define BACKENDS_CLOUD_GOOGLEDRIVE_GOOGLEDRIVEUPLOADREQUEST_H

#include "backends/cloud/storage.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/request.h"
#include "common/callback.h"

namespace Cloud {
namespace GoogleDrive {
class GoogleDriveStorage;

class GoogleDriveUploadRequest: public Networking::Request {
	GoogleDriveStorage *_storage;
	Common::String _savePath;
	Common::SeekableReadStream *_contentsStream;
	Storage::UploadCallback _uploadCallback;
	Request *_workingRequest;
	bool _ignoreCallback;
	Common::String _resolvedId, _parentId;
	Common::String _uploadUrl;
	uint64 _serverReceivedBytes;

	void start();
	void resolveId();
	void idResolvedCallback(Storage::UploadResponse response);
	void idResolveFailedCallback(Networking::ErrorResponse error);
	void startUpload();
	void startUploadCallback(Networking::JsonResponse response);
	void startUploadErrorCallback(Networking::ErrorResponse error);
	void uploadNextPart();
	void partUploadedCallback(Networking::JsonResponse response);
	void partUploadedErrorCallback(Networking::ErrorResponse error);
	bool handleHttp308(const Networking::NetworkReadStream *stream);
	void finishUpload(StorageFile status);

public:
	GoogleDriveUploadRequest(GoogleDriveStorage *storage, Common::String path, Common::SeekableReadStream *contents, Storage::UploadCallback callback, Networking::ErrorCallback ecb);
	virtual ~GoogleDriveUploadRequest();

	virtual void handle();
	virtual void restart();
};

} // End of namespace GoogleDrive
} // End of namespace Cloud

#endif
