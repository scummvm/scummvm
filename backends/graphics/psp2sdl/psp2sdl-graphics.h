/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef BACKENDS_GRAPHICS_PSP2SDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_PSP2SDL_GRAPHICS_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include <vita2d_fbo.h>

class PSP2SdlGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	PSP2SdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~PSP2SdlGraphicsManager();

	virtual OSystem::TransactionError endGFXTransaction() override;
	virtual const OSystem::GraphicsMode *getSupportedShaders() const override;
	virtual int getDefaultShader() const override;
	virtual int getShader() const override;
	virtual bool setShader(int id) override;

protected:
	virtual void setGraphicsModeIntern() override;
	virtual void unloadGFXMode() override;
	virtual bool hotswapGFXMode() override;

	virtual SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) override;
	virtual void SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) override;

private:
	void PSP2_UpdateFiltering();
	void updateShader();

	int _currentShader;
	int _numShaders;

	vita2d_texture *_vitatex_hwscreen;
	void *_sdlpixels_hwscreen;
	vita2d_shader *_shaders[6];
};

#endif
