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

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics3d/openglsdl/openglsdl-graphics3d.h"
#include "backends/graphics3d/opengl/surfacerenderer.h"
#include "backends/graphics3d/opengl/tiledsurface.h"
#include "backends/graphics3d/opengl/framebuffer.h"
#include "backends/events/sdl/sdl-events.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

#include "engines/engine.h"

#include "graphics/blit.h"
#include "graphics/opengl/context.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

OpenGLSdlGraphics3dManager::OpenGLSdlGraphics3dManager(SdlEventSource *eventSource, SdlWindow *window, bool supportsFrameBuffer)
	: SdlGraphicsManager(eventSource, window),
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_glContext(nullptr),
#endif
	_supportsFrameBuffer(supportsFrameBuffer),
	_overlayScreen(nullptr),
	_overlayBackground(nullptr),
	_fullscreen(false),
	_lockAspectRatio(true),
	_stretchMode(STRETCH_FIT),
	_frameBuffer(nullptr),
	_surfaceRenderer(nullptr),
	_engineRequestedWidth(0),
	_engineRequestedHeight(0),
	_transactionMode(kTransactionNone) {
	ConfMan.registerDefault("antialiasing", 0);
	ConfMan.registerDefault("aspect_ratio", true);

	// Don't start at zero so that the value is never the same as the surface graphics manager
	_screenChangeCount = 1 << (sizeof(int) * 5 - 2);

	// Set up proper SDL OpenGL context creation.
#if SDL_VERSION_ATLEAST(2, 0, 0)
	enum {
#ifdef USE_OPENGL_SHADERS
		DEFAULT_GL_MAJOR = 2,
		DEFAULT_GL_MINOR = 1,
#else
		DEFAULT_GL_MAJOR = 1,
		DEFAULT_GL_MINOR = 3,
#endif

		DEFAULT_GLES2_MAJOR = 2,
		DEFAULT_GLES2_MINOR = 0
	};

#if USE_FORCED_GLES2
	_glContextType = OpenGL::kContextGLES2;
	_glContextProfileMask = SDL_GL_CONTEXT_PROFILE_ES;
	_glContextMajor = DEFAULT_GLES2_MAJOR;
	_glContextMinor = DEFAULT_GLES2_MINOR;
#else
	bool noDefaults = false;

	// Obtain the default GL(ES) context SDL2 tries to setup.
	//
	// Please note this might not actually be SDL2's defaults when multiple
	// instances of this object have been created. But that is no issue
	// because then we already set up what we want to use.
	//
	// In case no defaults are given we prefer OpenGL over OpenGL ES.
	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &_glContextProfileMask) != 0) {
		_glContextProfileMask = 0;
		noDefaults = true;
	}

	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &_glContextMajor) != 0) {
		noDefaults = true;
	}

	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &_glContextMinor) != 0) {
		noDefaults = true;
	}

	if (noDefaults) {
		if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_ES) {
			_glContextMajor = DEFAULT_GLES2_MAJOR;
			_glContextMinor = DEFAULT_GLES2_MINOR;
		} else {
			_glContextProfileMask = 0;
			_glContextMajor = DEFAULT_GL_MAJOR;
			_glContextMinor = DEFAULT_GL_MINOR;
		}
	}

	if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_ES) {
		// TODO: Support GLES1 for games
		_glContextType = OpenGL::kContextGLES2;
	} else if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_CORE) {
		_glContextType = OpenGL::kContextGL;

		// Core profile does not allow legacy functionality, which we use.
		// Thus we request a standard OpenGL context.
		_glContextProfileMask = 0;
		_glContextMajor = DEFAULT_GL_MAJOR;
		_glContextMinor = DEFAULT_GL_MINOR;
	} else {
		_glContextType = OpenGL::kContextGL;
	}
#endif
#else
	_glContextType = OpenGL::kContextGL;
#endif

	_vsync = ConfMan.getBool("vsync");
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
		(f == OSystem::kFeatureStretchMode) ||
		(f == OSystem::kFeatureOverlaySupportsAlpha && _overlayFormat.aBits() > 3);
}

