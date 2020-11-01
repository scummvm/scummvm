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

#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#include "backends/graphics/opengl/texture.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "graphics/scaler/aspect.h"

#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

OpenGLSdlGraphicsManager::OpenGLSdlGraphicsManager(SdlEventSource *eventSource, SdlWindow *window)
    : SdlGraphicsManager(eventSource, window), _lastRequestedHeight(0),
#if SDL_VERSION_ATLEAST(2, 0, 0)
      _glContext(),
#else
      _lastVideoModeLoad(0),
#endif
      _graphicsScale(2), _ignoreLoadVideoMode(false), _gotResize(false), _wantsFullScreen(false), _ignoreResizeEvents(0),
      _desiredFullscreenWidth(0), _desiredFullscreenHeight(0) {
	// Setup OpenGL attributes for SDL
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Set up proper SDL OpenGL context creation.
#if SDL_VERSION_ATLEAST(2, 0, 0)
	OpenGL::ContextType glContextType;

	// Context version 1.4 is choosen arbitrarily based on what most shader
	// extensions were written against.
	enum {
		DEFAULT_GL_MAJOR = 1,
		DEFAULT_GL_MINOR = 4,

		DEFAULT_GLES_MAJOR = 1,
		DEFAULT_GLES_MINOR = 1,

		DEFAULT_GLES2_MAJOR = 2,
		DEFAULT_GLES2_MINOR = 0
	};

#if USE_FORCED_GL
	glContextType = OpenGL::kContextGL;
	_glContextProfileMask = 0;
	_glContextMajor = DEFAULT_GL_MAJOR;
	_glContextMinor = DEFAULT_GL_MINOR;
#elif USE_FORCED_GLES
	glContextType = OpenGL::kContextGLES;
	_glContextProfileMask = SDL_GL_CONTEXT_PROFILE_ES;
	_glContextMajor = DEFAULT_GLES_MAJOR;
	_glContextMinor = DEFAULT_GLES_MINOR;
#elif USE_FORCED_GLES2
	glContextType = OpenGL::kContextGLES2;
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
			glContextType = OpenGL::kContextGLES2;
		} else {
			glContextType = OpenGL::kContextGLES;
		}
	} else if (_glContextProfileMask == SDL_GL_CONTEXT_PROFILE_CORE) {
		glContextType = OpenGL::kContextGL;

		// Core profile does not allow legacy functionality, which we use.
		// Thus we request a standard OpenGL context.
		_glContextProfileMask = 0;
		_glContextMajor = DEFAULT_GL_MAJOR;
		_glContextMinor = DEFAULT_GL_MINOR;
	} else {
		glContextType = OpenGL::kContextGL;
	}
#endif

	setContextType(glContextType);
#else
	setContextType(OpenGL::kContextGL);
#endif

	// Retrieve a list of working fullscreen modes
#if SDL_VERSION_ATLEAST(2, 0, 0)
	const int numModes = SDL_GetNumDisplayModes(0);
	for (int i = 0; i < numModes; ++i) {
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(0, i, &mode)) {
			continue;
		}

		_fullscreenVideoModes.push_back(VideoMode(mode.w, mode.h));
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

	Common::Rect desktopRes = _window->getDesktopResolution();

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
#if SDL_VERSION_ATLEAST(2, 0, 0)
	notifyContextDestroy();
	SDL_GL_DeleteContext(_glContext);
#endif
}

bool OpenGLSdlGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
	case OSystem::kFeatureIconifyWindow:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	case OSystem::kFeatureFullscreenToggleKeepsContext:
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
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_window && _window->getSDLWindow()) {
			return (SDL_GetWindowFlags(_window->getSDLWindow()) & SDL_WINDOW_FULLSCREEN) != 0;
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

	default:
		return OpenGLGraphicsManager::getFeatureState(f);
	}
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

	return OpenGLGraphicsManager::initSize(w, h, format);
}

