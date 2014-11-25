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
	_charSegSwitch = false;

	_canoeLane = 0;
	_canoeYPos = 0;
	_hitCount = 0;
	_saveRiver = 0;
	_hitSafe = 0;
	_oldTitleChapter = _chapter = 0;
	_updateChapter = -1;
	_topList = nullptr;
	_botList = nullptr;
	_riverIndex = 0;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	_tilePos = Common::Point(0, 0);
	_hintLevel = 0;

	_antDirection = NONE;
	_pitDirection = NONE;
	_antCel = 0;
	_torchCel = 0;
	_pitCel = 0;
	_stabCel = 0;
	_antPos = Common::Point(0, 0);
	_antDieFl = _antEatFl = false;
	_stabFl = false;
	_pitPos = Common::Point(0, 0);

	Common::fill(&_esTabTable[0], &_esTabTable[100], 0);
	memset(_tileData, 0, sizeof(_tileData));
	
	_chapterCells.push_back(CellIdent(0, 96, 17));
	_inactive._spritesPtr = nullptr;
	_inactive._altSpritesPtr = nullptr;
	_inactive._flags = _inactive._frameNumber = _inactive._offsetY = 0;
	_inactive._position = Common::Point(0, 0);
}

AmazonEngine::~AmazonEngine() {
	delete _inactive._altSpritesPtr;
}

void AmazonEngine::freeInactivePlayer() {
	delete _inactive._altSpritesPtr;
	_inactive._altSpritesPtr = nullptr;
}

void AmazonEngine::playGame() {
	// Initialize Amazon game-specific objects
	_room = new AmazonRoom(this);
	_scripts = new AmazonScripts(this);

	// Setup the game
	setupGame();

	if (_loadSaveSlot == -1) {
		// Do introduction
		doIntroduction();
		if (shouldQuit())
			return;
	}

	_screen->clearScreen();
	_screen->setPanel(0);
	_screen->forceFadeOut();
	_events->showCursor();

	// If there's a pending savegame to load, load it
	if (_loadSaveSlot != -1)
		loadGameState(_loadSaveSlot);

	// Execute the room
	_room->doRoom();
}

void AmazonEngine::doIntroduction() {
	_screen->setInitialPalettte();
	_events->setCursor(CURSOR_ARROW);
	_events->showCursor();
	_screen->setPanel(0);
	_screen->setPalette();

	_events->setCursor(CURSOR_ARROW);
	_events->showCursor();
	_screen->setPanel(3);
	doTitle();

	if (shouldQuit() || _skipStart)
		return;

	_screen->setPanel(3);
	((AmazonScripts *)_scripts)->mWhileDoOpen();

	if (shouldQuit() || _skipStart)
		return;

	doTent();
}

void AmazonEngine::doCredit() {
	if (_pCount < 15)
		return;

	if (_pCount <= 75)
		_buffer2.plotImage(_objectsTable[0], 0, Common::Point(90, 35));
	else if (_pCount <= 210)
		_buffer2.plotImage(_objectsTable[0], 1, Common::Point(65, 35));
	else if (_pCount <= 272)
		_buffer2.plotImage(_objectsTable[0], 2, Common::Point(96, 45));
	else if (_pCount <= 334)
		_buffer2.plotImage(_objectsTable[0], 3, Common::Point(68, 54));
	else if (_pCount <= 396)
		_buffer2.plotImage(_objectsTable[0], 4, Common::Point(103, 54));
	else if (_pCount <= 458) {
		_buffer2.plotImage(_objectsTable[0], 5, Common::Point(8, 5));
		_buffer2.plotImage(_objectsTable[0], 12, Common::Point(88, 55));
		_buffer2.plotImage(_objectsTable[0], 6, Common::Point(194, 98));
	} else if (_pCount <= 520) {
		_buffer2.plotImage(_objectsTable[0], 7, Common::Point(90, 35));
		_buffer2.plotImage(_objectsTable[0], 8, Common::Point(90, 35));
	} else if (_pCount <= 580) {
		_buffer2.plotImage(_objectsTable[0], 9, Common::Point(18, 15));
		_buffer2.plotImage(_objectsTable[0], 10, Common::Point(164, 81));
	} else
		_buffer2.plotImage(_objectsTable[0], 11, Common::Point(106, 55));
}

