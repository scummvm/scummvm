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
 */

#include "graphics/thumbnail.h"
#include "graphics/scaler.h"
#include "graphics/colormasks.h"
#include "common/endian.h"
#include "common/algorithm.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Graphics {

namespace {
#define THMB_VERSION 1

struct ThumbnailHeader {
	uint32 type;
	uint32 size;
	byte version;
	uint16 width, height;
	byte bpp;
};

#define ThumbnailHeaderSize (4+4+1+2+2+1)

bool loadHeader(Common::SeekableReadStream &in, ThumbnailHeader &header, bool outputWarnings) {
	header.type = in.readUint32BE();
	// We also accept the bad 'BMHT' header here, for the sake of compatibility
	// with some older savegames which were written incorrectly due to a bug in
	// ScummVM which wrote the thumb header type incorrectly on LE systems.
	if (header.type != MKTAG('T','H','M','B') && header.type != MKTAG('B','M','H','T')) {
		if (outputWarnings)
			warning("couldn't find thumbnail header type");
		return false;
	}

	header.size = in.readUint32BE();
	header.version = in.readByte();

	if (header.version > THMB_VERSION) {
		if (outputWarnings)
			warning("trying to load a newer thumbnail version: %d instead of %d", header.version, THMB_VERSION);
		return false;
	}

	header.width = in.readUint16BE();
	header.height = in.readUint16BE();
	header.bpp = in.readByte();

	return true;
}
} // end of anonymous namespace

bool checkThumbnailHeader(Common::SeekableReadStream &in) {
	uint32 position = in.pos();
	ThumbnailHeader header;

	bool hasHeader = loadHeader(in, header, false);

	in.seek(position, SEEK_SET);

	return hasHeader;
}

bool skipThumbnail(Common::SeekableReadStream &in) {
	uint32 position = in.pos();
	ThumbnailHeader header;

	if (!loadHeader(in, header, false)) {
		in.seek(position, SEEK_SET);
		return false;
	}

	in.seek(header.size - (in.pos() - position), SEEK_CUR);
	return true;
}

Graphics::Surface *loadThumbnail(Common::SeekableReadStream &in) {
	ThumbnailHeader header;

	if (!loadHeader(in, header, true))
		return 0;

	if ((header.bpp == 1) || (header.bpp == 3)) {
		warning("trying to load thumbnail with unsupported bit depth %d", header.bpp);
		return 0;
	}

	Graphics::PixelFormat format = g_system->getOverlayFormat();
	Graphics::Surface *const to = new Graphics::Surface();
	to->create(header.width, header.height, format);
	OverlayColor *pixels2Bpp = (OverlayColor *)to->pixels;
	uint32 *pixels4Bpp = (uint32 *)to->pixels;
	for (int y = 0; y < to->h; ++y) {
		for (int x = 0; x < to->w; ++x) {
			uint8 a = 0xFF;
			uint8 r = 0, g = 0, b = 0;
			switch (header.bpp) {
			case 2:
				colorToRGB<ColorMasks<565> >(in.readUint16BE(), r, g, b);
				break;
			case 4:
				colorToARGB<ColorMasks<8888> >(in.readUint32BE(), a, r, g, b);
				break;
			}
			// converting to current OSystem Color
			switch (format.bytesPerPixel) {
			case 2:
				*pixels2Bpp++ = format.RGBToColor(r, g, b);
				break;
			case 4:
				*pixels4Bpp++ = format.ARGBToColor(a, r, g, b);
				break;
			}
		}
	}
	return to;
}

bool saveThumbnail(Common::WriteStream &out) {
	Graphics::Surface thumb;

	if (!createThumbnailFromScreen(&thumb)) {
		warning("Couldn't create thumbnail from screen, aborting thumbnail save");
		return false;
	}

	bool success = saveThumbnail(out, thumb);
	thumb.free();

	return success;
}

bool saveThumbnail(Common::WriteStream &out, const Graphics::Surface &thumb) {
	ThumbnailHeader header;
	header.type = MKTAG('T','H','M','B');
	header.size = ThumbnailHeaderSize + thumb.w*thumb.h*thumb.format.bytesPerPixel;
	header.version = THMB_VERSION;
	header.width = thumb.w;
	header.height = thumb.h;
	header.bpp = thumb.format.bytesPerPixel;

	out.writeUint32BE(header.type);
	out.writeUint32BE(header.size);
	out.writeByte(header.version);
	out.writeUint16BE(header.width);
	out.writeUint16BE(header.height);
	out.writeByte(header.bpp);

	uint16 *pixels16;
	uint32 *pixels32;
	switch (thumb.format.bytesPerPixel) {
	case 2:
		pixels16 = (uint16 *)thumb.pixels;
		for (uint32 p = 0; p < (uint32)thumb.w * thumb.h; ++p, ++pixels16) {
			out.writeUint16BE(*pixels16);
		}
		break;
	case 4:
		pixels32 = (uint32 *)thumb.pixels;
		for (uint32 p = 0; p < (uint32)thumb.w * thumb.h; ++p, ++pixels32) {
			out.writeUint32BE(*pixels32);
		}
		break;
	}

	return true;
}


/**
 * Returns an array indicating which pixels of a source image horizontally or vertically get
 * included in a scaled image
 */
int *scaleLine(int size, int srcSize) {
	int scale = 100 * size / srcSize;
	assert(scale > 0);
	int *v = new int[size];
	Common::fill(v, &v[size], 0);

	int distCtr = 0;
	int *destP = v;
	for (int distIndex = 0; distIndex < srcSize; ++distIndex) {
		distCtr += scale;
		while (distCtr >= 100) {
			assert(destP < &v[size]);
			*destP++ = distIndex;
			distCtr -= 100;
		}
	}

	return v;
}

Graphics::Surface *scale(const Graphics::Surface &srcImage, int xSize, int ySize) {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(xSize, ySize, srcImage.format);

	int *horizUsage = scaleLine(xSize, srcImage.w);
	int *vertUsage = scaleLine(ySize, srcImage.h);

	// Loop to create scaled version
	for (int yp = 0; yp < ySize; ++yp) {
		const byte *srcP = (const byte *)srcImage.getBasePtr(0, vertUsage[yp]);
		byte *destP = (byte *)s->getBasePtr(0, yp);

		for (int xp = 0; xp < xSize; ++xp) {
			const byte *tempSrcP = srcP + (horizUsage[xp] * srcImage.format.bytesPerPixel);
			for (int byteCtr = 0; byteCtr < srcImage.format.bytesPerPixel; ++byteCtr) {
				*destP++ = *tempSrcP++;
			}
		}
	}

	// Delete arrays and return surface
	delete[] horizUsage;
	delete[] vertUsage;
	return s;
}

} // End of namespace Graphics
