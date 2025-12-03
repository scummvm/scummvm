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

#include "common/debug.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/blit.h"
#include "image/tga.h"
#include "zvision/detection.h"
#include "zvision/zvision.h"
#include "zvision/file/lzss_read_stream.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/text.h"

namespace ZVision {

RenderManager::RenderManager(ZVision *engine, const ScreenLayout layout, const Graphics::PixelFormat pixelFormat, bool doubleFPS, bool widescreen)
	: _engine(engine),
	  _system(engine->_system),
	  _layout(layout),
	  _screenArea(layout.screenArea),
	  _workingArea(layout.workingArea),
	  _workingAreaCenter(Common::Point(_workingArea.width() / 2, _workingArea.height() / 2)),
	  _textArea(layout.textArea),
	  _menuArea(layout.menuArea),
	  _pixelFormat(pixelFormat),
	  _backgroundWidth(0),
	  _backgroundHeight(0),
	  _backgroundOffset(0),
	  _renderTable(engine, layout.workingArea.width(), layout.workingArea.height(), pixelFormat),
	  _doubleFPS(doubleFPS),
	  _widescreen(widescreen),
	  _frameLimiter(engine->_system, doubleFPS ? 60 : 30) {
	debugC(1, kDebugGraphics, "creating render manager");
	// Define graphics modes & screen subarea geometry
	Graphics::ModeList modes;
	_textOffset = _layout.workingArea.origin() - _layout.textArea.origin();
	modes.push_back(Graphics::Mode(_screenArea.width(), _screenArea.height()));
#if defined(USE_MPEG2) && defined(USE_A52)
	if (_engine->getGameId() == GID_GRANDINQUISITOR && (_engine->getFeatures() & ADGF_DVD)) {
		if (_widescreen)
			modes.push_back(Graphics::Mode(_HDscreenAreaWide.width(), _HDscreenAreaWide.height()));
		else
			modes.push_back(Graphics::Mode(_HDscreenArea.width(), _HDscreenArea.height()));
	}
#endif
	initGraphicsModes(modes);
	// Create backbuffers
	_backgroundSurface.create(_workingArea.width(), _workingArea.height(), _pixelFormat);
	_effectSurface.create(_workingArea.width(), _workingArea.height(), _pixelFormat);
	_warpedSceneSurface.create(_workingArea.width(), _workingArea.height(), _pixelFormat);
	_menuSurface.create(_menuArea.width(), _menuArea.height(), _pixelFormat);
	_textSurface.create(_textArea.width(), _textArea.height(), _pixelFormat);
	debugC(1, kDebugGraphics, "render manager created");
	initialize(false);
}

RenderManager::~RenderManager() {
	_currentBackgroundImage.free();
	_backgroundSurface.free();
	_workingManagedSurface.free();
	_effectSurface.free();
	_warpedSceneSurface.free();
	_menuSurface.free();
	_menuManagedSurface.free();
	_textSurface.free();
	_textManagedSurface.free();
	_screen.free();
}

void RenderManager::initialize(bool hiRes) {
	debugC(1, kDebugGraphics, "Initializing render manager");
	_hiRes = hiRes;

	_screenArea = _layout.screenArea;
	_workingArea = _layout.workingArea;
	_textArea = _layout.textArea;
	_menuArea = _layout.menuArea;

	if (_widescreen) {
		_workingArea.moveTo(0, 0);
		_screenArea = _workingArea;
		_menuArea.moveTo(_workingArea.origin());
		_menuLetterbox.moveTo(_menuArea.origin());
		_textArea.moveTo(_workingArea.left, _workingArea.bottom - _textArea.height());
		_textLetterbox.moveTo(_textArea.origin());
	}

	// Screen
#if defined(USE_MPEG2) && defined(USE_A52)
	if (_hiRes) {
		debugC(1, kDebugGraphics, "Switching to high resolution");
		upscaleRect(_screenArea);
		upscaleRect(_workingArea);
		upscaleRect(_textArea);
	} else
		debugC(1, kDebugGraphics, "Switching to standard resolution");
#endif
	_screen.create(_screenArea.width(), _screenArea.height(), _engine->_screenPixelFormat);
	_screen.setTransparentColor((uint32)-1);
	_screen.clear();

	debugC(1, kDebugGraphics, "_workingAreaCenter = %d,%d", _workingAreaCenter.x, _workingAreaCenter.y);

	// Managed screen subsurfaces
	_workingManagedSurface.create(_screen, _workingArea);
	_menuManagedSurface.create(_screen, _menuArea);
	_textManagedSurface.create(_screen, _textArea);
	debugC(2, kDebugGraphics, "screen area: %d,%d,%d,%d", _screenArea.left, _screenArea.top, _screenArea.bottom, _screenArea.right);
	debugC(2, kDebugGraphics, "working area: %d,%d,%d,%d", _workingArea.left, _workingArea.top, _workingArea.bottom, _workingArea.right);
	debugC(2, kDebugGraphics, "text area: %d,%d,%d,%d", _textArea.left, _textArea.top, _textArea.bottom, _textArea.right);

	// Menu & text area dirty rectangles
	_menuOverlay = _menuArea.findIntersectingRect(_workingArea);
	if (!_menuOverlay.isEmpty() && _menuArea.left >= _workingArea.left && _menuArea.right <= _workingArea.right)
		_menuLetterbox = Common::Rect(_menuArea.left, _menuArea.top, _menuArea.right, _workingArea.top);
	else
		_menuLetterbox = _menuArea;

	_textOverlay = _textArea.findIntersectingRect(_workingArea);
	if (!_textOverlay.isEmpty() && _textArea.left >= _workingArea.left && _textArea.right <= _workingArea.right)
		_textLetterbox = Common::Rect(_textArea.left, _workingArea.bottom, _textArea.right, _textArea.bottom);
	else
		_textLetterbox = _textArea;

	debugC(2, kDebugGraphics, "text overlay area: %d,%d,%d,%d", _textOverlay.left, _textOverlay.top, _textOverlay.bottom, _textOverlay.right);
	debugC(2, kDebugGraphics, "menu overlay area: %d,%d,%d,%d", _menuOverlay.left, _menuOverlay.top, _menuOverlay.bottom, _menuOverlay.right);

	debugC(2, kDebugGraphics, "Clearing backbuffers");
	// Clear backbuffer surfaces
	clearMenuSurface(true);
	clearTextSurface(true);
	debugC(2, kDebugGraphics, "Backbuffers cleared");

	// Set hardware/window resolution
	debugC(1, kDebugGraphics, "_screen.w = %d, _screen.h = %d", _screen.w, _screen.h);
	initGraphics(_screen.w, _screen.h, &_engine->_screenPixelFormat);
	_frameLimiter.initialize();
	debugC(1, kDebugGraphics, "Render manager initialized");
}

bool RenderManager::renderSceneToScreen(bool immediate, bool overlayOnly, bool preStream) {
	debugC(5, kDebugGraphics, "\nrenderSceneToScreen%s%s%s", immediate ? ", immediate" : "", overlayOnly ? ", overlay only" : "", preStream ? ", pre-stream" : "");
	uint32 startTime = _system->getMillis();
	if (!overlayOnly) {
		Graphics::Surface *inputSurface = &_backgroundSurface;
		Common::Rect outWndDirtyRect;
		// Apply graphical effects to temporary effects buffer and/or directly to current background image, as appropriate
		if (!_effects.empty()) {
			debugC(6, kDebugGraphics, "Rendering effects");
			bool copied = false;
			const Common::Rect windowRect(_workingArea.width(), _workingArea.height());
			for (EffectsList::iterator it = _effects.begin(); it != _effects.end(); it++) {
				Common::Rect rect = (*it)->getRegion();
				Common::Rect screenSpaceLocation = rect;
				if ((*it)->isPort())
					screenSpaceLocation = transformBackgroundSpaceRectToScreenSpace(screenSpaceLocation);
				if (windowRect.intersects(screenSpaceLocation)) {
					if (!copied) {
						copied = true;
						_effectSurface.copyFrom(_backgroundSurface);
						inputSurface = &_effectSurface;
					}
					const Graphics::Surface *post;
					if ((*it)->isPort())
						post = (*it)->draw(_currentBackgroundImage.getSubArea(rect));
					else
						post = (*it)->draw(_effectSurface.getSubArea(rect));
					Common::Rect empty;
					blitSurfaceToSurface(*post, empty, _effectSurface, screenSpaceLocation.left, screenSpaceLocation.top);
					debugC(1, kDebugGraphics, "windowRect %d,%d,%d,%d, screenSpaceLocation %d,%d,%d,%d", windowRect.left, windowRect.top, windowRect.bottom, windowRect.right, screenSpaceLocation.left, screenSpaceLocation.top, screenSpaceLocation.bottom, screenSpaceLocation.right);
					screenSpaceLocation.clip(windowRect);
					if (_backgroundSurfaceDirtyRect.isEmpty())
						_backgroundSurfaceDirtyRect = screenSpaceLocation;
					else
						_backgroundSurfaceDirtyRect.extend(screenSpaceLocation);
				}
			}
			debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
		}
		// Apply panorama/tilt warp to background image
		switch (_renderTable.getRenderState()) {
		case RenderTable::PANORAMA:
		case RenderTable::TILT:
			debugC(5, kDebugGraphics, "Rendering panorama");
			if (!_backgroundSurfaceDirtyRect.isEmpty()) {
				_renderTable.mutateImage(&_warpedSceneSurface, inputSurface, _engine->getScriptManager()->getStateValue(StateKey_HighQuality));
				_outputSurface = &_warpedSceneSurface;
				outWndDirtyRect = Common::Rect(_workingArea.width(), _workingArea.height());
			}
			break;
		default:
			_outputSurface = inputSurface;
			outWndDirtyRect = _backgroundSurfaceDirtyRect;
			break;
		}
		debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
		debugC(5, kDebugGraphics, "Rendering working area");
		_workingManagedSurface.simpleBlitFrom(*_outputSurface); // TODO - use member functions of managed surface to eliminate manual juggling of dirty rectangles, above.
		debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
	}
	if (preStream && !_hiRes) {
		debugC(5, kDebugGraphics, "Pre-rendering text area for video stream");
		_workingManagedSurface.simpleBlitFrom(*_outputSurface, _textOverlay, _textOverlay.origin()); // Prevents subtitle visual corruption when streaming videos that don't fully overlap them, e.g. Nemesis sarcophagi
		return false;
	} else {
		debugC(5, kDebugGraphics, "Rendering menu");
		_menuManagedSurface.transBlitFrom(_menuSurface, (uint32)-1);
		debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
		debugC(5, kDebugGraphics, "Rendering text");
		_textManagedSurface.transBlitFrom(_textSurface, (uint32)-1);
		debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
		if (immediate) {
			_frameLimiter.startFrame();
			debugC(5, kDebugGraphics, "Updating screen, immediate");
			_screen.update();
			debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
			debugC(10, kDebugGraphics, "~renderSceneToScreen, immediate");
			return true;
		} else if (_engine->canRender()) {
			_frameLimiter.delayBeforeSwap();
			_frameLimiter.startFrame();
			debugC(5, kDebugGraphics, "Updating screen, frame limited");
			_screen.update();
			debugC(5, kDebugGraphics, "\tCumulative render time this frame: %d ms", _system->getMillis() - startTime);
			debugC(10, kDebugGraphics, "~renderSceneToScreen, frame limited");
			return true;
		} else {
			debugC(2, kDebugGraphics, "Skipping screen update; engine forbids rendering at this time.");
			return false;
		}
	}
}

Graphics::ManagedSurface &RenderManager::getVidSurface(Common::Rect dstRect) {
	dstRect.translate(_workingArea.left, _workingArea.top);  // Convert to screen coordinates
	_vidManagedSurface.create(_screen, dstRect);
	debugC(1, kDebugGraphics, "Obtaining managed video surface at %d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
	return _vidManagedSurface;
}

void RenderManager::renderImageToBackground(const Common::Path &fileName, int16 destX, int16 destY) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	blitSurfaceToBkg(surface, destX, destY);
	surface.free();
}

void RenderManager::renderImageToBackground(const Common::Path &fileName, int16 destX, int16 destY, uint32 colorkey) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	blitSurfaceToBkg(surface, destX, destY, colorkey);
	surface.free();
}

