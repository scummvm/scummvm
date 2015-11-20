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

// Needed so we don't get errors when including the Raspberry Pi headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "backends/graphics/egl-raspberrypi/egl-raspberrypi.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

#ifdef USE_EGL_RASPBERRYPI
#include <bcm_host.h>
#endif

struct dispvarsStruct {
	DISPMANX_ELEMENT_HANDLE_T element;
	DISPMANX_DISPLAY_HANDLE_T display;
	DISPMANX_UPDATE_HANDLE_T update;
	EGL_DISPMANX_WINDOW_T nativeWindow;
};

void EGLRaspberryPiGraphicsManager::initEGL() {
	static const EGLint attributeList[] = {
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
	    EGL_ALPHA_SIZE, 8,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_NONE
	};
	
	// create an EGL rendering context
	static const EGLint contextAttributes[] = {
	    EGL_CONTEXT_CLIENT_VERSION, 1,
	    EGL_NONE
	};

	EGLint numConfig;

	bcm_host_init();
	VC_RECT_T dstRect;
	VC_RECT_T srcRect;
	
	// create a Dispmanx EGL window surface
	int32_t success = graphics_get_display_size(0, &_eglInfo.width, &_eglInfo.height);
	assert(success >= 0);

	vc_dispmanx_rect_set(&dstRect, 0, 0, _eglInfo.width, _eglInfo.height);
    
	vc_dispmanx_rect_set(&srcRect, 0, 0, _eglInfo.width << 16, _eglInfo.height << 16);
    
	_dispvars->display = vc_dispmanx_display_open(0);
	_dispvars->update = vc_dispmanx_update_start(0);
	_dispvars->element = vc_dispmanx_element_add(_dispvars->update, _dispvars->display,
                                              10, &dstRect, 0, &srcRect,
                                              DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);
    
	_dispvars->nativeWindow.element = _dispvars->element;
	_dispvars->nativeWindow.width = _eglInfo.width;
	_dispvars->nativeWindow.height = _eglInfo.height;
    
	vc_dispmanx_update_submit_sync(_dispvars->update);
	
	_eglInfo.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(_eglInfo.display != EGL_NO_DISPLAY);

	// initialize the EGL display connection
	EGLBoolean result = eglInitialize(_eglInfo.display, NULL, NULL);
	assert(EGL_FALSE != result);
    
	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(_eglInfo.display, attributeList, &_eglInfo.config, 1, &numConfig);
	assert(EGL_FALSE != result);
    
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	
	_eglInfo.context = eglCreateContext(_eglInfo.display, _eglInfo.config, EGL_NO_CONTEXT, contextAttributes);
	assert(_eglInfo.context != EGL_NO_CONTEXT);

	_eglInfo.surface = eglCreateWindowSurface(_eglInfo.display, _eglInfo.config, &_dispvars->nativeWindow, NULL);
	assert(_eglInfo.surface != EGL_NO_SURFACE);

	// connect the context to the surface
	result = eglMakeCurrent(_eglInfo.display, _eglInfo.surface, _eglInfo.surface, _eglInfo.context);
	assert(EGL_FALSE != result);

	eglSwapInterval(_eglInfo.display, 1);
}

void EGLRaspberryPiGraphicsManager::deinitEGL() {
	// Release context resources
	eglMakeCurrent(_eglInfo.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(_eglInfo.display, _eglInfo.surface);
	eglDestroyContext(_eglInfo.display, _eglInfo.context);
	eglTerminate(_eglInfo.display);

	_dispvars->update = vc_dispmanx_update_start(0);
	vc_dispmanx_element_remove(_dispvars->update, _dispvars->element);
	vc_dispmanx_update_submit_sync(_dispvars->update);
	vc_dispmanx_display_close(_dispvars->display);
}

EGLRaspberryPiGraphicsManager::EGLRaspberryPiGraphicsManager(SdlEventSource *eventSource)
	: SdlGraphicsManager(eventSource), _hwScreen(nullptr) {
	_dispvars = new dispvarsStruct;	
	initEGL();	
}

EGLRaspberryPiGraphicsManager::~EGLRaspberryPiGraphicsManager() {
	deinitEGL();
}

void EGLRaspberryPiGraphicsManager::activateManager() {
	// We activate SDL manager here
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void EGLRaspberryPiGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

bool EGLRaspberryPiGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return true;
	case OSystem::kFeatureIconifyWindow:
		return false;

	default:
		return OpenGLGraphicsManager::hasFeature(f);
	}
}

void EGLRaspberryPiGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	OpenGLGraphicsManager::setFeatureState(f, enable);
}

bool EGLRaspberryPiGraphicsManager::getFeatureState(OSystem::Feature f) {
	return OpenGLGraphicsManager::getFeatureState(f);
}

bool EGLRaspberryPiGraphicsManager::setGraphicsMode(int mode) {
	return OpenGLGraphicsManager::setGraphicsMode(mode);
}

void EGLRaspberryPiGraphicsManager::resetGraphicsScale() {
	OpenGLGraphicsManager::resetGraphicsScale();
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> EGLRaspberryPiGraphicsManager::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> formats;

	// Our default mode is (memory layout wise) RGBA8888 which is a different
	// logical layout depending on the endianness. We chose this mode because
	// it is the only 32bit color mode we can safely assume to be present in
	// OpenGL and OpenGL ES implementations. Thus, we need to supply different
	// logical formats based on endianness.
	
	// ABGR8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	// RGB565
	formats.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	// RGBA5551
	formats.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	// RGBA4444
	formats.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));

	formats.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return formats;
}
#endif

void EGLRaspberryPiGraphicsManager::updateScreen() {
	OpenGLGraphicsManager::updateScreen();
	eglSwapBuffers(_eglInfo.display, _eglInfo.surface);
}

void EGLRaspberryPiGraphicsManager::notifyVideoExpose() {
}

void EGLRaspberryPiGraphicsManager::notifyResize(const uint width, const uint height) {
}

void EGLRaspberryPiGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void EGLRaspberryPiGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

void EGLRaspberryPiGraphicsManager::setInternalMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool EGLRaspberryPiGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	// Set up the mode.
	return setupMode(requestedWidth, requestedHeight);
}

bool EGLRaspberryPiGraphicsManager::setupMode(uint width, uint height) {
	if (_hwScreen) {
		//If we have a GLES context already, we have what we need and there's no need go any further
		return(true);
	}

	_hwScreen = SDL_SetVideoMode(0, 0, 16, 0);
	
	if (_hwScreen) {
		// This is pretty confusing since RGBA8888 talks about the memory
		// layout here. This is a different logical layout depending on
		// whether we run on little endian or big endian. However, we can
		// only safely assume that RGBA8888 in memory layout is supported.
		// Thus, we chose this one.
		
		const Graphics::PixelFormat rgba8888 = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		notifyContextCreate(rgba8888, rgba8888);
		setActualScreenSize(_hwScreen->w, _hwScreen->h);
	}

	return _hwScreen != nullptr;
}

bool EGLRaspberryPiGraphicsManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYUP:
		return isHotkey(event);

	case Common::EVENT_KEYDOWN:
		if (event.kbd.hasFlags(Common::KBD_ALT)) {
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
			}  else if (event.kbd.keycode == Common::KEYCODE_a) {
				// Ctrl+Alt+a toggles the aspect ratio correction state.
				beginGFXTransaction();
					setFeatureState(OSystem::kFeatureAspectRatioCorrection, !getFeatureState(OSystem::kFeatureAspectRatioCorrection));
				endGFXTransaction();

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

				beginGFXTransaction();
					setGraphicsMode(modeDesc->id);
				endGFXTransaction();

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

bool EGLRaspberryPiGraphicsManager::isHotkey(const Common::Event &event) {
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
