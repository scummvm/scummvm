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

#include "common/textconsole.h"
#include "dgds/hoc_intro.h"
#include "dgds/dgds.h"
#include "dgds/sound.h"
#include "dgds/game_palettes.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/globals.h"

namespace Dgds {

HocIntro::HocIntro() :  _scrollCountdown1(0), _xOffset203e(0), _xOffset2042(0), _scrollCountdown2(0) {

}

void HocIntro::init() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());

	if (globals->getIntroState())
		return;

	_scrollCountdown2 = 137;
	_xOffset2042 = 320;
	_scrollCountdown1 = 150;
	engine->_soundPlayer->loadMusic("INTRO1.SNG");
	engine->_soundPlayer->playMusic(0);
	engine->getGamePals()->loadPalette("xx.pal");
	Image tmp(engine->getResourceManager(), engine->getDecompressor());
	tmp.drawScreen("xx.scr", engine->getBackgroundBuffer());

	_maskImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_noMaskImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_maskImg->loadBitmap("xx_mask.bmp");
	_noMaskImg->loadBitmap("xx_nomas.bmp");

	engine->_compositionBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 81);
	globals->setIntroState(1);

	// TODO: Is this the right variable?
	globals->setNativeGameState(32);

	engine->_soundPlayer->playSFX(35);
}

void HocIntro::tick() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());
	if (!globals->getIntroState())
		return;

	if (_scrollCountdown2 != 0)
	  _scrollCountdown2--;

	if (_xOffset2042 > -90)
	  _xOffset2042 -= 3;

	_scrollCountdown1--;

	doScroll();

	_drawWin.left = 0;
}

void HocIntro::doScroll() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());
	assert(globals->getIntroState());

	if (_scrollCountdown1 == 0) {
	  globals->setIntroState(2);
	  _scrollCountdown1 = 1;
	}

	// For the start of the scroll, we want to blank out the background.
	// The original does this differently but this will do.
	if (_xOffset2042 > 0)
		engine->_compositionBuffer.fillRect(Common::Rect(_xOffset2042, 200), 81);

	// set clip window
	_drawWin.top = 0;
	_drawWin.left = MAX((int16)0, _xOffset2042);
	_drawWin.right = 320;
	_drawWin.bottom = 200;
	clean1(_scrollCountdown2);
	clean2(_xOffset2042);
	draw1(_scrollCountdown2);
	draw2(_xOffset2042);
}

static int16 _clipXOffset(int16 x1, int16 x2) {
	if (x1 + x2 > 320)
		x2 = 320 - x1;

	if (x2 < 0)
		x2 = 0;

	return x2;
}

void HocIntro::leave() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());
	if (!globals->getIntroState())
		return;

	//engine->getGamePals()->freePal("xx.pal");

	// Store the current frame in the background
	engine->getBackgroundBuffer().blitFrom(engine->_compositionBuffer);

	_maskImg.reset();
	_noMaskImg.reset();

	globals->setIntroState(0);
	static_cast<HocGlobals *>(engine->getGameGlobals())->setNativeGameState(0);
}

void HocIntro::doCopy(int16 x1, int16 y1, int16 x2, int16 y2) {
	int16 xx = _xOffset203e + x1;
	if (xx < _drawWin.left)
		xx = _drawWin.left;

	if (xx < 320) {
		DgdsEngine *engine = DgdsEngine::getInstance();
		Graphics::ManagedSurface &bg = engine->getBackgroundBuffer();
		Graphics::ManagedSurface &comp = engine->_compositionBuffer;
		int16 xx2 = _clipXOffset(xx, x2);
		Common::Rect copyRect(Common::Point(xx, y1), xx2, y2);
		comp.blitFrom(bg, copyRect, copyRect);
	}
}

void HocIntro::clean1(int16 xoff) {
  _xOffset203e = xoff;
  doCopy(0x6c, 0x58, 0x10, 0x1a);
  doCopy(0x87, 0x38, 0x2b, 0x11);
  doCopy(0xa4, 0x48, 0x1b, 0x1e);
  doCopy(0xc0, 0x40, 0xd, 4);
}

void HocIntro::clean2(int16 xoff) {
  _xOffset203e = xoff;
  doCopy(0, 0x14, 0x46, 0x25);
  doCopy(0x31, 0x39, 0x2b, 0x1a);
  doCopy(0x5b, 0x52, 0x36, 0x1f);
  doCopy(0x7d, 0x70, 0x1b, 6);
}

void HocIntro::draw1(int16 xoff) {
	Graphics::ManagedSurface &dst = DgdsEngine::getInstance()->_compositionBuffer;
	_noMaskImg->drawBitmap(1, xoff + 0xcc, 0x33, _drawWin, dst);
	_noMaskImg->drawBitmap(0, xoff + 0xf8, 0x14, _drawWin, dst);
	_noMaskImg->drawBitmap(2, xoff, 0x7a, _drawWin, dst);
	_maskImg->drawBitmap(1, xoff + 0xcf, 0x15, _drawWin, dst);
	_maskImg->drawBitmap(2, xoff + 0xbc, 0x35, _drawWin, dst);
	_maskImg->drawBitmap(3, xoff + 0xb7, 99, _drawWin, dst);
	_maskImg->drawBitmap(4, xoff + 0x79, 0x38, _drawWin, dst);
	_maskImg->drawBitmap(5, xoff + 0x5d, 0x58, _drawWin, dst);
	_maskImg->drawBitmap(0, xoff + 0x19, 0x72, _drawWin, dst);
}

void HocIntro::draw2(int16 xoff) {
	Graphics::ManagedSurface &dst = DgdsEngine::getInstance()->_compositionBuffer;

	if (8 < xoff)
		dst.fillRect(Common::Rect(Common::Point(0, 0x14), 8, 0x88), 81);

	if (_drawWin.left < 9) {
		if (_drawWin.left > 0)
			dst.fillRect(Common::Rect(Common::Point(0, 0x14), xoff, 100), 81);
	} else {
		dst.fillRect(Common::Rect(Common::Point(xoff - 8, 0x14), 8, 0x68), 81);
		_drawWin.left = 8;
	}

	dst.fillRect(Common::Rect(Common::Point(xoff, 0x31), 0x29, 0x6b), 81);

	_maskImg->drawBitmap(6, xoff, 0x14, _drawWin, dst);
	_noMaskImg->drawBitmap(3, xoff + 0x29, 0x49, _drawWin, dst);
	_noMaskImg->drawBitmap(4, xoff + 0x43, 0x53, _drawWin, dst);
	_noMaskImg->drawBitmap(5, xoff + 0x57, 0x6d, _drawWin, dst);
	_maskImg->drawBitmap(7, xoff + 0x29, 0x19, _drawWin, dst);
	_maskImg->drawBitmap(8, xoff + 0x43, 0x3c, _drawWin, dst);
	_maskImg->drawBitmap(9, xoff + 0x57, 0x52, _drawWin, dst);
	_maskImg->drawBitmap(10, xoff + 0x7a, 0x5c, _drawWin, dst);
	_maskImg->drawBitmap(11, xoff + 0x90, 0x8c, _drawWin, dst);
	_maskImg->drawBitmap(11, xoff + 0x13b, 0x8c, _drawWin, dst);
}

} // end namespace Dgds
