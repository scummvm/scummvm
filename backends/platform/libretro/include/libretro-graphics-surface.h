/* Copyright (C) 2024 Giovanni Cascione <ing.cascione@gmail.com>
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
#ifndef BACKENDS_LIBRETRO_GRAPHICS_SURFACE_H
#define BACKENDS_LIBRETRO_GRAPHICS_SURFACE_H

#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/managed_surface.h"
#include "backends/graphics/windowed.h"

class LibretroGraphics : public WindowedGraphicsManager {

public:
	Graphics::ManagedSurface _screen;
	Graphics::Surface _gameScreen;
	Graphics::Surface _overlay;
	Graphics::Surface _cursor;
	Graphics::Palette _cursorPalette;
	Graphics::Palette _gamePalette;

private:
	bool _cursorDontScale;
	bool _cursorPaletteEnabled;
	bool _screenUpdatePending;
	int _cursorHotspotX;
	int _cursorHotspotY;
	int _cursorKeyColor;
	int _screenChangeID;
	int _cursorHotspotXScaled;
	int _cursorHotspotYScaled;
	float _cursorWidthScaled;
	float _cursorHeightScaled;

public:
	LibretroGraphics();
	~LibretroGraphics();
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
	const OSystem::GraphicsMode *getSupportedGraphicsModes(void) const override;
	void initSize(uint width, uint height, const Graphics::PixelFormat *format) override;
	int16 getHeight(void) const override;
	int16 getWidth(void) const override;
	Graphics::PixelFormat getScreenFormat(void) const override;
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	void updateScreen(void) override;
	void clearOverlay(void) override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight(void) const override;
	int16 getOverlayWidth(void) const override;
	Graphics::PixelFormat getOverlayFormat() const override;
	const Graphics::ManagedSurface *getScreen(void);
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;
	bool isOverlayInGUI(void);

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	int getDefaultGraphicsMode() const override { return 0; }
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override { return true; }
	int getGraphicsMode() const override { return 0; }
	Graphics::Surface *lockScreen() override { return &_gameScreen; }
	void unlockScreen() override {}

	int getScreenChangeID() const override;
	void beginGFXTransaction() override {}
	OSystem::TransactionError endGFXTransaction() override;
	void fillScreen(uint32 col) override {}
	void fillScreen(const Common::Rect &r, uint32 col) override {}
	void setFocusRectangle(const Common::Rect &rect) override {}
	void clearFocusRectangle() override {}

	void realUpdateScreen(void);

	bool gameNeedsAspectRatioCorrection() const override { return false; }
	void handleResizeImpl(const int width, const int height) override;
	void setSystemMousePosition(const int x, const int y) override {}
	void setMousePosition(int x, int y);

	void displayMessageOnOSD(const Common::U32String &msg) override;

protected:
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
private:
	void overrideCursorScaling();
};
#endif //BACKENDS_LIBRETRO_GRAPHICS_SURFACE_H
