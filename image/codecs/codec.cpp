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

#include "common/list.h"
#include "common/scummsys.h"

#include "image/codecs/codec.h"

#include "image/jpeg.h"
#include "image/codecs/bmp_raw.h"
#include "image/codecs/cdtoons.h"
#include "image/codecs/cinepak.h"
#include "image/codecs/indeo3.h"
#include "image/codecs/indeo4.h"
#include "image/codecs/indeo5.h"
#include "image/codecs/jyv1.h"
#include "image/codecs/mjpeg.h"
#include "image/codecs/mpeg.h"
#include "image/codecs/msvideo1.h"
#include "image/codecs/msrle.h"
#include "image/codecs/msrle4.h"
#include "image/codecs/qtrle.h"
#include "image/codecs/rpza.h"
#include "image/codecs/smc.h"
#include "image/codecs/svq1.h"
#include "image/codecs/truemotion1.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Image {

namespace {

/**
 * Add a color to the QuickTime dither table check queue if it hasn't already been found.
 */
inline void addColorToQueue(uint16 color, uint16 index, byte *checkBuffer, Common::List<uint16> &checkQueue) {
	if ((READ_UINT16(checkBuffer + color * 2) & 0xFF) == 0) {
		// Previously unfound color
		WRITE_UINT16(checkBuffer + color * 2, index);
		checkQueue.push_back(color);
	}
}

inline byte adjustColorRange(byte currentColor, byte correctColor, byte palColor) {
	return CLIP<int>(currentColor - palColor + correctColor, 0, 255);
}

inline uint16 makeQuickTimeDitherColor(byte r, byte g, byte b) {
	// RGB554
	return ((r & 0xF8) << 6) | ((g & 0xF8) << 1) | (b >> 4);
}

} // End of anonymous namespace

byte *Codec::createQuickTimeDitherTable(const byte *palette, uint colorCount) {
	byte *buf = new byte[0x10000];
	memset(buf, 0, 0x10000);

	Common::List<uint16> checkQueue;

	bool foundBlack = false;
	bool foundWhite = false;

	const byte *palPtr = palette;

	// See what colors we have, and add them to the queue to check
	for (uint i = 0; i < colorCount; i++) {
		byte r = *palPtr++;
		byte g = *palPtr++;
		byte b = *palPtr++;
		uint16 n = (i << 8) | 1;
		uint16 col = makeQuickTimeDitherColor(r, g, b);

		if (col == 0) {
			// Special case for close-to-black
			// The original did more here, but it effectively discarded the value
			// due to a poor if-check (whole 16-bit value instead of lower 8-bits).
			WRITE_UINT16(buf, n);
			foundBlack = true;
		} else if (col == 0x3FFF) {
			// Special case for close-to-white
			// The original did more here, but it effectively discarded the value
			// due to a poor if-check (whole 16-bit value instead of lower 8-bits).
			WRITE_UINT16(buf + 0x7FFE, n);
			foundWhite = true;
		} else {
			// Previously unfound color
			addColorToQueue(col, n, buf, checkQueue);
		}
	}

	// More special handling for white
	if (foundWhite)
		checkQueue.push_front(0x3FFF);

	// More special handling for black
	if (foundBlack)
		checkQueue.push_front(0);

	// Go through the list of colors we have and match up similar colors
	// to fill in the table as best as we can.
	while (!checkQueue.empty()) {
		uint16 col = checkQueue.front();
		checkQueue.pop_front();
		uint16 index = READ_UINT16(buf + col * 2);

		uint32 x = col << 4;
		if ((x & 0xFF) < 0xF0)
			addColorToQueue((x + 0x10) >> 4, index, buf, checkQueue);
		if ((x & 0xFF) >= 0x10)
			addColorToQueue((x - 0x10) >> 4, index, buf, checkQueue);

		uint32 y = col << 7;
		if ((y & 0xFF00) < 0xF800)
			addColorToQueue((y + 0x800) >> 7, index, buf, checkQueue);
		if ((y & 0xFF00) >= 0x800)
			addColorToQueue((y - 0x800) >> 7, index, buf, checkQueue);

		uint32 z = col << 2;
		if ((z & 0xFF00) < 0xF800)
			addColorToQueue((z + 0x800) >> 2, index, buf, checkQueue);
		if ((z & 0xFF00) >= 0x800)
			addColorToQueue((z - 0x800) >> 2, index, buf, checkQueue);
	}

	// Contract the table back to just palette entries
	for (int i = 0; i < 0x4000; i++)
		buf[i] = READ_UINT16(buf + i * 2) >> 8;

	// Now go through and distribute the error to three more pixels
	byte *bufPtr = buf;
	for (uint realR = 0; realR < 0x100; realR += 8) {
		for (uint realG = 0; realG < 0x100; realG += 8) {
			for (uint realB = 0; realB < 0x100; realB += 16) {
				byte palIndex = *bufPtr;
				byte r = realR;
				byte g = realG;
				byte b = realB;

				byte palR = palette[palIndex * 3] & 0xF8;
				byte palG = palette[palIndex * 3 + 1] & 0xF8;
				byte palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0x4000] = palIndex;

				palR = palette[palIndex * 3] & 0xF8;
				palG = palette[palIndex * 3 + 1] & 0xF8;
				palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0x8000] = palIndex;

				palR = palette[palIndex * 3] & 0xF8;
				palG = palette[palIndex * 3 + 1] & 0xF8;
				palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0xC000] = palIndex;

				bufPtr++;
			}
		}
	}

	return buf;
}

