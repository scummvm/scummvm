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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics3d/ios/ios-graphics3d.h"
#include "backends/platform/ios7/ios7_osys_main.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/texture.h"
#include "graphics/blit.h"
#include "common/translation.h"

#include "engines/engine.h"

iOSGraphics3dManager::iOSGraphics3dManager() :
	_screenChangeCount(0),
	_stretchMode(STRETCH_FIT),
	_mouseDontScale(false),
	_aspectRatioCorrection(true),
	_overlayScreen(nullptr),
	_overlayBackground(nullptr),
	_mouseSurface(nullptr),
	_surfaceRenderer(nullptr),
	_frameBuffer(nullptr),
	_glFBO(0) {

	ConfMan.registerDefault("aspect_ratio", true);

	memset(_glRBOs, 0, sizeof(_glRBOs));

	initSurface();
}

iOSGraphics3dManager::~iOSGraphics3dManager() {
	delete _overlayScreen;
	delete _overlayBackground;
	delete _mouseSurface;
	delete _surfaceRenderer;
	delete _frameBuffer;
}

void iOSGraphics3dManager::initSurface() {
	OSystem_iOS7 *sys = dynamic_cast<OSystem_iOS7 *>(g_system);

	// Create OpenGL context if not existing
	GLuint glRBO = sys->createOpenGLContext();

	OpenGLContext.initialize(OpenGL::kContextGLES2);

	// Create the framebuffer attached to ObjC provided RBO
	glGenFramebuffers(1, &_glFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _glFBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, glRBO);

	// Attach a depth and stencil buffer
	createDepthAndStencilBuffer(sys->getScreenWidth(), sys->getScreenHeight());

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		error("Framebuffer is not complete! status: %d", status);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_old_touch_mode = kTouchModeTouchpad;

	//initSize will be called to set the size
}

void iOSGraphics3dManager::deinitSurface() {
	glDeleteFramebuffers(1, &_glFBO);
	glDeleteRenderbuffers(2, _glRBOs);

	OpenGLContext.destroy();
	dynamic_cast<OSystem_iOS7 *>(g_system)->destroyOpenGLContext();
}

void iOSGraphics3dManager::createDepthAndStencilBuffer(int width, int height) {
	glGenRenderbuffers(2, _glRBOs);
	glBindRenderbuffer(GL_RENDERBUFFER, _glRBOs[0]);

	if (OpenGLContext.packedDepthStencilSupported) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _glRBOs[0]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _glRBOs[0]);
	} else {
		glRenderbufferStorage(GL_RENDERBUFFER, OpenGLContext.OESDepth24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16,
				width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _glRBOs[0]);

		glBindRenderbuffer(GL_RENDERBUFFER, _glRBOs[1]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8,
				width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _glRBOs[1]);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void iOSGraphics3dManager::updateDepthAndStencilBuffer(int width, int height) {
	glBindRenderbuffer(GL_RENDERBUFFER, _glRBOs[0]);
	if (OpenGLContext.packedDepthStencilSupported) {
	  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	} else {
	  glRenderbufferStorage(GL_RENDERBUFFER, OpenGLContext.OESDepth24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16,
							width, height);
	  glBindRenderbuffer(GL_RENDERBUFFER, _glRBOs[1]);
	  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8,
							width, height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void iOSGraphics3dManager::notifyResize(const int width, const int height) {
	updateDepthAndStencilBuffer(width, height);
	handleResize(width, height);
}

iOSCommonGraphics::State iOSGraphics3dManager::getState() const {
	State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = getScreenFormat();
#endif
	return state;
}

bool iOSGraphics3dManager::setState(const iOSCommonGraphics::State &state) {
	initSize(state.screenWidth, state.screenHeight, nullptr);
	setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
	setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	return true;
}

bool iOSGraphics3dManager::gameNeedsAspectRatioCorrection() const {
	if (_aspectRatioCorrection) {
		const uint width = getWidth();
		const uint height = getHeight();
		// In case we enable aspect ratio correction we force a 4/3 ratio.
		// But just for 320x200 and 640x400 games, since other games do not need
		// this.
		return (width == 320 && height == 200) || (width == 640 && height == 400);
	}
	return false;
}

bool iOSGraphics3dManager::hasFeature(OSystem::Feature f) const {
	if ((f == OSystem::kFeatureOpenGLForGame) ||
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureStretchMode) ||
		(f == OSystem::kFeatureCursorAlpha) ||
		(f == OSystem::kFeatureOverlaySupportsAlpha && _overlayFormat.aBits() > 3)) {
		return true;
	}
	return false;
}

bool iOSGraphics3dManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		return _aspectRatioCorrection;
	default:
		return false;
	}
}

