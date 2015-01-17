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
#include "backends/graphics/gles-custom/gles-custom.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#ifdef USE_OSD
#include "common/translation.h"
#endif

#ifdef USE_GLES_RPI
#include <bcm_host.h>
#endif

#ifdef USE_GLES_FBDEV
// Includes for framebuffer size retrieval
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/vt.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef USE_GLES_KMS
// This hacky define is needed since there is an struct member called "virtual" in xf86drm.h
#define virtual __virtual
#include <xf86drm.h>
#undef virtual
#include <xf86drmMode.h>
#include <gbm.h>

drmEventContext evctx;

void drmPageFlipHandler(int fd, uint frame, uint sec, uint usec, void *data) {
	int *waiting_for_flip = (int*)data;
	*waiting_for_flip = 0;
}

drm_fb *OpenGLCustomGraphicsManager::drm_fb_get_from_bo(gbm_bo *bob) {
	drm_fb *fbu = (drm_fb*)gbm_bo_get_user_data(bob);
	uint32_t width, height, stride, handle;

	if (fbu) {
		return fbu;
	}
	
	fbu = (drm_fb*)calloc(1, sizeof *fbu);
	fbu->bo = bob;

	width = gbm_bo_get_width(bob);
	height = gbm_bo_get_height(bob);
	stride = gbm_bo_get_stride(bob);
	handle = gbm_bo_get_handle(bob).u32;

	if (drmModeAddFB(drm.fd, width, height, 24, 32, stride, handle, &fbu->fb_id)) {
		debug("Could not add drm framebuffer\n");
		free(fbu);
		return NULL;
	}

	// We used to pass the destroy callback function here. Now it's done manually in deinit_egl()
	gbm_bo_set_user_data(bob, fbu, NULL);
	return fbu;
}

void OpenGLCustomGraphicsManager::drmPageFlip(void) {
	int waiting_for_flip = 1;

	gbm_bo *next_bo = gbm_surface_lock_front_buffer(gbm.surface);
	fb = drm_fb_get_from_bo(next_bo);

	if (drmModePageFlip(drm.fd, drm.crtc_id, fb->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip)){
		debug ("Failed to queue pageflip\n");
		return;
	}

	while (waiting_for_flip) {
		FD_ZERO(&fds);
		
		FD_SET(0, &fds);
		FD_SET(drm.fd, &fds);

		select(drm.fd+1, &fds, NULL, NULL, NULL);
		
		drmHandleEvent(drm.fd, &evctx);
	}
	
	// release last buffer to render on again
	gbm_surface_release_buffer(gbm.surface, bo);
	bo = next_bo;
}

bool OpenGLCustomGraphicsManager::init_drm(void) {
	static const char *modules[] = {
			"i915", "radeon", "nouveau", "vmwgfx", "omapdrm", "exynos", "msm"
	};
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	uint i, area;

	// In plain C, we can just init evctx at declare time, but it's now allowed in C++
	evctx.version = DRM_EVENT_CONTEXT_VERSION;
	evctx.page_flip_handler = drmPageFlipHandler;

	for (i = 0; i < ARRAYSIZE(modules); i++) {
		debug("trying to load module %s...", modules[i]);
		drm.fd = drmOpen(modules[i], NULL);
		if (drm.fd < 0) {
			debug("failed.");
		} else {
			debug("success.");
			break;
		}
	}

	if (drm.fd < 0) {
		debug("could not open drm device\n");
		return false;
	}

	resources = drmModeGetResources(drm.fd);
	if (!resources) {
		debug("drmModeGetResources failed\n");
		return false;
	}

	// find a connected connector
	for (i = 0; i < (uint)resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm.fd, resources->connectors[i]);
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
			drm.mode = current_mode;
			area = current_area;
		}
	}

	if (!drm.mode) {
		debug("could not find mode\n");
		return false;
	}

	// find encoder
	for (i = 0; i < (uint)resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm.fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (!encoder) {
		debug("no encoder found\n");
		return false;
	}

	drm.crtc_id = encoder->crtc_id;
	drm.connector_id = connector->connector_id;

	return true;
}

