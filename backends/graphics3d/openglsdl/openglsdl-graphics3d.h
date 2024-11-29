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

#ifndef BACKENDS_GRAPHICS3D_OPENGLSDL_GRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_OPENGLSDL_GRAPHICS3D_H

#include "backends/graphics/sdl/sdl-graphics.h"

#include "math/rect2d.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/context.h"

namespace OpenGL {
	class FrameBuffer;
	class SurfaceRenderer;
	class TiledSurface;
}

/**
 * SDL OpenGL based graphics manager
 *
 * Used when rendering games with OpenGL
 */
class OpenGLSdlGraphics3dManager : public SdlGraphicsManager {
public:
	OpenGLSdlGraphics3dManager(SdlEventSource *eventSource, SdlWindow *window, bool supportsFrameBuffer);
	virtual ~OpenGLSdlGraphics3dManager();

	// GraphicsManager API - Features
	bool hasFeature(OSystem::Feature f) const override;
	bool getFeatureState(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	int getGraphicsMode() const override;

	const OSystem::GraphicsMode *getSupportedStretchModes() const override;
	int getDefaultStretchMode() const override;
	bool setStretchMode(int mode) override;
	int getStretchMode() const override;

	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

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
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = NULL) override {}
	void setCursorPalette(const byte *colors, uint start, uint num) override {}

	// SdlGraphicsManager API
	void notifyVideoExpose() override {};
	void notifyResize(const int width, const int height) override;

	bool gameNeedsAspectRatioCorrection() const override;

	void showSystemMouseCursor(bool visible) override;

#if defined(USE_IMGUI) && SDL_VERSION_ATLEAST(2, 0, 0)
	void *getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) override;
	void freeImGuiTexture(void *texture) override;
#endif

protected:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	int _glContextProfileMask, _glContextMajor, _glContextMinor;
	SDL_GLContext _glContext;
	void deinitializeRenderer();
#endif

	OpenGL::ContextType _glContextType;

	bool _supportsFrameBuffer;

	struct OpenGLPixelFormat {
		uint bytesPerPixel;
		uint redSize;
		uint blueSize;
		uint greenSize;
		uint alphaSize;
		int multisampleSamples;

		OpenGLPixelFormat(uint screenBytesPerPixel, uint red, uint blue, uint green, uint alpha, int samples);
	};

	/**
	 * Initialize an OpenGL window matching as closely as possible the required properties
	 *
	 * When unable to create a context with anti-aliasing this tries without.
	 * When unable to create a context with the desired pixel depth this tries lower values.
	 */
	bool createOrUpdateGLContext(uint gameWidth, uint gameHeight, uint effectiveWidth, uint effectiveHeight,
	                             bool renderToFramebuffer, bool engineSupportsArbitraryResolutions);

	void createOrUpdateScreen();
	void setupScreen();

	void handleResizeImpl(const int width, const int height) override;

#ifdef EMSCRIPTEN
	/**
	 * See https://registry.khronos.org/webgl/specs/latest/1.0/#2 :
	 * " By default, after compositing the contents of the drawing buffer shall be cleared to their default values [...]
	 *   Techniques like synchronous drawing buffer access (e.g., calling readPixels or toDataURL in the same function
	 *   that renders to the drawing buffer) can be used to get the contents of the drawing buffer "
	 *
	 * This means we need to take the screenshot at the correct time, which we do by queueing taking the screenshot
	 * for the next frame instead of taking it right away.
	 */
	bool _queuedScreenshot = false;
	void saveScreenshot() override;
#endif

	bool saveScreenshot(const Common::Path &filename) const override;

	uint _engineRequestedWidth, _engineRequestedHeight;

	int _screenChangeCount;
	int _antialiasing;
	int _stretchMode;
	bool _vsync;
	bool _fullscreen;
	bool _lockAspectRatio;

	OpenGL::TiledSurface *_overlayScreen;
	OpenGL::TiledSurface *_overlayBackground;
	OpenGL::SurfaceRenderer *_surfaceRenderer;

	Graphics::PixelFormat _overlayFormat;

	void initializeOpenGLContext() const;
	void drawOverlay();
	void closeOverlay();

	OpenGL::FrameBuffer *_frameBuffer;
	OpenGL::FrameBuffer *createFramebuffer(uint width, uint height);
	bool shouldRenderToFramebuffer() const;

protected:

	enum TransactionMode {
		kTransactionNone = 0,
		kTransactionActive = 1,
		kTransactionRollback = 2
	};

	/**
	 * The current transaction mode.
	 */
	TransactionMode _transactionMode;
};

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#endif
