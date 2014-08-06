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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "access/amazon/amazon_game.h"

namespace Access {

namespace Amazon {

AmazonEngine::AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		AccessEngine(syst, gameDesc) {
	_skipStart = false;

	_canoeLane = 0;
	_canoeYPos = 0;
	_hitCount = 0;
	_saveRiver = 0;
	_hitSafe = 0;
	_chapter = 0;
	_topList = 0;
	_botList = 0;
	_riverIndex = 0;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	Common::fill(&_esTabTable[0], &_esTabTable[100], 0);
}

AmazonEngine::~AmazonEngine() {
}

void AmazonEngine::playGame() {
	_screen->setInitialPalettte();
	_events->setCursor(CURSOR_0);
	_events->showCursor();
	_screen->setPanel(0);
	
	doTitle();
	if (shouldQuit())
		return;

	if (!_skipStart) {
		_screen->setPanel(3);
		doOpening();
		if (shouldQuit())
			return;

		if (!_skipStart) {
			doTent();
			if (shouldQuit())
				return;
		}
	}

	doTitle();
	if (shouldQuit())
		return;

	_screen->clearScreen();
	_screen->setPanel(0);
	_screen->forceFadeOut();


}

void AmazonEngine::doTitle() {
	_screen->setDisplayScan();
	_destIn = (byte *)_buffer2.getPixels();

	_screen->forceFadeOut();
	_events->hideCursor();

	_sound->queueSound(0, 98, 30);
	_sound->_soundPriority[0] = 1;

	_screen->_loadPalFlag = false;
	_files->loadScreen(0, 3);
	
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();
	_sound->playSound(1);

	_objectsTable = _files->loadFile(0, 2);
	_sound->playSound(1);

	_screen->_loadPalFlag = false;
	_files->loadScreen(0, 4);
	_sound->playSound(1);

	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_sound->playSound(1);

	const int COUNTDOWN[6] = { 2, 0x80, 1, 0x7d, 0, 0x87 };
	for (int _pCount = 0; _pCount < 3; ++_pCount) {
		_buffer2.copyFrom(_buffer1);
		int id = READ_LE_UINT16(COUNTDOWN + _pCount * 4);
		int xp = READ_LE_UINT16(COUNTDOWN + _pCount * 4 + 2);
		_screen->plotImage(_objectsTable, id, Common::Point(xp, 71));
	}
	// TODO: More to do
}

void AmazonEngine::doOpening() {
	// TODO
}

void AmazonEngine::doTent() {
	// TODO
}


} // End of namespace Amazon

} // End of namespace Access
