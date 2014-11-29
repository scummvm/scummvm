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
		_noSound(_flags[239]),
		_ant(this), _cast(this), _guard(this), _jungle(this), _opening(this), 
		_plane(this), _river(this) {

	_charSegSwitch = false;

	_oldTitleChapter = _chapter = 0;
	_updateChapter = -1;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	_tilePos = Common::Point(0, 0);
	_hintLevel = 0;

	Common::fill(&_esTabTable[0], &_esTabTable[100], 0);
	memset(_tileData, 0, sizeof(_tileData));
	Common::fill(&_help1[0], &_help1[366], 0);
	Common::fill(&_help2[0], &_help2[366], 0);
	Common::fill(&_help1[0], &_help3[366], 0);
	_helpTbl[0] = _help1;
	_helpTbl[1] = _help2;
	_helpTbl[2] = _help3;

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

void AmazonEngine::configSelect() {
	// Initialize fields contained in the config file.
	_hintLevel = 3;
}

void AmazonEngine::playGame() {
	// Initialize Amazon game-specific objects
	_room = new AmazonRoom(this);
	_scripts = new AmazonScripts(this);

	// Setup the game
	setupGame();
	configSelect();

	if (_loadSaveSlot == -1) {
		// Do introduction
		_opening.doIntroduction();
		if (shouldQuit())
			return;
	}

	_screen->clearScreen();
	_screen->setPanel(0);
	_screen->forceFadeOut();
	_events->showCursor();

	initVariables();

	// If there's a pending savegame to load, load it
	if (_loadSaveSlot != -1)
		loadGameState(_loadSaveSlot);

	// Execute the room
	_room->doRoom();
}

void AmazonEngine::setupGame() {
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

	initVariables();
}

void AmazonEngine::initVariables() {
	_chapter = 1;
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

void AmazonEngine::calcIQ() {
	int tmpIQ = 170;
	for (int i = 0; i < 256; i++) {
		if (_help1[i] == 1)
			tmpIQ -= 3;
	}

	for (int i = 0; i < 256; i++) {
		if (_help2[i] == 1)
			tmpIQ -= 5;
	}

	for (int i = 0; i < 256; i++) {
		if (_help3[i] == 1)
			tmpIQ -= 10;
	}

	if (tmpIQ < 0)
		tmpIQ = 0;

	_iqValue = tmpIQ;

	if (_iqValue <= 100)
		_badEnd = 1;

	if (_iqValue <= 0)
		_noHints = 1;
}

void AmazonEngine::helpTitle() {
	int width = _fonts._font2.stringWidth(_bubbleBox->_bubbleTitle);
	int posX = 160 - (width / 2);
	_fonts._font2._fontColors[0] = 0;
	_fonts._font2._fontColors[1] = 33;
	_fonts._font2._fontColors[2] = 34;
	_fonts._font2._fontColors[3] = 35;
	_fonts._font2.drawString(_screen, _bubbleBox->_bubbleTitle, Common::Point(posX, 24));

	width = _fonts._font2.stringWidth(HELPLVLTXT[_helpLevel]);
	posX = 160 - (width / 2);
	_fonts._font2._fontColors[0] = 0;
	_fonts._font2._fontColors[1] = 10;
	_fonts._font2._fontColors[2] = 11;
	_fonts._font2._fontColors[3] = 12;
	_fonts._font2.drawString(_screen, HELPLVLTXT[_helpLevel], Common::Point(posX, 36));

	Common::String iqText = "IQ: ";
	calcIQ();
	Common::String IQSCORE = Common::String::format("%d", _iqValue);
	while (IQSCORE.size() != 4)
		IQSCORE = " " + IQSCORE;

	iqText += IQSCORE;
	int index = _iqValue;
	if (index == 170)
		index = 169;

	index /= 20;
	
	iqText += " ";
	iqText += IQLABELS[index];

	width = _fonts._font2.stringWidth(iqText);
	posX = 160 - (width / 2);
	_fonts._font2._fontColors[0] = 0;
	_fonts._font2._fontColors[1] = 10;
	_fonts._font2._fontColors[2] = 11;
	_fonts._font2._fontColors[3] = 12;
	_fonts._font2.drawString(_screen, iqText, Common::Point(posX, 44));
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

void AmazonEngine::drawHelp(const Common::String str) {
	_events->hideCursor();
	if (_useItem == 0) {
		_buffer2.copyBuffer(_screen);
		if (_screen->_vesaMode) {
			_screen->setPanel(2);
			_screen->saveScreen();
		}
		_screen->savePalette();
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

	helpTitle();
	drawHelpText(str);
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
		_room->_function = FN_CLEAR1;
		_converseMode = 0;

		_scripts->cmdRetPos();
	}
}


void AmazonEngine::dead(int deathId) {
	_events->hideCursor();
	_screen->forceFadeOut();
	_scripts->cmdFreeSound();

	_sound->_soundTable.push_back(SoundEntry(_files->loadFile(98, 44), 1));

	_screen->clearScreen();
	_screen->setPanel(3);

	if (deathId != 10) {
		_sound->newMusic(62, 0);
		_files->_setPaletteFlag = false;
		_files->loadScreen(94, 0);
		_files->_setPaletteFlag = true;
		_buffer2.copyFrom(*_screen);

		for (int i = 0; i < 3; ++i) {
			_sound->playSound(0);
			_screen->forceFadeIn();
			_sound->playSound(0);
			_screen->forceFadeOut();
		}
		freeCells();

		// Load the cell list for the death screen
		DeathEntry &de = _deaths[deathId];
		Common::Array<CellIdent> cells;
		cells.push_back(_deaths._cells[de._screenId]);
		loadCells(cells);

		_screen->setDisplayScan();
		_files->_setPaletteFlag = false;
		_files->loadScreen(&_buffer2, 94, 1);
		_screen->setIconPalette();

		_buffer2.plotImage(_objectsTable[0], 0, Common::Point(105, 25));
		_buffer2.copyTo(_screen);
		_screen->forceFadeIn();

		_fonts._charSet._hi = 10;
		_fonts._charSet._lo = 1;
		_fonts._charFor._lo = 55;
		_fonts._charFor._hi = 255;
		_screen->_maxChars = 46;
		_screen->_printOrg = Common::Point(20, 155);
		_screen->_printStart = Common::Point(20, 155);

		Common::String &msg = de._msg;
		_printEnd = 180;
		printText(_screen, msg);
		_screen->forceFadeOut();

		_sound->newMusic(0, 1);
		_events->showCursor();
		_room->clearRoom();
		freeChar();

		warning("TODO: restart game");
		quitGame();
		_events->pollEvents();
	} else {
		quitGame();
		_events->pollEvents();
	}
}

void AmazonEngine::synchronize(Common::Serializer &s) {
	AccessEngine::synchronize(s);

	s.syncAsSint16LE(_chapter);
	s.syncAsSint16LE(_rawInactiveX);
	s.syncAsSint16LE(_rawInactiveY);
	s.syncAsSint16LE(_inactiveYOff);

	for (int i = 0; i < 100; ++i)
		s.syncAsSint16LE(_esTabTable[i]);
	for (int i = 0; i < 366; ++i) {
		s.syncAsByte(_help1[i]);
		s.syncAsByte(_help2[i]);
		s.syncAsByte(_help3[i]);
	}

	_river.synchronize(s);
}

} // End of namespace Amazon

} // End of namespace Access
