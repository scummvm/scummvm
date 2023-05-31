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

iOSGraphicsManager::iOSGraphicsManager() {
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
	// Currently iOS runs the ARMs in little-endian mode but prepare if
	// that is changed in the future.
#ifdef SCUMM_LITTLE_ENDIAN
	Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
	Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#else
	Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
	Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#endif
	handleResize(sys->getScreenWidth(), sys->getScreenHeight());
}

void iOSGraphicsManager::deinitSurface() {
	notifyContextDestroy();
	dynamic_cast<OSystem_iOS7 *>(g_system)->destroyOpenGLContext();
}

void iOSGraphicsManager::notifyResize(const int width, const int height) {
	handleResize(width, height);
}

iOSCommonGraphics::State iOSGraphicsManager::getState() const {
	State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = getScreenFormat();
#endif
	return state;
}

bool iOSGraphicsManager::setState(const iOSCommonGraphics::State &state) {
	beginGFXTransaction();

#ifdef USE_RGB_COLOR
	initSize(state.screenWidth, state.screenHeight, &state.pixelFormat);
#else
	initSize(state.screenWidth, state.screenHeight, nullptr);
#endif
	setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
	setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	return endGFXTransaction() == OSystem::kTransactionSuccess;
}


bool iOSGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	/* The iOS and tvOS video modes are always full screen */
	return true;
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
