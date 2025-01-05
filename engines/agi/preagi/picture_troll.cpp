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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/picture.h"

#include "agi/preagi/picture_troll.h"

namespace Agi {

PictureMgr_Troll::PictureMgr_Troll(AgiBase *agi, GfxMgr *gfx) :
	PictureMgr(agi, gfx) {
}

void PictureMgr_Troll::drawPicture() {
	debugC(kDebugLevelPictures, "Drawing picture");

	_dataOffset = 0;
	_dataOffsetNibble = false;
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_priColor = 4;
	_scrColor = 15;

	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf0:
			// happens in all Troll's Tale pictures
			// TODO: figure out what it was meant for
			break;
		case 0xf1:
			draw_SetColor();
			_scrOn = true;
			break;
		case 0xf3:
			if (_flags & kPicFf3Stop)
				return;
			break;
		case 0xf8:
			yCorner(true);
			break;
		case 0xf9:
			xCorner(true);
			break;
		case 0xfa:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfb:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfe:
			draw_SetColor();
			_scrOn = true;
			draw_Fill();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode %02x at %04x", curByte, _dataOffset - 1);
			break;
		}
	}
}

} // End of namespace Agi
