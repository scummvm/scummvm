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

#ifndef BACKENDS_GRAPHICS_RESVM_SURFACESDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_RESVM_SURFACESDL_GRAPHICS_H

#include "backends/graphics/resvm-sdl/resvm-sdl-graphics.h"

/**
 * SDL Surface based graphics manager
 *
 * Used when rendering the launcher, or games with TinyGL
 */
class ResVmSurfaceSdlGraphicsManager : public ResVmSdlGraphicsManager {
public:
	ResVmSurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~ResVmSurfaceSdlGraphicsManager();

	// GraphicsManager API - Features
	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) const override;

	// GraphicsManager API - Graphics mode
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
#endif
	virtual int getScreenChangeID() const override { return _screenChangeCount; }
	virtual void setupScreen(uint gameWidth, uint gameHeight, bool fullscreen, bool accel3d) override;
	virtual Graphics::PixelBuffer getScreenPixelBuffer() override;
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
	virtual int16 getOverlayWidth() const override { return _overlayscreen->w; }
	virtual int16 getOverlayHeight() const override { return _overlayscreen->h; }
	virtual bool isOverlayVisible() const override { return _overlayVisible; }

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
	SDL_Renderer *_renderer;
	SDL_Texture *_screenTexture;
	void deinitializeRenderer();
	SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
#endif

	SDL_Surface *_screen;
	SDL_Surface *_subScreen;
	void createOrUpdateScreen();

	SDL_Surface *_overlayscreen;
	bool _overlayDirty;
	bool _overlayVisible;

	Graphics::PixelFormat _overlayFormat;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
#endif
	uint _engineRequestedWidth, _engineRequestedHeight;

	bool _fullscreen;
	bool _lockAspectRatio;
	int _screenChangeCount;

	Math::Rect2d _gameRect;

	SDL_Surface *_sideSurfaces[2];

	void drawOverlay();
	void drawSideTextures();
	void closeOverlay();

	virtual bool saveScreenshot(const Common::String &filename) const override;

	virtual int getGraphicsModeScale(int mode) const override { return 1; }
};

#endif
