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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

AmazonEngine::AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc)
	: AccessEngine(syst, gameDesc), _guardLocation(_flags[122]), _guardFind(_flags[128]),
	  _helpLevel(_flags[167]), _jasMayaFlag(_flags[168]), _moreHelp(_flags[169]),
	  _flashbackFlag(_flags[171]), _riverFlag(_flags[185]), _aniOutFlag(_flags[195]),
	  _badEnd(_flags[218]), _noHints(_flags[219]), _aniFlag(_flags[229]),
	  _allenFlag(_flags[237]), _noSound(_flags[239]) {
	_ant = nullptr;
	_cast = nullptr;
	_guard = nullptr;
	_jungle = nullptr;
	_opening = nullptr;
	_plane = nullptr;
	_river = nullptr;

	_charSegSwitch = false;

	_oldTitleChapter = _chapter = 0;
	_updateChapter = -1;
	_rawInactiveX = 0;
	_rawInactiveY = 0;
	_inactiveYOff = 0;
	_hintLevel = 0;

	Common::fill(&_tileData[0], &_tileData[0] + sizeof(_tileData), 0);
	Common::fill(&_help1[0], &_help1[0] + sizeof(_help1), 0);
	Common::fill(&_help2[0], &_help2[0] + sizeof(_help2), 0);
	Common::fill(&_help3[0], &_help3[0] + sizeof(_help3), 0);
	_helpTbl[0] = _help1;
	_helpTbl[1] = _help2;
	_helpTbl[2] = _help3;

	_chapter = 0;
	_rawInactiveX = _rawInactiveY = 0;
	_inactiveYOff = 0;
	_hintLevel = 0;
	_updateChapter = 0;
	_oldTitleChapter = 0;
	_iqValue = 0;

	_chapterCells.push_back(CellIdent(0, 96, 17));
	_inactive._spritesPtr = nullptr;
	_inactive._flags = _inactive._frameNumber = _inactive._offsetY = 0;
	_inactive._position = Common::Point(0, 0);
}

AmazonEngine::~AmazonEngine() {
	delete _inactive._altSpritesPtr;

	delete _ant;
	delete _cast;
	delete _guard;
	delete _jungle;
	delete _opening;
	delete _plane;
	delete _river;
}

void AmazonEngine::freeInactivePlayer() {
	delete _inactive._altSpritesPtr;
	_inactive._altSpritesPtr = nullptr;
}

void AmazonEngine::configSelect() {
	// Initialize fields contained in the config file.
	_hintLevel = 3;
}

void AmazonEngine::initObjects() {
	_room = new AmazonRoom(this);
	_scripts = new AmazonScripts(this);

	_ant = new Ant(this);
	_cast = new Cast(this);
	_guard = new Guard(this);
	_jungle = new Jungle(this);
	_opening = new Opening(this);
	_plane = new Plane(this);
	_river = new River(this);
}

