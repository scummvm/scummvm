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
#define THMB_VERSION 2

struct ThumbnailHeader {
	uint32 type;
	uint32 size;
	byte version;
	uint16 width, height;
	PixelFormat format;
};

#define ThumbnailHeaderSize (4+4+1+2+2+(1+4+4))

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
	header.format.bytesPerPixel = in.readByte();
	// Starting from version 2 on we serialize the whole PixelFormat.
	if (header.version >= 2) {
		header.format.rLoss = in.readByte();
		header.format.gLoss = in.readByte();
		header.format.bLoss = in.readByte();
		header.format.aLoss = in.readByte();

		header.format.rShift = in.readByte();
		header.format.gShift = in.readByte();
		header.format.bShift = in.readByte();
		header.format.aShift = in.readByte();
	} else {
		// Version 1 used a hardcoded RGB565.
		header.format = createPixelFormat<565>();
	}

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

	if (header.format.bytesPerPixel != 2 && header.format.bytesPerPixel != 4) {
		warning("trying to load thumbnail with unsupported bit depth %d", header.format.bytesPerPixel);
		return 0;
	}

	Graphics::Surface *const to = new Graphics::Surface();
	to->create(header.width, header.height, header.format);

	for (int y = 0; y < to->h; ++y) {
		switch (header.format.bytesPerPixel) {
		case 2: {
			uint16 *pixels = (uint16 *)to->getBasePtr(0, y);
			for (uint x = 0; x < to->w; ++x) {
				*pixels++ = in.readUint16BE();
			}
			} break;

		case 4: {
			uint32 *pixels = (uint32 *)to->getBasePtr(0, y);
			for (uint x = 0; x < to->w; ++x) {
				*pixels++ = in.readUint32BE();
			}
			} break;

		default:
			assert(0);
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
	if (thumb.format.bytesPerPixel != 2 && thumb.format.bytesPerPixel != 4) {
		warning("trying to save thumbnail with bpp %u", thumb.format.bytesPerPixel);
		return false;
	}

	ThumbnailHeader header;
	header.type = MKTAG('T','H','M','B');
	header.size = ThumbnailHeaderSize + thumb.w*thumb.h*thumb.format.bytesPerPixel;
	header.version = THMB_VERSION;
	header.width = thumb.w;
	header.height = thumb.h;

	out.writeUint32BE(header.type);
	out.writeUint32BE(header.size);
	out.writeByte(header.version);
	out.writeUint16BE(header.width);
	out.writeUint16BE(header.height);

	// Serialize the PixelFormat
	out.writeByte(thumb.format.bytesPerPixel);
	out.writeByte(thumb.format.rLoss);
	out.writeByte(thumb.format.gLoss);
	out.writeByte(thumb.format.bLoss);
	out.writeByte(thumb.format.aLoss);
	out.writeByte(thumb.format.rShift);
	out.writeByte(thumb.format.gShift);
	out.writeByte(thumb.format.bShift);
	out.writeByte(thumb.format.aShift);

	// Serialize the pixel data
	for (uint y = 0; y < thumb.h; ++y) {
		switch (thumb.format.bytesPerPixel) {
		case 2: {
			const uint16 *pixels = (const uint16 *)thumb.getBasePtr(0, y);
			for (uint x = 0; x < thumb.w; ++x) {
				out.writeUint16BE(*pixels++);
			}
			} break;

		case 4: {
			const uint32 *pixels = (const uint32 *)thumb.getBasePtr(0, y);
			for (uint x = 0; x < thumb.w; ++x) {
				out.writeUint32BE(*pixels++);
			}
			} break;

		default:
			assert(0);
		}
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
