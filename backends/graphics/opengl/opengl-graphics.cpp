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

#ifdef USE_OPENGL

#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/opengl/glerrorcheck.h"
#include "common/mutex.h"
#include "common/translation.h"

OpenGLGraphicsManager::OpenGLGraphicsManager()
	:
	_gameTexture(0), _overlayTexture(0), _cursorTexture(0),
	_screenChangeCount(0), _screenNeedsRedraw(false),
	_currentShakePos(0), _newShakePos(0),
	_overlayVisible(false), _overlayNeedsRedraw(false),
	_transactionMode(kTransactionNone),
	_cursorNeedsRedraw(false), _cursorPaletteDisabled(true),
	_cursorVisible(false), _cursorKeyColor(0),
	_cursorTargetScale(1) {

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

	_videoMode.mode = OpenGL::GFX_DOUBLESIZE;
	_videoMode.scaleFactor = 2;
	_videoMode.fullscreen = false;
	_videoMode.antialiasing = false;

	_gamePalette = (byte *)calloc(sizeof(byte) * 4, 256);
	_cursorPalette = (byte *)calloc(sizeof(byte) * 4, 256);
	
	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 2, false);
}

OpenGLGraphicsManager::~OpenGLGraphicsManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher() != NULL)
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	free(_gamePalette);
	free(_cursorPalette);

	if (_gameTexture != NULL)
		delete _gameTexture;
	if (_overlayTexture != NULL)
		delete _overlayTexture;
	if (_cursorTexture != NULL)
		delete _cursorTexture;
}

//
// Feature
//

bool OpenGLGraphicsManager::hasFeature(OSystem::Feature f) {
	return
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureCursorHasPalette);
}

void OpenGLGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable ? -1 : 0);
		break;
	default:
		break;
	}
}

bool OpenGLGraphicsManager::getFeatureState(OSystem::Feature f) {
	return false;
}

//
// Screen format and modes
//

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"gl1x", _s("OpenGL Normal"), OpenGL::GFX_NORMAL},
#ifdef USE_SCALERS
	{"gl2x", "OpenGL 2x", OpenGL::GFX_DOUBLESIZE},
	{"gl3x", "OpenGL 3x", OpenGL::GFX_TRIPLESIZE},
#endif
	{0, 0, 0}
};

const OSystem::GraphicsMode *OpenGLGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OpenGLGraphicsManager::getDefaultGraphicsMode() const {
	return OpenGL::GFX_NORMAL;
}

bool OpenGLGraphicsManager::setGraphicsMode(int mode) {
	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode)
		return true;

	int newScaleFactor = 1;

	switch (mode) {
	case OpenGL::GFX_NORMAL:
		newScaleFactor = 1;
		break;
#ifdef USE_SCALERS
	case OpenGL::GFX_DOUBLESIZE:
		newScaleFactor = 2;
		break;
	case OpenGL::GFX_TRIPLESIZE:
		newScaleFactor = 3;
		break;
#endif
	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	if (_oldVideoMode.setup && _oldVideoMode.scaleFactor != newScaleFactor)
		_transactionDetails.needHotswap = true;

	_transactionDetails.needUpdatescreen = true;

	_videoMode.mode = mode;
	_videoMode.scaleFactor = newScaleFactor;

	return true;
}

int OpenGLGraphicsManager::getGraphicsMode() const {
	assert (_transactionMode == kTransactionNone);
	return _videoMode.mode;
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat OpenGLGraphicsManager::getScreenFormat() const {
	return _screenFormat;
}

#endif

void OpenGLGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

#ifdef USE_RGB_COLOR
	// Avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		newFormat = *format;

	assert(newFormat.bytesPerPixel > 0);

	if (newFormat != _videoMode.format) {
		_videoMode.format = newFormat;
		_transactionDetails.formatChanged = true;
		_screenFormat = newFormat;
	}
#endif

	// Avoid redundant res changes
	if ((int)width == _videoMode.screenWidth && (int)height == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = width;
	_videoMode.screenHeight = height;

	_transactionDetails.sizeChanged = true;
}

int OpenGLGraphicsManager::getScreenChangeID() const {
	return _screenChangeCount;
}

//
// GFX
//

void OpenGLGraphicsManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;
	_transactionDetails.sizeChanged = false;
	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;
	_transactionDetails.newContext = false;
	_transactionDetails.filterChanged = false;
#ifdef USE_RGB_COLOR
	_transactionDetails.formatChanged = false;
#endif

	_oldVideoMode = _videoMode;
}

