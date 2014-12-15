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

RenderManager::RenderManager(ZVision *engine, uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat)
	: _engine(engine),
	  _system(engine->_system),
	  _wrkWidth(workingWindow.width()),
	  _wrkHeight(workingWindow.height()),
	  _screenCenterX(_wrkWidth / 2),
	  _screenCenterY(_wrkHeight / 2),
	  _workingWindow(workingWindow),
	  _pixelFormat(pixelFormat),
	  _bkgWidth(0),
	  _bkgHeight(0),
	  _bkgOff(0),
	  _renderTable(_wrkWidth, _wrkHeight) {

	_wrkWnd.create(_wrkWidth, _wrkHeight, _pixelFormat);
	_effectWnd.create(_wrkWidth, _wrkHeight, _pixelFormat);
	_outWnd.create(_wrkWidth, _wrkHeight, _pixelFormat);
	_menuWnd.create(windowWidth, workingWindow.top, _pixelFormat);
	_subWnd.create(windowWidth, windowHeight - workingWindow.bottom, _pixelFormat);

	_menuWndRect = Common::Rect(0, 0, windowWidth, workingWindow.top);
	_subWndRect = Common::Rect(0, workingWindow.bottom, windowWidth, windowHeight);

	_subid = 0;
}

RenderManager::~RenderManager() {
	_curBkg.free();
	_wrkWnd.free();
	_effectWnd.free();
	_outWnd.free();
	_menuWnd.free();
	_subWnd.free();
}

