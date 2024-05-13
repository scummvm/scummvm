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

#include "backends/graphics/metal/metal-graphics.h"
#include "common/translation.h"

#ifdef USE_SCALERS
#include "graphics/scalerplugin.h"
#endif

// metal-cpp is a header-only library.
// Generate the implementation by defining the following:
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

// The implementation of the Metal library has to be generated
// before including these header files.
#include "backends/graphics/metal/shader.h"
#include "backends/graphics/metal/renderer.h"
#include "backends/graphics/metal/pipelines/pipeline.h"
#include "backends/graphics/metal/pipelines/shader.h"

namespace Metal {

MetalGraphicsManager::MetalGraphicsManager()
	: _pipeline(nullptr), _currentState(), _oldState(), _defaultFormat(), _defaultFormatAlpha(),
	_gameScreen(nullptr), _overlay(nullptr), _cursor(nullptr), _cursorMask(nullptr),
	_cursorHotspotX(0), _cursorHotspotY(0), _targetBuffer(nullptr), _renderer(nullptr),
	_cursorHotspotXScaled(0), _cursorHotspotYScaled(0), _cursorWidthScaled(0), _cursorHeightScaled(0),
	_cursorKeyColor(0), _cursorUseKey(true), _cursorDontScale(false), _cursorPaletteEnabled(false),
	_screenChangeID(1 << (sizeof(int) * 8 - 2)), _stretchMode(STRETCH_FIT)
#ifdef USE_SCALERS
	, _scalerPlugins(ScalerMan.getPlugins())
#endif
{
	memset(_gamePalette, 0, sizeof(_gamePalette));
}

MetalGraphicsManager::~MetalGraphicsManager()
{
	delete _gameScreen;
	delete _overlay;
	delete _cursor;
	delete _cursorMask;
	delete _pipeline;
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
	return WindowedGraphicsManager::showMouse(visible);
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

Surface *MetalGraphicsManager::createSurface(const Graphics::PixelFormat &format, bool wantAlpha, bool wantScaler, bool wantMask) {
	uint metalPixelFormat;

#ifdef USE_SCALERS
	if (wantScaler) {
		// TODO: Ensure that the requested pixel format is supported by the scaler
		if (getMetalPixelFormat(format, metalPixelFormat)) {
			return new ScaledTexture(_device, (MTL::PixelFormat)metalPixelFormat, format, format);
		} else {
#ifdef SCUMM_LITTLE_ENDIAN
			return new ScaledTexture(_device, MTL::PixelFormatRGBA8Unorm, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), format);

#else
			return new ScaledTexture(_device, MTL::PixelFormatRGBA8Unorm, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), format);
#endif
		}
	}
#endif
	if (format.bytesPerPixel == 1) {
		if (!wantMask) {
			return new TextureCLUT8GPU(_device, _renderer);
		}

		const Graphics::PixelFormat &virtFormat = wantAlpha ? _defaultFormatAlpha : _defaultFormat;
		const bool supported = getMetalPixelFormat(virtFormat, metalPixelFormat);
		if (!supported) {
			return nullptr;
		} else {
			return new FakeTexture(_device, (MTL::PixelFormat)metalPixelFormat, virtFormat, format);
		}
	} else if (getMetalPixelFormat(format, metalPixelFormat)) {
		return new Texture(_device, (MTL::PixelFormat)metalPixelFormat, format);
#ifdef SCUMM_LITTLE_ENDIAN
	} else if (format == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) { // RGBA8888
#else
	} else if (format == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) { // ABGR8888
#endif
		return new TextureRGBA8888Swap(_device);
	} else if (format == Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)) {
		// Metaldoes not support a texture format usable for RGB555.
		// Since SCUMM uses this pixel format for some games we use
		// pixel format conversion to a supported texture format.
		return new TextureRGB555(_device);
	} else {
		return nullptr;
	}
}

