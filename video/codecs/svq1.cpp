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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Sorenson Video 1 Codec
// Based off ffmpeg's SVQ1 decoder (written by Mike Melanson)

#include "video/codecs/svq1.h"
#include "video/codecs/svq1_cb.h"
#include "video/codecs/svq1_vlc.h"

#include "common/stream.h"
#include "common/bitstream.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "common/huffman.h"

#include "graphics/yuv_to_rgb.h"

namespace Video {

#define SVQ1_BLOCK_SKIP     0
#define SVQ1_BLOCK_INTER    1
#define SVQ1_BLOCK_INTER_4V 2
#define SVQ1_BLOCK_INTRA    3

SVQ1Decoder::SVQ1Decoder(uint16 width, uint16 height) {
	debug(1, "SVQ1Decoder::SVQ1Decoder(width:%d, height:%d)", width, height);
	_width = width;
	_height = height;
	_frameWidth = _frameHeight = 0;
	_surface = 0;

	_last[0] = 0;
	_last[1] = 0;
	_last[2] = 0;

	// Setup Variable Length Code Tables
	_blockType = new Common::Huffman(0, 4, s_svq1BlockTypeCodes, s_svq1BlockTypeLengths);

	for (int i = 0; i < 6; i++) {
		_intraMultistage[i] = new Common::Huffman(0, 8, s_svq1IntraMultistageCodes[i], s_svq1IntraMultistageLengths[i]);
		_interMultistage[i] = new Common::Huffman(0, 8, s_svq1InterMultistageCodes[i], s_svq1InterMultistageLengths[i]);
	}

	_intraMean = new Common::Huffman(0, 256, s_svq1IntraMeanCodes, s_svq1IntraMeanLengths);
	_interMean = new Common::Huffman(0, 512, s_svq1InterMeanCodes, s_svq1InterMeanLengths);
	_motionComponent = new Common::Huffman(0, 33, s_svq1MotionComponentCodes, s_svq1MotionComponentLengths);
}

SVQ1Decoder::~SVQ1Decoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}

	delete[] _last[0];
	delete[] _last[1];
	delete[] _last[2];

	delete _blockType;
	delete _intraMean;
	delete _interMean;
	delete _motionComponent;

	for (int i = 0; i < 6; i++) {
		delete _intraMultistage[i];
		delete _interMultistage[i];
	}
}

const Graphics::Surface *SVQ1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	debug(1, "SVQ1Decoder::decodeImage()");

	Common::BitStream32BEMSB frameData(*stream);

	uint32 frameCode = frameData.getBits(22);
	debug(1, " frameCode: %d", frameCode);

	if ((frameCode & ~0x70) || !(frameCode & 0x60)) { // Invalid
		warning("Invalid Image at frameCode");
		return _surface;
	}

	// swap some header bytes (why?)
	//if (frameCode != 0x20) {
	//  uint32 *src = stream;
	//
	//  for (i = 4; i < 8; i++) {
	//    src[i] = ((src[i] << 16) | (src[i] >> 16)) ^ src[7 - i];
	// }
	//}

#if 0
	static const uint16 checksum_table[256] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};
