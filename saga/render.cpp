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

	int tmp_w, tmp_h, tmp_bytepp;


	// Initialize FPS timer callback
	Common::g_timer->installTimerProc(&fpsTimerCallback, 1000000, this);

	// Create background buffer 
	_bg_buf_w = _vm->getDisplayWidth();
	_bg_buf_h = _vm->getDisplayHeight();
	_bg_buf = (byte *)calloc(_vm->getDisplayWidth(), _vm->getDisplayHeight());

	if (_bg_buf == NULL) {
		memoryError("Render::Render");
	}

	// Allocate temp buffer for animation decoding, 
	// graphics scalers (2xSaI), etc.
	tmp_w = _vm->getDisplayWidth();
	tmp_h = _vm->getDisplayHeight() + 4; // BG unbanking requres extra rows
	tmp_bytepp = 1;

	_tmp_buf = (byte *)calloc(1, tmp_w * tmp_h * tmp_bytepp);
	if (_tmp_buf == NULL) {
		free(_bg_buf);
		return;
	}

	_tmp_buf_w = tmp_w;
	_tmp_buf_h = tmp_h;

	_backbuf_surface = _vm->_gfx->getBackBuffer();
	_flags = 0;

	_initialized = true;
}

Render::~Render(void) {
	Common::g_timer->removeTimerProc(&fpsTimerCallback);
	free(_bg_buf);
	free(_tmp_buf);

	_initialized = false;
}

bool Render::initialized() {
	return _initialized;
}

int Render::drawScene() {
	SURFACE *backbuf_surface;
	char txt_buf[20];
	int fps_width;
	Point mouse_pt;

	if (!_initialized) {
		return FAILURE;
	}

	_framecount++;

	backbuf_surface = _backbuf_surface;

	// Get mouse coordinates
	mouse_pt = _vm->mousePos();

	if (!(_flags & RF_PLACARD)) {
		// Display scene background
		_vm->_scene->draw();

		if (_vm->_interface->getMode() != kPanelFade) {
			// Draw queued actors
			_vm->_actor->drawActors();

			if (_vm->_puzzle->isActive()) {
				_vm->_puzzle->movePiece(mouse_pt);
				_vm->_actor->drawSpeech();
			}

			if (getFlags() & RF_OBJECTMAP_TEST) {
				if (_vm->_scene->_objectMap)
					_vm->_scene->_objectMap->draw(backbuf_surface, mouse_pt, kITEColorBrightWhite, kITEColorBlack);
				if (_vm->_scene->_actionMap)
					_vm->_scene->_actionMap->draw(backbuf_surface, mouse_pt, kITEColorRed, kITEColorBlack);
			}
			if (getFlags() & RF_ACTOR_PATH_TEST) {
				_vm->_actor->drawPathTest();
			}
		}
	}

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
	_vm->textDrawList(_vm->_scene->_textList, backbuf_surface);

	// Handle user input
	_vm->processInput();

	// Display rendering information
	if (_flags & RF_SHOW_FPS) {
		sprintf(txt_buf, "%d", _fps);
		fps_width = _vm->_font->getStringWidth(SMALL_FONT_ID, txt_buf, 0, FONT_NORMAL);
		_vm->_font->draw(SMALL_FONT_ID, backbuf_surface, txt_buf, 0, backbuf_surface->w - fps_width, 2,
					kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE);
	}

	// Display "paused game" message, if applicable
	if (_flags & RF_RENDERPAUSE) {
		int msg_len = strlen(PAUSEGAME_MSG);
		int msg_w = _vm->_font->getStringWidth(BIG_FONT_ID, PAUSEGAME_MSG, msg_len, FONT_OUTLINE);
		_vm->_font->draw(BIG_FONT_ID, backbuf_surface, PAUSEGAME_MSG, msg_len,
				(backbuf_surface->w - msg_w) / 2, 90, kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE);
	}

	// Update user interface

	_vm->_interface->update(mouse_pt, UPDATE_MOUSEMOVE);

	// Display text formatting test, if applicable
	if (_flags & RF_TEXT_TEST) {
		_vm->textDraw(MEDIUM_FONT_ID, backbuf_surface, test_txt, mouse_pt.x, mouse_pt.y,
				kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE | FONT_CENTERED);
	}

	// Display palette test, if applicable
	if (_flags & RF_PALETTE_TEST) {
		drawPalette(backbuf_surface);
	}

	_system->copyRectToScreen((byte *)backbuf_surface->pixels, backbuf_surface->w, 0, 0, 
							  backbuf_surface->w, backbuf_surface->h);

	_system->updateScreen();
	return SUCCESS;
}

unsigned int Render::getFrameCount() {
	return _framecount;
}

unsigned int Render::resetFrameCount() {
	unsigned int framecount = _framecount;

	_framecount = 0;

	return framecount;
}

void Render::fpsTimerCallback(void *refCon) {
	((Render *)refCon)->fpsTimer();
}

void Render::fpsTimer(void) {
	_fps = _framecount;
	_framecount = 0;
}

unsigned int Render::getFlags() {
	return _flags;
}

void Render::setFlag(unsigned int flag) {
	_flags |= flag;
}

void Render::clearFlag(unsigned int flag) {
	_flags &= ~flag;
}

void Render::toggleFlag(unsigned int flag) {
	_flags ^= flag;
}

int Render::getBufferInfo(BUFFER_INFO *bufinfo) {
	assert(bufinfo != NULL);

	bufinfo->bg_buf = _bg_buf;
	bufinfo->bg_buf_w = _bg_buf_w;
	bufinfo->bg_buf_h = _bg_buf_h;

	bufinfo->tmp_buf = _tmp_buf;
	bufinfo->tmp_buf_w = _tmp_buf_w;
	bufinfo->tmp_buf_h = _tmp_buf_h;

	return SUCCESS;
}

} // End of namespace Saga