bool OpenGLCustomGraphicsManager::init_gbm() {
	gbm.dev = gbm_create_device(drm.fd);

	gbm.surface = gbm_surface_create(gbm.dev,
			drm.mode->hdisplay, drm.mode->vdisplay,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if (!gbm.surface) {
		debug ("failed to create gbm surface\n");
		return -1;
	}
	
	return true;
}
#endif // USE_GLES_KMS

void OpenGLCustomGraphicsManager::init_egl() {
	static const EGLint attribute_list[] = {
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
	    EGL_ALPHA_SIZE, 8,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_NONE
	};
	
	// create an EGL rendering context
	static const EGLint context_attributes[] = {
	    EGL_CONTEXT_CLIENT_VERSION, 1,
	    EGL_NONE
	};

	EGLint num_config;
	
#ifdef USE_GLES_RPI
	bcm_host_init();
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	
	// create a Dispmanx EGL window surface
	int32_t success = graphics_get_display_size(0, &eglInfo.width, &eglInfo.height);
	assert(success >= 0);

	vc_dispmanx_rect_set(&dst_rect, 0, 0, eglInfo.width, eglInfo.height);
    
	vc_dispmanx_rect_set(&src_rect, 0, 0, eglInfo.width << 16, eglInfo.height << 16);
    
	dispman_display = vc_dispmanx_display_open(0);
	dispman_update = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
                                              10, &dst_rect, 0, &src_rect,
                                              DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);
    
	nativewindow.element = dispman_element;
	nativewindow.width = eglInfo.width;
	nativewindow.height = eglInfo.height;
    
	vc_dispmanx_update_submit_sync(dispman_update);
#endif // USE_GLES_RPI
	
#ifdef USE_GLES_FBDEV
	fb_var_screeninfo vinfo;
	int fb = open("/dev/fb0", O_RDWR, 0);
	if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		debug("Error obtainig framebuffer info\n");
		return;
	}
	close (fb);

	nativewindow.width = vinfo.xres;
	nativewindow.height = vinfo.yres;
#endif	

#ifdef USE_GLES_KMS
	if (!init_drm()) {
		debug("failed to initialize DRM\n");
		return;
	}

	if (!init_gbm()) {
		debug("failed to initialize GBM\n");
		return;
	}
#endif	
	
#ifdef USE_GLES_KMS
	eglInfo.display = eglGetDisplay((NativeDisplayType)gbm.dev);
#else
	eglInfo.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
	assert(eglInfo.display != EGL_NO_DISPLAY);

	// initialize the EGL display connection
	EGLBoolean result = eglInitialize(eglInfo.display, NULL, NULL);
	assert(EGL_FALSE != result);
    
	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(eglInfo.display, attribute_list, &eglInfo.config, 1, &num_config);
	assert(EGL_FALSE != result);
    
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	
	eglInfo.context = eglCreateContext(eglInfo.display, eglInfo.config, EGL_NO_CONTEXT, context_attributes);
	assert(eglInfo.context != EGL_NO_CONTEXT);

#ifdef USE_GLES_KMS
        eglInfo.surface = eglCreateWindowSurface(eglInfo.display, eglInfo.config, 
		(EGLNativeWindowType) gbm.surface, NULL);
#else
	eglInfo.surface = eglCreateWindowSurface(eglInfo.display, eglInfo.config, &nativewindow, NULL);
#endif
	assert(eglInfo.surface != EGL_NO_SURFACE);

	// connect the context to the surface
	result = eglMakeCurrent(eglInfo.display, eglInfo.surface, eglInfo.surface, eglInfo.context);
	assert(EGL_FALSE != result);

	eglSwapInterval(eglInfo.display, 1);
	
#ifdef USE_GLES_KMS
	eglSwapBuffers(eglInfo.display, eglInfo.surface);
	bo = gbm_surface_lock_front_buffer(gbm.surface);
        fb = drm_fb_get_from_bo(bo);

        // set mode physical video mode
        if (drmModeSetCrtc(drm.fd, drm.crtc_id, fb->fb_id, 0, 0, &drm.connector_id, 1, drm.mode)) {
                debug ("failed to set mode\n");
                return;
        }

        eglInfo.width = drm.mode->hdisplay;
        eglInfo.height = drm.mode->vdisplay;
        eglInfo.refresh = drm.mode->vrefresh;
