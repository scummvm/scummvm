/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "backends/graphics3d/openglsdl/openglsdl-graphics3d.h"

#include "backends/events/sdl/sdl-events.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "engines/engine.h"
#include "graphics/pixelbuffer.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/framebuffer.h"
#include "graphics/opengl/surfacerenderer.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"
#include "graphics/opengl/tiledsurface.h"

#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

OpenGLSdlGraphics3dManager::OpenGLSdlGraphics3dManager(SdlEventSource *eventSource, SdlWindow *window, bool supportsFrameBuffer)
	: SdlGraphics3dManager(eventSource, window),
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_glContext(nullptr),
#endif
	_supportsFrameBuffer(supportsFrameBuffer),
	_overlayVisible(false),
	_overlayScreen(nullptr),
	_overlayBackground(nullptr),
	_gameRect(),
	_fullscreen(false),
	_lockAspectRatio(true),
	_frameBuffer(nullptr),
	_surfaceRenderer(nullptr),
	_engineRequestedWidth(0),
	_engineRequestedHeight(0),
	_transactionMode(kTransactionNone) {
	ConfMan.registerDefault("antialiasing", 0);
	ConfMan.registerDefault("aspect_ratio", true);

	// Don't start at zero so that the value is never the same as the surface graphics manager
	_screenChangeCount = 1 << (sizeof(int) * 5 - 2);
}

OpenGLSdlGraphics3dManager::~OpenGLSdlGraphics3dManager() {
	closeOverlay();
#if SDL_VERSION_ATLEAST(2, 0, 0)
	deinitializeRenderer();
#endif
}

bool OpenGLSdlGraphics3dManager::hasFeature(OSystem::Feature f) const {
	return
		(f == OSystem::kFeatureFullscreenMode) ||
		(f == OSystem::kFeatureOpenGLForGame) ||
#if SDL_VERSION_ATLEAST(2, 0, 0)
		(f == OSystem::kFeatureFullscreenToggleKeepsContext) ||
#endif
		(f == OSystem::kFeatureVSync) ||
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureOverlaySupportsAlpha && _overlayFormat.aBits() > 3);
}

bool OpenGLSdlGraphics3dManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
		case OSystem::kFeatureVSync:
			return isVSyncEnabled();
		case OSystem::kFeatureFullscreenMode:
			return _fullscreen;
		case OSystem::kFeatureAspectRatioCorrection:
			return _lockAspectRatio;
		default:
			return false;
	}
}

void OpenGLSdlGraphics3dManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
		case OSystem::kFeatureFullscreenMode:
			if (_fullscreen != enable) {
				_fullscreen = enable;
				if (_transactionMode == kTransactionNone)
					createOrUpdateScreen();
			}
			break;
		case OSystem::kFeatureAspectRatioCorrection:
			_lockAspectRatio = enable;
			break;
		default:
			break;
	}
}

void OpenGLSdlGraphics3dManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;
}

OSystem::TransactionError OpenGLSdlGraphics3dManager::endGFXTransaction() {
	assert(_transactionMode != kTransactionNone);

	setupScreen();

	_transactionMode = kTransactionNone;
	return OSystem::kTransactionSuccess;
}

const OSystem::GraphicsMode glGraphicsModes[] = {
	{ "opengl3d", "OpenGL 3D", 0 },
	{ nullptr, nullptr, 0 }
};

const OSystem::GraphicsMode *OpenGLSdlGraphics3dManager::getSupportedGraphicsModes() const {
	return glGraphicsModes;
}

int OpenGLSdlGraphics3dManager::getDefaultGraphicsMode() const {
	return 0;
}

bool OpenGLSdlGraphics3dManager::setGraphicsMode(int mode, uint flags) {
	assert(_transactionMode != kTransactionNone);
	assert(flags & OSystem::kGfxModeRender3d);

	return true;
}

int OpenGLSdlGraphics3dManager::getGraphicsMode() const {
	return 0;
}

