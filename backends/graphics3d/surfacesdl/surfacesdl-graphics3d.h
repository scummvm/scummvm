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

#ifndef BACKENDS_GRAPHICS3D_SURFACESDL_GRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_SURFACESDL_GRAPHICS3D_H

#include "backends/graphics3d/sdl/sdl-graphics3d.h"

/**
 * SDL Surface based graphics manager
 *
 * Used when rendering the launcher, or games with TinyGL
 */
class SurfaceSdlGraphics3dManager : public SdlGraphics3dManager {
public:
	SurfaceSdlGraphics3dManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~SurfaceSdlGraphics3dManager();

	// GraphicsManager API - Features
	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) const override;

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	virtual int getGraphicsMode() const override;

	virtual void beginGFXTransaction() override;
	virtual OSystem::TransactionError endGFXTransaction() override;

	// GraphicsManager API - Graphics mode
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
#endif
	virtual int getScreenChangeID() const override { return _screenChangeCount; }
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format) override;
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
	void setupScreen();

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

	void drawOverlay();
	void closeOverlay();

	virtual bool saveScreenshot(const Common::String &filename) const override;

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

#endif
