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

#include "common/scummsys.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/text.h"

#include "zvision/file/lzss_read_stream.h"

#include "common/file.h"
#include "common/system.h"
#include "common/stream.h"

#include "engines/util.h"

#include "image/tga.h"

namespace ZVision {

RenderManager::RenderManager(ZVision *engine, uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat, bool doubleFPS)
	: _engine(engine),
	  _system(engine->_system),
	  _screenCenterX(_workingWindow.width() / 2),
	  _screenCenterY(_workingWindow.height() / 2),
	  _workingWindow(workingWindow),
	  _pixelFormat(pixelFormat),
	  _backgroundWidth(0),
	  _backgroundHeight(0),
	  _backgroundOffset(0),
	  _renderTable(_workingWindow.width(), _workingWindow.height()),
	  _doubleFPS(doubleFPS),
	  _subid(0) {

	_backgroundSurface.create(_workingWindow.width(), _workingWindow.height(), _pixelFormat);
	_effectSurface.create(_workingWindow.width(), _workingWindow.height(), _pixelFormat);
	_warpedSceneSurface.create(_workingWindow.width(), _workingWindow.height(), _pixelFormat);
	_menuSurface.create(windowWidth, workingWindow.top, _pixelFormat);

	_menuArea = Common::Rect(0, 0, windowWidth, workingWindow.top);

	initSubArea(windowWidth, windowHeight, workingWindow);
}

RenderManager::~RenderManager() {
	_currentBackgroundImage.free();
	_backgroundSurface.free();
	_effectSurface.free();
	_warpedSceneSurface.free();
	_menuSurface.free();
	_subtitleSurface.free();
}

void RenderManager::renderSceneToScreen() {
	Graphics::Surface *out = &_warpedSceneSurface;
	Graphics::Surface *in = &_backgroundSurface;
	Common::Rect outWndDirtyRect;

	// If we have graphical effects, we apply them using a temporary buffer
	if (!_effects.empty()) {
		bool copied = false;
		Common::Rect windowRect(_workingWindow.width(), _workingWindow.height());

		for (EffectsList::iterator it = _effects.begin(); it != _effects.end(); it++) {
			Common::Rect rect = (*it)->getRegion();
			Common::Rect screenSpaceLocation = rect;

			if ((*it)->isPort()) {
				screenSpaceLocation = transformBackgroundSpaceRectToScreenSpace(screenSpaceLocation);
			}

			if (windowRect.intersects(screenSpaceLocation)) {
				if (!copied) {
					copied = true;
					_effectSurface.copyFrom(_backgroundSurface);
					in = &_effectSurface;
				}
				const Graphics::Surface *post;
				if ((*it)->isPort())
					post = (*it)->draw(_currentBackgroundImage.getSubArea(rect));
				else
					post = (*it)->draw(_effectSurface.getSubArea(rect));
				Common::Rect empty;
				blitSurfaceToSurface(*post, empty, _effectSurface, screenSpaceLocation.left, screenSpaceLocation.top);
				screenSpaceLocation.clip(windowRect);
				if (_backgroundSurfaceDirtyRect .isEmpty()) {
					_backgroundSurfaceDirtyRect = screenSpaceLocation;
				} else {
					_backgroundSurfaceDirtyRect.extend(screenSpaceLocation);
				}
			}
		}
	}

	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::PANORAMA || state == RenderTable::TILT) {
		if (!_backgroundSurfaceDirtyRect.isEmpty()) {
			_renderTable.mutateImage(&_warpedSceneSurface, in);
			out = &_warpedSceneSurface;
			outWndDirtyRect = Common::Rect(_workingWindow.width(), _workingWindow.height());
		}
	} else {
		out = in;
		outWndDirtyRect = _backgroundSurfaceDirtyRect;
	}

	if (!outWndDirtyRect.isEmpty()) {
		Common::Rect rect(
			outWndDirtyRect.left + _workingWindow.left,
			outWndDirtyRect.top + _workingWindow.top,
			outWndDirtyRect.left + _workingWindow.left + outWndDirtyRect.width(),
			outWndDirtyRect.top + _workingWindow.top + outWndDirtyRect.height()
		);
		copyToScreen(*out, rect, outWndDirtyRect.left, outWndDirtyRect.top);
	}
}

void RenderManager::copyToScreen(const Graphics::Surface &surface, Common::Rect &rect, int16 srcLeft, int16 srcTop) {
	// Convert the surface to RGB565, if needed
	Graphics::Surface *outSurface = surface.convertTo(_engine->_screenPixelFormat);
	_system->copyRectToScreen(outSurface->getBasePtr(srcLeft, srcTop),
		                        outSurface->pitch,
		                        rect.left,
		                        rect.top,
		                        rect.width(),
		                        rect.height());
	outSurface->free();
	delete outSurface;
}

void RenderManager::renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	blitSurfaceToBkg(surface, destX, destY);
	surface.free();
}

void RenderManager::renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY, uint32 keycolor) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	blitSurfaceToBkg(surface, destX, destY, keycolor);
	surface.free();
}

