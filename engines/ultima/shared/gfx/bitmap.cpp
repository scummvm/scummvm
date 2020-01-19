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

#include "ultima/shared/gfx/bitmap.h"
#include "ultima/shared/core/file.h"
#include "common/memstream.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

void Bitmap::load(const Common::String &filename) {
	File srcFile(filename);
	Common::MemoryWriteStreamDynamic decompressedFile(DisposeAfterUse::YES);
	decompress(&srcFile, &decompressedFile);

	// Set the bitmap size
	Common::MemoryReadStream f(decompressedFile.getData(), decompressedFile.size());
	int16 xs = f.readSint16LE();
	int16 ys = f.readSint16LE();
	create(xs, ys);
	assert(f.size() == (xs * ys + 4));

	Graphics::Surface s = getSubArea(Common::Rect(0, 0, xs, ys));

	// Read in the lines
	for (int y = 0; y < ys; ++y) {
		byte *dest = (byte *)s.getBasePtr(0, y);
		f.read(dest, xs);
	}
}

void Bitmap::flipHorizontally() {
	Graphics::Surface s = getSubArea(Common::Rect(0, 0, this->w, this->h));
	
	for (int y = 0; y < h; ++y) {
		byte *lineStart = (byte *)s.getBasePtr(0, y);
		byte *lineEnd = (byte *)s.getBasePtr(this->w - 1, y);

		for (int x = 0; x < (this->w - 1) / 2; ++x, ++lineStart, --lineEnd)
			SWAP(*lineStart, *lineEnd);
	}
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
