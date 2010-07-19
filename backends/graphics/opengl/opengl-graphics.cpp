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
	_screenChangeCount(0),
	_currentShakePos(0), _newShakePos(0),
	_overlayVisible(false),
	_transactionMode(kTransactionNone),
	_cursorNeedsRedraw(false), _cursorPaletteDisabled(true),
	_cursorVisible(false), _cursorData(0), _cursorKeyColor(0),
	_cursorTargetScale(1) {

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

	_videoMode.mode = OpenGL::GFX_NORMAL;
	_videoMode.scaleFactor = 1;
	_videoMode.fullscreen = false;

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
	if (_cursorData != NULL)
		free(_cursorData);

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
	return (f == OSystem::kFeatureCursorHasPalette);
}

void OpenGLGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {

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
	return true;
}

int OpenGLGraphicsManager::getGraphicsMode() const {
	return OpenGL::GFX_NORMAL;
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat OpenGLGraphicsManager::getScreenFormat() const {
	return _screenFormat;
}

#endif

void OpenGLGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

#ifdef USE_RGB_COLOR
	//avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
		//newFormat = Graphics::PixelFormat::createFormatCLUT8();
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
		/*} else if (_videoMode.aspectRatioCorrection != _oldVideoMode.aspectRatioCorrection) {
			errors |= OSystem::kTransactionAspectRatioFailed;

			_videoMode.aspectRatioCorrection = _oldVideoMode.aspectRatioCorrection;*/
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
			//_videoMode.aspectRatioCorrection == _oldVideoMode.aspectRatioCorrection &&
			_videoMode.mode == _oldVideoMode.mode &&
			_videoMode.screenWidth == _oldVideoMode.screenWidth &&
		   	_videoMode.screenHeight == _oldVideoMode.screenHeight) {

			_oldVideoMode.setup = false;
		}
	}

#ifdef USE_RGB_COLOR
	if (_transactionDetails.sizeChanged || _transactionDetails.formatChanged) {
#else
	if (_transactionDetails.sizeChanged) {
#endif
		unloadGFXMode();
		if (!loadGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			//setGraphicsModeIntern();
			//clearOverlay();

			_videoMode.setup = true;
			_screenChangeCount++;
		}
	} else if (_transactionDetails.needHotswap) {
		//setGraphicsModeIntern();
		if (!hotswapGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			_videoMode.setup = true;
			_screenChangeCount++;

			if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
	} else if (_transactionDetails.needUpdatescreen) {
		//setGraphicsModeIntern();
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
	_gameTexture->updateBuffer(buf, pitch, x, y, w, h);
}

Graphics::Surface *OpenGLGraphicsManager::lockScreen() {
	return _gameTexture->getSurface();
}

void OpenGLGraphicsManager::unlockScreen() {
	_gameTexture->refresh();
}

void OpenGLGraphicsManager::fillScreen(uint32 col) {
	if (_gameTexture == NULL)
		return;

	_gameTexture->fillBuffer(col);
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

	_overlayVisible = true;
}

void OpenGLGraphicsManager::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	_overlayVisible = false;
}

Graphics::PixelFormat OpenGLGraphicsManager::getOverlayFormat() const {
	return _overlayFormat;
}

void OpenGLGraphicsManager::clearOverlay() {
	_overlayTexture->fillBuffer(0);
}

void OpenGLGraphicsManager::grabOverlay(OverlayColor *buf, int pitch) {
	const Graphics::Surface* surface = _overlayTexture->getSurface();
	assert(surface->bytesPerPixel == sizeof(buf[0]));
	int h = surface->h;
	do {
		//memcpy(buf, surface->pixels, surface->w * sizeof(buf[0]));
		memset(buf, 0, surface->w * sizeof(buf[0]));
		buf += pitch;
	} while (--h);
}

void OpenGLGraphicsManager::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);

	if (_overlayTexture == NULL)
		return;

	_overlayTexture->updateBuffer(buf, pitch * sizeof(buf[0]), x, y, w, h);
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
	free(_cursorData);
	_cursorData = (byte *)malloc(w * h * _cursorFormat.bytesPerPixel);
	memcpy(_cursorData, buf, w * h * _cursorFormat.bytesPerPixel);

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