void RenderManager::renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY, int16  keyX, int16 keyY) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	uint16 keycolor = *(uint16 *)surface.getBasePtr(keyX, keyY);

	blitSurfaceToBkg(surface, destX, destY, keycolor);
	surface.free();
}

void RenderManager::readImageToSurface(const Common::String &fileName, Graphics::Surface &destination) {
	bool isTransposed = _renderTable.getRenderState() == RenderTable::PANORAMA;
	readImageToSurface(fileName, destination, isTransposed);
}

void RenderManager::readImageToSurface(const Common::String &fileName, Graphics::Surface &destination, bool transposed) {
	Common::File file;

	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		warning("Could not open file %s", fileName.c_str());
		return;
	}

	// Read the magic number
	// Some files are true TGA, while others are TGZ
	uint32 fileType = file.readUint32BE();

	uint32 imageWidth;
	uint32 imageHeight;
	Image::TGADecoder tga;
	uint16 *buffer;
	// All Z-Vision images are in RGB 555
	destination.format = _engine->_resourcePixelFormat;

	bool isTGZ;

	// Check for TGZ files
	if (fileType == MKTAG('T', 'G', 'Z', '\0')) {
		isTGZ = true;

		// TGZ files have a header and then Bitmap data that is compressed with LZSS
		uint32 decompressedSize = file.readSint32LE() / 2;
		imageWidth = file.readSint32LE();
		imageHeight = file.readSint32LE();

		LzssReadStream lzssStream(&file);
		buffer = (uint16 *)(new uint16[decompressedSize]);
		lzssStream.read(buffer, 2 * decompressedSize);
#ifndef SCUMM_LITTLE_ENDIAN
		for (uint32 i = 0; i < decompressedSize; ++i)
			buffer[i] = FROM_LE_16(buffer[i]);
#endif
	} else {
		isTGZ = false;

		// Reset the cursor
		file.seek(0);

		// Decode
		if (!tga.loadStream(file)) {
			warning("Error while reading TGA image");
			return;
		}

		Graphics::Surface tgaSurface = *(tga.getSurface());
		imageWidth = tgaSurface.w;
		imageHeight = tgaSurface.h;

		buffer = (uint16 *)tgaSurface.getPixels();
	}

	// Flip the width and height if transposed
	if (transposed) {
		uint16 temp = imageHeight;
		imageHeight = imageWidth;
		imageWidth = temp;
	}

	// If the destination internal buffer is the same size as what we're copying into it,
	// there is no need to free() and re-create
	if (imageWidth != destination.w || imageHeight != destination.h) {
		destination.create(imageWidth, imageHeight, _engine->_resourcePixelFormat);
	}

	// If transposed, 'un-transpose' the data while copying it to the destination
	// Otherwise, just do a simple copy
	if (transposed) {
		uint16 *dest = (uint16 *)destination.getPixels();

		for (uint32 y = 0; y < imageHeight; ++y) {
			uint32 columnIndex = y * imageWidth;

			for (uint32 x = 0; x < imageWidth; ++x) {
				dest[columnIndex + x] = buffer[x * imageHeight + y];
			}
		}
	} else {
		memcpy(destination.getPixels(), buffer, imageWidth * imageHeight * destination.format.bytesPerPixel);
	}

	// Cleanup
	if (isTGZ) {
		delete[] buffer;
	} else {
		tga.destroy();
	}
}

const Common::Point RenderManager::screenSpaceToImageSpace(const Common::Point &point) {
	if (_workingWindow.contains(point)) {
		// Convert from screen space to working window space
		Common::Point newPoint(point - Common::Point(_workingWindow.left, _workingWindow.top));

		RenderTable::RenderState state = _renderTable.getRenderState();
		if (state == RenderTable::PANORAMA || state == RenderTable::TILT) {
			newPoint = _renderTable.convertWarpedCoordToFlatCoord(newPoint);
		}

		if (state == RenderTable::PANORAMA) {
			newPoint += (Common::Point(_backgroundOffset - _screenCenterX, 0));
		} else if (state == RenderTable::TILT) {
			newPoint += (Common::Point(0, _backgroundOffset - _screenCenterY));
		}

		if (_backgroundWidth)
			newPoint.x %= _backgroundWidth;
		if (_backgroundHeight)
			newPoint.y %= _backgroundHeight;

		if (newPoint.x < 0)
			newPoint.x += _backgroundWidth;
		if (newPoint.y < 0)
			newPoint.y += _backgroundHeight;

		return newPoint;
	} else {
		return Common::Point(0, 0);
	}
}

RenderTable *RenderManager::getRenderTable() {
	return &_renderTable;
}

void RenderManager::setBackgroundImage(const Common::String &fileName) {
	readImageToSurface(fileName, _currentBackgroundImage);
	_backgroundWidth = _currentBackgroundImage.w;
	_backgroundHeight = _currentBackgroundImage.h;
	_backgroundDirtyRect = Common::Rect(_backgroundWidth, _backgroundHeight);
}

