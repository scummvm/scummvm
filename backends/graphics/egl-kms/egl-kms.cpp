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

#include "backends/graphics/egl-kms/egl-kms.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

#ifdef USE_EGL_KMS
// This hacky define is needed since there is an struct member called "virtual" in xf86drm.h
#define virtual __virtual
#include <xf86drm.h>
#undef virtual
#include <xf86drmMode.h>
#include <gbm.h>
#include <fcntl.h>

drmEventContext eventContext;

void drmPageFlipHandler(int fd, uint frame, uint sec, uint usec, void *data) {
	int *waiting_for_flip = (int *)data;
	*waiting_for_flip = 0;
}

drmFBStruct *EGLKMSGraphicsManager::drmFBGetFromBO(gbm_bo *bob) {
	drmFBStruct *fbu = (drmFBStruct *)gbm_bo_get_user_data(bob);
	uint32_t width, height, stride, handle;

	if (fbu) {
		return fbu;
	}
	
	fbu = (drmFBStruct *)calloc(1, sizeof *fbu);
	fbu->bo = bob;

	width = gbm_bo_get_width(bob);
	height = gbm_bo_get_height(bob);
	stride = gbm_bo_get_stride(bob);
	handle = gbm_bo_get_handle(bob).u32;

	if (drmModeAddFB(_drm.fd, width, height, 24, 32, stride, handle, &fbu->fb_id)) {
		debug("Could not add drm framebuffer\n");
		free(fbu);
		return NULL;
	}

	// We used to pass the destroy callback function here. Now it's done manually in deinitEGL()
	gbm_bo_set_user_data(bob, fbu, NULL);
	return fbu;
}

void EGLKMSGraphicsManager::drmPageFlip(void) {
	int waiting_for_flip = 1;
	fd_set fds;

	gbm_bo *next_bo = gbm_surface_lock_front_buffer(_gbm.surface);
	_fb = drmFBGetFromBO(next_bo);

	if (drmModePageFlip(_drm.fd, _drm.crtc_id, _fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip)) {
		debug ("Failed to queue pageflip\n");
		return;
	}

	while (waiting_for_flip) {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(_drm.fd, &fds);
		select(_drm.fd+1, &fds, NULL, NULL, NULL);
		drmHandleEvent(_drm.fd, &eventContext);
	}
	
	// release last buffer to render on again
	gbm_surface_release_buffer(_gbm.surface, _bo);
	_bo = next_bo;
}

