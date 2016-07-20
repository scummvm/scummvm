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

#include "titanic/support/video_surface.h"
#include "titanic/support/image_decoders.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

int CVideoSurface::_videoSurfaceCounter = 0;

CVideoSurface::CVideoSurface(CScreenManager *screenManager) :
		_screenManager(screenManager), _rawSurface(nullptr), _movie(nullptr),
		_pendingLoad(false), _transBlitFlag(false), _fastBlitFlag(false),
		_movieFrameSurface(nullptr), _transparencyMode(TRANS_DEFAULT), 
		_freeMovieSurface(DisposeAfterUse::NO), _hasFrame(true), _lockCount(0) {
	_videoSurfaceNum = _videoSurfaceCounter++;
}

CVideoSurface::~CVideoSurface() {
	if (_ddSurface)
		_videoSurfaceCounter -= freeSurface();
	--_videoSurfaceCounter;

	if (_freeMovieSurface == DisposeAfterUse::YES)
		delete _movieFrameSurface;
}

void CVideoSurface::setSurface(CScreenManager *screenManager, DirectDrawSurface *surface) {
	_screenManager = screenManager;
	_ddSurface = surface;
}

void CVideoSurface::blitFrom(const Point &destPos, CVideoSurface *src, const Rect *srcRect) {
	if (loadIfReady() && src->loadIfReady() && _ddSurface && src->_ddSurface) {
		Rect srcBounds, destBounds;
		clipBounds(srcBounds, destBounds, src, srcRect, &destPos);

		if (src->_transBlitFlag)
			blitRect2(srcBounds, destBounds, src);
		else
			blitRect1(srcBounds, destBounds, src);
	}
}

void CVideoSurface::blitFrom(const Point &destPos, const Graphics::Surface *src) {
	lock();
	_rawSurface->blitFrom(*src, destPos);
	unlock();
}

void CVideoSurface::clipBounds(Rect &srcRect, Rect &destRect,
		CVideoSurface *srcSurface, const Rect *subRect, const Point *destPos) {
	// Figure out initial source rect and dest rect, based on whether
	// specific subRect and/or destPos have been passed
	if (destPos) {
		destRect.left = destPos->x;
		destRect.top = destPos->y;
	} else {
		destRect.left = destRect.top = 0;
	}

	if (subRect) {
		destRect.right = destRect.left + subRect->width();
		destRect.bottom = destRect.top + subRect->height();
		srcRect = *subRect;
	} else {
		srcRect.right = srcRect.left + srcSurface->getWidth();
		srcRect.bottom = srcRect.top + srcSurface->getHeight();
		srcRect = Rect(0, 0, srcSurface->getWidth(), srcSurface->getHeight());
	}

	// Clip destination rect to be on-screen
	if (destRect.left < 0) {
		srcRect.left -= destRect.left;
		destRect.left = 0;
	}
	if (destRect.top < 0) {
		srcRect.top -= destRect.top;
		destRect.top = 0;
	}
	if (destRect.right > getWidth()) {
		srcRect.right += getWidth() - destRect.right;
		destRect.right = getWidth();
	}
	if (destRect.bottom > getHeight()) {
		srcRect.bottom += getHeight() - destRect.bottom;
		destRect.bottom = getHeight();
	}

	// Clip source rect to be within the source surface
	if (srcRect.left < 0) {
		destRect.left -= srcRect.left;
		srcRect.left = 0;
	}
	if (srcRect.top < 0) {
		destRect.top -= srcRect.top;
		srcRect.top = 0;
	}
	if (srcRect.right > srcSurface->getWidth()) {
		destRect.right += srcSurface->getWidth() - srcRect.right;
		srcRect.right = srcSurface->getWidth();
	}
	if (srcRect.bottom > srcSurface->getHeight()) {
		destRect.bottom += srcSurface->getHeight() - srcRect.bottom;
		srcRect.bottom = srcSurface->getHeight();
	}

	// Validate that the resulting rects are valid
	if (destRect.left >= destRect.right || destRect.top >= destRect.bottom
		|| srcRect.left >= srcRect.right || srcRect.top >= srcRect.bottom)
		error("Invalid rect");
}