void OpenGLSdlGraphicsManager::updateScreen() {
	if (_ignoreResizeEvents) {
		--_ignoreResizeEvents;
	}

	OpenGLGraphicsManager::updateScreen();
}

void OpenGLSdlGraphicsManager::notifyVideoExpose() {
}

void OpenGLSdlGraphicsManager::notifyResize(const int width, const int height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// We sometime get outdated resize events from SDL2. So check that the size we get
	// is the actual current window size. If not ignore the resize.
	// The issue for example occurs when switching from fullscreen to windowed mode or
	// when switching between different fullscreen resolutions because SDL_DestroyWindow
	// for a fullscreen window that doesn't have the SDL_WINDOW_FULLSCREEN_DESKTOP flag
	// causes a SDL_WINDOWEVENT_RESIZED event with the old resolution to be sent, and this
	// event is processed after recreating the window at the new resolution.
	int currentWidth, currentHeight;
	getWindowSizeFromSdl(&currentWidth, &currentHeight);
	if (width != currentWidth || height != currentHeight)
		return;
	// TODO: Implement high DPI support
	handleResize(width, height, 90, 90);
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

bool OpenGLSdlGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	// In some cases we might not want to load the requested video mode. This
	// will assure that the window size is not altered.
	if (_ignoreLoadVideoMode) {
		_ignoreLoadVideoMode = false;
		return true;
	}

	// This function should never be called from notifyResize thus we know
	// that the requested size came from somewhere else.
	_gotResize = false;

	// Save the requested dimensions.
	_lastRequestedWidth  = requestedWidth;
	_lastRequestedHeight = requestedHeight;

	// Apply the currently saved scale setting.
	requestedWidth  *= _graphicsScale;
	requestedHeight *= _graphicsScale;

	// Set up the mode.
	return setupMode(requestedWidth, requestedHeight);
}

void OpenGLSdlGraphicsManager::refreshScreen() {
	// Swap OpenGL buffers
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow(_window->getSDLWindow());
#else
	SDL_GL_SwapBuffers();
#endif
}

void *OpenGLSdlGraphicsManager::getProcAddress(const char *name) const {
	return SDL_GL_GetProcAddress(name);
}

void OpenGLSdlGraphicsManager::handleResizeImpl(const int width, const int height, const int xdpi, const int ydpi) {
	OpenGLGraphicsManager::handleResizeImpl(width, height, xdpi, ydpi);
	SdlGraphicsManager::handleResizeImpl(width, height, xdpi, ydpi);
}

bool OpenGLSdlGraphicsManager::saveScreenshot(const Common::String &filename) const {
	return OpenGLGraphicsManager::saveScreenshot(filename);
}

bool OpenGLSdlGraphicsManager::setupMode(uint width, uint height) {
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

	// This is pretty confusing since RGBA8888 talks about the memory
	// layout here. This is a different logical layout depending on
	// whether we run on little endian or big endian. However, we can
	// only safely assume that RGBA8888 in memory layout is supported.
	// Thus, we chose this one.
	const Graphics::PixelFormat rgba8888 =
#ifdef SCUMM_LITTLE_ENDIAN
	                                       Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else
	                                       Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_glContext) {
		notifyContextDestroy();

		SDL_GL_DeleteContext(_glContext);
		_glContext = nullptr;
	}

	uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
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

		flags |= SDL_WINDOW_FULLSCREEN;
	}

	// Request a OpenGL (ES) context we can use.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, _glContextProfileMask);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _glContextMajor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _glContextMinor);

	if (!createOrUpdateWindow(width, height, flags)) {
		return false;
	}

	_glContext = SDL_GL_CreateContext(_window->getSDLWindow());
	if (!_glContext) {
		return false;
	}

	notifyContextCreate(rgba8888, rgba8888);
	int actualWidth, actualHeight;
	getWindowSizeFromSdl(&actualWidth, &actualHeight);
	// TODO: Implement high DPI support
	handleResize(actualWidth, actualHeight, 90, 90);
	return true;