OSystem::TransactionError OpenGLGraphicsManager::endGFXTransaction() {
	int errors = OSystem::kTransactionSuccess;

	assert(_transactionMode != kTransactionNone);

	if (_transactionMode == kTransactionRollback) {
		if (_videoMode.fullscreen != _oldVideoMode.fullscreen) {
			errors |= OSystem::kTransactionFullscreenFailed;

			_videoMode.fullscreen = _oldVideoMode.fullscreen;
		} else if (_videoMode.aspectRatioCorrection != _oldVideoMode.aspectRatioCorrection) {
			errors |= OSystem::kTransactionAspectRatioFailed;

			_videoMode.aspectRatioCorrection = _oldVideoMode.aspectRatioCorrection;
		} else if (_videoMode.mode != _oldVideoMode.mode) {
			errors |= OSystem::kTransactionModeSwitchFailed;

			_videoMode.mode = _oldVideoMode.mode;
			_videoMode.scaleFactor = _oldVideoMode.scaleFactor;
#ifdef USE_RGB_COLOR
		} else if (_videoMode.format != _oldVideoMode.format) {
			errors |= OSystem::kTransactionFormatNotSupported;

			_videoMode.format = _oldVideoMode.format;
			_screenFormat = _videoMode.format;
#endif
		} else if (_videoMode.screenWidth != _oldVideoMode.screenWidth || _videoMode.screenHeight != _oldVideoMode.screenHeight) {
			errors |= OSystem::kTransactionSizeChangeFailed;

			_videoMode.screenWidth = _oldVideoMode.screenWidth;
			_videoMode.screenHeight = _oldVideoMode.screenHeight;
			_videoMode.overlayWidth = _oldVideoMode.overlayWidth;
			_videoMode.overlayHeight = _oldVideoMode.overlayHeight;
		}

		if (_videoMode.fullscreen == _oldVideoMode.fullscreen &&
			_videoMode.aspectRatioCorrection == _oldVideoMode.aspectRatioCorrection &&
			_videoMode.mode == _oldVideoMode.mode &&
			_videoMode.screenWidth == _oldVideoMode.screenWidth &&
		   	_videoMode.screenHeight == _oldVideoMode.screenHeight) {

			_oldVideoMode.setup = false;
		}
	}

#ifdef USE_RGB_COLOR
	if (_transactionDetails.sizeChanged || _transactionDetails.formatChanged || _transactionDetails.needHotswap) {
#else
	if (_transactionDetails.sizeChanged || _transactionDetails.needHotswap) {
#endif
		unloadGFXMode();
		if (!loadGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			clearOverlay();

			_videoMode.setup = true;
			_screenChangeCount++;
		}
	} else if (_transactionDetails.filterChanged) {
		loadTextures();
		internUpdateScreen();
	} else if (_transactionDetails.needUpdatescreen) {
		internUpdateScreen();
	}

	_transactionMode = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

//
// Screen
//

int16 OpenGLGraphicsManager::getHeight() {
	return _videoMode.screenHeight;
}

int16 OpenGLGraphicsManager::getWidth() {
	return _videoMode.screenWidth;
}

void OpenGLGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	
#ifdef USE_RGB_COLOR
	assert(_screenFormat.bytesPerPixel == 1);
#endif

	// Save the screen palette
	memcpy(_gamePalette + start * 4, colors, num * 4);

	_screenNeedsRedraw = true;

	if (_cursorPaletteDisabled)
		_cursorNeedsRedraw = true;
}

void OpenGLGraphicsManager::grabPalette(byte *colors, uint start, uint num) {
	assert(colors);
	
#ifdef USE_RGB_COLOR
	assert(_screenFormat.bytesPerPixel == 1);
#endif

	// Copies current palette to buffer
	memcpy(colors, _gamePalette + start * 4, num * 4);
}

void OpenGLGraphicsManager::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	assert(x >= 0 && x < _screenData.w);
	assert(y >= 0 && y < _screenData.h);
	assert(h > 0 && y + h <= _screenData.h);
	assert(w > 0 && x + w <= _screenData.w);

	// Copy buffer data to game screen internal buffer
	const byte *src = buf;
	byte *dst = (byte *)_screenData.pixels + y * _screenData.pitch;
	for (int i = 0; i < h; i++) {
		memcpy(dst + x * _screenData.bytesPerPixel, src, w * _screenData.bytesPerPixel);
		src += pitch;
		dst += _screenData.pitch;
	}

	if (!_screenNeedsRedraw) {
		const Common::Rect dirtyRect(x, y, x + w, y + h);
		_screenDirtyRect.extend(dirtyRect);
	}
}

