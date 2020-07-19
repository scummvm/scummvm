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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_FILE_MGR_H
#define HADESCH_FILE_MGR_H

#include "common/array.h"
#include "common/ptr.h"

namespace Common {
	class File;
	class SeekableReadStream;
}

namespace Hadesch {

class PodImage;

Common::SeekableReadStream *memSubstream(Common::SharedPtr<Common::SeekableReadStream> file,
					 uint32 offset, uint32 size);
class PodFile {
public:
	PodFile(const Common::String &debugName);
	bool openStore(const Common::String &name);
	bool openStore(const Common::SharedPtr<Common::SeekableReadStream> &parentstream);

	Common::SeekableReadStream *getFileStream(const Common::String &name) const;
	Common::String getDebugName() const;
	Common::Array <PodImage> loadImageArray() const;

private:
	struct Description {
		Common::String name;
		uint32 offset;
		uint32 size;
	};
	Common::SharedPtr<Common::SeekableReadStream> _file;
	Common::Array<Description> _descriptions;
	Common::String _debugName;
};

} // End of namespace Hadesch

#endif
