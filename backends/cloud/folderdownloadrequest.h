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

#ifndef BACKENDS_CLOUD_FOLDERDOWNLOADREQUEST_H
#define BACKENDS_CLOUD_FOLDERDOWNLOADREQUEST_H

#include "backends/networking/curl/request.h"
#include "backends/cloud/storage.h"
#include "gui/object.h"

namespace Cloud {

class FolderDownloadRequest: public Networking::Request, public GUI::CommandSender {
	Storage *_storage;
	Storage::FileArrayCallback _fileArrayCallback;
	Common::String _remoteDirectoryPath, _localDirectoryPath;
	bool _recursive;
	Common::Array<StorageFile> _pendingFiles, _failedFiles;
	StorageFile _currentFile;
	Request *_workingRequest;
	bool _ignoreCallback;
	uint32 _totalFiles;
	uint64 _downloadedBytes, _totalBytes, _wasDownloadedBytes, _currentDownloadSpeed;

	void start();
	void directoryListedCallback(Storage::ListDirectoryResponse response);
	void directoryListedErrorCallback(Networking::ErrorResponse error);
	void fileDownloadedCallback(Storage::BoolResponse response);
	void fileDownloadedErrorCallback(Networking::ErrorResponse error);
	void downloadNextFile();
	void finishDownload(Common::Array<StorageFile> &files);
public:
	FolderDownloadRequest(Storage *storage, Storage::FileArrayCallback callback, Networking::ErrorCallback ecb, Common::String remoteDirectoryPath, Common::String localDirectoryPath, bool recursive);
	virtual ~FolderDownloadRequest();

	virtual void handle();
	virtual void restart();

	/** Returns a number in range [0, 1], where 1 is "complete". */
	double getProgress() const;

	/** Returns a number of downloaded bytes. */
	uint64 getDownloadedBytes() const;

	/** Returns a total number of bytes to download. */
	uint64 getTotalBytesToDownload() const;

	/** Returns average download speed for the last second. */
	uint64 getDownloadSpeed() const;

	/** Returns remote directory path. */
	Common::String getRemotePath() const { return _remoteDirectoryPath; }

	/** Returns local directory path. */
	Common::String getLocalPath() const { return _localDirectoryPath; }
};

} // End of namespace Cloud

#endif
