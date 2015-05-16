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

#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

OpenGLSdlGraphicsManager::OpenGLSdlGraphicsManager(uint desktopWidth, uint desktopHeight, SdlEventSource *eventSource, SdlWindow *window)
    : SdlGraphicsManager(eventSource, window), _lastRequestedHeight(0),
#if SDL_VERSION_ATLEAST(2, 0, 0)
      _glContext(),
#else
      _lastVideoModeLoad(0), _hwScreen(nullptr),
#endif
      _graphicsScale(2), _ignoreLoadVideoMode(false), _gotResize(false), _wantsFullScreen(false), _ignoreResizeEvents(0),
      _desiredFullscreenWidth(0), _desiredFullscreenHeight(0) {
	// Setup OpenGL attributes for SDL
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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

	// In case SDL is fine with every mode we will force the desktop mode.
	// TODO? We could also try to add some default resolutions here.
	if (_fullscreenVideoModes.empty() && desktopWidth && desktopHeight) {
		_fullscreenVideoModes.push_back(VideoMode(desktopWidth, desktopHeight));
	}

	// Get information about display sizes from the previous runs.
	if (ConfMan.hasKey("last_fullscreen_mode_width", Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("last_fullscreen_mode_height", Common::ConfigManager::kApplicationDomain)) {
		_desiredFullscreenWidth  = ConfMan.getInt("last_fullscreen_mode_width", Common::ConfigManager::kApplicationDomain);
		_desiredFullscreenHeight = ConfMan.getInt("last_fullscreen_mode_height", Common::ConfigManager::kApplicationDomain);
	} else {
		// Use the desktop resolutions when no previous default has been setup.
		_desiredFullscreenWidth  = desktopWidth;
		_desiredFullscreenHeight = desktopHeight;
	}
}

OpenGLSdlGraphicsManager::~OpenGLSdlGraphicsManager() {
}

void OpenGLSdlGraphicsManager::activateManager() {
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void OpenGLSdlGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

bool OpenGLSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
	case OSystem::kFeatureIconifyWindow:
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

bool OpenGLSdlGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_window) {
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

bool OpenGLSdlGraphicsManager::setGraphicsMode(int mode) {
	// HACK: This is stupid but the SurfaceSDL backend defaults to 2x. This
	// assures that the launcher (which requests 320x200) has a reasonable
	// size. It also makes small games have a reasonable size (i.e. at least
	// 640x400). We follow the same logic here until we have a better way to
	// give hints to our backend for that.
	_graphicsScale = 2;

	return OpenGLGraphicsManager::setGraphicsMode(mode);
}

void OpenGLSdlGraphicsManager::resetGraphicsScale() {
	OpenGLGraphicsManager::resetGraphicsScale();

	// HACK: See OpenGLSdlGraphicsManager::setGraphicsMode.
	_graphicsScale = 1;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> OpenGLSdlGraphicsManager::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> formats;

	// Our default mode is (memory layout wise) RGBA8888 which is a different
	// logical layout depending on the endianness. We chose this mode because
	// it is the only 32bit color mode we can safely assume to be present in
	// OpenGL and OpenGL ES implementations. Thus, we need to supply different
	// logical formats based on endianness.
#ifdef SCUMM_LITTLE_ENDIAN
	// ABGR8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#else
	// RGBA8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#endif
	// RGB565
	formats.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	// RGBA5551
	formats.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	// RGBA4444
	formats.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));

#ifndef USE_GLES
#ifdef SCUMM_LITTLE_ENDIAN
	// RGBA8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	// ABGR8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
	// ARGB8888, this should not be here, but Sword25 requires it. :-/
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));

	// RGB555, this is used by SCUMM HE 16 bit games.
	formats.push_back(Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));
#endif

	formats.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return formats;
}
#endif

