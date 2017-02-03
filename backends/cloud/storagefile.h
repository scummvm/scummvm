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

#ifndef BACKENDS_CLOUD_STORAGEFILE_H
#define BACKENDS_CLOUD_STORAGEFILE_H

#include "common/str.h"

namespace Cloud {

/**
 * StorageFile represents a file storaged on remote cloud storage.
 * It contains basic information about a file, and might be used
 * when listing directories or syncing files.
 *
 * Some storages (Google Drive, for example) don't have an actual
 * path notation to address files. Instead, they are using ids.
 * As resolving id by path is not a fast operation, it's required
 * to use ids if they are known, but user-friendly paths are
 * necessary too, because these are used by Requests.
 *
 * If storage supports path notation, id would actually contain path.
 */
class StorageFile {
	Common::String _id, _path, _name;
	uint32 _size, _timestamp;
	bool _isDirectory;

public:
	StorageFile(); //invalid empty file
	StorageFile(Common::String pth, uint32 sz, uint32 ts, bool dir);
	StorageFile(Common::String fileId, Common::String filePath, Common::String fileName, uint32 sz, uint32 ts, bool dir);

	Common::String id() const { return _id; }
	Common::String path() const { return _path; }
	Common::String name() const { return _name; }
	uint32 size() const { return _size; }
	uint32 timestamp() const { return _timestamp; }
	bool isDirectory() const { return _isDirectory; }

	void setPath(Common::String path_) { _path = path_; }
};

} // End of namespace Cloud

#endif
