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

#include "access/resources.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_room.h"
#include "access/martian/martian_scripts.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

namespace Martian {

MartianEngine::MartianEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
AccessEngine(syst, gameDesc), _skipStart(false),
_creditsStream(nullptr)
{
}

MartianEngine::~MartianEngine() {
	_skipStart = false;
	_creditsStream = nullptr;
}

void MartianEngine::initObjects() {
	_room = new MartianRoom(this);
	_scripts = new MartianScripts(this);
}

void MartianEngine::configSelect() {
	// No implementation required in MM
}

void MartianEngine::initVariables() {
	// Set player room and position
	_player->_roomNumber = 7;

	_inventory->_startInvItem = 0;
	_inventory->_startInvBox = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[100], (SpriteResource *)nullptr);
	_player->_playerOff = false;

	setupTimers();

	_player->_playerX = _player->_rawPlayer.x = _res->ROOMTBL[_player->_roomNumber]._travelPos.x;
	_player->_playerY = _player->_rawPlayer.y = _res->ROOMTBL[_player->_roomNumber]._travelPos.y;
	_room->_selectCommand = -1;
	_events->setNormalCursor(CURSOR_CROSSHAIRS);
	_mouseMode = 0;
	_animation->clearTimers();

	ARRAYCLEAR(_travel);
	_travel[7] = 1;

	ARRAYCLEAR(_ask);
	_ask[33] = 1;

	ARRAYCLEAR(_flags);

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
	Font::_fontColors[3] = 0;

	_screen->_maxChars = 40;
	_screen->_printOrg = _screen->_printStart = Common::Point(59, 124);

	setNoteParams();

	Common::String lines = msg;
	Common::String line;
	int width = 0;
	bool lastLine = false;
	do {
		lastLine = _fonts._font1->getLine(lines, _screen->_maxChars, line, width, Font::kWidthInChars);
		_bubbleBox->printString(line);
		_screen->_printOrg = Common::Point(_screen->_printStart.x, _screen->_printOrg.y + 6);

		if (_screen->_printOrg.y == 196) {
			_events->waitKeyActionMouse();
			setNoteParams();
			_screen->_printOrg = _screen->_printStart;
		}
	} while (!lastLine);
	_events->waitKeyActionMouse();
}

void MartianEngine::doSpecial5(int param1) {
	// Seems redundant to store the song as this is
	// only ever called from restart or load?
	debug("TODO: Push midi song?");
	_midi->stopSong();
	_midi->setLoop(false);
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
	Common::String msg = notesRes->_stream->readString();
	delete notesRes;
	displayNote(msg);

	_midi->stopSong();
	_midi->freeMusic();

	_midi->setLoop(true);
}

void MartianEngine::playGame() {
	// Initialize Martian Memorandum game-specific objects
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

	int posX = _creditsStream->readSint16LE();
	int posY = 0;

	while (posX != -1) {
		posY = _creditsStream->readSint16LE();
		int frameNum = _creditsStream->readSint16LE();
		_screen->plotImage(_objectsTable[41], frameNum, Common::Point(posX, posY));

		posX = _creditsStream->readSint16LE();
	}

	posY = _creditsStream->readSint16LE();
	if (posY == -1) {
		_events->showCursor();
		_screen->forceFadeOut();
		return true;
	}

	_screen->forceFadeIn();
	_timers[3]._timer = _timers[3]._initTm = posY;

	while (!shouldQuit() && !_events->isKeyActionMousePressed() && _timers[3]._timer) {
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
	_midi->setLoop(false);
	_midi->loadMusic(47, 3);
	_midi->midiPlay();
	_screen->setDisplayScan();
	_events->hideCursor();
	_screen->forceFadeOut();
	Resource *data = _files->loadFile(41, 1);
	_objectsTable[41] = new SpriteResource(this, data);
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
		while (!shouldQuit() && !_events->isKeyActionMousePressed() && _events->_vbCount > 0)
			_events->pollEventsAndWait();

		_screen->forceFadeOut();
		while (!shouldQuit() && !_events->isKeyActionMousePressed()&& !showCredits())
			_events->pollEventsAndWait();

		delete _objectsTable[41];
		_objectsTable[41] = nullptr;
		_midi->freeMusic();
	}
	_midi->setLoop(true);
}

void MartianEngine::setupTimers() {
	_timers.clear();
	const int TIMER_DEFAULTS[] = { 4, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = 1;

		_timers.push_back(te);
	}
}

