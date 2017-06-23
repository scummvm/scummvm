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
#include "titanic/support/transparency_surface.h"
#include "titanic/titanic.h"

namespace Titanic {

int CVideoSurface::_videoSurfaceCounter = 0;
byte CVideoSurface::_palette1[32][32];
byte CVideoSurface::_palette2[32][32];

CVideoSurface::CVideoSurface(CScreenManager *screenManager) :
		_screenManager(screenManager), _rawSurface(nullptr), _movie(nullptr),
		_pendingLoad(false), _flipVertically(false), _fastBlitFlag(false),
		_transparencySurface(nullptr), _transparencyMode(TRANS_DEFAULT),
		_freeTransparencySurface(DisposeAfterUse::NO), _hasFrame(true), _lockCount(0) {
	_videoSurfaceNum = _videoSurfaceCounter++;
}

CVideoSurface::~CVideoSurface() {
	--_videoSurfaceCounter;

	if (_freeTransparencySurface == DisposeAfterUse::YES)
		delete _transparencySurface;
}

void CVideoSurface::setupPalette(byte palette[32][32], byte val) {
	for (uint idx1 = 0; idx1 < 32; ++idx1) {
		for (uint idx2 = 0, base = 0; idx2 < 32; ++idx2, base += idx1) {
			uint v = base / 31;
			palette[idx1][idx2] = (byte)v;

			if (val != 0xff && v != idx2) {
				assert(0);
			}
		}
	}
}

void CVideoSurface::setSurface(CScreenManager *screenManager, DirectDrawSurface *surface) {
	_screenManager = screenManager;
	_ddSurface = surface;
}

void CVideoSurface::blitFrom(const Point &destPos, CVideoSurface *src, const Rect *srcRect) {
	if (loadIfReady() && src->loadIfReady() && _ddSurface && src->_ddSurface) {
		Rect srcBounds, destBounds;
		clipBounds(srcBounds, destBounds, src, srcRect, &destPos);

		if (src->_flipVertically)
			flippedBlitRect(srcBounds, destBounds, src);
		else
			blitRect(srcBounds, destBounds, src);
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

void CVideoSurface::blitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src) {
	src->lock();
	lock();

	if (src->_fastBlitFlag) {
		_rawSurface->blitFrom(*src->_rawSurface, srcRect, Point(destRect.left, destRect.top));
	} else if (src->getTransparencySurface()) {
		transBlitRect(srcRect, destRect, src, false);
	} else if (lock()) {
		if (src->lock()) {
			const Graphics::ManagedSurface *srcSurface = src->_rawSurface;
			Graphics::ManagedSurface *destSurface = _rawSurface;
			const uint transColor = src->getTransparencyColor();

			destSurface->transBlitFrom(*srcSurface, srcRect, destRect, transColor);

			src->unlock();
		}

		unlock();
	}
}

void CVideoSurface::flippedBlitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src) {
	if (src->getTransparencySurface()) {
		transBlitRect(srcRect, destRect, src, true);
	} else if (lock()) {
		if (src->lock()) {
			Graphics::ManagedSurface *srcSurface = src->_rawSurface;
			Graphics::ManagedSurface *destSurface = _rawSurface;
			const Graphics::Surface srcArea = srcSurface->getSubArea(srcRect);
			const uint transColor = src->getTransparencyColor();

			// Vertically flip the source area
			Graphics::ManagedSurface flippedArea(srcArea.w, srcArea.h, srcArea.format);
			for (int y = 0; y < srcArea.h; ++y) {
				const byte *pSrc = (const byte *)srcArea.getBasePtr(0, y);
				byte *pDest = (byte *)flippedArea.getBasePtr(0, flippedArea.h - y - 1);
				Common::copy(pSrc, pSrc + srcArea.pitch, pDest);
			}

			destSurface->transBlitFrom(flippedArea,
				Common::Point(destRect.left, destRect.top), transColor);

			src->unlock();
		}

		unlock();
	}
}

void CVideoSurface::transBlitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src, bool flipFlag) {
	assert(srcRect.width() == destRect.width() && srcRect.height() == destRect.height());
	assert(src->getPixelDepth() == 2);

	if (lock()) {
		if (src->lock()) {
			Graphics::ManagedSurface *srcSurface = src->_rawSurface;
			Graphics::ManagedSurface *destSurface = _rawSurface;
			Graphics::Surface destArea = destSurface->getSubArea(destRect);

			const uint16 *srcPtr = (const uint16 *)srcSurface->getBasePtr(
				srcRect.left, flipFlag ? srcRect.top : srcRect.bottom - 1);
			uint16 *destPtr = (uint16 *)destArea.getBasePtr(0, destArea.h - 1);
			bool isAlpha = src->_transparencyMode == TRANS_ALPHA0 ||
				src->_transparencyMode == TRANS_ALPHA255;

			CTransparencySurface transSurface(src->getTransparencySurface(), src->_transparencyMode);

			for (int yCtr = 0; yCtr < srcRect.height(); ++yCtr) {
				// Prepare for copying the line
				const uint16 *lineSrcP = srcPtr;
				uint16 *lineDestP = destPtr;
				transSurface.setRow(flipFlag ? srcRect.top + yCtr : srcRect.bottom - yCtr - 1);
				transSurface.setCol(srcRect.left);

				for (int srcX = srcRect.left; srcX < srcRect.right; ++srcX) {
					if (transSurface.isPixelOpaque())
						*lineDestP = *lineSrcP;
					else if (!transSurface.isPixelTransparent())
						copyPixel(lineDestP, lineSrcP, transSurface.getAlpha() >> 3, srcSurface->format, isAlpha);

					++lineSrcP;
					++lineDestP;
					transSurface.moveX();
				}

				// Move to next line
				srcPtr = flipFlag ? srcPtr + (src->getPitch() / 2) :
					srcPtr - (src->getPitch() / 2);
				destPtr -= destArea.pitch / 2;
			}

			src->unlock();
		}

		unlock();
	}
}

