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

#ifndef BACKENDS_GRAPHICS_SDL_LINUXMOTO_H
#define BACKENDS_GRAPHICS_SDL_LINUXMOTO_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"

class LinuxmotoSdlGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	LinuxmotoSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);

	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	virtual void setGraphicsModeIntern() override;
	virtual int getGraphicsModeScale(int mode) const override;
	virtual void internUpdateScreen() override;
	virtual ScalerProc *getGraphicsScalerProc(int mode) const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool loadGFXMode() override;
	virtual void drawMouse() override;
	virtual void undrawMouse() override;
	virtual void showOverlay() override;
	virtual void hideOverlay() override;
	virtual void warpMouse(int x, int y) override;

	virtual void transformMouseCoordinates(Common::Point &point);
};

#endif
