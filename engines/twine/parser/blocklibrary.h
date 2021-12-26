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

#ifndef TWINE_PARSER_BLOCKLIBRARY_H
#define TWINE_PARSER_BLOCKLIBRARY_H

#include "common/array.h"
#include "common/stream.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct BlockDataEntry {
	uint8 brickShape;
	uint8 brickType;
	/**
	 * Index is not starting at 0 - but at 1. A 0 indicates an empty brick
	 */
	uint16 brickIdx;
	uint8 sound;
};

struct BlockData {
	Common::Array<BlockDataEntry> entries;
};

class BlockLibraryData : public Parser {
private:
	Common::Array<BlockData> _layouts;
	bool parseLayout(BlockData &blockData, Common::SeekableReadStream &stream, bool lba1);
protected:
	void reset() override;
public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;
	const BlockData *getLayout(int index) const;
};

} // End of namespace TwinE

#endif
