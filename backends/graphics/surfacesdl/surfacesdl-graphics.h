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

#ifndef BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H

#include "backends/graphics/graphics.h"
#include "backends/graphics/sdl/resvm-sdl-graphics.h"
#include "graphics/pixelformat.h"
#include "graphics/scaler.h"
#include "common/array.h"
#include "common/events.h"
#include "common/system.h"
#include "math/rect2d.h"

#include "backends/events/sdl/sdl-events.h"

#include "backends/platform/sdl/sdl-sys.h"

/**
 * SDL graphics manager
 */
class SurfaceSdlGraphicsManager : public ResVmSdlGraphicsManager {
public:
	SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~SurfaceSdlGraphicsManager();

	virtual bool hasFeature(OSystem::Feature f);

	virtual void setupScreen(uint gameWidth, uint gameHeight, bool fullscreen, bool accel3d); // ResidualVM specific method
	virtual Graphics::PixelBuffer getScreenPixelBuffer(); // ResidualVM specific method

	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual void updateScreen();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);

	void closeOverlay(); // ResidualVM specific method

	/* Render the passed Surfaces besides the game texture.
	 * This is used for widescreen support in the Grim engine.
	 * Note: we must copy the Surfaces, as they are free()d after this call.
	 */
	virtual void suggestSideTextures(Graphics::Surface *left, Graphics::Surface *right) override;

	virtual void warpMouse(int x, int y);


	// SdlGraphicsManager interface
	virtual void transformMouseCoordinates(Common::Point &point);

protected:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Renderer *_renderer;
	SDL_Texture *_screenTexture;
	void deinitializeRenderer();
	SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
#endif

	SDL_Surface *_screen;
	SDL_Surface *_subScreen;

	// overlay
	SDL_Surface *_overlayscreen;
	bool _overlayDirty;

	Math::Rect2d _gameRect;

	SDL_Surface *_sideSurfaces[2];

	void drawOverlay();
	void drawSideTextures();
};

#endif
