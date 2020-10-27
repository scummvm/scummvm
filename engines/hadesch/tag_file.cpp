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
#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"

#include "hadesch/hadesch.h"
#include "hadesch/tag_file.h"
#include "hadesch/pod_file.h" // for memSubstream

namespace Hadesch {

bool TagFile::openStoreHotSub(const Common::SharedPtr<Common::SeekableReadStream> &parentStream) {
	return openStoreReal(parentStream, 0, parentStream->size(), true, false);
}

bool TagFile::openStoreHot(const Common::SharedPtr<Common::SeekableReadStream> &parentStream) {
	if (parentStream->readUint32BE() != MKTAG('F', 'F', 'I', 'D')) {
		return false;
	}

	if (parentStream->readUint32BE() != MKTAG('S', 'T', 'O', 'H')) {
		return false;
	}

	return openStoreReal(parentStream, 8, parentStream->size() - 8, true, false);
}

bool TagFile::openStoreCel(const Common::SharedPtr<Common::SeekableReadStream> &parentStream) {
	if (parentStream->readUint32BE() != MKTAG('C', 'E', 'L', ' ')) {
		return false;
	}

	uint32 len = parentStream->readUint32BE();

	return openStoreReal(parentStream, 8, len - 8, false, true);
}

bool TagFile::openStoreReal(const Common::SharedPtr<Common::SeekableReadStream> &parentStream, uint32 offset, int32 len, bool isLittleEndian, bool sizeIncludesHeader) {
	int32 sectionSize = 0;

	for (int32 lenRemaining = len; lenRemaining >= 8; lenRemaining -= sectionSize + 8, offset += sectionSize + 8) {
		uint32 type = parentStream->readUint32BE();
		sectionSize = isLittleEndian ? parentStream->readUint32LE() : parentStream->readUint32BE();

		if (sizeIncludesHeader)
			sectionSize -= 8;

		if (sectionSize < 0) {
			debug("invalid section size");
			return false;
		}

		Description desc;
		desc.name = type;
		desc.offset = offset + 8;
		desc.size = sectionSize;

		_descriptions.push_back(desc);
		if (sectionSize != 0)
			parentStream->skip(sectionSize);
	}

	_file = parentStream;

	return true;
}

Common::SeekableReadStream *TagFile::getFileStream(uint32 name, int idx) {
	int curIdx = 0;
	for (uint j = 0; j < _descriptions.size(); ++j) {
		const Description &desc = _descriptions[j];
		if (desc.name == name) {
			if (curIdx == idx)
				return memSubstream(_file, desc.offset, desc.size);
			curIdx++;
		}
	}
	debugC(kHadeschDebugResources, "TagFile: %x not found", name);
	return nullptr;
}

} // End of namespace Hadesch
