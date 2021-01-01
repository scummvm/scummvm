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
	_fullRefresh = true;
	_dualSurface = (vm->getLanguage() == Common::JA_JPN);

#ifdef SAGA_DEBUG
	// Initialize FPS timer callback
	_vm->getTimerManager()->installTimerProc(&fpsTimerCallback, 1000000, this, "sagaFPS");
#endif

	_backGroundSurface.create(_vm->getDisplayInfo().width, _vm->getDisplayInfo().height, Graphics::PixelFormat::createFormatCLUT8());

	if (_dualSurface)
		_mergeSurface.create(_vm->getDisplayInfo().width << 1, _vm->getDisplayInfo().height << 1, Graphics::PixelFormat::createFormatCLUT8());

	_flags = 0;

	_initialized = true;
}

Render::~Render() {
#ifdef SAGA_DEBUG
	_vm->getTimerManager()->removeTimerProc(&fpsTimerCallback);
#endif

	_backGroundSurface.free();
	_mergeSurface.free();

	_initialized = false;
}

bool Render::initialized() {
	return _initialized;
}

void Render::drawScene() {
	Point mousePoint;
	Point textPoint;
	int curMode = _vm->_interface->getMode();
	assert(_initialized);

#ifdef SAGA_DEBUG
	_renderedFrameCount++;
#endif

	// Get mouse coordinates
	mousePoint = _vm->mousePos();

	if (!_fullRefresh)
		restoreChangedRects();
	else
		_dirtyRects.clear();

	if (!(_flags & (RF_DEMO_SUBST | RF_MAP) || curMode == kPanelPlacard)) {
		if (_vm->_interface->getFadeMode() != kFadeOut) {
			// Display scene background
			if (!(_flags & RF_DISABLE_ACTORS) || _vm->getGameId() == GID_ITE)
				_vm->_scene->draw();

			if (_vm->_scene->isITEPuzzleScene()) {
				_vm->_puzzle->movePiece(mousePoint);
				_vm->_actor->drawSpeech();
			} else {
				// Draw queued actors
				if (!(_flags & RF_DISABLE_ACTORS))
					_vm->_actor->drawActors();
			}

			// WORKAROUND
			// Bug #2886130: "ITE: Graphic Glitches during Cat Tribe Celebration"
			if (_vm->_scene->currentSceneNumber() == 274) {
				_vm->_interface->drawStatusBar();
			}

#ifdef SAGA_DEBUG
			if (getFlags() & RF_OBJECTMAP_TEST) {
				if (_vm->_scene->_objectMap)
					_vm->_scene->_objectMap->draw(mousePoint, kITEColorBrightWhite, kITEColorBlack);
				if (_vm->_scene->_actionMap)
					_vm->_scene->_actionMap->draw(mousePoint, kITEColorRed, kITEColorBlack);
			}
#endif

#ifdef ACTOR_DEBUG
			if (getFlags() & RF_ACTOR_PATH_TEST) {
				_vm->_actor->drawPathTest();
			}
#endif
		}
	} else {
		_fullRefresh = true;
	}

	if (_flags & RF_MAP)
		_vm->_interface->mapPanelDrawCrossHair();

	if ((curMode == kPanelOption) ||
		(curMode == kPanelQuit) ||
		(curMode == kPanelLoad) ||
		(curMode == kPanelSave)) {
		_vm->_interface->drawOption();

		if (curMode == kPanelQuit) {
			_vm->_interface->drawQuit();
		}
		if (curMode == kPanelLoad) {
			_vm->_interface->drawLoad();
		}
		if (curMode == kPanelSave) {
			_vm->_interface->drawSave();
		}
	}

	if (curMode == kPanelProtect) {
		_vm->_interface->drawProtect();
	}

	// Draw queued text strings
	_vm->_scene->drawTextList();

	// Handle user input
	_vm->processInput();

#ifdef SAGA_DEBUG
	// Display rendering information
	if (_flags & RF_SHOW_FPS) {
		char txtBuffer[20];
		sprintf(txtBuffer, "%d", _fps);
		textPoint.x = _vm->_gfx->getBackBufferWidth() - _vm->_font->getStringWidth(kKnownFontSmall, txtBuffer, 0, kFontOutline);
		textPoint.y = 2;

		_vm->_font->textDraw(kKnownFontSmall, txtBuffer, textPoint, kITEColorBrightWhite, kITEColorBlack, kFontOutline);
	}
#endif

	// Display "paused game" message, if applicable
	if (_flags & RF_RENDERPAUSE) {
		const char *pauseString = (_vm->getGameId() == GID_ITE) ? pauseStringITE : pauseStringIHNM;
		textPoint.x = (_vm->_gfx->getBackBufferWidth() - _vm->_font->getStringWidth(kKnownFontPause, pauseString, 0, kFontOutline)) / 2;
		textPoint.y = 90;

		_vm->_font->textDraw(kKnownFontPause, pauseString, textPoint,
							_vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
	}

	// Update user interface
	_vm->_interface->update(mousePoint, UPDATE_MOUSEMOVE);

#ifdef SAGA_DEBUG
	// Display text formatting test, if applicable
	if (_flags & RF_TEXT_TEST) {
		Rect rect(mousePoint.x, mousePoint.y, mousePoint.x + 100, mousePoint.y + 50);
		_vm->_font->textDrawRect(kKnownFontMedium, test_txt, rect,
				kITEColorBrightWhite, kITEColorBlack, (FontEffectFlags)(kFontOutline | kFontCentered));
	}

	// Display palette test, if applicable
	if (_flags & RF_PALETTE_TEST) {
		_vm->_gfx->drawPalette();
	}
#endif

	drawDirtyRects();

	_system->updateScreen();

	// TODO: Change this to false to use dirty rectangles
	// Still quite buggy
	_fullRefresh = true;
}

void Render::addDirtyRect(Common::Rect r) {
	if (_fullRefresh)
		return;

	// Clip rectangle
	r.clip(_backGroundSurface.w, _backGroundSurface.h);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	if (_vm->_interface->getFadeMode() != kFadeOut)
		_dirtyRects.push_back(r);
}

#define mCopyRectToScreen(x, y, w, h) \
	if (_dualSurface) { \
		scale2xAndMergeOverlay(x, y, w, h); \
		_system->copyRectToScreen(_mergeSurface.getPixels(), _mergeSurface.pitch, x << 1, y << 1, w << 1, h << 1); \
	} else \
		_system->copyRectToScreen(_vm->_gfx->getBackBufferPixels(), _vm->_gfx->getBackBufferWidth(), x, y, w, h)

void Render::restoreChangedRects() {
	if (!_fullRefresh) {
		for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			//_backGroundSurface.frameRect(*it, 1);		// DEBUG
			if (_vm->_interface->getFadeMode() != kFadeOut) {
				mCopyRectToScreen(it->left, it->top, it->width(), it->height());
			}
		}
	}
	_dirtyRects.clear();
}

