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

#ifndef BACKENDS_CLOUD_STORAGEINFO_H
#define BACKENDS_CLOUD_STORAGEINFO_H

#include "common/str.h"

namespace Cloud {

/**
* StorageInfo contains information about remote cloud storage.
* It's disk quota usage, owner name, and such.
*/

class StorageInfo {
	Common::String _uid, _name, _email;
	uint64 _usedBytes, _allocatedBytes;

public:
	StorageInfo(Common::String uid_, Common::String name_, Common::String email_, uint64 used_, uint64 allocated):
		_uid(uid_), _name(name_), _email(email_), _usedBytes(used_), _allocatedBytes(allocated) {}

	Common::String uid() const { return _uid; }
	Common::String name() const { return _name; }
	Common::String email() const { return _email; }
	uint64 used() const { return _usedBytes; }
	uint64 available() const { return _allocatedBytes; }

};

} // End of namespace Cloud

#endif