void iOSGraphics3dManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		_aspectRatioCorrection = enable;
		break;
	default:
		break;
	}
}

const OSystem::GraphicsMode *iOSGraphics3dManager::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{ "default", "Default", 0 },
		{ 0, 0, 0 },
	};

	return s_supportedGraphicsModes;
}

int iOSGraphics3dManager::getDefaultGraphicsMode() const {
	return 0;
}

bool iOSGraphics3dManager::setGraphicsMode(int mode, uint flags) {
	assert(flags & OSystem::kGfxModeRender3d);
	return true;
}

int iOSGraphics3dManager::getGraphicsMode() const {
	return 0;
}

void iOSGraphics3dManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	OSystem_iOS7 *sys = dynamic_cast<OSystem_iOS7 *>(g_system);

	bool engineSupportsArbitraryResolutions = !g_engine ||
			g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);
	if (!engineSupportsArbitraryResolutions) {
		if (_frameBuffer) {
			delete _frameBuffer;
		}
		// If the game can't adapt to any resolution, render it to a framebuffer
		// so it can be scaled to fill the available space.
		_frameBuffer = new OpenGL::FrameBuffer(w, h);
		_frameBuffer->attach();
	}

	_surfaceRenderer = OpenGL::createBestSurfaceRenderer();
	_overlayFormat = OpenGL::Texture::getRGBAPixelFormat();

	handleResize(sys->getScreenWidth(), sys->getScreenHeight());
}

int16 iOSGraphics3dManager::getHeight() const {
	if (_frameBuffer)
		return _frameBuffer->getHeight();
	else
		return _overlayScreen->getHeight();
}

int16 iOSGraphics3dManager::getWidth() const {
	if (_frameBuffer)
		return _frameBuffer->getWidth();
	else
		return _overlayScreen->getWidth();
}

float iOSGraphics3dManager::getHiDPIScreenFactor() const {
	return dynamic_cast<OSystem_iOS7 *>(g_system)->getSystemHiDPIScreenFactor();
}

void iOSGraphics3dManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {

	if (w == 0 || h == 0)
		return;

	const Graphics::PixelFormat pixelFormat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();

	if (_mouseSurface == nullptr)
		_mouseSurface = new OpenGL::TiledSurface(w, h, pixelFormat);

	_mouseHotspot = Common::Point(hotspotX, hotspotY);
	_mouseDontScale = dontScale;

	_mouseSurface->copyRectToSurface(buf, _mouseSurface->getBackingSurface()->pitch, hotspotX, hotspotY, w, h);

	updateCursorScaling();
}

void iOSGraphics3dManager::handleResizeImpl(const int width, const int height) {
	// Update the overlay
	delete _overlayScreen;
	_overlayScreen = new OpenGL::TiledSurface(width, height, _overlayFormat);

	delete _mouseSurface;
	_mouseSurface = nullptr;

	// Clear the overlay background so it is not displayed distorted while resizing
	delete _overlayBackground;
	_overlayBackground = nullptr;

	// Re-setup the scaling for the screen
	recalculateDisplayAreas();

	// Something changed, so update the screen change ID.
	_screenChangeCount++;
}