#endif
}

void OpenGLCustomGraphicsManager::deinit_egl() {
	// Release context resources
	eglMakeCurrent(eglInfo.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(eglInfo.display, eglInfo.surface);
	eglDestroyContext(eglInfo.display, eglInfo.context);
	eglTerminate(eglInfo.display);

#ifdef USE_GLES_RPI
	dispman_update = vc_dispmanx_update_start(0);
	vc_dispmanx_element_remove(dispman_update, dispman_element);
	vc_dispmanx_update_submit_sync(dispman_update);
	vc_dispmanx_display_close(dispman_display);
#endif

#ifdef USE_GLES_FBDEV
	// Re-enable cursor blinking
    	system("setterm -cursor on");
#endif
	
#ifdef USE_GLES_KMS
	if (fb->fb_id) {
		drmModeRmFB(drm.fd, fb->fb_id);
	}
	
	free(fb);
#endif
}

OpenGLCustomGraphicsManager::OpenGLCustomGraphicsManager(SdlEventSource *eventSource)
    : SdlGraphicsManager(eventSource), _lastVideoModeLoad(0), _hwScreen(nullptr), _lastRequestedWidth(0), _lastRequestedHeight(0),
      _graphicsScale(0), _ignoreLoadVideoMode(false), _gotResize(false), _wantsFullScreen(false), _ignoreResizeEvents(0),
      _desiredFullscreenWidth(0), _desiredFullscreenHeight(0) {

	init_egl();	
}

OpenGLCustomGraphicsManager::~OpenGLCustomGraphicsManager() {
	deinit_egl();
}

void OpenGLCustomGraphicsManager::activateManager() {
	// We activate SDL manager here
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void OpenGLCustomGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

bool OpenGLCustomGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return true;
	case OSystem::kFeatureIconifyWindow:
		return false;

	default:
		return OpenGLGraphicsManager::hasFeature(f);
	}
}

void OpenGLCustomGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		assert(getTransactionMode() != kTransactionNone);
		_wantsFullScreen = enable;
		break;

	default:
		OpenGLGraphicsManager::setFeatureState(f, enable);
	}
}

bool OpenGLCustomGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return _wantsFullScreen;
	default:
		return OpenGLGraphicsManager::getFeatureState(f);
	}
}

bool OpenGLCustomGraphicsManager::setGraphicsMode(int mode) {
	_graphicsScale = 1;
	return OpenGLGraphicsManager::setGraphicsMode(mode);
}

void OpenGLCustomGraphicsManager::resetGraphicsScale() {
	OpenGLGraphicsManager::resetGraphicsScale();
	_graphicsScale = 1;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> OpenGLCustomGraphicsManager::getSupportedFormats() const {
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

void OpenGLCustomGraphicsManager::updateScreen() {
	if (_ignoreResizeEvents) {
		--_ignoreResizeEvents;
	}

	OpenGLGraphicsManager::updateScreen();
	eglSwapBuffers(eglInfo.display, eglInfo.surface);

#ifdef USE_GLES_KMS 
       OpenGLCustomGraphicsManager::drmPageFlip();
#endif
}

void OpenGLCustomGraphicsManager::notifyVideoExpose() {
}

void OpenGLCustomGraphicsManager::notifyResize(const uint width, const uint height) {
}

void OpenGLCustomGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void OpenGLCustomGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

void OpenGLCustomGraphicsManager::setInternalMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool OpenGLCustomGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
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

bool OpenGLCustomGraphicsManager::setupMode(uint width, uint height) {
	if (_hwScreen) {
		//If we have a GLES context already, we have what we need and there's no need go any further
		return(true);
	}

	#ifdef USE_GLES_RPI
	_hwScreen = SDL_SetVideoMode(0, 0, 16, 0);
	#else
	_hwScreen = SDL_SetVideoMode(0, 0, 32, 0);
	#endif	
	
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

bool OpenGLCustomGraphicsManager::notifyEvent(const Common::Event &event) {
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

bool OpenGLCustomGraphicsManager::isHotkey(const Common::Event &event) {
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