void RenderManager::setBackgroundPosition(int offset) {
	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::TILT || state == RenderTable::PANORAMA)
		if (_backgroundOffset != offset)
			_backgroundDirtyRect = Common::Rect(_backgroundWidth, _backgroundHeight);
	_backgroundOffset = offset;

	_engine->getScriptManager()->setStateValue(StateKey_ViewPos, offset);
}

uint32 RenderManager::getCurrentBackgroundOffset() {
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		return _backgroundOffset;
	} else if (state == RenderTable::TILT) {
		return _backgroundOffset;
	} else {
		return 0;
	}
}

Graphics::Surface *RenderManager::tranposeSurface(const Graphics::Surface *surface) {
	Graphics::Surface *tranposedSurface = new Graphics::Surface();
	tranposedSurface->create(surface->h, surface->w, surface->format);

	const uint16 *source = (const uint16 *)surface->getPixels();
	uint16 *dest = (uint16 *)tranposedSurface->getPixels();

	for (uint32 y = 0; y < tranposedSurface->h; ++y) {
		uint32 columnIndex = y * tranposedSurface->w;

		for (uint32 x = 0; x < tranposedSurface->w; ++x) {
			dest[columnIndex + x] = source[x * surface->w + y];
		}
	}

	return tranposedSurface;
}

void RenderManager::scaleBuffer(const void *src, void *dst, uint32 srcWidth, uint32 srcHeight, byte bytesPerPixel, uint32 dstWidth, uint32 dstHeight) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);

	const float  xscale = (float)srcWidth / (float)dstWidth;
	const float  yscale = (float)srcHeight / (float)dstHeight;

	if (bytesPerPixel == 1) {
		const byte *srcPtr = (const byte *)src;
		byte *dstPtr = (byte *)dst;
		for (uint32 y = 0; y < dstHeight; ++y) {
			for (uint32 x = 0; x < dstWidth; ++x) {
				*dstPtr = srcPtr[(int)(x * xscale) + (int)(y * yscale) * srcWidth];
				dstPtr++;
			}
		}
	} else if (bytesPerPixel == 2) {
		const uint16 *srcPtr = (const uint16 *)src;
		uint16 *dstPtr = (uint16 *)dst;
		for (uint32 y = 0; y < dstHeight; ++y) {
			for (uint32 x = 0; x < dstWidth; ++x) {
				*dstPtr = srcPtr[(int)(x * xscale) + (int)(y * yscale) * srcWidth];
				dstPtr++;
			}
		}
	}
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int _x, int _y) {
	Common::Rect srcRect = _srcRect;
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	srcRect.clip(src.w, src.h);
	Common::Rect dstRect = Common::Rect(-_x + srcRect.left , -_y + srcRect.top, -_x + srcRect.left + dst.w, -_y + srcRect.top + dst.h);
	srcRect.clip(dstRect);

	if (srcRect.isEmpty() || !srcRect.isValidRect())
		return;

	Graphics::Surface *srcAdapted = src.convertTo(dst.format);

	// Copy srcRect from src surface to dst surface
	const byte *srcBuffer = (const byte *)srcAdapted->getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h) {
		srcAdapted->free();
		delete srcAdapted;
		return;
	}

	byte *dstBuffer = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		memcpy(dstBuffer, srcBuffer, w * srcAdapted->format.bytesPerPixel);
		srcBuffer += srcAdapted->pitch;
		dstBuffer += dst.pitch;
	}

	srcAdapted->free();
	delete srcAdapted;
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int _x, int _y, uint32 colorkey) {
	Common::Rect srcRect = _srcRect;
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	srcRect.clip(src.w, src.h);
	Common::Rect dstRect = Common::Rect(-_x + srcRect.left , -_y + srcRect.top, -_x + srcRect.left + dst.w, -_y + srcRect.top + dst.h);
	srcRect.clip(dstRect);

	if (srcRect.isEmpty() || !srcRect.isValidRect())
		return;

	Graphics::Surface *srcAdapted = src.convertTo(dst.format);
	uint32 keycolor = colorkey & ((1 << (src.format.bytesPerPixel << 3)) - 1);

	// Copy srcRect from src surface to dst surface
	const byte *srcBuffer = (const byte *)srcAdapted->getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h) {
		srcAdapted->free();
		delete srcAdapted;
		return;
	}

	byte *dstBuffer = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		switch (srcAdapted->format.bytesPerPixel) {
		case 1: {
			const uint *srcTemp = (const uint *)srcBuffer;
			uint *dstTemp = (uint *)dstBuffer;
			for (int32 x = 0; x < w; x++) {
				if (*srcTemp != keycolor)
					*dstTemp = *srcTemp;
				srcTemp++;
				dstTemp++;
			}
		}
		break;

		case 2: {
			const uint16 *srcTemp = (const uint16 *)srcBuffer;
			uint16 *dstTemp = (uint16 *)dstBuffer;
			for (int32 x = 0; x < w; x++) {
				if (*srcTemp != keycolor)
					*dstTemp = *srcTemp;
				srcTemp++;
				dstTemp++;
			}
		}
		break;

		case 4: {
			const uint32 *srcTemp = (const uint32 *)srcBuffer;
			uint32 *dstTemp = (uint32 *)dstBuffer;
			for (int32 x = 0; x < w; x++) {
				if (*srcTemp != keycolor)
					*dstTemp = *srcTemp;
				srcTemp++;
				dstTemp++;
			}
		}
		break;

		default:
			break;
		}
		srcBuffer += srcAdapted->pitch;
		dstBuffer += dst.pitch;
	}

	srcAdapted->free();
	delete srcAdapted;
}

