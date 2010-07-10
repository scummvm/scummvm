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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/graphics/opengl/opengl-graphics.h"
#include "common/mutex.h"

OpenGLGraphicsManager::OpenGLGraphicsManager()
	:
	_gameTexture(0), _overlayTexture(0), _mouseTexture(0) 	{
}

OpenGLGraphicsManager::~OpenGLGraphicsManager() {

}

void OpenGLGraphicsManager::init() {

}

//
// Feature
//

bool OpenGLGraphicsManager::hasFeature(OSystem::Feature f) {
	return false;
}

void OpenGLGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {

}

bool OpenGLGraphicsManager::getFeatureState(OSystem::Feature f) {
	return false;
}

//
// Screen format and modes
//

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Normal", GFX_NORMAL},
	{0, 0, 0}
};

const OSystem::GraphicsMode *OpenGLGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OpenGLGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

bool OpenGLGraphicsManager::setGraphicsMode(int mode) {
	return false;
}

int OpenGLGraphicsManager::getGraphicsMode() const {
	return 0;
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat OpenGLGraphicsManager::getScreenFormat() const {
	return Graphics::PixelFormat();
}

Common::List<Graphics::PixelFormat> OpenGLGraphicsManager::getSupportedFormats() {
	return Common::List<Graphics::PixelFormat>();
}

#endif

void OpenGLGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {

}

int OpenGLGraphicsManager::getScreenChangeID() const {
	return 0;
}

//
// GFX
//

void OpenGLGraphicsManager::beginGFXTransaction() {

}

OSystem::TransactionError OpenGLGraphicsManager::endGFXTransaction() {
	return OSystem::kTransactionSuccess;
}

//
// Screen
//

int16 OpenGLGraphicsManager::getHeight() {
	return 0;
}

int16 OpenGLGraphicsManager::getWidth() {
	return 0;
}

void OpenGLGraphicsManager::setPalette(const byte *colors, uint start, uint num) {

}

void OpenGLGraphicsManager::grabPalette(byte *colors, uint start, uint num) {

}

void OpenGLGraphicsManager::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {

}

Graphics::Surface *OpenGLGraphicsManager::lockScreen() {
	_lockedScreen = Graphics::Surface();
	return &_lockedScreen;
}

void OpenGLGraphicsManager::unlockScreen() {

}

void OpenGLGraphicsManager::fillScreen(uint32 col) {

}

void OpenGLGraphicsManager::updateScreen() {

}

void OpenGLGraphicsManager::setShakePos(int shakeOffset) {

}

void OpenGLGraphicsManager::setFocusRectangle(const Common::Rect& rect) {

}

void OpenGLGraphicsManager::clearFocusRectangle() {

}

//
// Overlay
//

void OpenGLGraphicsManager::showOverlay() {

}

void OpenGLGraphicsManager::hideOverlay() {

}

Graphics::PixelFormat OpenGLGraphicsManager::getOverlayFormat() const {
	return Graphics::PixelFormat();
}

void OpenGLGraphicsManager::clearOverlay() {

}

void OpenGLGraphicsManager::grabOverlay(OverlayColor *buf, int pitch) {

}

void OpenGLGraphicsManager::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {

}

int16 OpenGLGraphicsManager::getOverlayHeight() {
	return 0;
}

int16 OpenGLGraphicsManager::getOverlayWidth() {
	return 0;
}

//
// Cursor
//

bool OpenGLGraphicsManager::showMouse(bool visible) {
	return false;
}

void OpenGLGraphicsManager::warpMouse(int x, int y) {

}

void OpenGLGraphicsManager::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {

}

void OpenGLGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {

}

void OpenGLGraphicsManager::disableCursorPalette(bool disable) {

}

//
// Misc
//

void OpenGLGraphicsManager::displayMessageOnOSD(const char *msg) {

}

//
// Intern
//

void OpenGLGraphicsManager::internUpdateScreen() {

}

bool OpenGLGraphicsManager::loadGFXMode() {
	return false;
}

void OpenGLGraphicsManager::unloadGFXMode() {

}

bool OpenGLGraphicsManager::hotswapGFXMode() {
	return false;
}
