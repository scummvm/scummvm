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

#include "image/codecs/codec.h"

#include "image/jpeg.h"
#include "image/codecs/bmp_raw.h"
#include "image/codecs/cdtoons.h"
#include "image/codecs/cinepak.h"

#ifdef USE_INDEO3
#include "image/codecs/indeo3.h"
#endif
#ifdef USE_INDEO45
#include "image/codecs/indeo4.h"
#include "image/codecs/indeo5.h"
#endif

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
#include "image/codecs/xan.h"

#include "common/endian.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Image {

Graphics::PixelFormat Codec::getDefaultYUVFormat() {
	Graphics::PixelFormat format = g_system->getScreenFormat();

	// Default to a 32bpp format, if in 8bpp mode
	if (format.isCLUT8())
		return Graphics::PixelFormat::createFormatRGBA32();
	else
		return format;
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
		return new BitmapRawDecoder(width, height, bitsPerPixel, false);
	case SWAP_CONSTANT_32(1):
		return new MSRLEDecoder(width, height, bitsPerPixel);
	case SWAP_CONSTANT_32(2):
		return new MSRLE4Decoder(width, height, bitsPerPixel);
	case SWAP_CONSTANT_32(3):
		// Used with v4-v5 BMP headers to produce transparent BMPs
		return new BitmapRawDecoder(width, height, bitsPerPixel, false);
	case MKTAG('C','R','A','M'):
	case MKTAG('m','s','v','c'):
	case MKTAG('W','H','A','M'):
		return new MSVideo1Decoder(width, height, bitsPerPixel);
	case MKTAG('c','v','i','d'):
		return new CinepakDecoder(bitsPerPixel);

	case MKTAG('I','V','3','2'):
#ifdef USE_INDEO3
		return new Indeo3Decoder(width, height, bitsPerPixel);
#else
		warning("createBitmapCodec(): Indeo 3 codec is not compiled");
		return 0;
#endif
	case MKTAG('I', 'V', '4', '1'):
	case MKTAG('I', 'V', '4', '2'):
#ifdef USE_INDEO45
		return new Indeo4Decoder(width, height, bitsPerPixel);
#else
		warning("createBitmapCodec(): Indeo 4 & 5 codecs are not compiled");
		return 0;
#endif
	case MKTAG('I', 'V', '5', '0'):
#ifdef USE_INDEO45
		return new Indeo5Decoder(width, height, bitsPerPixel);
#else
		warning("createBitmapCodec(): Indeo 4 & 5 codecs are not compiled");
		return 0;
#endif

	case MKTAG('X', 'x', 'a', 'n'):
		return new XanDecoder(width, height, bitsPerPixel);
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
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this. Very popular for Director titles.
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
#ifdef USE_SVQ1
		// Sorenson Video 1: Used by some Myst ME videos.
		return new SVQ1Decoder(width, height);
#else
		warning("createQuickTimeCodec(): Sorenson Video 1 codec is not compiled");
		return 0;
#endif
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
		return new BitmapRawDecoder(width, height, bitsPerPixel, true, true);
	case MKTAG('I','V','3','2'):
#ifdef USE_INDEO3
		// Indeo 3: Used by Team Xtreme: Operation Weather Disaster (Spanish)
		return new Indeo3Decoder(width, height, bitsPerPixel);
#else
		warning("createQuickTimeCodec(): Indeo 3 codec is not compiled");
		return 0;
#endif
	default:
		warning("Unsupported QuickTime codec \'%s\'", tag2str(tag));
	}

	return 0;
}

} // End of namespace Image