void RenderManager::blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, int32 colorkey) {
	Common::Rect empt;
	if (colorkey >= 0)
		blitSurfaceToSurface(src, empt, _currentBackgroundImage, x, y, colorkey);
	else
		blitSurfaceToSurface(src, empt, _currentBackgroundImage, x, y);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_backgroundDirtyRect.isEmpty())
		_backgroundDirtyRect = dirty;
	else
		_backgroundDirtyRect.extend(dirty);
}

void RenderManager::blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect, int32 colorkey) {
	if (src.w == _dstRect.width() && src.h == _dstRect.height()) {
		blitSurfaceToBkg(src, _dstRect.left, _dstRect.top, colorkey);
	} else {
		Graphics::Surface *tmp = new Graphics::Surface;
		tmp->create(_dstRect.width(), _dstRect.height(), src.format);
		scaleBuffer(src.getPixels(), tmp->getPixels(), src.w, src.h, src.format.bytesPerPixel, _dstRect.width(), _dstRect.height());
		blitSurfaceToBkg(*tmp, _dstRect.left, _dstRect.top, colorkey);
		tmp->free();
		delete tmp;
	}
}

void RenderManager::blitSurfaceToMenu(const Graphics::Surface &src, int x, int y, int32 colorkey) {
	Common::Rect empt;
	if (colorkey >= 0)
		blitSurfaceToSurface(src, empt, _menuSurface, x, y, colorkey);
	else
		blitSurfaceToSurface(src, empt, _menuSurface, x, y);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_menuSurfaceDirtyRect.isEmpty())
		_menuSurfaceDirtyRect = dirty;
	else
		_menuSurfaceDirtyRect.extend(dirty);
}

Graphics::Surface *RenderManager::getBkgRect(Common::Rect &rect) {
	Common::Rect dst = rect;
	dst.clip(_backgroundWidth, _backgroundHeight);

	if (dst.isEmpty() || !dst.isValidRect())
		return NULL;

	Graphics::Surface *srf = new Graphics::Surface;
	srf->create(dst.width(), dst.height(), _currentBackgroundImage.format);

	srf->copyRectToSurface(_currentBackgroundImage, 0, 0, Common::Rect(dst));

	return srf;
}

Graphics::Surface *RenderManager::loadImage(Common::String file) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp);
	return tmp;
}

Graphics::Surface *RenderManager::loadImage(Common::String file, bool transposed) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp, transposed);
	return tmp;
}

void RenderManager::prepareBackground() {
	_backgroundDirtyRect.clip(_backgroundWidth, _backgroundHeight);
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		// Calculate the visible portion of the background
		Common::Rect viewPort(_workingWindow.width(), _workingWindow.height());
		viewPort.translate(-(_screenCenterX - _backgroundOffset), 0);
		Common::Rect drawRect = _backgroundDirtyRect;
		drawRect.clip(viewPort);

		// Render the visible portion
		if (!drawRect.isEmpty()) {
			blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _screenCenterX - _backgroundOffset + drawRect.left, drawRect.top);
		}

		// Mark the dirty portion of the surface
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
		_backgroundSurfaceDirtyRect.translate(_screenCenterX - _backgroundOffset, 0);

		// Panorama mode allows the user to spin in circles. Therefore, we need to render
		// the portion of the image that wrapped to the other side of the screen
		if (_backgroundOffset < _screenCenterX) {
			viewPort.moveTo(-(_screenCenterX - (_backgroundOffset + _backgroundWidth)), 0);
			drawRect = _backgroundDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _screenCenterX - (_backgroundOffset + _backgroundWidth) + drawRect.left, drawRect.top);

			Common::Rect tmp = _backgroundDirtyRect;
			tmp.translate(_screenCenterX - (_backgroundOffset + _backgroundWidth), 0);
			if (!tmp.isEmpty())
				_backgroundSurfaceDirtyRect.extend(tmp);

		} else if (_backgroundWidth - _backgroundOffset < _screenCenterX) {
			viewPort.moveTo(-(_screenCenterX + _backgroundWidth - _backgroundOffset), 0);
			drawRect = _backgroundDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _screenCenterX + _backgroundWidth - _backgroundOffset + drawRect.left, drawRect.top);

			Common::Rect tmp = _backgroundDirtyRect;
			tmp.translate(_screenCenterX + _backgroundWidth - _backgroundOffset, 0);
			if (!tmp.isEmpty())
				_backgroundSurfaceDirtyRect.extend(tmp);

		}
	} else if (state == RenderTable::TILT) {
		// Tilt doesn't allow wrapping, so we just do a simple clip
		Common::Rect viewPort(_workingWindow.width(), _workingWindow.height());
		viewPort.translate(0, -(_screenCenterY - _backgroundOffset));
		Common::Rect drawRect = _backgroundDirtyRect;
		drawRect.clip(viewPort);
		if (!drawRect.isEmpty())
			blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, drawRect.left, _screenCenterY - _backgroundOffset + drawRect.top);

		// Mark the dirty portion of the surface
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
		_backgroundSurfaceDirtyRect.translate(0, _screenCenterY - _backgroundOffset);

	} else {
		if (!_backgroundDirtyRect.isEmpty())
			blitSurfaceToSurface(_currentBackgroundImage, _backgroundDirtyRect, _backgroundSurface, _backgroundDirtyRect.left, _backgroundDirtyRect.top);
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
	}

	// Clear the dirty rect since everything is clean now
	_backgroundDirtyRect = Common::Rect();

	_backgroundSurfaceDirtyRect.clip(_workingWindow.width(), _workingWindow.height());
}

