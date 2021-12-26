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

#ifndef ANDROID_GRAPHICS_H
#define ANDROID_GRAPHICS_H

#include "common/scummsys.h"
#include "backends/graphics/graphics.h"
#include "graphics/tinygl/pixelbuffer.h"

class AndroidGraphicsManager : public GraphicsManager {
public:
	AndroidGraphicsManager();
	virtual ~AndroidGraphicsManager();

	void initSurface();
	void deinitSurface();

	void updateScreen() override;

	void displayMessageOnOSD(const Common::U32String &msg);

	bool notifyMousePosition(Common::Point &mouse);
	Common::Point getMousePosition() { return Common::Point(_cursorX, _cursorY); }
	void setMousePosition(int x, int y) { _cursorX = x; _cursorY = y; }

	virtual void beginGFXTransaction() {}
	virtual OSystem::TransactionError endGFXTransaction() { return OSystem::kTransactionSuccess; }

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	int getGraphicsMode() const override;

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	void showOverlay() override;
	void hideOverlay() override;
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	virtual void copyRectToOverlay(const void *buf, int pitch,
									int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override;
	int16 getOverlayWidth() const override;
	Graphics::PixelFormat getOverlayFormat() const override;
	bool isOverlayVisible() const override { return _show_overlay; }

	int16 getHeight() const override;
	int16 getWidth() const override;

	// PaletteManager API
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y,
									int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	virtual void fillScreen(uint32 col);

	virtual void setShakePos(int shakeXOffset, int shakeYOffset) {};
	virtual void setFocusRectangle(const Common::Rect& rect) {}
	virtual void clearFocusRectangle() {}

	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format) override;
	int getScreenChangeID() const override;

	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	bool lockMouse(bool lock) override;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								bool dontScale,
								const Graphics::PixelFormat *format) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;


	void setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d);

	void setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d, bool isGame);
	void updateScreenRect();
	const GLESBaseTexture *getActiveTexture() const;
	void clipMouse(Common::Point &p) const;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override;
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif

protected:
	void setSystemMousePosition(int x, int y) {}

	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format);

	void refreshScreen();

	void *getProcAddress(const char *name) const;

private:
	void setCursorPaletteInternal(const byte *colors, uint start, uint num);
	void disableCursorPalette();
	void initOverlay();
	void initViewport();
	void updateEventScale();
	void initSizeIntern(uint width, uint height, const Graphics::PixelFormat *format);

	enum FixupType {
		kClear = 0,		// glClear
		kClearSwap,		// glClear + swapBuffers
		kClearUpdate	// glClear + updateScreen
	};

	void clearScreen(FixupType type, byte count = 1);
#ifdef USE_RGB_COLOR
	void initTexture(GLESBaseTexture **texture, uint width, uint height,
						const Graphics::PixelFormat *format);
#endif

private:
	int _screenChangeID;
	int _graphicsMode;
	bool _opengl;
	bool _fullscreen;
	bool _ar_correction;
	bool _force_redraw;

	// Game layer
	GLESBaseTexture *_game_texture;
	Graphics::PixelBuffer _game_pbuf;
	OpenGL::FrameBuffer *_frame_buffer;

	/**
	 * The position of the mouse cursor, in window coordinates.
	 */
	int _cursorX, _cursorY;

	// Overlay layer
	GLES4444Texture *_overlay_texture;
	bool _show_overlay;

	// Mouse layer
	GLESBaseTexture *_mouse_texture;
	GLESBaseTexture *_mouse_texture_palette;
	GLES5551Texture *_mouse_texture_rgb;
	Common::Point _mouse_hotspot;
	uint32 _mouse_keycolor;
	int _mouse_targetscale;
	bool _show_mouse;
	bool _use_mouse_palette;
};

#endif