void RenderManager::renderBackbufferToScreen() {
	Graphics::Surface *out = &_outWnd;
	Graphics::Surface *in = &_wrkWnd;
	Common::Rect outWndDirtyRect;

	if (!_effects.empty()) {
		bool copied = false;
		Common::Rect windRect(_wrkWidth, _wrkHeight);
		for (effectsList::iterator it = _effects.begin(); it != _effects.end(); it++) {
			Common::Rect rect = (*it)->getRegion();
			Common::Rect scrPlace = rect;
			if ((*it)->isPort())
				scrPlace = bkgRectToScreen(scrPlace);
			if (windRect.intersects(scrPlace)) {
				if (!copied) {
					copied = true;
					_effectWnd.copyFrom(_wrkWnd);
					in = &_effectWnd;
				}
				const Graphics::Surface *post;
				if ((*it)->isPort())
					post = (*it)->draw(_curBkg.getSubArea(rect));
				else
					post = (*it)->draw(_effectWnd.getSubArea(rect));
				blitSurfaceToSurface(*post, _effectWnd, scrPlace.left, scrPlace.top);
				scrPlace.clip(windRect);
				if (_wrkWndDirtyRect .isEmpty()) {
					_wrkWndDirtyRect = scrPlace;
				} else {
					_wrkWndDirtyRect.extend(scrPlace);
				}
			}
		}
	}

	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::PANORAMA || state == RenderTable::TILT) {
		if (!_wrkWndDirtyRect.isEmpty()) {
			_renderTable.mutateImage(&_outWnd, in);
			out = &_outWnd;
			outWndDirtyRect = Common::Rect(_wrkWidth, _wrkHeight);
		}
	} else {
		out = in;
		outWndDirtyRect = _wrkWndDirtyRect;
	}

	if (!outWndDirtyRect.isEmpty()) {
		_system->copyRectToScreen(out->getBasePtr(outWndDirtyRect.left, outWndDirtyRect.top), out->pitch,
		                          outWndDirtyRect.left + _workingWindow.left,
		                          outWndDirtyRect.top + _workingWindow.top,
		                          outWndDirtyRect.width(),
		                          outWndDirtyRect.height());
	}
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
	bool isTransposed = _renderTable.getRenderState() == RenderTable::PANORAMA;
	// All ZVision images are in RGB 555
	Graphics::PixelFormat pixelFormat555 = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	destination.format = pixelFormat555;

	bool isTGZ;

	// Check for TGZ files
	if (fileType == MKTAG('T', 'G', 'Z', '\0')) {
		isTGZ = true;

		// TGZ files have a header and then Bitmap data that is compressed with LZSS
		uint32 decompressedSize = file.readSint32LE();
		imageWidth = file.readSint32LE();
		imageHeight = file.readSint32LE();

		LzssReadStream lzssStream(&file);
		buffer = (uint16 *)(new uint16[decompressedSize]);
		lzssStream.read(buffer, decompressedSize);
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
	if (isTransposed) {
		uint16 temp = imageHeight;
		imageHeight = imageWidth;
		imageWidth = temp;
	}

	// If the destination internal buffer is the same size as what we're copying into it,
	// there is no need to free() and re-create
	if (imageWidth != destination.w || imageHeight != destination.h) {
		destination.create(imageWidth, imageHeight, pixelFormat555);
	}

	// If transposed, 'un-transpose' the data while copying it to the destination
	// Otherwise, just do a simple copy
	if (isTransposed) {
		uint16 *dest = (uint16 *)destination.getPixels();

		for (uint32 y = 0; y < imageHeight; ++y) {
			uint32 columnIndex = y * imageWidth;

			for (uint32 x = 0; x < imageWidth; ++x) {
				dest[columnIndex + x] = buffer[x * imageHeight + y];
			}
		}
	} else {
		memcpy(destination.getPixels(), buffer, imageWidth * imageHeight * _pixelFormat.bytesPerPixel);
	}

	// Cleanup
	if (isTGZ) {
		delete[] buffer;
	} else {
		tga.destroy();
	}

	// Convert in place to RGB 565 from RGB 555
	destination.convertToInPlace(_pixelFormat);
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
	// All ZVision images are in RGB 555
	Graphics::PixelFormat pixelFormat555 = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	destination.format = pixelFormat555;

	bool isTGZ;

	// Check for TGZ files
	if (fileType == MKTAG('T', 'G', 'Z', '\0')) {
		isTGZ = true;

		// TGZ files have a header and then Bitmap data that is compressed with LZSS
		uint32 decompressedSize = file.readSint32LE();
		imageWidth = file.readSint32LE();
		imageHeight = file.readSint32LE();

		LzssReadStream lzssStream(&file);
		buffer = (uint16 *)(new uint16[decompressedSize]);
		lzssStream.read(buffer, decompressedSize);
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
		destination.create(imageWidth, imageHeight, pixelFormat555);
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
		memcpy(destination.getPixels(), buffer, imageWidth * imageHeight * _pixelFormat.bytesPerPixel);
	}

	// Cleanup
	if (isTGZ) {
		delete[] buffer;
	} else {
		tga.destroy();
	}

	// Convert in place to RGB 565 from RGB 555
	destination.convertToInPlace(_pixelFormat);
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
			newPoint += (Common::Point(_bkgOff - _screenCenterX, 0));
		} else if (state == RenderTable::TILT) {
			newPoint += (Common::Point(0, _bkgOff - _screenCenterY));
		}

		if (_bkgWidth)
			newPoint.x %= _bkgWidth;
		if (_bkgHeight)
			newPoint.y %= _bkgHeight;

		if (newPoint.x < 0)
			newPoint.x += _bkgWidth;
		if (newPoint.y < 0)
			newPoint.y += _bkgHeight;

		return newPoint;
	} else {
		return Common::Point(0, 0);
	}
}

RenderTable *RenderManager::getRenderTable() {
	return &_renderTable;
}

void RenderManager::setBackgroundImage(const Common::String &fileName) {
	readImageToSurface(fileName, _curBkg);
	_bkgWidth = _curBkg.w;
	_bkgHeight = _curBkg.h;
	_bkgDirtyRect = Common::Rect(_bkgWidth, _bkgHeight);
}

