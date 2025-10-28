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

#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#include "backends/graphics/opengl/texture.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "graphics/scaler/aspect.h"
#ifdef USE_SCALERS
#include "graphics/scalerplugin.h"
#endif

#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
static void sdlGLDestroyContext(SDL_GLContext context) {
	SDL_GL_DestroyContext(context);
}
#elif SDL_VERSION_ATLEAST(2, 0, 0)
static void sdlGLDestroyContext(SDL_GLContext context) {
	SDL_GL_DeleteContext(context);
}
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
static bool sdlSetSwapInterval(int interval) {
	return SDL_GL_SetSwapInterval(interval);
}
#elif SDL_VERSION_ATLEAST(2, 0, 0)
static bool sdlSetSwapInterval(int interval) {
	return SDL_GL_SetSwapInterval(interval) == 0;
}
#endif

#if !USE_FORCED_GL && !USE_FORCED_GLES && !USE_FORCED_GLES2
#if SDL_VERSION_ATLEAST(3, 0, 0)
static bool sdlGetAttribute(SDL_GLAttr attr, int *value) {
	return SDL_GL_GetAttribute(attr, value);
}
#else
static bool sdlGetAttribute(SDL_GLattr attr, int *value) {
	return SDL_GL_GetAttribute(attr, value) == 0;
}
#endif
#endif

OpenGLSdlGraphicsManager::OpenGLSdlGraphicsManager(SdlEventSource *eventSource, SdlWindow *window)
	: SdlGraphicsManager(eventSource, window), _lastRequestedHeight(0),
#if SDL_VERSION_ATLEAST(2, 0, 0)
	  _glContext(),
#else
	  _lastVideoModeLoad(0),
#endif
	  _graphicsScale(2), _gotResize(false), _wantsFullScreen(false), _ignoreResizeEvents(0),
	  _effectiveAntialiasing(-1), _requestedAntialiasing(0), _resizable(true),
	  _desiredFullscreenWidth(0), _desiredFullscreenHeight(0) {

	// Set up proper SDL OpenGL context creation.
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Context version 1.3 is chosen arbitrarily based on what most shader
	// extensions were written against.
	enum {
		DEFAULT_GL_MAJOR = 1,
		DEFAULT_GL_MINOR = 3,

		DEFAULT_GLES_MAJOR = 1,
		DEFAULT_GLES_MINOR = 1,

		DEFAULT_GLES2_MAJOR = 2,
		DEFAULT_GLES2_MINOR = 0
	};

#if USE_FORCED_GL
	_glContextType = OpenGL::kContextGL;
	_glContextProfileMask = 0;
	_glContextMajor = DEFAULT_GL_MAJOR;
	_glContextMinor = DEFAULT_GL_MINOR;
#elif USE_FORCED_GLES
	_glContextType = OpenGL::kContextGLES;
	_glContextProfileMask = SDL_GL_CONTEXT_PROFILE_ES;
	_glContextMajor = DEFAULT_GLES_MAJOR;
	_glContextMinor = DEFAULT_GLES_MINOR;
#elif USE_FORCED_GLES2
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
	if (!sdlGetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &_glContextProfileMask)) {
		_glContextProfileMask = 0;
		noDefaults = true;
	}

	if (!sdlGetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &_glContextMajor)) {
		noDefaults = true;
	}

	if (!sdlGetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &_glContextMinor)) {
		noDefaults = true;
	}

	if (noDefaults) {
		if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_ES) {
			_glContextMajor = DEFAULT_GLES_MAJOR;
			_glContextMinor = DEFAULT_GLES_MINOR;
		} else {
			_glContextProfileMask = 0;
			_glContextMajor = DEFAULT_GL_MAJOR;
			_glContextMinor = DEFAULT_GL_MINOR;
		}
	}

	if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_ES) {
		if (_glContextMajor >= 2) {
			_glContextType = OpenGL::kContextGLES2;
		} else {
			_glContextType = OpenGL::kContextGLES;
		}
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
#else // SDL_VERSION_ATLEAST(2, 0, 0)
	_glContextType = OpenGL::kContextGL;
#endif

	_vsync = ConfMan.getBool("vsync");

	// Retrieve a list of working fullscreen modes
	Common::Rect desktopRes = _window->getDesktopResolution();
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// With SDL2 we use the SDL_WINDOW_FULLSCREEN_DESKTOP flag.
	// Thus SDL always use the desktop resolution and it is useless to try to use something else.
	// Do nothing here as adding the desktop resolution to _fullscreenVideoModes is done as a fallback.
	_fullscreenVideoModes.push_back(VideoMode(desktopRes.width(), desktopRes.height()));

	if (ConfMan.hasKey("force_frame_update")) {
		_forceFrameUpdate = ConfMan.getInt("force_frame_update", Common::ConfigManager::kApplicationDomain);
	}
#else
	const SDL_Rect *const *availableModes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);
	// TODO: NULL means that there are no fullscreen modes supported. We
	// should probably use this information and disable any fullscreen support
	// in this case.
	if (availableModes != NULL && availableModes != (void *)-1) {
		for (;*availableModes; ++availableModes) {
			const SDL_Rect *mode = *availableModes;

			_fullscreenVideoModes.push_back(VideoMode(mode->w, mode->h));
		}
	}
