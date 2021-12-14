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

#include "backends/graphics/android/android-graphics.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

//
// AndroidGraphicsManager
//
AndroidGraphicsManager::AndroidGraphicsManager() {
	ENTER();

	// Initialize our OpenGL ES context.
	initSurface();

}

AndroidGraphicsManager::~AndroidGraphicsManager() {
	ENTER();

	deinitSurface();
}

void AndroidGraphicsManager::initSurface() {
	LOGD("initializing 2D surface");

	assert(!JNI::haveSurface());
	JNI::initSurface();

	// Notify the OpenGL code about our context.
	setContextType(OpenGL::kContextGLES2);

	if (JNI::egl_bits_per_pixel == 16) {
		// We default to RGB565 and RGBA5551 which is closest to what we setup in Java side
		notifyContextCreate(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	} else {
		// If not 16, this must be 24 or 32 bpp so make use of them
#ifdef SCUMM_BIG_ENDIAN
		notifyContextCreate(Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
		notifyContextCreate(Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0), Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
	}

	handleResize(JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing 2D surface");

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

	JNI::swapBuffers();
}

void *AndroidGraphicsManager::getProcAddress(const char *name) const {
	ENTER("%s", name);

	// TODO: Support dynamically loaded OpenGL
	return nullptr;
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
