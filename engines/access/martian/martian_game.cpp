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
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_room.h"
#include "access/martian/martian_scripts.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

namespace Martian {

MartianEngine::MartianEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		AccessEngine(syst, gameDesc) {
}

MartianEngine::~MartianEngine() {
}

void MartianEngine::playGame() {
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
	_room = new MartianRoom(this);
	_scripts = new MartianScripts(this);
	_room->doRoom();
}

void MartianEngine::doIntroduction() {
	_screen->setInitialPalettte();
	_events->setCursor(CURSOR_ARROW);
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

void MartianEngine::doTitle() {
	_screen->setDisplayScan();
	_destIn = &_buffer2;

	_screen->forceFadeOut();
	_events->hideCursor();

	_sound->queueSound(0, 98, 30);

	_files->_loadPalFlag = false;
	_files->loadScreen(0, 3);
	
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();
	_sound->playSound(1);

	Resource *spriteData = _files->loadFile(0, 2);
	_objectsTable[0] = new SpriteResource(this, spriteData);
	delete spriteData;

	_sound->playSound(1);

	_files->_loadPalFlag = false;
	_files->loadScreen(0, 4);
	_sound->playSound(1);

	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_sound->playSound(1);

	const int COUNTDOWN[6] = { 2, 0x80, 1, 0x7d, 0, 0x87 };
	for (_pCount = 0; _pCount < 3; ++_pCount) {
		_buffer2.copyFrom(_buffer1);
		int id = READ_LE_UINT16(COUNTDOWN + _pCount * 4);
		int xp = READ_LE_UINT16(COUNTDOWN + _pCount * 4 + 2);
		_screen->plotImage(_objectsTable[0], id, Common::Point(xp, 71));
	}
	// TODO: More to do

	delete _objectsTable[0];
}

void MartianEngine::doOpening() {
	warning("TODO doOpening");
}

void MartianEngine::doTent() {
	warning("TODO doTent");
}

void MartianEngine::setupGame() {

	// Setup timers
	const int TIMER_DEFAULTS[] = { 4, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = 1;

		_timers.push_back(te);
	}

	// Miscellaneous
	// TODO: Replace with Martian fonts when located
	_fonts._font1.load(Amazon::FONT6x6_INDEX, Amazon::FONT6x6_DATA);
	_fonts._font2.load(Amazon::FONT2_INDEX, Amazon::FONT2_DATA);

	// Set player room and position
	_player->_roomNumber = 7;
	_player->_playerX = _player->_rawPlayer.x = TRAVEL_POS[_player->_roomNumber][0];
	_player->_playerY = _player->_rawPlayer.y = TRAVEL_POS[_player->_roomNumber][1];
}

void MartianEngine::drawHelp() {
	error("TODO: drawHelp");
}

} // End of namespace Martian

} // End of namespace Access
