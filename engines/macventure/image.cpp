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

// Used to load the huffman table in PPIC3 decoding
byte loadBits[] = {
		0x08, 0x0f, 0x02, 0xff, 0x00,
		0x04, 0xff, 0x01,
		0x07, 0x09, 0x08, 0xff, 0x03,
		0x04, 0xff, 0x04,
		0x0a, 0x07, 0x0a, 0x0b, 0x06, 0xff, 0x05,
		0x06, 0x06, 0x0b, 0xff, 0x07,
		0x03, 0xff, 0x09,
		0x04, 0x03, 0x0e, 0xff, 0x0c,
		0x02, 0xff, 0x0d,
		0x01, 0xff, 0x0f,
		0xff };

ImageAsset::ImageAsset(ObjID original, Container * container) {
	_id = (original * 2);
	_mask = (original * 2) + 1;

	_container = container;	
	decodePPIC(_id, _imgData);

	if (_container->getItemByteSize(_mask)) // Has mask
		decodePPIC(_mask, _maskData);
}

ImageAsset::~ImageAsset() {
}

void ImageAsset::decodePPIC(ObjID id, Common::Array<byte> &data) {
	ObjID realID = id;
	uint32 size = _container->getItemByteSize(id);
	if (size == 2 || size == 0) {
		realID = _container->getItem(id)->readUint16BE();
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

	for (uint i = 0; i < _rowBytes * h; i++) {
		data.push_back(0);
	}

	switch (mode)
	{
	case MacVenture::kPPIC0:
		decodePPIC0(stream, data);
		break;
	case MacVenture::kPPIC1:
		decodePPIC1(stream, data);
		break;
	case MacVenture::kPPIC2:
		decodePPIC2(stream, data);
		break;
	case MacVenture::kPPIC3:
		decodePPIC3(stream, data);
		break;
	}
}

void ImageAsset::decodePPIC0(Common::BitStream & stream, Common::Array<byte> &data) {
	warning("Untested loading function: decode PPIC0");
	uint words = _bitWidth >> 4;
	for (uint y = 0; y <_bitHeight; y++)
		for (uint x = 0; x < words; x++)
			data[y * words + x] = (byte)stream.getBits(8);
}

void ImageAsset::decodePPIC1(Common::BitStream & stream, Common::Array<byte> &data) {
	decodeHuffGraphic(PPIC1Huff, stream, data);
}

void ImageAsset::decodePPIC2(Common::BitStream & stream, Common::Array<byte> &data) {
	decodeHuffGraphic(PPIC2Huff, stream, data);
}

void ImageAsset::decodePPIC3(Common::BitStream & stream, Common::Array<byte> &data) {
	// We need to load the huffman from the PPIC itself
	PPICHuff huff;
	uint16 v, bits;
	uint16 load = 0;
	while ((bits = loadBits[load++]) != 0xFF) {
		v = stream.getBits(bits);
		while ((bits = loadBits[load++]) != 0xFF) {
			huff.symbols[loadBits[load++]] = v % bits;
			v = (v / bits) | 0;
		}
		huff.symbols[loadBits[load++]] = v;
	}
	huff.symbols[0x10] = 0;
	for (uint i = 0x10; i > 0; i--)
		for (uint j = i; j <= 0x10; j++)
			if (huff.symbols[j] >= huff.symbols[i - 1])
				huff.symbols[j]++;

	for (uint i = 0x10; i >= 0; i--) {
		if (huff.symbols[i] == 0x10) {
			huff.symbols[i] = 0xff;
			break;
		}
	}

	bits = stream.getBits(2) + 1;
	uint16 mask = 0;
	for (uint i = 0; i < 0xf; i++) {
		if (i)
			while (!stream.getBit()) bits++;
		huff.lens[i] = bits;
		huff.masks[i] = mask;
		mask += 1 << (16 - bits);
	}
	huff.masks[0xf] = mask;
	while (mask&(1 << (16 - bits))) bits++;
	huff.masks[0x10] = mask | (1 << (16 - bits));
	huff.lens[0xf] = bits;
	huff.lens[0x10] = bits;

	decodeHuffGraphic(huff, stream, data);
}

void ImageAsset::decodeHuffGraphic(const PPICHuff & huff, Common::BitStream & stream, Common::Array<byte> &data) {
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
			data[pos++] = walkHuff(huff, stream) | hi;
		}
		if (odd) {
			data[pos] = walkHuff(huff, stream) << 4;
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

			data[pos] |= v & 0xff;
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
					data[pos] ^= v;
					v = data[pos];
					pos++;
				}
			}
			else {
				for (uint x = 0; x < _rowBytes; x++) {
					uint16 val = data[pos] ^ v;
					val ^= (val >> 4) & 0xf;
					data[pos] = val;
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
		for (uint i = 0;i < _bitHeight * _rowBytes - delta;i++) {
			data[q] ^= data[pos];
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

void ImageAsset::blitInto(Graphics::ManagedSurface *target, uint32 x, uint32 y, BlitMode mode) {
	if (mode == kBlitDirect) {
		blitDirect(target, x, y, _imgData);
	} else if (mode < kBlitXOR){
		if (_container->getItemByteSize(_mask)) { // Has mask
			switch (mode) {
			case MacVenture::kBlitBIC:
				blitBIC(target, x, y, _maskData);
				break;
			case MacVenture::kBlitOR:
				blitOR(target, x, y, _maskData);
				break;
			default:
				break;
			}
		} else if (_container->getItemByteSize(_id)) {
			switch (mode) {
			case MacVenture::kBlitBIC:
				target->fillRect(Common::Rect(x, y, x + _bitWidth, y + _bitHeight * 2), kColorWhite);
				break;
			case MacVenture::kBlitOR:
				target->fillRect(Common::Rect(x, y, x + _bitWidth, y + _bitHeight * 2), kColorBlack);
				break;
			default:
				break;
			}
		}

		if (_container->getItemByteSize(_id) && mode > 0) {
			blitXOR(target, x, y, _imgData);
		}
	}	
}

bool ImageAsset::isPointInside(Common::Point point) {
	if (point.x >= _bitWidth || point.y >= _bitHeight) return false;
	if (_maskData.empty()) return false;
	// We see if the point lands on the mask.
	uint pix = _maskData[(point.y * _rowBytes) + (point.x >> 3)] & (1 << (7 - (point.x & 7)));
	return pix != 0;
}

void ImageAsset::blitDirect(Graphics::ManagedSurface * target, uint32 ox, uint32 oy, const Common::Array<byte>& data) {
	/*
	if (_bitWidth == 0 || _bitHeight == 0) return;
	uint w = _bitWidth;
	uint h = _bitHeight;
	uint sx = 0;
	uint sy = 0;
	if (ox<0) { sx = -ox; ox = 0; }
	if (oy<0) { sy = -oy; oy = 0; }
	if (w + ox >= target->w) w = target->w - ox;
	if (h + oy >= target->h) h = target->h - oy;
	if (w == 0 || h == 0) return;
	*/
	
	for (uint y = 0; y < _bitHeight; y++) {
		uint bmpofs = y * _rowBytes;
		byte pix = 0;
		for (uint x = 0; x < _bitWidth; x++) {
			pix = data[bmpofs + (x >> 3)] & (1 << (7 - (x & 7)));
			pix = pix ? kColorBlack : kColorWhite;
			*((byte *)target->getBasePtr(ox + x, oy + y)) = pix;
		}
	}
}

void ImageAsset::blitBIC(Graphics::ManagedSurface * target, uint32 ox, uint32 oy, const Common::Array<byte> &data) {
	/*
	if (_bitWidth == 0 || _bitHeight == 0) return;
	uint w = _bitWidth;
	uint h = _bitHeight;
	uint sx = 0;
	uint sy = 0;
	if (ox<0) { sx = -ox; ox = 0; }
	if (oy<0) { sy = -oy; oy = 0; }
	if (w + ox >= target->w) w = target->w - ox;
	if (h + oy >= target->h) h = target->h - oy;
	if (w == 0 || h == 0) return;
	*/
	for (uint y = 0; y < _bitHeight; y++) {
		uint bmpofs = y * _rowBytes;
		byte pix = 0;
		for (uint x = 0; x < _bitWidth; x++) {
			pix = data[bmpofs + (x >> 3)] & (1 << (7 - (x & 7)));			
			if (pix) *((byte *)target->getBasePtr(ox + x, oy + y)) = kColorWhite;
		}
	}
}

void ImageAsset::blitOR(Graphics::ManagedSurface * target, uint32 ox, uint32 oy, const Common::Array<byte> &data) {
	/*
	if (_bitWidth == 0 || _bitHeight == 0) return;
	uint w = _bitWidth;
	uint h = _bitHeight;
	uint sx = 0;
	uint sy = 0;
	if (ox<0) { sx = -ox; ox = 0; }
	if (oy<0) { sy = -oy; oy = 0; }
	if (w + ox >= target->w) w = target->w - ox;
	if (h + oy >= target->h) h = target->h - oy;
	if (w == 0 || h == 0) return;
	*/
	for (uint y = 0; y < _bitHeight; y++) {
		uint bmpofs = y * _rowBytes;
		byte pix = 0;
		for (uint x = 0; x < _bitWidth; x++) {
			pix = data[bmpofs + (x >> 3)] & (1 << (7 - (x & 7)));

			if (pix) *((byte *)target->getBasePtr(ox + x, oy + y)) = kColorBlack;
		}
	}
}

void ImageAsset::blitXOR(Graphics::ManagedSurface * target, uint32 ox, uint32 oy, const Common::Array<byte> &data) {
	/*
	if (_bitWidth == 0 || _bitHeight == 0) return;
	uint w = _bitWidth;
	uint h = _bitHeight;
	uint sx = 0;
	uint sy = 0;
	if (ox<0) { sx = -ox; ox = 0; }
	if (oy<0) { sy = -oy; oy = 0; }
	if (w + ox >= target->w) w = target->w - ox;
	if (h + oy >= target->h) h = target->h - oy;
	if (w == 0 || h == 0) return;
	*/
	for (uint y = 0;y < _bitHeight; y++) {
		uint bmpofs = y * _rowBytes;
		byte pix = 0;
		for (uint x = 0; x < _bitWidth; x++) {
			pix = data[bmpofs + (x >> 3)] & (1 << (7 - (x & 7)));

			if (pix) { // We need to xor
				byte p = *((byte *)target->getBasePtr(ox + x, oy + y));

				*((byte *)target->getBasePtr(ox + x, oy + y)) = 
					(p == kColorWhite) ? kColorBlack : kColorWhite;
			}
		}
	}
}

} // End of namespace MacVenture
