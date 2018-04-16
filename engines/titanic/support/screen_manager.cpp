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

#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/titanic.h"
#include "graphics/screen.h"

namespace Titanic {

CScreenManager *CScreenManager::_screenManagerPtr;
CScreenManager *CScreenManager::_currentScreenManagerPtr;

CScreenManager::CScreenManager(TitanicEngine *vm): _vm(vm) {
	_screenManagerPtr = nullptr;
	_currentScreenManagerPtr = nullptr;

	_frontRenderSurface = nullptr;
	_mouseCursor = nullptr;
	_textCursor = nullptr;
	_inputHandler = nullptr;
	_fontNumber = 0;

	_screenManagerPtr = this;
}

CScreenManager::~CScreenManager() {
	_screenManagerPtr = nullptr;
}

void CScreenManager::setWindowHandle(int v) {
	// Not needed
}

bool CScreenManager::resetWindowHandle(int v) {
	hideCursor();
	return true;
}

CScreenManager *CScreenManager::setCurrent() {
	if (!_currentScreenManagerPtr)
		_currentScreenManagerPtr = _screenManagerPtr;

	return _currentScreenManagerPtr;
}

void CScreenManager::setSurfaceBounds(SurfaceNum surfaceNum, const Rect &r) {
	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		_backSurfaces[surfaceNum]._bounds = r;
	else if (surfaceNum == SURFACE_PRIMARY)
		_frontSurfaceBounds = r;
}

int CScreenManager::setFontNumber(int fontNumber) {
	int oldFontNumber = _fontNumber;
	_fontNumber = fontNumber;
	return oldFontNumber;
}

void CScreenManager::preLoad() {
	if (_textCursor)
		_textCursor->hide();
}

/*------------------------------------------------------------------------*/

OSScreenManager::OSScreenManager(TitanicEngine *vm): CScreenManager(vm),
		_directDrawManager(vm, false) {
	_field48 = 0;
	_field4C = 0;
	_field50 = 0;
	_field54 = 0;
}

OSScreenManager::~OSScreenManager() {
	destroyFrontAndBackBuffers();
	delete _mouseCursor;
	delete _textCursor;
}

void OSScreenManager::setMode(int width, int height, int bpp, uint numBackSurfaces, bool flag2) {
	assert(bpp == 16);
	destroyFrontAndBackBuffers();
	_directDrawManager.initVideo(width, height, bpp, numBackSurfaces);

	_vm->_screen->create(width, height, g_system->getScreenFormat());
	_frontRenderSurface = new OSVideoSurface(this, nullptr);
	_frontRenderSurface->setSurface(this, _directDrawManager._mainSurface);

	_backSurfaces.resize(numBackSurfaces);
	for (uint idx = 0; idx < numBackSurfaces; ++idx) {
		_backSurfaces[idx]._surface = new OSVideoSurface(this, nullptr);
		_backSurfaces[idx]._surface->setSurface(this, _directDrawManager._backSurfaces[idx]);
	}

	// Load fonts
	_fonts[0].load(149);
	_fonts[1].load(151);
	_fonts[2].load(152);
	_fonts[3].load(153);

	// Load the cursors
	loadCursors();
}

void OSScreenManager::drawCursors() {
	// The original did both text and mouse cursor drawing here.
	// For ScummVM, we only need to worry about the text cursor
	_textCursor->draw();
}

DirectDrawSurface *OSScreenManager::getDDSurface(SurfaceNum surfaceNum) {
	if (surfaceNum == SURFACE_PRIMARY)
		return _directDrawManager._mainSurface;
	else if (surfaceNum < (int)_backSurfaces.size())
		return _directDrawManager._backSurfaces[surfaceNum];
	else
		return nullptr;
}

CVideoSurface *OSScreenManager::lockSurface(SurfaceNum surfaceNum) {
	CVideoSurface *surface = getSurface(surfaceNum);
	surface->lock();
	return surface;
}

void OSScreenManager::unlockSurface(CVideoSurface *surface) {
	surface->unlock();
}

CVideoSurface *OSScreenManager::getSurface(SurfaceNum surfaceNum) const {
	if (surfaceNum == SURFACE_PRIMARY)
		return _frontRenderSurface;
	else if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		return _backSurfaces[surfaceNum]._surface;
	else
		return nullptr;
}

void OSScreenManager::fillRect(SurfaceNum surfaceNum, Rect *rect, byte r, byte g, byte b) {
	DirectDrawSurface *surface = getDDSurface(surfaceNum);
	if (!surface)
		return;

	// If bounds are provided, clip and use them. Otherwise, use entire surface area
	Rect surfaceRect(0, 0, surface->getWidth(), surface->getHeight());
	Rect tempRect;

	if (rect) {
		tempRect = *rect;
		tempRect.clip(surfaceRect);
	} else {
		tempRect = surfaceRect;
	}

	// Constrain the fill area to the set modification area of the surface
	Rect surfaceBounds = (surfaceNum == SURFACE_PRIMARY) ? _frontSurfaceBounds :
		_backSurfaces[surfaceNum]._bounds;
	if (!surfaceBounds.isEmpty())
		tempRect.constrain(surfaceBounds);

	// If there is any area defined, clear it
	if (tempRect.isValidRect())
		surface->fillRect(&tempRect, r, g, b);
}

void OSScreenManager::blitFrom(SurfaceNum surfaceNum, CVideoSurface *src,
		const Point *destPos, const Rect *srcRect) {
	// Get the dest surface
	CVideoSurface *destSurface = _frontRenderSurface;
	if (surfaceNum < -1)
		return;
	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		destSurface = _backSurfaces[surfaceNum]._surface;
	if (!destSurface->hasSurface())
		return;

	Point destPoint = destPos ? *destPos : Point(0, 0);
	Rect srcBounds = srcRect ? *srcRect : Rect(0, 0, src->getWidth(), src->getHeight());
	Rect *bounds = &srcBounds;
	Rect rect2;

	Rect surfaceBounds = (surfaceNum == SURFACE_PRIMARY) ? _frontSurfaceBounds :
		_backSurfaces[surfaceNum]._bounds;

	if (!surfaceBounds.isEmpty()) {
		// Perform clipping to the bounds of the back surface
		rect2 = srcBounds;
		rect2.translate(-srcBounds.left, -srcBounds.top);
		rect2.translate(destPoint.x, destPoint.y);
		rect2.constrain(surfaceBounds);

		rect2.translate(-destPoint.x, -destPoint.y);
		rect2.translate(srcBounds.left, srcBounds.top);

		if (rect2.isEmpty())
			return;

		destPoint.x += rect2.left - srcBounds.left;
		destPoint.y += rect2.top - srcBounds.top;
		bounds = &rect2;
	}

	if (!bounds->isEmpty())
		destSurface->blitFrom(destPoint, src, bounds);
}

void OSScreenManager::blitFrom(SurfaceNum surfaceNum, const Rect *rect, CVideoSurface *src, int v) {
	// Get the dest surface
	CVideoSurface *destSurface = _frontRenderSurface;
	if (surfaceNum < -1)
		return;
	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		destSurface = _backSurfaces[surfaceNum]._surface;
	if (!destSurface->hasSurface())
		return;

	if (!rect->isEmpty())
		destSurface->blitFrom(Point(rect->left, rect->top), src, rect);
}

int OSScreenManager::writeString(int surfaceNum, const Rect &destRect,
		int yOffset, const CString &str, CTextCursor *textCursor) {
	CVideoSurface *surface;
	Rect bounds;

	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size()) {
		surface = _backSurfaces[surfaceNum]._surface;
		bounds = _backSurfaces[surfaceNum]._bounds;
	} else if (surfaceNum == SURFACE_PRIMARY) {
		surface = _frontRenderSurface;
		bounds = _frontSurfaceBounds;
	} else {
		return -1;
	}