uint CVideoSurface::getTransparencyColor() {
	return getPixelDepth() == 2 ? 0xf81f : 0x7c1f;
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

#define RGB_SHIFT 3
void CVideoSurface::copyPixel(uint16 *destP, const uint16 *srcP, byte alpha,
		const Graphics::PixelFormat &srcFormat, bool isAlpha) {
	const Graphics::PixelFormat destFormat = _ddSurface->getFormat();
	alpha &= 0xff;
	assert(alpha < 32);

	// Get the source color
	byte r, g, b;
	srcFormat.colorToRGB(*srcP, r, g, b);
	r >>= RGB_SHIFT;
	g >>= RGB_SHIFT;
	b >>= RGB_SHIFT;

	if (isAlpha) {
		r = _palette1[31 - alpha][r];
		g = _palette1[31 - alpha][g];
		b = _palette1[31 - alpha][b];
	}

	byte r2, g2, b2;
	destFormat.colorToRGB(*destP, r2, g2, b2);
	r2 >>= RGB_SHIFT;
	g2 >>= RGB_SHIFT;
	b2 >>= RGB_SHIFT;
	r2 = _palette1[alpha][r2];
	g2 = _palette1[alpha][g2];
	b2 = _palette1[alpha][b2];

	*destP = destFormat.RGBToColor((r + r2) << RGB_SHIFT,
		(g + g2) << RGB_SHIFT, (b + b2) << RGB_SHIFT);
}

/*------------------------------------------------------------------------*/

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

OSVideoSurface::~OSVideoSurface() {
	if (_ddSurface)
		_videoSurfaceCounter -= OSVideoSurface::freeSurface();
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

void OSVideoSurface::recreate(int width, int height, int bpp) {
	freeSurface();

	_screenManager->resizeSurface(this, width, height, bpp);
	if (_ddSurface)
		_videoSurfaceCounter += _ddSurface->getSize();
}

void OSVideoSurface::resize(int width, int height, int bpp) {
	if (!_ddSurface || _ddSurface->getWidth() != width ||
			_ddSurface->getHeight() != height)
		recreate(width, height, bpp);
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
		if (_transparencySurface) {
			// WORKAROUND: Original had the setRow _flipVertically check in reverse.
			// Pretty sure putting it the way is below is the correct way
			CTransparencySurface transSurface(&_transparencySurface->rawSurface(), _transparencyMode);
			transSurface.setRow(_flipVertically ? getHeight() - pt.y - 1 : pt.y);
			transSurface.setCol(pt.x);

			if (transSurface.isPixelTransparent())
				return getTransparencyColor();
		}

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

void OSVideoSurface::shiftColors() {
	if (!loadIfReady())
		return;

	// Currently no further processing is needed, since for ScummVM,
	// we already convert 16-bit surfaces as soon as they're loaded
}

void OSVideoSurface::clear() {
	if (!loadIfReady())
		error("Could not load resource");

	_ddSurface->fill(nullptr, 0);
}

void OSVideoSurface::playMovie(uint flags, CGameObject *obj) {
	if (loadIfReady() && _movie)
		_movie->play(flags, obj);
}

void OSVideoSurface::playMovie(uint startFrame, uint endFrame, uint flags, CGameObject *obj) {
	if (loadIfReady() && _movie) {
		_movie->play(startFrame, endFrame, flags, obj);
		_movie->pause();
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
	if (!loadIfReady() || !_flipVertically)
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

	_flipVertically = false;
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

Graphics::ManagedSurface *OSVideoSurface::dupMovieTransparency() const {
	return _movie ? _movie->duplicateTransparency() : nullptr;
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
