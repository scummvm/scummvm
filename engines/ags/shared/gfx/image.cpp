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

#include "ags/shared/gfx/image.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/stream.h"
#include "ags/lib/allegro.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "image/bmp.h"
#include "image/iff.h"
#include "image/pcx.h"
#include "image/tga.h"

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

namespace AGS3 {

template<class DECODER>
BITMAP *decodeImageStream(Common::SeekableReadStream &stream, color *pal) {
	DECODER decoder;

	if (decoder.loadStream(stream)) {
		// Create the output surface
		const Graphics::Surface *src = decoder.getSurface();

		// Copy the decoded surface
		int bpp = 8 * src->format.bytesPerPixel;
		if (bpp == 24)
			bpp = 32;
		Surface *dest = (Surface *)create_bitmap_ex(bpp, src->w, src->h);
		dest->blitFrom(*src);

		// Copy the palette
		const Graphics::Palette &palP = decoder.getPalette();
		if (pal) {
			for (uint idx = 0; idx < palP.size(); ++idx) {
				palP.get(idx, pal[idx].r, pal[idx].g, pal[idx].b);
				pal[idx].filler = 0xff;
			}
		}

		return dest;
	} else {
		return nullptr;
	}
}

template<class DECODER>
BITMAP *decodeImage(const char *filename, color *pal) {
	AGS::Shared::Stream *file = AGS3::AGS::Shared::File::OpenFileRead(filename);
	if (!file)
		return nullptr;

	AGS::Shared::ScummVMReadStream f(file);
	return decodeImageStream<DECODER>(f, pal);
}

template<class DECODER>
BITMAP *decodeImage(PACKFILE *pf, color *pal) {
	if (!pf)
		return nullptr;

	AGS::Shared::ScummVMPackReadStream f(pf);
	f.seek(0);
	return decodeImageStream<DECODER>(f, pal);
}

BITMAP *load_bmp(const char *filename, color *pal) {
	return decodeImage<Image::BitmapDecoder>(filename, pal);
}

BITMAP *load_lbm(const char *filename, color *pal) {
	return decodeImage<Image::IFFDecoder>(filename, pal);
}

BITMAP *load_pcx(const char *filename, color *pal) {
	return decodeImage<Image::PCXDecoder>(filename, pal);
}

BITMAP *load_tga(const char *filename, color *pal) {
	return decodeImage<Image::TGADecoder>(filename, pal);
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

BITMAP *load_bitmap(PACKFILE *pf, color *pal) {
	BITMAP *result;

	if ((result = decodeImage<Image::BitmapDecoder>(pf, pal)) != nullptr)
		return result;
	if ((result = decodeImage<Image::IFFDecoder>(pf, pal)) != nullptr)
		return result;
	if ((result = decodeImage<Image::PCXDecoder>(pf, pal)) != nullptr)
		return result;
	if ((result = decodeImage<Image::TGADecoder>(pf, pal)) != nullptr)
		return result;

	error("Unknown image file");
}

bool save_bitmap(Common::WriteStream &out, BITMAP *bmp, const RGB *pal) {
	const Graphics::ManagedSurface &src = bmp->getSurface();
	if (bmp->format.isCLUT8() && pal) {
		byte palette[256 * 3];
		for (int c = 0, i = 0; c < 256; ++c, i += 3) {
			palette[i] = VGA_COLOR_TRANS(pal[c].r);
			palette[i + 1] = VGA_COLOR_TRANS(pal[c].g);
			palette[i + 2] = VGA_COLOR_TRANS(pal[c].b);
		}

		return Image::writeBMP(out, src, palette);
	} else {
		return Image::writeBMP(out, src);
	}
}

} // namespace AGS