Graphics::Surface *OpenGLGraphicsManager::lockScreen() {
	return &_screenData;
}

void OpenGLGraphicsManager::unlockScreen() {
	_screenNeedsRedraw = true;
}

void OpenGLGraphicsManager::fillScreen(uint32 col) {
	if (_gameTexture == NULL)
		return;

	if (_screenFormat.bytesPerPixel == 1) {
		memset(_screenData.pixels, col, _screenData.h * _screenData.pitch);
	} else if (_screenFormat.bytesPerPixel == 2) {
		uint16 *pixels = (uint16 *)_screenData.pixels;
		uint16 col16 = (uint16)col;
		for (int i = 0; i < _screenData.w * _screenData.h; i++) {
			pixels[i] = col16;
		}
	} else if (_screenFormat.bytesPerPixel == 3) {
		uint8 *pixels = (uint8 *)_screenData.pixels;
		byte r =  (col >> 16) & 0xFF;
		byte g =  (col >> 8) & 0xFF;
		byte b =  col & 0xFF;
		for (int i = 0; i < _screenData.w * _screenData.h; i++) {
			pixels[0] = r;
			pixels[1] = g;
			pixels[2] = b;
			pixels += 3;
		}
	} else if (_screenFormat.bytesPerPixel == 4) {
		uint32 *pixels = (uint32 *)_screenData.pixels;
		for (int i = 0; i < _screenData.w * _screenData.h; i++) {
			pixels[i] = col;
		}
	}

	_screenNeedsRedraw = true;
}

void OpenGLGraphicsManager::updateScreen() {
	assert (_transactionMode == kTransactionNone);
	internUpdateScreen();
}

void OpenGLGraphicsManager::setShakePos(int shakeOffset) {
	assert (_transactionMode == kTransactionNone);
	_newShakePos = shakeOffset;
}

void OpenGLGraphicsManager::setFocusRectangle(const Common::Rect& rect) {

}

void OpenGLGraphicsManager::clearFocusRectangle() {

}

//
// Overlay
//

void OpenGLGraphicsManager::showOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (_overlayVisible)
		return;

	_overlayVisible = true;

	clearOverlay();
}

void OpenGLGraphicsManager::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	_overlayVisible = false;

	clearOverlay();
}

Graphics::PixelFormat OpenGLGraphicsManager::getOverlayFormat() const {
	return _overlayFormat;
}

void OpenGLGraphicsManager::clearOverlay() {
	memset(_overlayData.pixels, 0, _overlayData.h * _overlayData.pitch);
	_overlayNeedsRedraw = true;
}

void OpenGLGraphicsManager::grabOverlay(OverlayColor *buf, int pitch) {
	assert(_overlayData.bytesPerPixel == sizeof(buf[0]));
	const byte *src = (byte *)_overlayData.pixels;
	for (int i = 0; i < _overlayData.h; i++) {
		memcpy(buf, src, _overlayData.pitch);
		buf += pitch;
		src += _overlayData.pitch;
	}
}

