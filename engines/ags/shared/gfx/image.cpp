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

#include "ags/shared/gfx/image.h"
#include "ags/lib/allegro.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "image/pcx.h"

namespace AGS3 {

template<class DECODER>
BITMAP *decodeImage(const char *filename, color *pal) {
	DECODER decoder;
	Common::File f;

	if (f.open(filename) && decoder.loadStream(f)) {
		// Create the output surface
		const Graphics::Surface *src = decoder.getSurface();

		// Copy the decoded surface
		Surface *dest = new Surface();
		dest->create(src->w, src->h, src->format);
		dest->blitFrom(*src);

		// Copy the palette
		const byte *palP = decoder.getPalette();
		if (palP) {
			for (int idx = 0; idx < 256; ++idx, palP += 3) {
				pal[idx].r = palP[0];
				pal[idx].g = palP[1];
				pal[idx].b = palP[2];
				pal[idx].filler = 0xff;
			}
		}

		return dest;
	} else {
		return nullptr;
	}
}

BITMAP *load_bmp_pf(PACKFILE *f, color *pal) {
	error("TODO: load_bmp_pf");
}

BITMAP *load_pcx_pf(PACKFILE *f, color *pal) {
	error("TODO: load_pcx_pf");
}

BITMAP *load_tga_pf(PACKFILE *f, color *pal) {
	error("TODO: load_tga_pf");
}

BITMAP *load_bmp(const char *filename, color *pal) {
	error("TODO: load_bmp");
}

BITMAP *load_lbm(const char *filename, color *pal) {
	error("TODO: load_lbm");
}

BITMAP *load_pcx(const char *filename, color *pal) {
	return decodeImage<Image::PCXDecoder>(filename, pal);
}

BITMAP *load_tga(const char *filename, color *pal) {
	error("TODO: load_tga");
}

BITMAP *load_bitmap(const char *filename, color *pal) {
	Common::String fname(filename);

	if (fname.hasSuffixIgnoreCase(".bmp"))
		return load_bmp(filename, pal);
	else if (fname.hasSuffixIgnoreCase(".lbm"))
		return load_lbm(filename, pal);
	else if (fname.hasSuffixIgnoreCase(".pcx"))
		return load_pcx(filename, pal);
	else if (fname.hasSuffixIgnoreCase(".tga"))
		return load_tga(filename, pal);
	else
		error("Unknown image file - %s", filename);
}

int save_bitmap(Common::WriteStream &out, BITMAP *bmp, const RGB *pal) {
#ifdef SCUMM_LITTLE_ENDIAN
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 16, 8, 0, 0);
#else
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 0, 8, 16, 0);
#endif

	Graphics::ManagedSurface &src = bmp->getSurface();
	Graphics::Surface *tmp = nullptr;
	const Graphics::Surface *surface;

	if (bmp->format == requiredFormat_3byte) {
		surface = &src.rawSurface();
	} else {
		surface = tmp = src.rawSurface().convertTo(requiredFormat_3byte);
	}

	int dstPitch = surface->w * 3;
	int extraDataLength = (dstPitch % 4) ? 4 - (dstPitch % 4) : 0;
	int padding = 0;

	out.writeByte('B');
	out.writeByte('M');
	out.writeUint32LE(surface->h * dstPitch + 54);
	out.writeUint32LE(0);
	out.writeUint32LE(54);
	out.writeUint32LE(40);
	out.writeUint32LE(surface->w);
	out.writeUint32LE(surface->h);
	out.writeUint16LE(1);
	out.writeUint16LE(24);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);

	for (uint y = surface->h; y-- > 0;) {
		out.write((const void *)surface->getBasePtr(0, y), dstPitch);
		out.write(&padding, extraDataLength);
	}

	// free tmp surface
	if (tmp) {
		tmp->free();
		delete tmp;
	}

	return true;
}

} // namespace AGS
