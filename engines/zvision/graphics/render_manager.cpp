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
#include "zvision/text/text.h"

#include "zvision/utility/lzss_read_stream.h"

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
	_outWnd.free();
}

void RenderManager::renderBackbufferToScreen() {
	Graphics::Surface *out = &_outWnd;

	RenderTable::RenderState state = _renderTable.getRenderState();
	if (state == RenderTable::PANORAMA || state == RenderTable::TILT) {
		if (!_wrkWndDirtyRect.isEmpty()) {
			_renderTable.mutateImage(&_outWnd, &_wrkWnd);
			out = &_outWnd;
			_outWndDirtyRect = Common::Rect(_wrkWidth, _wrkHeight);
		}
	} else {
		out = &_wrkWnd;
		_outWndDirtyRect = _wrkWndDirtyRect;
	}


	if (!_outWndDirtyRect.isEmpty()) {
		_system->copyRectToScreen(out->getBasePtr(_outWndDirtyRect.left, _outWndDirtyRect.top), out->pitch,
		                          _outWndDirtyRect.left + _workingWindow.left,
		                          _outWndDirtyRect.top + _workingWindow.top,
		                          _outWndDirtyRect.width(),
		                          _outWndDirtyRect.height());

		_outWndDirtyRect = Common::Rect();
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
	const byte *src_buf = (const byte *)src.getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h)
		return;

	byte *dst_buf = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		memcpy(dst_buf, src_buf, w * src.format.bytesPerPixel);
		src_buf += src.pitch;
		dst_buf += dst.pitch;
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
	const byte *src_buf = (const byte *)src.getBasePtr(srcRect.left, srcRect.top);

	int xx = _x;
	int yy = _y;

	if (xx < 0)
		xx = 0;
	if (yy < 0)
		yy = 0;

	if (_x >= dst.w || _y >= dst.h)
		return;

	byte *dst_buf = (byte *)dst.getBasePtr(xx, yy);

	int32 w = srcRect.width();
	int32 h = srcRect.height();

	for (int32 y = 0; y < h; y++) {
		switch (src.format.bytesPerPixel) {
		case 1: {
			const uint *src_tmp = (const uint *)src_buf;
			uint *dst_tmp = (uint *)dst_buf;
			for (int32 x = 0; x < w; x++) {
				if (*src_tmp != _keycolor)
					*dst_tmp = *src_tmp;
				src_tmp++;
				dst_tmp++;
			}
		}
		break;

		case 2: {
			const uint16 *src_tmp = (const uint16 *)src_buf;
			uint16 *dst_tmp = (uint16 *)dst_buf;
			for (int32 x = 0; x < w; x++) {
				if (*src_tmp != _keycolor)
					*dst_tmp = *src_tmp;
				src_tmp++;
				dst_tmp++;
			}
		}
		break;

		case 4: {
			const uint32 *src_tmp = (const uint32 *)src_buf;
			uint32 *dst_tmp = (uint32 *)dst_buf;
			for (int32 x = 0; x < w; x++) {
				if (*src_tmp != _keycolor)
					*dst_tmp = *src_tmp;
				src_tmp++;
				dst_tmp++;
			}
		}
		break;

		default:
			break;
		}
		src_buf += src.pitch;
		dst_buf += dst.pitch;
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
	sub._r = area;

	_subsList[_subid] = sub;

	return _subid;
}

uint16 RenderManager::createSubArea() {
	_subid++;

	oneSub sub;
	sub.redraw = false;
	sub.timer = -1;
	sub.todelete = false;
	sub._r = Common::Rect(_subWndRect.left, _subWndRect.top, _subWndRect.right, _subWndRect.bottom);
	sub._r.translate(-_workingWindow.left, -_workingWindow.top);

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
		sub->_txt = txt;
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
			if (sub->_txt.size()) {
				Graphics::Surface *rndr = new Graphics::Surface();
				rndr->create(sub->_r.width(), sub->_r.height(), _pixelFormat);
				_engine->getTextRenderer()->drawTxtInOneLine(sub->_txt, *rndr);
				blitSurfaceToSurface(*rndr, _subWnd, sub->_r.left - _subWndRect.left + _workingWindow.left, sub->_r.top - _subWndRect.top + _workingWindow.top);
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

} // End of namespace ZVision