#endif

	byte temporalReference = frameData.getBits(8);
	debug(1, " temporalReference: %d", temporalReference);
	const char* types[4] = { "I (Key)", "P (Delta from Previous)", "B (Delta from Next)", "Invalid" };
	byte frameType = frameData.getBits(2);
	debug(1, " frameType: %d = %s Frame", frameType, types[frameType]);
	if (frameType == 0) { // I Frame
		// TODO: Validate checksum if present
		if (frameCode == 0x50 || frameCode == 0x60) {
			uint32 checksum = frameData.getBits(16);
			debug(1, " checksum:0x%02x", checksum);
			//uint16 calculate_packet_checksum (const uint8 *data, const int length) {
			//  int value;
			//for (int i = 0; i < length; i++)
				//	value = checksum_table[data[i] ^ (value >> 8)] ^ ((value & 0xFF) << 8);
		}

		if ((frameCode ^ 0x10) >= 0x50) {
			// Skip embedded string
			uint8 stringLen = frameData.getBits(8);
			for (uint16 i = 0; i < stringLen-1; i++)
				frameData.skip(8);
		}

		byte unk1 = frameData.getBits(2); // Unknown
		debug(1, " unk1: %d", unk1);
		byte unk2 = frameData.getBits(2); // Unknown
		debug(1, " unk2: %d", unk2);
		bool unk3 = frameData.getBit(); // Unknown
		debug(1, " unk3: %d", unk3);

		static const struct { uint w, h; } standardFrameSizes[7] = {
			{ 160, 120 }, // 0
			{ 128,  96 }, // 1
			{ 176, 144 }, // 2
			{ 352, 288 }, // 3
			{ 704, 576 }, // 4
			{ 240, 180 }, // 5
			{ 320, 240 }  // 6
		};

		byte frameSizeCode = frameData.getBits(3);
		debug(1, " frameSizeCode: %d", frameSizeCode);

		if (frameSizeCode == 7) {
			_frameWidth = frameData.getBits(12);
			_frameHeight = frameData.getBits(12);
		} else {
			_frameWidth = standardFrameSizes[frameSizeCode].w;
			_frameHeight = standardFrameSizes[frameSizeCode].h;
		}
		debug(1, " frameWidth: %d", _frameWidth);
		debug(1, " frameHeight: %d", _frameHeight);

		// Now we'll create the surface
		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(_frameWidth, _frameHeight, g_system->getScreenFormat());
			_surface->w = _width;
			_surface->h = _height;
		}
	} else if (frameType == 2) { // B Frame
		warning("B Frames not supported by SVQ1 decoder");
		return _surface;
	} else if (frameType == 3) { // Invalid
		warning("Invalid Frame Type");
		return _surface;
	}

	bool checksumPresent = frameData.getBit();
	debug(1, " checksumPresent: %d", checksumPresent);
	if (checksumPresent) {
		bool usePacketChecksum = frameData.getBit();
		debug(1, " usePacketChecksum: %d", usePacketChecksum);
		bool componentChecksumsAfterImageData = frameData.getBit();
		debug(1, " componentChecksumsAfterImageData: %d", componentChecksumsAfterImageData);
		byte unk4 = frameData.getBits(2);
		debug(1, " unk4: %d", unk4);
		if (unk4 != 0)
			warning("Invalid Frame Header in SVQ1 Frame Decode");
	}

	bool unk5 = frameData.getBit();
	debug(1, " unk5: %d", unk5);
	if (unk5) {
		bool unk6 = frameData.getBit();
		debug(1, " unk6: %d", unk6);
		byte unk7 = frameData.getBits(4);
		debug(1, " unk7: %d", unk7);
		bool unk8 = frameData.getBit();
		debug(1, " unk8: %d", unk8);
		byte unk9 = frameData.getBits(2);
		debug(1, " unk9: %d", unk9);
		while (frameData.getBit()) {
			byte unk10 = frameData.getBits(8);
			debug(1, " unk10: %d", unk10);
		}
	}

	byte *current[3];
	// FIXME - Added extra _width of 16px blocks to stop out of
	//         range access causing crashes. Need to correct code...
	current[0] = new byte[_frameWidth * _frameHeight + (_frameWidth * 16)];
	current[1] = new byte[(_frameWidth / 4) * (_frameHeight / 4) + (_frameWidth / 4 * 16)];
	current[2] = new byte[(_frameWidth / 4) * (_frameHeight / 4) + (_frameWidth / 4 * 16)];

	// Decode Y, U and V component planes
	for (int i = 0; i < 3; i++) {
		int linesize, width, height;
		if (i == 0) {
			// Y Size is width * height
			width  = _frameWidth;
			if (width % 16) {
				width /= 16;
				width++;
				width *= 16;
			}
			assert(width % 16 == 0);
			height = _frameHeight;
			if (height % 16) {
				height /= 16;
				height++;
				height *= 16;
			}
			assert(height % 16 == 0);
			linesize = _frameWidth;
		} else {
			// U and V size is width/4 * height/4
			width  = _frameWidth / 4;
			if (width % 16) {
				width /= 16;
				width++;
				width *= 16;
			}
			assert(width % 16 == 0);
			height = _frameHeight / 4;
			if (height % 16) {
				height /= 16;
				height++;
				height *= 16;
			}
			assert(height % 16 == 0);
			linesize = _frameWidth / 4;
		}

		if (frameType == 0) { // I Frame
			// Keyframe (I)
			byte *currentP = current[i];
			for (uint16 y = 0; y < height; y += 16) {
				for (uint16 x = 0; x < width; x += 16) {
					if (int result = svq1DecodeBlockIntra(&frameData, &currentP[x], linesize) != 0) {
						warning("Error in svq1DecodeBlock %i (keyframe)", result);
						return _surface;
					}
				}
				currentP += 16 * linesize;
			}
		} else {
			// Delta frame (P or B)

			// Prediction Motion Vector
			Common::Point *pmv = new Common::Point[(width/8) + 3];

			byte *previous;
			if(frameType == 2) { // B Frame
				warning("B Frame not supported currently");
				//previous = _next[i];
			} else
				previous = _last[i];

			byte *currentP = current[i];
			for (uint16 y = 0; y < height; y += 16) {
				for (uint16 x = 0; x < width; x += 16) {
					if (int result = svq1DecodeDeltaBlock(&frameData, &currentP[x], previous, linesize, pmv, x, y) != 0) {
						warning("Error in svq1DecodeDeltaBlock %i", result);
						return _surface;
					}
				}

				pmv[0].x = pmv[0].y = 0;

				currentP += 16*linesize;
			}
			delete[] pmv;
		}
	}

	convertYUV410ToRGB(_surface, current[0], current[1], current[2], _frameWidth, _frameHeight, _frameWidth, _frameWidth / 4);

	for (int i = 0; i < 3; i++) {
		delete[] _last[i];
		_last[i] = current[i];
	}

	return _surface;
}