void OpenGLGraphicsManager::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);

	if (_overlayTexture == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
		y = 0;
	}

	if (w > _overlayData.w - x)
		w = _overlayData.w - x;

	if (h > _overlayData.h - y)
		h = _overlayData.h - y;

	if (w <= 0 || h <= 0)
		return;

	if (_overlayFormat.aBits() == 1) {
		// Copy buffer with the alpha bit on for all pixels for correct
		// overlay drawing.
		const uint16 *src = (uint16 *)buf;
		uint16 *dst = (uint16 *)_overlayData.pixels + y * _overlayData.w + x;
		for (int i = 0; i < h; i++) {
			for (int e = 0; e < w; e++)
				dst[e] = src[e] | 0x1;
			src += pitch;
			dst += _overlayData.w;
		}
	} else {
		// Copy buffer data to internal overlay surface
		const byte *src = (byte *)buf;
		byte *dst = (byte *)_overlayData.pixels + y * _overlayData.pitch;
		for (int i = 0; i < h; i++) {
			memcpy(dst + x * _overlayData.bytesPerPixel, src, w * _overlayData.bytesPerPixel);
			src += pitch * sizeof(buf[0]);
			dst += _overlayData.pitch;
		}
	}

	if (!_overlayNeedsRedraw) {
		const Common::Rect dirtyRect(x, y, x + w, y + h);
		_overlayDirtyRect.extend(dirtyRect);
	}
}

int16 OpenGLGraphicsManager::getOverlayHeight() {
	return _videoMode.overlayHeight;
}

int16 OpenGLGraphicsManager::getOverlayWidth() {
	return _videoMode.overlayWidth;
}

//
// Cursor
//

bool OpenGLGraphicsManager::showMouse(bool visible) {
	if (_cursorVisible == visible)
		return visible;

	bool last = _cursorVisible;
	_cursorVisible = visible;

	return last;
}

void OpenGLGraphicsManager::setMousePos(int x, int y) {
	_cursorState.x = x;
	_cursorState.y = y;
}

void OpenGLGraphicsManager::warpMouse(int x, int y) {
	setMousePos(x, y);
}

void OpenGLGraphicsManager::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
#ifdef USE_RGB_COLOR
	if (!format)
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		_cursorFormat = *format;
#else
	assert(keycolor <= 255);
#endif

	// Save cursor data
	_cursorData.create(w, h, _cursorFormat.bytesPerPixel);
	memcpy(_cursorData.pixels, buf, h * _cursorData.pitch);

	// Set cursor info
	_cursorState.w = w;
	_cursorState.h = h;
	_cursorState.hotX = hotspotX;
	_cursorState.hotY = hotspotY;
	_cursorKeyColor = keycolor;
	_cursorTargetScale = cursorTargetScale;
	_cursorNeedsRedraw = true;
}

void OpenGLGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	
	// Save the cursor palette
	memcpy(_cursorPalette + start * 4, colors, num * 4);

	_cursorPaletteDisabled = false;
	_cursorNeedsRedraw = true;
}

void OpenGLGraphicsManager::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
	_cursorNeedsRedraw = true;
}

//
// Misc
//

void OpenGLGraphicsManager::displayMessageOnOSD(const char *msg) {

}

//
// Intern
//

void OpenGLGraphicsManager::refreshGameScreen() {
	if (_screenNeedsRedraw)
		_screenDirtyRect = Common::Rect(0, 0, _screenData.w, _screenData.h);

	int x = _screenDirtyRect.left;
	int y = _screenDirtyRect.top;
	int w = _screenDirtyRect.width();
	int h = _screenDirtyRect.height();

	if (_screenData.bytesPerPixel == 1) {
		// Create a temporary RGB888 surface
		byte *surface = new byte[w * h * 3];

		// Convert the paletted buffer to RGB888
		const byte *src = (byte *)_screenData.pixels + y * _screenData.pitch;
		src += x * _screenData.bytesPerPixel;
		byte *dst = surface;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				dst[0] = _gamePalette[src[j] * 4];
				dst[1] = _gamePalette[src[j] * 4 + 1];
				dst[2] = _gamePalette[src[j] * 4 + 2];
				dst += 3;
			}
			src += _screenData.pitch;
		}

		// Update the texture
		_gameTexture->updateBuffer(surface, w * 3, x, y, w, h);

		// Free the temp surface
		delete[] surface;
	} else {
		// Update the texture
		_gameTexture->updateBuffer((byte *)_screenData.pixels + y * _screenData.pitch +
			x * _screenData.bytesPerPixel, _screenData.pitch, x, y, w, h);
	}

	_screenNeedsRedraw = false;
	_screenDirtyRect = Common::Rect();
}

