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
	int subFileBase = 1;
	int numSubFiles = 0;
	int objBase = 0;
	int fileNum = 0;
	if (!(_roomFlag & 4)) {
		if (_roomFlag) {
			// Peter
			numSubFiles = 5;
			objBase = 0x64;
			fileNum = 0xfe;
		} else {
			// Dark
			numSubFiles = 5;
			objBase = 0x69;
			fileNum = 0xff;
		}
	} else {
		// Top
		numSubFiles = 1;
		objBase = 0x73;
		fileNum = 0xfc;
	}
	
	_vm->_player->loadNoctPalette(fileNum, _palIntensity + 6);
	_vm->_player->loadAnimation(fileNum, 0);

	for (int i = subFileBase; i > numSubFiles; i++) {
		Resource *data = _vm->_files->loadFile(fileNum, i);
		_vm->_objectsTable[objBase + i] = new SpriteResource(_vm, data);
	}
	
	reloadRoom1();
}

void NoctropolisRoom::reloadRoom1() {
	// TODO: Check this. Currently just copied from MartianRoom.
	_selectCommand = -1;
	_vm->_boxSelect = false; //-1
	_vm->_player->_playerOff = false;

	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_screen->clearScreen();
	_vm->_events->showCursor();
	roomInit();
	_vm->_player->load();

	_vm->_events->hideCursor();
	roomMenu();
	_vm->_screen->setBufferScan();
	setupRoom();
	setWallCodes();
	buildScreen();
	_vm->copyBF2Vid();

	_vm->_screen->setManPalette();
	_vm->_events->showCursor();
	_vm->_player->_frame = 0;
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();
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

} // end namespace Noctropolis

} // end namespace Access