Codec *createBitmapCodec(uint32 tag, uint32 streamTag, int width, int height, int bitsPerPixel) {
	// Crusader videos are special cased here because the frame type is not in the "compression"
	// tag but in the "stream handler" tag for these files
	if (JYV1Decoder::isJYV1StreamTag(streamTag)) {
		assert(bitsPerPixel == 8);
		return new JYV1Decoder(width, height, streamTag);
	}

	switch (tag) {
	case SWAP_CONSTANT_32(0):
		return new BitmapRawDecoder(width, height, bitsPerPixel);
	case SWAP_CONSTANT_32(1):
		return new MSRLEDecoder(width, height, bitsPerPixel);
	case SWAP_CONSTANT_32(2):
		return new MSRLE4Decoder(width, height, bitsPerPixel);
	case MKTAG('C','R','A','M'):
	case MKTAG('m','s','v','c'):
	case MKTAG('W','H','A','M'):
		return new MSVideo1Decoder(width, height, bitsPerPixel);
	case MKTAG('c','v','i','d'):
		return new CinepakDecoder(bitsPerPixel);
	case MKTAG('I','V','3','2'):
		return new Indeo3Decoder(width, height, bitsPerPixel);
	case MKTAG('I', 'V', '4', '1'):
	case MKTAG('I', 'V', '4', '2'):
		return new Indeo4Decoder(width, height, bitsPerPixel);
	case MKTAG('I', 'V', '5', '0'):
		return new Indeo5Decoder(width, height, bitsPerPixel);
#ifdef IMAGE_CODECS_TRUEMOTION1_H
	case MKTAG('D','U','C','K'):
	case MKTAG('d','u','c','k'):
		return new TrueMotion1Decoder();
#endif
#ifdef USE_MPEG2
	case MKTAG('m','p','g','2'):
		return new MPEGDecoder();
#endif
	case MKTAG('M','J','P','G'):
	case MKTAG('m','j','p','g'):
		return new MJPEGDecoder();
	default:
		if (tag & 0x00FFFFFF)
			warning("Unknown BMP/AVI compression format \'%s\'", tag2str(tag));
		else
			warning("Unknown BMP/AVI compression format %d", SWAP_BYTES_32(tag));
	}

	return 0;
}

Codec *createQuickTimeCodec(uint32 tag, int width, int height, int bitsPerPixel) {
	switch (tag) {
	case MKTAG('c','v','i','d'):
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		return new CinepakDecoder(bitsPerPixel);
	case MKTAG('r','p','z','a'):
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		return new RPZADecoder(width, height);
	case MKTAG('r','l','e',' '):
		// QuickTime RLE: Used by some Myst ME videos.
		return new QTRLEDecoder(width, height, bitsPerPixel);
	case MKTAG('s','m','c',' '):
		// Apple SMC: Used by some Myst videos.
		return new SMCDecoder(width, height);
	case MKTAG('S','V','Q','1'):
		// Sorenson Video 1: Used by some Myst ME videos.
		return new SVQ1Decoder(width, height);
	case MKTAG('S','V','Q','3'):
		// Sorenson Video 3: Used by some Myst ME videos.
		warning("Sorenson Video 3 not yet supported");
		break;
	case MKTAG('j','p','e','g'):
		// JPEG: Used by some Myst ME 10th Anniversary videos.
		return new JPEGDecoder();
	case MKTAG('Q','k','B','k'):
		// CDToons: Used by most of the Broderbund games.
		return new CDToonsDecoder(width, height);
	case MKTAG('r','a','w',' '):
		// Used my L-Zone-mac (Director game)
		return new BitmapRawDecoder(width, height, bitsPerPixel);
	default:
		warning("Unsupported QuickTime codec \'%s\'", tag2str(tag));
	}

	return 0;
}

} // End of namespace Image