void RenderManager::clearMenuSurface() {
	_menuSurfaceDirtyRect = Common::Rect(0, 0, _menuSurface.w, _menuSurface.h);
	_menuSurface.fillRect(_menuSurfaceDirtyRect, 0);
}

void RenderManager::clearMenuSurface(const Common::Rect &r) {
	if (_menuSurfaceDirtyRect.isEmpty())
		_menuSurfaceDirtyRect = r;
	else
		_menuSurfaceDirtyRect.extend(r);
	_menuSurface.fillRect(r, 0);
}

void RenderManager::renderMenuToScreen() {
	if (!_menuSurfaceDirtyRect.isEmpty()) {
		_menuSurfaceDirtyRect.clip(Common::Rect(_menuSurface.w, _menuSurface.h));
		if (!_menuSurfaceDirtyRect.isEmpty()) {
			Common::Rect rect(
				_menuSurfaceDirtyRect.left + _menuArea.left,
				_menuSurfaceDirtyRect.top + _menuArea.top,
				_menuSurfaceDirtyRect.left + _menuArea.left + _menuSurfaceDirtyRect.width(),
				_menuSurfaceDirtyRect.top + _menuArea.top + _menuSurfaceDirtyRect.height()
			);
			copyToScreen(_menuSurface, rect, _menuSurfaceDirtyRect.left, _menuSurfaceDirtyRect.top);
		}
		_menuSurfaceDirtyRect = Common::Rect();
	}
}

void RenderManager::initSubArea(uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow) {
	_workingWindow = workingWindow;

	_subtitleSurface.free();

	_subtitleSurface.create(windowWidth, windowHeight - workingWindow.bottom, _pixelFormat);
	_subtitleArea = Common::Rect(0, workingWindow.bottom, windowWidth, windowHeight);
}

uint16 RenderManager::createSubArea(const Common::Rect &area) {
	_subid++;

	OneSubtitle sub;
	sub.redraw = false;
	sub.timer = -1;
	sub.todelete = false;
	sub.r = area;

	_subsList[_subid] = sub;

	return _subid;
}

uint16 RenderManager::createSubArea() {
	Common::Rect r(_subtitleArea.left, _subtitleArea.top, _subtitleArea.right, _subtitleArea.bottom);
	r.translate(-_workingWindow.left, -_workingWindow.top);
	return createSubArea(r);
}

void RenderManager::deleteSubArea(uint16 id) {
	if (_subsList.contains(id))
		_subsList[id].todelete = true;
}

void RenderManager::deleteSubArea(uint16 id, int16 delay) {
	if (_subsList.contains(id))
		_subsList[id].timer = delay;
}

void RenderManager::updateSubArea(uint16 id, const Common::String &txt) {
	if (_subsList.contains(id)) {
		OneSubtitle *sub = &_subsList[id];
		sub->txt = txt;
		sub->redraw = true;
	}
}