void RenderManager::setBackgroundPosition(int offset) {
	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::TILT || state == RenderTable::PANORAMA)
		if (_bkgOff != offset)
			_bkgDirtyRect = Common::Rect(_bkgWidth, _bkgHeight);
	_bkgOff = offset;

	_engine->getScriptManager()->setStateValue(StateKey_ViewPos, offset);
}

uint32 RenderManager::getCurrentBackgroundOffset() {
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		return _bkgOff;
	} else if (state == RenderTable::TILT) {
		return _bkgOff;
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

	if (src.format != dst.format)
		return;

	Common::Rect srcRect = _srcRect;
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	srcRect.clip(src.w, src.h);
	Common::Rect dstRect = Common::Rect(-_x + srcRect.left , -_y + srcRect.top, -_x + srcRect.left + dst.w, -_y + srcRect.top + dst.h);
	srcRect.clip(dstRect);

	if (srcRect.isEmpty() || !srcRect.isValidRect())
		return;

	// Copy srcRect from src surface to dst surface
	const byte *srcBuffer = (const byte *)src.getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h)
		return;

	byte *dstBuffer = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		memcpy(dstBuffer, srcBuffer, w * src.format.bytesPerPixel);
		srcBuffer += src.pitch;
		dstBuffer += dst.pitch;
	}
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int _x, int _y, uint32 colorkey) {

	if (src.format != dst.format)
		return;

	Common::Rect srcRect = _srcRect;
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	srcRect.clip(src.w, src.h);
	Common::Rect dstRect = Common::Rect(-_x + srcRect.left , -_y + srcRect.top, -_x + srcRect.left + dst.w, -_y + srcRect.top + dst.h);
	srcRect.clip(dstRect);

	if (srcRect.isEmpty() || !srcRect.isValidRect())
		return;

	uint32 _keycolor = colorkey & ((1 << (src.format.bytesPerPixel << 3)) - 1);

	// Copy srcRect from src surface to dst surface
	const byte *srcBuffer = (const byte *)src.getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h)
		return;

	byte *dstBuffer = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		switch (src.format.bytesPerPixel) {
		case 1: {
			const uint *srcTemp = (const uint *)srcBuffer;
			uint *dstTemp = (uint *)dstBuffer;
			for (int32 x = 0; x < w; x++) {
				if (*srcTemp != _keycolor)
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
				if (*srcTemp != _keycolor)
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
				if (*srcTemp != _keycolor)
					*dstTemp = *srcTemp;
				srcTemp++;
				dstTemp++;
			}
		}
		break;

		default:
			break;
		}
		srcBuffer += src.pitch;
		dstBuffer += dst.pitch;
	}
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int x, int y) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, dst, x, y);
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int x, int y, uint32 colorkey) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, dst, x, y, colorkey);
}

void RenderManager::blitSurfaceToBkg(const Graphics::Surface &src, int x, int y) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, _curBkg, x, y);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_bkgDirtyRect.isEmpty())
		_bkgDirtyRect = dirty;
	else
		_bkgDirtyRect.extend(dirty);
}

void RenderManager::blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, uint32 colorkey) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, _curBkg, x, y, colorkey);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_bkgDirtyRect.isEmpty())
		_bkgDirtyRect = dirty;
	else
		_bkgDirtyRect.extend(dirty);
}

void RenderManager::blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect) {
	if (src.w == _dstRect.width() && src.h == _dstRect.height())
		blitSurfaceToBkg(src, _dstRect.left, _dstRect.top);
	else {
		Graphics::Surface *tmp = new Graphics::Surface;
		tmp->create(_dstRect.width(), _dstRect.height(), src.format);
		scaleBuffer(src.getPixels(), tmp->getPixels(), src.w, src.h, src.format.bytesPerPixel, _dstRect.width(), _dstRect.height());
		blitSurfaceToBkg(*tmp, _dstRect.left, _dstRect.top);
		tmp->free();
		delete tmp;
	}
}