#endif

	// Sort the modes in ascending order.
	Common::sort(_fullscreenVideoModes.begin(), _fullscreenVideoModes.end());

	// Strip duplicates in video modes.
	for (uint i = 0; i + 1 < _fullscreenVideoModes.size();) {
		if (_fullscreenVideoModes[i] == _fullscreenVideoModes[i + 1]) {
			_fullscreenVideoModes.remove_at(i);
		} else {
			++i;
		}
	}

	// In case SDL is fine with every mode we will force the desktop mode.
	// TODO? We could also try to add some default resolutions here.
	if (_fullscreenVideoModes.empty() && !desktopRes.isEmpty()) {
		_fullscreenVideoModes.push_back(VideoMode(desktopRes.width(), desktopRes.height()));
	}

	// Get information about display sizes from the previous runs.
	if (ConfMan.hasKey("last_fullscreen_mode_width", Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("last_fullscreen_mode_height", Common::ConfigManager::kApplicationDomain)) {
		_desiredFullscreenWidth  = ConfMan.getInt("last_fullscreen_mode_width", Common::ConfigManager::kApplicationDomain);
		_desiredFullscreenHeight = ConfMan.getInt("last_fullscreen_mode_height", Common::ConfigManager::kApplicationDomain);
	} else {
		// Use the desktop resolutions when no previous default has been setup.
		_desiredFullscreenWidth  = desktopRes.width();
		_desiredFullscreenHeight = desktopRes.height();
	}
}

OpenGLSdlGraphicsManager::~OpenGLSdlGraphicsManager() {
	deinitOpenGLContext();
}

void OpenGLSdlGraphicsManager::deinitOpenGLContext() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (!_glContext) {
		return;
	}

#ifdef USE_IMGUI
	destroyImGui();
#endif

	notifyContextDestroy();
	sdlGLDestroyContext(_glContext);

	_glContext = nullptr;
	_effectiveAntialiasing = -1;
#else // SDL_VERSION_ATLEAST(2, 0, 0)
	if (_hwScreen) {
		notifyContextDestroy();
	}
	_hwScreen = nullptr;
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void OpenGLSdlGraphicsManager::destroyingWindow() {
	// We are about to destroy the window: cleanup the context first
	deinitOpenGLContext();
}
#endif

bool OpenGLSdlGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
	case OSystem::kFeatureIconifyWindow:
	case OSystem::kFeatureVSync:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	case OSystem::kFeatureRotationMode:
#endif
		return true;

	default:
		return OpenGLGraphicsManager::hasFeature(f);
	}
}

void OpenGLSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		assert(getTransactionMode() != kTransactionNone);
		_wantsFullScreen = enable;
		break;

	case OSystem::kFeatureVSync:
		assert(getTransactionMode() != kTransactionNone);
		_vsync = enable;
		break;

	case OSystem::kFeatureIconifyWindow:
		if (enable) {
			_window->iconifyWindow();
		}
		break;

	default:
		OpenGLGraphicsManager::setFeatureState(f, enable);
	}
}

bool OpenGLSdlGraphicsManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
#if SDL_VERSION_ATLEAST(3, 0, 0)
		if (_window && _window->getSDLWindow()) {
			// SDL_GetWindowFullscreenMode returns a pointer to the exclusive fullscreen mode to use or NULL for borderless
			return ((SDL_GetWindowFlags(_window->getSDLWindow()) & SDL_WINDOW_FULLSCREEN) != 0) && (SDL_GetWindowFullscreenMode(_window->getSDLWindow()) == NULL);
		} else {
			return _wantsFullScreen;
		}
#elif SDL_VERSION_ATLEAST(2, 0, 0)
		if (_window && _window->getSDLWindow()) {
			return (SDL_GetWindowFlags(_window->getSDLWindow()) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
		} else {
			return _wantsFullScreen;
		}
#else
		if (_hwScreen) {
			return (_hwScreen->flags & SDL_FULLSCREEN) != 0;
		} else {
			return _wantsFullScreen;
		}
#endif

	case OSystem::kFeatureVSync:
		return _vsync;

	default:
		return OpenGLGraphicsManager::getFeatureState(f);
	}
}

