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

#include "ultima/ultima4/config.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imageloader.h"
#include "ultima/ultima4/imageloader_fmtowns.h"
#include "ultima/ultima4/imageloader_u4.h"
//#include "ultima/ultima4/lzw/u4decode.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

//ImageLoader *FMTOWNSImageLoader::instance_pic = ImageLoader::registerLoader(new FMTOWNSImageLoader(0), "image/fmtowns-pic"); Doesn't work so easily, different graphics format.
ImageLoader *FMTOWNSImageLoader::instance_tif = ImageLoader::registerLoader(new FMTOWNSImageLoader(510), "image/fmtowns-tif");

/**
 * Loads in an FM TOWNS files, which we assume is 16 bits.
 */
Image *FMTOWNSImageLoader::load(U4FILE *file, int width, int height, int bpp) {
    if (width == -1 || height == -1 || bpp == -1) {
          errorFatal("dimensions not set for fmtowns image");
    }

    ASSERT((bpp == 16) | (bpp == 4), "invalid bpp: %d", bpp);

    long rawLen = file->length() - offset;
    file->seek(offset,0);
    unsigned char *raw = (unsigned char *) malloc(rawLen);
    file->read(raw, 1, rawLen);

    long requiredLength = (width * height * bpp / 8);
    if (rawLen < requiredLength) {
        if (raw)
            free(raw);
        errorWarning("FMTOWNS Image of size %d does not fit anticipated size %d", rawLen, requiredLength);
        return NULL;
    }

    Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
    if (!image) {
        if (raw)
            free(raw);
        return NULL;
    }

    if (bpp == 4)
    {
    	U4PaletteLoader pal;
    	image->setPalette(pal.loadEgaPalette(), 16);
    	setFromRawData(image, width, height, bpp, raw);
//    	if (width % 2)
//    		errorFatal("FMTOWNS 4bit images cannot handle widths not divisible by 2!");
//    	unsigned char nibble_mask = 0x0F;
//        for (int y = 0; y < height; y++)
//        {
//            for (int x = 0; x < width; x+=2)
//            {
//            	int byte = raw[(y * width + x) / 2];
//            	image->putPixelIndex(x  ,y,(byte & nibble_mask)  << 4);
//            	image->putPixelIndex(x+1,y,(byte			  )  	 );
//            }
//        }
    }


    if (bpp == 16)
    {

    //The FM towns uses 16 bits for graphics. I'm assuming 5R 5G 5B and 1 Misc bit.
    //Please excuse my ugly byte manipulation code

    //Masks
    //------------------------	//  0000000011111111	--Byte 0 and 1
    //------------------------	//	RRRRRGGGGGBBBBB?
    unsigned char low5 = 0x1F;	//  11111000--------	low5
    unsigned char high6 = ~3;	//	--------00111111	high6
    unsigned char high3 = ~31;	//	00000111--------	high3
    unsigned char low2 = 3;		//	--------11000000	low2
    unsigned char lastbit=128;	//	--------00000001	low2
    // Warning, this diagram is left-to-right, not standard right-to-left

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
        {
        	unsigned char byte0 = raw[(y * width + x) * 2];
        	unsigned char byte1 = raw[(y * width + x) * 2 + 1];

        	int r = (byte0 & low5);
        	r <<= 3;

        	int g = (byte0 & high3) >> 5;
        	g |= ((byte1 & low2) << 3);
        	g <<=3;

        	int b = byte1 & high6;
        	b <<= 1;




        	image->putPixel(x, y,
                            g,b,r,
                            lastbit & byte1 ? IM_TRANSPARENT : IM_OPAQUE);
        }
    }
    }


    free(raw);

    return image;
}

} // End of namespace Ultima4
} // End of namespace Ultima
