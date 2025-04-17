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

#include "awe/screenshot.h"
#include "awe/file.h"

namespace Awe {

static void TO_LE16(uint8_t *dst, uint16_t value) {
	for (int i = 0; i < 2; ++i) {
		dst[i] = value & 255;
		value >>= 8;
	}
}

#define kTgaImageTypeUncompressedTrueColor 2
#define kTgaImageTypeRunLengthEncodedTrueColor 10
#define kTgaDirectionTop (1 << 5)

static const int TGA_HEADER_SIZE = 18;

void saveTGA(const char *filename, const uint16_t *rgb555, int w, int h) {
	static const uint8_t kImageType = kTgaImageTypeRunLengthEncodedTrueColor;
	uint8_t buffer[TGA_HEADER_SIZE];
	buffer[0] = 0; // ID Length
	buffer[1] = 0; // ColorMap Type
	buffer[2] = kImageType;
	TO_LE16(buffer + 3, 0); // ColorMap Start
	TO_LE16(buffer + 5, 0); // ColorMap Length
	buffer[7] = 0;  // ColorMap Bits
	TO_LE16(buffer + 8, 0); // X-origin
	TO_LE16(buffer + 10, 0); // Y-origin
	TO_LE16(buffer + 12, w); // Image Width
	TO_LE16(buffer + 14, h); // Image Height
	buffer[16] = 16; // Pixel Depth
	buffer[17] = kTgaDirectionTop;  // Descriptor

	Common::DumpFile f;
	if (f.open(filename)) {
		f.write(buffer, sizeof(buffer));
		if (kImageType == kTgaImageTypeUncompressedTrueColor) {
			for (int i = 0; i < w * h; ++i) {
				const uint16_t color = *rgb555++;
				f.writeByte(color & 255);
				f.writeByte(color >> 8);
			}
		} else {
			assert(kImageType == kTgaImageTypeRunLengthEncodedTrueColor);
			uint16_t prevColor = *rgb555++;
			int count = 0;
			for (int i = 1; i < w * h; ++i) {
				const uint16_t color = *rgb555++;
				if (prevColor == color && count < 127) {
					++count;
					continue;
				}
				f.writeByte(count | 0x80);
				f.writeByte(prevColor & 255);
				f.writeByte(prevColor >> 8);
				count = 0;
				prevColor = color;
			}
			if (count != 0) {
				f.writeByte(count | 0x80);
				f.writeByte(prevColor & 255);
				f.writeByte(prevColor >> 8);
			}
		}
	}
}

static void fwriteUint16LE(Common::WriteStream *fp, uint16_t n) {
	fp->writeUint16LE(n);
}

static void fwriteUint32LE(Common::WriteStream *fp, uint32_t n) {
	fp->writeUint32LE(n);
}

static const uint16_t TAG_BM = 0x4D42;

void saveBMP(const char *filename, const uint8_t *bits, const uint8_t *pal, int w, int h) {
#ifdef TODO
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		int alignWidth = (w + 3) & ~3;
		int imageSize = alignWidth * h;

		// Write file header
		fwriteUint16LE(fp, TAG_BM);
		fwriteUint32LE(fp, 14 + 40 + 4 * 256 + imageSize);
		fwriteUint16LE(fp, 0); // reserved1
		fwriteUint16LE(fp, 0); // reserved2
		fwriteUint32LE(fp, 14 + 40 + 4 * 256);

		// Write info header
		fwriteUint32LE(fp, 40);
		fwriteUint32LE(fp, w);
		fwriteUint32LE(fp, h);
		fwriteUint16LE(fp, 1); // planes
		fwriteUint16LE(fp, 8); // bit_count
		fwriteUint32LE(fp, 0); // compression
		fwriteUint32LE(fp, imageSize); // size_image
		fwriteUint32LE(fp, 0); // x_pels_per_meter
		fwriteUint32LE(fp, 0); // y_pels_per_meter
		fwriteUint32LE(fp, 0); // num_colors_used
		fwriteUint32LE(fp, 0); // num_colors_important

		// Write palette data
		for (int i = 0; i < 256; ++i) {
			fp->writeByte(pal[2], fp);
			fp->writeByte(pal[1], fp);
			fp->writeByte(pal[0], fp);
			fp->writeByte(0, fp);
			pal += 3;
		}

		// Write bitmap data
		const int pitch = w;
		bits += h * pitch;
		for (int i = 0; i < h; ++i) {
			bits -= pitch;
			fwrite(bits, w, 1, fp);
			int pad = alignWidth - w;
			while (pad--) {
				fp->writeByte(0, fp);
			}
		}

		fclose(fp);
	}
#else
	error("TODO: saveBMP");
#endif
}

} // namespace Awe