void RenderManager::blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect, uint32 colorkey) {
	if (src.w == _dstRect.width() && src.h == _dstRect.height())
		blitSurfaceToBkg(src, _dstRect.left, _dstRect.top, colorkey);
	else {
		Graphics::Surface *tmp = new Graphics::Surface;
		tmp->create(_dstRect.width(), _dstRect.height(), src.format);
		scaleBuffer(src.getPixels(), tmp->getPixels(), src.w, src.h, src.format.bytesPerPixel, _dstRect.width(), _dstRect.height());
		blitSurfaceToBkg(*tmp, _dstRect.left, _dstRect.top, colorkey);
		tmp->free();
		delete tmp;
	}
}

void RenderManager::blitSurfaceToMenu(const Graphics::Surface &src, int x, int y) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, _menuWnd, x, y);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_menuWndDirtyRect.isEmpty())
		_menuWndDirtyRect = dirty;
	else
		_menuWndDirtyRect.extend(dirty);
}

void RenderManager::blitSurfaceToMenu(const Graphics::Surface &src, int x, int y, uint32 colorkey) {
	Common::Rect empt;
	blitSurfaceToSurface(src, empt, _menuWnd, x, y, colorkey);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_menuWndDirtyRect.isEmpty())
		_menuWndDirtyRect = dirty;
	else
		_menuWndDirtyRect.extend(dirty);
}

Graphics::Surface *RenderManager::getBkgRect(Common::Rect &rect) {
	Common::Rect dst = rect;
	dst.clip(_bkgWidth, _bkgHeight);

	if (dst.isEmpty() || !dst.isValidRect())
		return NULL;

	Graphics::Surface *srf = new Graphics::Surface;
	srf->create(dst.width(), dst.height(), _curBkg.format);

	srf->copyRectToSurface(_curBkg, 0, 0, Common::Rect(dst));

	return srf;
}

Graphics::Surface *RenderManager::loadImage(Common::String &file) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp);
	return tmp;
}

Graphics::Surface *RenderManager::loadImage(const char *file) {
	Common::String str = Common::String(file);
	return loadImage(str);
}

Graphics::Surface *RenderManager::loadImage(Common::String &file, bool transposed) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp, transposed);
	return tmp;
}

Graphics::Surface *RenderManager::loadImage(const char *file, bool transposed) {
	Common::String str = Common::String(file);
	return loadImage(str, transposed);
}

void RenderManager::prepareBkg() {
	_bkgDirtyRect.clip(_bkgWidth, _bkgHeight);
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		Common::Rect viewPort(_wrkWidth, _wrkHeight);
		viewPort.translate(-(_screenCenterX - _bkgOff), 0);
		Common::Rect drawRect = _bkgDirtyRect;
		drawRect.clip(viewPort);

		if (!drawRect.isEmpty())
			blitSurfaceToSurface(_curBkg, drawRect, _wrkWnd, _screenCenterX - _bkgOff + drawRect.left, drawRect.top);

		_wrkWndDirtyRect = _bkgDirtyRect;
		_wrkWndDirtyRect.translate(_screenCenterX - _bkgOff, 0);

		if (_bkgOff < _screenCenterX) {
			viewPort.moveTo(-(_screenCenterX - (_bkgOff + _bkgWidth)), 0);
			drawRect = _bkgDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_curBkg, drawRect, _wrkWnd, _screenCenterX - (_bkgOff + _bkgWidth) + drawRect.left, drawRect.top);

			Common::Rect tmp = _bkgDirtyRect;
			tmp.translate(_screenCenterX - (_bkgOff + _bkgWidth), 0);
			if (!tmp.isEmpty())
				_wrkWndDirtyRect.extend(tmp);

		} else if (_bkgWidth - _bkgOff < _screenCenterX) {
			viewPort.moveTo(-(_screenCenterX + _bkgWidth - _bkgOff), 0);
			drawRect = _bkgDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_curBkg, drawRect, _wrkWnd, _screenCenterX + _bkgWidth - _bkgOff + drawRect.left, drawRect.top);

			Common::Rect tmp = _bkgDirtyRect;
			tmp.translate(_screenCenterX + _bkgWidth - _bkgOff, 0);
			if (!tmp.isEmpty())
				_wrkWndDirtyRect.extend(tmp);

		}
	} else if (state == RenderTable::TILT) {
		Common::Rect viewPort(_wrkWidth, _wrkHeight);
		viewPort.translate(0, -(_screenCenterY - _bkgOff));
		Common::Rect drawRect = _bkgDirtyRect;
		drawRect.clip(viewPort);
		if (!drawRect.isEmpty())
			blitSurfaceToSurface(_curBkg, drawRect, _wrkWnd, drawRect.left, _screenCenterY - _bkgOff + drawRect.top);

		_wrkWndDirtyRect = _bkgDirtyRect;
		_wrkWndDirtyRect.translate(0, _screenCenterY - _bkgOff);

	} else {
		if (!_bkgDirtyRect.isEmpty())
			blitSurfaceToSurface(_curBkg, _bkgDirtyRect, _wrkWnd, _bkgDirtyRect.left, _bkgDirtyRect.top);
		_wrkWndDirtyRect = _bkgDirtyRect;
	}

	_bkgDirtyRect = Common::Rect();

	_wrkWndDirtyRect.clip(_wrkWidth, _wrkHeight);
}