void OpenGLSdlGraphics3dManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	_engineRequestedWidth = w;
	_engineRequestedHeight = h;
	if (_transactionMode == kTransactionNone)
		setupScreen();
}

void OpenGLSdlGraphics3dManager::setupScreen() {
	assert(_transactionMode == kTransactionActive);

	closeOverlay();

	_antialiasing = ConfMan.getInt("antialiasing");
	_lockAspectRatio = ConfMan.getBool("aspect_ratio");
	_vsync = ConfMan.getBool("vsync");

#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool needsWindowReset = false;
	if (_window->getSDLWindow()) {
		// The anti-aliasing setting cannot be changed without recreating the window.
		// So check if the window needs to be recreated.

		int currentSamples = 0;
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &currentSamples);

		// When rendering to a framebuffer, MSAA is enabled on that framebuffer, not on the screen
		int targetSamples = shouldRenderToFramebuffer() ? 0 : _antialiasing;

		if (currentSamples != targetSamples) {
			needsWindowReset = true;
		}
	}

	// Clear the GL context when going from / to the launcher
	SDL_GL_DeleteContext(_glContext);
	_glContext = nullptr;

	if (needsWindowReset) {
		_window->destroyWindow();
	}
#endif

	createOrUpdateScreen();

	int glflag;
#ifdef __MORPHOS__
	const GLbyte *str;
#else
	const GLubyte *str;
#endif

	str = glGetString(GL_VENDOR);
	debug("INFO: OpenGL Vendor: %s", str);
	str = glGetString(GL_RENDERER);
	debug("INFO: OpenGL Renderer: %s", str);
	str = glGetString(GL_VERSION);
	debug("INFO: OpenGL Version: %s", str);
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &glflag);
	debug("INFO: OpenGL Red bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &glflag);
	debug("INFO: OpenGL Green bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &glflag);
	debug("INFO: OpenGL Blue bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glflag);
	debug("INFO: OpenGL Alpha bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glflag);
	debug("INFO: OpenGL Z buffer depth bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glflag);
	debug("INFO: OpenGL Double Buffer: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &glflag);
	debug("INFO: OpenGL Stencil buffer bits: %d", glflag);
#ifdef USE_GLEW
	debug("INFO: GLEW Version: %s", glewGetString(GLEW_VERSION));
#endif
#ifdef USE_OPENGL_SHADERS
	debug("INFO: GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
}

void OpenGLSdlGraphics3dManager::createOrUpdateScreen() {
	closeOverlay();

	// If the game can't adapt to any resolution, render it to a framebuffer
	// so it can be scaled to fill the available space.
	bool engineSupportsArbitraryResolutions = !g_engine || g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);
	bool renderToFrameBuffer = shouldRenderToFramebuffer();

	// Choose the effective window size or fullscreen mode
	uint effectiveWidth;
	uint effectiveHeight;
	if (_fullscreen && (engineSupportsArbitraryResolutions || renderToFrameBuffer)) {
		Common::Rect fullscreenResolution = getPreferredFullscreenResolution();
		effectiveWidth = fullscreenResolution.width();
		effectiveHeight = fullscreenResolution.height();
	} else {
		effectiveWidth = _engineRequestedWidth;
		effectiveHeight = _engineRequestedHeight;
	}

	if (!createOrUpdateGLContext(_engineRequestedWidth, _engineRequestedHeight,
	                             effectiveWidth, effectiveHeight,
	                             renderToFrameBuffer, engineSupportsArbitraryResolutions)) {
		warning("SDL Error: %s", SDL_GetError());
		g_system->quit();
	}

#ifdef USE_GLEW
	GLenum err = glewInit();
#ifdef GLEW_ERROR_NO_GLX_DISPLAY
	if (err == GLEW_ERROR_NO_GLX_DISPLAY) {
		// Wayland: https://github.com/nigels-com/glew/issues/172
	} else
#endif
	if (err != GLEW_OK) {
		warning("Error: %s", glewGetErrorString(err));
		g_system->quit();
	}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 1)
	int obtainedWidth = 0, obtainedHeight = 0;
	SDL_GL_GetDrawableSize(_window->getSDLWindow(), &obtainedWidth, &obtainedHeight);
#else
	int obtainedWidth = effectiveWidth;
	int obtainedHeight = effectiveHeight;
#endif

	// Compute the rectangle where to draw the game inside the effective screen
	_gameRect = computeGameRect(renderToFrameBuffer, _engineRequestedWidth, _engineRequestedHeight,
	                            obtainedWidth, obtainedHeight);

	initializeOpenGLContext();
	_surfaceRenderer = OpenGL::createBestSurfaceRenderer();

	_overlayFormat = OpenGL::TextureGL::getRGBAPixelFormat();
	_overlayScreen = new OpenGL::TiledSurface(obtainedWidth, obtainedHeight, _overlayFormat);

	_screenChangeCount++;

#if !defined(AMIGAOS) && !defined(__MORPHOS__)
	if (renderToFrameBuffer) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_frameBuffer = createFramebuffer(_engineRequestedWidth, _engineRequestedHeight);
		_frameBuffer->attach();
	}
#endif
}

Math::Rect2d OpenGLSdlGraphics3dManager::computeGameRect(bool renderToFrameBuffer, uint gameWidth, uint gameHeight,
                                                      uint screenWidth, uint screenHeight) {
	if (renderToFrameBuffer) {
		if (_lockAspectRatio) {
			// The game is scaled to fit the screen, keeping the same aspect ratio
			float scale = MIN(screenHeight / float(gameHeight), screenWidth / float(gameWidth));
			float scaledW = scale * (gameWidth / float(screenWidth));
			float scaledH = scale * (gameHeight / float(screenHeight));
			return Math::Rect2d(
					Math::Vector2d(0.5 - (0.5 * scaledW), 0.5 - (0.5 * scaledH)),
					Math::Vector2d(0.5 + (0.5 * scaledW), 0.5 + (0.5 * scaledH))
			);
		} else {
			// The game occupies the whole screen
			return Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1));
		}
	} else {
		return Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1));
	}
}

