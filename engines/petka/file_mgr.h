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

#ifndef PETKA_FILE_MGR_H
#define PETKA_FILE_MGR_H

#include "common/list.h"
#include "common/ptr.h"

namespace Common {
	class File;
	class SeekableReadStream;
}

namespace Petka {

class FileMgr {
public:
	bool openStore(const Common::String &name);
	void closeStore(const Common::String &name);
	void closeAll();

	Common::SeekableReadStream *getFileStream(const Common::String &name);

private:
	struct Resource {
		Common::String name;
		uint32 offset;
		uint32 size;
	};
	struct Store {
		Common::SharedPtr<Common::File> file;
		Common::Array<Resource> descriptions;
	};
	Common::Array<Store> _stores;
};

} // End of namespace Petka

#endif