int SVQ1Decoder::svq1DecodeBlockIntra(Common::BitStream *s, uint8 *pixels, int pitch) {
	uint8 *list[63];
	uint32 *dst;
	int entries[6];
	int i, j, m, n;
	int mean, stages;
	unsigned int x, y, width, height, level;
	uint32 n1, n2, n3, n4;

	// initialize list for breadth first processing of vectors
	list[0] = pixels;

	// recursively process vector
	for (i=0, m=1, n=1, level=5; i < n; i++) {
		// SVQ1_PROCESS_VECTOR()
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}
			// divide block if next bit set
			if (s->getBit() == 0)
				break;
			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		dst = (uint32 *) list[i];
		width = 1 << ((4 + level) /2);
		height = 1 << ((3 + level) /2);

		// get number of stages (-1 skips vector, 0 for mean only)
		stages = _intraMultistage[level]->getSymbol(*s) - 1;

		if (stages == -1) {
			for (y=0; y < height; y++) {
				memset (&dst[y*(pitch / 4)], 0, width);
			}
		continue; // skip vector
		}

		if ((stages > 0) && (level >= 4)) {
			warning("Error (svq1_decode_block_intra): invalid vector: stages=%i level=%i", stages, level);
		return -1; // invalid vector
		}

		mean = _intraMean->getSymbol(*s);

		if (stages == 0) {
			for (y=0; y < height; y++) {
				memset (&dst[y*(pitch / 4)], mean, width);
			}
		} else {
			// SVQ1_CALC_CODEBOOK_ENTRIES(svq1_intra_codebooks);
			const uint32 *codebook = s_svq1IntraCodebooks[level];
			uint32 bit_cache = s->getBits(4*stages);
			// calculate codebook entries for this vector
			for (j=0; j < stages; j++) {
				entries[j] = (((bit_cache >> (4*(stages - j - 1))) & 0xF) + 16*j) << (level + 1);
			}
			mean -= (stages * 128);
			n4    = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

			// SVQ1_DO_CODEBOOK_INTRA()
			for (y=0; y < height; y++) {
				for (x=0; x < (width / 4); x++, codebook++) {
					n1 = n4;
					n2 = n4;
					// SVQ1_ADD_CODEBOOK()
					// add codebook entries to vector
					for (j=0; j < stages; j++) {
						n3  = codebook[entries[j]] ^ 0x80808080;
						n1 += ((n3 & 0xFF00FF00) >> 8);
						n2 +=  (n3 & 0x00FF00FF);
					}

					// clip to [0..255]
					if (n1 & 0xFF00FF00) {
						n3  = ((( n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 += 0x7F007F00;
						n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 &= (n3 & 0x00FF00FF);
					}

					if (n2 & 0xFF00FF00) {
						n3  = ((( n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 += 0x7F007F00;
						n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 &= (n3 & 0x00FF00FF);
					}

					// store result
					dst[x] = (n1 << 8) | n2;
				}
				dst += (pitch / 4);
			}
		}
	}

	return 0;
}

int SVQ1Decoder::svq1DecodeBlockNonIntra(Common::BitStream *s, uint8 *pixels, int pitch) {
	uint8 *list[63];
	uint32 *dst;
	int entries[6];
	int i, j, m, n;
	int mean, stages;
	int x, y, width, height, level;
	uint32 n1, n2, n3, n4;

	// initialize list for breadth first processing of vectors
	list[0] = pixels;

	// recursively process vector
	for (i=0, m=1, n=1, level=5; i < n; i++) {
		// SVQ1_PROCESS_VECTOR()
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}
			// divide block if next bit set
			if (s->getBit() == 0)
				break;
			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		dst = (uint32 *) list[i];
		width = 1 << ((4 + level) /2);
		height = 1 << ((3 + level) /2);

		// get number of stages (-1 skips vector, 0 for mean only)
		stages = _interMultistage[level]->getSymbol(*s) - 1;

		if (stages == -1) continue; // skip vector

		if ((stages > 0) && (level >= 4)) {
			warning("Error (svq1_decode_block_non_intra): invalid vector: stages=%i level=%i", stages, level);
			return -1;        // invalid vector
		}

		mean = _interMean->getSymbol(*s) - 256;

		// SVQ1_CALC_CODEBOOK_ENTRIES(svq1_inter_codebooks);
		const uint32 *codebook = s_svq1InterCodebooks[level];
		uint32 bit_cache = s->getBits(4*stages);
		// calculate codebook entries for this vector
		for (j=0; j < stages; j++) {
			entries[j] = (((bit_cache >> (4*(stages - j - 1))) & 0xF) + 16*j) << (level + 1);
		}
		mean -= (stages * 128);
		n4 = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

		// SVQ1_DO_CODEBOOK_NONINTRA()
		for (y=0; y < height; y++) {
			for (x=0; x < (width / 4); x++, codebook++) {
				n3 = dst[x];
				// add mean value to vector
				n1 = ((n3 & 0xFF00FF00) >> 8) + n4;
				n2 =  (n3 & 0x00FF00FF)          + n4;
				//SVQ1_ADD_CODEBOOK()
				// add codebook entries to vector
				for (j=0; j < stages; j++) {
					n3  = codebook[entries[j]] ^ 0x80808080;
					n1 += ((n3 & 0xFF00FF00) >> 8);
					n2 +=  (n3 & 0x00FF00FF);
				}

				// clip to [0..255]
				if (n1 & 0xFF00FF00) {
					n3  = ((( n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 += 0x7F007F00;
					n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 &= (n3 & 0x00FF00FF);
				}

				if (n2 & 0xFF00FF00) {
					n3  = ((( n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 += 0x7F007F00;
					n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 &= (n3 & 0x00FF00FF);
				}

				// store result
				dst[x] = (n1 << 8) | n2;
			}
			dst += (pitch / 4);
		}
	}
	return 0;
}

// median of 3
static inline int mid_pred(int a, int b, int c) {
	if (a > b) {
		if (c > b) {
			if (c > a) b = a;
			else b = c;
		}
	} else {
		if (b > c) {
			if (c > a) b = c;
			else b = a;
		}
	}
	return b;
}

int SVQ1Decoder::svq1DecodeMotionVector(Common::BitStream *s, Common::Point *mv, Common::Point **pmv) {
	for (int i=0; i < 2; i++) {
		// get motion code
		int diff = _motionComponent->getSymbol(*s);
		if (diff < 0)
			return -1;
		else if (diff) {
			if (s->getBit()) diff= -diff;
		}

		// add median of motion vector predictors and clip result
		if (i == 1)
			mv->y = ((diff + mid_pred(pmv[0]->y, pmv[1]->y, pmv[2]->y)) << 26) >> 26;
		else
			mv->x = ((diff + mid_pred(pmv[0]->x, pmv[1]->x, pmv[2]->x)) << 26) >> 26;
	}

	return 0;
}

void SVQ1Decoder::svq1SkipBlock(uint8 *current, uint8 *previous, int pitch, int x, int y) {
	uint8 *src;
	uint8 *dst;

	src = &previous[x + y*pitch];
	dst = current;

	for (int i = 0; i < 16; i++) {
		memcpy(dst, src, 16);
		src += pitch;
		dst += pitch;
	}
}

static void put_pixels8_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	for (int i = 0; i < h; i++) {
		*((uint32*)(block)) = READ_UINT32(pixels);
		*((uint32*)(block + 4)) = READ_UINT32(pixels + 4);
		pixels += line_size;
		block += line_size;
	}
}

static inline uint32 rnd_avg32(uint32 a, uint32 b) {
	return (a | b) - (((a ^ b) & ~((0x01)*0x01010101UL)) >> 1);
}

static inline void put_pixels8_l2(uint8 *dst, const uint8 *src1, const uint8 *src2, 
                                  int dst_stride, int src_stride1, int src_stride2, int h) {
	for (int i = 0; i < h; i++){
		uint32 a, b;
		a= READ_UINT32(&src1[i*src_stride1]);
		b= READ_UINT32(&src2[i*src_stride2]);
		*((uint32*)&dst[i*dst_stride]) = rnd_avg32(a, b);
		a= READ_UINT32(&src1[i*src_stride1 + 4]);
		b= READ_UINT32(&src2[i*src_stride2 + 4]);
		*((uint32*)&dst[i*dst_stride + 4]) = rnd_avg32(a, b);
	}
}

static inline void put_pixels8_x2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_l2(block, pixels, pixels+1, line_size, line_size, line_size, h);
}

static inline void put_pixels8_y2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_l2(block, pixels, pixels+line_size, line_size, line_size, line_size, h);
}

static inline void put_pixels8_xy2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	for (int j = 0; j < 2; j++) {
		uint32 a = READ_UINT32(pixels);
		uint32 b = READ_UINT32(pixels+1);
		uint32 l0 = (a & 0x03030303UL) + (b & 0x03030303UL) + 0x02020202UL;
		uint32 h0 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);
		uint32 l1, h1;

		pixels += line_size;
		for (int i = 0; i < h; i += 2) {
			a = READ_UINT32(pixels);
			b = READ_UINT32(pixels+1);
			l1 = (a & 0x03030303UL) + (b & 0x03030303UL);
			h1 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);
			*((uint32*)block) = h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL);
			pixels += line_size;
			block += line_size;
			a = READ_UINT32(pixels);
			b = READ_UINT32(pixels+1);
			l0 = (a & 0x03030303UL) + (b & 0x03030303UL) + 0x02020202UL;
			h0 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);
			*((uint32*)block) = h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL);
			pixels += line_size;
			block += line_size;
		}
		pixels += 4 - line_size*(h + 1);
		block += 4 - line_size*h;
	}
}

static void put_pixels16_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_c(block, pixels, line_size, h);
	put_pixels8_c(block+8, pixels+8, line_size, h);
}