bool MetalGraphicsManager::getMetalPixelFormat(const Graphics::PixelFormat &pixelFormat, uint &metalPixelFormat) const {
#ifdef SCUMM_LITTLE_ENDIAN
	if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) { // ABGR8888
#else
	if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) { // RGBA8888
#endif
		metalPixelFormat = MTL::PixelFormatRGBA8Unorm;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) { // RGB565
		metalPixelFormat = MTL::PixelFormatB5G6R5Unorm;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)) { // RGBA5551
		metalPixelFormat = MTL::PixelFormatBGR5A1Unorm;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0)) { // RGBA4444
		// TODO may need to convert this
		metalPixelFormat = MTL::PixelFormatABGR4Unorm;
		return true;
	} else {
		return false;
	}
}

int MetalGraphicsManager::getGameRenderScale() const {
	return _currentState.scaleFactor;
}

void MetalGraphicsManager::recalculateDisplayAreas() {
	if (!_gameScreen) {
		return;
	}

	WindowedGraphicsManager::recalculateDisplayAreas();

	// Setup drawing limitation for game graphics.
	_targetBuffer->setScissorBox(
		_gameDrawRect.left,
		_gameDrawRect.top,
		_gameDrawRect.width(),
		_gameDrawRect.height());

	_shakeOffsetScaled = Common::Point(_gameScreenShakeXOffset * _gameDrawRect.width() / (int)_currentState.gameWidth,
		_gameScreenShakeYOffset * _gameDrawRect.height() / (int)_currentState.gameHeight);

	// Update the cursor position to adjust for new display area.
	setMousePosition(_cursorX, _cursorY);

	// Force a redraw to assure screen is properly redrawn.
	_forceRedraw = true;
}

void MetalGraphicsManager::updateCursorPalette() {
	if (!_cursor || !_cursor->hasPalette()) {
		return;
	}

	if (_cursorPaletteEnabled) {
		_cursor->setPalette(0, 256, _cursorPalette);
	} else {
		_cursor->setPalette(0, 256, _gamePalette);
	}

	if (_cursorUseKey)
		_cursor->setColorKey(_cursorKeyColor);
}

void MetalGraphicsManager::recalculateCursorScaling() {
	if (!_cursor || !_gameScreen) {
		return;
	}

	uint cursorWidth = _cursor->getWidth();
	uint cursorHeight = _cursor->getHeight();

	// By default we use the unscaled versions.
	_cursorHotspotXScaled = _cursorHotspotX;
	_cursorHotspotYScaled = _cursorHotspotY;
	_cursorWidthScaled = cursorWidth;
	_cursorHeightScaled = cursorHeight;

	// In case scaling is actually enabled we will scale the cursor according
	// to the game screen.
	if (!_cursorDontScale) {
		const frac_t screenScaleFactorX = intToFrac(_gameDrawRect.width()) / _gameScreen->getWidth();
		const frac_t screenScaleFactorY = intToFrac(_gameDrawRect.height()) / _gameScreen->getHeight();

		_cursorHotspotXScaled = fracToInt(_cursorHotspotXScaled * screenScaleFactorX);
		_cursorWidthScaled    = fracToDouble(cursorWidth        * screenScaleFactorX);

		_cursorHotspotYScaled = fracToInt(_cursorHotspotYScaled * screenScaleFactorY);
		_cursorHeightScaled   = fracToDouble(cursorHeight       * screenScaleFactorY);
	}
}

void MetalGraphicsManager::updateLinearFiltering() {
	if (_gameScreen) {
		_gameScreen->enableLinearFiltering(_currentState.filtering);
	}

	if (_cursor) {
		_cursor->enableLinearFiltering(_currentState.filtering);
	}

	if (_cursorMask) {
		_cursorMask->enableLinearFiltering(_currentState.filtering);
	}

	// The overlay UI should also obey the filtering choice (managed via the Filter Graphics checkbox in Graphics Tab).
	// Thus, when overlay filtering is disabled, scaling in Metal is done with nearest (nearest neighbor scaling).
	// It may look crude, but it should be crispier and it's left to user choice to enable filtering.
	if (_overlay) {
		_overlay->enableLinearFiltering(_currentState.filtering);
	}
}

}
