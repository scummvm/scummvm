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
#ifdef USE_JYV1
	// Crusader videos are special cased here because the frame type is not in the "compression"
	// tag but in the "stream handler" tag for these files
	if (JYV1Decoder::isJYV1StreamTag(streamTag)) {
		assert(bitsPerPixel == 8);
		return new JYV1Decoder(width, height, streamTag);
	}
#endif

	const char *missingCodec = nullptr;

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
		missingCodec = "Indeo 3";
		break;
#endif
	case MKTAG('I', 'V', '4', '1'):
	case MKTAG('I', 'V', '4', '2'):
#ifdef USE_INDEO45
		return new Indeo4Decoder(width, height, bitsPerPixel);
#else
		missingCodec = "Indeo 4";
		break;
#endif
	case MKTAG('I', 'V', '5', '0'):
#ifdef USE_INDEO45
		return new Indeo5Decoder(width, height, bitsPerPixel);
#else
		missingCodec = "Indeo 5";
		break;
#endif

	case MKTAG('X', 'x', 'a', 'n'):
#ifdef USE_XAN
		return new XanDecoder(width, height, bitsPerPixel);
#else
		missingCodec = "Xan";
		break;
#endif
	case MKTAG('D','U','C','K'):
	case MKTAG('d','u','c','k'):
#ifdef USE_TRUEMOTION1
		return new TrueMotion1Decoder();
#else
		missingCodec = "TrueMotion1";
		break;
#endif
	case MKTAG('m','p','g','2'):
#ifdef USE_MPEG2
		return new MPEGDecoder();
#else
		missingCodec = "MPEG2";
		break;
#endif
	case MKTAG('M','J','P','G'):
	case MKTAG('m','j','p','g'):
#ifdef USE_MJPEG
		return new MJPEGDecoder();
#else
		missingCodec = "MJPEG";
		break;
#endif
	default:
		if (tag & 0x00FFFFFF)
			warning("Unknown BMP/AVI compression format \'%s\'", tag2str(tag));
		else
			warning("Unknown BMP/AVI compression format %d", SWAP_BYTES_32(tag));
		return 0;
	}

	assert(missingCodec);
	warning("createBitmapCodec(): %s codec is not compiled", missingCodec);
	return 0;
}

Codec *createQuickTimeCodec(uint32 tag, int width, int height, int bitsPerPixel) {
	const char *missingCodec = nullptr;

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
		missingCodec = "Sorenson Video 1";
		break;
#endif
	case MKTAG('S','V','Q','3'):
		// Sorenson Video 3: Used by some Myst ME videos.
		warning("Sorenson Video 3 not yet supported");
		return 0;
	case MKTAG('j','p','e','g'):
#ifdef USE_JPEG
		// JPEG: Used by some Myst ME 10th Anniversary videos.
		return new JPEGDecoder();
#else
		missingCodec = "JPEG";
		break;
#endif
	case MKTAG('Q','k','B','k'):
#ifdef USE_CDTOONS
		// CDToons: Used by most of the Broderbund games.
		return new CDToonsDecoder(width, height);
#else
		missingCodec = "CDToons";
		break;
#endif
	case MKTAG('r','a','w',' '):
		// Used my L-Zone-mac (Director game)
		return new BitmapRawDecoder(width, height, bitsPerPixel, true, true);
	case MKTAG('I','V','3','2'):
#ifdef USE_INDEO3
		// Indeo 3: Used by Team Xtreme: Operation Weather Disaster (Spanish)
		return new Indeo3Decoder(width, height, bitsPerPixel);
#else
		missingCodec = "Indeo 3";
		break;
#endif
	default:
		warning("Unsupported QuickTime codec \'%s\'", tag2str(tag));
		return 0;
	}

	assert(missingCodec);
	warning("createBitmapCodec(): %s codec is not compiled", missingCodec);
	return 0;
}

} // End of namespace Image
