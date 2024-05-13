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

#include "backends/graphics/metal/metal-graphics.h"
#include "common/translation.h"

enum {
	GFX_METAL = 0
};

namespace Metal {

MetalGraphicsManager::MetalGraphicsManager()
{
	//TODO: Implement
}

MetalGraphicsManager::~MetalGraphicsManager()
{
	//TODO: Implement
}

// Windowed
bool MetalGraphicsManager::gameNeedsAspectRatioCorrection() const {
	//TODO: Implement
	return false;
}

void MetalGraphicsManager::handleResizeImpl(const int width, const int height) {
	//TODO: Implement
}

// GraphicsManager
bool MetalGraphicsManager::hasFeature(OSystem::Feature f) const {
	//TODO: Implement
	switch (f) {
	default:
		return false;
	}
}

void MetalGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	//TODO: Implement
	switch (f) {
	default:
		break;
	}
}

bool MetalGraphicsManager::getFeatureState(OSystem::Feature f) const {
	//TODO: Implement
	switch (f) {
	default:
		return false;
	}
}

const OSystem::GraphicsMode metalGraphicsModes[] = {
	{ "metal",  _s("Metal"), GFX_METAL },
	{ nullptr, nullptr, 0 }
};

const OSystem::GraphicsMode *MetalGraphicsManager::getSupportedGraphicsModes() const {
	return metalGraphicsModes;
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat MetalGraphicsManager::getScreenFormat() const {
	//TODO: Use current state instead
	return _currentFormat;
}

Common::List<Graphics::PixelFormat> MetalGraphicsManager::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> formats;
	// The list of pixel formats that are natively supported by Metal can
	// be found at: https://developer.apple.com/documentation/metal/mtlpixelformat
#ifdef SCUMM_LITTLE_ENDIAN
	// ABGR8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#else
	// RGBA8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#endif
	// RGB565
	formats.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	// RGBA5551
	formats.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0));
	// RGBA4444
	formats.push_back(Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0));

	// These formats are not natively supported by Metal,
	// we convert the pixel format internally.
#ifdef SCUMM_LITTLE_ENDIAN
	// RGBA8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	// ABGR8888
	formats.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
	// RGB555, this is used by SCUMM HE 16 bit games.
	formats.push_back(Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	formats.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return formats;
}
#endif

void MetalGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	//TODO: Implement
}

int MetalGraphicsManager::getScreenChangeID() const {
	//TODO: Implement
	return 0;
}

void MetalGraphicsManager::beginGFXTransaction() {
	//TODO: Implement
}

OSystem::TransactionError MetalGraphicsManager::endGFXTransaction() {
	//TODO: Implement
	return OSystem::kTransactionSuccess;
}

int16 MetalGraphicsManager::getHeight() const {
	//TODO: Implement
	return 0;
}

int16 MetalGraphicsManager::getWidth() const {
	//TODO: Implement
	return 0;
}

void MetalGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//TODO: Implement
}

Graphics::Surface *MetalGraphicsManager::lockScreen() {
	//TODO: Implement
	return nullptr;
}

void MetalGraphicsManager::unlockScreen() {
	//TODO: Implement
}

void MetalGraphicsManager::fillScreen(uint32 col) {
	//TODO: Implement
}

void MetalGraphicsManager::fillScreen(const Common::Rect &r, uint32 col) {
	//TODO: Implement
}

void MetalGraphicsManager::updateScreen() {
	//TODO: Implement
}

void MetalGraphicsManager::setFocusRectangle(const Common::Rect& rect) {
	//TODO: Implement
}

void MetalGraphicsManager::clearFocusRectangle() {
	//TODO: Implement
}

Graphics::PixelFormat MetalGraphicsManager::getOverlayFormat() const {
	return _currentFormat;
}

void MetalGraphicsManager::clearOverlay() {
	//TODO: Implement
}

void MetalGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	//TODO: Implement
}

void MetalGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	//TODO: Implement
}

int16 MetalGraphicsManager::getOverlayHeight() const {
	//TODO: Implement
	return 0;
}

int16 MetalGraphicsManager::getOverlayWidth() const {
	//TODO: Implement
	return 0;
}

bool MetalGraphicsManager::showMouse(bool visible) {
	//TODO: Implement
	return false;
}

void MetalGraphicsManager::warpMouse(int x, int y) {
	//TODO: Implement
}

void MetalGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	//TODO: Implement
}

void MetalGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	//TODO: Implement
}

// PaletteManager
void MetalGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	//TODO: Implement
}

void MetalGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	//TODO: Implement
}

}