void iOSGraphics3dManager::updateScreen() {
	GLint prevStateViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevStateViewport);

	// Make sure our framebuffer is still bound
	glBindFramebuffer(GL_FRAMEBUFFER, _glFBO);

	if (_frameBuffer) {
		_surfaceRenderer->prepareState();
		glViewport(_gameDrawRect.left, _windowHeight - _gameDrawRect.top - _gameDrawRect.height(), _gameDrawRect.width(), _gameDrawRect.height());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_surfaceRenderer->render(_frameBuffer, Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1)));
		_surfaceRenderer->restorePreviousState();
	}

	if (_overlayVisible) {
		_overlayScreen->update();

		// If the overlay is in game we expect the game to continue calling OpenGL
		if (_overlayBackground && _overlayInGUI) {
			_overlayBackground->update();
		}

		if (_cursorVisible && _mouseSurface) {
			_mouseSurface->update();
		}

		_surfaceRenderer->prepareState();

		glViewport(_overlayDrawRect.left, _windowHeight - _overlayDrawRect.top - _overlayDrawRect.height(), getOverlayWidth(), getOverlayHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if (_overlayBackground) {
			_overlayBackground->draw(_surfaceRenderer);
		}

		_surfaceRenderer->enableAlphaBlending(true);
		_surfaceRenderer->setFlipY(true);

		_overlayScreen->draw(_surfaceRenderer);

		if (_cursorVisible && _mouseSurface) {
			glViewport(_cursorX - _mouseHotspotScaled.x, _overlayScreen->getHeight() - _cursorY - _mouseHeightScaled - _mouseHotspotScaled.y, _mouseWidthScaled, _mouseHeightScaled);
			_mouseSurface->draw(_surfaceRenderer);
		}
		_surfaceRenderer->restorePreviousState();
	}

	if (_frameBuffer) {
		_frameBuffer->attach();
	}

	dynamic_cast<OSystem_iOS7 *>(g_system)->refreshScreen();
	glViewport(prevStateViewport[0], prevStateViewport[1], prevStateViewport[2], prevStateViewport[3]);
}

const OSystem::GraphicsMode glStretchModes[] = {
	{"center", _s("Center"), STRETCH_CENTER},
	{"pixel-perfect", _s("Pixel-perfect scaling"), STRETCH_INTEGRAL},
	{"even-pixels", _s("Even pixels scaling"), STRETCH_INTEGRAL_AR},
	{"fit", _s("Fit to window"), STRETCH_FIT},
	{"stretch", _s("Stretch to window"), STRETCH_STRETCH},
	{"fit_force_aspect", _s("Fit to window (4:3)"), STRETCH_FIT_FORCE_ASPECT},
	{nullptr, nullptr, 0}
};

const OSystem::GraphicsMode *iOSGraphics3dManager::getSupportedStretchModes() const {
	return glStretchModes;
}

int iOSGraphics3dManager::getDefaultStretchMode() const {
	return STRETCH_FIT;
}

bool iOSGraphics3dManager::setStretchMode(int mode) {
	if (mode == _stretchMode)
		return true;

	// Check this is a valid mode
	const OSystem::GraphicsMode *sm = getSupportedStretchModes();
	bool found = false;
	while (sm->name) {
		if (sm->id == mode) {
			found = true;
			break;
		}
		sm++;
	}
	if (!found) {
		warning("unknown stretch mode %d", mode);
		return false;
	}

	_stretchMode = mode;
	return true;
}

int iOSGraphics3dManager::getStretchMode() const {
	return _stretchMode;
}

void iOSGraphics3dManager::clearOverlay() {
	_overlayScreen->fill(0);
}

void iOSGraphics3dManager::grabOverlay(Graphics::Surface &surface) const {
	const Graphics::Surface *overlayData = _overlayScreen->getBackingSurface();

	assert(surface.w >= overlayData->w);
	assert(surface.h >= overlayData->h);
	assert(surface.format.bytesPerPixel == overlayData->format.bytesPerPixel);

	const byte *src = (const byte *)overlayData->getPixels();
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch, overlayData->pitch, overlayData->w, overlayData->h, overlayData->format.bytesPerPixel);
}

