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

#ifndef BACKENDS_GRAPHICS_GPH_H
#define BACKENDS_GRAPHICS_GPH_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "graphics/scaler/aspect.h" // for aspect2Real
#include "graphics/scaler/downscaler.h"

enum {
	GFX_HALF = 12
};

class GPHGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	GPHGraphicsManager(SdlEventSource *boss, SdlWindow *window);

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const;
	int getDefaultGraphicsMode() const override;

	void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getGraphicsModeScale(int mode) const override;
	ScalerProc *getGraphicsScalerProc(int mode) const override;
	void internUpdateScreen() override;
	void showOverlay() override;
	void hideOverlay() override;
	bool loadGFXMode() override;
	void drawMouse() override;
	void undrawMouse() override;
	void warpMouse(int x, int y) override;

	virtual void transformMouseCoordinates(Common::Point &point);
};

#endif /* BACKENDS_GRAPHICS_GPH_H */
