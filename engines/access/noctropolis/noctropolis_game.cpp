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
#include "access/noctropolis/noctropolis_player.h"
#include "access/noctropolis/noctropolis_scripts.h"
#include "access/noctropolis/noctropolis_resources.h"
#include "access/noctropolis/noctropolis_comicviewer.h"
#include "access/noctropolis/noctropolis_inventory.h"
#include "image/png.h"
#include "graphics/color_quantizer.h"
#include "common/config-manager.h"

namespace Access {

namespace Noctropolis {


NoctropolisEngine::NoctropolisEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
AccessEngine(syst, gameDesc), _invScript(nullptr), _stil(nullptr), _loadFlag(false),
_travScrollX(0), _travScrollY(0), _travScrollCol(0), _travScrollRow(0), _stilFlag1(_flags[205]),
_stilFlag2(_flags[234])
{
}

NoctropolisEngine::~NoctropolisEngine() {
	delete _invScript;
}

void NoctropolisEngine::initObjects() {
	_room = new NoctropolisRoom(this);
	_scripts = new NoctropolisScripts(this);
	_invScript = new NoctropolisScripts(this);
	_inventory = new NoctropolisInventory(this);
	_video = new VideoPlayer_v2(this);
	_stil = new NoctropolisPlayer(this, true);

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	// Current defaults to screen in Noctropolis.
	_current = _screen;
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
	debug("TODO: set player starting point? using first _TravManPos entry for now");
	_player->_playerX = _player->_rawPlayer.x = _player->_moveTo.x = 280;
	_player->_playerY = _player->_rawPlayer.y = _player->_moveTo.y = 390;
	_room->_selectCommand = -1;
	_events->setNormalCursor(CURSOR_ARROW);
	_mouseMode = 0;
	_animation->clearTimers();

	initMinigame();
	ARRAYCLEAR(_flags);
	ARRAYCLEAR(_ask);
	for (auto &entry : _inventory->_inv) {
		entry._value = ITEM_NOT_FOUND;
	}

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

void NoctropolisEngine::drawOverlays() {
	AccessEngine::drawOverlays();

	drawDeathTimer();
}

void NoctropolisEngine::drawDeathTimer() {
	if (!_flags[200])
		return;

	int seconds = _timers[0x12]._timer / 60;
	Common::String time = Common::String::format("%02d:%02d", seconds / 60, seconds % 60);
	_screen->fillRect(Common::Rect(0, 384, 47, 399), 0xf6);
	const Font *font = _fonts.getFont(4);
	Font::_fontColors[0] = 0;
	Font::_fontColors[1] = 0xf4;
	font->drawString(_screen, time, Common::Point(2, 384));
}

Common::Error NoctropolisEngine::loadGameState(int slot) {
	_loadFlag = true;
	return AccessEngine::loadGameState(slot);
}


void NoctropolisEngine::doFlashLogo() {
	_events->hideCursor();

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
	const Common::Path nightDive("DARK/nds.png");

	if (pngFile.exists(nightDive)) {
		// The nightdive re-release, show their logo.
		pngFile.open(nightDive);

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

	} else {
		// The original EA release, show their logo.
		//
		// Interestingly, these files are availble even in the re-release.
		// It would be possible to expose as an option, if anyone
		// was particularly keen on restoring this publisher logo..
		//
		_screen->clearScreen();
		int soundId = _sound->loadRawSound("AUD/EALOGO.WAV", 1);
		_sound->playSound(soundId);
		VideoPlayer_v2 vidPlayer(this, true);
		// TODO: This video should really be played at double-size as it's 320x200
		vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(160, 100), Common::Path("VID/EALOGO.VID"), 0);
		vidPlayer.playToEnd();
		// If the video is skipped the sfx isn't done yet.
		if (_sound->isSFXPlaying())
			_sound->stopSound();
	}
}

void NoctropolisEngine::doUpsell() {
	_files->loadScreen(Common::Path("TITLE.SCN"));
	_screen->fadeIn();
	if (shouldQuit())
		return;

	VideoPlayer_v2 vidPlayer(this, true);
	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(110, 17), Common::Path("TITL.VID"), 0);
	vidPlayer.playToEnd();