void OpenGLGraphicsManager::refreshCursor() {
	_cursorNeedsRedraw = false;

	if (_cursorFormat == Graphics::PixelFormat::createFormatCLUT8()) {
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
		byte *dst = surface;
		for (int i = 0; i < _cursorState.w * _cursorState.h; i++) {
			// Check for keycolor
			if (_cursorData[i] != _cursorKeyColor) {
				dst[0] = palette[_cursorData[i] * 4];
				dst[1] = palette[_cursorData[i] * 4 + 1];
				dst[2] = palette[_cursorData[i] * 4 + 2];
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

//
// Misc
//

void OpenGLGraphicsManager::displayMessageOnOSD(const char *msg) {

}

//
// Intern
//

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
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)) {  // RGB555
		bpp = 2;
		glFormat = GL_RGB;
		gltype = GL_UNSIGNED_SHORT_5_5_5_1;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0)) {  // RGBA4444
		bpp = 2;
		glFormat = GL_RGBA;
		gltype = GL_UNSIGNED_SHORT_4_4_4_4;
	} else if (pixelFormat == Graphics::PixelFormat::createFormatCLUT8()) {  // CLUT8
		bpp = 1;
		glFormat = GL_COLOR_INDEX;
		gltype = GL_UNSIGNED_BYTE;
	} else {
		error("Not supported format");
	}
}

void OpenGLGraphicsManager::internUpdateScreen() {
	// Clear the screen
	CHECK_GL_ERROR( glClear(GL_COLOR_BUFFER_BIT) );

	// Draw the game screen
	_gameTexture->drawTexture(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight);

	// Draw the overlay
	if (_overlayVisible)
		_overlayTexture->drawTexture(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight);

	// Draw the cursor
	if (_cursorVisible) {
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
	CHECK_GL_ERROR( glDisable(GL_CULL_FACE) );
	CHECK_GL_ERROR( glDisable(GL_DEPTH_TEST) );
	CHECK_GL_ERROR( glDisable(GL_LIGHTING) );
	CHECK_GL_ERROR( glDisable(GL_FOG) );
	CHECK_GL_ERROR( glDisable(GL_DITHER) );
	CHECK_GL_ERROR( glShadeModel(GL_FLAT) );
	CHECK_GL_ERROR( glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST) );

	// Setup alpha blend (For overlay and cursor)
	CHECK_GL_ERROR( glEnable(GL_BLEND) );
	CHECK_GL_ERROR( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

	// Enable rendering with vertex and coord arrays
	CHECK_GL_ERROR( glEnableClientState(GL_VERTEX_ARRAY) );
	CHECK_GL_ERROR( glEnableClientState(GL_TEXTURE_COORD_ARRAY) );

	CHECK_GL_ERROR( glEnable(GL_TEXTURE_2D) );

	// Setup the GL viewport
	CHECK_GL_ERROR( glViewport(0, 0, _videoMode.hardwareWidth, _videoMode.hardwareHeight) );

	// Setup coordinates system
	CHECK_GL_ERROR( glMatrixMode(GL_PROJECTION) );
	CHECK_GL_ERROR( glLoadIdentity() );
	CHECK_GL_ERROR( glOrtho(0, _videoMode.hardwareWidth, _videoMode.hardwareHeight, 0, -1, 1) );
	CHECK_GL_ERROR( glMatrixMode(GL_MODELVIEW) );
	CHECK_GL_ERROR( glLoadIdentity() );
}

bool OpenGLGraphicsManager::loadGFXMode() {
	// Initialize OpenGL settings
	initGL();

	if (!_gameTexture) {
		byte bpp;
		GLenum format;
		GLenum type;
		getGLPixelFormat(_screenFormat, bpp, format, type);
		_gameTexture = new GLTexture(bpp, format, type);
	} else
		_gameTexture->refresh();

	_overlayFormat = Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0);

	if (!_overlayTexture)
		_overlayTexture = new GLTexture(2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
	else
		_overlayTexture->refresh();

	if (!_cursorTexture)
		_cursorTexture = new GLTexture(4, GL_RGBA, GL_UNSIGNED_BYTE);
	else
		_cursorTexture->refresh();

	_gameTexture->allocBuffer(_videoMode.screenWidth, _videoMode.screenHeight);
	_overlayTexture->allocBuffer(_videoMode.overlayWidth, _videoMode.overlayHeight);
	_cursorTexture->allocBuffer(16, 16);

	internUpdateScreen();

	return true;
}

void OpenGLGraphicsManager::unloadGFXMode() {

}

bool OpenGLGraphicsManager::hotswapGFXMode() {
	return false;
}

void OpenGLGraphicsManager::setScale(int newScale) {
	if (newScale == _videoMode.scaleFactor)
		return;

	_videoMode.scaleFactor = newScale;

	_transactionDetails.sizeChanged = true;
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
			newEvent.mouse.x /= (float)_videoMode.hardwareWidth / _videoMode.overlayWidth;
		if (_videoMode.hardwareHeight != _videoMode.overlayHeight)
			newEvent.mouse.y /= (float)_videoMode.hardwareHeight / _videoMode.overlayHeight;
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
