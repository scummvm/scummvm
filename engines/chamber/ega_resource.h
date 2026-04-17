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

#ifndef CHAMBER_EGA_RESOURCE_H
#define CHAMBER_EGA_RESOURCE_H

#include "common/array.h"
#include "common/file.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Chamber {

// ---------------------------------------------------------------------------
// EgaSpriteResource
//
// Loads a .EGA sprite bank file and decodes each sprite into a CLUT8
// Graphics::Surface (w*4 wide × h pixels tall, 1 byte per pixel).
//
// On-disk format (reference: kult/resource.cpp, DumpEga_main.cpp):
//   4 bytes   — junk header (skip)
//   per sprite:
//     uint16 size   — total byte count for this record (including the 4-byte header)
//     byte   w      — width in 4-pixel units  → actual pixel width = w * 4
//     byte   h      — height in pixels
//     (size-4) bytes — packed 4-bpp pixel data, 2 pixels per byte:
//                       pixel_hi = (byte >> 4) & 0x0F
//                       pixel_lo =  byte       & 0x0F
// ---------------------------------------------------------------------------
class EgaSpriteResource {
public:
	EgaSpriteResource();
	~EgaSpriteResource();

	void appendFromFile(const char *filename);
	void appendFromStream(Common::SeekableReadStream &stream);

	Graphics::Surface *getSprite(uint index) const { return _sprites[index]; }
	uint getSpriteCount() const { return _sprites.size(); }

private:
	Common::Array<Graphics::Surface *> _sprites;
};

// Global EGA sprite banks (allocated in kult.cpp EGA init branch)
extern EgaSpriteResource *ega_sprit_res;  // SPRIT.EGA
extern EgaSpriteResource *ega_puzzl_res;  // PUZZL.EGA + PUZZ1.EGA
extern EgaSpriteResource *ega_perso_res;  // PERSO.EGA

} // End of namespace Chamber

#endif // CHAMBER_EGA_RESOURCE_H