static void put_pixels16_x2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_x2_c(block, pixels, line_size, h);
	put_pixels8_x2_c(block+8, pixels+8, line_size, h);
}

static void put_pixels16_y2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_y2_c(block, pixels, line_size, h);
	put_pixels8_y2_c(block+8, pixels+8, line_size, h);
}

static void put_pixels16_xy2_c(uint8 *block, const uint8 *pixels, int line_size, int h) {
	put_pixels8_xy2_c(block, pixels, line_size, h);
	put_pixels8_xy2_c(block+8, pixels+8, line_size, h);
}

int SVQ1Decoder::svq1MotionInterBlock(Common::BitStream *ss,
                                uint8 *current, uint8 *previous, int pitch,
                                Common::Point *motion, int x, int y) {
	uint8 *src;
	uint8 *dst;
	Common::Point mv;
	Common::Point *pmv[3];
	int result;

	// predict and decode motion vector
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	result = svq1DecodeMotionVector(ss, &mv, pmv);

	if (result != 0)
		return result;

	motion[0].x                =
	motion[(x / 8) + 2].x      =
	motion[(x / 8) + 3].x      = mv.x;
	motion[0].y                =
	motion[(x / 8) + 2].y      =
	motion[(x / 8) + 3].y      = mv.y;

	if(y + (mv.y >> 1)<0)
		mv.y= 0;
	if(x + (mv.x >> 1)<0)
		mv.x= 0;

#if 0
	int w = (s->width+15)&~15;
	int h = (s->height+15)&~15;
	if(x + (mv.x >> 1)<0 || y + (mv.y >> 1)<0 || x + (mv.x >> 1) + 16 > w || y + (mv.y >> 1) + 16> h)
		debug(1, "%d %d %d %d", x, y, x + (mv.x >> 1), y + (mv.y >> 1));
#endif

	src = &previous[(x + (mv.x >> 1)) + (y + (mv.y >> 1))*pitch];
	dst = current;

	// Halfpel motion compensation with rounding (a+b+1)>>1.
	// 4 motion compensation functions for the 4 halfpel positions
	// for 16x16 blocks
	switch(((mv.y & 1)*2) + (mv.x & 1)) {
	case 0:
		put_pixels16_c(dst, src, pitch, 16);
		break;
	case 1:
		put_pixels16_x2_c(dst, src, pitch, 16);
		break;
	case 2:
		put_pixels16_y2_c(dst, src, pitch, 16);
		break;
	case 3:
		put_pixels16_xy2_c(dst, src, pitch, 16);
		break;
	default:
		error("Motion Compensation Function Lookup Error. Should Not Happen!");
		break;
	}

	return 0;
}