void AmazonEngine::scrollTitle() {
	copyBF1BF2();
	_newRects.clear();
	doCredit();
	copyRects();
	copyBF2Vid();
}

void AmazonEngine::doTitle() {
	_screen->setDisplayScan();
	_destIn = &_buffer2;

	_screen->forceFadeOut();
	_events->hideCursor();

	_sound->queueSound(0, 98, 30);
	_sound->queueSound(1, 98, 8);

	_files->_setPaletteFlag = false;
	_files->loadScreen(0, 3);
	
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();
	_sound->playSound(1);

	// HACK: This delay has been added so that the very first screen is visible.
	// The original was using disk loading time to display it, and it's too fast
	// nowadays to be visible.
	_events->_vbCount = 70;
	while (!shouldQuit() && _events->_vbCount > 0) {
		_events->pollEvents();
		g_system->delayMillis(10);
	}

	Resource *spriteData = _files->loadFile(0, 2);
	_objectsTable[0] = new SpriteResource(this, spriteData);
	delete spriteData;

	_sound->playSound(1);

	_files->_setPaletteFlag = false;
	_files->loadScreen(0, 4);
	_sound->playSound(1);

	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_sound->playSound(1);

	const int COUNTDOWN[6] = { 2, 0x80, 1, 0x7d, 0, 0x87 };
	for (_pCount = 0; _pCount < 3; ++_pCount) {
		_buffer2.copyFrom(_buffer1);
		int id = COUNTDOWN[_pCount * 2];
		int xp = COUNTDOWN[_pCount * 2 + 1];
		_buffer2.plotImage(_objectsTable[0], id, Common::Point(xp, 71));
		_screen->copyFrom(_buffer2);

		_events->_vbCount = 70;
		while (!shouldQuit() && _events->_vbCount > 0) {
			_events->pollEvents();
			g_system->delayMillis(10);
		}
	}

	_sound->playSound(0);
	_screen->forceFadeOut();
	_events->_vbCount = 100;
	while (!shouldQuit() && _events->_vbCount > 0) {
		_events->pollEvents();
		g_system->delayMillis(10);
	}

	_sound->freeSounds();
	delete _objectsTable[0];
	_objectsTable[0] = nullptr;

	_files->_setPaletteFlag = false;
	_files->loadScreen(0, 5);
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();
	_sound->newMusic(1, 0);
	_events->_vbCount = 700;
	warning("TODO: check on KEYBUFCNT");
	while (!shouldQuit() && (_events->_vbCount > 0) && (!_events->_leftButton) && (!_events->_rightButton)) {
		_events->pollEvents();
		g_system->delayMillis(10);
	}
	if (_events->_rightButton) {
		_skipStart = true;
		_room->clearRoom();
		_events->showCursor();
		return;
	}

	_sound->newMusic(1, 1);
	_sound->_musicRepeat = false;
	_events->zeroKeys();
	_room->loadRoom(0);
	_screen->clearScreen();
	_screen->setBufferScan();
	_screen->_scrollRow = _screen->_scrollCol = 0;
	_screen->_scrollX = _screen->_scrollY = 0;
	_player->_rawPlayer = Common::Point(0, 0);
	_screen->forceFadeOut();
	_screen->_scrollX = 0;
	_room->buildScreen();
	copyBF2Vid();
	_screen->forceFadeIn();
	_oldRects.clear();
	_newRects.clear();
	// KEYFLG = 0;
	_player->_scrollAmount = 1;
	_pCount = 0;

	while (!shouldQuit()) {
		if (_events->_rightButton) {
			_skipStart = true;
			_room->clearRoom();
			_events->showCursor();
			return;
		}

		warning("TODO: check on KEYBUFCNT");
		if (_events->_leftButton) {
			_room->clearRoom();
			_events->showCursor();
			return;
		}

		_events->_vbCount = 4;
		if (_screen->_scrollCol + _screen->_vWindowWidth != _room->_playFieldWidth) {
			_screen->_scrollX += _player->_scrollAmount;

			while (_screen->_scrollX >= TILE_WIDTH) {
				_screen->_scrollX -= TILE_WIDTH;
				++_screen->_scrollCol;

				_buffer1.moveBufferLeft();
				_room->buildColumn(_screen->_scrollCol + _screen->_vWindowWidth, _screen->_vWindowBytesWide);
			}
			scrollTitle();
			++_pCount;

			while (!shouldQuit() && (_events->_vbCount > 0)) {
				_events->pollEvents();
				g_system->delayMillis(10);
			}
			continue;
		}

		_events->_vbCount = 120;
		while (!shouldQuit() && (_events->_vbCount > 0)) {
			_events->pollEvents();
			g_system->delayMillis(10);
		}

		while (!shouldQuit()) {
			_pCount = 0;
			_events->_vbCount = 3;
			if (_screen->_scrollRow + _screen->_vWindowHeight >= _room->_playFieldHeight) {
				_room->clearRoom();
				_events->showCursor();
				return;
			}

			_screen->_scrollY = _screen->_scrollY + _player->_scrollAmount;

			while (_screen->_scrollY >= TILE_HEIGHT && !shouldQuit()) {
				_screen->_scrollY -= TILE_HEIGHT;
				++_screen->_scrollRow;
				_buffer1.moveBufferUp();

				_room->buildRow(_screen->_scrollRow + _screen->_vWindowHeight, _screen->_vWindowLinesTall * _screen->_bufferBytesWide);

				if (_screen->_scrollRow + _screen->_vWindowHeight >= _room->_playFieldHeight) {
					_room->clearRoom();
					_events->showCursor();
					return;
				}
				
				scrollTitle();
				while (!shouldQuit() && (_events->_vbCount > 0)) {
					_events->pollEvents();
					g_system->delayMillis(10);
				}
			}
		}
	}
}

