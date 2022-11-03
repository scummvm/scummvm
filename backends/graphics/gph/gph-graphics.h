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

#ifndef BACKENDS_GRAPHICS_GPH_H
#define BACKENDS_GRAPHICS_GPH_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"

class GPHGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	GPHGraphicsManager(SdlEventSource *boss, SdlWindow *window);

	void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	bool loadGFXMode() override;

protected:
	void setupHardwareSize() override;
};

#endif /* BACKENDS_GRAPHICS_GPH_H */