void OpenGLGraphicsManager::refreshOverlay() {
	if (_overlayNeedsRedraw)
		_overlayDirtyRect = Common::Rect(0, 0, _overlayData.w, _overlayData.h);

	int x = _overlayDirtyRect.left;
	int y = _overlayDirtyRect.top;
	int w = _overlayDirtyRect.width();
	int h = _overlayDirtyRect.height();

	if (_overlayData.bytesPerPixel == 1) {
		// Create a temporary RGB888 surface
		byte *surface = new byte[w * h * 3];

		// Convert the paletted buffer to RGB888
		const byte *src = (byte *)_overlayData.pixels + y * _overlayData.pitch;
		src += x * _overlayData.bytesPerPixel;
		byte *dst = surface;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				dst[0] = _gamePalette[src[j] * 4];
				dst[1] = _gamePalette[src[j] * 4 + 1];
				dst[2] = _gamePalette[src[j] * 4 + 2];
				dst += 3;
			}
			src += _screenData.pitch;
		}

		// Update the texture
		_overlayTexture->updateBuffer(surface, w * 3, x, y, w, h);

		// Free the temp surface
		delete[] surface;
	} else {
		// Update the texture
		_overlayTexture->updateBuffer((byte *)_overlayData.pixels + y * _overlayData.pitch +
			x * _overlayData.bytesPerPixel, _overlayData.pitch, x, y, w, h);
	}

	_overlayNeedsRedraw = false;
	_overlayDirtyRect = Common::Rect();
}

void OpenGLGraphicsManager::refreshCursor() {
	_cursorNeedsRedraw = false;

	if (_cursorFormat.bytesPerPixel == 1) {
		// Create a temporary RGBA8888 surface
		byte *surface = new byte[_cursorState.w * _cursorState.h * 4];
		memset(surface, 0, _cursorState.w * _cursorState.h * 4);

		// Select palette
		byte *palette;
		if (_cursorPaletteDisabled)
			palette = _gamePalette;
		else
			palette = _cursorPalette;

		// Convert the paletted cursor to RGBA8888
		const byte *src = (byte *)_cursorData.pixels;
		byte *dst = surface;
		for (int i = 0; i < _cursorState.w * _cursorState.h; i++) {
			// Check for keycolor
			if (src[i] != _cursorKeyColor) {
				dst[0] = palette[src[i] * 4];
				dst[1] = palette[src[i] * 4 + 1];
				dst[2] = palette[src[i] * 4 + 2];
				dst[3] = 255;
			}
			dst += 4;
		}

		// Allocate a texture big enough for cursor
		_cursorTexture->allocBuffer(_cursorState.w, _cursorState.h);

		// Update the texture with new cursor
		_cursorTexture->updateBuffer(surface, _cursorState.w * 4, 0, 0, _cursorState.w, _cursorState.h);

		// Free the temp surface
		delete[] surface;
	}
}

