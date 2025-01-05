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
			if (_stopOnF3)
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

/**
 * Flood fills from a series of start positions.
 *
 * Troll's Tale contains two separate flood fill implementations to handle the
 * special case of drawing the Troll. The game sets a global before drawing to
 * to activate Troll mode. We implement this by overriding this method and
 * the check method.
 */
void PictureMgr_Troll::draw_Fill() {
	draw_SetColor();
	_scrOn = true;

	byte x, y;
	if (_scrColor == 15) { // white
		// White flood fills are only allowed when drawing the Troll, otherwise they
		// are completely ignored. Several room pictures contain white flood fills.
		while (getNextCoordinates(x, y)) {
			if (_trollMode) {
				PictureMgr::draw_Fill(x, y);
			}
		}
	} else {
		// When not drawing the Troll, do a regular flood fill.
		// When drawing the Troll, first fill with white, and then fill normally.
		byte fillColor = _scrColor;
		while (getNextCoordinates(x, y)) {
			if (_trollMode) {
				_scrColor = 15; // white
				PictureMgr::draw_Fill(x, y);
				_scrColor = fillColor;
			}
			PictureMgr::draw_Fill(x, y);
		}
	}
}

/**
 * Checks if flood fill is allowed at a position.
 *
 * Troll's Tale contains two separate flood fill implementations to handle the
 * special case of drawing the Troll. The game sets a global before drawing to
 * to activate Troll mode.
 *
 * The Troll is a large picture with flood fills that is drawn over many busy
 * room pictures, and always in the same location. This is a problem because the
 * picture format is only meant to fill white areas. Sierra handled this by
 * reserving a color for the Troll's lines (11, light blue) and implementing a
 * second set of routines that fill white and treat the Troll's color as a
 * boundary, and sometimes white as well. When drawing the Troll, a non-white
 * fill is preceded by a special white fill to clear the area. This does not
 * work well if there are existing white pixels, and rooms do have these.
 * The Troll has incomplete fills in these rooms, but this is original behavior.
 * In some rooms, such as those with white checkered floors, the results are
 * so bad that Sierra added them to the list of rooms the Troll never visits.
 *
 * We implement Troll mode without a second flood fill algorithm; instead we
 * override the check method, and our AGI algorithm in the base class provides
 * the context so we know which of the two color checks to use.
 */
bool PictureMgr_Troll::draw_FillCheck(int16 x, int16 y, bool horizontalCheck) {
	if (_trollMode && _scrColor == 15) {
		if (!getGraphicsCoordinates(x, y)) {
			return false;
		}

		byte screenColor = _gfx->getColor(x, y);

		// when filling white during troll mode...
		if (horizontalCheck) {
			// horizontal checks only stop on troll line color
			return (screenColor != 11);
		} else {
			// all other checks stop on troll line color or white
			return (screenColor != 11) && (screenColor != 15);
		}
	}
	
	return PictureMgr::draw_FillCheck(x, y, horizontalCheck);
}

} // End of namespace Agi
