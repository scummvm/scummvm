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

#include "common/textconsole.h"

#include "dune/detection.h"
#include "dune/dialogue.h"

namespace Dune {

bool Dialogue::load(Archive &archive) {
	Resource file = archive.openMember("DIALOGUE.HSQ");
	if (!file) {
		warning("Dune: DIALOGUE.HSQ is missing");
		return false;
	}

	const uint32 size = file->size();
	Common::Array<byte> data;
	data.resize(size);
	file->read(data.begin(), size);

	const uint count = READ_LE_UINT16(data.begin()) / 2;
	if (count == 0 || 2 * count > size)
		return false;

	_entries.resize(count);

	for (uint i = 0; i != count; i++) {
		uint32 at = READ_LE_UINT16(data.begin() + 2 * i);

		while (at + 3 < size) {
			if (data[at] == 0xff && data[at + 1] == 0xff)
				break;

			// The line is kept the other way round from everything else: ten
			// bits of it are the line's own number and the rest is the game's
			// own marking, of which the top two bits carry the high part of
			// the condition.
			const uint16 word = (data[at + 2] << 8) | data[at + 3];

			// 0x9FC1 builds the condition out of the record's own byte and the
			// top two bits of the line's high byte, and it is counted from
			// one; a nought names no condition at all.
			const uint16 named = data[at + 1] | (uint16)(((data[at + 2] >> 6) & 3) << 8);
			const uint16 condition = named != 0 ? (uint16)(named - 1) : (uint16)0xffff;

			const Record record = { data[at], data[at], condition, (uint16)(word & 0x3ff),
				(uint16)((at - 2) / 4), (byte)((word >> 10) & 3), (word & 0x1000) != 0 };

			_entries[i].push_back(record);
			at += 4;
		}
	}

	debugC(1, kDebugScript, "DIALOGUE.HSQ: %u parts", count);
	return true;
}

int Dialogue::speakerBase(uint speaker) const {
	// Eight parts to a speaker, and the first of them with anything in it says
	// where their lines begin.
	for (uint i = 0; i != 8; i++) {
		const uint part = 8 * speaker + i;

		if (part < _entries.size() && !_entries[part].empty())
			return (int)_entries[part][0].line - 1;

	}

	return 0;
}

const Dialogue::Record *Dialogue::bookRecord(uint16 key) const {
	for (uint part = 0; part < _entries.size(); part++)
		for (uint line = 0; line < _entries[part].size(); line++)
			if (_entries[part][line].bookKey == (key & 0x7ff))
				return &_entries[part][line];
	return nullptr;
}

} // End of namespace Dune