bool OpenGLSdlGraphicsManager::canSwitchFullscreen() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// In SDL2+, we can always switch the fullscreen state
	// The OpenGL context is not reset in this case.
	return true;
#elif defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	// In SDL 1.2, we can only switch dynamically if we are in 2D
	// In 3D, the context may get reset which would upset the engine.
	return _renderer3d == nullptr;
#else
	// In SDL1.2 with only 2D, no problem
	return true;
#endif
}

float OpenGLSdlGraphicsManager::getHiDPIScreenFactor() const {
	return _window->getDpiScalingFactor();
}

void OpenGLSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	// HACK: This is stupid but the SurfaceSDL backend defaults to 2x. This
	// assures that the launcher (which requests 320x200) has a reasonable
	// size. It also makes small games have a reasonable size (i.e. at least
	// 640x400). We follow the same logic here until we have a better way to
	// give hints to our backend for that.
	if (w > 320) {
		_graphicsScale = 1;
	} else {
		_graphicsScale = 2;
	}

	if (ConfMan.getBool("force_resize", Common::ConfigManager::kApplicationDomain)) {
		notifyResize(w, h);
	}

	return OpenGLGraphicsManager::initSize(w, h, format);
}

void OpenGLSdlGraphicsManager::updateScreen() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	static uint32 lastUpdateTime = 0;

	if (_forceFrameUpdate) {
		if (_forceRedraw) {
			lastUpdateTime = SDL_GetTicks();
		} else {
			// This works for the most part. Anything between 20 and 40 yields
			// around 24fps. It's not till we set it to 20 that it changes and
			// jumps to around 40fps.
			// 50 ~ 16fps
			// 40 ~ 24fps
			// 20 ~ 45fps
			if (SDL_GetTicks() - lastUpdateTime > _forceFrameUpdate) {
				_forceRedraw = true;
				lastUpdateTime = SDL_GetTicks();
			}
		}
	}
#endif

	if (_ignoreResizeEvents) {
		--_ignoreResizeEvents;
	}

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
	if (_imGuiCallbacks.render) {
		_forceRedraw = true;
	}
#endif

	OpenGLGraphicsManager::updateScreen();
}

void OpenGLSdlGraphicsManager::notifyVideoExpose() {
	_forceRedraw = true;
}

void OpenGLSdlGraphicsManager::notifyResize(const int width, const int height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// We sometime get inaccurate resize events from SDL2. So use the real drawable size
	// we get from SDL2 and ignore the event data.
	// The issue for example occurs when switching from fullscreen to windowed mode or
	// when switching between different fullscreen resolutions because SDL_DestroyWindow
	// for a fullscreen window that doesn't have the SDL_WINDOW_FULLSCREEN_DESKTOP flag
	// causes a SDL_WINDOWEVENT_RESIZED event with the old resolution to be sent, and this
	// event is processed after recreating the window at the new resolution.
	int currentWidth, currentHeight;
	getWindowSizeFromSdl(&currentWidth, &currentHeight);
	float dpiScale = _window->getSdlDpiScalingFactor();

	if (ConfMan.getBool("force_resize", Common::ConfigManager::kApplicationDomain)) {
		currentWidth = width;
		currentHeight = height;
	}

	debug(3, "req: %d x %d  cur: %d x %d, scale: %f", width, height, currentWidth, currentHeight, dpiScale);

	if (ConfMan.getBool("force_resize", Common::ConfigManager::kApplicationDomain)) {
		createOrUpdateWindow(currentWidth, currentHeight, 0);
	}

	handleResize(currentWidth, currentHeight);

	// Remember window size in windowed mode
	if (!_wantsFullScreen) {
		currentWidth = (int)(currentWidth / dpiScale + 0.5f);
		currentHeight = (int)(currentHeight / dpiScale + 0.5f);

		// Check if the ScummVM window is maximized and store the current
		// window dimensions.
		if (SDL_GetWindowFlags(_window->getSDLWindow()) & SDL_WINDOW_MAXIMIZED) {
			ConfMan.setInt("window_maximized_width", currentWidth, Common::ConfigManager::kApplicationDomain);
			ConfMan.setInt("window_maximized_height", currentHeight, Common::ConfigManager::kApplicationDomain);
			ConfMan.setBool("window_maximized", true, Common::ConfigManager::kApplicationDomain);
		} else {
			ConfMan.setInt("last_window_width", currentWidth, Common::ConfigManager::kApplicationDomain);
			ConfMan.setInt("last_window_height", currentHeight, Common::ConfigManager::kApplicationDomain);
			ConfMan.setBool("window_maximized", false, Common::ConfigManager::kApplicationDomain);
		}
		ConfMan.flushToDisk();
	}

#else
	if (!_ignoreResizeEvents && _hwScreen && !(_hwScreen->flags & SDL_FULLSCREEN)) {
		// We save that we handled a resize event here. We need to know this
		// so we do not overwrite the users requested window size whenever we
		// switch aspect ratio or similar.
		_gotResize = true;
		if (!setupMode(width, height)) {
			warning("OpenGLSdlGraphicsManager::notifyResize: Resize failed ('%s')", SDL_GetError());
			g_system->quit();
		}
	}
#endif
}

