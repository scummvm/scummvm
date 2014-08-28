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
	_oldTitleChap = _chapter = 0;
	_updateChapter = -1;
	_topList = 0;
	_botList = 0;
	_riverIndex = 0;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	_tilePos = Common::Point(0, 0);

	Common::fill(&_esTabTable[0], &_esTabTable[100], 0);
	memset(_tileData, 0, sizeof(_tileData));

	_hintLevel = 0;
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
	_events->setCursor(CURSOR_ARROW);
	_events->showCursor();
	_screen->setPanel(0);

	//TODO: Implement the rest of the intro
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
	_destIn = &_buffer2;

	_screen->forceFadeOut();
	_events->hideCursor();

	_sound->queueSound(0, 98, 30);
	_sound->_soundPriority[0] = 1;

	_sound->queueSound(1, 98, 8);
	_sound->_soundPriority[1] = 1;

	_screen->_loadPalFlag = false;
	_files->loadScreen(0, 3);
	
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();
	_sound->playSound(1);

	byte *spriteData = _files->loadFile(0, 2);
	_objectsTable[0] = new SpriteResource(this, spriteData, _files->_filesize,
		DisposeAfterUse::YES);

	_sound->playSound(1);

	_screen->_loadPalFlag = false;
	_files->loadScreen(0, 4);
	_sound->playSound(1);

	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_sound->playSound(1);

	const int COUNTDOWN[6] = { 2, 0x80, 1, 0x7d, 0, 0x87 };
	for (_pCount = 0; _pCount <= 3; ++_pCount) {
		if (_pCount != 3) {
			_buffer2.copyFrom(_buffer1);
			int id = READ_LE_UINT16(COUNTDOWN + _pCount * 2);
			int xp = READ_LE_UINT16(COUNTDOWN + _pCount * 2 + 1);
			_screen->plotImage(_objectsTable[0], id, Common::Point(xp, 71));
			//TODO : more intro
		} else {
			//TODO : more intro
		}
	}
	//TODO : more intro
	delete _objectsTable[0];
}

void AmazonEngine::doOpening() {
	warning("TODO doOpening");
}

void AmazonEngine::doTent() {
	warning("TODO doTent");
}

void AmazonEngine::setupGame() {
	_chapter = 1;

	// Setup timers
	const int TIMER_DEFAULTS[] = { 3, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = 1;

		_timers.push_back(te);
	}

	// Miscellaneous
	_fonts._font1.load(FONT6x6_INDEX, FONT6x6_DATA);
	_fonts._font2.load(FONT2_INDEX, FONT2_DATA);

	// Set player room and position
	_player->_roomNumber = 4;
	_player->_playerX = _player->_rawPlayer.x = TRAVEL_POS[_player->_roomNumber][0];
	_player->_playerY = _player->_rawPlayer.y = TRAVEL_POS[_player->_roomNumber][1];
}

void AmazonEngine::establish(int esatabIndex, int sub) {
	_establishMode = 0;
	_establishGroup = 0;
	doEstablish(esatabIndex, sub);
}

void AmazonEngine::establishCenter(int esatabIndex, int sub) {
	_establishMode = 1;
	doEstablish(esatabIndex, sub);
}

const char *const _estTable[] = { "ETEXT0.DAT", "ETEXT1.DAT", "ETEXT2.DAT", "ETEXT3.DAT" };

void AmazonEngine::loadEstablish(int sub) {
	if (!_files->existFile("ETEXT.DAT")) {
		int oldGroup = _establishGroup;
		_establishGroup = 0;

		_eseg = _files->loadFile(_estTable[oldGroup]);
	} else {
		_eseg = _files->loadFile("ETEXT.DAT");
	}

	_establishCtrlTblOfs = READ_LE_UINT16(_eseg);

	int ofs = _establishCtrlTblOfs + (sub * 2);
	int idx = READ_LE_UINT16(_eseg + ofs);
	_narateFile = READ_LE_UINT16(_eseg + idx);
	_txtPages = READ_LE_UINT16(_eseg + idx + 2);

	if (!_txtPages)
		return;

	_sndSubFile = READ_LE_UINT16(_eseg + idx + 4);
	for (int i = 0; i < _txtPages; ++i)
		_countTbl[i] = READ_LE_UINT16(_eseg + idx + 6 + (2 * i));
}

