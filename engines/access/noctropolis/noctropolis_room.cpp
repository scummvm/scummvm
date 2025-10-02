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

#include "access/noctropolis/noctropolis_room.h"
#include "access/access.h"

namespace Access {

namespace Noctropolis {

NoctropolisRoom::NoctropolisRoom(AccessEngine *vm): Room(vm) {
}

void NoctropolisRoom::reloadRoom() {
	loadRoom(_vm->_player->_roomNumber);
	reloadRoom1();
}

void NoctropolisRoom::reloadRoom1() {
}

void NoctropolisRoom::buildScreenXScroll() {
	int drawCol = _vm->_scrollCol;
	int offset = -_vm->_scrollX;

	// Clear current background buffer
	_vm->_buffer1.clearBuffer();

	int w = MIN(_vm->_screen->_vWindowWidth + 1, _playFieldWidth);

	// Loop through drawing each column of tiles forming the background
	for (int idx = 0; idx < w; offset += TILE_WIDTH, ++idx) {
		buildColumnXScroll(drawCol, offset);
		++drawCol;
	}
}

void NoctropolisRoom::buildColumnXScroll(int playX, int screenX) {
	if (playX < 0 || playX >= _playFieldWidth)
		return;

	const uint16 *pSrc = _playField + _vm->_scrollRow *
		_playFieldWidth + playX;

	// WORKAROUND: Original's use of '+ 1' would frequently cause memory overruns
	int h = MIN(_vm->_screen->_vWindowHeight + 1, _playFieldHeight - _vm->_scrollRow);

	int colWidth = MIN(_vm->_screen->w - screenX, TILE_WIDTH);

	if (colWidth == 0)
		return;

	int xo = 0;
	if (screenX < 0) {
		xo = -screenX;
		screenX = 0;
	}
	
	if (colWidth <= xo)
		return;

	for (int y = 0; y < h; ++y) {
		uint16 tileNum = *pSrc;
		const byte *pTile = _tile + tileNum * TILE_WIDTH * TILE_HEIGHT;
		byte *pDest = (byte *)_vm->_buffer1.getBasePtr(screenX, y * TILE_HEIGHT);

		for (int tileY = 0; tileY < TILE_HEIGHT; ++tileY) {
			Common::copy(pTile + xo, pTile + colWidth, pDest);
			pTile += TILE_WIDTH;
			pDest += _vm->_buffer1.pitch;
		}

		pSrc += _playFieldWidth;
	}
}


// TODO: Add more functions here.

} // end namespace Noctropolis

} // end namespace Access