	if (shouldQuit())
		return;
	_screen->fadeOut();
}

void NoctropolisEngine::doIntro() {
	static constexpr int lettersX[] = {106, 153, 197, 229, 271, 309, 357, 394, 443, 476, 504};
	static constexpr int titlesSpriteX[] = {238, 237, 237, 200, 200, 216, 231, 207, 198, 201, 234, 200, 220, 235, 214};
	static constexpr int titlesSpriteY[] = {108, 112, 125, 128, 128, 124, 128, 115, 131, 130, 102, 112, 112, 102, 112};

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

	// This ignores window values, but we know that's ok here.
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

	_buffer1.clear();
	_buffer2.clear();
	_screen->clearScreen();
	_room->loadPlayField(0, 0); // MAP.AP
	_screen->setPalette();
	_screen->setIconPalette();
	_scrollRow = _travScrollRow;
	_scrollCol = _travScrollCol;
	_scrollX = _travScrollX;
	_scrollY = _travScrollY;
	// Load MAP.AP
	Resource *spriteData = _files->loadFile(0, 1);
	_objectsTable[0] = new SpriteResource(this, spriteData);
	delete spriteData;

	// This section is "state 1" in NoctTravelEngine::ticker
	((NoctropolisRoom *)_room)->buildScreenXScroll();
	copyBF1BF2();
	copyBF2Vid();
	_screen->setPaletteCycle(0xb5, 0xbe, 5);
	_screen->fadeIn();

	const Font *font = _fonts.getFont(3);
	Font::_fontColors[0] = 0;
	Font::_fontColors[1] = 181;

	Common::Point rawMouse;
	int selectedLoc = -1;
	int hoveredLoc = -1;
	// Show the map and wait for clicks
	while (!shouldQuitOrRestart() && selectedLoc == -1) {
		_midi->midiRepeat();

		_room->buildScreen();
		_buffer2.copyFrom(_buffer1);

		_player->calcPlayer();

		for (int i = 0; i < 15; i++) {
			if (_travel[i]) {
				int x = TRAV_ICONS[i * 3 + 0];
				int y = TRAV_ICONS[i * 3 + 1];
				int imgNum = TRAV_ICONS[i * 3 + 2];
				Common::Point pt(x - _screen->_bufferStart.x, y - _screen->_bufferStart.y);
				_buffer2.plotImage(_objectsTable[0], imgNum, pt);
			}
		}

		copyBF2Vid();

		_screen->cyclePaletteForward();
		selectedLoc = -1;
		rawMouse = _events->calcRawMouse();

		int boxResult = _room->checkBoxes1(rawMouse);
		if (boxResult != hoveredLoc) {
			_screen->fillRect(Common::Rect(Common::Point(220, 380), 300, 16), 246);
			if (boxResult != -1) {
				const char *name = ((NoctropolisResources *)_res)->getPlaceName(boxResult);
				font->drawString(_screen, name, Common::Point(220, 380));
			}
			hoveredLoc = boxResult;
		}
		if (_events->_leftButton) {
			_events->debounceLeft();
			if (boxResult != -1 && _travel[boxResult])
				selectedLoc = boxResult;
		}

		if (!_events->_leftButton || selectedLoc == -1) {
			if (_events->_mousePos.y < 33 && _scrollRow > 0) {
				_scrollRow--;
			} else if (_events->_mousePos.y > 367 && _screen->_vWindowHeight + _scrollRow < _room->_playFieldHeight) {
				_scrollRow++;
			}
		}

		_events->pollEventsAndWait();
	}

	if (shouldQuitOrRestart())
		return;

	assert(selectedLoc >= 0 && selectedLoc < 15);

	// This is NoctTravelEngine::done
	_screen->fadeOut();
	_travScrollRow = _scrollRow;
	_travScrollCol = _scrollCol;
	_travScrollX = _scrollX;
	_travScrollY = _scrollY;
	_player->_roomNumber = TRAV_ROOMS[selectedLoc];
	_player->_rawPlayer.x = TRAV_MAN_POS[selectedLoc * 2];
	_player->_rawPlayer.y = TRAV_MAN_POS[selectedLoc * 2 + 1];
	_player->_moveTo = _player->_rawPlayer;
	setStilettoPos();
	_room->_function = FN_CLEAR1;
	_room->loadRoom(_player->_roomNumber);
}

static constexpr short StilYDirOff[] = {
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
	static constexpr struct { int16 x, y; } DEAD_COORD[] = {
		{167, 21},
		{161, 22},
		{186, 29},
		{180, 29},
		{194, 37},
		{215, 93},
		{215, 93},
		{227, 0}
	};

	static constexpr int DEAD_VIDS[] = {
		3, 0, 0, 0, 0, 0, 5, 6, 7, 1, 1, 2, 6, 4, 3, 3, 0, 3,
	};

	const int vidType = DEAD_VIDS[deathType];

	const Common::Path vidFile(Common::String::format("VID2/DEATH%d.VID", vidType + 1));

	const char *deathScreenFile =
		(deathType < 6 || deathType == 9 || deathType == 10 || deathType == 11)
		 ? "DEATH.SCN" : "DEATH1.SCN";

	const Common::Point vidPos(DEAD_COORD[vidType].x, DEAD_COORD[vidType].y);

	const NoctropolisResources *res = (const NoctropolisResources *)_res;

	int16 deathTextX = deathType == 8 ? 80 : 130;
	int16 deathTextY = deathType == 8 ? 310 : 220;
	const char *deathTextCaption =
		(deathType == 6 || deathType == 12 || deathType == 7)
		? res->getImprisonedTitle() : res->getDeathTitle();

	_screen->fadeOut();
	_midi->loadMusic(98, 39);
	_midi->midiPlay();

	_files->loadScreen(deathScreenFile);

	_screen->_printOrg = _screen->_printStart = Common::Point(deathTextX, deathTextY);
	_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
	_bubbleBox->_bubbleDisplStr = deathTextCaption;
	_bubbleBox->placeBubble(res->getDeathText(deathType));

	_screen->setIconPalette();
	_screen->setPalette();
	_screen->fadeIn();

	VideoPlayer_v2 vidPlayer(this);
	vidPlayer.VideoPlayer::setVideo(_screen, vidPos, vidFile, 0);
	vidPlayer.playToEnd();

	_events->waitKeyActionMouse();

	_screen->fadeOut();

	_bubbleBox->clearBubbles();

	_restartFl = true;
}

void NoctropolisEngine::initMinigame() {
	static constexpr byte minigameInitialConfig[9] = {2, 8, 9, 5, 6, 4, 1, 7, 3};
	memcpy(_minigameCurrentConfig, minigameInitialConfig, 9);
	_flags[91] = 0;
}

void NoctropolisEngine::displayPegsTick() {
	static constexpr byte redSol[3] = {9, 6, 3};
	static constexpr byte greenSol[3] = {2, 5, 8};
	static constexpr byte blueSol[3] = {1, 4, 7};
	static constexpr struct { int16 x, y; } pegPos[10] = {
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
		ie._flags |= IMGFLAG_UNSCALED;
		_images.addToList(ie);
	}
}

void NoctropolisEngine::movePeg(int16 slot) {
	static constexpr struct { int16 a, b; } pegPath[] = {
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
	_events->hideCursor();
	_midi->loadMusic(98, 4);
	_midi->midiPlay();
	_screen->fadeOut();
	_screen->clearScreen();
	VideoPlayer_v2 vidPlayer(this, true);
	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(118, 118), Common::Path("VID1/DRLM00.VID"), 0);
	_screen->setIconPalette();
	_screen->setPalette();
	vidPlayer.playToEnd();
	_events->showCursor();
}

void NoctropolisEngine::flashPaletteEffect() {
	_screen->flashPalette(255);
	_screen->flashPalette(64);
	_screen->flashPalette(16);
}

void NoctropolisEngine::shotoMeanwhile() {
	_midi->stopSong();
	_events->hideCursor();
	_midi->loadMusic(98, 2);
	_midi->midiPlay();
	_screen->fadeOut();
	_screen->clearScreen();
	Font::_fontColors[0] = 0;
	Font::_fontColors[1] = 244;
	const char *meanwhileTxt = ((NoctropolisResources *)_res)->getMeanwhileMessage();
	_fonts._fonts[3]->drawString(_screen, meanwhileTxt, Common::Point(100, 200));
	_screen->fadeIn();
	_system->updateScreen();
	_events->_vbCount = 180;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}
	_screen->fadeOut();
	_screen->clearScreen();
	_screen->_printOrg = _screen->_printStart = Common::Point(300, 300);
	_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
	_bubbleBox->_bubbleDisplStr = ((NoctropolisResources *)_res)->getShotoTitle();
	_bubbleBox->placeBubble(((NoctropolisResources *)_res)->getShotoText());
	VideoPlayer_v2 vidPlayer(this, true);
	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(120, 30), Common::Path("VID1/B126MEAN.VID"), 0);
	_screen->setIconPalette();
	_screen->setPalette();
	vidPlayer.playToEnd();
	_midi->stopSong();
	_screen->copyRawPalToTempPal();
	_screen->fadeOut();
	_events->showCursor();
}