void Render::drawDirtyRects() {
	if (!_fullRefresh) {
		for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			//_backGroundSurface.frameRect(*it, 2);		// DEBUG
			if (_vm->_interface->getFadeMode() != kFadeOut) {
				mCopyRectToScreen(it->left, it->top, it->width(), it->height());
			}
		}
	} else {
		mCopyRectToScreen(0, 0, _backGroundSurface.w, _backGroundSurface.h);
	}
	_dirtyRects.clear();
}

#undef mCopyRectToScreen

void Render::scale2xAndMergeOverlay(int x, int y, int w, int h) {
	int src0Pitch = _vm->_gfx->getBackBufferPitch();
	int src1Pitch = _vm->_gfx->getSJISBackBufferPitch();
	int dst1Pitch = _mergeSurface.pitch;
	const byte *src00 = _vm->_gfx->getBackBufferPixels() + y * src0Pitch + x;
	const byte *src10 = _vm->_gfx->getSJISBackBufferPixels() + y * 2 * src1Pitch + x * 2;
	const byte *src11 = src10 + src1Pitch;
	byte *dst10 = (byte*)_mergeSurface.getBasePtr(x << 1, y << 1);
	byte *dst11 = dst10 + dst1Pitch;
	src0Pitch -= w;
	src1Pitch += (src1Pitch - (w << 1));
	dst1Pitch += (dst1Pitch - (w << 1));

	while (h--) {
		for (int i = 0; i < w; ++i) {
			// v0: pixels from "normal" surface that have to be scaled
			// v1: pixels from hires text surface that go on top
			uint8 v0 = *src00++;
			uint8 v1 = *src10++;
			*dst10++ = v1 ? v1 : v0;
			v1 = *src10++;
			*dst10++ = v1 ? v1 : v0;
			v1 = *src11++;
			*dst11++ = v1 ? v1 : v0;
			v1 = *src11++;
			*dst11++ = v1 ? v1 : v0;
		}
		src00 += src0Pitch;
		src10 += src1Pitch;
		src11 += src1Pitch;
		dst10 += dst1Pitch;
		dst11 += dst1Pitch;
	}
}

#ifdef SAGA_DEBUG
void Render::fpsTimerCallback(void *refCon) {
	((Render *)refCon)->fpsTimer();
}

void Render::fpsTimer() {
	// CHECKME: This is potentially called from a different thread because it is
	// called from a timer callback. However, it does not seem to take any
	// precautions to avoid race conditions.
	_fps = _renderedFrameCount;
	_renderedFrameCount = 0;
}
#endif

} // End of namespace Saga