void AmazonEngine::playGame() {
	// Initialize Amazon game-specific objects
	initObjects();

	// Setup the game
	setupGame();
	configSelect();

	if (_loadSaveSlot == -1) {
		// Do introduction
		_opening->doIntroduction();
		if (shouldQuit())
			return;
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

void AmazonEngine::setupGame() {
	Amazon::AmazonResources &res = *((Amazon::AmazonResources *)_res);

	// Load death list
	_deaths.resize(_res->DEATHS.size());

	for (uint idx = 0; idx < _deaths.size(); ++idx) {
		_deaths[idx]._screenId = res.DEATHS[idx]._screenId;
		_deaths[idx]._msg = res.DEATHS[idx]._msg;
	}

	// Load the deaths cells
	_deaths._cells.resize(13);
	for (int i = 0; i < 13; ++i)
		_deaths._cells[i] = CellIdent(DEATH_CELLS[i][0], DEATH_CELLS[i][1], DEATH_CELLS[i][2]);

	// Miscellaneous
	_fonts.load(res._font6x6, res._font3x5);

	initVariables();
}

void AmazonEngine::initVariables() {
	_chapter = 1;
	// Set player room and position
	if (isDemo())
		_player->_roomNumber = 33;
	else
		_player->_roomNumber = 4;

	_converseMode = 0;
	_inventory->_startInvItem = 0;
	_inventory->_startInvBox = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[100], (SpriteResource *)nullptr);
	_player->_playerOff = false;

	// Setup timers
	const int TIMER_DEFAULTS[] = { 3, 10, 8, 1, 1, 1, 1, 2 };
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = te._timer = (i < 8) ? TIMER_DEFAULTS[i] : 1;
		te._flag = 1;

		_timers.push_back(te);
	}

	_player->_playerX = _player->_rawPlayer.x = _res->ROOMTBL[_player->_roomNumber]._travelPos.x;
	_player->_playerY = _player->_rawPlayer.y = _res->ROOMTBL[_player->_roomNumber]._travelPos.y;
	_room->_selectCommand = -1;
	_events->setNormalCursor(CURSOR_CROSSHAIRS);
	_mouseMode = 0;
	_numAnimTimers = 0;
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

		_establish = _files->loadFile(_estTable[oldGroup]);
		_establishCtrlTblOfs = READ_LE_UINT16(_establish->data());

		int ofs = _establishCtrlTblOfs + (estabIndex * 2);
		int idx = READ_LE_UINT16(_establish->data() + ofs);
		_narateFile = READ_LE_UINT16(_establish->data() + idx);
		_txtPages = READ_LE_UINT16(_establish->data() + idx + 2);

		if (!_txtPages)
			return;

		_sndSubFile = READ_LE_UINT16(_establish->data() + idx + 4);
		for (int i = 0; i < _txtPages; ++i)
			_countTbl[i] = READ_LE_UINT16(_establish->data() + idx + 6 + (2 * i));
	} else {
		_establishGroup = 0;
		_narateFile = 0;
		_txtPages = 0;
		_sndSubFile = 0;
		_establish = _files->loadFile("ETEXT.DAT");
	}
}

