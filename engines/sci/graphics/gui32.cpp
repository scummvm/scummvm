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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/gui32.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/robot.h"
#include "sci/graphics/text.h"
#include "sci/graphics/view.h"

namespace Sci {

SciGui32::SciGui32(EngineState *state, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, Cursor *cursor)
	: _s(state), _screen(screen), _palette(palette), _cache(cache), _cursor(cursor) {

	_compare = new GfxCompare(_s->_segMan, _s->_kernel, _cache, _screen);
}

SciGui32::~SciGui32() {
	delete _compare;
	delete _cache;
}

void SciGui32::resetEngineState(EngineState *s) {
	_s = s;
}

void SciGui32::init() {
}

void SciGui32::globalToLocal(int16 *x, int16 *y, reg_t planeObj) {
	*x = *x - GET_SEL32V(_s->_segMan, planeObj, left);
	*y = *y - GET_SEL32V(_s->_segMan, planeObj, top);
}

void SciGui32::localToGlobal(int16 *x, int16 *y, reg_t planeObj) {
	*x = *x + GET_SEL32V(_s->_segMan, planeObj, left);
	*y = *y + GET_SEL32V(_s->_segMan, planeObj, top);
}

void SciGui32::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	*textWidth = 0;
	*textHeight = 0;
}

void SciGui32::shakeScreen(uint16 shakeCount, uint16 directions) {
	while (shakeCount--) {
		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			_screen->setVerticalShakePos(10);
		// TODO: horizontal shakes
		g_system->updateScreen();
		g_system->delayMillis(50);
		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			_screen->setVerticalShakePos(0);
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
}

uint16 SciGui32::onControl(byte screenMask, Common::Rect rect) {
	Common::Rect adjustedRect = rect;
	uint16 result;

	adjustedRect.translate(0, 10);

	result = _compare->onControl(screenMask, rect);
	return result;
}

void SciGui32::setNowSeen(reg_t objectReference) {
	_compare->SetNowSeen(objectReference);
}

bool SciGui32::canBeHere(reg_t curObject, reg_t listReference) {
	Common::Rect checkRect;
	uint16 signal, controlMask;
	bool result;

	checkRect.left = GET_SEL32V(_s->_segMan, curObject, brLeft);
	checkRect.top = GET_SEL32V(_s->_segMan, curObject, brTop);
	checkRect.right = GET_SEL32V(_s->_segMan, curObject, brRight);
	checkRect.bottom = GET_SEL32V(_s->_segMan, curObject, brBottom);
	signal = GET_SEL32V(_s->_segMan, curObject, signal);
	controlMask = GET_SEL32V(_s->_segMan, curObject, illegalBits);
	result = (_compare->onControl(SCI_SCREEN_MASK_CONTROL, checkRect) & controlMask) ? false : true;
	if ((result)) { // gui16 && (signal & (kSignalIgnoreActor | kSignalRemoveView)) == 0) {
		List *list = _s->_segMan->lookupList(listReference);
		if (!list)
			error("kCanBeHere called with non-list as parameter");

		result = _compare->CanBeHereCheckRectList(curObject, checkRect, list);
	}
	return result;
}

bool SciGui32::isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) {
	View *tmpView = _cache->getView(viewId);
	CelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	position.x = CLIP<int>(position.x, 0, celInfo->width - 1);
	position.y = CLIP<int>(position.y, 0, celInfo->height - 1);
	byte *celData = tmpView->getBitmap(loopNo, celNo);
	bool result = (celData[position.y * celInfo->width + position.x] == celInfo->clearKey);
	return result;
}

void SciGui32::baseSetter(reg_t object) {
	if (lookup_selector(_s->_segMan, object, _s->_kernel->_selectorCache.brLeft, NULL, NULL) == kSelectorVariable) {
		int16 x = GET_SEL32V(_s->_segMan, object, x);
		int16 y = GET_SEL32V(_s->_segMan, object, y);
		int16 z = (_s->_kernel->_selectorCache.z > -1) ? GET_SEL32V(_s->_segMan, object, z) : 0;
		int16 yStep = GET_SEL32V(_s->_segMan, object, yStep);
		GuiResourceId viewId = GET_SEL32V(_s->_segMan, object, view);
		int16 loopNo = GET_SEL32V(_s->_segMan, object, loop);
		int16 celNo = GET_SEL32V(_s->_segMan, object, cel);

		if (viewId != SIGNAL_OFFSET) {
			View *tmpView = _cache->getView(viewId);
			Common::Rect celRect;

			tmpView->getCelRect(loopNo, celNo, x, y, z, &celRect);
			celRect.bottom = y + 1;
			celRect.top = celRect.bottom - yStep;

			PUT_SEL32V(_s->_segMan, object, brLeft, celRect.left);
			PUT_SEL32V(_s->_segMan, object, brRight, celRect.right);
			PUT_SEL32V(_s->_segMan, object, brTop, celRect.top);
			PUT_SEL32V(_s->_segMan, object, brBottom, celRect.bottom);
		}
	}
}

void SciGui32::hideCursor() {
	_cursor->hide();
}

void SciGui32::showCursor() {
	_cursor->show();
}

bool SciGui32::isCursorVisible() {
	return _cursor->isVisible();
}

void SciGui32::setCursorShape(GuiResourceId cursorId) {
	_cursor->setShape(cursorId);
}

void SciGui32::setCursorView(GuiResourceId viewNum, int loopNum, int cellNum, Common::Point *hotspot) {
	_cursor->setView(viewNum, loopNum, cellNum, hotspot);
}

void SciGui32::setCursorPos(Common::Point pos) {
	//pos.y += _gfx->GetPort()->top;
	//pos.x += _gfx->GetPort()->left;
	moveCursor(pos);
}

