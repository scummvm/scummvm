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

#ifndef ULTIMA8_GFX_PALETTEMANAGER_H
#define ULTIMA8_GFX_PALETTEMANAGER_H

#include "ultima/shared/std/containers.h"
#include "graphics/pixelformat.h"
#include "ultima/ultima8/gfx/pal_transforms.h"

namespace Ultima {
namespace Ultima8 {

class Palette;

class PaletteManager {
public:
	explicit PaletteManager(const Graphics::PixelFormat &format);
	~PaletteManager();

	static PaletteManager *get_instance() {
		return _paletteManager;
	}

	enum PalIndex {
		Pal_Game = 0,
		Pal_Movie = 1,
		Pal_Diff = 2,	// Crusaders only - difficulty screen??
		Pal_Misc = 3,	// Crusaders only - game menu
		Pal_Misc2 = 4,	// Crusaders only - ??
		Pal_Star = 5,	// Crusaders only - ??
		Pal_Cred = 6,	// Crusader: No regret only (but mentioned in the no remorse exe!)
		Pal_JPFontStart = 16
	};

	void load(PalIndex index, Common::ReadStream &rs, Common::ReadStream &xformrs);
	void load(PalIndex index, Common::ReadStream &rs);
	Palette *getPalette(PalIndex index);
	uint getNumPalettes() const { return _palettes.size(); }

	void duplicate(PalIndex src, PalIndex dest);

	//! Re-convert a palette to native format after modifying it. If maxindex is set,
	//! only recalculate color indexes up to that value.
	void updatedPalette(PalIndex index, int maxindex = 0);

	//! Apply a transform matrix to a palette (-4.11 fixed)
	void transformPalette(PalIndex index, const int16 matrix[12]);

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

	//! Change the pixel format used by the PaletteManager
	void PixelFormatChanged(const Graphics::PixelFormat &format);

	//! Reset the Palette Manager
	void reset();

	//! Reset all the transforms back to default
	void resetTransforms();
	bool loadTransforms(Common::ReadStream &rs);
	void saveTransforms(Common::WriteStream &ws);

private:
	Std::vector<Palette *> _palettes;
	Graphics::PixelFormat _format;

	static PaletteManager *_paletteManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