void AmazonEngine::doTent() {
	int step = 0;
	_screen->setDisplayScan();
	_screen->forceFadeOut();
	_events->hideCursor();
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 39), 1));
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 14), 1));
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 15), 1));
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 16), 1));
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 31), 2));
	_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(98, 52), 2));
	_sound->playSound(0);

	_files->_setPaletteFlag = false;
	_files->loadScreen(2, 0);
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);
	_screen->forceFadeIn();

	_video->setVideo(_screen, Common::Point(126, 73), FileIdent(2, 1), 10);
	while (!shouldQuit() && !_video->_videoEnd) {
		_video->playVideo();
		if ((_video->_videoFrame == 32) || (_video->_videoFrame == 34))
			_sound->playSound(4);
		else if (_video->_videoFrame == 36) {
			if (step != 2) {
				_sound->playSound(2);
				step = 2;
			}
		} else if (_video->_videoFrame == 18) {
			if (step != 1) {
				_sound->newMusic(73, 1);
				_sound->newMusic(11, 0);
				step = 1;
				_sound->playSound(1);
			}
		}

		g_system->delayMillis(10);
		_events->pollEvents();
	}	

	_sound->playSound(5);
	_video->setVideo(_screen, Common::Point(43, 11), FileIdent(2, 2), 10);
	while (!shouldQuit() && !_video->_videoEnd) {
		_video->playVideo();
		if (_video->_videoFrame == 26) {
			_sound->playSound(5);
		} else if (_video->_videoFrame == 15) {
			if (step !=3) {
				_sound->playSound(3);
				step = 3;
			}
		}

		g_system->delayMillis(10);
		_events->pollEvents();
	}

	_events->_vbCount = 200;
	while (!shouldQuit() && _events->_vbCount > 0) {
		_events->pollEvents();
		g_system->delayMillis(10);
	}
	_events->showCursor();
	_sound->newMusic(11, 1);
	_sound->_soundTable.clear();

	establishCenter(0, 4);
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

	// Load death list
	_deaths.resize(58);
	for (int i = 0; i < 58; ++i) {
		_deaths[i]._screenId = DEATH_SCREENS[i];
		_deaths[i]._msg = DEATH_TEXT[i];
	}
	_deaths._cells.resize(12);
	for (int i = 0; i < 12; ++i)
		_deaths._cells[i] = CellIdent(DEATH_CELLS[i][0], DEATH_CELLS[i][1], DEATH_CELLS[i][2]);

	// Miscellaneous
	_fonts._font1.load(FONT6x6_INDEX, FONT6x6_DATA);
	_fonts._font2.load(FONT2_INDEX, FONT2_DATA);

	// Set player room and position
	_player->_roomNumber = 4;
	_player->_playerX = _player->_rawPlayer.x = TRAVEL_POS[_player->_roomNumber][0];
	_player->_playerY = _player->_rawPlayer.y = TRAVEL_POS[_player->_roomNumber][1];
}