void CVideoSurface::blitRect1(const Rect &srcRect, const Rect &destRect, CVideoSurface *src) {
	src->lock();
	lock();

	if (src->_fastBlitFlag) {
		_rawSurface->blitFrom(*src->_rawSurface, srcRect, Point(destRect.left, destRect.top));
	} else if (getMovieFrameSurface()) {
		movieBlitRect(srcRect, destRect, src);
	} else {
		_rawSurface->transBlitFrom(*src->_rawSurface, srcRect, destRect, src->getTransparencyColor());
	}

	src->unlock();
	unlock();
}

void CVideoSurface::blitRect2(const Rect &srcRect, const Rect &destRect, CVideoSurface *src) {
	if (getMovieFrameSurface()) {
		movieBlitRect(srcRect, destRect, src);
	} else {
		src->lock();
		lock();

		_rawSurface->blitFrom(*src->_rawSurface, srcRect, Point(destRect.left, destRect.top));

		src->unlock();
		unlock();
	}
}

void CVideoSurface::movieBlitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src) {
	// TODO
}

uint CVideoSurface::getTransparencyColor() {
	uint32 val = -(getPixelDepth() - 2);
	val &= 0xFFFF8400;
	val += 0xF81F;
	return val;
}

bool CVideoSurface::hasFrame() {
	if (_hasFrame) {
		_hasFrame = false;
		return true;
	} else if (_movie) {
		return _movie->hasVideoFrame();
	} else {
		return false;
	}
}

/*------------------------------------------------------------------------*/

byte OSVideoSurface::_palette1[32][32];
byte OSVideoSurface::_palette2[32][32];

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface) :
		CVideoSurface(screenManager) {
	_ddSurface = surface;
}

OSVideoSurface::OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool pendingLoad) :
		CVideoSurface(screenManager) {
	_ddSurface = nullptr;
	_pendingLoad = pendingLoad;
	
	if (_pendingLoad) {
		loadResource(key);
	} else {
		_resourceKey = key;
		load();
	}
}

void OSVideoSurface::setupPalette(byte palette[32][32], byte val) {
	for (uint idx1 = 0; idx1 < 32; ++idx1) {
		for (uint idx2 = 0, base = 0; idx2 < 32; ++idx2, base += idx1) {
			int64 v = 0x84210843;
			v *= base;
			uint v2 = (v >> 36);
			v = ((v2 >> 31) + v2) & 0xff;
			palette[idx1][idx2] = v << 3;

			if (val != 0xff && v != idx2) {
				v = 0x80808081 * v * val;
				v2 = v >> 39;
				palette[idx1][idx2] = ((v2 >> 31) + v2) << 3;
			}
		}
	}
}

void OSVideoSurface::loadResource(const CResourceKey &key) {
	_resourceKey = key;
	_pendingLoad = true;

	if (hasSurface())
		load();
}

void OSVideoSurface::loadTarga(const CResourceKey &key) {
	// Decode the image
	CTargaDecode decoder;
	decoder.decode(*this, key.getString());

	if (getPixelDepth() == 2)
		shiftColors();

	_resourceKey = key;

}

void OSVideoSurface::loadJPEG(const CResourceKey &key) {
	// Decode the image
	CJPEGDecode decoder;
	decoder.decode(*this, key.getString());

	if (getPixelDepth() == 2)
		shiftColors();

	_resourceKey = key;
}

void OSVideoSurface::loadTarga(const CString &name) {
	CResourceKey key(name);
	loadTarga(key);
}

void OSVideoSurface::loadMovie(const CResourceKey &key, bool destroyFlag) {
	// Delete any prior movie
	if (_movie) {
		delete _movie;
		_movie = nullptr;
	}

	// Create the new movie and load the first frame to the video surface
	_movie = g_vm->_movieManager.createMovie(key, this);
	_movie->setFrame(0);

	// If flagged to destroy, then immediately destroy movie instance
	if (destroyFlag) {
		delete _movie;
		_movie = nullptr;
	}

	_resourceKey = key;
}

bool OSVideoSurface::lock() {
	if (!loadIfReady())
		return false;

	++_lockCount;
	_rawSurface = _ddSurface->lock(nullptr, 0);
	return true;
}

