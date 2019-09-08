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

#include "backends/platform/android/android.h"
#include "backends/platform/android/graphics.h"
#include "backends/platform/android/jni.h"

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
	LOGD("initializing surface");

	assert(!JNI::haveSurface());
	JNI::initSurface();

	// Notify the OpenGL code about our context.
	// FIXME: Support OpenGL ES 2 contexts
	setContextType(OpenGL::kContextGLES);

	// We default to RGB565 and RGBA5551 which is closest to the actual output
	// mode we setup.
	notifyContextCreate(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0), Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));

	handleResize(JNI::egl_surface_width, JNI::egl_surface_height);
}

void AndroidGraphicsManager::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing surface");

	notifyContextDestroy();

	JNI::deinitSurface();
}

void AndroidGraphicsManager::updateScreen() {
	//ENTER();

	if (!JNI::haveSurface())
		return;

	OpenGLGraphicsManager::updateScreen();
}

void AndroidGraphicsManager::displayMessageOnOSD(const char *msg) {
	ENTER("%s", msg);

	JNI::displayMessageOnOSD(msg);
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
