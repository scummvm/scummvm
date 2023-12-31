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

#ifndef DGDS_DIALOUGE_H
#define DGDS_DIALOUGE_H

#include "common/stream.h"
#include "common/array.h"

namespace Dgds {

class DialogueLine {
public:
    uint16 unk[9];
    uint16 x;
    uint16 y;
    uint16 w;
    uint16 h;
    uint16 bgcol;
    uint16 fontcol;   // 0 = black , 0xf = white
    uint16 unk2;
    uint16 unk3;
    uint16 fontsize;  // 01 = 8x8, 02 = 6x6, 03 = 4x5
    uint16 just;      // 0x00 =t top, 0x03 = center
    uint16 unk4;
    uint16 frametype; // 01 =simple frame, 02 = with title (text before :), 03 = baloon, 04 = eliptical baloon
    uint16 unk5;
    uint16 unk6;
    uint16 textLen;
    Common::String text;
};

class Dialogue {
public:
	void parseSDS(Common::SeekableReadStream *s);

	const Common::Array<DialogueLine> &getLines() const { return _dialogue; }
private:
	Common::Array<DialogueLine> _dialogue;
};

} // End of namespace Dgds

#endif // DGDS_DIALOUGE_H