void OSVideoSurface::unlock() {
	if (!--_lockCount) {
		if (_rawSurface)
			_ddSurface->unlock();
		_rawSurface = nullptr;
	}
}

bool OSVideoSurface::hasSurface() {
	return _ddSurface != nullptr;
}

int OSVideoSurface::getWidth() {
	if (!loadIfReady())
		error("Could not load resource");

	return _ddSurface->getWidth();
}

int OSVideoSurface::getHeight() {
	if (!loadIfReady())
		error("Could not load resource");

	return _ddSurface->getHeight();
}

int OSVideoSurface::getPitch() {
	if (!loadIfReady())
		error("Could not load resource");

	return _ddSurface->getPitch();
}

int OSVideoSurface::getBpp() {
	if (!loadIfReady())
		error("Could not load resource");

	return getPixelDepth();
}

void OSVideoSurface::recreate(int width, int height) {
	freeSurface();

	_screenManager->resizeSurface(this, width, height);
	if (_ddSurface)
		_videoSurfaceCounter += _ddSurface->getSize();
}

void OSVideoSurface::resize(int width, int height) {
	if (!_ddSurface || _ddSurface->getWidth() != width || 
			_ddSurface->getHeight() != height)
		recreate(width, height);
}

void OSVideoSurface::detachSurface() {
	_ddSurface = nullptr;
}

int OSVideoSurface::getPixelDepth() {
	if (!loadIfReady())
		error("Could not load resource");

	lock();
	
	int result = _rawSurface->format.bytesPerPixel;
	if (result == 1)
		// Paletted 8-bit images don't store the color directly in the pixels
		result = 0;

	unlock();
	return result;
}

bool OSVideoSurface::load() {
	if (!_resourceKey.scanForFile())
		return false;

	switch (_resourceKey.fileTypeSuffix()) {
	case FILETYPE_IMAGE:
		switch (_resourceKey.imageTypeSuffix()) {
		case IMAGETYPE_TARGA:
			loadTarga(_resourceKey);
			break;
		case IMAGETYPE_JPEG:
			loadJPEG(_resourceKey);
			break;
		default:
			break;
		}
		return true;

	case FILETYPE_MOVIE:
		loadMovie(_resourceKey);
		return true;

	default:
		return false;
	}
}

uint16 OSVideoSurface::getPixel(const Common::Point &pt) {
	if (!loadIfReady())
		return 0;

	if (pt.x >= 0 && pt.y >= 0 && pt.x < getWidth() && pt.y < getHeight()) {
		lock();
		uint16 pixel = *(uint16 *)_rawSurface->getBasePtr(pt.x, pt.y);
		unlock();
		return pixel;
	} else {
		return getTransparencyColor();
	}
}

void OSVideoSurface::setPixel(const Point &pt, uint pixel) {
	assert(getPixelDepth() == 2);

	uint16 *pixelP = (uint16 *)_rawSurface->getBasePtr(pt.x, pt.y);
	*pixelP = pixel;
}

void OSVideoSurface::changePixel(uint16 *pixelP, uint16 *color, byte srcVal, bool remapFlag) {
	assert(getPixelDepth() == 2);
	const Graphics::PixelFormat &destFormat = _ddSurface->getFormat();
	const Graphics::PixelFormat srcFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);

	// Get the color
	byte r, g, b;
	srcFormat.colorToRGB(*color, r, g, b);
	if (remapFlag) {
		r = _palette1[31 - srcVal][r >> 3];
		g = _palette1[31 - srcVal][g >> 3];
		b = _palette1[31 - srcVal][b >> 3];
	}

	byte r2, g2, b2;
	destFormat.colorToRGB(*pixelP, r2, g2, b2);
	r2 = _palette1[srcVal][r2 >> 3];
	g2 = _palette1[srcVal][g2 >> 3];
	b2 = _palette1[srcVal][b2 >> 3];

	*pixelP = destFormat.RGBToColor(r + r2, g + g2, b + b2);
}

void OSVideoSurface::shiftColors() {
	if (!loadIfReady())
		return;

	// Currently no further processing is needed, since for ScummVM,
	// we already convert 16-bit surfaces as soon as they're loaded
}

