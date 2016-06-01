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

#include "backends/cloud/storage.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/cloud/folderdownloadrequest.h"
#include "backends/cloud/savessyncrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "common/debug.h"
#include "common/file.h"

namespace Cloud {

Storage::Storage(): _runningRequestsCount(0) {}

Storage::~Storage() {}

Networking::ErrorCallback Storage::getErrorPrintingCallback() {
	return new Common::Callback<Storage, Networking::ErrorResponse>(this, &Storage::printErrorResponse);
}

void Storage::printErrorResponse(Networking::ErrorResponse error) {
	debug("error response (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug("%s", error.response.c_str());
}

Networking::Request *Storage::addRequest(Networking::Request *request) {
	++_runningRequestsCount;
	if (_runningRequestsCount == 1) debug("Storage is working now");
	return ConnMan.addRequest(request, new Common::Callback<Storage, Networking::Request *>(this, &Storage::requestFinishedCallback));
}

void Storage::requestFinishedCallback(Networking::Request *invalidRequestPointer) {
	--_runningRequestsCount;
	if (_runningRequestsCount == 0) debug("Storage is not working now");
}

Networking::Request *Storage::upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::File *f = new Common::File();
	if (!f->open(localPath)) {
		warning("Storage: unable to open file to upload from");
		if (errorCallback) (*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "", -1));
		delete errorCallback;
		delete callback;
		delete f;
		return nullptr;
	}

	return upload(remotePath, f, callback, errorCallback);
}

Networking::Request *Storage::download(Common::String remotePath, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(localPath, true)) {
		warning("Storage: unable to open file to download into");
		if (errorCallback) (*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "", -1));
		delete errorCallback;
		delete callback;
		delete f;
		return nullptr;
	}

	return addRequest(new DownloadRequest(this, callback, errorCallback, remotePath, f));
}

Networking::Request *Storage::downloadFolder(Common::String remotePath, Common::String localPath, FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	return addRequest(new FolderDownloadRequest(this, callback, errorCallback, remotePath, localPath, recursive));
}

Networking::Request *Storage::syncSaves(BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	return addRequest(new SavesSyncRequest(this, callback, errorCallback));
}

bool Storage::isWorking() {
	return _runningRequestsCount > 0;
}

} // End of namespace Cloud

