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

#ifndef GRAPHICS_JPEG_H
#define GRAPHICS_JPEG_H

#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

struct PixelFormat;

#define JPEG_MAX_QUANT_TABLES 4
#define JPEG_MAX_HUFF_TABLES 2

class JPEG {
public:
	JPEG();
	~JPEG();

	bool read(Common::SeekableReadStream *str);
	bool isLoaded() const { return _numComp && _w && _h; }
	uint16 getWidth() const { return _w; }
	uint16 getHeight() const { return _h; }

	Surface *getComponent(uint c);
	Surface *getSurface(const PixelFormat &format);

private:
	void reset();

	Common::SeekableReadStream *_stream;
	uint16 _w, _h;

	// Image components
	uint8 _numComp;
	struct Component {
		// Global values
		uint8 id;
		uint8 factorH;
		uint8 factorV;
		uint8 quantTableSelector;

		// Scan specific values
		uint8 DCentropyTableSelector;
		uint8 ACentropyTableSelector;
		int16 DCpredictor;

		// Result image for this component
		Surface surface;
	};

	Component *_components;

	// Scan components
	uint8 _numScanComp;
	Component **_scanComp;
	Component *_currentComp;

	// Maximum sampling factors, used to calculate the interleaving of the MCU
	uint8 _maxFactorV;
	uint8 _maxFactorH;

	// Quantization tables
	uint16 *_quant[JPEG_MAX_QUANT_TABLES];

	// Huffman tables
	struct HuffmanTable {
		uint8 count;
		uint8 *values;
		uint8 *sizes;
		uint16 *codes;
	} _huff[2 * JPEG_MAX_HUFF_TABLES];

	// Marker read functions
	bool readJFIF();
	bool readSOF0();
	bool readDHT();
	bool readSOS();
	bool readDQT();

	// Helper functions
	bool readMCU(uint16 xMCU, uint16 yMCU);
	bool readDataUnit(uint16 x, uint16 y);
	int16 readDC();
	void readAC(int16 *out);
	int16 readSignedBits(uint8 numBits);

	// Huffman decoding
	uint8 readHuff(uint8 table);
	uint8 readBit();
	uint8 _bitsData;
	uint8 _bitsNumber;

	// Inverse Discrete Cosine Transformation
	void idct8x8(float dst[64], const int16 src[64]);
};

} // End of Graphics namespace

#endif // GRAPHICS_JPEG_H
