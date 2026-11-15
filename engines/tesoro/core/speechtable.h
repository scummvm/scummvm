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

#ifndef TESORO_CORE_SPEECHTABLE_H
#define TESORO_CORE_SPEECHTABLE_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Tesoro {

class Archive;

/**
 * One of the four text files of spoken lines (OBJETOS, ANIMS, CONVERS, ICONS). Each line is `ID, "text"`; the
 * voice of a line is the archive entry ID.WAV. Lines are kept sorted by the key of the ID, largest first.
 */
class SpeechTable {
public:
	struct Line {
		const char *str() const { return text.begin(); }
		uint64 key;
		uint32 wavOffset;	// 0 when the archive has no ID.WAV
		Common::Array<char> text;	// NUL ended, bytes as in the file (an old DOS code page)
	};

	/** Reads a text file image. */
	void load(const byte *text, uint32 size, const Archive &archive);

	uint32 count() const { return _lines.size(); }
	const Line &line(uint32 i) const { return _lines[i]; }
	/** Index of the line with this key, or -1. */
	int find(uint64 key) const;

private:
	Common::Array<Line> _lines;
};

/** The four tables in the order the game searches them. */
class SpeechTables {
public:
	static const uint kCount = 4;

	SpeechTable &table(uint i) { return _tables[i]; }
	const SpeechTable &table(uint i) const { return _tables[i]; }

	/** What a script holds for a spoken line: the table and line numbers plus one, or 0 if no table has the name. */
	uint32 find(const char *name) const;
	const SpeechTable::Line &lineOf(uint32 ref) const { return _tables[(ref - 1) >> 24].line((ref - 1) & 0xffffff); }

private:
	SpeechTable _tables[kCount];
};

} // End of namespace Tesoro

#endif