void RenderManager::renderImageToBackground(const Common::Path &fileName, int16 destX, int16 destY, int16  keyX, int16 keyY) {
	Graphics::Surface surface;
	readImageToSurface(fileName, surface);

	uint16 keycolor = *(uint16 *)surface.getBasePtr(keyX, keyY);

	blitSurfaceToBkg(surface, destX, destY, keycolor);
	surface.free();
}

void RenderManager::readImageToSurface(const Common::Path &fileName, Graphics::Surface &destination) {
	bool isTransposed = _renderTable.getRenderState() == RenderTable::PANORAMA;
	readImageToSurface(fileName, destination, isTransposed);
}

void RenderManager::readImageToSurface(const Common::Path &fileName, Graphics::Surface &destination, bool transposed) {
	Common::File file;

	if (!file.open(fileName)) {
		warning("Could not open file %s", fileName.toString().c_str());
		return;
	}

	// Read the magic number
	// Some files are true TGA, while others are TGZ
	uint32 fileType = file.readUint32BE();

	int imageWidth;
	int imageHeight;
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
		SWAP(imageWidth, imageHeight);
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

		for (int y = 0; y < imageHeight; ++y) {
			uint32 columnIndex = y * imageWidth;

			for (int x = 0; x < imageWidth; ++x) {
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
	debugC(9, kDebugGraphics, "screenSpaceToImageSpace()");
	if (_workingArea.contains(point)) {
		// Convert from screen space to working image space, i.e. panoramic background image or static image
		Common::Point newPoint(point - _workingArea.origin());
		switch (_renderTable.getRenderState()) {
		case RenderTable::PANORAMA:
			newPoint = _renderTable.convertWarpedCoordToFlatCoord(newPoint);
			newPoint += (Common::Point(_backgroundOffset - _workingAreaCenter.x, 0));
			break;
		case RenderTable::TILT:
			newPoint = _renderTable.convertWarpedCoordToFlatCoord(newPoint);
			newPoint += (Common::Point(0, _backgroundOffset - _workingAreaCenter.y));
			break;
		default:
			break;
		}


		if (_backgroundWidth)
			newPoint.x %= _backgroundWidth;
		if (_backgroundHeight)
			newPoint.y %= _backgroundHeight;

		if (newPoint.x < 0)
			newPoint.x += _backgroundWidth;
		if (newPoint.y < 0)
			newPoint.y += _backgroundHeight;
		debugC(9, kDebugGraphics, "~screenSpaceToImageSpace()");
		return newPoint;
	} else {
		debugC(9, kDebugGraphics, "~screenSpaceToImageSpace()");
		return Common::Point(0, 0);
	}
}

RenderTable *RenderManager::getRenderTable() {
	return &_renderTable;
}

void RenderManager::setBackgroundImage(const Common::Path &fileName) {
	readImageToSurface(fileName, _currentBackgroundImage);
	_backgroundWidth = _currentBackgroundImage.w;
	_backgroundHeight = _currentBackgroundImage.h;
	_backgroundDirtyRect = Common::Rect(_backgroundWidth, _backgroundHeight);
}

void RenderManager::setBackgroundPosition(int offset) {
	switch (_renderTable.getRenderState()) {
	case RenderTable::PANORAMA:
	case RenderTable::TILT:
		if (_backgroundOffset != offset)
			_backgroundDirtyRect = Common::Rect(_backgroundWidth, _backgroundHeight);
		break;
	default:
		break;
	}
	_backgroundOffset = offset;

	_engine->getScriptManager()->setStateValue(StateKey_ViewPos, offset);
}

uint32 RenderManager::getCurrentBackgroundOffset() {
	switch (_renderTable.getRenderState()) {
	case RenderTable::PANORAMA:
	case RenderTable::TILT:
		return _backgroundOffset;
	default:
		return 0;
	}
}

Graphics::Surface *RenderManager::tranposeSurface(const Graphics::Surface *surface) {
	Graphics::Surface *tranposedSurface = new Graphics::Surface();
	tranposedSurface->create(surface->h, surface->w, surface->format);

	const uint16 *source = (const uint16 *)surface->getPixels();
	uint16 *dest = (uint16 *)tranposedSurface->getPixels();

	for (int y = 0; y < tranposedSurface->h; ++y) {
		int columnIndex = y * tranposedSurface->w;

		for (int x = 0; x < tranposedSurface->w; ++x) {
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

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, int _x, int _y) {
	debugC(9, kDebugGraphics, "blitSurfaceToSurface");
	Common::Point dstPos = Common::Point(_x, _y);
	// Default to using whole source surface
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	// Clip source rectangle to within bounds of source buffer
	srcRect.clip(src.w, src.h);

	// Generate destination rectangle
	Common::Rect dstRect = Common::Rect(dst.w, dst.h);
	// Translate destination rectangle to its position relative to source rectangle
	dstRect.translate(srcRect.left - _x, srcRect.top - _y);
	// clip source rectangle to within bounds of offset destination rectangle
	srcRect.clip(dstRect);

	// Abort if nothing to blit
	if (!srcRect.isEmpty()) {
		// Convert pixel format of source to match destination
		Graphics::Surface *srcAdapted = src.convertTo(dst.format);
		// Get pointer for source buffer blit rectangle origin
		const byte *srcBuffer = (const byte *)srcAdapted->getBasePtr(srcRect.left, srcRect.top);

		// Default to blitting into origin of target surface if negative valued
		if (dstPos.x < 0)
			dstPos.x = 0;
		if (dstPos.y < 0)
			dstPos.y = 0;

		// If _x & _y lie within destination surface
		if (dstPos.x < dst.w && dstPos.y < dst.h) {
			// Get pointer for destination buffer blit rectangle origin
			byte *dstBuffer = (byte *)dst.getBasePtr(dstPos.x, dstPos.y);
			Graphics::copyBlit(dstBuffer, srcBuffer, dst.pitch, srcAdapted->pitch, srcRect.width(), srcRect.height(), srcAdapted->format.bytesPerPixel);
		}
		srcAdapted->free();
		delete srcAdapted;
	}
}

void RenderManager::blitSurfaceToSurface(const Graphics::Surface &src, Common::Rect srcRect , Graphics::Surface &dst, int _x, int _y, uint32 colorkey) {
	debugC(9, kDebugGraphics, "blitSurfaceToSurface");
	if (srcRect.isEmpty())
		srcRect = Common::Rect(src.w, src.h);
	srcRect.clip(src.w, src.h);
	Common::Rect dstRect = Common::Rect(-_x + srcRect.left , -_y + srcRect.top, -_x + srcRect.left + dst.w, -_y + srcRect.top + dst.h);
	srcRect.clip(dstRect);

	// Abort if nothing to blit
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

	if (_x < dst.w && _y < dst.h) {
		byte *dstBuffer = (byte *)dst.getBasePtr(xx, yy);
		Graphics::keyBlit(dstBuffer, srcBuffer, dst.pitch, srcAdapted->pitch, srcRect.width(), srcRect.height(), srcAdapted->format.bytesPerPixel, keycolor);
	}


	srcAdapted->free();
	delete srcAdapted;
}

void RenderManager::blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, int32 colorkey) {
	if (colorkey >= 0)
		blitSurfaceToSurface(src, _currentBackgroundImage, x, y, colorkey);
	else
		blitSurfaceToSurface(src, _currentBackgroundImage, x, y);
	Common::Rect dirty(src.w, src.h);
	dirty.translate(x, y);
	if (_backgroundDirtyRect.isEmpty())
		_backgroundDirtyRect = dirty;
	else
		_backgroundDirtyRect.extend(dirty);
}

void RenderManager::blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &dstRect, int32 colorkey) {
	if (src.w == dstRect.width() && src.h == dstRect.height()) {
		blitSurfaceToBkg(src, dstRect.left, dstRect.top, colorkey);
	} else {
		Graphics::Surface *tmp = new Graphics::Surface;
		tmp->create(dstRect.width(), dstRect.height(), src.format);
		scaleBuffer(src.getPixels(), tmp->getPixels(), src.w, src.h, src.format.bytesPerPixel, dstRect.width(), dstRect.height());
		blitSurfaceToBkg(*tmp, dstRect.left, dstRect.top, colorkey);
		tmp->free();
		delete tmp;
	}
}

void RenderManager::blitSurfaceToMenu(const Graphics::Surface &src, int16 x, int16 y, int32 colorkey) {
	blitSurfaceToSurface(src, _menuSurface, x, y, colorkey);
	Common::Rect dirty(src.w, src.h);
	dirty.moveTo(x, y);
	if (_menuSurfaceDirtyRect.isEmpty())
		_menuSurfaceDirtyRect = dirty;
	else
		_menuSurfaceDirtyRect.extend(dirty);
}

void RenderManager::clearMenuSurface(bool force, int32 colorkey) {
	if (force)
		_menuSurfaceDirtyRect = Common::Rect(_menuArea.width(), _menuArea.height());
	if (!_menuSurfaceDirtyRect.isEmpty()) {
		// Convert to local menuArea coordinates
		Common::Rect letterbox = _menuLetterbox;
		Common::Rect overlay = _menuOverlay;
		letterbox.translate(-_menuArea.left, -_menuArea.top);
		overlay.translate(-_menuArea.left, -_menuArea.top);
		Common::Rect _menuLetterboxDirty = _menuSurfaceDirtyRect.findIntersectingRect(letterbox);
		if (!_menuLetterboxDirty.isEmpty())
			_menuSurface.fillRect(_menuLetterboxDirty, 0);
		Common::Rect _menuOverlayDirty = _menuSurfaceDirtyRect.findIntersectingRect(overlay);
		if (!_menuOverlayDirty.isEmpty()) {
			_menuSurface.fillRect(_menuOverlayDirty, colorkey);
			// TODO - mark working window dirty here so that it will redraw & blank overlaid residue on next frame
		}
		_menuSurfaceDirtyRect = Common::Rect(0, 0);
	}
}

void RenderManager::blitSurfaceToText(const Graphics::Surface &src, int16 x, int16 y, int32 colorkey) {
	blitSurfaceToSurface(src, _textSurface, x, y, colorkey);
	Common::Rect dirty(src.w, src.h);
	dirty.moveTo(x, y);
	if (_textSurfaceDirtyRect.isEmpty())
		_textSurfaceDirtyRect = dirty;
	else
		_textSurfaceDirtyRect.extend(dirty);
}

void RenderManager::clearTextSurface(bool force, int32 colorkey) {
	if (force)
		_textSurfaceDirtyRect = Common::Rect(_textArea.width(), _textArea.height());
	if (!_textSurfaceDirtyRect.isEmpty()) {
		// Convert to local textArea coordinates
		Common::Rect letterbox = _textLetterbox;
		Common::Rect overlay = _textOverlay;
		letterbox.translate(-_textArea.left, -_textArea.top);
		overlay.translate(-_textArea.left, -_textArea.top);
		Common::Rect _textLetterboxDirty = _textSurfaceDirtyRect.findIntersectingRect(letterbox);
		if (!_textLetterboxDirty.isEmpty())
			_textSurface.fillRect(_textLetterboxDirty, 0);
		Common::Rect _textOverlayDirty = _textSurfaceDirtyRect.findIntersectingRect(overlay);
		if (!_textOverlayDirty.isEmpty()) {
			_textSurface.fillRect(_textOverlayDirty, colorkey);
			// TODO - mark working window dirty here so that it will redraw & blank overlaid residue on next frame
		}
		_textSurfaceDirtyRect = Common::Rect(0, 0);
	}
}

Graphics::Surface *RenderManager::getBkgRect(Common::Rect &rect) {
	debugC(11, kDebugGraphics, "getBkgRect()");
	Common::Rect dst = rect;
	dst.clip(_backgroundWidth, _backgroundHeight);

	if (dst.isEmpty() || !dst.isValidRect())
		return NULL;

	Graphics::Surface *srf = new Graphics::Surface;
	srf->create(dst.width(), dst.height(), _currentBackgroundImage.format);

	srf->copyRectToSurface(_currentBackgroundImage, 0, 0, Common::Rect(dst));

	return srf;
}

Graphics::Surface *RenderManager::loadImage(const Common::Path &file) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp);
	return tmp;
}

