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

#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_room.h"
#include "access/noctropolis/noctropolis_scripts.h"
#include "access/noctropolis/noctropolis_resources.h"
#include "access/noctropolis/noctropolis_comicviewer.h"
#include "image/png.h"
#include "graphics/color_quantizer.h"
#include "common/config-manager.h"

namespace Access {

namespace Noctropolis {


NoctropolisEngine::NoctropolisEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
AccessEngine(syst, gameDesc), _invScript(nullptr), _stil(nullptr), _loadFlag(false),
_travScrollX(0), _travScrollY(0), _travScrollCol(0), _travScrollRow(0)
{
}

NoctropolisEngine::~NoctropolisEngine() {
	delete _invScript;
}

void NoctropolisEngine::initObjects() {
	_room = new NoctropolisRoom(this);
	_scripts = new NoctropolisScripts(this);
	_invScript = new NoctropolisScripts(this);
	_stil = new Player(this);

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "comdata");
}

void NoctropolisEngine::setupGame() {
	_timers.clear();
	for (int i = 0; i < 32; ++i) {
		TimerEntry te;
		te._initTm = 1;
		te._timer = 1;
		te._flag = true;
		_timers.push_back(te);
	}
	_timers[1]._initTm = _timers[1]._timer = 10;
	_timers[2]._initTm = _timers[2]._timer = 8;
	_timers[3]._initTm = _timers[3]._timer = 4;
	_timers[4]._initTm = _timers[4]._timer = 2;
	_timers[8]._initTm = _timers[8]._timer = 10;
	_timers[25]._initTm = _timers[25]._timer = 3600;
}

void NoctropolisEngine::initVariables() {
	// Set player room and position
	// Room 2 for intro sequence, 3 for first interactive room.
	_player->_roomNumber = ConfMan.getBool("skip_intro") ? 3 : 2;

	_invScript->setScript(_files->loadRawFile("INVTEXT.AP"));

	_converseMode = 0;
	_inventory->_startInvItem = 0;
	_inventory->_startInvBox = 0;
	Common::fill(&_objectsTable[0], &_objectsTable[128], (SpriteResource *)nullptr);
	_player->_playerOff = false;
	debug("TODO: set player starting point - using first _TravManPos entry for now");
	_player->_playerX = _player->_rawPlayer.x = _player->_moveTo.x = 280;
	_player->_playerY = _player->_rawPlayer.y = _player->_moveTo.y = 390;
	_room->_selectCommand = -1;
	_events->setNormalCursor(CURSOR_ARROW);
	_mouseMode = 0;
	_animation->clearTimers();

	// This is Noct_InitTravel in the original
	_travScrollRow = 15;
	_travScrollCol = 0;
	_travScrollX = 0;
	_travScrollY = 0;
}

void NoctropolisEngine::playGame() {
	bool skipIntro = ConfMan.getBool("skip_intro");
	if (_loadSaveSlot == -1 && !skipIntro) {
		bool keepGoing = true;
		Common::CustomEventType action = kActionNone;
		doFlashLogo();
		if (shouldQuit())
			return;

		_events->getAction(action);
		keepGoing &= (action != kActionSkip);

		if (keepGoing)
			doPublisherLogo();
		if (shouldQuit())
			return;

		_events->getAction(action);
		keepGoing &= (action != kActionSkip);

		if (keepGoing)
			doIntro();
		if (shouldQuit())
			return;
	}

	do {
		_restartFl = false;
		_screen->clearScreen();
		_screen->setPanel(0);
		_screen->forceFadeOut();
		_events->centerMousePos();
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


Common::Error NoctropolisEngine::loadGameState(int slot) {
	_loadFlag = true;
	return AccessEngine::Engine::loadGameState(slot);
}


void NoctropolisEngine::doFlashLogo() {
	_events->hideCursor();

	// TODO: should be "DARK/FLASH.SCN".
	_files->loadScreen(Common::Path("FLASH.SCN"));
	_screen->fadeIn();
	if (shouldQuit())
		return;

	_events->_vbCount = 0x7e;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}
	if (shouldQuit())
		return;
	_screen->fadeOut();
}

void NoctropolisEngine::doPublisherLogo() {
	Common::File pngFile;
	// TODO: should be "DARK/nds.png".
	pngFile.open(Common::Path("nds.png"));

	// TODO: Original has a movie here instead of PNG, will need an update.
	// The version on GOG has PNG for both windows and mac.
	Image::PNGDecoder decoder;
	decoder.loadStream(pngFile);

	// Find the best 8-bit palette for this logo as the png is 24-bit and we're
	// not changing the output surface format for this one logo at the start!
	Graphics::ColorQuantizer quant(256);
	const Graphics::Surface *pngSurf = decoder.getSurface();
	// The image comes in a bit big too
	Graphics::Surface *scaledPng = pngSurf->scale(640, 360, true);
	Graphics::PixelFormat format = scaledPng->format;
	assert(format.bytesPerPixel == 3);
	for (int y = 0; y < scaledPng->h; y++) {
		for (int x = 0; x < scaledPng->w; x++) {
			byte r,g,b;
			format.colorToRGB(scaledPng->getPixel(x, y), r, g, b);
			quant.addColor(r, g, b);
		}
	}
	Graphics::Palette *bestPal = quant.getPalette();
	_screen->clearScreen();
	_screen->setRawPalette(*bestPal);
	_screen->setPalette();
	for (int y = 0; y < MIN(scaledPng->h, _screen->h); y++) {
		for (int x = 0; x < MIN(scaledPng->w, _screen->w); x++) {
			byte r,g,b;
			format.colorToRGB(scaledPng->getPixel(x, y), r, g, b);
			byte col = bestPal->findBestColor(r, g, b);
			_screen->setPixel(x, y, col);
		}
	}
	delete bestPal;
	_screen->fadeIn();

	_events->_vbCount = 0x7e;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}
	if (shouldQuit())
		return;
	_screen->fadeOut();
}

void NoctropolisEngine::doIntro() {
	static const int lettersX[] = {106, 153, 197, 229, 271, 309, 357, 394, 443, 476, 504};
	static const int titlesSpriteX[] = {238, 237, 237, 200, 200, 216, 231, 207, 198, 201, 234, 200, 220, 235, 214};
	static const int titlesSpriteY[] = {108, 112, 125, 128, 128, 124, 128, 115, 131, 130, 102, 112, 112, 102, 112};

	int16 skylineSpriteOfsX1 = 740;
	int16 skylineSpriteOfsX2 = 1050;
	int16 skylineSpriteOfsX3 = 1250;
	int16 skylineSpriteOfsX4 = 1300;
	int16 skylineSpriteOfsX5 = 1500;
	int16 titlesSpriteIndex = 0;
	int lettersMax = 0;

	_timers[26]._initTm = 45;
	_timers[27]._initTm = 7;
	_timers[28]._initTm = 240;

	_screen->clearScreen();
	_screen->forceFadeOut();

	_midi->loadMusic(98, 1);
	_midi->midiPlay();
	_room->loadPlayField(1, 0);
	_buffer2.copyFrom(*_screen);
	_buffer1.copyFrom(*_screen);

	Resource *spriteRes = _files->loadFile(1, 1);
	SpriteResource *sprites = new SpriteResource(this, spriteRes);
	delete spriteRes;

	// TODO: Check these fades
	_screen->setPalette();
	((NoctropolisRoom *)_room)->buildScreenXScroll();
	copyBF2Vid();
	_screen->fadeIn();

	_timers[26].reset();
	_timers[27].reset();

	while (!shouldQuit() && !_events->isKeyActionMousePressed()) {

		if (!_timers[27].isActive()) {
			_timers[27].reset();
			if (_screen->_vWindowWidth + _scrollCol == _room->_playFieldWidth || _events->_leftButton || _events->_rightButton)
				break;

			_scrollX += 2;

			if (_scrollX >= TILE_WIDTH) {
				_scrollX -= TILE_WIDTH;
				++_scrollCol;
			}

			((NoctropolisRoom *)_room)->buildScreenXScroll();

			// Don't use copyBF1BF2() here as we want to do sub-tile scrolling.
			_buffer2.copyFrom(_buffer1);

			for (int i = 0; i < lettersMax; i++) {
				_buffer2.plotImage(sprites, i + 8, Common::Point(lettersX[i], 40));
			}

			if (!_timers[26].isActive()) {
				if (lettersMax <= 10) {
					if (lettersMax == 9)
						_timers[28].reset();
					_timers[26].reset();
					lettersMax++;
				} else if (titlesSpriteIndex < ARRAYSIZE(titlesSpriteX)) {
					_buffer2.plotImage(sprites, titlesSpriteIndex + 19, Common::Point(titlesSpriteX[titlesSpriteIndex], titlesSpriteY[titlesSpriteIndex]));
					if (!_timers[28].isActive()) {
						titlesSpriteIndex++;
						_timers[26].reset();
						_timers[28].reset();
					}
				}

			}

			if (skylineSpriteOfsX1 > -100) {
				_buffer2.plotImage(sprites, 0, Common::Point(skylineSpriteOfsX1, 60));
				skylineSpriteOfsX1 -= 5;
			}
			if (skylineSpriteOfsX2 > -100) {
				_buffer2.plotImage(sprites, 3, Common::Point(skylineSpriteOfsX2, 16));
				skylineSpriteOfsX2 -= 8;
			}
			if (skylineSpriteOfsX4 > -100) {
				_buffer2.plotImage(sprites, 1, Common::Point(skylineSpriteOfsX4, 145));
				skylineSpriteOfsX4 -= 7;
			}
			if (skylineSpriteOfsX3 > -100) {
				_buffer2.plotImage(sprites, 4, Common::Point(skylineSpriteOfsX3, 115));
				skylineSpriteOfsX3 -= 5;
			}
			if (skylineSpriteOfsX5 > -100) {
				_buffer2.plotImage(sprites, 2, Common::Point(skylineSpriteOfsX5, 126));
				skylineSpriteOfsX5 -= 7;
			}

			copyBF2Vid();

		}

		_events->pollEventsAndWait();
	}

	_screen->fadeOut();

	delete sprites;
}

void NoctropolisEngine::doTravel() {
	_events->setCursor(CURSOR_ARROW);
	_player->_playerOff = true;
	_stil->_playerOff = true;

	// Original starts music and video fade at the same time.  our fadeOut is
	// synchronous so start the music fade first.
	_midi->startMusicFade();
	_screen->fadeOut();

	// This section is "state 0" in NoctTravelEngine::ticker
	_room->clearRoom();
	_midi->loadMusic(98, 0x1b);
	_midi->midiRepeat();
	_midi->midiPlay();

	//loadPlayField(0, 0); //??
	_screen->setIconPalette();
	_scrollRow = _travScrollRow;
	_scrollCol = _travScrollCol;
	_scrollX = _travScrollX;
	_scrollY = _travScrollY;
	Resource *spriteData = _files->loadFile(0, 1);
	_objectsTable[0] = new SpriteResource(this, spriteData);
	delete spriteData;

	// This section is "state 1" in NoctTravelEngine::ticker
	_screen->setPaletteCycle(0xb5, 0xbe, 5);
	_screen->fadeIn();

	Common::Point rawMouse;
	int locFlag = -1;
	// Show the map and wait for clicks
	while (!shouldQuitOrRestart()) {
		_midi->midiRepeat();

		// TODO: check me.. is buildScreen equivalent??
		//NoctPF_RenderPlayfield();
		_room->buildScreen();

		_player->calcPlayer();

		for (int i = 0; i < 15; i++) {
			if (_travel[i]) {
				int imgNum = TRAV_ICONS[i * 3];
				int x = TRAV_ICONS[i * 3 + 1];
				int y = TRAV_ICONS[i * 3 + 2];
				Common::Point pt(x - _screen->_bufferStart.x, y - _screen->_bufferStart.y);
				_buffer2.plotImage(_objectsTable[0], imgNum, pt);
			}
		}
		copyBF2Vid();

		_screen->cyclePaletteForward();
		locFlag = -1;
		rawMouse = _events->calcRawMouse();
		int boxResult = _room->checkBoxes1(rawMouse);
		if (boxResult != -1 && _travel[boxResult])
			locFlag = boxResult;

		if (!_events->_leftButton || locFlag == -1) {
			int scrolly = _scrollY;

			// TODO: Clean up these if()s a bit
			if (rawMouse.y < 33) {
				if (_scrollRow != 0 && (scrolly = _scrollY + -8, (_scrollY + -8) < 0)) {
					int row = _scrollRow + -1;
					scrolly = _scrollY + 8;
					_scrollRow = 0;
					if (-1 < row)
						_scrollRow = row;
				}
			} else if (367 < rawMouse.y &&
						(_screen->_vWindowHeight + _scrollRow != _room->_playFieldHeight) &&
					   (scrolly = _scrollY + 8, 0xf < (_scrollY + 8))) {
				scrolly = _scrollY + -8;
				_scrollRow = _scrollRow + 1;
			}
		}

		_events->pollEventsAndWait();
	}

	// This is NoctTravelEngine::done
	_screen->fadeOut();
	_travScrollRow = _scrollRow;
	_travScrollCol = _scrollCol;
	_travScrollX = _scrollX;
	_travScrollY = _scrollY;
	assert(locFlag < 15);
	_player->_roomNumber = TRAV_ROOMS[locFlag];
	_player->_rawPlayer.x = TRAV_MAN_POS[locFlag * 2];
	_player->_rawPlayer.y = TRAV_MAN_POS[locFlag * 2 + 1];
	_player->_moveTo = _player->_rawPlayer;
	setStilettoPos();
	_room->clearRoom();
}

static const short StilYDirOff[] = {
	10, 10, 10, 10, 5, 5, 10, 10, 10,
};

void NoctropolisEngine::setStilettoPos() {
	_stil->_playerDirection = _player->_playerDirection;
	int dir = _player->_playerDirection;
	_stil->_moveTo = _player->_rawPlayer;
	// The original has a table for X index into the scale table here, but it
	// in practice evaluates to 30.  There is also some weird double-negation.
	if (dir < 5) {
		_stil->_moveTo.x -= _screen->_scaleTable1[30];
		_stil->_moveTo.y -= _screen->_scaleTable1[StilYDirOff[dir]];
	} else if (dir == 5) {
		_stil->_moveTo.x += _screen->_scaleTable1[30];
		_stil->_moveTo.y -= _screen->_scaleTable1[StilYDirOff[dir]];
	} else {
		_stil->_moveTo.x += _screen->_scaleTable1[30];
		_stil->_moveTo.y += _screen->_scaleTable1[StilYDirOff[dir]];
	}

	_stil->_rawPlayer = _stil->_moveTo;
}


void NoctropolisEngine::dead(int deathType) {
	// aka DeadMeat1
	/*
	static const struct { int16 x, y; } deathVideoPos[] = {
		{167, 21},
		{161, 22},
		{186, 29},
		{180, 29},
		{194, 37},
		{215, 93},
		{215, 93},
		{227, 0}
	};

	int16 deathTextX = deathType == 8 ? 80 : 130;
	int16 deathTextY = deathType == 8 ? 310 : 220;
	const char *deathScreenBackground =
		(deathType == 6 || deathType == 12 || deathType == 7 || deathType == 8)
		? "death1.scn" : "death.scn";
	const char *deathTextCaption =
		(deathType == 6 || deathType == 12 || deathType == 7)
		? "IMPRISONED" : "DEAD";
	int videoIndex;

	NoctropolisDeathResource *deathTable = new NoctropolisDeathResource();
	StringResource *deathVideoFilenames = new StringResource();

	_res->load(deathTable, GID_NOCTROPOLIS, kResDeathTable, 0);
	_res->load(deathVideoFilenames, GID_NOCTROPOLIS, kResStringTable, 5);

	videoIndex = deathTable->getVideoIndex(deathType);

	// TODO: Stop current music and play death music
	_screen->fadeOut();

	_vgaScreen->lock();
	showScreen(deathScreenBackground);
	drawTextBox(deathTextX, deathTextY, kTextBoxPlain | kTextBoxCaption,
		deathTable->getText(deathType), deathTextCaption);
	_vgaScreen->unlock();

	copySystemPalette();
	fadeToPalette();

	playVideo((const char*)deathVideoFilenames->getString(videoIndex),
		deathVideoPos[videoIndex].x, deathVideoPos[videoIndex].y, false, false);

	waitUntilAnyButtonIsClicked();
	restoreTextBoxRect();

	delete deathTable;
	delete deathVideoFilenames;
	*/
	// TODO: Restart game or something
	error("TODO: Finish implementing NoctropolisEngine::dead(%d)", deathType);
}

void NoctropolisEngine::initMinigame() {
	static const byte minigameInitialConfig[9] = {2, 8, 9, 5, 6, 4, 1, 7, 3};
	memcpy(_minigameCurrentConfig, minigameInitialConfig, 9);
	_flags[91] = 0;
}

void NoctropolisEngine::displayPegsTick() {
	static const byte redSol[3] = {9, 6, 3};
	static const byte greenSol[3] = {2, 5, 8};
	static const byte blueSol[3] = {1, 4, 7};
	static const struct { int16 x, y; } pegPos[10] = {
		{255, 201}, {143, 211},
		{27, 57}, {249, 56},
		{141, 174}, {67, 75},
		{210, 75}, {140, 137},
		{107, 94}, {170, 94}
	};

	// Check if the game is solved
	if (_flags[91])
		return;

	if (minigameIsSolved(0, redSol) &&
		minigameIsSolved(3, greenSol) &&
		minigameIsSolved(6, blueSol)) {
		debug("Minigame solved!");
		_flags[91] = 1;
		return;
	}

	for (uint i = 0; i < 9; i++) {
		ImageEntry ie;
		ie._spritesPtr = _objectsTable[65];
		ie._frameNumber = i / 3;
		ie._position.x = pegPos[_minigameCurrentConfig[i]].x + 181 - 4;
		ie._position.y = pegPos[_minigameCurrentConfig[i]].y + 116 - 3;
		ie._offsetY = 7;
		//spriteDrawItem.flags = 8; // Why? Scaling is 0! CHECKME
		//spriteDrawItem.scaling;
		_images.addToList(ie);
	}
}

void NoctropolisEngine::movePeg(int16 slot) {
	static const struct { int16 a, b; } pegPath[] = {
		{0, 1}, {1, 2}, {1, 5}, {1, 3}, {2, 3},
		{2, 6}, {3, 4}, {4, 5}, {4, 8}, {4, 6},
		{5, 6}, {5, 9}, {6, 7}, {7, 8}, {8, 9},
		{9, 7}, {0xFF, 0xFF}
	};

	for (uint i = 0; i < 9; i++) {
		if (_minigameCurrentConfig[i] == slot) {
			for (uint mapIndex = 0; pegPath[mapIndex].a != 0xFF; mapIndex++) {
				if (pegPath[mapIndex].a == slot || pegPath[mapIndex].b == slot) {
					int16 a = (pegPath[mapIndex].a == slot) ? pegPath[mapIndex].b : pegPath[mapIndex].a;
					uint index = 9;
					bool done = false;
					for (uint j = 0; !done && _minigameCurrentConfig[j] != a; j++) {
						if (--index == 0) {
							_minigameCurrentConfig[i] = a;
							_sound->playSound(1);
							done = true;
						}
					}
				}
			}
			break;
		}
	}

#if 0
	debug("%d %d %d", _minigameCurrentConfig[0], _minigameCurrentConfig[1], _minigameCurrentConfig[2]);
	debug("%d %d %d", _minigameCurrentConfig[3], _minigameCurrentConfig[4], _minigameCurrentConfig[5]);
	debug("%d %d %d", _minigameCurrentConfig[6], _minigameCurrentConfig[7], _minigameCurrentConfig[8]);
	debug("-------------------");
#endif

}

bool NoctropolisEngine::minigameIsSolved(uint rowIndex, const byte *finalRow) {
	// Test if the colors in this 'row' match
	return
			(_minigameCurrentConfig[rowIndex] == finalRow[0] ||
			_minigameCurrentConfig[rowIndex + 1] == finalRow[0] ||
			_minigameCurrentConfig[rowIndex + 2] == finalRow[0]) &&
			(_minigameCurrentConfig[rowIndex] == finalRow[1] ||
			_minigameCurrentConfig[rowIndex + 1] == finalRow[1] ||
			_minigameCurrentConfig[rowIndex + 2] == finalRow[1]) &&
			(_minigameCurrentConfig[rowIndex] == finalRow[2] ||
			_minigameCurrentConfig[rowIndex + 1] == finalRow[2] ||
			_minigameCurrentConfig[rowIndex + 2] == finalRow[2]);
}

void NoctropolisEngine::playStilMorph() {
	_midi->stopSong();
	_system->showMouse(false);
	_midi->loadMusic(98, 4);
	_midi->midiPlay();
	_screen->fadeOut();
	_screen->clearScreen();
	warning("TODO: Implement Noctropolis type video player");
	//playVideo("VID1\\DRLM00.VID", 118, 118, true, true, true);
}

void NoctropolisEngine::flashPaletteEffect() {
	_screen->flashPalette(255);
	_screen->flashPalette(64);
	_screen->flashPalette(16);
}

void NoctropolisEngine::shotoMeanwhile() {
	_midi->stopSong();
	_system->showMouse(false);
	_midi->loadMusic(98, 2);
	_midi->midiPlay();
	_screen->fadeOut();
	_screen->clearScreen();
	// TODO: Check these colors
	Font::_fontColors[0] = 0;
	Font::_fontColors[3] = 244;
	const char *meanwhileTxt = ((NoctropolisResources *)_res)->getMeanwhileMessage();
	_fonts._fonts[3]->drawString(_screen, meanwhileTxt, Common::Point(100, 200));
	_screen->fadeIn();
	_system->updateScreen();
	_system->delayMillis(180 * 17);
	_screen->fadeOut();
	_screen->clearScreen();
	_screen->_printOrg = _screen->_printStart = Common::Point(300, 300);
	_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
	_bubbleBox->_bubbleTitle = ((NoctropolisResources *)_res)->getShotoTitle();
	_bubbleBox->placeBubble(((NoctropolisResources *)_res)->getShotoText());
	_events->waitKeyActionMouse();
	warning("TODO: Implement Noctropolis type video player");
	//playVideo("VID1\\B126MEAN.VID", 120, 30, false, true, true);
	_midi->stopSong();
	_screen->fadeOut();
	_system->showMouse(true);
}

void NoctropolisEngine::makeVidPaletteCurrent() {
	warning("TODO: Implement Noctropolis type video player");
	//copyPaletteRange(_video->getPalette(), 0, 256);
	_screen->setPalette();
}

void NoctropolisEngine::showComicCover() {
	_midi->stopSong();
	_midi->loadMusic(98, 1);
	_midi->midiPlay();
	_files->loadScreen(Common::Path("comic93.scn"));
	_screen->setPalette();
	_system->updateScreen();
	_system->delayMillis(300 * 17);
	_midi->stopSong();
	_screen->fadeOut();
	_screen->clearScreen();
}

void NoctropolisEngine::playSuccubusSplit() {
	_midi->stopSong();
	_midi->loadMusic(98, 25);
	_midi->midiPlay();
	_screen->clearScreen();
	warning("TODO: Implement Noctropolis type video player");
	//playVideo("VID1\\SUCSPLT1.VID", 116, 2, false, true);
	_midi->stopSong();
	_screen->fadeOut();
}

void NoctropolisEngine::playSuccubusAttack() {
	_midi->stopSong();
	_midi->loadMusic(98, 4);
	_midi->midiPlay();
	_screen->fadeOut();
	_files->loadScreen(Common::Path("scene06.scn")); // "\\DARK\\SCENE06.SCN"
	_screen->fadeIn();
	_system->updateScreen();
	_system->delayMillis(60 * 17);
	warning("TODO: Implement Noctropolis type video player");
	//playVideo("VID1\\SUCATT1.VID", 118, 118, false, false);
	_screen->clearScreen();
	warning("TODO: Implement Noctropolis type video player");
	//playVideo("VID1\\SUCATT2.VID", 196, 96, false, false);
	_sound->playSound(1);
	_midi->stopSong();
	_screen->fadeOut();
}

void NoctropolisEngine::doLastComic() {
	_midi->stopSong();
	ComicViewer *viewer = new ComicViewer(this);
	const ComicResource *comic = ((NoctropolisResources *)_res)->getLastComicResource();
	viewer->run(comic);
	delete viewer;
	delete comic;
	_screen->fadeOut();
}

void NoctropolisEngine::doSpecialComic() {
	_midi->stopSong();
	ComicViewer *viewer = new ComicViewer(this);
	const ComicResource *comic = ((NoctropolisResources *)_res)->getSpecialComicResource();
	viewer->run(comic);
	delete viewer;
	delete comic;
	_screen->fadeOut();
}


} // end namespace Noctropolis

} // end namespace Access