void AmazonEngine::doEstablish(int screenId, int estabIndex) {
	_establishMode = 1;

	_events->clearEvents();
	_screen->forceFadeOut();
	_screen->clearScreen();
	_screen->setPanel(3);

	if (screenId != -1) {
		_files->loadScreen(95, screenId);
		_buffer2.copyBuffer(_screen);
	}

	_screen->setIconPalette();
	_screen->forceFadeIn();

	if (getGameID() == GType_MartianMemorandum) {
		_fonts._charSet._lo = 1;
		_fonts._charSet._hi = 10;
		_fonts._charFor._lo = 0xF7;
		_fonts._charFor._hi = 0xFF;

		_screen->_maxChars = 50;
		_screen->_printOrg = _screen->_printStart = Common::Point(24, 18);
	} else {
		_fonts._charSet._lo = 1;
		_fonts._charSet._hi = 10;
		_fonts._charFor._lo = 29;
		_fonts._charFor._hi = 32;

		_screen->_maxChars = 37;
		_screen->_printOrg = _screen->_printStart = Common::Point(48, 35);
	}

	loadEstablish(estabIndex);
	uint16 msgOffset;
	if (!isCD())
		msgOffset = READ_LE_UINT16(_establish->data() + (estabIndex * 2));
	else
		msgOffset = READ_LE_UINT16(_establish->data() + (estabIndex * 2) + 2);

	_printEnd = 155;
	Common::String msg((const char *)_establish->data() + msgOffset);

	if ((_txtPages == 0) || !isCD()) {
		printText(_screen, msg);
	} else {
		speakText(_screen, msg);
	}

	_screen->forceFadeOut();
	_screen->clearScreen();

	delete _establish;
	_establish = nullptr;

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
	Common::Point tilePos;
	for (tilePos.y = 0; tilePos.y < 480; tilePos.y += y) {
		for (tilePos.x = 0; tilePos.x < 640; tilePos.x += x)
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
	AmazonResources &res = *(AmazonResources *)_res;
	int width = _fonts._font2->stringWidth(_bubbleBox->_bubbleTitle);
	int posX = 160 - (width / 2);
	_fonts._font2->_fontColors[0] = 0;
	_fonts._font2->_fontColors[1] = 33;
	_fonts._font2->_fontColors[2] = 34;
	_fonts._font2->_fontColors[3] = 35;
	_fonts._font2->drawString(_screen, _bubbleBox->_bubbleTitle, Common::Point(posX, 24));

	width = _fonts._font2->stringWidth(res.HELPLVLTXT[_helpLevel]);
	posX = 160 - (width / 2);
	_fonts._font2->_fontColors[0] = 0;
	_fonts._font2->_fontColors[1] = 10;
	_fonts._font2->_fontColors[2] = 11;
	_fonts._font2->_fontColors[3] = 12;
	_fonts._font2->drawString(_screen, res.HELPLVLTXT[_helpLevel], Common::Point(posX, 36));

	Common::String iqText = "IQ: ";
	calcIQ();
	Common::String scoreIQ = Common::String::format("%d", _iqValue);
	while (scoreIQ.size() < 4)
		scoreIQ = " " + scoreIQ;

	iqText += scoreIQ;
	int index = _iqValue;
	if (index == 170)
		index = 169;

	index /= 20;

	iqText += " ";
	iqText += res.IQLABELS[index];

	width = _fonts._font2->stringWidth(iqText);
	posX = 160 - (width / 2);
	_fonts._font2->_fontColors[0] = 0;
	_fonts._font2->_fontColors[1] = 10;
	_fonts._font2->_fontColors[2] = 11;
	_fonts._font2->_fontColors[3] = 12;
	_fonts._font2->drawString(_screen, iqText, Common::Point(posX, 44));
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
		lastLine = _fonts._font2->getLine(lines, _screen->_maxChars * 6, line, width);

		// Set font colors
		_fonts._font2->_fontColors[0] = 0;
		_fonts._font2->_fontColors[1] = 27;
		_fonts._font2->_fontColors[2] = 28;
		_fonts._font2->_fontColors[3] = 29;

		_fonts._font2->drawString(_screen, line, _screen->_printOrg);
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
		BaseSurface *oldDest = _destIn;
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

		_midi->newMusic(32, 0);
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

		if (isCD()) {
			_sound->loadSoundTable(0, 115, 0);
			_sound->loadSoundTable(1, 115, 1);
			_sound->playSound(0);
			_sound->playSound(1);

			_sound->freeSounds();
		}

		// Wait loop
		while (!shouldQuit() && !_events->isKeyMousePressed() && _timers[20]._flag) {
			_events->pollEventsAndWait();
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
	_buffer2.blitFrom(*_screen);

	const int *chapImg = &CHAPTER_TABLE[_chapter - 1][0];
	_screen->plotImage(_objectsTable[0], _chapter - 1,
		Common::Point(chapImg[1], chapImg[2]));
	_screen->plotImage(_objectsTable[_chapter], 0,
		Common::Point(chapImg[3], chapImg[4]));
	if (chapter == 14)
		_screen->plotImage(_objectsTable[_chapter], 1, Common::Point(169, 76));

	_midi->newMusic(chapImg[4], 1);
	_midi->newMusic(33, 0);
	_screen->forceFadeIn();

	_timers[20]._timer = 950;
	_timers[20]._initTm = 950;
	_timers[20]._flag++;

	// Wait loop
	while (!shouldQuit() && !_events->isKeyMousePressed() && _timers[20]._flag) {
		_events->pollEventsAndWait();
	}
	if (shouldQuit())
		return;

	_screen->forceFadeOut();
	_events->debounceLeft();
	_events->zeroKeys();

	_screen->clearBuffer();
	_files->loadScreen(96, 16);
	_buffer2.blitFrom(*_screen);
	_screen->plotImage(_objectsTable[0], chapImg[0], Common::Point(90, 7));

	_midi->newMusic(7, 1);
	_midi->newMusic(34, 0);

	_screen->forceFadeIn();
	_buffer2.blitFrom(*_screen);

	_fonts._charSet._lo = 1;
	_fonts._charSet._hi = 10;
	_fonts._charFor._lo = 55;
	_fonts._charFor._hi = 0xFF;
	_screen->_maxChars = 43;
	_screen->_printOrg = Common::Point(31, 77);
	_screen->_printStart = Common::Point(31, 77);

	_establishGroup = 1;
	loadEstablish(0x40 + _chapter);

	byte *entryOffset = _establish->data() + ((0x40 + _chapter) * 2);
	if (isCD())
		entryOffset += 2;

	uint16 msgOffset = READ_LE_UINT16(entryOffset);
	_printEnd = 170;

	Common::String msg((const char *)_establish->data() + msgOffset);

	if ((_txtPages == 0) || !isCD()) {
		printText(_screen, msg);
	} else {
		speakText(_screen, msg);
	}
	if (shouldQuit())
		return;

	_screen->forceFadeOut();
	_screen->clearBuffer();
	freeCells();

	_midi->newMusic(_chapter * 2, 1);

	if (chapter != 1 && chapter != 14) {
		_room->init4Quads();
	}

	if (isCD()) {
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
}


void AmazonEngine::dead(int deathId) {
	_events->hideCursor();
	_screen->forceFadeOut();
	_scripts->cmdFreeSound();
	_events->debounceLeft();
	_events->zeroKeys();

	_sound->_soundTable.push_back(SoundEntry(_files->loadFile(98, 44), 1));

	_screen->clearScreen();
	_screen->setPanel(3);

	if ((deathId == 10) && !isDemo()) {
		quitGame();
		_events->pollEvents();
		return;
	} else {
		if (!isDemo())
			_midi->newMusic(62, 0);
		_files->_setPaletteFlag = false;
		_files->loadScreen(94, 0);
		_files->_setPaletteFlag = true;
		_buffer2.blitFrom(*_screen);

		if (!isDemo() || deathId != 10) {
			for (int i = 0; i < 3; ++i) {
				_sound->playSound(0);
				_screen->forceFadeIn();
				_sound->playSound(0);
				_screen->forceFadeOut();

				_events->pollEvents();
				if (shouldQuit())
					return;
			}
		}

		if (!isDemo()) {
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

			_midi->newMusic(0, 1);
			_events->showCursor();
			_room->clearRoom();
			freeChar();

			_currentManOld = 1;
			_player->removeSprite1();

		} else {
			_files->loadScreen(_screen, 94, _deaths[deathId]._screenId);
			_screen->forceFadeIn();

			_fonts._charSet._hi = 10;
			_fonts._charSet._lo = 1;
			_fonts._charFor._lo = 55;
			_fonts._charFor._hi = 255;
			_screen->_maxChars = 49;
			_screen->_printOrg = Common::Point(15, 165);
			_screen->_printStart = Common::Point(15, 165);

			Common::String msg = Common::String(_deaths[deathId]._msg);
			_printEnd = 200;

			printText(_screen, msg);
			_screen->fadeOut();

			_events->showCursor();
			_room->clearRoom();
			freeChar();

			_currentManOld = 1;
			_player->removeSprite1();
		}

		// The original was jumping to the restart label in main
		_restartFl = true;
		_events->pollEvents();
	}
}

void AmazonEngine::synchronize(Common::Serializer &s) {
	AccessEngine::synchronize(s);

	s.syncAsSint16LE(_chapter);
	s.syncAsSint16LE(_rawInactiveX);
	s.syncAsSint16LE(_rawInactiveY);
	s.syncAsSint16LE(_inactiveYOff);

	for (int i = 0; i < 366; ++i) {
		s.syncAsByte(_help1[i]);
		s.syncAsByte(_help2[i]);
		s.syncAsByte(_help3[i]);
	}

	_river->synchronize(s);
	_ant->synchronize(s);
}

} // End of namespace Amazon

} // End of namespace Access