bool EGLKMSGraphicsManager::initDRM(void) {
	// In plain C, we can just init eventContext at declare time, but it's now allowed in C++
	eventContext.version = DRM_EVENT_CONTEXT_VERSION;
	eventContext.page_flip_handler = drmPageFlipHandler;

	drmModeConnector *connector;
	drmModeEncoder *encoder;
	uint i, area;

	_drm.fd = open("/dev/dri/card0", O_RDWR);

	if (_drm.fd < 0) {
		debug("could not open drm device\n");
		return false;
	}

	drmModeRes *resources = drmModeGetResources(_drm.fd);
	if (!resources) {
		debug("drmModeGetResources failed\n");
		return false;
	}

	// find a connected connector
	for (i = 0; i < (uint)resources->count_connectors; i++) {
		connector = drmModeGetConnector(_drm.fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED) {
			// it's connected, let's use this!
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if (!connector) {
		// we could be fancy and listen for hotplug events and wait for
		// a connector..
		debug("no connected connector found\n");
		return false;
	}
	// find highest resolution mode
	for (i = 0, area = 0; i < (uint)connector->count_modes; i++) {
		drmModeModeInfo *current_mode = &connector->modes[i];
		uint current_area = current_mode->hdisplay * current_mode->vdisplay;
		if (current_area > area) {
			_drm.mode = current_mode;
			area = current_area;
		}
	}

	if (!_drm.mode) {
		debug("could not find mode\n");
		return false;
	}

	// find encoder
	for (i = 0; i < (uint)resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(_drm.fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (!encoder) {
		debug("no encoder found\n");
		return false;
	}

	_drm.crtc_id = encoder->crtc_id;
	_drm.connector_id = connector->connector_id;

	return true;
}

bool EGLKMSGraphicsManager::initGBM() {
	_gbm.dev = gbm_create_device(_drm.fd);

	_gbm.surface = gbm_surface_create(_gbm.dev,
			_drm.mode->hdisplay, _drm.mode->vdisplay,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if (!_gbm.surface) {
		debug ("failed to create gbm surface\n");
		return -1;
	}
	
	return true;
}
#endif

void EGLKMSGraphicsManager::initEGL() {
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
	
	if (!initDRM()) {
		debug("failed to initialize DRM\n");
		return;
	}

	if (!initGBM()) {
		debug("failed to initialize GBM\n");
		return;
	}
	
	_eglInfo.display = eglGetDisplay((NativeDisplayType)_gbm.dev);
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

        _eglInfo.surface = eglCreateWindowSurface(_eglInfo.display, _eglInfo.config, 
		(EGLNativeWindowType) _gbm.surface, NULL);
	assert(_eglInfo.surface != EGL_NO_SURFACE);

	// connect the context to the surface
	result = eglMakeCurrent(_eglInfo.display, _eglInfo.surface, _eglInfo.surface, _eglInfo.context);
	assert(EGL_FALSE != result);

	eglSwapInterval(_eglInfo.display, 1);
	
	eglSwapBuffers(_eglInfo.display, _eglInfo.surface);
	_bo = gbm_surface_lock_front_buffer(_gbm.surface);
        _fb = drmFBGetFromBO(_bo);

        // set mode physical video mode
        if (drmModeSetCrtc(_drm.fd, _drm.crtc_id, _fb->fb_id, 0, 0, &_drm.connector_id, 1, _drm.mode)) {
                debug ("failed to set mode\n");
                return;
        }

        _eglInfo.width = _drm.mode->hdisplay;
        _eglInfo.height = _drm.mode->vdisplay;
        _eglInfo.refresh = _drm.mode->vrefresh;
}

void EGLKMSGraphicsManager::deinitEGL() {
	// Release context resources
	eglMakeCurrent(_eglInfo.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(_eglInfo.display, _eglInfo.surface);
	eglDestroyContext(_eglInfo.display, _eglInfo.context);
	eglTerminate(_eglInfo.display);

	if (_fb->fb_id) {
		drmModeRmFB(_drm.fd, _fb->fb_id);
	}
	
	free(_fb);
}

EGLKMSGraphicsManager::EGLKMSGraphicsManager(SdlEventSource *eventSource)
    : SdlGraphicsManager(eventSource), _hwScreen(nullptr) {

	initEGL();	
}

EGLKMSGraphicsManager::~EGLKMSGraphicsManager() {
	deinitEGL();
}

void EGLKMSGraphicsManager::activateManager() {
	// We activate SDL manager here
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void EGLKMSGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

bool EGLKMSGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return true;
	case OSystem::kFeatureIconifyWindow:
		return false;

	default:
		return OpenGLGraphicsManager::hasFeature(f);
	}
}

void EGLKMSGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	OpenGLGraphicsManager::setFeatureState(f, enable);
}

bool EGLKMSGraphicsManager::getFeatureState(OSystem::Feature f) {
	return OpenGLGraphicsManager::getFeatureState(f);
}

bool EGLKMSGraphicsManager::setGraphicsMode(int mode) {
	return OpenGLGraphicsManager::setGraphicsMode(mode);
}

void EGLKMSGraphicsManager::resetGraphicsScale() {
	OpenGLGraphicsManager::resetGraphicsScale();
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> EGLKMSGraphicsManager::getSupportedFormats() const {
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

void EGLKMSGraphicsManager::updateScreen() {
	OpenGLGraphicsManager::updateScreen();
	eglSwapBuffers(_eglInfo.display, _eglInfo.surface);

	EGLKMSGraphicsManager::drmPageFlip();
}

void EGLKMSGraphicsManager::notifyVideoExpose() {
}

void EGLKMSGraphicsManager::notifyResize(const uint width, const uint height) {
}

void EGLKMSGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void EGLKMSGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

void EGLKMSGraphicsManager::setInternalMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool EGLKMSGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	return setupMode(requestedWidth, requestedHeight);
}

bool EGLKMSGraphicsManager::setupMode(uint width, uint height) {
	if (_hwScreen) {
		//If we have a GLES context already, we have what we need and there's no need go any further
		return(true);
	}

	_hwScreen = SDL_SetVideoMode(0, 0, 32, 0);
	
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

bool EGLKMSGraphicsManager::notifyEvent(const Common::Event &event) {
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

bool EGLKMSGraphicsManager::isHotkey(const Common::Event &event) {
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
