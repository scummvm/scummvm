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

#ifndef BACKENDS_CLOUD_STORAGE_H
#define BACKENDS_CLOUD_STORAGE_H

#include "common/array.h"
#include "common/stream.h"
#include "common/str.h"

namespace Cloud {

class StorageFile {
	Common::String _path, _name;
	uint32 _size, _timestamp;
	bool _isDirectory;

public:
	StorageFile(Common::String pth, uint32 sz, uint32 ts, bool dir) {
		_path = pth;

		_name = pth;
		for (uint32 i = _name.size() - 1; i >= 0; --i) {
			if (_name[i] == '/' || _name[i] == '\\') {
				_name.erase(0, i);
				break;
			}
			if (i == 0) break; //OK, I admit that's strange
		}

		_size = sz;
		_timestamp = ts;
		_isDirectory = dir;
	}

	Common::String path() const { return _path; }
	Common::String name() const { return _name; }
	uint32 size() const { return _size; }
	uint32 timestamp() const { return _timestamp; }
	bool isDirectory() const { return _isDirectory; }
};

class StorageInfo {
	Common::String _info;

public:
	StorageInfo(Common::String info): _info(info) {}

	Common::String info() const { return _info; }
};

class Storage {
public:
	typedef void(*ListDirectoryCallback)(Common::Array<StorageFile>& result);
	typedef void(*DownloadCallback)(Common::ReadStream* result);
	typedef void(*InfoCallback)(StorageInfo result);
	typedef void(*OperationCallback)(bool successed);

	Storage() {}
	virtual ~Storage() {}

	/** Returns pointer to Common::Array<CloudFile>. */
	virtual void listDirectory(Common::String path, ListDirectoryCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual void upload(Common::String path, Common::ReadStream* contents, OperationCallback callback) = 0;

	/** Returns pointer to Common::ReadStream. */
	virtual void download(Common::String path, DownloadCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual void remove(Common::String path, OperationCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual void syncSaves(OperationCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual void createDirectory(Common::String path, OperationCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual void touch(Common::String path, OperationCallback callback) = 0;

	/** Returns pointer to the ServiceInfo struct. */
	virtual void info(InfoCallback callback) = 0;

	/** Returns whether saves sync process is running. */
	virtual bool isSyncing() = 0;

	/** Returns whether there are any requests running. */
	virtual bool isWorking() = 0;
};

} //end of namespace Cloud

#endif