void AmazonEngine::establish(int screenId, int esatabIndex) {
	_establishMode = 0;
	_establishGroup = 0;
	doEstablish(screenId, esatabIndex);
}

void AmazonEngine::establishCenter(int screenId, int esatabIndex) {
	_establishMode = 1;
	doEstablish(screenId, esatabIndex);
}

const char *const _estTable[] = { "ETEXT0.DAT", "ETEXT1.DAT", "ETEXT2.DAT", "ETEXT3.DAT" };

void AmazonEngine::loadEstablish(int estabIndex) {
	if (!_files->existFile("ETEXT.DAT")) {
		int oldGroup = _establishGroup;
		_establishGroup = 0;

		_eseg = _files->loadFile(_estTable[oldGroup]);
		_establishCtrlTblOfs = READ_LE_UINT16(_eseg->data());

		int ofs = _establishCtrlTblOfs + (estabIndex * 2);
		int idx = READ_LE_UINT16(_eseg->data() + ofs);
		_narateFile = READ_LE_UINT16(_eseg->data() + idx);
		_txtPages = READ_LE_UINT16(_eseg->data() + idx + 2);

		if (!_txtPages)
			return;

		_sndSubFile = READ_LE_UINT16(_eseg->data() + idx + 4);
		for (int i = 0; i < _txtPages; ++i)
			_countTbl[i] = READ_LE_UINT16(_eseg->data() + idx + 6 + (2 * i));
	} else {
		_establishGroup = 0;
		_narateFile = 0;
		_txtPages = 0;
		_sndSubFile = 0;
		_eseg = _files->loadFile("ETEXT.DAT");
	}
}