void RenderManager::processSubs(uint16 deltatime) {
	bool redraw = false;
	for (SubtitleMap::iterator it = _subsList.begin(); it != _subsList.end(); it++) {
		if (it->_value.timer != -1) {
			it->_value.timer -= deltatime;
			if (it->_value.timer <= 0)
				it->_value.todelete = true;
		}
		if (it->_value.todelete) {
			_subsList.erase(it);
			redraw = true;
		} else if (it->_value.redraw) {
			redraw = true;
		}
	}

	if (redraw) {
		_subtitleSurface.fillRect(Common::Rect(_subtitleSurface.w, _subtitleSurface.h), 0);

		for (SubtitleMap::iterator it = _subsList.begin(); it != _subsList.end(); it++) {
			OneSubtitle *sub = &it->_value;
			if (sub->txt.size()) {
				Graphics::Surface subtitleSurface;
				subtitleSurface.create(sub->r.width(), sub->r.height(), _engine->_resourcePixelFormat);
				_engine->getTextRenderer()->drawTextWithWordWrapping(sub->txt, subtitleSurface);
				Common::Rect empty;
				blitSurfaceToSurface(subtitleSurface, empty, _subtitleSurface, sub->r.left - _subtitleArea.left + _workingWindow.left, sub->r.top - _subtitleArea.top + _workingWindow.top);
				subtitleSurface.free();
			}
			sub->redraw = false;
		}

		Common::Rect rect(
			_subtitleArea.left,
			_subtitleArea.top,
			_subtitleArea.left + _subtitleSurface.w,
			_subtitleArea.top + _subtitleSurface.h
		);
		copyToScreen(_subtitleSurface, rect, 0, 0);
	}
}

Common::Point RenderManager::getBkgSize() {
	return Common::Point(_backgroundWidth, _backgroundHeight);
}

void RenderManager::addEffect(GraphicsEffect *_effect) {
	_effects.push_back(_effect);
}

void RenderManager::deleteEffect(uint32 ID) {
	for (EffectsList::iterator it = _effects.begin(); it != _effects.end(); it++) {
		if ((*it)->getKey() == ID) {
			delete *it;
			it = _effects.erase(it);
		}
	}
}

Common::Rect RenderManager::transformBackgroundSpaceRectToScreenSpace(const Common::Rect &src) {
	Common::Rect tmp = src;
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		if (_backgroundOffset < _screenCenterX) {
			Common::Rect rScreen(_screenCenterX + _backgroundOffset, _workingWindow.height());
			Common::Rect lScreen(_workingWindow.width() - rScreen.width(), _workingWindow.height());
			lScreen.translate(_backgroundWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (rScreen.width() < lScreen.width()) {
				tmp.translate(_screenCenterX - _backgroundOffset - _backgroundWidth, 0);
			} else {
				tmp.translate(_screenCenterX - _backgroundOffset, 0);
			}
		} else if (_backgroundWidth - _backgroundOffset < _screenCenterX) {
			Common::Rect rScreen(_screenCenterX - (_backgroundWidth - _backgroundOffset), _workingWindow.height());
			Common::Rect lScreen(_workingWindow.width() - rScreen.width(), _workingWindow.height());
			lScreen.translate(_backgroundWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (lScreen.width() < rScreen.width()) {
				tmp.translate(_screenCenterX + (_backgroundWidth - _backgroundOffset), 0);
			} else {
				tmp.translate(_screenCenterX - _backgroundOffset, 0);
			}
		} else {
			tmp.translate(_screenCenterX - _backgroundOffset, 0);
		}
	} else if (state == RenderTable::TILT) {
		tmp.translate(0, (_screenCenterY - _backgroundOffset));
	}

	return tmp;
}

EffectMap *RenderManager::makeEffectMap(const Common::Point &xy, int16 depth, const Common::Rect &rect, int8 *_minComp, int8 *_maxComp) {
	Common::Rect bkgRect(_backgroundWidth, _backgroundHeight);
	if (!bkgRect.contains(xy))
		return NULL;

	if (!bkgRect.intersects(rect))
		return NULL;

	uint16 color = *(uint16 *)_currentBackgroundImage.getBasePtr(xy.x, xy.y);
	uint8 stC1, stC2, stC3;
	_currentBackgroundImage.format.colorToRGB(color, stC1, stC2, stC3);
	EffectMap *newMap = new EffectMap;

	EffectMapUnit unit;
	unit.count = 0;
	unit.inEffect = false;

	int16 w = rect.width();
	int16 h = rect.height();

	bool first = true;

	uint8 minComp = MIN(MIN(stC1, stC2), stC3);
	uint8 maxComp = MAX(MAX(stC1, stC2), stC3);

	uint8 depth8 = depth << 3;

	for (int16 j = 0; j < h; j++) {
		uint16 *pix = (uint16 *)_currentBackgroundImage.getBasePtr(rect.left, rect.top + j);
		for (int16 i = 0; i < w; i++) {
			uint16 curClr = pix[i];
			uint8 cC1, cC2, cC3;
			_currentBackgroundImage.format.colorToRGB(curClr, cC1, cC2, cC3);

			bool use = false;

			if (curClr == color)
				use = true;
			else if (curClr > color) {
				if ((cC1 - stC1 < depth8) &&
				        (cC2 - stC2 < depth8) &&
				        (cC3 - stC3 < depth8))
					use = true;
			} else { /* if (curClr < color) */
				if ((stC1 - cC1 < depth8) &&
				        (stC2 - cC2 < depth8) &&
				        (stC3 - cC3 < depth8))
					use = true;
			}

			if (first) {
				unit.inEffect = use;
				first = false;
			}

			if (use) {
				uint8 cMinComp = MIN(MIN(cC1, cC2), cC3);
				uint8 cMaxComp = MAX(MAX(cC1, cC2), cC3);
				if (cMinComp < minComp)
					minComp = cMinComp;
				if (cMaxComp > maxComp)
					maxComp = cMaxComp;
			}

			if (unit.inEffect == use)
				unit.count++;
			else {
				newMap->push_back(unit);
				unit.count = 1;
				unit.inEffect = use;
			}
		}
	}
	newMap->push_back(unit);

	if (_minComp) {
		if (minComp - depth8 < 0)
			*_minComp = -(minComp >> 3);
		else
			*_minComp = -depth;
	}
	if (_maxComp) {
		if ((int16)maxComp + (int16)depth8 > 255)
			*_maxComp = (255 - maxComp) >> 3;
		else
			*_maxComp = depth;
	}

	return newMap;
}

EffectMap *RenderManager::makeEffectMap(const Graphics::Surface &surf, uint16 transp) {
	EffectMapUnit unit;
	unit.count = 0;
	unit.inEffect = false;

	int16 w = surf.w;
	int16 h = surf.h;

	EffectMap *newMap = new EffectMap;

	bool first = true;

	for (int16 j = 0; j < h; j++) {
		const uint16 *pix = (const uint16 *)surf.getBasePtr(0, j);
		for (int16 i = 0; i < w; i++) {
			bool use = false;
			if (pix[i] != transp)
				use = true;

			if (first) {
				unit.inEffect = use;
				first = false;
			}

			if (unit.inEffect == use)
				unit.count++;
			else {
				newMap->push_back(unit);
				unit.count = 1;
				unit.inEffect = use;
			}
		}
	}
	newMap->push_back(unit);

	return newMap;
}

void RenderManager::markDirty() {
	_backgroundDirtyRect = Common::Rect(_backgroundWidth, _backgroundHeight);
}

#if 0
void RenderManager::bkgFill(uint8 r, uint8 g, uint8 b) {
	_currentBackgroundImage.fillRect(Common::Rect(_currentBackgroundImage.w, _currentBackgroundImage.h), _currentBackgroundImage.format.RGBToColor(r, g, b));
	markDirty();
}
#endif

void RenderManager::timedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = createSubArea();
	updateSubArea(msgid, str);
	deleteSubArea(msgid, milsecs);
}

