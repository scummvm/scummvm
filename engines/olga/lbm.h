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

#ifndef OLGA_LBM_H
#define OLGA_LBM_H

#include "common/array.h"
#include "common/path.h"
#include "common/ptr.h"
#include "graphics/managed_surface.h"

namespace Olga {

/** One picture of an LBM file: 8-bit pixels, with its own key colour. */
struct LbmImage {
	Graphics::ManagedSurface surface;
	bool keyed = false; // BMHD masking 2: the key colour is transparent
	byte key = 0;       // BMHD transparentColor, differs per image
};

/**
 * Deluxe Paint PBM files as the game uses them: a plain FORM PBM (one
 * 320x200 picture) or a LIST PBM of a PROP (shared CMAP, or empty) and
 * one FORM per image. Images are numbered from 1 as in the game: in a room
 * file image 1 is the 320x138 background, the rest are sprites.
 */
class LbmFile {
public:
	bool load(const Common::Path &name);

	uint size() const { return _images.size(); }
	/** Image n, 1-based. */
	const LbmImage &image(uint n) const { return *_images[n - 1]; }

	bool hasPalette() const { return _hasPalette; }
	/** 256 colours, 8-bit RGB as in the CMAP chunk. */
	const byte *palette() const { return _palette; }

private:
	bool readForm(const byte *data, uint32 start, uint32 end);

	Common::Array<Common::SharedPtr<LbmImage> > _images;
	byte _palette[256 * 3];
	bool _hasPalette = false;
};

} // End of namespace Olga

#endif // OLGA_LBM_H