void OSVideoSurface::clear() {
	if (!loadIfReady())
		error("Could not load resource");

}

void OSVideoSurface::playMovie(uint flags, CGameObject *obj) {
	if (loadIfReady() && _movie)
		_movie->play(flags, obj);

	_ddSurface->fill(nullptr, 0);
}

void OSVideoSurface::playMovie(uint startFrame, uint endFrame, uint flags, CGameObject *obj) {
	if (loadIfReady() && _movie) {
		_movie->play(startFrame, endFrame, flags, obj);
	}
}

void OSVideoSurface::playMovie(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) {
	if (loadIfReady() && _movie) {
		_movie->play(startFrame, endFrame, initialFrame, flags, obj);
	}
}

void OSVideoSurface::stopMovie() {
	if (_movie)
		_movie->stop();
}

void OSVideoSurface::setMovieFrame(uint frameNumber) {
	if (loadIfReady() && _movie)
		_movie->setFrame(frameNumber);
}

void OSVideoSurface::addMovieEvent(int frameNumber, CGameObject *obj) {
	if (_movie)
		_movie->addEvent(frameNumber, obj);
}

void OSVideoSurface::setMovieFrameRate(double rate) {
	if (_movie)
		_movie->setFrameRate(rate);
}

const CMovieRangeInfoList *OSVideoSurface::getMovieRangeInfo() const {
	return _movie ? _movie->getMovieRangeInfo() : nullptr;
}

void OSVideoSurface::flipVertically(bool needsLock) {
	if (!loadIfReady() || !_transBlitFlag)
		return;

	if (needsLock)
		lock();

	byte lineBuffer[SCREEN_WIDTH * 2];
	int pitch = getBpp() * getWidth();
	assert(pitch < (SCREEN_WIDTH * 2));

	for (int yp = 0; yp < (_rawSurface->h / 2); ++yp) {
		byte *line1P = (byte *)_rawSurface->getBasePtr(0, yp);
		byte *line2P = (byte *)_rawSurface->getBasePtr(0, _rawSurface->h - yp - 1);

		Common::copy(line1P, line1P + pitch, lineBuffer);
		Common::copy(line2P, line2P + pitch, line1P);
		Common::copy(lineBuffer, lineBuffer + pitch, line1P);
	}

	_transBlitFlag = false;
	if (needsLock)
		unlock();
}

bool OSVideoSurface::loadIfReady() {
	_videoSurfaceNum = _videoSurfaceCounter;

	if (hasSurface()) {
		return true;
	} else if (_pendingLoad) {
		_hasFrame = true;
		load();
		return true;
	} else {
		return false;
	}
}

void OSVideoSurface::transPixelate() {
	if (!loadIfReady())
		return;

	lock();
	Graphics::ManagedSurface *surface = _rawSurface;
	uint transColor = getTransparencyColor();
	// TODO: Check whether color is correct
	uint pixelColor = surface->format.RGBToColor(0x50, 0, 0);

	for (int yp = 0; yp < surface->h; ++yp) {
		uint16 *pixelsP = (uint16 *)surface->getBasePtr(0, yp);
		bool bitFlag = (yp % 2) == 0;
		int replaceCtr = yp & 3;

		for (int xp = 0; xp < surface->w; ++xp, ++pixelsP) {
			if (bitFlag && *pixelsP == transColor && replaceCtr == 0)
				*pixelsP = pixelColor;

			bitFlag = !bitFlag;
			replaceCtr = (replaceCtr + 1) & 3;
		}
	}

	surface->markAllDirty();
	unlock();
}

Graphics::ManagedSurface *OSVideoSurface::dupMovieFrame() const {
	return _movie ? _movie->duplicateFrame() : nullptr;
}

int OSVideoSurface::freeSurface() {
	if (!_ddSurface)
		return 0;
	int surfaceSize = _ddSurface->getSize();

	delete _movie;
	_movie = nullptr;
	delete _ddSurface;
	_ddSurface = nullptr;

	return surfaceSize;
}

uint16 *OSVideoSurface::getBasePtr(int x, int y) {
	assert(_rawSurface);
	return (uint16 *)_rawSurface->getBasePtr(x, y);
}

} // End of namespace Titanic