bool RenderManager::askQuestion(const Common::String &str) {
	Graphics::Surface textSurface;
	textSurface.create(_subtitleArea.width(), _subtitleArea.height(), _engine->_resourcePixelFormat);
	_engine->getTextRenderer()->drawTextWithWordWrapping(str, textSurface);
	copyToScreen(textSurface, _subtitleArea, 0, 0);

	_engine->stopClock();

	int result = 0;

	while (result == 0) {
		Common::Event evnt;
		while (_engine->getEventManager()->pollEvent(evnt)) {
			if (evnt.type == Common::EVENT_KEYDOWN) {
				// English: yes/no
				// German: ja/nein
				// Spanish: si/no
				// French Nemesis: F4/any other key
				// French ZGI: oui/non
				// TODO: Handle this using the keymapper
				switch (evnt.kbd.keycode) {
				case Common::KEYCODE_y:
					if (_engine->getLanguage() == Common::EN_ANY)
						result = 2;
					break;
				case Common::KEYCODE_j:
					if (_engine->getLanguage() == Common::DE_DEU)
						result = 2;
					break;
				case Common::KEYCODE_s:
					if (_engine->getLanguage() == Common::ES_ESP)
						result = 2;
					break;
				case Common::KEYCODE_o:
					if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_GRANDINQUISITOR)
						result = 2;
					break;
				case Common::KEYCODE_F4:
					if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_NEMESIS)
						result = 2;
					break;
				case Common::KEYCODE_n:
					result = 1;
					break;
				default:
					if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_NEMESIS)
						result = 1;
					break;
				}
			}
		}
		_system->updateScreen();
		if (_doubleFPS)
			_system->delayMillis(33);
		else
			_system->delayMillis(66);
	}

	// Draw over the text in order to clear it
	textSurface.fillRect(Common::Rect(_subtitleArea.width(), _subtitleArea.height()), 0);
	copyToScreen(textSurface, _subtitleArea, 0, 0);

	// Free the surface
	textSurface.free();

	_engine->startClock();
	return result == 2;
}

void RenderManager::delayedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = createSubArea();
	updateSubArea(msgid, str);
	processSubs(0);
	renderSceneToScreen();
	_engine->stopClock();

	uint32 stopTime = _system->getMillis() + milsecs;
	while (_system->getMillis() < stopTime) {
		Common::Event evnt;
		while (_engine->getEventManager()->pollEvent(evnt)) {
			if (evnt.type == Common::EVENT_KEYDOWN &&
			        (evnt.kbd.keycode == Common::KEYCODE_SPACE ||
			         evnt.kbd.keycode == Common::KEYCODE_RETURN ||
			         evnt.kbd.keycode == Common::KEYCODE_ESCAPE))
				break;
		}
		_system->updateScreen();
		if (_doubleFPS)
			_system->delayMillis(33);
		else
			_system->delayMillis(66);
	}
	deleteSubArea(msgid);
	_engine->startClock();
}