void AmazonEngine::doEstablish(int screenId, int estabIndex) {
	_establishMode = 1;

	_screen->forceFadeOut();
	_screen->clearScreen();
	_screen->setPanel(3);

	if (screenId != -1) {
		_files->loadScreen(95, screenId);
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
	loadEstablish(estabIndex);
	_et = estabIndex;
	uint16 msgOffset = READ_LE_UINT16(_eseg->data() + (estabIndex * 2) + 2);

	_printEnd = 155;
	Common::String msg((const char *)_eseg->data() + msgOffset);

	if (_txtPages == 0) {
		printText(_screen, msg);
	} else {
		speakText(_screen, msg);
	}

	_screen->forceFadeOut();
	_screen->clearScreen();

	delete _eseg;
	_eseg = nullptr;

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

	if (!_clearSummaryFlag && (_oldTitleChapter == _chapter))
		return;

	_oldTitleChapter = _chapter;
	int idx = _chapter - 1;

	if (!_files->existFile(_tileFiles[idx]))
		return;

	Resource *res = _files->loadFile(_tileFiles[idx]);
	int x = res->_stream->readSint16LE();
	int y = res->_stream->readSint16LE();
	int size = ((x + 2) * y) + 10;
	
	for (int i = 0; i < size; ++i)
		_tileData[i] = res->_stream->readByte();

	// CHECKME: Depending on the Vesa mode during initialization, 400 or 480
	for (_tilePos.y = 0; _tilePos.y < 480; _tilePos.y += y) {
		for (_tilePos.x = 0; _tilePos.x < 640; _tilePos.x += x)
			warning("TODO: DRAWOBJECT");
	}

	delete res;
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

void AmazonEngine::HELPTITLE() {
	warning("TODO: HELPTITLE");
}

void AmazonEngine::drawHelpText(const Common::String &msg) {
	_screen->_maxChars = 39;
	_screen->_printOrg = Common::Point(26, 58);
	_screen->_printStart = Common::Point(26, 58);

	Common::String lines = msg;
	Common::String line;
	int width = 0;
	bool lastLine = false;
	do {
		lastLine = _fonts._font2.getLine(lines, _screen->_maxChars * 6, line, width);

		// Set font colors
		_fonts._font2._fontColors[0] = 0;
		_fonts._font2._fontColors[1] = 27;
		_fonts._font2._fontColors[2] = 28;
		_fonts._font2._fontColors[3] = 29;

		_fonts._font2.drawString(_screen, line, _screen->_printOrg);
		_screen->_printOrg = Common::Point(_screen->_printStart.x, _screen->_printOrg.y + 8);
	} while (!lastLine);

	_events->showCursor();
}

void AmazonEngine::drawHelp() {
	_events->hideCursor();
	if (_useItem == 0) {
		_buffer2.copyBuffer(_screen);
		if (_screen->_vesaMode) {
			_screen->setPanel(2);
			_screen->saveScreen();
		}
		_screen->setPalette();
		_screen->fadeOut();
		_screen->clearBuffer();
		if (_moreHelp == 1) {
			// Set cells
			Common::Array<CellIdent> cells;
			cells.push_back(CellIdent(95, 95, 3));
			loadCells(cells);
		}
	}

	_files->loadScreen(95, 2);
	if (_moreHelp == 1) {
		ASurface *oldDest = _destIn;
		_destIn = _screen;
		int oldClip = _screen->_clipHeight;
		_screen->_clipHeight = 200;
		_screen->plotImage(_objectsTable[95], 0, Common::Point(76, 168));
		_destIn = oldDest;
		_screen->_clipHeight = oldClip;		
	}

	if ((_useItem == 0) && (_screen->_vesaMode == 0))
		_screen->fadeIn();

	HELPTITLE();
	drawHelpText("TODO: WHICH BUFFER?");
}

void AmazonEngine::startChapter(int chapter) {
	_chapter = chapter;
	assert(_chapter <= 14);

	if (chapter != 1) {
		_room->clearRoom();
		freeChar();
		
		_sound->newMusic(32, 0);
		playVideo(0, Common::Point());
		if (shouldQuit())
			return;

		_events->debounceLeft();
		_events->zeroKeys();
		playVideo(_chapter, Common::Point(4, 113));
		if (shouldQuit())
			return;

		_timers[20]._timer = 500;
		_timers[20]._initTm = 500;
		_timers[20]._flag++;
		_sound->freeSounds();

		_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(115, 0), 1));
		_sound->playSound(0);
		_sound->freeSounds();

		_sound->_soundTable.push_back(SoundEntry(_sound->loadSound(115, 1), 1));
		_sound->playSound(0);
		_sound->freeSounds();

		// Wait loop
		while (!shouldQuit() && !_events->_leftButton && !_events->_rightButton
				&& _events->_keypresses.size() == 0 && _timers[20]._flag) {
			_events->pollEvents();
			g_system->delayMillis(10);
		}
	}

	_screen->forceFadeOut();
	_events->debounceLeft();
	_events->zeroKeys();
	_screen->clearScreen();

	_screen->setPanel(3);
	
	// Set up cells for the chapter display
	Common::Array<CellIdent> chapterCells;
	chapterCells.push_back(CellIdent(0, 96, 17));
	const int *chapCell = &CHAPTER_CELLS[_chapter - 1][0];
	chapterCells.push_back(CellIdent(chapCell[0], chapCell[1], chapCell[2]));
	loadCells(chapterCells);

	// Show chapter screen
	_files->loadScreen(96, 15);
	_buffer2.copyFrom(*_screen);

	const int *chapImg = &CHAPTER_TABLE[_chapter - 1][0];
	_screen->plotImage(_objectsTable[0], _chapter - 1, 
		Common::Point(chapImg[1], chapImg[2]));
	_screen->plotImage(_objectsTable[_chapter], 0,
		Common::Point(chapImg[3], chapImg[4]));
	if (chapter == 14)
		_screen->plotImage(_objectsTable[_chapter], 1, Common::Point(169, 76));

	_sound->newMusic(chapImg[4], 1);
	_sound->newMusic(33, 0);
	_screen->forceFadeIn();

	_timers[20]._timer = 950;
	_timers[20]._initTm = 950;
	_timers[20]._flag++;

	// Wait loop
	while (!shouldQuit() && !_events->_leftButton && !_events->_rightButton
		&& _events->_keypresses.size() == 0 && _timers[20]._flag) {
		_events->pollEvents();
		g_system->delayMillis(10);
	}
	if (shouldQuit())
		return;

	_screen->forceFadeOut();
	_events->debounceLeft();
	_events->zeroKeys();
	
	_screen->clearBuffer();
	_files->loadScreen(96, 16);
	_buffer2.copyFrom(*_screen);
	_screen->plotImage(_objectsTable[0], chapImg[0], Common::Point(90, 7));

	_sound->newMusic(7, 1);
	_sound->newMusic(34, 0);

	_screen->forceFadeIn();
	_buffer2.copyFrom(*_screen);

	_fonts._charSet._lo = 1;
	_fonts._charSet._hi = 10;
	_fonts._charFor._lo = 55;
	_fonts._charFor._hi = 0xFF;
	_screen->_maxChars = 43;
	_screen->_printOrg = Common::Point(31, 77);
	_screen->_printStart = Common::Point(31, 77);

	_establishGroup = 1;
	loadEstablish(0x40 + _chapter);
	uint16 msgOffset = READ_LE_UINT16(_eseg->data() + ((0x40 + _chapter) * 2) + 2);
	_printEnd = 170;

	Common::String msg((const char *)_eseg->data() + msgOffset);

	if (_txtPages == 0) {
		printText(_screen, msg);
	} else {
		speakText(_screen, msg);
	}
	if (shouldQuit())
		return;

	_screen->forceFadeOut();
	_screen->clearBuffer();
	freeCells();
	
	_sound->newMusic(_chapter * 2, 1);

	if (chapter != 1 && chapter != 14) {
		_room->init4Quads();
	}

	if (chapter == 14) {
		_conversation = 31;
		_char->loadChar(_conversation);
		_events->setCursor(CURSOR_ARROW);

		_images.clear();
		_oldRects.clear();
		_scripts->_sequence = 0;
		_scripts->searchForSequence();

		if (_screen->_vesaMode) {
			_converseMode = 1;
		}
	} else if (chapter != 1) {
		_player->_roomNumber = CHAPTER_JUMP[_chapter - 1];
		_room->_function = 1;
		_converseMode = 0;

		_scripts->cmdRetPos();
	}
}