void OpenGLSdlGraphics3dManager::notifyResize(const int width, const int height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Get the updated size directly from SDL, in case there are multiple
	// resize events in the message queue.
	int newWidth = 0, newHeight = 0;
	SDL_GL_GetDrawableSize(_window->getSDLWindow(), &newWidth, &newHeight);

	if (newWidth == _overlayScreen->getWidth() && newHeight == _overlayScreen->getHeight()) {
		return; // nothing to do
	}

	// Compute the rectangle where to draw the game inside the effective screen
	_gameRect = computeGameRect(_frameBuffer != nullptr,
	                            _engineRequestedWidth, _engineRequestedHeight,
	                            newWidth, newHeight);

	// Update the overlay
	delete _overlayScreen;
	_overlayScreen = new OpenGL::TiledSurface(newWidth, newHeight, _overlayFormat);

	// Clear the overlay background so it is not displayed distorted while resizing
	delete _overlayBackground;
	_overlayBackground = nullptr;

	_screenChangeCount++;
#endif
}

void OpenGLSdlGraphics3dManager::initializeOpenGLContext() const {
	OpenGL::ContextOGLType type;

#ifdef USE_GLES2
	type = OpenGL::kOGLContextGLES2;
#else
	type = OpenGL::kOGLContextGL;
#endif

	OpenGLContext.initialize(type);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (SDL_GL_SetSwapInterval(_vsync ? 1 : 0)) {
		warning("Unable to %s VSync: %s", _vsync ? "enable" : "disable", SDL_GetError());
	}
#endif
}

OpenGLSdlGraphics3dManager::OpenGLPixelFormat::OpenGLPixelFormat(uint screenBytesPerPixel, uint red, uint blue, uint green, uint alpha, int samples) :
		bytesPerPixel(screenBytesPerPixel),
		redSize(red),
		blueSize(blue),
		greenSize(green),
		alphaSize(alpha),
		multisampleSamples(samples) {

}