void RenderManager::showDebugMsg(const Common::String &msg, int16 delay) {
	uint16 msgid = createSubArea();
	updateSubArea(msgid, msg);
	deleteSubArea(msgid, delay);
}

void RenderManager::updateRotation() {
	int16 _velocity = _engine->getMouseVelocity() + _engine->getKeyboardVelocity();
	ScriptManager *scriptManager = _engine->getScriptManager();

	if (_doubleFPS)
		_velocity /= 2;

	if (_velocity) {
		RenderTable::RenderState renderState = _renderTable.getRenderState();
		if (renderState == RenderTable::PANORAMA) {
			int16 startPosition = scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + (_renderTable.getPanoramaReverse() ? -_velocity : _velocity);

			int16 zeroPoint = _renderTable.getPanoramaZeroPoint();
			if (startPosition >= zeroPoint && newPosition < zeroPoint)
				scriptManager->setStateValue(StateKey_Rounds, scriptManager->getStateValue(StateKey_Rounds) - 1);
			if (startPosition <= zeroPoint && newPosition > zeroPoint)
				scriptManager->setStateValue(StateKey_Rounds, scriptManager->getStateValue(StateKey_Rounds) + 1);

			int16 screenWidth = getBkgSize().x;
			if (screenWidth)
				newPosition %= screenWidth;

			if (newPosition < 0)
				newPosition += screenWidth;

			setBackgroundPosition(newPosition);
		} else if (renderState == RenderTable::TILT) {
			int16 startPosition = scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + _velocity;

			int16 screenHeight = getBkgSize().y;
			int16 tiltGap = (int16)_renderTable.getTiltGap();

			if (newPosition >= (screenHeight - tiltGap))
				newPosition = screenHeight - tiltGap;
			if (newPosition <= tiltGap)
				newPosition = tiltGap;

			setBackgroundPosition(newPosition);
		}
	}
}

void RenderManager::checkBorders() {
	RenderTable::RenderState renderState = _renderTable.getRenderState();
	if (renderState == RenderTable::PANORAMA) {
		int16 startPosition = _engine->getScriptManager()->getStateValue(StateKey_ViewPos);

		int16 newPosition = startPosition;

		int16 screenWidth = getBkgSize().x;

		if (screenWidth)
			newPosition %= screenWidth;

		if (newPosition < 0)
			newPosition += screenWidth;

		if (startPosition != newPosition)
			setBackgroundPosition(newPosition);
	} else if (renderState == RenderTable::TILT) {
		int16 startPosition = _engine->getScriptManager()->getStateValue(StateKey_ViewPos);

		int16 newPosition = startPosition;

		int16 screenHeight = getBkgSize().y;
		int16 tiltGap = (int16)_renderTable.getTiltGap();

		if (newPosition >= (screenHeight - tiltGap))
			newPosition = screenHeight - tiltGap;
		if (newPosition <= tiltGap)
			newPosition = tiltGap;

		if (startPosition != newPosition)
			setBackgroundPosition(newPosition);
	}
}

void RenderManager::rotateTo(int16 _toPos, int16 _time) {
	if (_renderTable.getRenderState() != RenderTable::PANORAMA)
		return;

	if (_time == 0)
		_time = 1;

	int32 maxX = getBkgSize().x;
	int32 curX = getCurrentBackgroundOffset();
	int32 dx = 0;

	if (curX == _toPos)
		return;

	if (curX > _toPos) {
		if (curX - _toPos > maxX / 2)
			dx = (_toPos + (maxX - curX)) / _time;
		else
			dx = -(curX - _toPos) / _time;
	} else {
		if (_toPos - curX > maxX / 2)
			dx = -((maxX - _toPos) + curX) / _time;
		else
			dx = (_toPos - curX) / _time;
	}

	_engine->stopClock();

	for (int16 i = 0; i <= _time; i++) {
		if (i == _time)
			curX = _toPos;
		else
			curX += dx;

		if (curX < 0)
			curX = maxX - curX;
		else if (curX >= maxX)
			curX %= maxX;

		setBackgroundPosition(curX);

		prepareBackground();
		renderSceneToScreen();

		_system->updateScreen();

		_system->delayMillis(500 / _time);
	}

	_engine->startClock();
}

void RenderManager::upscaleRect(Common::Rect &rect) {
	rect.top = rect.top * HIRES_WINDOW_HEIGHT / WINDOW_HEIGHT;
	rect.left = rect.left * HIRES_WINDOW_WIDTH / WINDOW_WIDTH;
	rect.bottom = rect.bottom * HIRES_WINDOW_HEIGHT / WINDOW_HEIGHT;
	rect.right = rect.right * HIRES_WINDOW_WIDTH / WINDOW_WIDTH;
}

} // End of namespace ZVision