void MartianEngine::setupGame() {
	// Load death list
	_deaths.resize(_res->DEATHS.size());
	for (uint idx = 0; idx < _deaths.size(); ++idx) {
		_deaths[idx]._screenId = _res->DEATHS[idx]._screenId;
		_deaths[idx]._msg = _res->DEATHS[idx]._msg;
	}

	setupTimers();

	// Miscellaneous
	Martian::MartianResources &res = *((Martian::MartianResources *)_res);
	_fonts.load(res._font1, res._font2, res._bitFont);

	// Set player room and position
	_player->_roomNumber = 7;
	_player->_playerX = _player->_rawPlayer.x = _res->ROOMTBL[_player->_roomNumber]._travelPos.x;
	_player->_playerY = _player->_rawPlayer.y = _res->ROOMTBL[_player->_roomNumber]._travelPos.y;
}

void MartianEngine::showExpositionText(Common::String msg) {
	Common::String line = "";
	int width = 0;
	bool lastLine;
	do {
		lastLine = _fonts._font2->getLine(msg, _screen->_maxChars, line, width, Font::kWidthInChars);
		// Draw the text
		_bubbleBox->printString(line);

		_screen->_printOrg.y += 6;
		_screen->_printOrg.x = _screen->_printStart.x;

		if (_screen->_printOrg.y == 180) {
			_events->waitKeyActionMouse();
			_screen->copyBuffer(&_buffer2);
			_screen->_printOrg.y = _screen->_printStart.y;
		}
	} while (!lastLine);
	// Avoid re-using double-click
	_events->clearEvents();
	_events->waitKeyActionMouse();
}

void MartianEngine::dead(int deathId) {
	// Load and display death screen
	_events->hideCursor();
	_screen->forceFadeOut();
	_files->loadScreen(48, _deaths[deathId]._screenId - 1);
	_screen->setIconPalette();
	_buffer2.copyBuffer(_screen);
	_screen->forceFadeIn();
	_events->showCursor();

	// Setup fonts
	_fonts._charSet._hi = 10;
	_fonts._charSet._lo = 1;
	_fonts._charFor._lo = 247;
	_fonts._charFor._hi = 255;
	Font::_fontColors[3] = 247;
	_screen->_maxChars = 50;
	_screen->_printOrg = Common::Point(24, 18);
	_screen->_printStart = Common::Point(24, 18);

	// Display death message
	showExpositionText(_deaths[deathId]._msg);

	_screen->forceFadeOut();
	_room->clearRoom();
	freeChar();

	// The original was jumping to the restart label in main
	_restartFl = true;
	_events->pollEvents();
}

void MartianEngine::establish(int estabIndex, int sub) {
	_fonts._charSet._hi = 10;
	Font::_fontColors[0] = 0xff;
	Font::_fontColors[1] = 0xf7;
	Font::_fontColors[2] = 0xff;
	Font::_fontColors[3] = 0xf7;

	_screen->_maxChars = 50;
	_screen->_printOrg = _screen->_printStart = Common::Point(24, 18);

	// TODO: Original has a small delay here.

	Resource *notesRes = _files->loadFile("ETEXT.DAT");
	notesRes->_stream->seek(2 * sub);
	uint16 msgOffset = notesRes->_stream->readUint16LE();
	if (msgOffset == 0 || msgOffset >= notesRes->_stream->size()) {
		error("MartianEngine::establish: Invalid message offset %d for msg %d", msgOffset, sub);
	}

	notesRes->_stream->seek(msgOffset);

	Common::String msg = notesRes->_stream->readString();
	showExpositionText(msg);

	_events->hideCursor();
	if (sub != 0x3f) {
		_screen->forceFadeOut();
		_screen->clearScreen();
	}

	_events->showCursor();
}

void MartianEngine::synchronize(Common::Serializer &s) {
	AccessEngine::synchronize(s);

	for (int i = 0; i < ARRAYSIZE(_travel); i++) {
		s.syncAsByte(_travel[i]);
	}

	for (int i = 0; i < ARRAYSIZE(_ask); i++) {
		s.syncAsByte(_ask[i]);
	}

	/*
	TODO: Do any of these need to be synchronized here?
	Mostly involved in modal dialogs.
	_startTravelItem
	_startTravelBox
	_startAboutItem
	_startAboutBox
	_byte26CB5
	_bcnt
	_boxDataStart
	_boxDataEnd
	_boxSelectY
	_boxSelectYOld
	_numLines
	*/
}


} // End of namespace Martian

} // End of namespace Access
