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

#ifndef HADESCH_TAG_FILE_H
#define HADESCH_TAG_FILE_H

#include "common/array.h"

namespace Common {
	class File;
	class SeekableReadStream;
}

namespace Hadesch {

class TagFile {
public:

	bool openStoreCel(const Common::SharedPtr<Common::SeekableReadStream> &parentstream);
	bool openStoreHot(const Common::SharedPtr<Common::SeekableReadStream> &parentstream);
	bool openStoreHotSub(const Common::SharedPtr<Common::SeekableReadStream> &parentstream);

	Common::SeekableReadStream *getFileStream(uint32 name, int idx = 0);

private:
	bool openStoreReal(const Common::SharedPtr<Common::SeekableReadStream> &parentstream, uint32 offset,
			   int32 len, bool isLittleEndian, bool sizeIncludesHeader);

	struct Description {
		uint32 name;
		uint32 offset;
		uint32 size;
	};
	Common::SharedPtr<Common::SeekableReadStream> _file;
	Common::Array<Description> _descriptions;
};

} // End of namespace Hadesch

#endif