void AmazonEngine::synchronize(Common::Serializer &s) {
	AccessEngine::synchronize(s);

	int dummy = 0;

	s.syncAsSint16LE(_canoeLane);
	s.syncAsSint16LE(_canoeYPos);
	s.syncAsSint16LE(_hitCount);
	s.syncAsSint16LE(_saveRiver);
	s.syncAsSint16LE(_hitSafe);
	s.syncAsSint16LE(_chapter);
	s.syncAsSint16LE(dummy);
	s.syncAsSint16LE(dummy);
	s.syncAsSint16LE(_riverIndex);
	s.syncAsSint16LE(_rawInactiveX);
	s.syncAsSint16LE(_rawInactiveY);
	s.syncAsSint16LE(_inactiveYOff);
	for (int i = 0; i < 100; ++i)
		s.syncAsSint16LE(_esTabTable[i]);

	warning("TODO: 	s.syncAsSint16LE(_topList);");
	warning("TODO: 	s.syncAsSint16LE(_botList);");
}

/*------------------------------------------------------------------------*/

Guard::Guard() {
	_guardCel = 0;
	_gCode1 = _gCode2 = 0;
	_xMid = _yMid = 0;
}

Plane::Plane() {
	_pCount = 0;
	_planeCount = 0;
	_propCount = 0;
	_xCount = 0;
}

} // End of namespace Amazon

} // End of namespace Access