void NoctropolisEngine::showComicCover() {
	_midi->stopSong();
	_midi->loadMusic(98, 1);
	_midi->midiPlay();
	_files->loadScreen(Common::Path("comic93.scn"));
	_screen->setPalette();
	_system->updateScreen();
	_events->_vbCount = 300;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}
	_midi->stopSong();
	_screen->fadeOutThenClearAndSetPal();
}

void NoctropolisEngine::playSuccubusSplit() {
	_midi->stopSong();
	_midi->loadMusic(98, 25);
	_midi->midiPlay();
	_screen->clearScreen();
	VideoPlayer_v2 vidPlayer(this, true);
	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(116, 2), Common::Path("VID1/SUCSPLT1.VID"), 0);
	vidPlayer.playToEnd();
	_midi->stopSong();
	_screen->fadeOutThenClearAndSetPal();
}

void NoctropolisEngine::playSuccubusAttack() {
	_midi->stopSong();
	_midi->loadMusic(98, 4);
	_midi->midiPlay();
	_files->loadScreen(Common::Path("scene06.scn")); // "DARK/SCENE06.SCN"
	_screen->fadeIn();
	_system->updateScreen();
	_events->_vbCount = 60;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}

	VideoPlayer_v2 vidPlayer(this);

	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(118, 118), Common::Path("VID1/SUCATT1.VID"), 0);
	vidPlayer.playToEnd();

	_screen->clearScreen();
	vidPlayer.VideoPlayer::setVideo(_screen, Common::Point(196, 96), Common::Path ("VID1/SUCATT2.VID"), 0);
	vidPlayer.playToEnd();

	_sound->playSound(1);
	_midi->stopSong();
	_screen->fadeOutThenClearAndSetPal();
}