bool OpenGLSdlGraphics3dManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
		case OSystem::kFeatureVSync:
			return _vsync;
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
		case OSystem::kFeatureVSync:
			assert(_transactionMode != kTransactionNone);
			_vsync = enable;
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

const OSystem::GraphicsMode glStretchModes[] = {
	{"center", _s("Center"), STRETCH_CENTER},
	{"pixel-perfect", _s("Pixel-perfect scaling"), STRETCH_INTEGRAL},
	{"even-pixels", _s("Even pixels scaling"), STRETCH_INTEGRAL_AR},
	{"fit", _s("Fit to window"), STRETCH_FIT},
	{"stretch", _s("Stretch to window"), STRETCH_STRETCH},
	{"fit_force_aspect", _s("Fit to window (4:3)"), STRETCH_FIT_FORCE_ASPECT},
	{nullptr, nullptr, 0}
};

const OSystem::GraphicsMode *OpenGLSdlGraphics3dManager::getSupportedStretchModes() const {
	return glStretchModes;
}

int OpenGLSdlGraphics3dManager::getDefaultStretchMode() const {
	return STRETCH_FIT;
}

bool OpenGLSdlGraphics3dManager::setStretchMode(int mode) {
	assert(_transactionMode != kTransactionNone);

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

int OpenGLSdlGraphics3dManager::getStretchMode() const {
	return _stretchMode;
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool needsWindowReset = false;
	if (_window->getSDLWindow() && SDL_GL_GetCurrentContext()) {
		// The anti-aliasing setting cannot be changed without recreating the window.
		// So check if the window needs to be recreated.

		int currentSamples = 0;

		#if defined(__EMSCRIPTEN__)
		// SDL_GL_MULTISAMPLESAMPLES isn't available on a  WebGL 1.0 context
		// (or not bridged in Emscripten?). This forces a windows reset.
		currentSamples = -1;
		#else
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &currentSamples);
		#endif

		// When rendering to a framebuffer, MSAA is enabled on that framebuffer, not on the screen
		int targetSamples = shouldRenderToFramebuffer() ? 0 : _antialiasing;

		if (currentSamples != targetSamples) {
			needsWindowReset = true;
		}
	}

	deinitializeRenderer();

	if (needsWindowReset) {
		_window->destroyWindow();
	}
#endif

	createOrUpdateScreen();

	int glflag;
	const GLubyte *str;
	str = glGetString(GL_VENDOR);
	debug(2, "INFO: OpenGL Vendor: %s", str);
	str = glGetString(GL_RENDERER);
	debug(2, "INFO: OpenGL Renderer: %s", str);
	str = glGetString(GL_VERSION);
	debug(2, "INFO: OpenGL Version: %s", str);
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &glflag);
	debug(2, "INFO: OpenGL Red bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &glflag);
	debug(2, "INFO: OpenGL Green bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &glflag);
	debug(2, "INFO: OpenGL Blue bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glflag);
	debug(2, "INFO: OpenGL Alpha bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glflag);
	debug(2, "INFO: OpenGL Z buffer depth bits: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glflag);
	debug(2, "INFO: OpenGL Double Buffer: %d", glflag);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &glflag);
	debug(2, "INFO: OpenGL Stencil buffer bits: %d", glflag);
#ifdef USE_OPENGL_SHADERS
	debug(2, "INFO: GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
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

#if SDL_VERSION_ATLEAST(2, 0, 1)
	int obtainedWidth = 0, obtainedHeight = 0;
	SDL_GL_GetDrawableSize(_window->getSDLWindow(), &obtainedWidth, &obtainedHeight);
#else
	int obtainedWidth = effectiveWidth;
	int obtainedHeight = effectiveHeight;
#endif

	_surfaceRenderer = OpenGL::createBestSurfaceRenderer();
	_overlayFormat = OpenGL::Texture::getRGBAPixelFormat();

	if (renderToFrameBuffer) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_frameBuffer = createFramebuffer(_engineRequestedWidth, _engineRequestedHeight);
		_frameBuffer->attach();
		handleResize(_engineRequestedWidth, _engineRequestedHeight);
	} else {
		handleResize(obtainedWidth, obtainedHeight);
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

	handleResize(newWidth, newHeight);
#else
	handleResize(width, height);
#endif
}

void OpenGLSdlGraphics3dManager::handleResizeImpl(const int width, const int height) {
	// Update the overlay
	delete _overlayScreen;
	_overlayScreen = new OpenGL::TiledSurface(width, height, _overlayFormat);

	// Clear the overlay background so it is not displayed distorted while resizing
	delete _overlayBackground;
	_overlayBackground = nullptr;

	// Re-setup the scaling for the screen
	recalculateDisplayAreas();

	// Something changed, so update the screen change ID.
	_screenChangeCount++;
}

bool OpenGLSdlGraphics3dManager::gameNeedsAspectRatioCorrection() const {
	if (_lockAspectRatio) {
		const uint width = getWidth();
		const uint height = getHeight();

		// In case we enable aspect ratio correction we force a 4/3 ratio.
		// But just for 320x200 and 640x400 games, since other games do not need
		// this.
		return (width == 320 && height == 200) || (width == 640 && height == 400);
	}

	return false;
}

void OpenGLSdlGraphics3dManager::initializeOpenGLContext() const {
	OpenGLContext.initialize(_glContextType);

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

	bool clear = false;

	// Unfortunately, SDL does not provide a list of valid pixel formats
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
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, _glContextProfileMask);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _glContextMajor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _glContextMinor);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
		uint32 sdlflags = SDL_WINDOW_OPENGL;

#ifdef NINTENDO_SWITCH
		// Switch quirk: Switch seems to need this flag, otherwise the screen
		// is zoomed when switching from Normal graphics mode to OpenGL
		sdlflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif

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
					clear = true;

#ifdef USE_IMGUI
					// Setup Dear ImGui
					initImGui(nullptr, _glContext);
#endif
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

	if (it == pixelFormats.end())
		return false;

	initializeOpenGLContext();

	if (clear)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	return true;
}

bool OpenGLSdlGraphics3dManager::shouldRenderToFramebuffer() const {
	bool engineSupportsArbitraryResolutions = !g_engine || g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);
	return !engineSupportsArbitraryResolutions && _supportsFrameBuffer;
}

void OpenGLSdlGraphics3dManager::drawOverlay() {
	_surfaceRenderer->prepareState();

	glViewport(_overlayDrawRect.left, _windowHeight - _overlayDrawRect.top - _overlayDrawRect.height(), _overlayDrawRect.width(), _overlayDrawRect.height());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (_overlayBackground) {
		_overlayBackground->draw(_surfaceRenderer);
	}

	_surfaceRenderer->enableAlphaBlending(true);
	_surfaceRenderer->setFlipY(true);
	_overlayScreen->draw(_surfaceRenderer);

	_surfaceRenderer->restorePreviousState();
}

OpenGL::FrameBuffer *OpenGLSdlGraphics3dManager::createFramebuffer(uint width, uint height) {
#if !USE_FORCED_GLES2
	if (_antialiasing && OpenGLContext.framebufferObjectMultisampleSupported) {
		return new OpenGL::MultiSampleFrameBuffer(width, height, _antialiasing);
	} else
#endif
	{
		return new OpenGL::FrameBuffer(width, height);
	}
}

void OpenGLSdlGraphics3dManager::updateScreen() {

	GLint prevStateViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevStateViewport);
	if (_frameBuffer) {
		_frameBuffer->detach();
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

		drawOverlay();
	}

#ifdef EMSCRIPTEN
	if (_queuedScreenshot) {
		SdlGraphicsManager::saveScreenshot();
		_queuedScreenshot = false;
	}
#endif

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
	renderImGui();
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow(_window->getSDLWindow());
#else
	SDL_GL_SwapBuffers();
#endif

	if (_frameBuffer) {
		_frameBuffer->attach();
	}
	glViewport(prevStateViewport[0], prevStateViewport[1], prevStateViewport[2], prevStateViewport[3]);
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

void OpenGLSdlGraphics3dManager::showOverlay(bool inGUI) {
	if (_overlayVisible && _overlayInGUI == inGUI) {
		return;
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

void OpenGLSdlGraphics3dManager::hideOverlay() {
	if (!_overlayVisible) {
		return;
	}
	WindowedGraphicsManager::hideOverlay();

	delete _overlayBackground;
	_overlayBackground = nullptr;

	if (_surfaceRenderer) {
		// If there is double buffering we need to redraw twice
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

void OpenGLSdlGraphics3dManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlayScreen->copyRectToSurface(buf, pitch, x, y, w, h);
}

void OpenGLSdlGraphics3dManager::clearOverlay() {
	_overlayScreen->fill(0);
}

void OpenGLSdlGraphics3dManager::grabOverlay(Graphics::Surface &surface) const {
	const Graphics::Surface *overlayData = _overlayScreen->getBackingSurface();

	assert(surface.w >= overlayData->w);
	assert(surface.h >= overlayData->h);
	assert(surface.format.bytesPerPixel == overlayData->format.bytesPerPixel);

	const byte *src = (const byte *)overlayData->getPixels();
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch, overlayData->pitch, overlayData->w, overlayData->h, overlayData->format.bytesPerPixel);
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

	OpenGLContext.reset();
}

int16 OpenGLSdlGraphics3dManager::getOverlayHeight() const {
	return _overlayScreen->getHeight();
}

int16 OpenGLSdlGraphics3dManager::getOverlayWidth() const {
	return _overlayScreen->getWidth();
}

bool OpenGLSdlGraphics3dManager::showMouse(bool visible) {
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
	return true;
}

void OpenGLSdlGraphics3dManager::showSystemMouseCursor(bool visible) {
	// HACK: SdlGraphicsManager disables the system cursor when the mouse is in the
	// active draw rect, however the 3D graphics manager uses it instead of the
	// standard mouse graphic.
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void OpenGLSdlGraphics3dManager::deinitializeRenderer() {
#ifdef USE_IMGUI
	destroyImGui();
#endif

	SDL_GL_DeleteContext(_glContext);
	_glContext = nullptr;
}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)

#ifdef EMSCRIPTEN
void OpenGLSdlGraphics3dManager::saveScreenshot() {
	_queuedScreenshot = true;
}
#endif

bool OpenGLSdlGraphics3dManager::saveScreenshot(const Common::Path &filename) const {
	// Largely based on the implementation from ScummVM
	uint width = _overlayScreen->getWidth();
	uint height = _overlayScreen->getHeight();

#ifdef EMSCRIPTEN
	const uint lineSize        = width * 4; // RGBA (see comment below)
#else
	uint linePaddingSize = width % 4;
	uint lineSize = width * 3 + linePaddingSize;
#endif

	Common::DumpFile out;
	if (!out.open(filename)) {
		return false;
	}

	Common::Array<uint8> pixels;
	pixels.resize(lineSize * height);
#ifdef EMSCRIPTEN
	// WebGL doesn't support GL_RGB, see https://registry.khronos.org/webgl/specs/latest/1.0/#5.14.12:
	// "Only two combinations of format and type are accepted. The first is format RGBA and type UNSIGNED_BYTE.
	// The second is an implementation-chosen format. " and the implementation-chosen formats are buggy:
	// https://github.com/KhronosGroup/WebGL/issues/2747
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels.front());
	const Graphics::PixelFormat format(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else

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

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
void *OpenGLSdlGraphics3dManager::getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) {
	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
	Graphics::Surface *s = image.convertTo(Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0));
	glPixelStorei(GL_UNPACK_ALIGNMENT, s->format.bytesPerPixel);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB, GL_UNSIGNED_BYTE, s->getPixels());
	s->free();
	delete s;
	return (void *)(intptr_t)image_texture;
}

void OpenGLSdlGraphics3dManager::freeImGuiTexture(void *texture) {
	GLuint textureID = (intptr_t)texture;
	glDeleteTextures(1, &textureID);
}
#endif

#endif
