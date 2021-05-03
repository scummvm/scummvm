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
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

RenderObject::RenderObject(uint16 zOrder) :
		_needsRedraw(true),
		_isVisible(true),
		_redrawFrom(nullptr),
		_z(zOrder) {}

RenderObject::RenderObject(RenderObject &redrawFrom, uint16 zOrder) :
		_needsRedraw(true),
		_isVisible(true),
		_redrawFrom(&redrawFrom),
		_z(zOrder) {}

RenderObject::RenderObject(RenderObject &redrawFrom, uint16 zOrder, Graphics::ManagedSurface &surface, const Common::Rect &srcBounds, const Common::Rect &destBounds) :
		RenderObject(redrawFrom, zOrder) {
	_drawSurface.create(surface, srcBounds);
	_screenPosition = destBounds;
}

void RenderObject::init() {
	_previousScreenPosition = _screenPosition;
}

void RenderObject::registerGraphics() {
	g_nancy->_graphicsManager->addObject(this);
}

RenderObject::~RenderObject() {
	g_nancy->_graphicsManager->removeObject(this);
	if (_drawSurface.getPixels()) {
		_drawSurface.free();
	}
}

void RenderObject::moveTo(Common::Point position) {
	_previousScreenPosition = _screenPosition;
	_screenPosition.moveTo(position);
	_needsRedraw = true;
}

void RenderObject::setVisible(bool visible) {
	_isVisible = visible;
	_needsRedraw = true;
}

void RenderObject::setTransparent(bool isTransparent) {
	if (isTransparent) {
		_drawSurface.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
	} else {
		_drawSurface.clearTransparentColor();
	}
}

Common::Rect RenderObject::getScreenPosition() const {
	if (isViewportRelative()) {
		return NancySceneState.getViewport().convertViewportToScreen(_screenPosition);
	} else {
		return _screenPosition;
	}
}

Common::Rect RenderObject::getPreviousScreenPosition() const {
	if (isViewportRelative()) {
		return NancySceneState.getViewport().convertViewportToScreen(_previousScreenPosition);
	} else {
		return _previousScreenPosition;
	}
}

// Convert from screen to local space. Does NOT take _drawSurface's offset into account
Common::Rect RenderObject::convertToLocal(const Common::Rect &screen) const {
	Common::Rect ret = screen;
	Common::Point offset;

	if (isViewportRelative()) {
		Common::Rect viewportScreenPos = NancySceneState.getViewport().getScreenPosition();
		offset.x -= viewportScreenPos.left;
		offset.y -= viewportScreenPos.top;
		uint viewportScroll = NancySceneState.getViewport().getCurVerticalScroll();
		offset.y += viewportScroll;
	}

	offset.x -= _screenPosition.left;
	offset.y -= _screenPosition.top;

	ret.translate(offset.x, offset.y);
	return ret;
}

// Convert from local to screen space. Does NOT take _drawSurface's offset into account
Common::Rect RenderObject::convertToScreen(const Common::Rect &rect) const {

	Common::Rect ret = rect;
	Common::Point offset;

	if (isViewportRelative()) {
		Common::Rect viewportScreenPos = NancySceneState.getViewport().getScreenPosition();
		offset.x += viewportScreenPos.left;
		offset.y += viewportScreenPos.top;
		uint viewportScroll = NancySceneState.getViewport().getCurVerticalScroll();
		offset.y -= viewportScroll;
	}

	offset.x += _screenPosition.left;
	offset.y += _screenPosition.top;

	ret.translate(offset.x, offset.y);
	return ret;
}

} // End of namespace Nancy
