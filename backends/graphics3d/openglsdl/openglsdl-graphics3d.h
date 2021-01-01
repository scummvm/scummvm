/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS3D_OPENGLSDL_GRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_OPENGLSDL_GRAPHICS3D_H

#include "backends/graphics3d/sdl/sdl-graphics3d.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

namespace OpenGL {
	class FrameBuffer;
	class SurfaceRenderer;
	class TextureGL;
	class TiledSurface;
}

/**
 * SDL OpenGL based graphics manager
 *
 * Used when rendering games with OpenGL
 */
class OpenGLSdlGraphics3dManager : public SdlGraphics3dManager {
public:
	OpenGLSdlGraphics3dManager(SdlEventSource *eventSource, SdlWindow *window, bool supportsFrameBuffer);
	virtual ~OpenGLSdlGraphics3dManager();

	// GraphicsManager API - Features
	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual bool getFeatureState(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	virtual int getGraphicsMode() const override;

	virtual void beginGFXTransaction() override;
	virtual OSystem::TransactionError endGFXTransaction() override;

	// GraphicsManager API - Graphics mode
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override { return _overlayFormat; }
#endif
	virtual int getScreenChangeID() const override { return _screenChangeCount; }
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format) override;
	virtual int16 getHeight() const override;
	virtual int16 getWidth() const override;

	// GraphicsManager API - Draw methods
	virtual void updateScreen() override;

	// GraphicsManager API - Overlay
	virtual void showOverlay() override;
	virtual void hideOverlay() override;
	virtual Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }
	virtual void clearOverlay() override;
	virtual void grabOverlay(void *buf, int pitch) const override;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayWidth() const override;
	int16 getOverlayHeight() const override;
	virtual bool isOverlayVisible() const override { return _overlayVisible; }

	// GraphicsManager API - Mouse
	virtual void warpMouse(int x, int y) override;

	// SdlGraphicsManager API
	virtual void transformMouseCoordinates(Common::Point &point) override;

	void notifyResize(const int width, const int height) override;

protected:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GLContext _glContext;
	void deinitializeRenderer();
#endif

	bool _supportsFrameBuffer;

	Math::Rect2d _gameRect;

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

	/** Compute the size and position of the game rectangle in the screen */
	Math::Rect2d computeGameRect(bool renderToFrameBuffer, uint gameWidth, uint gameHeight,
	                             uint screenWidth, uint screenHeight);

	virtual bool saveScreenshot(const Common::String &filename) const override;

	uint _engineRequestedWidth, _engineRequestedHeight;

	int _screenChangeCount;
	int _antialiasing;
	bool _vsync;
	bool _fullscreen;
	bool _lockAspectRatio;
	bool _overlayVisible;

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

	bool isVSyncEnabled() const;

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

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#endif
