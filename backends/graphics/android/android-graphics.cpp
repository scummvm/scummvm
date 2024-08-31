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

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"
#include "backends/graphics/android/android-graphics.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"

#include "graphics/blit.h"
#include "graphics/managed_surface.h"

//
// AndroidGraphicsManager
//
AndroidGraphicsManager::AndroidGraphicsManager() :
	_touchcontrols(nullptr),
	_old_touch_mode(OSystem_Android::TOUCH_MODE_TOUCHPAD) {
	ENTER();

	// Initialize our OpenGL ES context.
	initSurface();

	// not in 3D, not in GUI
	dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(false, false);
	dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
}

AndroidGraphicsManager::~AndroidGraphicsManager() {
	ENTER();

	deinitSurface();

	delete _touchcontrols;
}

void AndroidGraphicsManager::initSurface() {
	LOGD("initializing 2D surface");

	assert(!JNI::haveSurface());
	if (!JNI::initSurface()) {
		error("JNI::initSurface failed");
	}

	if (JNI::egl_bits_per_pixel == 16) {
		// We default to RGB565 and RGBA5551 which is closest to what we setup in Java side
		notifyContextCreate(OpenGL::kContextGLES2,
				new OpenGL::Backbuffer(),
				Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
				Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	} else {
		// If not 16, this must be 24 or 32 bpp so make use of them
		notifyContextCreate(OpenGL::kContextGLES2,
				new OpenGL::Backbuffer(),
#ifdef SCUMM_BIG_ENDIAN
				Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),
				Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)
#else
				Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),
				Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)
#endif
		);
	}

	if (_touchcontrols) {
		_touchcontrols->recreate();
		_touchcontrols->updateGLTexture();
	} else {
		_touchcontrols = createSurface(_defaultFormatAlpha);
	}
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    this, JNI::egl_surface_width, JNI::egl_surface_height);

	handleResize(JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing 2D surface");

	// Deregister us from touch control
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    nullptr, 0, 0);
	if (_touchcontrols) {
		_touchcontrols->destroy();
	}

	notifyContextDestroy();

	JNI::deinitSurface();
}

void AndroidGraphicsManager::resizeSurface() {

	// If we had lost surface just init it again
	if (!JNI::haveSurface()) {
		initSurface();
		return;
	}

	// Recreate the EGL surface, context is preserved
	JNI::deinitSurface();
	if (!JNI::initSurface()) {
		error("JNI::initSurface failed");
	}

	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().setDrawer(
	    this, JNI::egl_surface_width, JNI::egl_surface_height);

	handleResize(JNI::egl_surface_width, JNI::egl_surface_height);
}


void AndroidGraphicsManager::updateScreen() {
	//ENTER();

	if (!JNI::haveSurface())
		return;

	// Sets _forceRedraw if needed
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().beforeDraw();

	OpenGLGraphicsManager::updateScreen();
}

void AndroidGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
	ENTER("%s", msg.encode().c_str());

	JNI::displayMessageOnOSD(msg);
}

void AndroidGraphicsManager::showOverlay(bool inGUI) {
	if (_overlayVisible && inGUI == _overlayInGUI)
		return;

	// Don't change touch mode when not changing mouse coordinates
	if (inGUI) {
		_old_touch_mode = JNI::getTouchMode();
		// not in 3D, in overlay
		dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(false, true);
		dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
	} else if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		JNI::setTouchMode(_old_touch_mode);
	}

	OpenGL::OpenGLGraphicsManager::showOverlay(inGUI);
}

void AndroidGraphicsManager::hideOverlay() {
	if (!_overlayVisible)
		return;

	if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		JNI::setTouchMode(_old_touch_mode);
		dynamic_cast<OSystem_Android *>(g_system)->applyOrientationSettings();
	}

	OpenGL::OpenGLGraphicsManager::hideOverlay();
}

float AndroidGraphicsManager::getHiDPIScreenFactor() const {
	JNI::DPIValues dpi;
	JNI::getDPI(dpi);
	// Scale down the Android factor else the GUI is too big and
	// there is not much options to go smaller
	return dpi[2] / 1.2f;
}

bool AndroidGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	ENTER("%d, %d, %s", requestedWidth, requestedHeight, format.toString().c_str());

	// We get this whenever a new resolution is requested. Since Android is
	// using a fixed output size we do nothing like that here.
	return true;
}

void AndroidGraphicsManager::refreshScreen() {
	//ENTER();

	// Last minute draw of touch controls
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().draw();

	JNI::swapBuffers();
}

void AndroidGraphicsManager::syncVirtkeyboardState(bool virtkeybd_on) {
	_screenAlign = SCREEN_ALIGN_CENTER;
	if (virtkeybd_on) {
		_screenAlign |= SCREEN_ALIGN_TOP;
	} else {
		_screenAlign |= SCREEN_ALIGN_MIDDLE;
	}
	recalculateDisplayAreas();
	_forceRedraw = true;
}

void AndroidGraphicsManager::touchControlInitSurface(const Graphics::ManagedSurface &surf) {
	if (_touchcontrols->getWidth() == surf.w && _touchcontrols->getHeight() == surf.h) {
		return;
	}

	_touchcontrols->allocate(surf.w, surf.h);
	Graphics::Surface *dst = _touchcontrols->getSurface();

	Graphics::crossBlit(
			(byte *)dst->getPixels(), (const byte *)surf.getPixels(),
			dst->pitch, surf.pitch,
			surf.w, surf.h,
			surf.format, dst->format);
	_touchcontrols->updateGLTexture();
}

void AndroidGraphicsManager::touchControlDraw(uint8 alpha, int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) {
	_targetBuffer->enableBlend(OpenGL::Framebuffer::kBlendModeTraditionalTransparency);
	OpenGL::Pipeline *pipeline = getPipeline();
	pipeline->activate();
	if (alpha != 255) {
		pipeline->setColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
	}
	pipeline->drawTexture(_touchcontrols->getGLTexture(),
	                      x, y, w, h, clip);
	if (alpha != 255) {
		pipeline->setColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void AndroidGraphicsManager::touchControlNotifyChanged() {
	// Make sure we redraw the screen
	_forceRedraw = true;
}

bool AndroidGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, _activeArea.drawRect.left, _activeArea.drawRect.right);
	mouse.y = CLIP<int16>(mouse.y, _activeArea.drawRect.top, _activeArea.drawRect.bottom);

	setMousePosition(mouse.x, mouse.y);
	mouse = convertWindowToVirtual(mouse.x, mouse.y);

	return true;
}

AndroidCommonGraphics::State AndroidGraphicsManager::getState() const {
	State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.fullscreen    = getFeatureState(OSystem::kFeatureFullscreenMode);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = getScreenFormat();
#endif
	return state;
}

bool AndroidGraphicsManager::setState(const AndroidCommonGraphics::State &state) {
	beginGFXTransaction();

#ifdef USE_RGB_COLOR
		initSize(state.screenWidth, state.screenHeight, &state.pixelFormat);
#else
		initSize(state.screenWidth, state.screenHeight, nullptr);
#endif
		setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
		setFeatureState(OSystem::kFeatureFullscreenMode, state.fullscreen);
		setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	return endGFXTransaction() == OSystem::kTransactionSuccess;
}