bool OpenGLSdlGraphics3dManager::createOrUpdateGLContext(uint gameWidth, uint gameHeight,
                                                       uint effectiveWidth, uint effectiveHeight,
                                                       bool renderToFramebuffer,
                                                       bool engineSupportsArbitraryResolutions) {
	// Build a list of OpenGL pixel formats usable by ScummVM
	Common::Array<OpenGLPixelFormat> pixelFormats;
	if (_antialiasing > 0 && !renderToFramebuffer) {
		// Don't enable screen level multisampling when rendering to a framebuffer
		pixelFormats.push_back(OpenGLPixelFormat(32, 8, 8, 8, 8, _antialiasing));
		pixelFormats.push_back(OpenGLPixelFormat(16, 5, 5, 5, 1, _antialiasing));
		pixelFormats.push_back(OpenGLPixelFormat(16, 5, 6, 5, 0, _antialiasing));
	}
	pixelFormats.push_back(OpenGLPixelFormat(32, 8, 8, 8, 8, 0));
	pixelFormats.push_back(OpenGLPixelFormat(16, 5, 5, 5, 1, 0));
	pixelFormats.push_back(OpenGLPixelFormat(16, 5, 6, 5, 0, 0));

	// Unfortunatly, SDL does not provide a list of valid pixel formats
	// for the current OpenGL implementation and hardware.
	// SDL may not be able to create a screen with the preferred pixel format.
	// Try all the pixel formats in the list until SDL returns a valid screen.
	Common::Array<OpenGLPixelFormat>::const_iterator it = pixelFormats.begin();
	for (; it != pixelFormats.end(); it++) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, it->redSize);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, it->greenSize);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, it->blueSize);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, it->alphaSize);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, it->multisampleSamples > 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, it->multisampleSamples);
#if !SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, _vsync ? 1 : 0);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
#ifdef USE_GLES2
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
		// The OpenGL implementation on AmigaOS4 is close to 1.3. Until that changes we need
		// to use 1.3 as version or ScummVM will cease working at all on that platform.
		// Profile Mask has to be 0 as well.
		// This will be revised and removed once AmigaOS4 supports OpenGL 2.x or OpenGLES2.
		#ifdef __amigaos4__
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		#endif
#endif
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
		uint32 sdlflags = SDL_WINDOW_OPENGL;

		if (renderToFramebuffer || engineSupportsArbitraryResolutions) {
			sdlflags |= SDL_WINDOW_RESIZABLE;
		}

		if (_fullscreen) {
			// On Linux/X11, when toggling to fullscreen, the window manager saves
			// the window size to be able to restore it when going back to windowed mode.
			// If the user configured ScummVM to start in fullscreen mode, we first
			// create a window and then toggle it to fullscreen to give the window manager
			// a chance to save the window size. That way if the user switches back
			// to windowed mode, the window manager has a window size to apply instead
			// of leaving the window at the fullscreen resolution size.
			if (!_window->getSDLWindow()) {
				_window->createOrUpdateWindow(gameWidth, gameHeight, sdlflags);
			}

			sdlflags |= SDL_WINDOW_FULLSCREEN;
		}

		if (_window->createOrUpdateWindow(effectiveWidth, effectiveHeight, sdlflags)) {
			// Get the current GL context from SDL in case the previous one
			// was destroyed because the window was recreated.
			_glContext = SDL_GL_GetCurrentContext();
			if (!_glContext) {
				_glContext = SDL_GL_CreateContext(_window->getSDLWindow());
				if (_glContext) {
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}
			}

			if (_glContext) {
				assert(SDL_GL_GetCurrentWindow() == _window->getSDLWindow());
				break;
			}
		}

		_window->destroyWindow();
#else
		uint32 sdlflags = SDL_OPENGL;
		if (_fullscreen)
			sdlflags |= SDL_FULLSCREEN;

		SDL_Surface *screen = SDL_SetVideoMode(effectiveWidth, effectiveHeight, it->bytesPerPixel, sdlflags);
		if (screen) {
			break;
		}