void RenderManager::clearMenuSurface() {
	_menuWndDirtyRect = Common::Rect(0, 0, _menuWnd.w, _menuWnd.h);
	_menuWnd.fillRect(_menuWndDirtyRect, 0);
}

void RenderManager::clearMenuSurface(const Common::Rect &r) {
	if (_menuWndDirtyRect.isEmpty())
		_menuWndDirtyRect = r;
	else
		_menuWndDirtyRect.extend(r);
	_menuWnd.fillRect(r, 0);
}

void RenderManager::renderMenuToScreen() {
	if (!_menuWndDirtyRect.isEmpty()) {
		_menuWndDirtyRect.clip(Common::Rect(_menuWnd.w, _menuWnd.h));
		if (!_menuWndDirtyRect.isEmpty())
			_system->copyRectToScreen(_menuWnd.getBasePtr(_menuWndDirtyRect.left, _menuWndDirtyRect.top), _menuWnd.pitch,
			                          _menuWndDirtyRect.left + _menuWndRect.left,
			                          _menuWndDirtyRect.top + _menuWndRect.top,
			                          _menuWndDirtyRect.width(),
			                          _menuWndDirtyRect.height());
		_menuWndDirtyRect = Common::Rect();
	}
}

uint16 RenderManager::createSubArea(const Common::Rect &area) {
	_subid++;

	oneSub sub;
	sub.redraw = false;
	sub.timer = -1;
	sub.todelete = false;
	sub.r = area;

	_subsList[_subid] = sub;

	return _subid;
}

uint16 RenderManager::createSubArea() {
	_subid++;

	oneSub sub;
	sub.redraw = false;
	sub.timer = -1;
	sub.todelete = false;
	sub.r = Common::Rect(_subWndRect.left, _subWndRect.top, _subWndRect.right, _subWndRect.bottom);
	sub.r.translate(-_workingWindow.left, -_workingWindow.top);

	_subsList[_subid] = sub;

	return _subid;
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
		oneSub *sub = &_subsList[id];
		sub->txt = txt;
		sub->redraw = true;
	}
}

