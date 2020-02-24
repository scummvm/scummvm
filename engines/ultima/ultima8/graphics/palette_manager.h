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

#ifndef ULTIMA8_GRAPHICS_PALETTEMANAGER_H
#define ULTIMA8_GRAPHICS_PALETTEMANAGER_H

#include "ultima/ultima8/graphics/palette.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

class IDataSource;
class RenderSurface;

class PaletteManager {
public:
	explicit PaletteManager(RenderSurface *rs);
	~PaletteManager();

	static PaletteManager *get_instance() {
		return _paletteManager;
	}

	enum PalIndex {
		Pal_Game = 0,
		Pal_Movie = 1,
		Pal_JPFontStart = 16
	};

	void load(PalIndex index, IDataSource &ds, IDataSource &xformds);
	void load(PalIndex index, IDataSource &ds);
	Palette *getPalette(PalIndex index);

	void duplicate(PalIndex src, PalIndex dest);

	//! Re-convert a palette to native format after modifying it
	void updatedFont(PalIndex index);

	//! Apply a transform matrix to a palette (-4.11 fixed)
	void transformPalette(PalIndex index, int16 matrix[12]);

	//! reset the transformation matrix of a palette
	void untransformPalette(PalIndex index);

	//! Get the current TransformMatrix for the given index
	bool getTransformMatrix(int16 matrix[12], PalIndex index);

	// Get a TransformMatrix from a PalTransforms value (-4.11 fixed)
	static void getTransformMatrix(int16 matrix[12],
	                               PalTransforms trans);

	// Create a custom Transform Matrix from RGBA col32. (-4.11 fixed)
	// Alpha will set how much of original palette to keep. 0 = keep none
	static void getTransformMatrix(int16 matrix[12], uint32 col32);

	//! Change the Render Surface used by the PaletteManager
	void RenderSurfaceChanged(RenderSurface *rs);

	//! Reset the Palette Manager
	void reset();

	//! Reset all the transforms back to default
	void resetTransforms();

private:
	Std::vector<Palette *> _palettes;
	RenderSurface *_renderSurface;

	static PaletteManager *_paletteManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
