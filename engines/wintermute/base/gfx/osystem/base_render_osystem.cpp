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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/gfx/osystem/render_ticket.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/util.h"

#include "common/system.h"
#include "common/queue.h"
#include "common/config-manager.h"

#define DIRTY_RECT_LIMIT 800

namespace Wintermute {

BaseRenderer *makeOSystemRenderer(BaseGame *inGame) {
	return new BaseRenderOSystem(inGame);
}

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::BaseRenderOSystem(BaseGame *inGame) : BaseRenderer(inGame) {
	_renderSurface = new Graphics::Surface();
	_blankSurface = new Graphics::Surface();
	_lastFrameIter = _renderQueue.end();
	_needsFlip = true;
	_skipThisFrame = false;

	_borderLeft = _borderRight = _borderTop = _borderBottom = 0;
	_ratioX = _ratioY = 1.0f;
	_dirtyRect = nullptr;
	_disableDirtyRects = false;
	if (ConfMan.hasKey("dirty_rects")) {
		_disableDirtyRects = !ConfMan.getBool("dirty_rects");
	}

	_lastScreenChangeID = g_system->getScreenChangeID();
}

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::~BaseRenderOSystem() {
	RenderQueueIterator it = _renderQueue.begin();
	while (it != _renderQueue.end()) {
		RenderTicket *ticket = *it;
		it = _renderQueue.erase(it);
		delete ticket;
	}

	delete _dirtyRect;

	_renderSurface->free();
	delete _renderSurface;
	_blankSurface->free();
	delete _blankSurface;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::initRenderer(int width, int height, bool windowed) {
	_width = width;
	_height = height;
	_renderRect.setWidth(_width);
	_renderRect.setHeight(_height);

	_realWidth = width;
	_realHeight = height;

	//TODO: Tiny resolution-displays might want to do some resolution-selection logic here

	//_realWidth = BaseEngine::instance().getRegistry()->readInt("Debug", "ForceResWidth", _width);
	//_realHeight = BaseEngine::instance().getRegistry()->readInt("Debug", "ForceResHeight", _height);

	float origAspect = (float)_width / (float)_height;
	float realAspect = (float)_realWidth / (float)_realHeight;

	float ratio;
	if (origAspect < realAspect) {
		// normal to wide
		ratio = (float)_realHeight / (float)_height;
	} else {
		// wide to normal
		ratio = (float)_realWidth / (float)_width;
	}

	_borderLeft = (int)((_realWidth - (_width * ratio)) / 2);
	_borderRight = (int)(_realWidth - (_width * ratio) - _borderLeft);

	_borderTop = (int)((_realHeight - (_height * ratio)) / 2);
	_borderBottom = (int)(_realHeight - (_height * ratio) - _borderTop);

	_ratioX = (float)(_realWidth - _borderLeft - _borderRight) / (float)_width;
	_ratioY = (float)(_realHeight - _borderTop - _borderBottom) / (float)_height;

	_windowed = !ConfMan.getBool("fullscreen");

	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	initGraphics(_width, _height, &format);

	if (g_system->getScreenFormat() != format) {
		warning("Couldn't setup GFX-backend for %dx%dx%d", _width, _height, format.bytesPerPixel * 8);
		return STATUS_FAILED;
	}

	g_system->showMouse(false);

	_renderSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_blankSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_blankSurface->fillRect(Common::Rect(0, 0, _blankSurface->h, _blankSurface->w), _blankSurface->format.ARGBToColor(255, 0, 0, 0));
	_active = true;

	_clearColor = _renderSurface->format.ARGBToColor(255, 0, 0, 0);

	return STATUS_OK;
}

bool BaseRenderOSystem::indicatorFlip() {
	if (_indicatorWidthDrawn > 0 && _indicatorHeight > 0) {
		g_system->copyRectToScreen((byte *)_renderSurface->getBasePtr(_indicatorX, _indicatorY), _renderSurface->pitch, _indicatorX, _indicatorY, _indicatorWidthDrawn, _indicatorHeight);
		g_system->updateScreen();
	}
	return STATUS_OK;
}

bool BaseRenderOSystem::forcedFlip() {
	g_system->copyRectToScreen((byte *)_renderSurface->getPixels(), _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
	g_system->updateScreen();
	return STATUS_OK;
}

bool BaseRenderOSystem::flip() {
	if (_skipThisFrame) {
		_skipThisFrame = false;
		delete _dirtyRect;
		_dirtyRect = nullptr;
		g_system->updateScreen();
		_needsFlip = false;

		// Reset ticketing state
		_lastFrameIter = _renderQueue.end();
		RenderQueueIterator it;
		for (it = _renderQueue.begin(); it != _renderQueue.end(); ++it) {
			(*it)->_wantsDraw = false;
		}

		addDirtyRect(_renderRect);
		return true;
	}
	if (!_disableDirtyRects) {
		drawTickets();
	} else {
		// Clear the scale-buffered tickets that wasn't reused.
		RenderQueueIterator it = _renderQueue.begin();
		while (it != _renderQueue.end()) {
			if ((*it)->_wantsDraw == false) {
				RenderTicket *ticket = *it;
				it = _renderQueue.erase(it);
				delete ticket;
			} else {
				(*it)->_wantsDraw = false;
				++it;
			}
		}
	}

	int oldScreenChangeID = _lastScreenChangeID;
	_lastScreenChangeID = g_system->getScreenChangeID();
	bool screenChanged = _lastScreenChangeID != oldScreenChangeID;

	if (_needsFlip || _disableDirtyRects || screenChanged) {
		if (_disableDirtyRects || screenChanged) {
			g_system->copyRectToScreen((byte *)_renderSurface->getPixels(), _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
		}
		//  g_system->copyRectToScreen((byte *)_renderSurface->getPixels(), _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());
		delete _dirtyRect;
		_dirtyRect = nullptr;
		_needsFlip = false;
	}
	_lastFrameIter = _renderQueue.end();

	g_system->updateScreen();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::onWindowChange() {
	_windowed = !g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
}

//////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::setWindowed(bool windowed) {
	ConfMan.setBool("fullscreen", !windowed);
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !windowed);
	g_system->endGFXTransaction();
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::fill(byte r, byte g, byte b, Common::Rect *rect) {
	_clearColor = _renderSurface->format.ARGBToColor(0xFF, r, g, b);
	if (!_disableDirtyRects) {
		return STATUS_OK;
	}
	if (!rect) {
// TODO: This should speed things up, but for some reason it misses the size by quite a bit.
/*		if (r == 0 && g == 0 && b == 0) {
			// Simply memcpy from the buffered black-surface, way faster than Surface::fillRect.
			memcpy(_renderSurface->pixels, _blankSurface->pixels, _renderSurface->pitch * _renderSurface->h);
			return STATUS_OK;
		}*/
		rect = &_renderRect;
	}
	// TODO: This doesn't work with dirty rects
	_renderSurface->fillRect(*rect, _clearColor);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::fade(uint16 alpha) {
	byte dwAlpha = (byte)(255 - alpha);
	return fadeToColor(0, 0, 0, dwAlpha);
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::fadeToColor(byte r, byte g, byte b, byte a) {
	Common::Rect fillRect;

	Rect32 rc;
	_gameRef->getCurrentViewportRect(&rc);
	fillRect.left = (int16)rc.left;
	fillRect.top = (int16)rc.top;
	fillRect.setWidth((int16)(rc.right - rc.left));
	fillRect.setHeight((int16)(rc.bottom - rc.top));

	modTargetRect(&fillRect);

	//TODO: This is only here until I'm sure about the final pixelformat
	uint32 col = _renderSurface->format.ARGBToColor(a, r, g, b);

	Graphics::Surface surf;
	surf.create((uint16)fillRect.width(), (uint16)fillRect.height(), _renderSurface->format);
	Common::Rect sizeRect(fillRect);
	sizeRect.translate(-fillRect.top, -fillRect.left);
	surf.fillRect(fillRect, col);
	Graphics::TransformStruct temp = Graphics::TransformStruct();
	temp._alphaDisable = false;
	drawSurface(nullptr, &surf, &sizeRect, &fillRect, temp);
	surf.free();

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	//SDL_RenderFillRect(_renderer, &fillRect);
}

Graphics::PixelFormat BaseRenderOSystem::getPixelFormat() const {
	return _renderSurface->format;
}

void BaseRenderOSystem::drawSurface(BaseSurfaceOSystem *owner, const Graphics::Surface *surf,
                                    Common::Rect *srcRect, Common::Rect *dstRect, Graphics::TransformStruct &transform) {
	if (_disableDirtyRects) {
		RenderTicket *ticket = new RenderTicket(owner, surf, srcRect, dstRect, transform);
		ticket->_wantsDraw = true;
		_renderQueue.push_back(ticket);
		drawFromSurface(ticket);
		return;
	}

	// Skip rects that are completely outside the screen:
	if ((dstRect->left < 0 && dstRect->right < 0) || (dstRect->top < 0 && dstRect->bottom < 0)) {
		return;
	}

	if (owner) { // Fade-tickets are owner-less
		RenderTicket compare(owner, nullptr, srcRect, dstRect, transform);
		RenderQueueIterator it = _lastFrameIter;
		++it;
		// Avoid calling end() and operator* every time, when potentially going through
		// LOTS of tickets.
		RenderQueueIterator endIterator = _renderQueue.end();
		RenderTicket *compareTicket = nullptr;
		for (; it != endIterator; ++it) {
			compareTicket = *it;
			if (*(compareTicket) == compare && compareTicket->_isValid) {
				if (_disableDirtyRects) {
					drawFromSurface(compareTicket);
				} else {
					drawFromQueuedTicket(it);
				}
				return;
			}
		}
	}
	RenderTicket *ticket = new RenderTicket(owner, surf, srcRect, dstRect, transform);
	if (!_disableDirtyRects) {
		drawFromTicket(ticket);
	} else {
		ticket->_wantsDraw = true;
		_renderQueue.push_back(ticket);
		drawFromSurface(ticket);
	}
}

void BaseRenderOSystem::invalidateTicket(RenderTicket *renderTicket) {
	addDirtyRect(renderTicket->_dstRect);
	renderTicket->_isValid = false;
//	renderTicket->_canDelete = true; // TODO: Maybe readd this, to avoid even more duplicates.
}

void BaseRenderOSystem::invalidateTicketsFromSurface(BaseSurfaceOSystem *surf) {
	RenderQueueIterator it;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); ++it) {
		if ((*it)->_owner == surf) {
			invalidateTicket(*it);
		}
	}
}

void BaseRenderOSystem::drawFromTicket(RenderTicket *renderTicket) {
	renderTicket->_wantsDraw = true;

	++_lastFrameIter;
	// In-order
	if (_renderQueue.empty() || _lastFrameIter == _renderQueue.end()) {
		_lastFrameIter--;
		_renderQueue.push_back(renderTicket);
		++_lastFrameIter;
		addDirtyRect(renderTicket->_dstRect);
	} else {
		// Before something
		RenderQueueIterator pos = _lastFrameIter;
		_renderQueue.insert(pos, renderTicket);
		--_lastFrameIter;
		addDirtyRect(renderTicket->_dstRect);
	}
}

void BaseRenderOSystem::drawFromQueuedTicket(const RenderQueueIterator &ticket) {
	RenderTicket *renderTicket = *ticket;
	assert(!renderTicket->_wantsDraw);
	renderTicket->_wantsDraw = true;

	++_lastFrameIter;
	// Not in the same order?
	if (*_lastFrameIter != renderTicket) {
		--_lastFrameIter;
		// Remove the ticket from the list
		assert(*_lastFrameIter != renderTicket);
		_renderQueue.erase(ticket);
		// Is not in order, so readd it as if it was a new ticket
		drawFromTicket(renderTicket);
	}
}

void BaseRenderOSystem::addDirtyRect(const Common::Rect &rect) {
	if (!_dirtyRect) {
		_dirtyRect = new Common::Rect(rect);
	} else {
		_dirtyRect->extend(rect);
	}
	_dirtyRect->clip(_renderRect);
}

void BaseRenderOSystem::drawTickets() {
	RenderQueueIterator it = _renderQueue.begin();
	// Clean out the old tickets
	// Note: We draw invalid tickets too, otherwise we wouldn't be honoring
	// the draw request they obviously made BEFORE becoming invalid, either way
	// we have a copy of their data, so their invalidness won't affect us.
	while (it != _renderQueue.end()) {
		if ((*it)->_wantsDraw == false) {
			RenderTicket *ticket = *it;
			addDirtyRect((*it)->_dstRect);
			it = _renderQueue.erase(it);
			delete ticket;
		} else {
			++it;
		}
	}
	if (!_dirtyRect || _dirtyRect->width() == 0 || _dirtyRect->height() == 0) {
		it = _renderQueue.begin();
		while (it != _renderQueue.end()) {
			RenderTicket *ticket = *it;
			ticket->_wantsDraw = false;
			++it;
		}
		return;
	}

	it = _renderQueue.begin();
	_lastFrameIter = _renderQueue.end();
	// A special case: If the screen has one giant OPAQUE rect to be drawn, then we skip filling
	// the background color. Typical use-case: Fullscreen FMVs.
	// Caveat: The FPS-counter will invalidate this.
	if (it != _lastFrameIter && _renderQueue.front() == _renderQueue.back() && (*it)->_transform._alphaDisable == true) {
		// If our single opaque rect fills the dirty rect, we can skip filling.
		if (*_dirtyRect != (*it)->_dstRect) {
			// Apply the clear-color to the dirty rect.
			_renderSurface->fillRect(*_dirtyRect, _clearColor);
		}
		// Otherwise Do NOT fill.
	} else {
		// Apply the clear-color to the dirty rect.
		_renderSurface->fillRect(*_dirtyRect, _clearColor);
	}
	for (; it != _renderQueue.end(); ++it) {
		RenderTicket *ticket = *it;
		if (ticket->_dstRect.intersects(*_dirtyRect)) {
			// dstClip is the area we want redrawn.
			Common::Rect dstClip(ticket->_dstRect);
			// reduce it to the dirty rect
			dstClip.clip(*_dirtyRect);
			// we need to keep track of the position to redraw the dirty rect
			Common::Rect pos(dstClip);
			int16 offsetX = ticket->_dstRect.left;
			int16 offsetY = ticket->_dstRect.top;
			// convert from screen-coords to surface-coords.
			dstClip.translate(-offsetX, -offsetY);

			drawFromSurface(ticket, &pos, &dstClip);
			_needsFlip = true;
		}
		// Some tickets want redraw but don't actually clip the dirty area (typically the ones that shouldn't become clear-color)
		ticket->_wantsDraw = false;
	}
	g_system->copyRectToScreen((byte *)_renderSurface->getBasePtr(_dirtyRect->left, _dirtyRect->top), _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());

	it = _renderQueue.begin();
	// Clean out the old tickets
	while (it != _renderQueue.end()) {
		if ((*it)->_isValid == false) {
			RenderTicket *ticket = *it;
			addDirtyRect((*it)->_dstRect);
			it = _renderQueue.erase(it);
			delete ticket;
		} else {
			++it;
		}
	}

}

// Replacement for SDL2's SDL_RenderCopy
void BaseRenderOSystem::drawFromSurface(RenderTicket *ticket) {
	ticket->drawToSurface(_renderSurface);
}

void BaseRenderOSystem::drawFromSurface(RenderTicket *ticket, Common::Rect *dstRect, Common::Rect *clipRect) {
	ticket->drawToSurface(_renderSurface, dstRect, clipRect);
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	// This function isn't used outside of indicator-displaying, and thus quite unused in
	// BaseRenderOSystem when dirty-rects are enabled.
	if (!_disableDirtyRects && !_indicatorDisplay) {
		error("BaseRenderOSystem::DrawLine - doesn't work for dirty rects yet");
	}

	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);
	byte a = RGBCOLGetA(color);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

	Point32 point1, point2;
	point1.x = x1;
	point1.y = y1;
	pointToScreen(&point1);

	point2.x = x2;
	point2.y = y2;
	pointToScreen(&point2);

	// TODO: This thing is mostly here until I'm sure about the final color-format.
	uint32 colorVal = _renderSurface->format.ARGBToColor(a, r, g, b);
	_renderSurface->drawLine(point1.x, point1.y, point2.x, point2.y, colorVal);
	//SDL_RenderDrawLine(_renderer, point1.x, point1.y, point2.x, point2.y);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseImage *BaseRenderOSystem::takeScreenshot() {
	// TODO: Clip by viewport.
	BaseImage *screenshot = new BaseImage();
	screenshot->copyFrom(_renderSurface);
	return screenshot;
}

//////////////////////////////////////////////////////////////////////////
Common::String BaseRenderOSystem::getName() const {
	return "ScummVM-OSystem-renderer";
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::setViewport(int left, int top, int right, int bottom) {
	Common::Rect rect;
	// TODO: Hopefully this is the same logic that ScummVM uses.
	rect.left = (int16)(left + _borderLeft);
	rect.top = (int16)(top + _borderTop);
	rect.setWidth((int16)((right - left) * _ratioX));
	rect.setHeight((int16)((bottom - top) * _ratioY));

	_renderRect = rect;
	return STATUS_OK;
}

Rect32 BaseRenderOSystem::getViewPort() {
	Rect32 ret;
	ret.top = _renderRect.top;
	ret.bottom = _renderRect.bottom;
	ret.left = _renderRect.left;
	ret.right = _renderRect.right;
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::modTargetRect(Common::Rect *rect) {
	return;
	int newWidth = (int16)MathUtil::roundUp(rect->width() * _ratioX);
	int newHeight = (int16)MathUtil::roundUp(rect->height() * _ratioY);
	rect->left = (int16)MathUtil::round(rect->left * _ratioX + _borderLeft);
	rect->top = (int16)MathUtil::round(rect->top * _ratioY + _borderTop);
	rect->setWidth(newWidth);
	rect->setHeight(newHeight);
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointFromScreen(Point32 *point) {
	point->x = (int16)(point->x / _ratioX - _borderLeft / _ratioX + _renderRect.left);
	point->y = (int16)(point->y / _ratioY - _borderTop / _ratioY + _renderRect.top);
}


//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointToScreen(Point32 *point) {
	point->x = (int16)MathUtil::roundUp(point->x * _ratioX) + _borderLeft - _renderRect.left;
	point->y = (int16)MathUtil::roundUp(point->y * _ratioY) + _borderTop - _renderRect.top;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::dumpData(const char *filename) {
	warning("BaseRenderOSystem::DumpData(%s) - stubbed", filename); // TODO
}

BaseSurface *BaseRenderOSystem::createSurface() {
	return new BaseSurfaceOSystem(_gameRef);
}

void BaseRenderOSystem::endSaveLoad() {
	BaseRenderer::endSaveLoad();

	// Clear the scale-buffered tickets as we just loaded.
	RenderQueueIterator it = _renderQueue.begin();
	while (it != _renderQueue.end()) {
		RenderTicket *ticket = *it;
		it = _renderQueue.erase(it);
		delete ticket;
	}
	// HACK: After a save the buffer will be drawn before the scripts get to update it,
	// so just skip this single frame.
	_skipThisFrame = true;
	_lastFrameIter = _renderQueue.end();

	_renderSurface->fillRect(Common::Rect(0, 0, _renderSurface->w, _renderSurface->h), _renderSurface->format.ARGBToColor(255, 0, 0, 0));
	g_system->copyRectToScreen((byte *)_renderSurface->getPixels(), _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
	g_system->updateScreen();
}

bool BaseRenderOSystem::startSpriteBatch() {
	return STATUS_OK;
}

bool BaseRenderOSystem::endSpriteBatch() {
	return STATUS_OK;
}

} // End of namespace Wintermute
