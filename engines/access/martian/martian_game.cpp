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

MartianEngine::MartianEngine(OSystem *syst, const AccessGameDescription *gameDesc) : AccessEngine(syst, gameDesc) {
}

MartianEngine::~MartianEngine() {
}

void MartianEngine::initObjects() {
	_room = new MartianRoom(this);
	_scripts = new MartianScripts(this);
}

void MartianEngine::configSelect() {
	// No implementation required in MM
}

void MartianEngine::initVariables() {
	warning("TODO: initVariables");

	// Set player room and position
	_player->_roomNumber = 7;

	_inventory->_startInvItem = 0;
	_inventory->_startInvBox = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[100], (SpriteResource *)nullptr);
	_player->_playerOff = false;

	// Setup timers
	const int TIMER_DEFAULTS[] = { 4, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = 1;

		_timers.push_back(te);
	}

	_player->_playerX = _player->_rawPlayer.x = TRAVEL_POS[_player->_roomNumber][0];
	_player->_playerY = _player->_rawPlayer.y = TRAVEL_POS[_player->_roomNumber][1];
	_room->_selectCommand = -1;
	_events->setNormalCursor(CURSOR_CROSSHAIRS);
	_mouseMode = 0;
	_numAnimTimers = 0;

	for (int i = 0; i < 60; i++)
		TRAVEL[i] = 0;
	TRAVEL[7] = 1;

	for (int i = 0; i < 40; i++)
		ASK[i] = 0;
	ASK[33] = 1;
}

void MartianEngine::setNoteParams() {
	_events->hideCursor();

	_screen->_orgX1 = 58;
	_screen->_orgY1 = 124;
	_screen->_orgX2 = 297;
	_screen->_orgY2 = 199;
	_screen->_lColor = 51;
	_screen->drawRect();

	_events->showCursor();
}

void MartianEngine::displayNote(const Common::String &msg) {
	_fonts._charSet._lo = 1;
	_fonts._charSet._hi = 8;
	_fonts._charFor._lo = 0;
	_fonts._charFor._hi = 255;

	_screen->_maxChars = 40;
	_screen->_printOrg = _screen->_printStart = Common::Point(59, 124);
	
	setNoteParams();

	Common::String lines = msg;
	Common::String line;
	int width = 0;
	bool lastLine = false;
	do {
		lastLine = _fonts._font1.getLine(lines, _screen->_maxChars * 6, line, width);
		_fonts._font1.drawString(_screen, line, _screen->_printOrg);
		_screen->_printOrg = Common::Point(_screen->_printStart.x, _screen->_printOrg.y + 6);

		if (_screen->_printOrg.y == 196) {
			_events->waitKeyMouse();
			setNoteParams();
			_screen->_printOrg = _screen->_printStart;
		}
	} while (!lastLine);
	_events->waitKeyMouse();
}

void MartianEngine::doSpecial5(int param1) {
	warning("TODO: Push midi song");
	_midi->stopSong();
	_midi->_byte1F781 = false;
	_midi->loadMusic(47, 4);
	_midi->midiPlay();
	_screen->setDisplayScan();
	_events->clearEvents();
	_screen->forceFadeOut();
	_events->hideCursor();
	_files->loadScreen("DATA.SC");
	_events->showCursor();
	_screen->setIconPalette();
	_screen->forceFadeIn();

	Resource *cellsRes = _files->loadFile("CELLS00.LZ");
	_objectsTable[0] = new SpriteResource(this, cellsRes);
	delete cellsRes;

	_timers[20]._timer = _timers[20]._initTm = 30;
	Resource *notesRes = _files->loadFile("NOTES.DAT");
	notesRes->_stream->skip(param1 * 2);
	int pos = notesRes->_stream->readUint16LE();
	notesRes->_stream->seek(pos);
	Common::String msg = "";
	byte c;
	while ((c = (char)notesRes->_stream->readByte()) != '\0')
		msg += c;

	displayNote(msg);
	
	_midi->stopSong();
	_midi->freeMusic();

	warning("TODO: Pop Midi");
	// _midi->_byte1F781 = true;
}

void MartianEngine::playGame() {
	// Initialize Amazon game-specific objects
	initObjects();

	// Setup the game
	setupGame();
	configSelect();

	if (_loadSaveSlot == -1) {
		// Do introduction
		doCredits();
		if (shouldQuit())
			return;

		// Display Notes screen
		doSpecial5(4);
		if (shouldQuit())
			return;
		_screen->forceFadeOut();
	}

	do {
		_restartFl = false;
		_screen->clearScreen();
		_screen->setPanel(0);
		_screen->forceFadeOut();
		_events->showCursor();

		initVariables();

		// If there's a pending savegame to load, load it
		if (_loadSaveSlot != -1) {
			loadGameState(_loadSaveSlot);
			_loadSaveSlot = -1;
		}

		// Execute the room
		_room->doRoom();
	} while (_restartFl);
}

bool MartianEngine::showCredits() {
	_events->hideCursor();
	_screen->clearScreen();
	_destIn = _screen;

	int val1 = _creditsStream->readSint16LE();
	int val2 = 0;
	int val3 = 0;

	while(val1 != -1) {
		val2 = _creditsStream->readSint16LE();
		val3 = _creditsStream->readSint16LE();
		_screen->plotImage(_introObjects, val3, Common::Point(val1, val2));

		val1 = _creditsStream->readSint16LE();
	}

	val2 = _creditsStream->readSint16LE();
	if (val2 == -1) {
		_events->showCursor();
		_screen->forceFadeOut();
		return true;
	}

	_screen->forceFadeIn();
	_timers[3]._timer = _timers[3]._initTm = val2;

	while (!shouldQuit() && !_events->isKeyMousePressed() && _timers[3]._timer) {
		_events->pollEventsAndWait();
	}

	_events->showCursor();
	_screen->forceFadeOut();

	if (_events->_rightButton)
		return true;
	else
		return false;
}

void MartianEngine::doCredits() {
	_midi->_byte1F781 = false;
	_midi->loadMusic(47, 3);
	_midi->midiPlay();
	_screen->setDisplayScan();
	_events->hideCursor();
	_screen->forceFadeOut();
	Resource *data = _files->loadFile(41, 1);
	_introObjects = new SpriteResource(this, data);
	delete data;

	_files->loadScreen(41, 0);
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_events->showCursor();
	_creditsStream = new Common::MemoryReadStream(CREDIT_DATA, 180);

	if (!showCredits()) {
		_screen->copyFrom(_buffer2);
		_screen->forceFadeIn();

		_events->_vbCount = 550;
		while (!shouldQuit() && !_events->isKeyMousePressed() && _events->_vbCount > 0)
			_events->pollEventsAndWait();

		_screen->forceFadeOut();
		while (!shouldQuit() && !_events->isKeyMousePressed()&& !showCredits())
			_events->pollEventsAndWait();

		warning("TODO: Free word_21E2B");
		_midi->freeMusic();
	}
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
