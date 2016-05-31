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
#include "common/file.h"

namespace Cloud {

Networking::Request *Storage::upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback) {
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

} // End of namespace Cloud

