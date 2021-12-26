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

#ifndef AGS_ENGINE_GFX_GFX_DEFINES_H
#define AGS_ENGINE_GFX_GFX_DEFINES_H

#include "ags/shared/core/types.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

// TODO: find the way to merge this with sprite batch transform
enum GlobalFlipType {
	kFlip_None,
	kFlip_Horizontal, // this means - mirror over horizontal middle line
	kFlip_Vertical,   // this means - mirror over vertical middle line
	kFlip_Both
};

// GraphicResolution struct determines image size and color depth
struct GraphicResolution : Size {
	int32_t ColorDepth; // color depth in bits per pixel

	GraphicResolution()
		: ColorDepth(0) {
	}

	GraphicResolution(int32_t width, int32_t height, int32_t color_depth)
		: Size(width, height), ColorDepth(color_depth) {
	}

	GraphicResolution(Size size, int32_t color_depth)
		: Size(size), ColorDepth(color_depth) {
	}

	inline bool IsValid() const {
		return Width > 0 && Height > 0 && ColorDepth > 0;
	}
};

// DisplayMode struct provides extended description of display mode
struct DisplayMode : public GraphicResolution {
	int32_t RefreshRate;
	bool    Vsync;
	bool    Windowed;

	DisplayMode()
		: RefreshRate(0)
		, Vsync(false)
		, Windowed(false) {
	}

	DisplayMode(const GraphicResolution &res, bool windowed = false, int32_t refresh = 0, bool vsync = false)
		: GraphicResolution(res)
		, RefreshRate(refresh)
		, Vsync(vsync)
		, Windowed(windowed) {
	}
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