int SVQ1Decoder::svq1MotionInter4vBlock(Common::BitStream *ss,
                                  uint8 *current, uint8 *previous, int pitch,
                                  Common::Point *motion, int x, int y) {
	uint8 *src;
	uint8 *dst;
	Common::Point mv;
	Common::Point *pmv[4];
	int i, result;

	// predict and decode motion vector (0)
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	result = svq1DecodeMotionVector(ss, &mv, pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (1)
	pmv[0] = &mv;
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 3];
	}
	result = svq1DecodeMotionVector(ss, &motion[0], pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (2)
	pmv[1] = &motion[0];
	pmv[2] = &motion[(x / 8) + 1];

	result = svq1DecodeMotionVector(ss, &motion[(x / 8) + 2], pmv);

	if (result != 0)
		return result;

	// predict and decode motion vector (3)
	pmv[2] = &motion[(x / 8) + 2];
	pmv[3] = &motion[(x / 8) + 3];

	result = svq1DecodeMotionVector(ss, pmv[3], pmv);

	if (result != 0)
		return result;

	// form predictions
	for (i=0; i < 4; i++) {
		int mvx = pmv[i]->x + (i&1)*16;
		int mvy = pmv[i]->y + (i>>1)*16;

		///XXX /FIXME clipping or padding?
		if(y + (mvy >> 1)<0)
			mvy = 0;
		if(x + (mvx >> 1)<0)
			mvx = 0;

#if 0
		int w = (s->width+15)&~15;
		int h = (s->height+15)&~15;
		if(x + (mvx >> 1)<0 || y + (mvy >> 1)<0 || x + (mvx >> 1) + 8 > w || y + (mvy >> 1) + 8> h)
			debug(1, "%d %d %d %d", x, y, x + (mvx >> 1), y + (mvy >> 1));
#endif
		src = &previous[(x + (mvx >> 1)) + (y + (mvy >> 1))*pitch];
		dst = current;

		// Halfpel motion compensation with rounding (a+b+1)>>1.
		// 4 motion compensation functions for the 4 halfpel positions
		// for 8x8 blocks
		switch(((mvy & 1)*2) + (mvx & 1)) {
		case 0:
			put_pixels8_c(dst, src, pitch, 8);
			break;
		case 1:
			put_pixels8_x2_c(dst, src, pitch, 8);
			break;
		case 2:
			put_pixels8_y2_c(dst, src, pitch, 8);
			break;
		case 3:
			put_pixels8_xy2_c(dst, src, pitch, 8);
			break;
		default:
			error("Motion Compensation Function Lookup Error. Should Not Happen!");
			break;
		}

		// select next block
		if (i & 1) {
			current  += 8*(pitch - 1);
		} else {
			current  += 8;
		}
	}

	return 0;
}

