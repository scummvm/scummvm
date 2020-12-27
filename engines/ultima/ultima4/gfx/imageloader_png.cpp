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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imageloader_png.h"
#include "common/stream.h"
#include "image/png.h"

namespace Ultima {
namespace Ultima4 {

Image *PngImageLoader::load(Common::SeekableReadStream &stream, int width, int height, int bpp) {
	if (width != -1 || height != -1 || bpp != -1) {
		warning("dimensions set for PNG image, will be ignored");
	}

	::Image::PNGDecoder decoder;
	if (!decoder.loadStream(stream))
		return nullptr;

	const Graphics::Surface *img = decoder.getSurface();
	bpp = img->format.bpp();
	if (img->format.bytesPerPixel == 1)
		bpp = decoder.getPaletteColorCount() == 256 ? 8 : 4;

	Image *image = Image::create(img->w, img->h, bpp == 4 || bpp == 8, Image::HARDWARE);
	image->blitFrom(*img);

	if (img->format.bytesPerPixel == 1) {
		int palCount = decoder.getPaletteColorCount();
		const byte *pal = decoder.getPalette();

		RGBA *palette = new RGBA[palCount];
		for (int c = 0; c < palCount; ++c, pal += 3) {
			palette[c].r = pal[0];
			palette[c].g = pal[1];
			palette[c].b = pal[2];
			palette[c].a = IM_OPAQUE;
		}

		image->setPalette(palette, palCount);
		delete[] palette;
	}

	return image;
}

} // End of namespace Ultima4
} // End of namespace Ultima
