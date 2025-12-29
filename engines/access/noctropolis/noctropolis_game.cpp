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
#include "image/png.h"
#include "graphics/color_quantizer.h"
#include "common/config-manager.h"

namespace Access {

namespace Noctropolis {


NoctropolisEngine::NoctropolisEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
AccessEngine(syst, gameDesc), _invScript(nullptr), _stil(nullptr)
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
	_player->_roomNumber = 3;

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
	_events->setNormalCursor(CURSOR_CROSSHAIRS);
	_mouseMode = 0;
	_animation->clearTimers();
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

} // end namespace Noctropolis

} // end namespace Access