void OpenGLGraphicsManager::getGLPixelFormat(Graphics::PixelFormat pixelFormat, byte &bpp, GLenum &glFormat, GLenum &gltype) {
	if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) { // RGBA8888
		bpp = 4;
		glFormat = GL_RGBA;
		gltype = GL_UNSIGNED_BYTE;
	} else if (pixelFormat == Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0)) {  // RGB888
		bpp = 3;
		glFormat = GL_RGB;
		gltype = GL_UNSIGNED_BYTE;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) {  // RGB565
		bpp = 2;
		glFormat = GL_RGB;
		gltype = GL_UNSIGNED_SHORT_5_6_5;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)) {  // RGB5551
		bpp = 2;
		glFormat = GL_RGBA;
		gltype = GL_UNSIGNED_SHORT_5_5_5_1;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0)) {  // RGBA4444
		bpp = 2;
		glFormat = GL_RGBA;
		gltype = GL_UNSIGNED_SHORT_4_4_4_4;
	} else if (pixelFormat.bytesPerPixel == 1) {  // CLUT8
		// If uses a palette, create texture as RGB888. The pixel data will be converted
		// later.
		bpp = 3;
		glFormat = GL_RGB;
		gltype = GL_UNSIGNED_BYTE;
	} else {
		error("Pixel format not supported");
	}
}

void OpenGLGraphicsManager::internUpdateScreen() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT); CHECK_GL_ERROR();

	// Draw the game screen
	if (_screenNeedsRedraw || !_screenDirtyRect.isEmpty())
		// Refresh texture if dirty
		refreshGameScreen();

	_gameTexture->drawTexture(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight);

	// Draw the overlay
	if (_overlayVisible) {

		// Refresh texture if dirty
		if (_overlayNeedsRedraw || !_overlayDirtyRect.isEmpty())
			refreshOverlay();

		_overlayTexture->drawTexture(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}

	// Draw the cursor
	if (_cursorVisible) {

		// Refresh texture if dirty
		if (_cursorNeedsRedraw)
			refreshCursor();

		_cursorTexture->drawTexture(_cursorState.x - _cursorState.hotX,
		_cursorState.y - _cursorState.hotY,	_cursorState.w, _cursorState.h);
	}
}

void OpenGLGraphicsManager::initGL() {
	// Check available GL Extensions
	GLTexture::initGLExtensions();

	// Disable 3D properties
	glDisable(GL_CULL_FACE); CHECK_GL_ERROR();
	glDisable(GL_DEPTH_TEST); CHECK_GL_ERROR();
	glDisable(GL_LIGHTING); CHECK_GL_ERROR();
	glDisable(GL_FOG); CHECK_GL_ERROR();
	glDisable(GL_DITHER); CHECK_GL_ERROR();
	glShadeModel(GL_FLAT); CHECK_GL_ERROR();
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST); CHECK_GL_ERROR();

	// Setup alpha blend (For overlay and cursor)
	glEnable(GL_BLEND); CHECK_GL_ERROR();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECK_GL_ERROR();

	// Enable rendering with vertex and coord arrays
	glEnableClientState(GL_VERTEX_ARRAY); CHECK_GL_ERROR();
	glEnableClientState(GL_TEXTURE_COORD_ARRAY); CHECK_GL_ERROR();

	glEnable(GL_TEXTURE_2D); CHECK_GL_ERROR();

	// Setup the GL viewport
	glViewport(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight); CHECK_GL_ERROR();

	// Setup coordinates system
	glMatrixMode(GL_PROJECTION); CHECK_GL_ERROR();
	glLoadIdentity(); CHECK_GL_ERROR();
	glOrtho(0, _videoMode.hardwareWidth, _videoMode.hardwareHeight, 0, -1, 1); CHECK_GL_ERROR();
	glMatrixMode(GL_MODELVIEW); CHECK_GL_ERROR();
	glLoadIdentity(); CHECK_GL_ERROR();
}