Graphics::Surface *RenderManager::loadImage(const Common::Path &file, bool transposed) {
	Graphics::Surface *tmp = new Graphics::Surface;
	readImageToSurface(file, *tmp, transposed);
	return tmp;
}

void RenderManager::prepareBackground() {
	debugC(5, kDebugGraphics, "prepareBackground()");
	_backgroundDirtyRect.clip(_backgroundWidth, _backgroundHeight);
	switch (_renderTable.getRenderState()) {
	case RenderTable::PANORAMA: {
		// Calculate the visible portion of the background
		Common::Rect viewPort(_workingArea.width(), _workingArea.height());
		viewPort.translate(-(_workingAreaCenter.x - _backgroundOffset), 0);
		Common::Rect drawRect = _backgroundDirtyRect;
		drawRect.clip(viewPort);

		// Render the visible portion
		if (!drawRect.isEmpty()) {
			blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _workingAreaCenter.x - _backgroundOffset + drawRect.left, drawRect.top);
		}

		// Mark the dirty portion of the surface
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
		_backgroundSurfaceDirtyRect.translate(_workingAreaCenter.x - _backgroundOffset, 0);

		// Panorama mode allows the user to spin in circles. Therefore, we need to render
		// the portion of the image that wrapped to the other side of the screen
		if (_backgroundOffset < _workingAreaCenter.x) {
			viewPort.moveTo(-(_workingAreaCenter.x - (_backgroundOffset + _backgroundWidth)), 0);
			drawRect = _backgroundDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _workingAreaCenter.x - (_backgroundOffset + _backgroundWidth) + drawRect.left, drawRect.top);

			Common::Rect tmp = _backgroundDirtyRect;
			tmp.translate(_workingAreaCenter.x - (_backgroundOffset + _backgroundWidth), 0);
			if (!tmp.isEmpty())
				_backgroundSurfaceDirtyRect.extend(tmp);

		} else if (_backgroundWidth - _backgroundOffset < _workingAreaCenter.x) {
			viewPort.moveTo(-(_workingAreaCenter.x + _backgroundWidth - _backgroundOffset), 0);
			drawRect = _backgroundDirtyRect;
			drawRect.clip(viewPort);

			if (!drawRect.isEmpty())
				blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, _workingAreaCenter.x + _backgroundWidth - _backgroundOffset + drawRect.left, drawRect.top);

			Common::Rect tmp = _backgroundDirtyRect;
			tmp.translate(_workingAreaCenter.x + _backgroundWidth - _backgroundOffset, 0);
			if (!tmp.isEmpty())
				_backgroundSurfaceDirtyRect.extend(tmp);
		}
		break;
	}
	case RenderTable::TILT: {
		// Tilt doesn't allow wrapping, so we just do a simple clip
		Common::Rect viewPort(_workingArea.width(), _workingArea.height());
		viewPort.translate(0, -(_workingAreaCenter.y - _backgroundOffset));
		Common::Rect drawRect = _backgroundDirtyRect;
		drawRect.clip(viewPort);
		if (!drawRect.isEmpty())
			blitSurfaceToSurface(_currentBackgroundImage, drawRect, _backgroundSurface, drawRect.left, _workingAreaCenter.y - _backgroundOffset + drawRect.top);

		// Mark the dirty portion of the surface
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
		_backgroundSurfaceDirtyRect.translate(0, _workingAreaCenter.y - _backgroundOffset);
		break;
	}
	default: {
		if (!_backgroundDirtyRect.isEmpty())
			blitSurfaceToSurface(_currentBackgroundImage, _backgroundDirtyRect, _backgroundSurface, _backgroundDirtyRect.left, _backgroundDirtyRect.top);
		_backgroundSurfaceDirtyRect = _backgroundDirtyRect;
		break;
	}
	}
	// Clear the dirty rect since everything is clean now
	_backgroundDirtyRect = Common::Rect();
	_backgroundSurfaceDirtyRect.clip(_workingArea.width(), _workingArea.height());
	debugC(11, kDebugGraphics, "~prepareBackground()");
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
	debugC(10, kDebugGraphics, "transformBackgroundSpaceRectToScreenSpace");
	Common::Rect tmp = src;
	switch (_renderTable.getRenderState()) {
	case RenderTable::PANORAMA: {
		if (_backgroundOffset < _workingAreaCenter.x) {
			Common::Rect rScreen(_workingAreaCenter.x + _backgroundOffset, _workingArea.height());
			Common::Rect lScreen(_workingArea.width() - rScreen.width(), _workingArea.height());
			lScreen.translate(_backgroundWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (rScreen.width() < lScreen.width())
				tmp.translate(_workingAreaCenter.x - _backgroundOffset - _backgroundWidth, 0);
			else
				tmp.translate(_workingAreaCenter.x - _backgroundOffset, 0);
		} else if (_backgroundWidth - _backgroundOffset < _workingAreaCenter.x) {
			Common::Rect rScreen(_workingAreaCenter.x - (_backgroundWidth - _backgroundOffset), _workingArea.height());
			Common::Rect lScreen(_workingArea.width() - rScreen.width(), _workingArea.height());
			lScreen.translate(_backgroundWidth - lScreen.width(), 0);
			lScreen.clip(src);
			rScreen.clip(src);
			if (lScreen.width() < rScreen.width())
				tmp.translate(_workingAreaCenter.x + (_backgroundWidth - _backgroundOffset), 0);
			else
				tmp.translate(_workingAreaCenter.x - _backgroundOffset, 0);
		} else
			tmp.translate(_workingAreaCenter.x - _backgroundOffset, 0);
		break;
	}
	case RenderTable::TILT:
		tmp.translate(0, (_workingAreaCenter.y - _backgroundOffset));
		break;
	default:
		break;
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

/*
//Only needed by ActionDissolve, which is disabled unless and until a better dissolve effect can be implemented.  Gameplay is not significantly effected.
void RenderManager::bkgFill(uint8 r, uint8 g, uint8 b) {
	_currentBackgroundImage.fillRect(Common::Rect(_currentBackgroundImage.w, _currentBackgroundImage.h), _currentBackgroundImage.format.RGBToColor(r, g, b));
	markDirty();
}
*/


void RenderManager::updateRotation() {
	int16 velocity = _engine->getMouseVelocity() + _engine->getKeyboardVelocity();
	ScriptManager *scriptManager = _engine->getScriptManager();

	if (_doubleFPS || !_frameLimiter.isEnabled()) // Assuming 60fps when in Vsync mode.
		velocity /= 2;

	if (velocity) {
		switch (_renderTable.getRenderState()) {
		case RenderTable::PANORAMA: {
			int16 startPosition = scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + (_renderTable.getPanoramaReverse() ? -velocity : velocity);

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
			break;
		}
		case RenderTable::TILT: {
			int16 startPosition = scriptManager->getStateValue(StateKey_ViewPos);

			int16 newPosition = startPosition + velocity;

			int16 screenHeight = getBkgSize().y;
			int16 tiltGap = (int16)_renderTable.getTiltGap();

			if (newPosition >= (screenHeight - tiltGap))
				newPosition = screenHeight - tiltGap;
			if (newPosition <= tiltGap)
				newPosition = tiltGap;

			setBackgroundPosition(newPosition);
			break;
		}
		default:
			break;
		}
	}
}

void RenderManager::checkBorders() {
	switch (_renderTable.getRenderState()) {
	case RenderTable::PANORAMA: {
		int16 startPosition = _engine->getScriptManager()->getStateValue(StateKey_ViewPos);

		int16 newPosition = startPosition;

		int16 screenWidth = getBkgSize().x;

		if (screenWidth)
			newPosition %= screenWidth;

		if (newPosition < 0)
			newPosition += screenWidth;

		if (startPosition != newPosition)
			setBackgroundPosition(newPosition);
		break;
	}
	case RenderTable::TILT: {
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
		break;
	}
	default:
		break;
	}
}

void RenderManager::rotateTo(int16 _toPos, int16 _time) {
	if (_renderTable.getRenderState() != RenderTable::PANORAMA)
		return;
	debugC(1, kDebugGraphics, "Rotating panorama to %d", _toPos);
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


		_system->delayMillis(500 / _time);
	}

	_engine->startClock();
}

void RenderManager::upscaleRect(Common::Rect &rect) {
	Common::Rect HDscreen = _widescreen ? _HDscreenAreaWide : _HDscreenArea;
	Common::Rect SDscreen = _widescreen ? _layout.workingArea : _layout.screenArea;
	SDscreen.moveTo(0, 0);
	rect.top = rect.top * HDscreen.height() / SDscreen.height();
	rect.left = rect.left * HDscreen.width() / SDscreen.width();
	rect.bottom = rect.bottom * HDscreen.height() / SDscreen.height();
	rect.right = rect.right * HDscreen.width() / SDscreen.width();
}

} // End of namespace ZVision