void AmazonEngine::doEstablish(int esatabIndex, int sub) {
	_establishMode = 1;

	_screen->forceFadeOut();
	_screen->clearScreen();
	_screen->setPanel(3);

	if (esatabIndex != -1) {
		_files->loadScreen(95, esatabIndex);
		_buffer2.copyBuffer(_screen);
	}

	_screen->setIconPalette();
	_screen->forceFadeIn();

	_fonts._charSet._lo = 1;
	_fonts._charSet._hi = 10;
	_fonts._charFor._lo = 29;
	_fonts._charFor._hi = 32;

	_screen->_maxChars = 37;
	_screen->_printOrg = _screen->_printStart = Common::Point(48, 35);
	loadEstablish(sub);
	_et = sub;
	uint16 msgOffset = READ_LE_UINT16(_eseg + (sub * 2) + 2);

	_printEnd = 155;
	if (_txtPages == 0) {
		Common::String msg((const char *)_eseg + msgOffset);
		_fonts._font2.printText(_screen, msg);
	} else {
		Common::Array<Common::String> msgArr;
		for (int i = 0; i < _txtPages; ++i) {
			Common::String msg((const char *)_eseg + msgOffset);
			msgOffset += msg.size() + 1;
			msgArr.push_back(msg);
		}
		speakText(_screen, msgArr);
	}

	_screen->forceFadeOut();
	_screen->clearScreen();

	free(_eseg);
	if (_establishMode == 0)
		_room->init4Quads();
}

const char *const _tileFiles[] = {
	"GRAY.BLK", "RED.BLK", "LTBROWN.BLK", "DKBROWN.BLK", "VIOLET.BLK", "LITEBLUE.BLK",
	"DARKBLUE.BLK", "CYAN.BLK", "GREEN.BLK", "OLIVE.BLK", "GRAY.BLK", "RED.BLK",
	"LTBROWN.BLK", "DKBROWN.BLK", "VIOLET.BLK", "OLIVE.BLK"
};

void AmazonEngine::tileScreen() {
	if (!_screen->_vesaMode)
		return;

	if (!_clearSummaryFlag && (_oldTitleChap == _chapter))
		return;

	_oldTitleChap = _chapter;
	int idx = _chapter - 1;

	if (!_files->existFile(_tileFiles[idx]))
		return;

	byte *data = _files->loadFile(_tileFiles[idx]);
	int x = READ_LE_UINT16(data);
	int y = READ_LE_UINT16(data + 2);
	int size = ((x + 2) * y) + 10;
	
	for (int i = 0; i < size; ++i)
		_tileData[i] = data[i + 4];

	// CHECKME: Depending on the Vesa mode during initialization, 400 or 480
	for (_tilePos.y = 0; _tilePos.y < 480; _tilePos.y += y) {
		for (_tilePos.x = 0; _tilePos.x < 640; _tilePos.x += x)
			warning("TODO: DRAWOBJECT");
	}

}

void AmazonEngine::updateSummary(int chap) {
	if (!_screen->_vesaMode)
		return;

	int chapter = chap;
	if (chapter > 16)
		chapter = 16;

	if (!_clearSummaryFlag && (chapter == _updateChapter))
		return;

	_clearSummaryFlag = false;
	int celSubFile = 0;
	_updateChapter = chapter;
	Common::Array<CellIdent> summaryCells;
	loadCells(summaryCells);
	
	for (int i = celSubFile; i < 16; ++i) {
		if (i > 7)
			warning("TODO: DRAWOBJECT");
		else
			warning("TODO: DRAWOBJECT");
	}

	delete _objectsTable[93];
	_objectsTable[93] = nullptr;

	for (int i = 1; i <= _updateChapter; ++i) {
		celSubFile = i;
		loadCells(summaryCells);
		if (i > 8)
			warning("TODO: DRAWOBJECT");
		else
			warning("TODO: DRAWOBJECT");

		delete _objectsTable[93];
		_objectsTable[93] = nullptr;
	}
}

void AmazonEngine::drawHelp() {
	error("TODO: drawHelp");
}

} // End of namespace Amazon

} // End of namespace Access