void NoctropolisEngine::doLastComic() {
	_midi->stopSong();
	ComicViewer *viewer = new ComicViewer(this);
	const ComicResource *comic = ((NoctropolisResources *)_res)->getLastComicResource();
	viewer->run(comic);
	delete viewer;
	delete comic;
}

void NoctropolisEngine::doSpecialComic() {
	_midi->stopSong();
	ComicViewer *viewer = new ComicViewer(this);
	const ComicResource *comic = ((NoctropolisResources *)_res)->getSpecialComicResource();
	viewer->run(comic);
	delete viewer;
	delete comic;
}

void NoctropolisEngine::stilWalk() {
	if (!(_room->_roomFlag & kRoomFlagStiletto) || _stilFlag1 == 1)
		return;
	if ((_player->_roomNumber == 26 && _stilFlag2 == 1) || (_player->_roomNumber != 26 && _stilFlag2 != 2))
		return;
	_stil->walk();
}

void NoctropolisEngine::playInventoryStinger() {
	static const FileIdent stingerIdent = FileIdent(99, 64);

	if (!_sound->hasLoadedSound(stingerIdent))
		_sound->loadAndAddSound(99, 64, 1);

	// playSound will not add it to the queue if it's already there.
	_sound->playSoundByIdent(stingerIdent);

	((NoctropolisInventory *)_inventory)->_topItemIndex = 0;
}