#else
	// WORKAROUND: Working around infamous SDL bugs when switching
	// resolutions too fast. This might cause the event system to supply
	// incorrect mouse position events otherwise.
	// Reference: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=665779
	const uint32 curTime = SDL_GetTicks();
	if (_hwScreen && (curTime < _lastVideoModeLoad || curTime - _lastVideoModeLoad < 100)) {
		for (int i = 10; i > 0; --i) {
			SDL_PumpEvents();
			SDL_Delay(10);
		}
	}

	uint32 flags = SDL_OPENGL;
	if (_wantsFullScreen) {
		width  = _desiredFullscreenWidth;
		height = _desiredFullscreenHeight;
		flags |= SDL_FULLSCREEN;
	} else {
		flags |= SDL_RESIZABLE;
	}

	if (_hwScreen) {
		// When a video mode has been setup already we notify the manager that
		// the context is about to be destroyed.
		// We do this because on Windows SDL_SetVideoMode can destroy and
		// recreate the OpenGL context.
		notifyContextDestroy();
	}

	_hwScreen = SDL_SetVideoMode(width, height, 32, flags);

	if (!_hwScreen) {
		// We treat fullscreen requests as a "hint" for now. This means in
		// case it is not available we simply ignore it.
		if (_wantsFullScreen) {
			_hwScreen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL | SDL_RESIZABLE);
		}
	}

	// Part of the WORKAROUND mentioned above.
	_lastVideoModeLoad = SDL_GetTicks();

	if (_hwScreen) {
		notifyContextCreate(rgba8888, rgba8888);
		handleResize(_hwScreen->w, _hwScreen->h, 90, 90);
	}

	// Ignore resize events (from SDL) for a few frames, if this isn't
	// caused by a notification from SDL. This avoids bad resizes to a
	// (former) resolution for which we haven't processed an event yet.
	if (!_gotResize)
		_ignoreResizeEvents = 10;

	return _hwScreen != nullptr;
#endif
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

			// In case no modes are available we do nothing.
			if (_fullscreenVideoModes.empty()) {
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
			_graphicsScale = MAX<int>(windowWidth / _lastRequestedWidth, windowHeight / _lastRequestedHeight);
			_graphicsScale = MAX<int>(_graphicsScale + direction, 1);

			// Since we overwrite a user resize here we reset its
			// flag here. This makes enabling AR smoother because it
			// will change the window size like in surface SDL.
			_gotResize = false;

			// Try to setup the mode.
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

	case kActionToggleAspectRatioCorrection:
		// In case the user changed the window size manually we will
		// not change the window size again here.
		_ignoreLoadVideoMode = _gotResize;

		// Toggles the aspect ratio correction state.
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !getFeatureState(OSystem::kFeatureAspectRatioCorrection));
		endGFXTransaction();

		// Make sure we do not ignore the next resize. This
		// effectively checks whether loadVideoMode has been called.
		assert(!_ignoreLoadVideoMode);

#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureAspectRatioCorrection))
			displayMessageOnOSD(_("Enabled aspect ratio correction"));
		else
			displayMessageOnOSD(_("Disabled aspect ratio correction"));
#endif

		return true;

	case kActionToggleFilteredScaling:
		// Never ever try to resize the window when we simply want to enable or disable filtering.
		// This assures that the window size does not change.
		_ignoreLoadVideoMode = true;

		// Ctrl+Alt+f toggles filtering on/off
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureFilteringMode, !getFeatureState(OSystem::kFeatureFilteringMode));
		endGFXTransaction();

		// Make sure we do not ignore the next resize. This
		// effectively checks whether loadVideoMode has been called.
		assert(!_ignoreLoadVideoMode);

#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureFilteringMode)) {
			displayMessageOnOSD(_("Filtering enabled"));
		} else {
			displayMessageOnOSD(_("Filtering disabled"));
		}
#endif

		return true;

	case kActionCycleStretchMode: {
		// Never try to resize the window when changing the scaling mode.
		_ignoreLoadVideoMode = true;

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