bool OpenGLSdlGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, bool resizable, int antialiasing) {
	// This function should never be called from notifyResize thus we know
	// that the requested size came from somewhere else.
	_gotResize = false;

	// Save the requested dimensions.
	_lastRequestedWidth  = requestedWidth;
	_lastRequestedHeight = requestedHeight;

	// Fetch current desktop resolution and determining max. width and height
	Common::Rect desktopRes = _window->getDesktopResolution();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool isMaximized = ConfMan.getBool("window_maximized", Common::ConfigManager::kApplicationDomain);
	if (!_wantsFullScreen) {
		if (isMaximized && ConfMan.hasKey("window_maximized_width", Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("window_maximized_height", Common::ConfigManager::kApplicationDomain)) {
			// Set the window size to the values stored when the window was maximized
			// for the last time.
			requestedWidth  = ConfMan.getInt("window_maximized_width", Common::ConfigManager::kApplicationDomain);
			requestedHeight = ConfMan.getInt("window_maximized_height", Common::ConfigManager::kApplicationDomain);

		} else if (!isMaximized && ConfMan.hasKey("last_window_width", Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("last_window_height", Common::ConfigManager::kApplicationDomain)) {
			// Load previously stored window dimensions.
			requestedWidth  = ConfMan.getInt("last_window_width", Common::ConfigManager::kApplicationDomain);
			requestedHeight = ConfMan.getInt("last_window_height", Common::ConfigManager::kApplicationDomain);

		} else {
			// Set the basic window size based on the desktop resolution
			// since we have no values stored, e.g. on first launch.
			requestedWidth  = MAX<uint>(desktopRes.width() / 2, 640);
			requestedHeight = requestedWidth * 3 / 4;

			// Save current window dimensions
			ConfMan.setInt("last_window_width", requestedWidth, Common::ConfigManager::kApplicationDomain);
			ConfMan.setInt("last_window_height", requestedHeight, Common::ConfigManager::kApplicationDomain);
			ConfMan.flushToDisk();
		}

		if (requestedHeight == 0)
			requestedHeight = 100; // Add at least some sane value instead of dividing by zero
	}
#else
	// Set the basic window size based on the desktop resolution
	// since we cannot reliably determine the current window state
	// on SDL1.
	requestedWidth  = MAX<uint>(desktopRes.width() / 2, 640);
	requestedHeight = requestedWidth * 3 / 4;
#endif

	// In order to prevent any unnecessary downscaling (e.g. when launching
	// a game in 800x600 while having a smaller screen size stored in the configuration file),
	// we override the window dimensions with the "real" resolution request made by the engine.
	if ((requestedWidth < _lastRequestedWidth  * _graphicsScale || requestedHeight < _lastRequestedHeight * _graphicsScale) && ConfMan.getActiveDomain()) {
		requestedWidth  = _lastRequestedWidth  * _graphicsScale;
		requestedHeight = _lastRequestedHeight * _graphicsScale;
	}

	// Set allowed dimensions
	uint maxAllowedWidth   = desktopRes.width();
	uint maxAllowedHeight  = desktopRes.height();
	float ratio = (float)requestedWidth / (float)requestedHeight;

	// Check if we request a larger window than physically possible,
	// e.g. by starting with additional launcher parameters forcing
	// specific (openGL) scaler modes that could exceed the desktop/screen size
	if (requestedWidth  > maxAllowedWidth) {
		requestedWidth  = maxAllowedWidth;
		requestedHeight = requestedWidth / ratio;
	}

	if (requestedHeight > maxAllowedHeight) {
		requestedHeight = maxAllowedHeight;
		requestedWidth  = requestedHeight * ratio;
	}

	// Force the requested size if not resizable
	if (!resizable) {
		requestedWidth = _lastRequestedWidth;
		requestedHeight = _lastRequestedHeight;
	}

	_requestedAntialiasing = antialiasing;
	_resizable = resizable;

	// Set up the mode
	return setupMode(requestedWidth, requestedHeight);
}

void OpenGLSdlGraphicsManager::refreshScreen() {
	// Swap OpenGL buffers
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
}

void OpenGLSdlGraphicsManager::handleResizeImpl(const int width, const int height) {
	OpenGLGraphicsManager::handleResizeImpl(width, height);
	SdlGraphicsManager::handleResizeImpl(width, height);
}

#ifdef EMSCRIPTEN
void OpenGLSdlGraphicsManager::saveScreenshot() {
	_queuedScreenshot = true;
}
#endif

bool OpenGLSdlGraphicsManager::saveScreenshot(const Common::Path &filename) const {
	return OpenGLGraphicsManager::saveScreenshot(filename);
}

bool OpenGLSdlGraphicsManager::setupMode(uint width, uint height) {
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	const bool supportsAntialiasing = g_system->getSupportedAntiAliasingLevels().size() > 0;
#else
	const bool supportsAntialiasing = false;
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// If current antialiasing is not what's expected, destroy the window (and the context)
	// This will force to recreate it
	if (supportsAntialiasing && _effectiveAntialiasing != _requestedAntialiasing && _glContext) {
		_window->destroyWindow();
	}
#endif

	// In case we request a fullscreen mode we will use the mode the user
	// has chosen last time or the biggest mode available.
	if (_wantsFullScreen) {
		if (_desiredFullscreenWidth && _desiredFullscreenHeight) {
			// In case only a distinct set of modes is available we check
			// whether the requested mode is actually available.
			if (!_fullscreenVideoModes.empty()) {
				VideoModeArray::const_iterator i = Common::find(_fullscreenVideoModes.begin(),
				                                                _fullscreenVideoModes.end(),
				                                                VideoMode(_desiredFullscreenWidth, _desiredFullscreenHeight));
				// It's not available fall back to default.
				if (i == _fullscreenVideoModes.end()) {
					_desiredFullscreenWidth = 0;
					_desiredFullscreenHeight = 0;
				}
			}
		}

		// In case no desired mode has been set we default to the biggest mode
		// available or the requested mode in case we don't know any
		// any fullscreen modes.
		if (!_desiredFullscreenWidth || !_desiredFullscreenHeight) {
			if (!_fullscreenVideoModes.empty()) {
				VideoModeArray::const_iterator i = _fullscreenVideoModes.end();
				--i;

				_desiredFullscreenWidth  = i->width;
				_desiredFullscreenHeight = i->height;
			} else {
				_desiredFullscreenWidth  = width;
				_desiredFullscreenHeight = height;
			}
		}

		// Remember our choice.
		ConfMan.setInt("last_fullscreen_mode_width", _desiredFullscreenWidth, Common::ConfigManager::kApplicationDomain);
		ConfMan.setInt("last_fullscreen_mode_height", _desiredFullscreenHeight, Common::ConfigManager::kApplicationDomain);
	}

	Common::Array<Graphics::PixelFormat> formats;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window->getSDLWindow() && _defaultFormat.bytesPerPixel > 0)
#else
	if (_hwScreen && _defaultFormat.bytesPerPixel > 0)
#endif
	{
		formats = { _defaultFormat, _defaultFormatAlpha };
	} else {
		formats = {
			// First format: RGB888/RGBA8888
			OpenGL::Texture::getRGBPixelFormat(),
			OpenGL::Texture::getRGBAPixelFormat(),
			// Second format: RGB565/RGB5551
			Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
			Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),
			// Third format: RGB5551/RGB5551
			Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),
			Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)
		};
	}

	int antialiasing = supportsAntialiasing ? _requestedAntialiasing : 0;
	for (Common::Array<Graphics::PixelFormat>::const_iterator it = formats.begin(); ; it += 2) {
		if (it == formats.end()) {
			if (antialiasing == 0) {
				// We failed to get a proper window
				return false;
			}
			antialiasing = 0;
			it = formats.begin();
		}

#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_GL_ResetAttributes();
#endif
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, it->rBits());
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, it->gBits());
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, it->bBits());
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, it->aBits());
		if (_glContextType != OpenGL::kContextGLES) {
			// Always request 24-bits depth buffer and stencil buffer even in 2D to avoid extraneous context switches
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		} else {
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		}
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		if (supportsAntialiasing) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, antialiasing > 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasing);
		}

