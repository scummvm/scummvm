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
#include "backends/networking/curl/networkreadstream.h"
#include "backends/cloud/storage.h"
#include "common/file.h"

namespace Cloud {

class FolderDownloadRequest: public Networking::Request {
	Storage *_storage;
	Storage::FileArrayCallback _fileArrayCallback;
	Common::String _remoteDirectoryPath, _localDirectoryPath;
	bool _recursive;
	Common::Array<StorageFile> _files, _failedFiles;
	StorageFile _currentFile;
	Request *_workingRequest;
	bool _ignoreCallback;

	void start();
	void directoryListedCallback(Storage::FileArrayResponse pair);
	void fileDownloadedCallback(Storage::BoolResponse pair);
	void downloadNextFile();
	void finishFiles(Common::Array<StorageFile> &files);
public:
	FolderDownloadRequest(Storage *storage, Storage::FileArrayCallback callback, Common::String remoteDirectoryPath, Common::String localDirectoryPath, bool recursive);
	virtual ~FolderDownloadRequest() {}

	virtual void handle() {}
	virtual void restart() { start(); }
	virtual void finish();
};

} //end of namespace Cloud

#endif
