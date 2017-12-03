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

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_OPENGLSDL_GRAPHICS_H

#include "backends/graphics/sdl/resvm-sdl-graphics.h"

namespace OpenGL {
	class FrameBuffer;
	class SurfaceRenderer;
	class Texture;
	class TiledSurface;
}

/**
 * SDL OpenGL based graphics manager
 *
 * Used when rendering games with OpenGL
 */
class OpenGLSdlGraphicsManager : public ResVmSdlGraphicsManager {
public:
	OpenGLSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window, const Capabilities &capabilities);
	virtual ~OpenGLSdlGraphicsManager();

	// GraphicsManager API - Features
	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual bool getFeatureState(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;

	// GraphicsManager API - Graphics mode
	virtual void setupScreen(uint gameWidth, uint gameHeight, bool fullscreen, bool accel3d) override;
	virtual Graphics::PixelBuffer getScreenPixelBuffer() override;
	virtual int16 getHeight() const override;
	virtual int16 getWidth() const override;

	// GraphicsManager API - Draw methods
	virtual void updateScreen();

	// GraphicsManager API - Overlay
	virtual void showOverlay() override;
	virtual void hideOverlay() override;
	virtual void clearOverlay() override;
	virtual void grabOverlay(void *buf, int pitch) const override;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;

	/* Render the passed Surfaces besides the game texture.
	 * This is used for widescreen support in the Grim engine.
	 * Note: we must copy the Surfaces, as they are free()d after this call.
	 */
	virtual void suggestSideTextures(Graphics::Surface *left, Graphics::Surface *right) override;

	// GraphicsManager API - Mouse
	virtual void warpMouse(int x, int y) override;

	// SdlGraphicsManager API
	virtual void transformMouseCoordinates(Common::Point &point) override;

protected:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GLContext _glContext;
	void deinitializeRenderer();
#endif

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
	bool createOrUpdateGLContext(uint effectiveWidth, uint effectiveHeight, GameRenderTarget gameRenderTarget);

	void createOrUpdateScreen();

	int _antialiasing;
	bool _vsync;

	OpenGL::TiledSurface *_overlayScreen;
	OpenGL::TiledSurface *_overlayBackground;
	OpenGL::Texture *_sideTextures[2];
	OpenGL::SurfaceRenderer *_surfaceRenderer;

	void initializeOpenGLContext() const;
	void drawOverlay();
	void drawSideTextures();
	void closeOverlay();

	OpenGL::FrameBuffer *_frameBuffer;
	OpenGL::FrameBuffer *createFramebuffer(uint width, uint height);

	bool isVSyncEnabled() const;
};

#endif
