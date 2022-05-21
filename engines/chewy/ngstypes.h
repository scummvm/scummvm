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

#ifndef CHEWY_NGSTYPES_H
#define CHEWY_NGSTYPES_H

#include "graphics/screen.h"

namespace Chewy {

struct TafInfo {
	int16 _count = 0;
	byte *_palette = nullptr;
	int16 *_correction = nullptr;
	byte **_image = nullptr;
};

struct TafSeqInfo {
	int16 _count = 0;
	int16 *_correction = nullptr;
	byte **_image = nullptr;
};

struct NewPhead {
	char _id[4] = { 0 };
	uint16 _type = 0;
	uint16 _poolNr = 0;

	bool load(Common::SeekableReadStream *src);
};

struct MouseInfo {
	int16 _button = 0;
};

struct musik_info {
	int16 musik_playing = 0;
	int16 play_mode = 0;
	int16 pattern_line = 0;
	int16 sequence_pos = 0;
	int16 cur_pattnr = 0;
	char *cur_pattern = nullptr;
};

struct GedChunkHeader {
	uint32 Len = 0;
	int16 X = 0;
	int16 Y = 0;
	int16 Ebenen = 0;

	bool load(Common::SeekableReadStream *src);
};

struct CurBlk {
	byte **sprite = nullptr;
};

struct CustomInfo {
	byte *TempArea = 0;
};

} // namespace Chewy

#endif
