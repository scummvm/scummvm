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

#ifndef BACKENDS_GRAPHICS3D_ANDROID_ANDROID_GRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_ANDROID_ANDROID_GRAPHICS3D_H

#include "common/scummsys.h"

#include "backends/graphics/graphics.h"
#include "backends/graphics/android/android-graphics.h"
#include "backends/graphics3d/opengl/framebuffer.h"
#include "backends/graphics3d/android/texture.h"

#include "backends/platform/android/touchcontrols.h"

class AndroidGraphics3dManager :
	public GraphicsManager, public AndroidCommonGraphics, public TouchControlsDrawer {
public:
	AndroidGraphics3dManager();
	virtual ~AndroidGraphics3dManager();

	virtual void initSurface() override;
	virtual void deinitSurface() override;
	virtual void resizeSurface() override;

	virtual AndroidCommonGraphics::State getState() const override;
	virtual bool setState(const AndroidCommonGraphics::State &state) override;

	void updateScreen() override;

	void displayMessageOnOSD(const Common::U32String &msg);

	virtual bool notifyMousePosition(Common::Point &mouse) override;
	virtual Common::Point getMousePosition() override {
		return Common::Point(_cursorX, _cursorY);
	}
	void setMousePosition(int x, int y) {
		_cursorX = x;
		_cursorY = y;
	}

	virtual void beginGFXTransaction() {}
	virtual OSystem::TransactionError endGFXTransaction() {
		return OSystem::kTransactionSuccess;
	}

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	virtual int getGraphicsMode() const override;

	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) const override;

	virtual void showOverlay(bool inGUI) override;
	virtual void hideOverlay() override;
	virtual void clearOverlay() override;
	virtual void grabOverlay(Graphics::Surface &surface) const override;
	virtual void copyRectToOverlay(const void *buf, int pitch,
	                               int x, int y, int w, int h) override;
	virtual int16 getOverlayHeight() const override;
	virtual int16 getOverlayWidth() const override;
	virtual Graphics::PixelFormat getOverlayFormat() const override;
	virtual bool isOverlayVisible() const override {
		return _show_overlay;
	}

	virtual int16 getHeight() const override;
	virtual int16 getWidth() const override;

	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num) override;
	virtual void grabPalette(byte *colors, uint start, uint num) const override;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y,
	                              int w, int h) override;
	virtual Graphics::Surface *lockScreen() override;
	virtual void unlockScreen() override;
	virtual void fillScreen(uint32 col) override;
	virtual void fillScreen(const Common::Rect &r, uint32 col) override;

	virtual void setShakePos(int shakeXOffset, int shakeYOffset) {};
	virtual void setFocusRectangle(const Common::Rect &rect) {}
	virtual void clearFocusRectangle() {}

	virtual void initSize(uint width, uint height,
	                      const Graphics::PixelFormat *format) override;
	virtual int getScreenChangeID() const override;

	virtual bool showMouse(bool visible) override;
	virtual void warpMouse(int x, int y) override;
	virtual bool lockMouse(bool lock) override;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
	                            int hotspotY, uint32 keycolor,
	                            bool dontScale,
	                            const Graphics::PixelFormat *format, const byte *mask) override;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override;

	float getHiDPIScreenFactor() const override;

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif

	void touchControlInitSurface(const Graphics::ManagedSurface &surf) override;
	void touchControlNotifyChanged() override;
	void touchControlDraw(uint8 alpha, int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) override;

	void syncVirtkeyboardState(bool virtkeybd_on) override;
	void applyTouchSettings() const override;

protected:
	void updateScreenRect();
	void updateCursorScaling();
	const GLESBaseTexture *getActiveTexture() const;

	Common::Point convertScreenToVirtual(int &x, int &y) const;
	Common::Point convertVirtualToScreen(int x, int y) const;

	void setSystemMousePosition(int x, int y) {}

	bool loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format);

private:
	void initOverlay();

	enum FixupType {
		kClear = 0,     // glClear
		kClearSwap,     // glClear + swapBuffers
		kClearUpdate    // glClear + updateScreen
	};

	void clearScreen(FixupType type, byte count = 1);

private:
	int _screenChangeID;
	int _graphicsMode;
	bool _fullscreen;
	bool _ar_correction;
	bool _force_redraw;

	bool _virtkeybd_on;

	// Game layer
	GLESTexture *_game_texture;
	OpenGL::FrameBuffer *_frame_buffer;

#ifdef USE_RGB_COLOR
	// Backup of the previous pixel format to pass it back when we leave 3d
	Graphics::PixelFormat _2d_pixel_format;
#endif

	/**
	 * The position of the mouse cursor, in window coordinates.
	 */
	int _cursorX, _cursorY;

	// Overlay layer
	GLESTexture *_overlay_background;
	GLESTexture *_overlay_texture;
	bool _show_overlay;
	bool _overlay_in_gui;

	// Mouse layer
	GLESBaseTexture *_mouse_texture;
	GLESFakePaletteTexture *_mouse_texture_palette;
	GLESTexture *_mouse_texture_rgb;
	Common::Point _mouse_hotspot;
	Common::Point _mouse_hotspot_scaled;
	int _mouse_width_scaled, _mouse_height_scaled;
	bool _mouse_dont_scale;
	bool _show_mouse;

	// Touch controls layer
	GLESTexture *_touchcontrols_texture;
	int _old_touch_mode;
};

#endif
