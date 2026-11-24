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

#ifndef DUNE_DIALOGUE_H
#define DUNE_DIALOGUE_H

#include "common/array.h"
#include "common/str.h"

#include "dune/archive.h"

namespace Dune {

/**
 * DIALOGUE.HSQ holds what each of the people in the game has to say.  It
 * opens with an index of its own - offsets to 136 parts, the first offset
 * giving their number - and each part is four byte records ended by 0xFFFF.
 * The routine at 0x9F9E is the one that reads them:
 *
 *   byte flags;       // 0x80 with 0x40 clear means the low bits are tested
 *                     // against a byte of game state before going further
 *   byte condition;   // which program of CONDIT.HSQ decides whether the
 *                     // line is reached
 *   byte line[2];     // big endian; the low ten bits are the line spoken
 *
 * Read in order, a part is plainly one person's: part 1 is the Duke, from
 * asking Paul what he has done with Gurney to leaving for the desert.
 */
class Dialogue {
public:
	struct Record {
		byte   flags;
		// The high bit is also set by the game after a line is spoken.  Keep
		// the resource's original byte beside the mutable one so loading a
		// checkpoint can undo marks made on an abandoned timeline without
		// erasing high bits which were present in DIALOGUE.HSQ itself.
		byte   baseFlags;
		// Which expression of CONDIT.HSQ decides whether the line is reached.
		// The byte of the record holds its low eight bits and the top two bits
		// of the line's own high byte the next two, which 0x9FC1 puts together;
		// 0xA3A3 then reads the offset two bytes before the one the index
		// names, so the expressions are counted from one.  What is kept here is
		// what CONDIT.HSQ is asked for, counting from nought, and 0xffff where
		// the record names none.
		uint16 condition;
		// Which line of the phrase files, counted from one: 0xCF71 takes one
		// off the number before it looks the line up.  Ten bits are enough -
		// the highest any record names is 479, which is the last line of the
		// first of the two files - and the bits above them are the game's own
		// marking.
		uint16 line;
		// A05E/B150 identify book pages by the resource record, not its
		// PHRASE number. Bits 10/11 of the packed line classify its topic.
		uint16 bookKey;
		byte bookTopic;
		// A0F1: packed line bit12 forces this local report to use balloons
		// even when the retained user preference is No Subtitles.
		bool forceBalloon;
	};

	bool load(Archive &archive);

	// Where a speaker's lines begin, worked out the way 0xCFB9 does: of the
	// eight parts that belong to them, the first that is not empty, the line
	// its first record names, less one.
	int speakerBase(uint speaker) const;
	const Record *bookRecord(uint16 key) const;

	uint count() const { return _entries.size(); }
	Common::Array<Record> &entry(uint index) { return _entries[index]; }
	const Common::Array<Record> &entry(uint index) const { return _entries[index]; }

private:
	Common::Array<Common::Array<Record> > _entries;
};

} // End of namespace Dune

#endif
