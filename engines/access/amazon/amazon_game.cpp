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

#include "access/resources.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_room.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonEngine::AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		AccessEngine(syst, gameDesc),
		_guardLocation(_flags[122]), _guardFind(_flags[128]), _helpLevel(_flags[167]), 
		_jasMayaFlag(_flags[168]), _moreHelp(_flags[169]), _flashbackFlag(_flags[171]),
		_riverFlag(_flags[185]), _aniOutFlag(_flags[195]), _badEnd(_flags[218]), 
		_noHints(_flags[219]), _aniFlag(_flags[229]), _allenFlag(_flags[237]), 
		_noSound(_flags[239]) {
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
	// Do introduction
	doIntroduction();
	if (shouldQuit())
		return;

	// Setup the game
	setupGame();

	_screen->clearScreen();
	_screen->setPanel(0);
	_screen->forceFadeOut();

	_events->showCursor();

	// Setup and execute the room
	_room = new AmazonRoom(this);
	_scripts = new AmazonScripts(this);
	_room->doRoom();
}

void AmazonEngine::doIntroduction() {
	_screen->setInitialPalettte();
	_events->setCursor(CURSOR_0);
	_events->showCursor();
	_screen->setPanel(0);

	// TODO: Worry about implementing full intro sequence later
	return;

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

	_objectsTable[0] = _files->loadFile(0, 2);
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
		_screen->plotImage(_objectsTable[0], id, Common::Point(xp, 71));
	}
	// TODO: More to do
}

void AmazonEngine::doOpening() {
	// TODO
}

void AmazonEngine::doTent() {
	// TODO
}

void AmazonEngine::setupGame() {
	_chapter = 1;
	clearCellTable();

	// Setup timers
	const int TIMER_DEFAULTS[] = { 3, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = true;

		_timers.push_back(te);
	}

	// Set miscellaneous fields
	_player->_roomNumber = 4;
	_player->_playerX = _player->_rawPlayer.x = TRAVEL_POS[_player->_roomNumber][0];
	_player->_playerY = _player->_rawPlayer.y = TRAVEL_POS[_player->_roomNumber][1];
	_selectCommand = -1;
}

} // End of namespace Amazon

} // End of namespace Access