void OpenGLGraphicsManager::loadTextures() {
#ifdef USE_RGB_COLOR
	if (_transactionDetails.formatChanged && _gameTexture)
		delete _gameTexture;
#endif

	if (!_gameTexture) {
		byte bpp;
		GLenum format;
		GLenum type;
#ifdef USE_RGB_COLOR
		getGLPixelFormat(_screenFormat, bpp, format, type);
#else
		getGLPixelFormat(Graphics::PixelFormat::createFormatCLUT8(), bpp, format, type);
#endif
		_gameTexture = new GLTexture(bpp, format, type);
	} 

	_overlayFormat = Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0);

	if (!_overlayTexture) {
		byte bpp;
		GLenum format;
		GLenum type;
		getGLPixelFormat(_overlayFormat, bpp, format, type);
		_overlayTexture = new GLTexture(bpp, format, type);
	}

	if (!_cursorTexture)
		_cursorTexture = new GLTexture(4, GL_RGBA, GL_UNSIGNED_BYTE);
		
	GLint filter = _videoMode.antialiasing ? GL_LINEAR : GL_NEAREST;
	_gameTexture->setFilter(filter);
	_overlayTexture->setFilter(filter);
	_cursorTexture->setFilter(filter);

	if (_transactionDetails.newContext || _transactionDetails.filterChanged) {
		_gameTexture->refresh();
		_overlayTexture->refresh();
		_cursorTexture->refresh();
	}

	_gameTexture->allocBuffer(_videoMode.screenWidth, _videoMode.screenHeight);
	_overlayTexture->allocBuffer(_videoMode.overlayWidth, _videoMode.overlayHeight);
	_cursorTexture->allocBuffer(_cursorState.w, _cursorState.h);

	if (_transactionDetails.formatChanged || 
			_oldVideoMode.screenWidth != _videoMode.screenWidth ||
			_oldVideoMode.screenHeight != _videoMode.screenHeight)
		_screenData.create(_videoMode.screenWidth, _videoMode.screenHeight,
			_screenFormat.bytesPerPixel);

	if (_oldVideoMode.overlayWidth != _videoMode.overlayWidth ||
		_oldVideoMode.overlayHeight != _videoMode.overlayHeight)
		_overlayData.create(_videoMode.overlayWidth, _videoMode.overlayHeight,
			_overlayFormat.bytesPerPixel);
	
	_screenNeedsRedraw = true;
	_overlayNeedsRedraw = true;
	_cursorNeedsRedraw = true;
}

bool OpenGLGraphicsManager::loadGFXMode() {
	// Initialize OpenGL settings
	initGL();

	loadTextures();

	internUpdateScreen();

	return true;
}

void OpenGLGraphicsManager::unloadGFXMode() {

}

void OpenGLGraphicsManager::setScale(int newScale) {
	if (newScale == _videoMode.scaleFactor)
		return;

	_videoMode.scaleFactor = newScale;

	_transactionDetails.sizeChanged = true;
}

void OpenGLGraphicsManager::setAspectRatioCorrection(int ratio) {
	if (_oldVideoMode.setup && _oldVideoMode.aspectRatioCorrection == ratio)
		return;

	if (_transactionMode == kTransactionActive) {
		if (ratio == -1)
			_videoMode.aspectRatioCorrection = (_videoMode.aspectRatioCorrection + 1) % 4;
		else
			_videoMode.aspectRatioCorrection = ratio;
		_transactionDetails.needHotswap = true;
	}
}

void OpenGLGraphicsManager::adjustMouseEvent(const Common::Event &event) {
	if (!event.synthetic) {
		Common::Event newEvent(event);
		newEvent.synthetic = true;
		if (!_overlayVisible) {
			newEvent.mouse.x /= _videoMode.scaleFactor;
			newEvent.mouse.y /= _videoMode.scaleFactor;
			//if (_videoMode.aspectRatioCorrection)
			//	newEvent.mouse.y = aspect2Real(newEvent.mouse.y);
		}
		if (_videoMode.hardwareWidth != _videoMode.overlayWidth)
			newEvent.mouse.x = newEvent.mouse.x * _videoMode.overlayWidth / _videoMode.hardwareWidth;
		if (_videoMode.hardwareHeight != _videoMode.overlayHeight)
			newEvent.mouse.y = newEvent.mouse.y * _videoMode.overlayHeight / _videoMode.hardwareHeight;
		g_system->getEventManager()->pushEvent(newEvent);
	}
}

bool OpenGLGraphicsManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		if (!event.synthetic)
			setMousePos(event.mouse.x, event.mouse.y);
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP:
		adjustMouseEvent(event);
		return !event.synthetic;

	default:
		break;
	}

	return false;
}

bool OpenGLGraphicsManager::saveScreenshot(const char *filename) {
	return false;
}

#endif
