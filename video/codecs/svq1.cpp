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
#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "graphics/yuv_to_rgb.h"

namespace Video {

// TODO: Common could do with a good Bitstream Reader Class 
//       capable of wrapping ReadStream and byte *buffers
//       This would replace this, a similar class in SMK, QDM2
//       and probably a number of other similar pieces of code.
class SVQ1BitStream {
public:
	SVQ1BitStream(Common::SeekableReadStream *stream)
		: _stream(stream), _bitCount(0) {
	}

	bool getBit();
	byte getBitsByte(uint n);
	uint32 getBitsUint32(uint n);

private:
	Common::SeekableReadStream *_stream;

	byte _curByte;
	byte  _bitCount;
};

bool SVQ1BitStream::getBit() {
	if (_bitCount == 0) {
		assert(_stream->pos() < _stream->size());
		_curByte = _stream->readByte();
		_bitCount = 8;
	}

	bool v = _curByte & 1;

	_curByte >>= 1;
	--_bitCount;

	return v;
}

byte SVQ1BitStream::getBitsByte(uint n) {
	assert (n <= 8);
	byte v = 0;

	for (uint i = 0; i < 8; i++) {
		v >>= 1;
		if (getBit() && i < n)
			v |= 0x80;
	}

	return v;
}

uint32 SVQ1BitStream::getBitsUint32(uint n) {
	assert (n <= 32);
	uint32 v = 0;

	for (uint i = 0; i < 32; i++) {
		v >>= 1;
		if (getBit() && i < n)
			v |= 0x8000;
	}

	return v;
}

SVQ1Decoder::SVQ1Decoder(uint16 width, uint16 height) {
	_surface = new Graphics::Surface();
	_surface->create(width, height, g_system->getScreenFormat());
}

SVQ1Decoder::~SVQ1Decoder() {
	_surface->free();
	delete _surface;
}

const Graphics::Surface *SVQ1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	debug(1, "SVQ1Decoder::decodeImage()");

	// Debugging Output to compare with output of Bitstream Reader
#if 1
	int32 startPos = stream->pos();
	for (uint32 i = 0; i < 6; i++) {
		debug(1, " Stream Byte %d: 0x%02x", i, stream->readByte());
	}
	stream->seek(startPos, SEEK_SET);
#endif

	SVQ1BitStream *frameData = new SVQ1BitStream(stream);

	uint32 frameCode = frameData->getBitsUint32(22);
	debug(1, " frameCode: %d", frameCode);

  if ((frameCode & ~0x70) || !(frameCode & 0x60)) // Invalid
    return _surface;

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

	byte temporalReference = frameData->getBitsByte(8);
	debug(1, " temporalReference: %d", temporalReference);
	const char* types[4] = { "I Frame", "P Frame", "B Frame", "Invalid" };
	byte pictureType = frameData->getBitsByte(2);
	debug(1, " pictureType: %d (%s)", pictureType, types[pictureType]);
	if (pictureType == 3) // Invalid
		return _surface;
	else if (pictureType == 0) { // I Frame
		if (frameCode == 0x50 || frameCode == 0x60) {
			uint32 checksum = frameData->getBitsUint32(16);
			debug(1, " checksum:0x%02x", checksum);
			// TODO: Validate checksum
			//uint16 calculate_packet_checksum (const uint8 *data, const int length) {
			//  int value;
  		//	for (int i = 0; i < length; i++)
    	//		value = checksum_table[data[i] ^ (value >> 8)] ^ ((value & 0xFF) << 8);
  		//	return value;
			//}
		}
	}

	static const uint8 stringXORTable[256] = {
		0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
		0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
		0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
		0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
		0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
		0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
		0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
		0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
		0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
		0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
		0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
		0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
		0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
		0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
		0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
		0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
		0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
		0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
		0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
		0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
		0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
		0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
		0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
		0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
		0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
		0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
		0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
		0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
		0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
		0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
		0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
		0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
	};

