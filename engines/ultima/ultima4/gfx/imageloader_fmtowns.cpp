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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imageloader_fmtowns.h"
#include "ultima/ultima4/gfx/imageloader_u4.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {

Image *FMTOWNSImageLoader::load(Common::SeekableReadStream &stream, int width, int height, int bpp) {
	if (width == -1 || height == -1 || bpp == -1) {
		error("dimensions not set for fmtowns image");
	}

	assertMsg((bpp == 16) | (bpp == 4), "invalid bpp: %d", bpp);

	int rawLen = stream.size() - _offset;
	stream.seek(_offset, 0);
	byte *raw = (byte *) malloc(rawLen);
	stream.read(raw, rawLen);

	int requiredLength = (width * height * bpp / 8);
	if (rawLen < requiredLength) {
		if (raw)
			free(raw);
		warning("FMTOWNS Image of size %d does not fit anticipated size %d", rawLen, requiredLength);
		return nullptr;
	}

	Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
	if (!image) {
		if (raw)
			free(raw);
		return nullptr;
	}

	if (bpp == 4) {
		U4PaletteLoader pal;
		image->setPalette(pal.loadEgaPalette(), 16);
		setFromRawData(image, width, height, bpp, raw);
//      if (width % 2)
//          error("FMTOWNS 4bit images cannot handle widths not divisible by 2!");
//      byte nibble_mask = 0x0F;
//        for (int y = 0; y < height; y++)
// {
//            for (int x = 0; x < width; x+=2)
// {
//              int byte = raw[(y * width + x) / 2];
//              image->putPixelIndex(x  ,y,(byte & nibble_mask)  << 4);
//              image->putPixelIndex(x+1,y,(byte              )      );
//            }
//        }
	}


	if (bpp == 16) {

		//The FM towns uses 16 bits for graphics. I'm assuming 5R 5G 5B and 1 Misc bit.
		//Please excuse my ugly byte manipulation code

		//Masks
		//------------------------  //  0000000011111111    --Byte 0 and 1
		//------------------------  //  RRRRRGGGGGBBBBB?
		byte low5 = 0x1F;          //  11111000--------    low5
		byte high6 = (byte)~3U;    //  --------00111111    high6
		byte high3 = (byte)~31U;   //  00000111--------    high3
		byte low2 = 3;             //  --------11000000    low2
		byte lastbit = 128;        //  --------00000001    low2
		// Warning, this diagram is left-to-right, not standard right-to-left

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				byte byte0 = raw[(y * width + x) * 2];
				byte byte1 = raw[(y * width + x) * 2 + 1];

				int r = (byte0 & low5);
				r <<= 3;

				int g = (byte0 & high3) >> 5;
				g |= ((byte1 & low2) << 3);
				g <<= 3;

				int b = byte1 & high6;
				b <<= 1;

				// TODO: Previously r & b were reversed. See if this proper
				// order is correct, and if not properly swap value calculations
				image->putPixel(x, y, r, g, b,
					lastbit & byte1 ? IM_TRANSPARENT : IM_OPAQUE);
			}
		}
	}


	free(raw);

	return image;
}

} // End of namespace Ultima4
} // End of namespace Ultima