void NoctropolisEngine::showNightdiveCredits() {
	const Font *font = _fonts.getFont(1);

	const Common::Path nightDive("DARK/nds.png");
	if (!Common::File().exists(nightDive))
		return;

	_events->clearEvents();
	_screen->fillRect(Common::Rect(345, 0, 640, 400), 0);

	Font::_fontColors[1] = 0xff;
	font->drawString(_screen, "NIGHT DIVE STUDIOS IS:", Common::Point(0x15b, 0x28));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "EXECUTIVE PRODUCER-", Common::Point(0x15b, 0x36));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "STEPHEN KICK", Common::Point(0x1e3, 0x44));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "CHIEF ENGINEER-", Common::Point(0x15b, 0x52));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "JAMES HALEY", Common::Point(0x1e3, 0x60));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "KEX RENDERING TECH-", Common::Point(0x15b, 0x6e));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "SAMUEL VILLARREAL", Common::Point(0x1e3, 0x7c));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "VIDEO PLAYER-", Common::Point(0x15b, 0x8a));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "BRENT ERICKSON", Common::Point(0x1e3, 0x98));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "QA LEAD-", Common::Point(0x15b, 0xa6));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "DANIEL GRAYSHON", Common::Point(0x1e3, 0xb4));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "PLAYTESTING-", Common::Point(0x15b, 0xc2));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "SAMUEL VILLARREAL", Common::Point(0x1e3, 0xd0));
	Font::_fontColors[1] = 0xed;
	font->drawString(_screen, "TECHNICAL ADVISOR-", Common::Point(0x15b, 0xde));
	Font::_fontColors[1] = 0xee;
	font->drawString(_screen, "BRENT ERICKSON", Common::Point(0x1e3, 0xec));

	_events->_vbCount = 7200;
	while (!shouldQuit() && (_events->_vbCount > 0) && !_events->isKeyActionMousePressed()) {
		_events->pollEventsAndWait();
	}
}

Common::Error NoctropolisEngine::synchronize(Common::Serializer &s) {
	Common::Error result = AccessEngine::synchronize(s);

	for (int i = 0; i < ARRAYSIZE(_travel); i++)
		s.syncAsByte(_travel[i]);

	for (int i = 0; i < ARRAYSIZE(_ask); i++)
		s.syncAsByte(_ask[i]);

	for (int i = 0; i < ARRAYSIZE(_asked); i++)
		s.syncAsByte(_asked[i]);

	// This game uses a longer establish table. first 100 bytes were already handled in the common engine sync
	for (int i = 100; i < 128; i++)
		s.syncAsByte(_establishTable[i]);

	_stil->synchronize(s);

	if (s.isLoading()) { // clear minigame state on load
		initMinigame();
		((NoctropolisInventory *)_inventory)->_topItemIndex = 0;
	}

	return result;
}

} // end namespace Noctropolis

} // end namespace Access