void OpenGLSdlGraphicsManager::updateScreen() {
	if (_ignoreResizeEvents) {
		--_ignoreResizeEvents;
	}

	OpenGLGraphicsManager::updateScreen();

	// Swap OpenGL buffers
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SwapWindow(_window->getSDLWindow());
#else
	SDL_GL_SwapBuffers();
#endif
}

void OpenGLSdlGraphicsManager::notifyVideoExpose() {
}

void OpenGLSdlGraphicsManager::notifyResize(const uint width, const uint height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	setActualScreenSize(width, height);
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

void OpenGLSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void OpenGLSdlGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

void OpenGLSdlGraphicsManager::setInternalMousePosition(int x, int y) {
	_window->warpMouseInWindow(x, y);
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

		// Use our choice.
		width  = _desiredFullscreenWidth;
		height = _desiredFullscreenHeight;
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

	_window->destroyWindow();

	uint32 flags = SDL_WINDOW_OPENGL;
	if (_wantsFullScreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	} else {
		flags |= SDL_WINDOW_RESIZABLE;
	}

	if (!_window->createWindow(width, height, flags)) {
		// We treat fullscreen requests as a "hint" for now. This means in
		// case it is not available we simply ignore it.
		if (_wantsFullScreen) {
			_window->createWindow(width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}

		if (!_window->getSDLWindow()) {
			return false;
		}
	}

	_glContext = SDL_GL_CreateContext(_window->getSDLWindow());
	if (!_glContext) {
		return false;
	}

	notifyContextCreate(rgba8888, rgba8888);
	int actualWidth, actualHeight;
	getWindowDimensions(&actualWidth, &actualHeight);
	setActualScreenSize(actualWidth, actualHeight);
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
		setActualScreenSize(_hwScreen->w, _hwScreen->h);
	}

	// Ignore resize events (from SDL) for a few frames, if this isn't
	// caused by a notification from SDL. This avoids bad resizes to a
	// (former) resolution for which we haven't processed an event yet.
	if (!_gotResize)
		_ignoreResizeEvents = 10;

	return _hwScreen != nullptr;
#endif
}

void OpenGLSdlGraphicsManager::getWindowDimensions(int *width, int *height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GetWindowSize(_window->getSDLWindow(), width, height);
#else
	if (width) {
		*width = _hwScreen->w;
	}

	if (height) {
		*height = _hwScreen->h;
	}
#endif
}

bool OpenGLSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYUP:
		return isHotkey(event);

	case Common::EVENT_KEYDOWN:
		if (event.kbd.hasFlags(Common::KBD_ALT)) {
			if (   event.kbd.keycode == Common::KEYCODE_RETURN
			    || event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER) {
				// Alt-Return and Alt-Enter toggle full screen mode
				beginGFXTransaction();
					setFeatureState(OSystem::kFeatureFullscreenMode, !getFeatureState(OSystem::kFeatureFullscreenMode));
				endGFXTransaction();

#ifdef USE_OSD
				if (getFeatureState(OSystem::kFeatureFullscreenMode)) {
					displayMessageOnOSD("Fullscreen mode");
				} else {
					displayMessageOnOSD("Windowed mode");
				}
#endif
				return true;
			}

			if (event.kbd.keycode == Common::KEYCODE_s) {
				// Alt-s creates a screenshot
				Common::String filename;

				for (int n = 0;; n++) {
					SDL_RWops *file;

					filename = Common::String::format("scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename.c_str(), "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}

				saveScreenshot(filename.c_str());
				debug("Saved screenshot '%s'", filename.c_str());

				return true;
			}
		} else if (event.kbd.hasFlags(Common::KBD_CTRL | Common::KBD_ALT)) {
			if (   event.kbd.keycode == Common::KEYCODE_PLUS || event.kbd.keycode == Common::KEYCODE_MINUS
			    || event.kbd.keycode == Common::KEYCODE_KP_PLUS || event.kbd.keycode == Common::KEYCODE_KP_MINUS) {
				// Ctrl+Alt+Plus/Minus Increase/decrease the size
				const int direction = (event.kbd.keycode == Common::KEYCODE_PLUS || event.kbd.keycode == Common::KEYCODE_KP_PLUS) ? +1 : -1;

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
					getWindowDimensions(&windowWidth, &windowHeight);
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
				getWindowDimensions(&windowWidth, &windowHeight);
				const Common::String osdMsg = Common::String::format("Resolution: %dx%d", windowWidth, windowHeight);
				displayMessageOnOSD(osdMsg.c_str());
#endif

				return true;
			} else if (event.kbd.keycode == Common::KEYCODE_a) {
				// In case the user changed the window size manually we will
				// not change the window size again here.
				_ignoreLoadVideoMode = _gotResize;

				// Ctrl+Alt+a toggles the aspect ratio correction state.
				beginGFXTransaction();
					setFeatureState(OSystem::kFeatureAspectRatioCorrection, !getFeatureState(OSystem::kFeatureAspectRatioCorrection));
				endGFXTransaction();

				// Make sure we do not ignore the next resize. This
				// effectively checks whether loadVideoMode has been called.
				assert(!_ignoreLoadVideoMode);

#ifdef USE_OSD
				Common::String osdMsg = "Aspect ratio correction: ";
				osdMsg += getFeatureState(OSystem::kFeatureAspectRatioCorrection) ? "enabled" : "disabled";
				displayMessageOnOSD(osdMsg.c_str());
#endif

				return true;
			} else if (event.kbd.keycode == Common::KEYCODE_f) {
				// Ctrl+Alt+f toggles the graphics modes.

				// We are crazy we will allow the OpenGL base class to
				// introduce new graphics modes like shaders for special
				// filtering. If some other OpenGL subclass needs this,
				// we can think of refactoring this.
				int mode = getGraphicsMode();
				const OSystem::GraphicsMode *supportedModes = getSupportedGraphicsModes();
				const OSystem::GraphicsMode *modeDesc = nullptr;

				// Search the current mode.
				for (; supportedModes->name; ++supportedModes) {
					if (supportedModes->id == mode) {
						modeDesc = supportedModes;
						break;
					}
				}
				assert(modeDesc);

				// Try to use the next mode in the list.
				++modeDesc;
				if (!modeDesc->name) {
					modeDesc = getSupportedGraphicsModes();
				}

				// Never ever try to resize the window when we simply want to
				// switch the graphics mode. This assures that the window size
				// does not change.
				_ignoreLoadVideoMode = true;

				beginGFXTransaction();
					setGraphicsMode(modeDesc->id);
				endGFXTransaction();

				// Make sure we do not ignore the next resize. This
				// effectively checks whether loadVideoMode has been called.
				assert(!_ignoreLoadVideoMode);

#ifdef USE_OSD
				const Common::String osdMsg = Common::String::format("Graphics mode: %s", _(modeDesc->description));
				displayMessageOnOSD(osdMsg.c_str());
#endif

				return true;
			}
		}
		// Fall through

	default:
		return false;
	}
}

bool OpenGLSdlGraphicsManager::isHotkey(const Common::Event &event) {
	if (event.kbd.hasFlags(Common::KBD_ALT)) {
		return    event.kbd.keycode == Common::KEYCODE_RETURN
		       || event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER
		       || event.kbd.keycode == Common::KEYCODE_s;
	} else if (event.kbd.hasFlags(Common::KBD_CTRL | Common::KBD_ALT)) {
		return    event.kbd.keycode == Common::KEYCODE_PLUS || event.kbd.keycode == Common::KEYCODE_MINUS
		       || event.kbd.keycode == Common::KEYCODE_KP_PLUS || event.kbd.keycode == Common::KEYCODE_KP_MINUS
		       || event.kbd.keycode == Common::KEYCODE_a
		       || event.kbd.keycode == Common::KEYCODE_f;
	}

	return false;
}
