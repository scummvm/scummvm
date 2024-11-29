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
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/graphics/ios/ios-graphics.h"
#include "backends/graphics/ios/renderbuffer.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"
#include "backends/platform/ios7/ios7_osys_main.h"

iOSGraphicsManager::iOSGraphicsManager() :
	_insets{0, 0, 0, 0} {
	initSurface();
}

iOSGraphicsManager::~iOSGraphicsManager() {
	deinitSurface();
}

void iOSGraphicsManager::initSurface() {
	OSystem_iOS7 *sys = dynamic_cast<OSystem_iOS7 *>(g_system);

	// Create OpenGL context
	GLuint rbo = sys->createOpenGLContext();

	notifyContextCreate(OpenGL::kContextGLES2,
	new OpenGL::RenderbufferTarget(rbo),
	OpenGL::Texture::getRGBAPixelFormat(),
	OpenGL::Texture::getRGBAPixelFormat());
	handleResize(sys->getScreenWidth(), sys->getScreenHeight());

	_old_touch_mode = kTouchModeTouchpad;

	// not in 3D, not in GUI
	sys->applyTouchSettings(false, false);
}

void iOSGraphicsManager::deinitSurface() {
	notifyContextDestroy();
	dynamic_cast<OSystem_iOS7 *>(g_system)->destroyOpenGLContext();
}

void iOSGraphicsManager::notifyResize(const int width, const int height) {
	handleResize(width, height);
}

bool iOSGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, bool resizable, int antialiasing) {
	// As GLES2 provides FBO, OpenGL graphics manager must ask us for a resizable surface
	assert(resizable);
	if (antialiasing != 0) {
		warning("Requesting antialiased video mode while not available");
	}

	/* The iOS and tvOS video modes are always full screen */
	return true;
}

void iOSGraphicsManager::showOverlay(bool inGUI) {
	if (_overlayVisible && inGUI == _overlayInGUI)
		return;

	// Don't change touch mode when not changing mouse coordinates
	if (inGUI) {
		_old_touch_mode = dynamic_cast<OSystem_iOS7 *>(g_system)->getCurrentTouchMode();
		// not in 3D, in overlay
		dynamic_cast<OSystem_iOS7 *>(g_system)->applyTouchSettings(false, true);
	} else if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		dynamic_cast<OSystem_iOS7 *>(g_system)->setCurrentTouchMode(static_cast<TouchMode>(_old_touch_mode));
	}

	OpenGL::OpenGLGraphicsManager::showOverlay(inGUI);
}

void iOSGraphicsManager::hideOverlay() {
	if (_overlayInGUI) {
		// Restore touch mode active before overlay was shown
		dynamic_cast<OSystem_iOS7 *>(g_system)->setCurrentTouchMode(static_cast<TouchMode>(_old_touch_mode));
	}

	OpenGL::OpenGLGraphicsManager::hideOverlay();
}

float iOSGraphicsManager::getHiDPIScreenFactor() const {
	return dynamic_cast<OSystem_iOS7 *>(g_system)->getSystemHiDPIScreenFactor();
}

void iOSGraphicsManager::refreshScreen() {
	dynamic_cast<OSystem_iOS7 *>(g_system)->refreshScreen();
}

bool iOSGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, _activeArea.drawRect.left, _activeArea.drawRect.right);
	mouse.y = CLIP<int16>(mouse.y, _activeArea.drawRect.top, _activeArea.drawRect.bottom);

	setMousePosition(mouse.x, mouse.y);
	mouse = convertWindowToVirtual(mouse.x, mouse.y);

	return true;
}
