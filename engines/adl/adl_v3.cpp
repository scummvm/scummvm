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

#include "adl/adl_v3.h"

namespace Adl {

AdlEngine_v3::AdlEngine_v3(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine_v2(syst, gd) {
}

Common::String AdlEngine_v3::getItemDescription(const Item &item) const {
	return _itemDesc[item.description - 1];
}

void AdlEngine_v3::loadItemDescriptions(Common::SeekableReadStream &stream, byte count) {
	int32 startPos = stream.pos();
	uint16 baseAddr = stream.readUint16LE();

	// This code assumes that the first pointer points to a string that
	// directly follows the pointer table
	assert(baseAddr != 0);
	baseAddr -= count * 2;

	for (uint i = 0; i < count; ++i) {
		stream.seek(startPos + i * 2);
		uint16 offset = stream.readUint16LE();

		if (offset > 0) {
			stream.seek(startPos + offset - baseAddr);
			_itemDesc.push_back(readString(stream, 0xff));
		} else
			_itemDesc.push_back(Common::String());
	}

	if (stream.eos() || stream.err())
		error("Error loading item descriptions");
}

int AdlEngine_v3::o_isNounNotInRoom(ScriptEnv &e) {
	OP_DEBUG_1("\t&& NO_SUCH_ITEMS_IN_ROOM(%s)", itemRoomStr(e.arg(1)).c_str());

	Common::List<Item>::const_iterator item;

	bool isAnItem = false;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun == e.getNoun()) {
			isAnItem = true;

			if (item->room == roomArg(e.arg(1)))
				return -1;
		}
	}

	return (isAnItem ? 1 : -1);
}

} // End of namespace Adl
