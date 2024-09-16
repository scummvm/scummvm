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
#ifndef BACKENDS_LIBRETRO_GRAPHICS_H
#define BACKENDS_LIBRETRO_GRAPHICS_H

#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "backends/graphics/graphics.h"

#ifdef USE_OPENGL
#include "backends/graphics/opengl/opengl-graphics.h"
#endif

class LibretroPalette {
public:
	const byte *_prevColorsSource;
	unsigned char _colors[256 * 3];
	LibretroPalette(void);
	~LibretroPalette(void) {};
	void set(const byte *colors, uint start, uint num);
	void get(byte *colors, uint start, uint num) const;
	unsigned char *getColor(uint aIndex) const;
	void reset(void) {
		_prevColorsSource = NULL;
	}
};

class LibretroGraphics : public GraphicsManager {

public:
	Graphics::Surface _screen;
	Graphics::Surface _gameScreen;
	Graphics::Surface _overlay;
	Graphics::Surface _mouseImage;
	LibretroPalette _mousePalette;
	LibretroPalette _gamePalette;

private:
	bool _overlayInGUI;
	bool _overlayVisible;
	bool _mouseDontScale;
	bool _mousePaletteEnabled;
	bool _mouseVisible;
	bool _screenUpdatePending;
	int _mouseHotspotX;
	int _mouseHotspotY;
	int _mouseKeyColor;

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
	void showOverlay(bool inGUI) override;
	void hideOverlay(void) override;
	void clearOverlay(void) override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight(void) const override;
	int16 getOverlayWidth(void) const override;
	Graphics::PixelFormat getOverlayFormat() const override;
	const Graphics::Surface *getScreen(void);
	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;
	bool isOverlayInGUI(void);

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	int getDefaultGraphicsMode() const override {
		return 0;
	}
	bool isOverlayVisible() const override {
		return _overlayVisible;
	}
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override {
		return true;
	}
	int getGraphicsMode() const override {
		return 0;
	}
	Graphics::Surface *lockScreen() override {
		return &_gameScreen;
	}
	void unlockScreen() override {}

	void setShakePos(int shakeXOffset, int shakeYOffset) override {}
	int getScreenChangeID() const override {
		return 0;
	}
	void beginGFXTransaction() override {}
	OSystem::TransactionError endGFXTransaction() override {
		return OSystem::kTransactionSuccess;
	}
	void fillScreen(uint32 col) override {}
	void fillScreen(const Common::Rect &r, uint32 col) override {}
	void setFocusRectangle(const Common::Rect &rect) override {}
	void clearFocusRectangle() override {}
	void displayMessageOnOSD(const Common::U32String &msg) override;
	void displayActivityIconOnOSD(const Graphics::Surface *icon) override {}

	void realUpdateScreen(void);

protected:
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
};

#ifdef USE_OPENGL
class LibretroOpenGLGraphics : public OpenGL::OpenGLGraphicsManager {
public:
	LibretroOpenGLGraphics(OpenGL::ContextType contextType);
	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) override { return true; };
	void refreshScreen() override;
	void setSystemMousePosition(const int x, const int y) override {};
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) override;

	bool isOverlayInGUI(void){ return _overlayInGUI; }
	void setMousePosition(int x, int y);
	Common::Point convertWindowToVirtual(int x, int y) const;
	void resetContext(OpenGL::ContextType contextType);
};

class LibretroHWFramebuffer : public OpenGL::Backbuffer {

protected:
	void activateInternal() override;
};
#endif
#endif //BACKENDS_LIBRETRO_GRAPHICS_H
