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

#ifndef BACKENDS_CLOUD_SAVESSYNCREQUEST_H
#define BACKENDS_CLOUD_SAVESSYNCREQUEST_H

#include "backends/networking/curl/request.h"
#include "backends/cloud/storage.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Cloud {

class SavesSyncRequest: public Networking::Request {
	Storage *_storage;
	Storage::BoolCallback _boolCallback;
	Common::HashMap<Common::String, uint32> _localFilesTimestamps;
	Common::Array<StorageFile> _filesToDownload;
	Common::Array<Common::String> _filesToUpload;
	StorageFile _currentDownloadingFile;
	Common::String _currentUploadingFile;
	Request *_workingRequest;
	bool _ignoreCallback;
	uint32 _totalFilesToHandle;
	Common::String _date;
	uint32 _bytesToDownload, _bytesDownloaded;

	void start();
	void directoryListedCallback(Storage::ListDirectoryResponse response);
	void directoryListedErrorCallback(Networking::ErrorResponse error);
	void directoryCreatedCallback(Storage::BoolResponse response);
	void directoryCreatedErrorCallback(Networking::ErrorResponse error);
	void fileDownloadedCallback(Storage::BoolResponse response);
	void fileDownloadedErrorCallback(Networking::ErrorResponse error);
	void fileUploadedCallback(Storage::UploadResponse response);
	void fileUploadedErrorCallback(Networking::ErrorResponse error);
	void downloadNextFile();
	void uploadNextFile();
	virtual void finishError(Networking::ErrorResponse error, Networking::RequestState state = Networking::FINISHED);
	void finishSync(bool success);

	uint32 getDownloadedBytes() const;
	uint32 getBytesToDownload() const;

public:
	SavesSyncRequest(Storage *storage, Storage::BoolCallback callback, Networking::ErrorCallback ecb);
	virtual ~SavesSyncRequest();

	virtual void handle();
	virtual void restart();

	/** Returns a number in range [0, 1], where 1 is "complete". */
	double getDownloadingProgress() const;

	/** Fills a struct with numbers about current sync downloading progress. */
	void getDownloadingInfo(Storage::SyncDownloadingInfo &info) const;

	/** Returns a number in range [0, 1], where 1 is "complete". */
	double getProgress() const;

	/** Returns an array of saves names which are not downloaded yet. */
	Common::Array<Common::String> getFilesToDownload();
};

} // End of namespace Cloud

#endif
