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

#include "glk/debugger.h"
#include "glk/glk.h"
#include "glk/raw_decoder.h"
#include "common/file.h"
#include "graphics/managed_surface.h"
#include "image/png.h"

namespace Glk {

Debugger::Debugger() : GUI::Debugger() {
	registerCmd("dumppic", WRAP_METHOD(Debugger, cmdDumpPic));
}

int Debugger::strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Debugger::cmdDumpPic(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: dumppic <picture number>\n");
	} else {
		Common::File f;
		int picNum = strToInt(argv[1]);

		Common::Path filename(Common::String::format("pic%d.png", picNum));
		if (!f.exists(filename))
			filename = Common::Path(Common::String::format("pic%d.jpg", picNum));

		if (f.open(filename)) {
			// png or jpeg file
			Common::DumpFile df;
			if (df.open(filename)) {
				// Write out a copy of the file
				byte *data = new byte[f.size()];
				f.read(data, f.size());
				df.write(data, f.size());
				delete[] data;
				df.close();

				debugPrintf("Dumped picture\n");
			} else {
				debugPrintf("Could not find specified picture\n");
			}
		} else if (f.exists(Common::Path(Common::String::format("pic%d.rect", picNum)))) {
			debugPrintf("Picture is only a placeholder rectangle\n");
		} else if (f.open(Common::Path(Common::String::format("pic%d.raw", picNum)))) {
			// Raw picture
#ifdef USE_PNG
			Common::DumpFile df;
			RawDecoder rd;

			if (rd.loadStream(f) && df.open(Common::Path(Common::String::format("pic%d.png", picNum)))) {
				saveRawPicture(rd, df);
				debugPrintf("Dumped picture\n");
			} else {
				debugPrintf("Couldn't save picture\n");
			}
#else
			debugPrintf("PNG support needed to dump raw pictures\n");
#endif
		} else {
			debugPrintf("No such picture exists\n");
		}
	}

	return true;
}

void Debugger::saveRawPicture(const RawDecoder &rd, Common::WriteStream &ws) {
#ifdef USE_PNG
	const Graphics::Surface *surface = rd.getSurface();
	const byte *palette = rd.getPalette();
	int paletteCount = rd.getPaletteColorCount();
	int palStart = 0;
	bool hasTransColor = rd.hasTransparentColor();
	uint32 transColor = rd.getTransparentColor();

	// If the image doesn't have a palette, we can directly write out the image
	if (!palette) {
		Image::writePNG(ws, *surface);
		return;
	}

	// Create a new RGBA temporary surface
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::ManagedSurface destSurface(surface->w, surface->h, format);

	for (int y = 0; y < surface->h; ++y) {
		const byte *srcP = (const byte *)surface->getBasePtr(0, y);
		uint32 *destP = (uint32 *)destSurface.getBasePtr(0, y);

		for (int x = 0; x < surface->w; ++x, ++srcP, ++destP) {
			if ((hasTransColor && (uint32)*srcP == transColor) || (int)*srcP < palStart) {
				*destP = format.ARGBToColor(0, 0, 0, 0);
			} else {
				assert(*srcP < paletteCount);
				const byte *palP = &palette[*srcP * 3];
				*destP = format.ARGBToColor(255, palP[0], palP[1], palP[2]);
			}
		}
	}

	Image::writePNG(ws, destSurface);
#endif
}

} // End of namespace Glk
