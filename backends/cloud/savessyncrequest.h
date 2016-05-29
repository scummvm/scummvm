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

#ifndef BACKENDS_CLOUD_SAVESSYNCREQUEST_H
#define BACKENDS_CLOUD_SAVESSYNCREQUEST_H

#include "backends/networking/curl/request.h"
#include "backends/cloud/storage.h"
#include "common/hashmap.h"

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

	void start();
	void directoryListedCallback(Storage::FileArrayResponse pair);
	void fileDownloadedCallback(Storage::BoolResponse pair);
	void fileUploadedCallback(Storage::BoolResponse pair);
	void downloadNextFile();
	void uploadNextFile();
	void finishBool(bool success);
	void loadTimestamps();
	void saveTimestamps();	
public:
	SavesSyncRequest(Storage *storage, Storage::BoolCallback callback);
	virtual ~SavesSyncRequest();

	virtual void handle();
	virtual void restart();
	virtual void finish();
};

} // End of namespace Cloud

#endif