Common::Point SciGui32::getCursorPos() {
	return _cursor->getPosition();
}

void SciGui32::moveCursor(Common::Point pos) {
	// pos.y += _windowMgr->_picWind->rect.top;
	// pos.x += _windowMgr->_picWind->rect.left;

	// pos.y = CLIP<int16>(pos.y, _windowMgr->_picWind->rect.top, _windowMgr->_picWind->rect.bottom - 1);
	// pos.x = CLIP<int16>(pos.x, _windowMgr->_picWind->rect.left, _windowMgr->_picWind->rect.right - 1);

	if (pos.x > _screen->getWidth() || pos.y > _screen->getHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	_cursor->setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_s->_event->get(SCI_EVENT_PEEK);
}

void SciGui32::setCursorZone(Common::Rect zone) {
	_cursor->setMoveZone(zone);
}

void SciGui32::syncWithFramebuffer() {
	_screen->syncWithFramebuffer();
}

void SciGui32::addScreenItem(reg_t object) {
	_screenItems.push_back(object);
	warning("addScreenItem %X:%X (%s)", object.segment, object.offset, _s->_segMan->getObjectName(object));
}

void SciGui32::deleteScreenItem(reg_t object) {
	for (uint32 itemNr = 0; itemNr < _screenItems.size(); itemNr++) {
		if (_screenItems[itemNr] == object) {
			_screenItems.remove_at(itemNr);
			return;
		}
	}
}

void SciGui32::addPlane(reg_t object) {
	_planes.push_back(object);
}

void SciGui32::updatePlane(reg_t object) {
}

void SciGui32::deletePlane(reg_t object) {
	for (uint32 planeNr = 0; planeNr < _planes.size(); planeNr++) {
		if (_planes[planeNr] == object) {
			_planes.remove_at(planeNr);
			return;
		}
	}
}

void SciGui32::frameOut() {
	for (uint32 planeNr = 0; planeNr < _planes.size(); planeNr++) {
		reg_t planeObj = _planes[planeNr];
		int16 priority = GET_SEL32V(_s->_segMan, planeObj, priority);

		if (priority == -1)
			continue;

		int16 picNum = GET_SEL32V(_s->_segMan, planeObj, picture);
		if (picNum > -1) {
			SciGuiPicture *picture = new SciGuiPicture(_s->resMan, 0, _screen, _palette, picNum, false);

			picture->draw(100, false, false, 0);
			delete picture;
			//_gfx->drawPicture(picNum, 100, false, false, 0);
		}

		// FIXME: This code doesn't currently work properly because of the way we set up the
		// view port. We are starting at 10 pixels from the top automatically. The offset should
		// be based on the plane's top in SCI32 instead. Here we would be adding 10 to 10 and
		// therefore drawing too low. We would need to draw each picture at the correct offset
		// which doesn't currently happen.
		int16 planeTop = GET_SEL32V(_s->_segMan, planeObj, top);
		int16 planeLeft = GET_SEL32V(_s->_segMan, planeObj, left);

		for (uint32 itemNr = 0; itemNr < _screenItems.size(); itemNr++) {
			reg_t viewObj = _screenItems[itemNr];
			reg_t planeOfItem = GET_SEL32(_s->_segMan, viewObj, plane);
			if (planeOfItem == _planes[planeNr]) {
				uint16 viewId = GET_SEL32V(_s->_segMan, viewObj, view);
				uint16 loopNo = GET_SEL32V(_s->_segMan, viewObj, loop);
				uint16 celNo = GET_SEL32V(_s->_segMan, viewObj, cel);
				uint16 x = GET_SEL32V(_s->_segMan, viewObj, x);
				uint16 y = GET_SEL32V(_s->_segMan, viewObj, y);
				uint16 z = GET_SEL32V(_s->_segMan, viewObj, z);
				priority = GET_SEL32V(_s->_segMan, viewObj, priority);
				uint16 scaleX = GET_SEL32V(_s->_segMan, viewObj, scaleX);
				uint16 scaleY = GET_SEL32V(_s->_segMan, viewObj, scaleY);
				//int16 signal = GET_SEL32V(_s->_segMan, viewObj, signal);

				// FIXME: See above
				x += planeLeft;
				y += planeTop;

				// Theoretically, leftPos and topPos should be sane
				// Apparently, sometimes they're not, therefore I'm adding some sanity checks here so that
				// the hack underneath does not try and draw cels outside the screen coordinates
				if (x >= _screen->getWidth()) {
					continue;
				}

				if (y >= _screen->getHeight()) {
					continue;
				}

				if (viewId != 0xffff) {
					Common::Rect celRect;
					View *view = _cache->getView(viewId);

					view->getCelRect(loopNo, celNo, x, y, z, &celRect);

					if (celRect.top < 0 || celRect.top >= _screen->getHeight())
						continue;

					if (celRect.left < 0 || celRect.left >= _screen->getWidth())
						continue;

					if ((scaleX == 128) && (scaleY == 128))
						view->draw(celRect, celRect, celRect, loopNo, celNo, 255, 0, false);
					else
						view->drawScaled(celRect, celRect, celRect, loopNo, celNo, 255, scaleX, scaleY);
					//_gfx->drawCel(view, loopNo, celNo, celRect, priority, 0, scaleX, scaleY);
				}
			}
		}
	}
	_screen->copyToScreen();
}

void SciGui32::drawRobot(GuiResourceId robotId) {
	Robot *test = new Robot(_s->resMan, _screen, robotId);
	test->draw();
	delete test;
}

bool SciGui32::debugShowMap(int mapNo) {
	_screen->debugShowMap(mapNo);
	return false;
}

} // End of namespace Sci
