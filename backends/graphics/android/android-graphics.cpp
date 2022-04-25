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

#include "graphics/conversion.h"

#include "backends/graphics/opengl/pipelines/pipeline.h"

#include "backends/graphics/android/android-graphics.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

static void loadBuiltinTexture(JNI::BitmapResources resource, OpenGL::Surface *surf) {
	const Graphics::Surface *src = JNI::getBitmapResource(resource);
	if (!src) {
		error("Failed to fetch touch arrows bitmap");
	}

	surf->allocate(src->w, src->h);
	Graphics::Surface *dst = surf->getSurface();

	Graphics::crossBlit(
			(byte *)dst->getPixels(), (const byte *)src->getPixels(),
			dst->pitch, src->pitch,
			src->w, src->h,
			src->format, dst->format);

	delete src;
}

//
// AndroidGraphicsManager
//
AndroidGraphicsManager::AndroidGraphicsManager() :
	_touchcontrols(nullptr),
	_old_touch_mode(OSystem_Android::TOUCH_MODE_TOUCHPAD) {
	ENTER();

	// Initialize our OpenGL ES context.
	initSurface();

	_touchcontrols = createSurface(_defaultFormatAlpha);
	loadBuiltinTexture(JNI::BitmapResources::TOUCH_ARROWS_BITMAP, _touchcontrols);
	_touchcontrols->updateGLTexture();

	// not in 3D, not in overlay
	dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(false, false);
}

AndroidGraphicsManager::~AndroidGraphicsManager() {
	ENTER();

	deinitSurface();
}

void AndroidGraphicsManager::initSurface() {
	LOGD("initializing 2D surface");

	assert(!JNI::haveSurface());
	JNI::initSurface();

	if (JNI::egl_bits_per_pixel == 16) {
		// We default to RGB565 and RGBA5551 which is closest to what we setup in Java side
		notifyContextCreate(OpenGL::kContextGLES2,
				Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
				Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	} else {
		// If not 16, this must be 24 or 32 bpp so make use of them
		notifyContextCreate(OpenGL::kContextGLES2,
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
	}
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().init(
	    this, JNI::egl_surface_width, JNI::egl_surface_height);

	handleResize(JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing 2D surface");

	// Deregister us from touch control
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().init(
	    nullptr, 0, 0);
	if (_touchcontrols) {
		_touchcontrols->destroy();
	}

	notifyContextDestroy();

	JNI::deinitSurface();
}

void AndroidGraphicsManager::updateScreen() {
	//ENTER();

	if (!JNI::haveSurface())
		return;

	OpenGLGraphicsManager::updateScreen();
}

void AndroidGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
	ENTER("%s", msg.encode().c_str());

	JNI::displayMessageOnOSD(msg);
}

void AndroidGraphicsManager::showOverlay() {
	if (_overlayVisible)
		return;

	_old_touch_mode = JNI::getTouchMode();
	// not in 3D, in overlay
	dynamic_cast<OSystem_Android *>(g_system)->applyTouchSettings(false, true);

	OpenGL::OpenGLGraphicsManager::showOverlay();
}

void AndroidGraphicsManager::hideOverlay() {
	if (!_overlayVisible)
		return;

	// Restore touch mode active before overlay was shown
	JNI::setTouchMode(_old_touch_mode);

	OpenGL::OpenGLGraphicsManager::hideOverlay();
}

float AndroidGraphicsManager::getHiDPIScreenFactor() const {
	// TODO: Use JNI to get DisplayMetrics.density, which according to the documentation
	// seems to be what we want.
	// "On a medium-density screen, DisplayMetrics.density equals 1.0; on a high-density
	//  screen it equals 1.5; on an extra-high-density screen, it equals 2.0; and on a
	//  low-density screen, it equals 0.75. This figure is the factor by which you should
	//  multiply the dp units in order to get the actual pixel count for the current screen."

	return 2.f;
}

bool AndroidGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	ENTER("%d, %d, %s", requestedWidth, requestedHeight, format.toString().c_str());

	// We get this whenever a new resolution is requested. Since Android is
	// using a fixed output size we do nothing like that here.
	// TODO: Support screen rotation
	return true;
}

void AndroidGraphicsManager::refreshScreen() {
	//ENTER();

	// Last minute draw of touch controls
	dynamic_cast<OSystem_Android *>(g_system)->getTouchControls().draw();

	JNI::swapBuffers();
}

void AndroidGraphicsManager::touchControlDraw(int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) {
	_backBuffer.enableBlend(OpenGL::Framebuffer::kBlendModeTraditionalTransparency);
	OpenGL::Pipeline::getActivePipeline()->drawTexture(_touchcontrols->getGLTexture(),
	                                                   x, y, w, h, clip);
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
