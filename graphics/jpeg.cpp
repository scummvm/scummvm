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
 * $URL$
 * $Id$
 *
 */

#include "graphics/jpeg.h"

#include "common/endian.h"
#include "common/util.h"

namespace Graphics {

// Order used to traverse the quantization tables
static const uint8 _zigZagOrder[64] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

JPEG::JPEG() :
	_str(NULL), _w(0), _h(0), _numComp(0), _components(NULL), _numScanComp(0),
	_scanComp(NULL), _currentComp(NULL) {

	// Initialize the quantization tables
	for (int i = 0; i < JPEG_MAX_QUANT_TABLES; i++)
		_quant[i] = NULL;

	// Initialize the Huffman tables
	for (int i = 0; i < 2 * JPEG_MAX_HUFF_TABLES; i++) {
		_huff[i].count = 0;
		_huff[i].values = NULL;
		_huff[i].sizes = NULL;
		_huff[i].codes = NULL;
	}
}

JPEG::~JPEG() {
	reset();
}

void JPEG::reset() {
	// Reset member variables
	_str = NULL;
	_w = _h = 0;

	// Free the components
	for (int c = 0; c < _numComp; c++)
		_components[c].surface.free();
	delete[] _components; _components = NULL;
	_numComp = 0;

	// Free the scan components
	delete[] _scanComp; _scanComp = NULL;
	_numScanComp = 0;
	_currentComp = NULL;

	// Free the quantization tables
	for (int i = 0; i < JPEG_MAX_QUANT_TABLES; i++) {
		delete[] _quant[i];
		_quant[i] = NULL;
	}

	// Free the Huffman tables
	for (int i = 0; i < 2 * JPEG_MAX_HUFF_TABLES; i++) {
		_huff[i].count = 0;
		delete[] _huff[i].values; _huff[i].values = NULL;
		delete[] _huff[i].sizes; _huff[i].sizes = NULL;
		delete[] _huff[i].codes; _huff[i].codes = NULL;
	}
}

bool JPEG::read(Common::SeekableReadStream *str) {
	// Reset member variables and tables from previous reads
	reset();

	// Save the input stream
	_str = str;

	bool ok = true;
	bool done = false;
	while (!_str->eos() && ok && !done) {
		// Read the marker
		uint16 marker = _str->readByte();
		if (marker != 0xFF) {
			error("JPEG: Invalid marker[0]: 0x%02X", marker);
			ok = false;
			break;
		}
		
		while (marker == 0xFF)
			marker = _str->readByte();

		// Process the marker data
		switch (marker) {
		case 0xC0: // Start Of Frame
			ok = readSOF0();
			break;
		case 0xC4: // Define Huffman Tables
			ok = readDHT();
			break;
		case 0xD8: // Start Of Image
			break;
		case 0xD9: // End Of Image
			done = true;
			break;
		case 0xDA: // Start Of Scan
			ok = readSOS();
			break;
		case 0xDB: // Define Quantization Tables
			ok = readDQT();
			break;
		case 0xE0: // JFIF/JFXX segment
			ok = readJFIF();
			break;
		case 0xFE: // Comment
			_str->seek(_str->readUint16BE() - 2, SEEK_CUR);
			break;
		default: { // Unknown marker
			uint16 size = _str->readUint16BE();
			warning("JPEG: Unknown marker %02X, skipping %d bytes", marker, size - 2);
			_str->seek(size - 2, SEEK_CUR);
		}
		}
	}
	return ok;
}

bool JPEG::readJFIF() {
	/* uint16 length = */ _str->readUint16BE();
	uint32 tag = _str->readUint32BE();
	if (tag != MKID_BE('JFIF'))
		return false;
	_str->readByte(); // NULL
	/* byte majorVersion = */ _str->readByte();
	/* byte minorVersion = */ _str->readByte();
	/* byte densityUnits = */ _str->readByte();
	/* uint16 xDensity = */ _str->readUint16BE();
	/* uint16 yDensity = */ _str->readUint16BE();
	byte thumbW = _str->readByte();
	byte thumbH = _str->readByte();
	_str->seek(thumbW * thumbH * 3, SEEK_CUR); // Ignore thumbnail
	return true;
}

// Marker 0xC0 (Start Of Frame, Baseline DCT)
bool JPEG::readSOF0() {
	debug(5, "JPEG: readSOF0");
	uint16 size = _str->readUint16BE();

	// Read the sample precision
	uint8 precision = _str->readByte();
	if (precision != 8) {
		warning("JPEG: Just 8 bit precision supported at the moment");
		return false;
	}

	// Image size
	_h = _str->readUint16BE();
	_w = _str->readUint16BE();

	// Number of components
	_numComp = _str->readByte();
	if (size != 8 + 3 * _numComp) {
		warning("JPEG: Invalid number of components");
		return false;
	}

	// Allocate the new components
	delete[] _components;
	_components = new Component[_numComp];

	// Read the components details
	for (int c = 0; c < _numComp; c++) {
		_components[c].id = _str->readByte();
		_components[c].factorH = _str->readByte();
		_components[c].factorV = _components[c].factorH & 0xF;
		_components[c].factorH >>= 4;
		_components[c].quantTableSelector = _str->readByte();
	}

	return true;
}

// Marker 0xC4 (Define Huffman Tables)
bool JPEG::readDHT() {
	debug(5, "JPEG: readDHT");
	uint16 size = _str->readUint16BE() - 2;
	uint32 pos = _str->pos();

	while ((uint32)_str->pos() < (size + pos)) {
		// Read the table type and id
		uint8 tableId = _str->readByte();
		uint8 tableType = tableId >> 4; // type 0: DC, 1: AC
		tableId &= 0xF;
		uint8 tableNum = (tableId << 1) + tableType;

		// Free the Huffman table
		delete[] _huff[tableNum].values; _huff[tableNum].values = NULL;
		delete[] _huff[tableNum].sizes; _huff[tableNum].sizes = NULL;
		delete[] _huff[tableNum].codes;	_huff[tableNum].codes = NULL;

		// Read the number of values for each length
		uint8 numValues[16];
		_huff[tableNum].count = 0;
		for (int len = 0; len < 16; len++) {
			numValues[len] = _str->readByte();
			_huff[tableNum].count += numValues[len];
		}

		// Allocate memory for the current table
		_huff[tableNum].values = new uint8[_huff[tableNum].count];
		_huff[tableNum].sizes = new uint8[_huff[tableNum].count];
		_huff[tableNum].codes = new uint16[_huff[tableNum].count];

		// Read the table contents
		int cur = 0;
		for (int len = 0; len < 16; len++) {
			for (int i = 0; i < numValues[len]; i++) {
				_huff[tableNum].values[cur] = _str->readByte();
				_huff[tableNum].sizes[cur] = len + 1;
				cur++;
			}
		}

		// Fill the table of Huffman codes
		cur = 0;
		uint16 curCode = 0;
		uint8 curCodeSize = _huff[tableNum].sizes[0];
		while (cur < _huff[tableNum].count) {
			// Increase the code size to fit the request
			while (_huff[tableNum].sizes[cur] != curCodeSize) {
				curCode <<= 1;
				curCodeSize++;
			}

			// Assign the current code
			_huff[tableNum].codes[cur] = curCode;
			curCode++;
			cur++;
		}
	}
	
	return true;
}

// Marker 0xDA (Start Of Scan)
bool JPEG::readSOS() {
	debug(5, "JPEG: readSOS");
	uint16 size = _str->readUint16BE();

	// Number of scan components
	_numScanComp = _str->readByte();
	if (size != 6 + 2 * _numScanComp) {
		warning("JPEG: Invalid number of components");
		return false;
	}

	// Allocate the new scan components
	delete[] _scanComp;
	_scanComp = new Component *[_numScanComp];

	// Reset the maximum sampling factors
	_maxFactorV = 0;
	_maxFactorH = 0;

	// Component-specification parameters
	for (int c = 0; c < _numScanComp; c++) {
		// Read the desired component id
		uint8 id = _str->readByte();

		// Search the component with the specified id
		bool found = false;
		for (int i = 0; !found && i < _numComp; i++) {
			if (_components[i].id == id) {
				// We found the desired component
				found = true;

				// Assign the found component to the c'th scan component
				_scanComp[c] = &_components[i];
			}
		}
		
		if (!found) {
			warning("JPEG: Invalid component");
			return false;
		}

		// Read the entropy table selectors
		_scanComp[c]->DCentropyTableSelector = _str->readByte();
		_scanComp[c]->ACentropyTableSelector = _scanComp[c]->DCentropyTableSelector & 0xF;
		_scanComp[c]->DCentropyTableSelector >>= 4;

		// Calculate the maximum sampling factors
		if (_scanComp[c]->factorV > _maxFactorV)
			_maxFactorV = _scanComp[c]->factorV;

		if (_scanComp[c]->factorH > _maxFactorH)
			_maxFactorH = _scanComp[c]->factorH;

		// Initialize the DC predictor
		_scanComp[c]->DCpredictor = 0;
	}
	
	// Initialize the scan surfaces
	for (int c = 0; c < _numScanComp; c++)
		_scanComp[c]->surface.create(_w, _h, 1);

	// Start of spectral selection
	if (_str->readByte() != 0) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// End of spectral selection
	if (_str->readByte() != 63) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// Successive approximation parameters
	if (_str->readByte() != 0) {
		warning("JPEG: Progressive scanning not supported");
		return false;
	}

	// Entropy coded sequence starts, initialize Huffman decoder
	_bitsNumber = 0;

	// Read all the scan MCUs
	uint16 xMCU = _w / (_maxFactorH * 8);
	uint16 yMCU = _h / (_maxFactorV * 8);

	// Check for non- multiple-of-8 dimensions
	if (_w % 8 != 0)
		xMCU++;
	if (_h % 8 != 0)
		yMCU++;

	bool ok = true;
	for (int y = 0; ok && (y < yMCU); y++) 
		for (int x = 0; ok && (x < xMCU); x++)
			ok = readMCU(x, y);

	return ok;
}

// Marker 0xDB (Define Quantization Tables)
bool JPEG::readDQT() {
	debug(5, "JPEG: readDQT");
	uint16 size = _str->readUint16BE() - 2;
	uint32 pos = _str->pos();
	
	while ((uint32)_str->pos() < (pos + size)) {	
		// Read the table precision and id
		uint8 tableId = _str->readByte();
		bool highPrecision = (tableId & 0xF0) != 0;
		
		// Validate the table id
		tableId &= 0xF;
		if (tableId > JPEG_MAX_QUANT_TABLES) {
			warning("JPEG: Invalid number of components");
			return false;
		}

		// Create the new table if necessary
		if (!_quant[tableId])
			_quant[tableId] = new uint16[64];

		// Read the table (stored in Zig-Zag order)
		for (int i = 0; i < 64; i++)
			_quant[tableId][i] = highPrecision ? _str->readUint16BE() : _str->readByte();
	}
	
	return true;
}

bool JPEG::readMCU(uint16 xMCU, uint16 yMCU) {
	bool ok = true;
	for (int c = 0; ok && (c < _numComp); c++) {
		// Set the current component
		_currentComp = _scanComp[c];

		// Read the data units of the current component
		for (int y = 0; ok && (y < _scanComp[c]->factorV); y++)
			for (int x = 0; ok && (x < _scanComp[c]->factorH); x++)
				ok = readDataUnit(xMCU * _scanComp[c]->factorH + x, yMCU * _scanComp[c]->factorV + y);
	}

	return ok;
}

float JPEG::idct(int x, int y, int weight, int fx, int fy) {
	float vx = cos((2 * x + 1) * fx * PI / 16);
	float vy = cos((2 * y + 1) * fy * PI / 16);
	float ret = (float)weight * vx * vy;
	
	if (fx == 0)
		ret /= sqrt(2.0f);
		
	if (fy == 0)
		ret /= sqrt(2.0f);

	return ret;
}

bool JPEG::readDataUnit(uint16 x, uint16 y) {
	// Prepare an empty data array
	int16 readData[64];
	for (int i = 1; i < 64; i++)
		readData[i] = 0;

	// Read the DC component
	readData[0] = _currentComp->DCpredictor + readDC();
	_currentComp->DCpredictor = readData[0];

	// Read the AC components (stored in Zig-Zag)
	readAC(readData);

	// Calculate the DCT coefficients from the input sequence
	int16 DCT[64];
	for (int i = 0; i < 64; i++) {
		// Dequantize
		int16 val = readData[i];
		int16 quant = _quant[_currentComp->quantTableSelector][i];
		val *= quant;

		// Store the normalized coefficients, undoing the Zig-Zag
		DCT[_zigZagOrder[i]] = val;
	}

	// Shortcut the IDCT for DC component
	float result[64];
	for (int i = 0; i < 64; i++)
		result[i] = DCT[0] / 2;

	// Apply the IDCT (PAG31)
	for (int i = 1; i < 64; i++) {
		if (DCT[i])
			for (int _y = 0; _y < 8; _y++)
				for (int _x = 0; _x < 8; _x++)
					result[_y * 8 + _x] += idct(_x, _y, DCT[i], i % 8, i / 8);
	}

	// Level shift to make the values unsigned
	// Divide by 4 is final part of IDCT
	for (int i = 0; i < 64; i++) {
		result[i] = result[i] / 4 + 128;
		
		if (result[i] < 0)
			result[i] = 0;

		if (result[i] > 255)
			result[i] = 255;
	}

	// Paint the component surface
	uint8 scalingV = _maxFactorV / _currentComp->factorV;
	uint8 scalingH = _maxFactorH / _currentComp->factorH;

	// Convert coordinates from MCU blocks to pixels
	x <<= 3;
	y <<= 3;

	// Handle non- multiple-of-8 dimensions
	byte xLim = 8;
	byte yLim = 8;
	if (x*scalingH + 8 > _w)
		xLim -= (x*scalingH + 8 - _w);
	if (y*scalingV + 8 > _h)
		yLim -= (y*scalingV + 8 - _h);

	for (int j = 0; j < yLim; j++) {
		for (int sV = 0; sV < scalingV; sV++) {
			// Get the beginning of the block line
			byte *ptr = (byte *)_currentComp->surface.getBasePtr(x * scalingH, (y + j) * scalingV + sV);

			for (int i = 0; i < xLim; i++) {
				for (uint8 sH = 0; sH < scalingH; sH++) {
					*ptr = (byte)(result[j * 8 + i]);
					ptr++;
				}
			}
		}
	}

	return true;
}

int16 JPEG::readDC() {
	// DC is type 0
	uint8 tableNum = _currentComp->DCentropyTableSelector << 1;

	// Get the number of bits to read
	uint8 numBits = readHuff(tableNum);

	// Read the requested bits
	return readSignedBits(numBits);
}

void JPEG::readAC(int16 *out) {
	// AC is type 1
	uint8 tableNum = (_currentComp->ACentropyTableSelector << 1) + 1;

	// Start reading AC element 1
	uint8 cur = 1;
	while (cur < 64) {
		uint8 s = readHuff(tableNum);
		uint8 r = s >> 4;
		s &= 0xF;

		if (s == 0) {
			if (r == 15) {
				// Skip 16 values
				cur += 16;
			} else {
				// EOB: end of block
				cur = 64;
			}
		} else {
			// Skip r values
			cur += r;

			// Read the next value
			out[cur] = readSignedBits(s);
			cur++;
		}
	}
}

int16 JPEG::readSignedBits(uint8 numBits) {
	uint16 ret = 0;
	if (numBits > 16) error("requested %d bits", numBits); //XXX

	// MSB=0 for negatives, 1 for positives
	for (int i = 0; i < numBits; i++)
		ret = (ret << 1) + readBit();

	// Extend sign bits (PAG109)
	if (!(ret >> (numBits - 1)))
	{
		uint16 tmp = ((uint16)-1 << numBits) + 1;
		ret = ret + tmp;
	}
	return ret;
}

// TODO: optimize?
uint8 JPEG::readHuff(uint8 table) {
	bool foundCode = false;
	uint8 val = 0;

	uint8 cur = 0;
	uint8 codeSize = 1;
	uint16 code = readBit();
	while (!foundCode) {
		// Prepare a code of the current size
		while (codeSize < _huff[table].sizes[cur]) {
			code = (code << 1) + readBit();
			codeSize++;
		}

		// Compare the codes of the current size
		while (!foundCode && (codeSize == _huff[table].sizes[cur])) {
			if (code == _huff[table].codes[cur]) {
				// Found the code
				val = _huff[table].values[cur];
				foundCode = true;
			} else {
				// Continue reading
				cur++;
			}
		}
	}

	return val;
}

uint8 JPEG::readBit() {
	// Read a whole byte if necessary
	if (_bitsNumber == 0) {
		_bitsData = _str->readByte();
		_bitsNumber = 8;

		// Detect markers
		if (_bitsData == 0xFF) {
			uint8 byte2 = _str->readByte();

			// A stuffed 0 validates the previous byte
			if (byte2 != 0) {
				if (byte2 == 0xDC) {
					// DNL marker: Define Number of Lines
					// TODO: terminate scan
					printf("DNL marker detected: terminate scan\n");
				} else {
					printf("Error: marker 0x%02X read in entropy data\n", byte2);
				}
			}
		}
	}
	_bitsNumber--;

	return (_bitsData & (1 << _bitsNumber)) ? 1 : 0;
}

Surface *JPEG::getComponent(uint c) {
	for (int i = 0; i < _numComp; i++)
		if (_components[i].id == c) // We found the desired component
			return &_components[i].surface;

	return NULL;
}

} // End of Graphics namespace
