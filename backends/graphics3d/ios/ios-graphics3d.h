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

#ifndef BACKENDS_GRAPHICS3D_IOS_IOS_GRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_IOS_IOS_GRAPHICS3D_H

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "common/scummsys.h"

#include "backends/graphics/windowed.h"
#include "backends/graphics/ios/ios-graphics.h"
#include "backends/graphics3d/opengl/framebuffer.h"
#include "backends/graphics3d/opengl/tiledsurface.h"
#include "backends/graphics3d/opengl/surfacerenderer.h"

class iOSGraphics3dManager : virtual public WindowedGraphicsManager, public iOSCommonGraphics {
public:
	iOSGraphics3dManager();
	virtual ~iOSGraphics3dManager();

	//WindowedGraphicsManager
	bool gameNeedsAspectRatioCorrection() const override;
	void handleResizeImpl(const int width, const int height) override;
	virtual void setSystemMousePosition(const int x, const int y) override {};

	//iOSCommonGraphics
	void initSurface() override;
	void deinitSurface() override;

	void notifyResize(const int width, const int height) override;

	virtual iOSCommonGraphics::State getState() const override;
	virtual bool setState(const iOSCommonGraphics::State &state) override;

	bool notifyMousePosition(Common::Point &mouse) override;
	Common::Point getMousePosition() override { return Common::Point(_cursorX, _cursorY); }

	// GraphicsManager API - Features
	bool hasFeature(OSystem::Feature f) const override;
	bool getFeatureState(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	int getGraphicsMode() const override;

	void beginGFXTransaction() override {}
	OSystem::TransactionError endGFXTransaction() override {
		return OSystem::kTransactionSuccess;
	}

	// GraphicsManager API - Graphics mode
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override { return _overlayFormat; }
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override {
		Common::List<Graphics::PixelFormat> supportedFormats;
		return supportedFormats;
	}
#endif
	int getScreenChangeID() const override { return _screenChangeCount; }
	void initSize(uint w, uint h, const Graphics::PixelFormat *format) override;
	int16 getHeight() const override;
	int16 getWidth() const override;

	float getHiDPIScreenFactor() const override;

	// GraphicsManager API - Draw methods
	void updateScreen() override;
	// Following methods are not used by 3D graphics managers
	void setPalette(const byte *colors, uint start, uint num) override {}
	void grabPalette(byte *colors, uint start, uint num) const override {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override {}
	Graphics::Surface *lockScreen() override { return nullptr; }
	void unlockScreen() override {}
	void fillScreen(uint32 col) override {}
	void fillScreen(const Common::Rect &r, uint32 col) override {}
	void setShakePos(int shakeXOffset, int shakeYOffset) override {};
	void setFocusRectangle(const Common::Rect& rect) override {}
	void clearFocusRectangle() override {}

	// GraphicsManager API - Overlay
	const OSystem::GraphicsMode *getSupportedStretchModes() const override;
	int getDefaultStretchMode() const override;
	bool setStretchMode(int mode) override;
	int getStretchMode() const override;
	void showOverlay(bool inGUI) override;
	void hideOverlay() override;
	Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayWidth() const override;
	int16 getOverlayHeight() const override;

	// GraphicsManager API - Mouse
	bool showMouse(bool visible) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = NULL) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override {}

protected:
	void updateCursorScaling();
	void createDepthAndStencilBuffer(int width, int height);
	void updateDepthAndStencilBuffer(int width, int height);

	int _screenChangeCount;
	int _stretchMode;
	bool _aspectRatioCorrection;
	bool _mouseDontScale;
	Common::Point _mouseHotspot;
	Common::Point _mouseHotspotScaled;
	int _mouseWidthScaled, _mouseHeightScaled;

	Graphics::PixelFormat _overlayFormat;
	OpenGL::TiledSurface *_overlayScreen;
	OpenGL::TiledSurface *_overlayBackground;
	OpenGL::TiledSurface *_mouseSurface;
	OpenGL::SurfaceRenderer *_surfaceRenderer;

	// FBO used to render games not supporting arbitary resolutions
	OpenGL::FrameBuffer *_frameBuffer;
	// FBO used as a backbuffer for Apple API
	GLuint _glFBO;
	// RBOs used for depth and stencil buffer only
	GLuint _glRBOs[2];
};

#endif
#endif