	return _fonts[_fontNumber].writeString(surface, destRect, bounds,
		yOffset, str, textCursor);
}

void OSScreenManager::writeString(int surfaceNum, const Point &destPos,
		const Rect &clipRect, const CString &str, int lineWidth) {
	CVideoSurface *surface;
	Rect bounds;

	if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size()) {
		surface = _backSurfaces[surfaceNum]._surface;
		bounds = _backSurfaces[surfaceNum]._bounds;
	} else if (surfaceNum == -1) {
		surface = _frontRenderSurface;
		bounds = Rect(0, 0, surface->getWidth(), surface->getHeight());
	} else {
		return;
	}

	Rect destRect = clipRect;
	destRect.constrain(bounds);

	_fonts[_fontNumber].writeString(surface, destPos, destRect, str, lineWidth);
}

void OSScreenManager::setFontColor(byte r, byte g, byte b) {
	_fonts[_fontNumber].setColor(r, g, b);
}

int OSScreenManager::getTextBounds(const CString &str, int maxWidth, Point *sizeOut) const {
	return _fonts[_fontNumber].getTextBounds(str, maxWidth, sizeOut);
}

int OSScreenManager::getFontHeight() const {
	return _fonts[_fontNumber]._fontHeight;
}

int OSScreenManager::stringWidth(const CString &str) {
	return _fonts[_fontNumber].stringWidth(str);
}