#if SDL_VERSION_ATLEAST(2, 0, 0)
		// Request a OpenGL (ES) context we can use.
		// This must be done before any window creation
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, _glContextProfileMask);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _glContextMajor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _glContextMinor);

#if SDL_VERSION_ATLEAST(3, 0, 0)
		uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#else
		uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
#endif
		if (_resizable) {
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if (_wantsFullScreen) {
			// On Linux/X11, when toggling to fullscreen, the window manager saves
			// the window size to be able to restore it when going back to windowed mode.
			// If the user configured ScummVM to start in fullscreen mode, we first
			// create a window and then toggle it to fullscreen to give the window manager
			// a chance to save the window size. That way if the user switches back
			// to windowed mode, the window manager has a window size to apply instead
			// of leaving the window at the fullscreen resolution size.
			const char *driver = SDL_GetCurrentVideoDriver();
			if (!_window->getSDLWindow() && driver && strcmp(driver, "x11") == 0) {
				_window->createOrUpdateWindow(width, height, flags);
			}

			width  = _desiredFullscreenWidth;
			height = _desiredFullscreenHeight;

#if SDL_VERSION_ATLEAST(3, 0, 0)
			flags |= SDL_WINDOW_FULLSCREEN;
			SDL_SetWindowFullscreenMode(_window->getSDLWindow(), NULL);
			SDL_SyncWindow(_window->getSDLWindow());
#else
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
		}

		if (!_wantsFullScreen && ConfMan.getBool("window_maximized", Common::ConfigManager::kApplicationDomain)) {
			flags |= SDL_WINDOW_MAXIMIZED;
		}

#if defined(NINTENDO_SWITCH) && !SDL_VERSION_ATLEAST(3, 0, 0)
		// Switch quirk: Switch seems to need this flag, otherwise the screen
		// is zoomed when switching from Normal graphics mode to OpenGL
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
		if (!createOrUpdateWindow(width, height, flags)) {
			// Try the next pixel format
			continue;
		}

		const bool newContext = !_glContext;
		if (newContext) {
			// If createOrUpdateWindow reused the exisiting window, _glContext will still have its previous value
			_glContext = SDL_GL_CreateContext(_window->getSDLWindow());
			if (!_glContext) {
				warning("SDL_GL_CreateContext failed: %s", SDL_GetError());
				// Try the next pixel format
				continue;
			}
		}

		// Now that we have a context, the AA is really effective
		_effectiveAntialiasing = antialiasing;

		if (!sdlSetSwapInterval(_vsync ? 1 : 0)) {
			warning("Unable to %s VSync: %s", _vsync ? "enable" : "disable", SDL_GetError());
		}

		if (newContext) {
			notifyContextCreate(_glContextType, new OpenGL::Backbuffer(), it[0], it[1]);
		}
		int actualWidth, actualHeight;
		getWindowSizeFromSdl(&actualWidth, &actualHeight);

		handleResize(actualWidth, actualHeight);

#ifdef USE_IMGUI
		if (newContext) {
			// Setup Dear ImGui
			initImGui(nullptr, _glContext);
		}
#endif

#ifdef WIN32
		// WORKAROUND: Prevent (nearly) offscreen positioning of the ScummVM window by forcefully
		// trigger a re-positioning event to center the window.
		if (!_wantsFullScreen && !(SDL_GetWindowFlags(_window->getSDLWindow()) & SDL_WINDOW_MAXIMIZED)) {

			// Read the current window position
			int _xWindowPos;
			SDL_GetWindowPosition(_window->getSDLWindow(), &_xWindowPos, nullptr);

			// Relocate the window to the center of the screen in case we try to draw
			// outside the window area. In this case, _xWindowPos always returns 0.
			if (_xWindowPos == 0) {
				SDL_SetWindowPosition(_window->getSDLWindow(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			}
		}
#endif
#else // SDL_VERSION_ATLEAST(2, 0, 0)
		// WORKAROUND: Working around infamous SDL bugs when switching
		// resolutions too fast. This might cause the event system to supply
		// incorrect mouse position events otherwise.
		// Reference: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=665779
		const uint32 curTime = SDL_GetTicks();
		if (_hwScreen && (curTime < _lastVideoModeLoad || curTime - _lastVideoModeLoad < 100)) {
			for (int i = 10; i > 0; --i) {
				SDL_PumpEvents();
				SDL_Delay(10);
			}
		}

		// Destroy OpenGL context before messing with the window: in SDL1.2 we can't now when the window will get destroyed or not
		deinitOpenGLContext();

		uint32 flags = SDL_OPENGL;
		if (_wantsFullScreen) {
			width  = _desiredFullscreenWidth;
			height = _desiredFullscreenHeight;
			flags |= SDL_FULLSCREEN;
		}
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
		if (_resizable && !_renderer3d) {
#else
		if (_resizable) {
#endif
			// In SDL1.2, resizing the window may invalidate the context
			// This would kill all the engine objects
			flags |= SDL_RESIZABLE;
		}

		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, _vsync ? 1 : 0);

		_hwScreen = SDL_SetVideoMode(width, height, 32, flags);

		if (!_hwScreen) {
			// We treat fullscreen requests as a "hint" for now. This means in
			// case it is not available we simply ignore it.
			if (_wantsFullScreen) {
				_hwScreen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL | SDL_RESIZABLE);
			}
		}

		if (!_hwScreen) {
			// Try the next pixel format
			continue;
		}

		// Now that we have a screen, the AA is really effective
		_effectiveAntialiasing = antialiasing;

		// Part of the WORKAROUND mentioned above.
		_lastVideoModeLoad = SDL_GetTicks();

		notifyContextCreate(_glContextType, new OpenGL::Backbuffer(), it[0], it[1]);
		handleResize(_hwScreen->w, _hwScreen->h);

		// Ignore resize events (from SDL) for a few frames, if this isn't
		// caused by a notification from SDL. This avoids bad resizes to a
		// (former) resolution for which we haven't processed an event yet.
		if (!_gotResize)
			_ignoreResizeEvents = 10;

#endif
		// Display a warning if the effective pixel format is not the preferred one
		const bool wantsAA = _requestedAntialiasing > 0;
		const bool gotAA = antialiasing > 0;
		if (it != formats.begin() || (wantsAA && !gotAA)) {
			warning("Couldn't create a %d-bit visual%s, using to %d-bit%s instead",
				formats.front().bpp(),
				wantsAA && !gotAA ? " with AA" : "",
				it->bpp(),
				wantsAA && !gotAA ? " without AA" : "");
		}

		return true;
	}
	// We should never end up here
	return false;
}

