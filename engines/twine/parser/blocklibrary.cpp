/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "twine/parser/blocklibrary.h"
#include "common/stream.h"

namespace TwinE {

void BlockLibraryData::reset() {
	_layouts.clear();
}

bool BlockLibraryData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	reset();
	const uint32 numLayouts = stream.readUint32LE() / 4;
	_layouts.resize(numLayouts);
	stream.seek(0);
	for (uint32 i = 0; i < numLayouts; ++i) {
		BlockData &blockData = _layouts[i];
		const uint32 offset = stream.readUint32LE();
		const uint32 nextOffset = stream.pos();
		if (!stream.seek(offset)) {
			return false;
		}
		if (!parseLayout(blockData, stream, lba1)) {
			return false;
		}
		stream.seek(nextOffset);
	}
	return !stream.err();
}

bool BlockLibraryData::parseLayout(BlockData &blockData, Common::SeekableReadStream &stream, bool lba1) {
	const uint8 x = stream.readByte();
	const uint8 y = stream.readByte();
	const uint8 z = stream.readByte();
	const int32 numBricks = x * y * z;
	blockData.entries.resize(numBricks);
	for (int32 i = 0; i < numBricks; ++i) {
		BlockDataEntry &blockEntry = blockData.entries[i];
		blockEntry.brickShape = stream.readByte();
		blockEntry.brickType = stream.readByte();
		blockEntry.brickIdx = stream.readUint16LE();
		blockEntry.sound = bits(blockEntry.brickType, 0, 4);
	}
	return !stream.err();
}

const BlockData *BlockLibraryData::getLayout(int index) const {
	if (index < 0 || index >= (int)_layouts.size()) {
		error("Block library index out of range: %i", index);
	}
	return &_layouts[index];
}

} // End of namespace TwinE