void RenderManager::processSubs(uint16 deltatime) {
	bool redraw = false;
	for (subMap::iterator it = _subsList.begin(); it != _subsList.end(); it++) {
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
		_subWnd.fillRect(Common::Rect(_subWnd.w, _subWnd.h), 0);

		for (subMap::iterator it = _subsList.begin(); it != _subsList.end(); it++) {
			oneSub *sub = &it->_value;
			if (sub->txt.size()) {
				Graphics::Surface *rndr = new Graphics::Surface();
				rndr->create(sub->r.width(), sub->r.height(), _pixelFormat);
				_engine->getTextRenderer()->drawTxtInOneLine(sub->txt, *rndr);
				blitSurfaceToSurface(*rndr, _subWnd, sub->r.left - _subWndRect.left + _workingWindow.left, sub->r.top - _subWndRect.top + _workingWindow.top);
				rndr->free();
				delete rndr;
			}
			sub->redraw = false;
		}

		_system->copyRectToScreen(_subWnd.getPixels(), _subWnd.pitch,
		                          _subWndRect.left,
		                          _subWndRect.top,
		                          _subWnd.w,
		                          _subWnd.h);
	}
}

Common::Point RenderManager::getBkgSize() {
	return Common::Point(_bkgWidth, _bkgHeight);
}

void RenderManager::addEffect(Effect *_effect) {
	_effects.push_back(_effect);
}

void RenderManager::deleteEffect(uint32 ID) {
	for (effectsList::iterator it = _effects.begin(); it != _effects.end(); it++) {
		if ((*it)->getKey() == ID) {
			delete *it;
			it = _effects.erase(it);
		}
	}
}

Common::Rect RenderManager::bkgRectToScreen(const Common::Rect &src) {
	Common::Rect tmp = src;
	RenderTable::RenderState state = _renderTable.getRenderState();

	if (state == RenderTable::PANORAMA) {
		if (_bkgOff < _screenCenterX) {
			Common::Rect rScreen(_screenCenterX + _bkgOff, _wrkHeight);
			Common::Rect lScreen(_wrkWidth - rScreen.width(), _wrkHeight);
			lScreen.translate(_bkgWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (rScreen.width() < lScreen.width()) {
				tmp.translate(_screenCenterX - _bkgOff - _bkgWidth, 0);
			} else {
				tmp.translate(_screenCenterX - _bkgOff, 0);
			}
		} else if (_bkgWidth - _bkgOff < _screenCenterX) {
			Common::Rect rScreen(_screenCenterX - (_bkgWidth - _bkgOff), _wrkHeight);
			Common::Rect lScreen(_wrkWidth - rScreen.width(), _wrkHeight);
			lScreen.translate(_bkgWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (lScreen.width() < rScreen.width()) {
				tmp.translate(_screenCenterX + (_bkgWidth - _bkgOff), 0);
			} else {
				tmp.translate(_screenCenterX - _bkgOff, 0);
			}
		} else {
			tmp.translate(_screenCenterX - _bkgOff, 0);
		}
	} else if (state == RenderTable::TILT) {
		tmp.translate(0, (_screenCenterY - _bkgOff));
	}

	return tmp;
}

EffectMap *RenderManager::makeEffectMap(const Common::Point &xy, int16 depth, const Common::Rect &rect, int8 *_minComp, int8 *_maxComp) {
	Common::Rect bkgRect(_bkgWidth, _bkgHeight);
	if (!bkgRect.contains(xy))
		return NULL;

	if (!bkgRect.intersects(rect))
		return NULL;

	uint16 color = *(uint16 *)_curBkg.getBasePtr(xy.x, xy.y);
	uint8 stC1, stC2, stC3;
	_curBkg.format.colorToRGB(color, stC1, stC2, stC3);
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
		uint16 *pix = (uint16 *)_curBkg.getBasePtr(rect.left, rect.top + j);
		for (int16 i = 0; i < w; i++) {
			uint16 curClr = pix[i];
			uint8 cC1, cC2, cC3;
			_curBkg.format.colorToRGB(curClr, cC1, cC2, cC3);

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
	_bkgDirtyRect = Common::Rect(_bkgWidth, _bkgHeight);
}

void RenderManager::bkgFill(uint8 r, uint8 g, uint8 b) {
	_curBkg.fillRect(Common::Rect(_curBkg.w, _curBkg.h), _curBkg.format.RGBToColor(r, g, b));
	markDirty();
}

} // End of namespace ZVision
