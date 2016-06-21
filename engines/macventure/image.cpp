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

#include "macventure/image.h"

namespace MacVenture {

PPICHuff PPIC1Huff = {
	// Masks
	{ 0x0000,0x2000,0x4000,0x5000,0x6000,0x7000,0x8000,0x9000,0xa000,
	0xb000,0xc000,0xd000,0xd800,0xe000,0xe800,0xf000,0xf800 },
	// Lens
	{ 3,3,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5 },
	// Symbols
	{ 0x00,0x0f,0x03,0x05,0x06,0x07,0x08,0x09,0x0a,0x0c,0xff,0x01,
	0x02,0x04,0x0b,0x0d,0xe }
};

PPICHuff PPIC2Huff = {
	// Masks
	{ 0x0000,0x4000,0x8000,0xc000,0xc800,0xd000,0xd800,0xe000,0xe800,
	0xf000,0xf400,0xf600,0xf800,0xfa00,0xfc00,0xfe00,0xff00 },
	// Lens
	{ 2,2,2,5,5,5,5,5,5,6,7,7,7,7,7,8,8 },
	// Symbols
	{ 0xff,0x00,0x0f,0x01,0x03,0x07,0x0e,0x0c,0x08,0x06,0x02,0x04,
	0x09,0x0d,0x0b,0x0a,0x05 }
};

ImageAsset::ImageAsset(ObjID id, Container * container) {
	_id = id;
	_container = container;

	decodePPIC();
}

ImageAsset::~ImageAsset() {
	delete _surface;
	delete _mask;
	delete[] _data;
}

void ImageAsset::blit(Graphics::ManagedSurface * target) {
	debug("Blitting image %x ", _id);
}

void ImageAsset::decodePPIC() {
	ObjID realID = _id;
	uint32 size = _container->getItemByteSize(_id);
	if (size == 2 || size == 0) {
		realID = _container->getItem(_id)->readUint16BE();
	}
	Common::BitStream32BEMSB stream(_container->getItem(realID));

	uint8 mode = stream.getBits(3);
	int w, h;
	if (stream.getBit()) h = stream.getBits(10);
	else h = stream.getBits(6);

	if (stream.getBit()) w = stream.getBits(10);
	else w = stream.getBits(6);

	_rowBytes = ((w + 0xF) >> 3) & 0xFFFE;
	_bitWidth = w;
	_bitHeight = h;

	_surface = new Graphics::ManagedSurface(_rowBytes, h, Graphics::PixelFormat::createFormatCLUT8());
	_mask = new Graphics::ManagedSurface(_rowBytes, h, Graphics::PixelFormat::createFormatCLUT8());
	_data = new byte[_surface->w * _surface->h];

	switch (mode)
	{
	case MacVenture::kPPIC0:
		decodePPIC0(stream);
		break;
	case MacVenture::kPPIC1:
		decodePPIC1(stream);
		break;
	case MacVenture::kPPIC2:
		decodePPIC2(stream);
		break;
	case MacVenture::kPPIC3:
		decodePPIC3(stream);
		break;
	}
}

void ImageAsset::decodePPIC0(Common::BitStream & stream) {	
	for (uint y = 0; y < _surface->h; y++)
		for (uint x = 0; x < _surface->w; x++)
			*(byte*)_surface->getBasePtr(x, y) = (byte)stream.getBits(8);
}

void ImageAsset::decodePPIC1(Common::BitStream & stream) {
	decodeHuffGraphic(PPIC1Huff, stream);
}

void ImageAsset::decodePPIC2(Common::BitStream & stream) {
	decodeHuffGraphic(PPIC2Huff, stream);
}

void ImageAsset::decodePPIC3(Common::BitStream & stream) {

}

void ImageAsset::decodeHuffGraphic(const PPICHuff & huff, Common::BitStream & stream) {
	byte flags = 0;
	_walkRepeat = 0;
	_walkLast = 0;
	if (_bitWidth & 3)
		flags = stream.getBits(5);
	else
		flags = stream.getBits(4) << 1;

	byte odd = 0;
	byte blank = _bitWidth & 0xf;
	if (blank) {
		blank >>= 2;
		odd = blank & 1;
		blank = 2 - (blank >> 1);
	}

	uint pos = 0;
	for (uint y = 0; y < _bitHeight; y++) {
		uint x = 0;
		for (; x < _bitWidth >> 3; x++) {
			byte hi = walkHuff(huff, stream) << 4;
			_data[pos++] = walkHuff(huff, stream) | hi;
		}
		if (odd) {
			_data[pos] = walkHuff(huff, stream) << 4;
		}
		pos += blank;
	}

	uint16 edge = _bitWidth & 3;	
	if (edge) {
		pos = _rowBytes - blank;
		uint16 bits = 0;
		uint16 val = 0;
		uint16 v;
		for (uint y = 0; y < _bitHeight; y++) {
			if (flags & 1) {
				if (bits < edge) {
					v = walkHuff(huff, stream) << 4;
					val |= v >> bits;
					bits += 4;
				}
				bits -= edge;
				v = val;
				val <<= edge;
				val &= 0xFF;
			} else {
				v = stream.getBits(edge);
				v <<= 8 - edge;
			}
			if (odd)
				v >>= 4;

			_data[pos] |= v & 0xff;
			pos += _rowBytes;
		}
	}
	if (flags & 8) {
		pos = 0;
		for (uint y = 0; y < _bitHeight; y++) {
			uint16 v = 0;
			if (flags & 2) {
				for (uint x = 0; x < _rowBytes; x++)
				{
					_data[pos] ^= v;
					v = _data[pos];
					pos++;
				}
			}
			else {
				for (uint x = 0; x < _rowBytes; x++) {
					uint16 val = _data[pos] ^ v;
					val ^= (val >> 4) & 0xf;
					_data[pos] = val;
					pos++;
					v = (val << 4) & 0xff;
				}
			}
		}
	}
	if (flags & 4) {
		uint16 delta = _rowBytes * 4;
		if (flags & 2) delta *= 2;
		pos = 0;
		uint q = delta;
		for (uint i = 0;i < _surface->h * _rowBytes - delta;i++) {
			_data[q] ^= _data[pos];
			q++;
			pos++;
		}
	}
}

byte ImageAsset::walkHuff(const PPICHuff & huff, Common::BitStream & stream) {
	if (_walkRepeat) {
		_walkRepeat--;
		_walkLast = ((_walkLast << 8) & 0xFF00) | (_walkLast >> 8);
		return _walkLast & 0xFF;
	}
	uint16 dw = stream.peekBits(16);
	uint16 i = 0;
	for (;i < 16; i++) {
		if (huff.masks[i + 1] > dw)
			break;
	}
	stream.skip(huff.lens[i]);
	uint8 val = huff.symbols[i];
	if (val == 0xFF) {
		if (!stream.getBit()) {
			_walkLast &= 0xFF;
			_walkLast |= _walkLast << 8;
		}
		_walkRepeat = stream.getBits(3);
		if (_walkRepeat < 3) {
			_walkRepeat <<= 4;
			_walkRepeat |= stream.getBits(4);
			if (_walkRepeat < 8) {
				_walkRepeat <<= 8;
				_walkRepeat |= stream.getBits(8);
			}
		}
		_walkRepeat -= 2;
		_walkLast = ((_walkLast << 8) & 0xFF00) | (_walkLast >> 8);
		return _walkLast & 0xFF;
	} else {
		_walkLast <<= 8;
		_walkLast |= val;
		_walkLast &= 0xFFFF;
	}
	return val;
}

} // End of namespace MacVenture