bool OpenGLSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START) {
		return SdlGraphicsManager::notifyEvent(event);
	}

	switch ((CustomEventAction) event.customType) {
	case kActionIncreaseScaleFactor:
	case kActionDecreaseScaleFactor: {
		const int direction = event.customType == kActionIncreaseScaleFactor ? +1 : -1;

		if (getFeatureState(OSystem::kFeatureFullscreenMode)) {
			// In case we are in fullscreen we will choose the previous
			// or next mode.

			// In case no modes are available or we only have one mode we do nothing.
			if (_fullscreenVideoModes.size() < 2) {
				return true;
			}

			// Look for the current mode.
			VideoModeArray::const_iterator i = Common::find(_fullscreenVideoModes.begin(),
			                                                _fullscreenVideoModes.end(),
			                                                VideoMode(_desiredFullscreenWidth, _desiredFullscreenHeight));
			if (i == _fullscreenVideoModes.end()) {
				return true;
			}

			// Cycle through the modes in the specified direction.
			if (direction > 0) {
				++i;
				if (i == _fullscreenVideoModes.end()) {
					i = _fullscreenVideoModes.begin();
				}
			} else {
				if (i == _fullscreenVideoModes.begin()) {
					i = _fullscreenVideoModes.end();
				}
				--i;
			}

			_desiredFullscreenWidth  = i->width;
			_desiredFullscreenHeight = i->height;

			// Try to setup the mode.
			if (!setupMode(_lastRequestedWidth, _lastRequestedHeight)) {
				warning("OpenGLSdlGraphicsManager::notifyEvent: Fullscreen resize failed ('%s')", SDL_GetError());
				g_system->quit();
			}
		} else {
			// Calculate the next scaling setting. We approximate the
			// current scale setting in case the user resized the
			// window. Then we apply the direction change.
			int windowWidth = 0, windowHeight = 0;
			getWindowSizeFromSdl(&windowWidth, &windowHeight);

			float dpiScale = _window->getSdlDpiScalingFactor();
			windowWidth = (int)(windowWidth / dpiScale + 0.5f);
			windowHeight = (int)(windowHeight / dpiScale + 0.5f);

			if (direction > 0)
				_graphicsScale = MAX<int>(windowWidth / _lastRequestedWidth, windowHeight / _lastRequestedHeight);
			else
				_graphicsScale = 1 + MIN<int>((windowWidth - 1) / _lastRequestedWidth, (windowHeight - 1) / _lastRequestedHeight);
			_graphicsScale = MAX<int>(_graphicsScale + direction, 1);

			// Since we overwrite a user resize here we reset its
			// flag here. This makes enabling AR smoother because it
			// will change the window size like in surface SDL.
			_gotResize = false;

			// Try to setup the mode.
#if SDL_VERSION_ATLEAST(2, 0, 0)
			unlockWindowSize();
#endif
			if (!setupMode(_lastRequestedWidth * _graphicsScale, _lastRequestedHeight * _graphicsScale)) {
				warning("OpenGLSdlGraphicsManager::notifyEvent: Window resize failed ('%s')", SDL_GetError());
				g_system->quit();
			}
		}

#ifdef USE_OSD
		int windowWidth = 0, windowHeight = 0;
		getWindowSizeFromSdl(&windowWidth, &windowHeight);
		const Common::U32String osdMsg = Common::U32String::format(_("Resolution: %dx%d"), windowWidth, windowHeight);
		displayMessageOnOSD(osdMsg);
#endif

		return true;
	}

#ifdef USE_SCALERS
	case kActionNextScaleFilter:
	case kActionPreviousScaleFilter: {
		if (_scalerPlugins.size() > 0) {
			const int direction = event.customType == kActionNextScaleFilter ? 1 : -1;

			uint scalerIndex = getScaler();
			switch (direction) {
			case 1:
				if (scalerIndex >= _scalerPlugins.size() - 1) {
					scalerIndex = 0;
				} else {
					++scalerIndex;
				}
				break;

			case -1:
			default:
				if (scalerIndex == 0) {
					scalerIndex = _scalerPlugins.size() - 1;
				} else {
					--scalerIndex;
				}
				break;
			}

			beginGFXTransaction();
			setScaler(scalerIndex, getScaleFactor());
			endGFXTransaction();

#ifdef USE_OSD
			int windowWidth = 0, windowHeight = 0;
			getWindowSizeFromSdl(&windowWidth, &windowHeight);
			const char *newScalerName = _scalerPlugins[scalerIndex]->get<ScalerPluginObject>().getPrettyName();
			if (newScalerName) {
				const Common::U32String message = Common::U32String::format(
					"%S %s%d\n%d x %d",
					_("Active graphics filter:").c_str(),
					newScalerName,
					getScaleFactor(),
					windowWidth, windowHeight);
				displayMessageOnOSD(message);
			}
#endif
		}

		return true;
	}
#endif // USE_SCALERS

	case kActionToggleAspectRatioCorrection:
		// Toggles the aspect ratio correction state.
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !getFeatureState(OSystem::kFeatureAspectRatioCorrection));
		endGFXTransaction();

