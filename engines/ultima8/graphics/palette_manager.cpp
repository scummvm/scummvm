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

#include "ultima8/misc/pent_include.h"

#include "ultima8/graphics/palette_manager.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/graphics/texture.h"

namespace Ultima8 {

PaletteManager *PaletteManager::palettemanager = 0;

PaletteManager::PaletteManager(RenderSurface *rs)
	: rendersurface(rs) {
	con.Print(MM_INFO, "Creating PaletteManager...\n");

	assert(palettemanager == 0);
	palettemanager = this;
}

PaletteManager::~PaletteManager() {
	reset();
	con.Print(MM_INFO, "Destroying PaletteManager...\n");
	palettemanager = 0;
}

// Reset the Palette Manager
void PaletteManager::reset() {
	con.Print(MM_INFO, "Resetting PaletteManager...\n");

	for (unsigned int i = 0; i < palettes.size(); ++i)
		delete palettes[i];
	palettes.clear();
}

void PaletteManager::updatedFont(PalIndex index) {
	Pentagram::Palette *pal = getPalette(index);
	if (pal)
		rendersurface->CreateNativePalette(pal); // convert to native format
}

// Reset all the transforms back to default
void PaletteManager::resetTransforms() {
	con.Print(MM_INFO, "Resetting Palette Transforms...\n");

	int16 matrix[12];
	getTransformMatrix(matrix, Pentagram::Transform_None);

	for (unsigned int i = 0; i < palettes.size(); ++i) {
		Pentagram::Palette *pal = palettes[i];
		if (!pal) continue;
		pal->transform = Pentagram::Transform_None;
		for (int j = 0; j < 12; j++) pal->matrix[j] = matrix[j];
		rendersurface->CreateNativePalette(pal); // convert to native format
	}
}

// Change the Render Surface used by the PaletteManager
void PaletteManager::RenderSurfaceChanged(RenderSurface *rs) {
	rendersurface = rs;

	// Create native palettes for all currently loaded palettes
	for (unsigned int i = 0; i < palettes.size(); ++i)
		if (palettes[i])
			rendersurface->CreateNativePalette(palettes[i]);
}

void PaletteManager::load(PalIndex index, IDataSource &ds, IDataSource &xformds) {
	if (palettes.size() <= static_cast<unsigned int>(index))
		palettes.resize(index + 1);

	if (palettes[index])
		delete palettes[index];

	Pentagram::Palette *pal = new Pentagram::Palette;
	pal->load(ds, xformds);
	rendersurface->CreateNativePalette(pal); // convert to native format

	palettes[index] = pal;
}

void PaletteManager::load(PalIndex index, IDataSource &ds) {
	if (palettes.size() <= static_cast<unsigned int>(index))
		palettes.resize(index + 1);

	if (palettes[index])
		delete palettes[index];

	Pentagram::Palette *pal = new Pentagram::Palette;
	pal->load(ds);
	rendersurface->CreateNativePalette(pal); // convert to native format

	palettes[index] = pal;
}

void PaletteManager::duplicate(PalIndex src, PalIndex dest) {
	Pentagram::Palette *newpal = getPalette(dest);
	if (!newpal)
		newpal = new Pentagram::Palette;
	Pentagram::Palette *srcpal = getPalette(src);
	if (srcpal)
		*newpal = *srcpal;

	rendersurface->CreateNativePalette(newpal); // convert to native format
	if (palettes.size() <= static_cast<unsigned int>(dest))
		palettes.resize(dest + 1);
	palettes[dest] = newpal;
}

Pentagram::Palette *PaletteManager::getPalette(PalIndex index) {
	if (static_cast<unsigned int>(index) >= palettes.size())
		return 0;

	return palettes[index];
}

void PaletteManager::transformPalette(PalIndex index, int16 matrix[12]) {
	Pentagram::Palette *pal = getPalette(index);

	if (!pal) return;

	for (int i = 0; i < 12; i++) pal->matrix[i] = matrix[i];
	rendersurface->CreateNativePalette(pal); // convert to native format
}

void PaletteManager::untransformPalette(PalIndex index) {
	Pentagram::Palette *pal = getPalette(index);

	if (!pal) return;

	pal->transform = Pentagram::Transform_None;
	int16 matrix[12];
	getTransformMatrix(matrix, Pentagram::Transform_None);
	transformPalette(index, matrix);
}

void PaletteManager::getTransformMatrix(int16 matrix[12], Pentagram::PalTransforms trans) {
	switch (trans) {
	// Normal untransformed palette
	case Pentagram::Transform_None: {
		matrix[0] = 0x800;
		matrix[1] = 0;
		matrix[2]  = 0;
		matrix[3]  = 0;
		matrix[4] = 0;
		matrix[5] = 0x800;
		matrix[6]  = 0;
		matrix[7]  = 0;
		matrix[8] = 0;
		matrix[9] = 0;
		matrix[10] = 0x800;
		matrix[11] = 0;
	}
	break;

	// O[i] = I[r]*0.375 + I[g]*0.5 + I[b]*0.125;
	case Pentagram::Transform_Greyscale: {
		for (int i = 0; i < 3; i++) {
			matrix[i * 4 + 0] = 0x0300;
			matrix[i * 4 + 1] = 0x0400;
			matrix[i * 4 + 2] = 0x0100;
			matrix[i * 4 + 3] = 0;
		}
	}
	break;

	// O[r] = 0;
	case Pentagram::Transform_NoRed: {
		matrix[0] = 0;
		matrix[1] = 0;
		matrix[2]  = 0;
		matrix[3]  = 0;
		matrix[4] = 0;
		matrix[5] = 0x800;
		matrix[6]  = 0;
		matrix[7]  = 0;
		matrix[8] = 0;
		matrix[9] = 0;
		matrix[10] = 0x800;
		matrix[11] = 0;
	}
	break;

	// O[i] = (I[i] + Grey)*0.25 + 0.1875;
	case Pentagram::Transform_RainStorm: {
		for (int i = 0; i < 3; i++) {
			matrix[i * 4 + 0] = (0x0300 * 0x0200) >> 11;
			matrix[i * 4 + 1] = (0x0400 * 0x0200) >> 11;
			matrix[i * 4 + 2] = (0x0100 * 0x0200) >> 11;

			matrix[i * 4 + i] += 0x0200;

			matrix[i * 4 + 3] = 0x0180;
		}
	}
	break;

	// O[r] = I[r]*0.5 + Grey*0.5  + 0.1875;
	// O[g] = I[g]*0.5 + Grey*0.25;
	// O[b] = I[b]*0.5;
	case Pentagram::Transform_FireStorm: {
		// O[r] = I[r]*0.5 + Grey*0.5 + 0.1875;
		matrix[0] = ((0x0300 * 0x0400) >> 11) + 0x0400;
		matrix[1] = (0x0400 * 0x0400) >> 11;
		matrix[2] = (0x0100 * 0x0400) >> 11;
		matrix[3]  = 0x0180;

		// O[g] = I[g]*0.5 + Grey*0.25;
		matrix[4] = (0x0300 * 0x0200) >> 11;
		matrix[5] = ((0x0400 * 0x0200) >> 11) + 0x0400;
		matrix[6] = (0x0100 * 0x0200) >> 11;
		matrix[7]  = 0;

		// O[b] = I[b]*0.5;
		matrix[8]  = 0;
		matrix[9]  = 0;
		matrix[10] = 0x0400;
		matrix[11] = 0;
	}
	break;

	// O[i] = I[i]*2 -Grey;
	case Pentagram::Transform_Saturate: {
		for (int i = 0; i < 3; i++) {
			matrix[i * 4 + 0] = -0x0300;
			matrix[i * 4 + 1] = -0x0400;
			matrix[i * 4 + 2] = -0x0100;
			matrix[i * 4 + 3] = 0;
			matrix[i * 4 + i] += 0x1000;
		}
	}
	break;

	// O[b] = I[r]; O[r] = I[g]; O[g] = I[b];
	case Pentagram::Transform_BRG: {
		matrix[0] = 0;
		matrix[1] = 0x800;
		matrix[2]  = 0;
		matrix[3]  = 0;
		matrix[4] = 0;
		matrix[5] = 0;
		matrix[6]  = 0x800;
		matrix[7]  = 0;
		matrix[8] = 0x800;
		matrix[9] = 0;
		matrix[10] = 0;
		matrix[11] = 0;
	}
	break;

	// O[g] = I[r]; O[b] = I[g]; O[r] = I[b];
	case Pentagram::Transform_GBR: {
		matrix[0] = 0;
		matrix[1] = 0;
		matrix[2]  = 0x800;
		matrix[3]  = 0;
		matrix[4] = 0x800;
		matrix[5] = 0;
		matrix[6]  = 0;
		matrix[7]  = 0;
		matrix[8] = 0;
		matrix[9] = 0x800;
		matrix[10] = 0;
		matrix[11] = 0;
	}
	break;

	// Unknown
	default: {
		perr << "Unknown Palette Transformation: " << trans << std::endl;
		matrix[0] = 0x800;
		matrix[1] = 0;
		matrix[2]  = 0;
		matrix[3]  = 0;
		matrix[4] = 0;
		matrix[5] = 0x800;
		matrix[6]  = 0;
		matrix[7]  = 0;
		matrix[8] = 0;
		matrix[9] = 0;
		matrix[10] = 0x800;
		matrix[11] = 0;
	}
	break;
	}
}


void PaletteManager::getTransformMatrix(int16 matrix[12], uint32 col32) {
	matrix[0]  = (static_cast<int32>(TEX32_A(col32)) * 0x800) / 255;
	matrix[1]  = 0;
	matrix[2]  = 0;
	matrix[3]  = (static_cast<int32>(TEX32_R(col32)) * 0x800) / 255;

	matrix[4]  = 0;
	matrix[5]  = (static_cast<int32>(TEX32_A(col32)) * 0x800) / 255;
	matrix[6]  = 0;
	matrix[7]  = (static_cast<int32>(TEX32_G(col32)) * 0x800) / 255;

	matrix[8]  = 0;
	matrix[9]  = 0;
	matrix[10] = (static_cast<int32>(TEX32_A(col32)) * 0x800) / 255;
	matrix[11] = (static_cast<int32>(TEX32_B(col32)) * 0x800) / 255;
}

} // End of namespace Ultima8