void iOSGraphics3dManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlayScreen->copyRectToSurface(buf, pitch, x, y, w, h);
}

int16 iOSGraphics3dManager::getOverlayHeight() const {
	return _overlayScreen->getHeight();
}

int16 iOSGraphics3dManager::getOverlayWidth() const {
	return _overlayScreen->getWidth();
}

void iOSGraphics3dManager::showOverlay(bool inGUI) {
	if (_overlayVisible && _overlayInGUI == inGUI) {
		return;
	}

	// Don't change touch mode when not changing mouse coordinates
	if (inGUI) {
		_old_touch_mode = dynamic_cast<OSystem_iOS7 *>(g_system)->getCurrentTouchMode();
		// in 3D, in overlay
		dynamic_cast<OSystem_iOS7 *>(g_system)->applyTouchSettings(true, true);
	} else if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		dynamic_cast<OSystem_iOS7 *>(g_system)->setCurrentTouchMode(static_cast<TouchMode>(_old_touch_mode));
	}

	WindowedGraphicsManager::showOverlay(inGUI);

	delete _overlayBackground;
	_overlayBackground = nullptr;

	if (g_engine) {
		if (_frameBuffer)
			_frameBuffer->detach();

		// If there is a game running capture the screen, so that it can be shown "below" the overlay.
		_overlayBackground = new OpenGL::TiledSurface(_overlayScreen->getWidth(), _overlayScreen->getHeight(), _overlayFormat);
		Graphics::Surface *background = _overlayBackground->getBackingSurface();
		glReadPixels(0, 0, background->w, background->h, GL_RGBA, GL_UNSIGNED_BYTE, background->getPixels());

		if (_frameBuffer)
			_frameBuffer->attach();
	}
}

void iOSGraphics3dManager::hideOverlay() {
	if (!_overlayVisible) {
		return;
	}

	if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		dynamic_cast<OSystem_iOS7 *>(g_system)->setCurrentTouchMode(static_cast<TouchMode>(_old_touch_mode));
	}

	WindowedGraphicsManager::hideOverlay();

	delete _overlayBackground;
	_overlayBackground = nullptr;

	if (_surfaceRenderer) {
		_surfaceRenderer->prepareState();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_surfaceRenderer->restorePreviousState();
		updateScreen();
		_surfaceRenderer->prepareState();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_surfaceRenderer->restorePreviousState();
		updateScreen();
	}
}

bool iOSGraphics3dManager::showMouse(bool visible) {
	return WindowedGraphicsManager::showMouse(visible);
}

bool iOSGraphics3dManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, _activeArea.drawRect.left, _activeArea.drawRect.right);
	mouse.y = CLIP<int16>(mouse.y, _activeArea.drawRect.top, _activeArea.drawRect.bottom);

	setMousePosition(mouse.x, mouse.y);
	mouse = convertWindowToVirtual(mouse.x, mouse.y);

	return true;
}

void iOSGraphics3dManager::updateCursorScaling() {
	// By default we use the unscaled versions.
	_mouseHotspotScaled = _mouseHotspot;
	_mouseWidthScaled = _mouseSurface->getWidth();
	_mouseHeightScaled = _mouseSurface->getHeight();

	// In case scaling is actually enabled we will scale the cursor according
	// to the game screen.
	uint16 w = getWidth();
	uint16 h = getHeight();

	if (!_mouseDontScale && w && h) {
		const frac_t screen_scale_factor_x = intToFrac(_gameDrawRect.width()) / w;
		const frac_t screen_scale_factor_y = intToFrac(_gameDrawRect.height()) / h;

		_mouseHotspotScaled = Common::Point(
			fracToInt(_mouseHotspotScaled.x * screen_scale_factor_x),
			fracToInt(_mouseHotspotScaled.y * screen_scale_factor_y));

		_mouseWidthScaled  = fracToInt(_mouseWidthScaled * screen_scale_factor_x);
		_mouseHeightScaled = fracToInt(_mouseHeightScaled * screen_scale_factor_y);
	}
}

#endif
