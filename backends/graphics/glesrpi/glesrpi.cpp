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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "backends/graphics/glesrpi/glesrpi.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

#include <bcm_host.h>

static struct {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
	uint32_t width, height, refresh;
} eglInfo;

static DISPMANX_RESOURCE_HANDLE_T   resource_bg;

DISPMANX_ELEMENT_HANDLE_T dispman_element;
DISPMANX_ELEMENT_HANDLE_T dispman_element_bg;
DISPMANX_DISPLAY_HANDLE_T dispman_display;
DISPMANX_UPDATE_HANDLE_T dispman_update;
static EGL_DISPMANX_WINDOW_T nativewindow;

void OpenGLRPIGraphicsManager::init_egl(){
		
	bcm_host_init();
	
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
    	
	// get an EGL display connection
	eglInfo.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(eglInfo.display != EGL_NO_DISPLAY);
    
	// initialize the EGL display connection
	EGLBoolean result = eglInitialize(eglInfo.display, NULL, NULL);
	assert(EGL_FALSE != result);
    
	// get an appropriate EGL frame buffer configuration
	EGLint num_config;

	static const EGLint attribute_list[] =
	{
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_NONE
	};
	
	result = eglChooseConfig(eglInfo.display, attribute_list, &eglInfo.config, 1, &num_config);
	assert(EGL_FALSE != result);
    
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
    
	// create an EGL rendering context
	static const EGLint context_attributes[] =
	{
	    EGL_CONTEXT_CLIENT_VERSION, 1,
	    EGL_NONE
	};
	eglInfo.context = eglCreateContext(eglInfo.display, eglInfo.config, EGL_NO_CONTEXT, context_attributes);
	assert(eglInfo.context != EGL_NO_CONTEXT);
    
	// create an EGL window surface
	int32_t success = graphics_get_display_size(0, &eglInfo.width, &eglInfo.height);
	assert(success >= 0);

	vc_dispmanx_rect_set( &dst_rect, 0, 0, eglInfo.width, eglInfo.height);
    
	vc_dispmanx_rect_set( &src_rect, 0, 0, eglInfo.width << 16, eglInfo.height << 16);
    
	dispman_display = vc_dispmanx_display_open(0);
	dispman_update = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
                                              10, &dst_rect, 0, &src_rect,
                                              DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);
    
	//Black background surface dimensions
	vc_dispmanx_rect_set( &dst_rect, 0, 0, eglInfo.width, eglInfo.height );
	vc_dispmanx_rect_set( &src_rect, 0, 0, 128 << 16, 128 << 16);
    
	//Create a blank background for the whole screen, make sure width is divisible by 32!
	uint32_t crap;
	resource_bg = vc_dispmanx_resource_create(VC_IMAGE_RGB565, 128, 128, &crap);
	dispman_element_bg = vc_dispmanx_element_add(  dispman_update, dispman_display,
                                                 9, &dst_rect, resource_bg, &src_rect,
                                                 DISPMANX_PROTECTION_NONE, 0, 0,
                                                 (DISPMANX_TRANSFORM_T) 0 );

	nativewindow.element = dispman_element;
	nativewindow.width = eglInfo.width;
	nativewindow.height = eglInfo.height;
    
	vc_dispmanx_update_submit_sync(dispman_update);

	//Dispmanx window ready, let's go for EGL.
	eglInfo.surface = eglCreateWindowSurface(eglInfo.display, eglInfo.config, &nativewindow, NULL);
	assert(eglInfo.surface != EGL_NO_SURFACE);
    
	// connect the context to the surface
	result = eglMakeCurrent(eglInfo.display, eglInfo.surface, eglInfo.surface, eglInfo.context);
	assert(EGL_FALSE != result);

	eglSwapInterval (eglInfo.display, 1);
}

void OpenGLRPIGraphicsManager::deinit_egl(){
    // Release context resources
    eglMakeCurrent( eglInfo.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( eglInfo.display, eglInfo.surface );
    eglDestroyContext( eglInfo.display, eglInfo.context );
    eglTerminate( eglInfo.display );
    
    dispman_update = vc_dispmanx_update_start( 0 );
    vc_dispmanx_element_remove( dispman_update, dispman_element );
    vc_dispmanx_element_remove( dispman_update, dispman_element_bg );
    vc_dispmanx_update_submit_sync( dispman_update );
    vc_dispmanx_resource_delete( resource_bg );
    vc_dispmanx_display_close( dispman_display );
}


OpenGLRPIGraphicsManager::OpenGLRPIGraphicsManager(SdlEventSource *eventSource)
    : SdlGraphicsManager(eventSource), _lastVideoModeLoad(0), _hwScreen(nullptr), _lastRequestedWidth(0), _lastRequestedHeight(0),
      _graphicsScale(0), _ignoreLoadVideoMode(false), _gotResize(false), _wantsFullScreen(false), _ignoreResizeEvents(0),
      _desiredFullscreenWidth(0), _desiredFullscreenHeight(0) {

	init_egl();	
}

OpenGLRPIGraphicsManager::~OpenGLRPIGraphicsManager() {
}

void OpenGLRPIGraphicsManager::activateManager() {
	//MAC We activate SDL manager here...
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void OpenGLRPIGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

bool OpenGLRPIGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return true;
	case OSystem::kFeatureIconifyWindow:
		return false;

	default:
		return OpenGLGraphicsManager::hasFeature(f);
	}
}

void OpenGLRPIGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		assert(getTransactionMode() != kTransactionNone);
		_wantsFullScreen = enable;
		break;

	default:
		OpenGLGraphicsManager::setFeatureState(f, enable);
	}
}

bool OpenGLRPIGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
			return _wantsFullScreen;
	default:
		return OpenGLGraphicsManager::getFeatureState(f);
	}
}

bool OpenGLRPIGraphicsManager::setGraphicsMode(int mode) {
	// HACK: This is stupid but the SurfaceSDL backend defaults to 2x. This
	// assures that the launcher (which requests 320x200) has a reasonable
	// size. It also makes small games have a reasonable size (i.e. at least
	// 640x400). We follow the same logic here until we have a better way to
	// give hints to our backend for that.
	
	_graphicsScale = 1;

	return OpenGLGraphicsManager::setGraphicsMode(mode);
}

void OpenGLRPIGraphicsManager::resetGraphicsScale() {
	OpenGLGraphicsManager::resetGraphicsScale();

	_graphicsScale = 1;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> OpenGLRPIGraphicsManager::getSupportedFormats() const {
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

void OpenGLRPIGraphicsManager::updateScreen() {
	if (_ignoreResizeEvents) {
		--_ignoreResizeEvents;
	}

	OpenGLGraphicsManager::updateScreen();

	eglSwapBuffers(eglInfo.display, eglInfo.surface);
}

void OpenGLRPIGraphicsManager::notifyVideoExpose() {
}

void OpenGLRPIGraphicsManager::notifyResize(const uint width, const uint height) {
}

void OpenGLRPIGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void OpenGLRPIGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

void OpenGLRPIGraphicsManager::setInternalMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool OpenGLRPIGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
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

bool OpenGLRPIGraphicsManager::setupMode(uint width, uint height) {
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

	if (_hwScreen) {
		// When a video mode has been setup already we notify the manager that
		// the context is about to be destroyed.
		// We do this because on Windows SDL_SetVideoMode can destroy and
		// recreate the OpenGL context.
		notifyContextDestroy();
	}

	_hwScreen = SDL_SetVideoMode(eglInfo.width, eglInfo.height, 32, 0);
	SDL_WarpMouse (eglInfo.width / 2, eglInfo.height / 2);
		
	// Part of the WORKAROUND mentioned above.
	_lastVideoModeLoad = SDL_GetTicks();

	if (_hwScreen) {
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
		notifyContextCreate(rgba8888, rgba8888);
		setActualScreenSize(_hwScreen->w, _hwScreen->h);
	}

	// Ignore resize events (from SDL) for a few frames, if this isn't
	// caused by a notification from SDL. This avoids bad resizes to a
	// (former) resolution for which we haven't processed an event yet.
	if (!_gotResize)
		_ignoreResizeEvents = 10;

	return _hwScreen != nullptr;
}

bool OpenGLRPIGraphicsManager::notifyEvent(const Common::Event &event) {
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
						warning("OpenGLRPIGraphicsManager::notifyEvent: Fullscreen resize failed ('%s')", SDL_GetError());
						g_system->quit();
					}
				} else {
					// Calculate the next scaling setting. We approximate the
					// current scale setting in case the user resized the
					// window. Then we apply the direction change.
					_graphicsScale = MAX<int>(_hwScreen->w / _lastRequestedWidth, _hwScreen->h / _lastRequestedHeight);
					_graphicsScale = MAX<int>(_graphicsScale + direction, 1);

					// Since we overwrite a user resize here we reset its
					// flag here. This makes enabling AR smoother because it
					// will change the window size like in surface SDL.
					_gotResize = false;

					// Try to setup the mode.
					if (!setupMode(_lastRequestedWidth * _graphicsScale, _lastRequestedHeight * _graphicsScale)) {
						warning("OpenGLRPIGraphicsManager::notifyEvent: Window resize failed ('%s')", SDL_GetError());
						g_system->quit();
					}
				}

#ifdef USE_OSD
				const Common::String osdMsg = Common::String::format("Resolution: %dx%d", _hwScreen->w, _hwScreen->h);
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

bool OpenGLRPIGraphicsManager::isHotkey(const Common::Event &event) {
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
