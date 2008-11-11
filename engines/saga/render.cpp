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
 * $URL$
 * $Id$
 *
 */

// Main rendering loop

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/font.h"
#include "saga/gfx.h"
#include "saga/interface.h"
#include "saga/objectmap.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/scene.h"

#include "common/timer.h"
#include "common/system.h"

namespace Saga {

const char *test_txt = "The quick brown fox jumped over the lazy dog. She sells sea shells down by the sea shore.";
const char *pauseStringITE = "PAWS GAME";
const char *pauseStringIHNM = "Game Paused";

Render::Render(SagaEngine *vm, OSystem *system) {
	_vm = vm;
	_system = system;
	_initialized = false;

	// Initialize FPS timer callback
	_vm->_timer->installTimerProc(&fpsTimerCallback, 1000000, this);

	_backGroundSurface.create(_vm->getDisplayWidth(), _vm->getDisplayHeight(), 1);

	_flags = 0;

	_initialized = true;
}

Render::~Render(void) {
	_vm->_timer->removeTimerProc(&fpsTimerCallback);
	_backGroundSurface.free();

	_initialized = false;
}

bool Render::initialized() {
	return _initialized;
}

void Render::drawScene() {
	Surface *backBufferSurface;
	char txtBuffer[20];
	Point mousePoint;
	Point textPoint;

	assert(_initialized);

	_renderedFrameCount++;

	backBufferSurface = _vm->_gfx->getBackBuffer();

	// Get mouse coordinates
	mousePoint = _vm->mousePos();

	if (!(_flags & (RF_DEMO_SUBST | RF_MAP) || _vm->_interface->getMode() == kPanelPlacard)) {
		if (_vm->_interface->getFadeMode() != kFadeOut) {
			// Display scene background
			if (!(_flags & RF_DISABLE_ACTORS) || _vm->getGameType() == GType_ITE)
				_vm->_scene->draw();

			if (_vm->_puzzle->isActive()) {
				_vm->_puzzle->movePiece(mousePoint);
				_vm->_actor->drawSpeech();
			} else {
				// Draw queued actors
				if (!(_flags & RF_DISABLE_ACTORS))
					_vm->_actor->drawActors();
			}

			if (getFlags() & RF_OBJECTMAP_TEST) {
				if (_vm->_scene->_objectMap)
					_vm->_scene->_objectMap->draw(backBufferSurface, mousePoint, kITEColorBrightWhite, kITEColorBlack);
				if (_vm->_scene->_actionMap)
					_vm->_scene->_actionMap->draw(backBufferSurface, mousePoint, kITEColorRed, kITEColorBlack);
			}
			if (getFlags() & RF_ACTOR_PATH_TEST) {
				_vm->_actor->drawPathTest();
			}
		}
	}

	if (_flags & RF_MAP)
		_vm->_interface->mapPanelDrawCrossHair();

	if ((_vm->_interface->getMode() == kPanelOption) ||
		(_vm->_interface->getMode() == kPanelQuit) ||
		(_vm->_interface->getMode() == kPanelLoad) ||
		(_vm->_interface->getMode() == kPanelSave)) {
		_vm->_interface->drawOption();

		if (_vm->_interface->getMode() == kPanelQuit) {
			_vm->_interface->drawQuit();
		}
		if (_vm->_interface->getMode() == kPanelLoad) {
			_vm->_interface->drawLoad();
		}
		if (_vm->_interface->getMode() == kPanelSave) {
			_vm->_interface->drawSave();
		}
	}

	if (_vm->_interface->getMode() == kPanelProtect) {
		_vm->_interface->drawProtect();
	}

	// Draw queued text strings
	_vm->_scene->drawTextList(backBufferSurface);

	// Handle user input
	_vm->processInput();

	// Display rendering information
	if (_flags & RF_SHOW_FPS) {
		sprintf(txtBuffer, "%d", _fps);
		textPoint.x = backBufferSurface->w - _vm->_font->getStringWidth(kKnownFontSmall, txtBuffer, 0, kFontOutline);
		textPoint.y = 2;

		_vm->_font->textDraw(kKnownFontSmall, backBufferSurface, txtBuffer, textPoint, kITEColorBrightWhite, kITEColorBlack, kFontOutline);
	}

	// Display "paused game" message, if applicable
	if (_flags & RF_RENDERPAUSE) {
		if (_vm->getGameType() == GType_ITE) {
			textPoint.x = (backBufferSurface->w - _vm->_font->getStringWidth(kKnownFontPause, pauseStringITE, 0, kFontOutline)) / 2;
			textPoint.y = 90;

			_vm->_font->textDraw(kKnownFontPause, backBufferSurface, pauseStringITE, textPoint, _vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
		} else {
			textPoint.x = (backBufferSurface->w - _vm->_font->getStringWidth(kKnownFontPause, pauseStringIHNM, 0, kFontOutline)) / 2;
			textPoint.y = 90;

			_vm->_font->textDraw(kKnownFontPause, backBufferSurface, pauseStringIHNM, textPoint, _vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
		}
	}

	// Update user interface
	_vm->_interface->update(mousePoint, UPDATE_MOUSEMOVE);

	// Display text formatting test, if applicable
	if (_flags & RF_TEXT_TEST) {
		Rect rect(mousePoint.x, mousePoint.y, mousePoint.x + 100, mousePoint.y + 50);
		_vm->_font->textDrawRect(kKnownFontMedium, backBufferSurface, test_txt, rect,
				kITEColorBrightWhite, kITEColorBlack, (FontEffectFlags)(kFontOutline | kFontCentered));
	}

	// Display palette test, if applicable
	if (_flags & RF_PALETTE_TEST) {
		backBufferSurface->drawPalette();
	}

	_system->copyRectToScreen((byte *)backBufferSurface->pixels, backBufferSurface->w, 0, 0,
							  backBufferSurface->w, backBufferSurface->h);

	_system->updateScreen();
}

void Render::fpsTimerCallback(void *refCon) {
	((Render *)refCon)->fpsTimer();
}

void Render::fpsTimer(void) {
	_fps = _renderedFrameCount;
	_renderedFrameCount = 0;
}

} // End of namespace Saga