void OSScreenManager::frameRect(SurfaceNum surfaceNum, const Rect &rect, byte r, byte g, byte b) {
	Rect top(rect.left, rect.top, rect.right, rect.top + 1);
	fillRect(surfaceNum, &top, r, g, b);
	Rect bottom(rect.left, rect.bottom - 1, rect.right, rect.bottom);
	fillRect(surfaceNum, &bottom, r, g, b);
	Rect left(rect.left, rect.top, rect.left + 1, rect.bottom);
	fillRect(surfaceNum, &left, r, g, b);
	Rect right(rect.right - 1, rect.top, rect.right, rect.bottom);
}

void OSScreenManager::clearSurface(SurfaceNum surfaceNum, Rect *bounds) {
	if (surfaceNum == SURFACE_PRIMARY)
		_directDrawManager._mainSurface->fill(bounds, 0);
	else if (surfaceNum >= 0 && surfaceNum < (int)_backSurfaces.size())
		_directDrawManager._backSurfaces[surfaceNum]->fill(bounds, 0);
}

void OSScreenManager::resizeSurface(CVideoSurface *surface, int width, int height, int bpp) {
	DirectDrawSurface *ddSurface = _directDrawManager.createSurface(width, height, bpp, 0);
	surface->setSurface(this, ddSurface);
}

CVideoSurface *OSScreenManager::createSurface(int w, int h, int bpp) {
	DirectDrawSurface *ddSurface = _directDrawManager.createSurface(w, h, bpp, 0);
	return new OSVideoSurface(this, ddSurface);
}

CVideoSurface *OSScreenManager::createSurface(const CResourceKey &key) {
	return new OSVideoSurface(this, key);
}

void OSScreenManager::showCursor() {
	CScreenManager::_screenManagerPtr->_mouseCursor->unsuppressCursor();
}

void OSScreenManager::hideCursor() {
	CScreenManager::_screenManagerPtr->_mouseCursor->suppressCursor();
}

void OSScreenManager::destroyFrontAndBackBuffers() {
	delete _frontRenderSurface;
	_frontRenderSurface = nullptr;

	for (uint idx = 0; idx < _backSurfaces.size(); ++idx)
		delete _backSurfaces[idx]._surface;
	_backSurfaces.clear();
}

void OSScreenManager::loadCursors() {
	if (_mouseCursor) {
		hideCursor();
		delete _mouseCursor;
	}
	_mouseCursor = new CMouseCursor(this);

	if (!_textCursor) {
		_textCursor = new CTextCursor(this);
	}
}

} // End of namespace Titanic