#endif
	}

	// Display a warning if the effective pixel format is not the preferred one
	if (it != pixelFormats.begin() && it != pixelFormats.end()) {
		bool wantsAA = pixelFormats.front().multisampleSamples > 0;
		bool gotAA = it->multisampleSamples > 0;

		warning("Couldn't create a %d-bit visual%s, using to %d-bit%s instead",
		        pixelFormats.front().bytesPerPixel,
		        wantsAA && !gotAA ? " with AA" : "",
		        it->bytesPerPixel,
		        wantsAA && !gotAA ? " without AA" : "");
	}

	return it != pixelFormats.end();
}

bool OpenGLSdlGraphics3dManager::shouldRenderToFramebuffer() const {
	bool engineSupportsArbitraryResolutions = !g_engine || g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);
	return !engineSupportsArbitraryResolutions && _supportsFrameBuffer;
}

bool OpenGLSdlGraphics3dManager::isVSyncEnabled() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_GL_GetSwapInterval() != 0;
#else
	int swapControl = 0;
	SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &swapControl);
	return swapControl != 0;
#endif
}

void OpenGLSdlGraphics3dManager::drawOverlay() {
	glViewport(0, 0, _overlayScreen->getWidth(), _overlayScreen->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	_surfaceRenderer->prepareState();

	if (_overlayBackground) {
		_overlayBackground->draw(_surfaceRenderer);
	}

	_surfaceRenderer->enableAlphaBlending(true);
	_surfaceRenderer->setFlipY(true);
	_overlayScreen->draw(_surfaceRenderer);

	_surfaceRenderer->restorePreviousState();
}

#if !defined(AMIGAOS) && !defined(__MORPHOS__)
OpenGL::FrameBuffer *OpenGLSdlGraphics3dManager::createFramebuffer(uint width, uint height) {
#if !defined(USE_GLES2)
	if (_antialiasing && OpenGLContext.framebufferObjectMultisampleSupported) {
		return new OpenGL::MultiSampleFrameBuffer(width, height, _antialiasing);
	} else
#endif
	{
		return new OpenGL::FrameBuffer(width, height);
	}
}
#endif // AMIGAOS

void OpenGLSdlGraphics3dManager::updateScreen() {
	if (_frameBuffer) {
		_frameBuffer->detach();
		glViewport(0, 0, _overlayScreen->getWidth(), _overlayScreen->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_surfaceRenderer->prepareState();
		_surfaceRenderer->render(_frameBuffer, _gameRect);
		_surfaceRenderer->restorePreviousState();
	}

	if (_overlayVisible) {
		_overlayScreen->update();

		if (_overlayBackground) {
			_overlayBackground->update();
		}

		drawOverlay();
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow(_window->getSDLWindow());
#else
	SDL_GL_SwapBuffers();
#endif

	if (_frameBuffer) {
		_frameBuffer->attach();
	}
}

int16 OpenGLSdlGraphics3dManager::getHeight() const {
	if (_frameBuffer)
		return _frameBuffer->getHeight();
	else
		return _overlayScreen->getHeight();
}

int16 OpenGLSdlGraphics3dManager::getWidth() const {
	if (_frameBuffer)
		return _frameBuffer->getWidth();
	else
		return _overlayScreen->getWidth();
}

#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void OpenGLSdlGraphics3dManager::showOverlay() {
	if (_overlayVisible) {
		return;
	}
	_overlayVisible = true;

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

void OpenGLSdlGraphics3dManager::hideOverlay() {
	if (!_overlayVisible) {
		return;
	}
	_overlayVisible = false;

	delete _overlayBackground;
	_overlayBackground = nullptr;
}

void OpenGLSdlGraphics3dManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlayScreen->copyRectToSurface(buf, pitch, x, y, w, h);
}

void OpenGLSdlGraphics3dManager::clearOverlay() {
	_overlayScreen->fill(0);
}

void OpenGLSdlGraphics3dManager::grabOverlay(void *buf, int pitch) const {
	const Graphics::Surface *overlayData = _overlayScreen->getBackingSurface();

	const byte *src = (const byte *)overlayData->getPixels();
	byte *dst = (byte *)buf;

	for (uint h = overlayData->h; h > 0; --h) {
		memcpy(dst, src, overlayData->w * overlayData->format.bytesPerPixel);
		dst += pitch;
		src += overlayData->pitch;
	}
}

void OpenGLSdlGraphics3dManager::closeOverlay() {
	if (_overlayScreen) {
		delete _overlayScreen;
		_overlayScreen = nullptr;
	}

	delete _surfaceRenderer;
	_surfaceRenderer = nullptr;

	delete _frameBuffer;
	_frameBuffer = nullptr;

	OpenGL::ContextGL::destroy();
}

int16 OpenGLSdlGraphics3dManager::getOverlayHeight() const {
	return _overlayScreen->getHeight();
}

int16 OpenGLSdlGraphics3dManager::getOverlayWidth() const {
	return _overlayScreen->getWidth();
}

void OpenGLSdlGraphics3dManager::warpMouse(int x, int y) {
	if (!_overlayVisible && _frameBuffer) {
		// Scale from game coordinates to screen coordinates
		x = (x * _gameRect.getWidth() * _overlayScreen->getWidth()) / _frameBuffer->getWidth();
		y = (y * _gameRect.getHeight() * _overlayScreen->getHeight()) / _frameBuffer->getHeight();

		x += _gameRect.getTopLeft().getX() * _overlayScreen->getWidth();
		y += _gameRect.getTopLeft().getY() * _overlayScreen->getHeight();
	}

	_window->warpMouseInWindow(x, y);
}

void OpenGLSdlGraphics3dManager::transformMouseCoordinates(Common::Point &point) {
	if (_overlayVisible || !_frameBuffer)
		return;

	// Scale from screen coordinates to game coordinates
	point.x -= _gameRect.getTopLeft().getX() * _overlayScreen->getWidth();
	point.y -= _gameRect.getTopLeft().getY() * _overlayScreen->getHeight();

	point.x = (point.x * _frameBuffer->getWidth())  / (_gameRect.getWidth() * _overlayScreen->getWidth());
	point.y = (point.y * _frameBuffer->getHeight()) / (_gameRect.getHeight() * _overlayScreen->getHeight());

	// Make sure we only supply valid coordinates.
	point.x = CLIP<int16>(point.x, 0, _frameBuffer->getWidth() - 1);
	point.y = CLIP<int16>(point.y, 0, _frameBuffer->getHeight() - 1);
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void OpenGLSdlGraphics3dManager::deinitializeRenderer() {
	SDL_GL_DeleteContext(_glContext);
	_glContext = nullptr;
}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)

bool OpenGLSdlGraphics3dManager::saveScreenshot(const Common::String &filename) const {
	// Largely based on the implementation from ScummVM
	uint width = _overlayScreen->getWidth();
	uint height = _overlayScreen->getHeight();

	uint linePaddingSize = width % 4;
	uint lineSize = width * 3 + linePaddingSize;

	Common::DumpFile out;
	if (!out.open(filename)) {
		return false;
	}

	Common::Array<uint8> pixels;
	pixels.resize(lineSize * height);

	if (_frameBuffer) {
		_frameBuffer->detach();
	}
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixels.front());
	if (_frameBuffer) {
		_frameBuffer->attach();
	}

#ifdef SCUMM_LITTLE_ENDIAN
	const Graphics::PixelFormat format(3, 8, 8, 8, 0, 0, 8, 16, 0);
#else
	const Graphics::PixelFormat format(3, 8, 8, 8, 0, 16, 8, 0, 0);
#endif
	Graphics::Surface data;
	data.init(width, height, lineSize, &pixels.front(), format);
	data.flipVertical(Common::Rect(width, height));
#ifdef USE_PNG
	return Image::writePNG(out, data);
#else
	return Image::writeBMP(out, data);
#endif
}

#endif