	if ((frameCode ^ 0x10) >= 0x50) {
		// Decode embedded string
		Common::String str;
		uint8 stringLen = frameData->getBitsByte(8);
		byte xorVal = stringXORTable[stringLen];

		for (uint16 i = 0; i < stringLen-1; i++) {
			byte data = frameData->getBitsByte(8);
			str += data ^ xorVal;
			xorVal = stringXORTable[data];
		}
		debug(1, " Embedded String of %d Characters: \"%s\"", stringLen, str.c_str());
	}

	byte unk1 = frameData->getBitsByte(2); // Unknown
	debug(1, "unk1: %d", unk1);
	byte unk2 = frameData->getBitsByte(2); // Unknown
	debug(1, "unk2: %d", unk2);
	bool unk3 = frameData->getBit(); // Unknown
	debug(1, "unk3: %d", unk3);

	static const struct { uint w, h; } standardFrameSizes[7] = {
		{ 160, 120 }, // 0
		{ 128,  96 }, // 1
		{ 176, 144 }, // 2
		{ 352, 288 }, // 3
	  { 704, 576 }, // 4
		{ 240, 180 }, // 5
		{ 320, 240 }  // 6
	};

	byte frameSizeCode = frameData->getBitsByte(3);
	debug(1, " frameSizeCode: %d", frameSizeCode);
	uint16 frameWidth, frameHeight;
	if (frameSizeCode == 7) {
		frameWidth = frameData->getBitsUint32(12);
		frameHeight = frameData->getBitsUint32(12);
	} else {
		frameWidth = standardFrameSizes[frameSizeCode].w;
		frameHeight = standardFrameSizes[frameSizeCode].h;
	}
	debug(1, " frameWidth: %d", frameWidth);
	debug(1, " frameHeight: %d", frameHeight);
	if (frameWidth == 0 || frameHeight == 0) // Invalid
		return _surface;

	bool checksumPresent = frameData->getBit();
	debug(1, " checksumPresent: %d", checksumPresent);
	if (checksumPresent) {
		bool usePacketChecksum = frameData->getBit();
		debug(1, " usePacketChecksum: %d", usePacketChecksum);
		bool componentChecksumsAfterImageData = frameData->getBit();
		debug(1, " componentChecksumsAfterImageData: %d", componentChecksumsAfterImageData);
		byte unk4 = frameData->getBitsByte(2);
		debug(1, " unk4: %d", unk4);
		if (unk4 != 0)
			warning("Invalid Frame Header in SVQ1 Frame Decode");
	}

	bool unk5 = frameData->getBit();
	debug(1, " unk5: %d", unk5);
	if (unk5) {
		bool unk6 = frameData->getBit();
		debug(1, " unk6: %d", unk6);
		byte unk7 = frameData->getBitsByte(4);
		debug(1, " unk7: %d", unk7);
		bool unk8 = frameData->getBit();
		debug(1, " unk8: %d", unk8);
		byte unk9 = frameData->getBitsByte(2);
		debug(1, " unk9: %d", unk9);
		while (frameData->getBit()) {
			byte unk10 = frameData->getBitsByte(8);
			debug(1, " unk10: %d", unk10);
		}
	}

	if (frameWidth <= _surface->w && frameHeight <= _surface->h) {
		byte *yPlane = new byte[frameWidth*frameHeight];
		byte *uPlane = new byte[(frameWidth/2)*(frameHeight/2)];
		byte *vPlane = new byte[(frameWidth/2)*(frameHeight/2)];

		// TODO: Read Plane Data
		for (uint32 i = 0; i < frameWidth*frameHeight; i++)
			yPlane[i] = 0;
		for (uint32 i = 0; i < (frameWidth/2)*(frameHeight/2); i++) {
			uPlane[i] = 0;
			vPlane[i] = 0;
		}

		delete frameData;

		convertYUV410ToRGB(_surface, yPlane, uPlane, vPlane, frameWidth, frameHeight, frameWidth, frameWidth/2);
	
		delete[] yPlane;
		delete[] uPlane;
		delete[] vPlane;
	} else
		warning("FrameWidth/Height Sanity Check Failed!");

	return _surface;
}

} // End of namespace Video