int SVQ1Decoder::svq1DecodeDeltaBlock(Common::BitStream *ss,
                        uint8 *current, uint8 *previous, int pitch,
                        Common::Point *motion, int x, int y) {
	uint32 block_type;
	int result = 0;

	// get block type
	block_type = _blockType->getSymbol(*ss);

	// reset motion vectors
	if (block_type == SVQ1_BLOCK_SKIP || block_type == SVQ1_BLOCK_INTRA) {
		motion[0].x                 =
		motion[0].y                 =
		motion[(x / 8) + 2].x =
		motion[(x / 8) + 2].y =
		motion[(x / 8) + 3].x =
		motion[(x / 8) + 3].y = 0;
	}

	switch (block_type) {
	case SVQ1_BLOCK_SKIP:
		svq1SkipBlock(current, previous, pitch, x, y);
		break;

	case SVQ1_BLOCK_INTER:
		result = svq1MotionInterBlock(ss, current, previous, pitch, motion, x, y);
		if (result != 0) {
			warning("Error in svq1MotionInterBlock %i", result);
			break;
		}
		result = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;

	case SVQ1_BLOCK_INTER_4V:
		result = svq1MotionInter4vBlock(ss, current, previous, pitch, motion, x, y);
		if (result != 0) {
			warning("Error in svq1MotionInter4vBlock %i", result);
			break;
		}
		result = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;

	case SVQ1_BLOCK_INTRA:
		result = svq1DecodeBlockIntra(ss, current, pitch);
		break;
	}

	return result;
}

} // End of namespace Video