#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureAspectRatioCorrection))
			displayMessageOnOSD(_("Enabled aspect ratio correction"));
		else
			displayMessageOnOSD(_("Disabled aspect ratio correction"));
#endif

		return true;

	case kActionToggleFilteredScaling:
		// Ctrl+Alt+f toggles filtering on/off
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureFilteringMode, !getFeatureState(OSystem::kFeatureFilteringMode));
		endGFXTransaction();

#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureFilteringMode)) {
			displayMessageOnOSD(_("Filtering enabled"));
		} else {
			displayMessageOnOSD(_("Filtering disabled"));
		}
#endif

		return true;

	case kActionCycleStretchMode: {
		// Ctrl+Alt+s cycles through stretch mode
		int index = 0;
		const OSystem::GraphicsMode *stretchModes = getSupportedStretchModes();
		const OSystem::GraphicsMode *sm = stretchModes;
		while (sm->name) {
			if (sm->id == getStretchMode())
				break;
			sm++;
			index++;
		}
		index++;
		if (!stretchModes[index].name)
			index = 0;
		beginGFXTransaction();
		setStretchMode(stretchModes[index].id);
		endGFXTransaction();

#ifdef USE_OSD
		Common::U32String message = Common::U32String::format("%S: %S",
			_("Stretch mode").c_str(),
			_(stretchModes[index].description).c_str()
		);
		displayMessageOnOSD(message);
#endif

		return true;
	}

	default:
		return SdlGraphicsManager::notifyEvent(event);
	}
}

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
void *OpenGLSdlGraphicsManager::getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) {
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
	Graphics::Surface *s = image.convertTo(OpenGL::Texture::getRGBPixelFormat(), palette, palCount);
	glPixelStorei(GL_UNPACK_ALIGNMENT, s->format.bytesPerPixel);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB, GL_UNSIGNED_BYTE, s->getPixels());
	s->free();
	delete s;
	return (void *)(intptr_t)image_texture;
}

void OpenGLSdlGraphicsManager::freeImGuiTexture(void *texture) {
	GLuint textureID = (intptr_t)texture;
	glDeleteTextures(1, &textureID);
}
#endif
