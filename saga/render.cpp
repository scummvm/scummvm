/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
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
#include "saga/text.h"

#include "common/timer.h"
#include "common/system.h"

namespace Saga {

const char *test_txt = "The quick brown fox jumped over the lazy dog. She sells sea shells down by the sea shore.";

Render::Render(SagaEngine *vm, OSystem *system) {
	_vm = vm;
	_system = system;
	_initialized = false;

	// Initialize FPS timer callback
	Common::g_timer->installTimerProc(&fpsTimerCallback, 1000000, this);

	_backGroundSurface.create(_vm->getDisplayWidth(), _vm->getDisplayHeight(), 1);

	_flags = 0;

	_initialized = true;
}

Render::~Render(void) {
	Common::g_timer->removeTimerProc(&fpsTimerCallback);
	_backGroundSurface.free();

	_initialized = false;
}

bool Render::initialized() {
	return _initialized;
}

void Render::drawScene() {
	Surface *backBufferSurface;
	char txt_buf[20];
	int fps_width;
	Point mouse_pt;

	assert(_initialized);

	_frameCount++;

	backBufferSurface = _vm->_gfx->getBackBuffer();

	// Get mouse coordinates
	mouse_pt = _vm->mousePos();

	if (/*_vm->_interface->getMode() != kPanelPlacard*/!(_flags & (RF_PLACARD | RF_MAP))) {
		// Display scene background
		_vm->_scene->draw();

		if (_vm->_interface->getFadeMode() != kFadeOut) {
			if (_vm->_puzzle->isActive()) {
				_vm->_puzzle->movePiece(mouse_pt);
				_vm->_actor->drawSpeech();
			} else {
				// Draw queued actors
				if (!(_flags & RF_DISABLE_ACTORS))
					_vm->_actor->drawActors();
			}

			if (getFlags() & RF_OBJECTMAP_TEST) {
				if (_vm->_scene->_objectMap)
					_vm->_scene->_objectMap->draw(backBufferSurface, mouse_pt, kITEColorBrightWhite, kITEColorBlack);
				if (_vm->_scene->_actionMap)
					_vm->_scene->_actionMap->draw(backBufferSurface, mouse_pt, kITEColorRed, kITEColorBlack);
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

	// Draw queued text strings
	_vm->textDrawList(_vm->_scene->_textList, backBufferSurface);

	// Handle user input
	_vm->processInput();

	// Display rendering information
	if (_flags & RF_SHOW_FPS) {
		sprintf(txt_buf, "%d", _fps);
		fps_width = _vm->_font->getStringWidth(SMALL_FONT_ID, txt_buf, 0, FONT_NORMAL);
		_vm->_font->draw(SMALL_FONT_ID, backBufferSurface, txt_buf, 0, backBufferSurface->w - fps_width, 2,
					kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE);
	}

	// Display "paused game" message, if applicable
	if (_flags & RF_RENDERPAUSE) {
		int msg_len = strlen(PAUSEGAME_MSG);
		int msg_w = _vm->_font->getStringWidth(BIG_FONT_ID, PAUSEGAME_MSG, msg_len, FONT_OUTLINE);
		_vm->_font->draw(BIG_FONT_ID, backBufferSurface, PAUSEGAME_MSG, msg_len,
				(backBufferSurface->w - msg_w) / 2, 90, kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE);
	}

	// Update user interface

	_vm->_interface->update(mouse_pt, UPDATE_MOUSEMOVE);

	// Display text formatting test, if applicable
	if (_flags & RF_TEXT_TEST) {
		_vm->textDraw(MEDIUM_FONT_ID, backBufferSurface, test_txt, mouse_pt.x, mouse_pt.y,
				kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE | FONT_CENTERED);
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
	_fps = _frameCount;
	_frameCount = 0;
}

} // End of namespace Saga
