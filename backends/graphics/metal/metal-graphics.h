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

#ifndef BACKENDS_GRAPHICS_METAL_METAL_GRAPHICS_H
#define BACKENDS_GRAPHICS_METAL_METAL_GRAPHICS_H

#include "backends/graphics/windowed.h"

namespace Metal {

class MetalGraphicsManager : virtual public WindowedGraphicsManager {
public:
	MetalGraphicsManager();
	virtual ~MetalGraphicsManager();

protected:
	// Windowed
	bool gameNeedsAspectRatioCorrection() const override;
	void handleResizeImpl(const int width, const int height) override;

	// GraphicsManager
	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif

	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override;
	int getScreenChangeID() const override;

	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

	int16 getHeight() const override;
	int16 getWidth() const override;

	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void fillScreen(uint32 col) override;
	void fillScreen(const Common::Rect &r, uint32 col) override;
	void updateScreen() override;
	void setFocusRectangle(const Common::Rect& rect) override;
	void clearFocusRectangle() override;

	Graphics::PixelFormat getOverlayFormat() const override;
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override;
	int16 getOverlayWidth() const override;

	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = nullptr, const byte *mask = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;

	// PaletteManager
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

private:
	Graphics::PixelFormat _currentFormat;
};

} // End of namespace Metal

#endif
