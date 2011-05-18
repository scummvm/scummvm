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
 */

#include "graphics/dither.h"

#include "common/endian.h"
#include "common/stream.h"

namespace Graphics {

PaletteLUT::PaletteLUT(byte depth, PaletteFormat format) {
	assert((depth > 1) && (depth < 9));

	// For adjusting depth
	_depth1 = depth;
	_depth2 = 2 * _depth1;
	_shift = 8 - _depth1;

	// The table's dimensions
	_dim1 = (1 << _depth1);
	_dim2 = _dim1 * _dim1;
	_dim3 = _dim1 * _dim1 * _dim1;

	_format = format;

	// What's already built
	_got = _dim1;
	_gots = new byte[_dim1];

	// The lookup table
	_lut = new byte[_dim3];

	memset(_lutPal, 0, 768);
	memset(_realPal, 0, 768);
	memset(_gots, 1, _dim1);
}

void PaletteLUT::setPalette(const byte *palette, PaletteFormat format,
		byte depth, int transp) {

	assert((depth > 1) && (depth < 9));

	_transp = transp;

	int shift = 8 - depth;

	// Checking for the table's and the palette's pixel format
	if ((_format == kPaletteRGB) && (format == kPaletteYUV)) {
		byte *newPal = _realPal;
		const byte *oldPal = palette;
		for (int i = 0; i < 256; i++, newPal += 3, oldPal += 3)
			YUV2RGB(oldPal[0] << shift, oldPal[1] << shift, oldPal[2] << shift,
					newPal[0], newPal[1], newPal[2]);
	} else if ((_format == kPaletteYUV) && (format == kPaletteRGB)) {
		byte *newPal = _realPal;
		const byte *oldPal = palette;
		for (int i = 0; i < 256; i++, newPal += 3, oldPal += 3)
			RGB2YUV(oldPal[0] << shift, oldPal[1] << shift, oldPal[2] << shift,
					newPal[0], newPal[1], newPal[2]);
	} else
		memcpy(_realPal, palette, 768);

	// Using the specified depth for the lookup
	byte *newPal = _lutPal, *oldPal = _realPal;
	for (int i = 0; i < 768; i++)
		*newPal++ = (*oldPal++) >> _shift;

	// Everything has to be rebuilt
	_got = 0;
	memset(_gots, 0, _dim1);
}

PaletteLUT::~PaletteLUT() {
	delete[] _lut;
	delete[] _gots;
}

void PaletteLUT::buildNext() {
	if (_got >= _dim1)
		return;

	build(_got++);
}

#define SQR(x) ((x) * (x))
// Building one "slice"
void PaletteLUT::build(int d1) {
	// First dimension
	byte *lut = _lut + d1 * _dim2;

	// Second dimension
	for (uint32 j = 0; j < _dim1; j++) {
		// Third dimension
		for (uint32 k = 0; k < _dim1; k++) {
			const byte *p = _lutPal;
			uint32 d = 0xFFFFFFFF;
			byte n = 0;

			// Going over every palette entry, searching for the closest
			for (int c = 0; c < 256; c++, p += 3) {
				// Ignore the transparent color
				if (c == _transp)
					continue;

				uint32 di = SQR(d1 - p[0]) + SQR(j - p[1]) + SQR(k - p[2]);
				if (di < d) {
					d = di;
					n = c;
					if (d == 0)
						break;
				}
			}

			*lut++ = n;
		}
	}

	// Got this slice now
	_gots[d1] = 1;
}

inline int PaletteLUT::getIndex(byte c1, byte c2, byte c3) const {
	return ((c1 >> _shift) << _depth2) | ((c2 >> _shift) << _depth1) | (c3 >> _shift);
}

void PaletteLUT::getEntry(byte index, byte &c1, byte &c2, byte &c3) const {
	c1 = _realPal[index * 3 + 0];
	c2 = _realPal[index * 3 + 1];
	c3 = _realPal[index * 3 + 2];
}

byte PaletteLUT::findNearest(byte c1, byte c2, byte c3) {
	return _lut[getIndex(c1, c2, c3)];
}

byte PaletteLUT::findNearest(byte c1, byte c2, byte c3, byte &nC1, byte &nC2, byte &nC3) {
	// If we don't have the required "slice" yet, build it
	if (!_gots[c1 >> _shift])
		build(c1 >> _shift);

	int palIndex = _lut[getIndex(c1, c2, c3)];
	int i = palIndex * 3;

	nC1 = _realPal[i + 0];
	nC2 = _realPal[i + 1];
	nC3 = _realPal[i + 2];

	return palIndex;
}

bool PaletteLUT::save(Common::WriteStream &stream) {
	// The table has to be completely built before we can save
	while (_got < _dim1)
		buildNext();

	stream.writeUint32BE(MKTAG('P','L','U','T')); // Magic
	stream.writeUint32BE(kVersion);
	stream.writeByte(_depth1);
	if (stream.write(_realPal, 768) != 768)
		return false;
	if (stream.write(_lutPal, 768) != 768)
		return false;
	if (stream.write(_lut, _dim3) != _dim3)
		return false;
	if (!stream.flush())
		return false;

	if (stream.err())
		return false;

	return true;
}

bool PaletteLUT::load(Common::SeekableReadStream &stream) {
	//             _realPal + _lutPal + _lut  + _depth1 + magic + version
	int32 needSize =  768   +   768   + _dim3 +    1    +   4   +    4;

	if ((stream.size() - stream.pos()) < needSize)
		return false;

	// Magic
	if (stream.readUint32BE() != MKTAG('P','L','U','T'))
		return false;

	if (stream.readUint32BE() != kVersion)
		return false;

	byte depth1 = stream.readByte();

	if (depth1 != _depth1)
		return false;

	if (stream.read(_realPal, 768) != 768)
		return false;
	if (stream.read(_lutPal, 768) != 768)
		return false;
	if (stream.read(_lut, _dim3) != _dim3)
		return false;

	_got = _dim1;
	memset(_gots, 1, _dim1);

	return true;
}

SierraLight::SierraLight(int16 width, PaletteLUT *palLUT) {
	assert(width > 0);

	_width = width;
	_palLUT = palLUT;

	// Big buffer for the errors of the current and next line
	_errorBuf = new int32[3 * (2 * (_width + 2*1))];
	memset(_errorBuf, 0, (3 * (2 * (_width + 2*1))) * sizeof(int32));

	_curLine = 0;
	_errors[0] = _errorBuf + 3;
	_errors[1] = _errors[0] + 3 * (_width + 2*1);
}

SierraLight::~SierraLight() {
	delete[] _errorBuf;
}

void SierraLight::newFrame() {
	_curLine = 0;
	memset(_errors[0], 0, 3 * _width * sizeof(int32));
	memset(_errors[1], 0, 3 * _width * sizeof(int32));
}

void SierraLight::nextLine() {
	// Clear the finished line, it will become the last line in the buffer
	memset(_errors[_curLine], 0, 3 * _width * sizeof(int32));

	_curLine = (_curLine + 1) % 2;
}

byte SierraLight::dither(byte c1, byte c2, byte c3, uint32 x) {
	assert(_palLUT);
	assert(x < (uint32)_width);

	int32 eC1, eC2, eC3;

	getErrors(x, eC1, eC2, eC3);

	// Apply error on values
	c1 = CLIP<int>(c1 + eC1, 0, 255);
	c2 = CLIP<int>(c2 + eC2, 0, 255);
	c3 = CLIP<int>(c3 + eC3, 0, 255);

	// Find color
	byte newC1, newC2, newC3;
	byte newPixel = _palLUT->findNearest(c1, c2, c3, newC1, newC2, newC3);

	// Calculate new error
	eC1 = c1 - newC1;
	eC2 = c2 - newC2;
	eC3 = c3 - newC3;

	// Add them
	addErrors(x, eC1, eC2, eC3);

	return newPixel;
}

inline void SierraLight::getErrors(uint32 x, int32 &eC1, int32 &eC2, int32 &eC3) {
	int32 *errCur = _errors[_curLine];

	x *= 3;
	eC1 = errCur[x + 0] >> 2;
	eC2 = errCur[x + 1] >> 2;
	eC3 = errCur[x + 2] >> 2;
}

inline void SierraLight::addErrors(uint32 x, int32 eC1, int32 eC2, int32 eC3) {
	int32 *errCur  = _errors[_curLine];
	int32 *errNext = _errors[(_curLine + 1) % 2];

	// Indices for current error
	int x0 = 3 * (x + 1);
	int x1 = 3 * (x + 0);
	int x2 = 3 * (x - 1);

	errCur [x0 + 0] += eC1 << 1;
	errCur [x0 + 1] += eC2 << 1;
	errCur [x0 + 2] += eC3 << 1;
	errNext[x1 + 0] += eC1;
	errNext[x1 + 1] += eC2;
	errNext[x1 + 2] += eC3;
	errNext[x2 + 0] += eC1;
	errNext[x2 + 1] += eC2;
	errNext[x2 + 2] += eC3;
}

} // End of namespace Graphics
