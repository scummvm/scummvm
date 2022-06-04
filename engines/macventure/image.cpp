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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/image.h"

namespace MacVenture {

static const PPICHuff PPIC1Huff = {
	// Masks
	{ 0x0000,0x2000,0x4000,0x5000,0x6000,0x7000,0x8000,0x9000,0xa000,
	0xb000,0xc000,0xd000,0xd800,0xe000,0xe800,0xf000,0xf800 },
	// Lens
	{ 3,3,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5 },
	// Symbols
	{ 0x00,0x0f,0x03,0x05,0x06,0x07,0x08,0x09,0x0a,0x0c,0xff,0x01,
	0x02,0x04,0x0b,0x0d,0xe }
};

static const PPICHuff PPIC2Huff = {
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
static const byte loadBits[] = {
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
	0xff
};

ImageAsset::ImageAsset(ObjID original, Container *container) {
	_id = (original * 2);
	_mask = (original * 2) + 1;

	uint imgRowBytes = 0;
	uint imgBitWidth = 0;
	uint imgBitHeight = 0;
	uint maskRowBytes = 0;
	uint maskBitWidth = 0;
	uint maskBitHeight = 0;

	_container = container;
	decodePPIC(_id, _imgData, imgBitHeight, imgBitWidth, imgRowBytes);
	_imgRowBytes = imgRowBytes;
	_imgBitWidth = imgBitWidth;
	_imgBitHeight = imgBitHeight;

	if (_container->getItemByteSize(_mask)) {
		decodePPIC(_mask, _maskData, maskBitHeight, maskBitWidth, maskRowBytes);
	}
	_maskRowBytes = maskRowBytes;
	_maskBitWidth = maskBitWidth;
	_maskBitHeight = maskBitHeight;
}

ImageAsset::~ImageAsset() {
	debugC(3, kMVDebugImage, "~ImageAsset(%d)", _id / 2);
}

void ImageAsset::decodePPIC(ObjID id, Common::Array<byte> &data, uint &bitHeight, uint &bitWidth, uint &rowBytes) {
	ObjID realID = id;
	uint32 size = _container->getItemByteSize(id);
	if (size < 2) {
		rowBytes = 0;
		bitHeight = 0;
		bitWidth = 0;
		return;
	}
	if (size == 2) {
		Common::SeekableReadStream *newItemStream = _container->getItem(id);
		realID = newItemStream->readUint16BE();
		delete newItemStream;
	}
	Common::SeekableReadStream *baseStream = _container->getItem(realID);
	Common::BitStream32BEMSB stream(baseStream);

	uint8 mode = stream.getBits<3>();
	int w, h;
	if (stream.getBit()) {
		h = stream.getBits<10>();
	} else {
		h = stream.getBits<6>();
	}

	if (stream.getBit()) {
		w = stream.getBits<10>();
	} else {
		w = stream.getBits<6>();
	}

	rowBytes = ((w + 0xF) >> 3) & 0xFFFE;
	bitWidth = w;
	bitHeight = h;

	for (uint i = 0; i < rowBytes * h; i++) {
		data.push_back(0);
	}

	switch (mode) {
	case MacVenture::kPPIC0:
		decodePPIC0(stream, data, bitHeight, bitWidth, rowBytes);
		break;
	case MacVenture::kPPIC1:
		decodePPIC1(stream, data, bitHeight, bitWidth, rowBytes);
		break;
	case MacVenture::kPPIC2:
		decodePPIC2(stream, data, bitHeight, bitWidth, rowBytes);
		break;
	case MacVenture::kPPIC3:
		decodePPIC3(stream, data, bitHeight, bitWidth, rowBytes);
		break;
	default:
		warning("decodePPIC(): Unknown mode!");
		break;
	}

	delete baseStream;
}

void ImageAsset::decodePPIC0(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	uint words = bitWidth >> 4;
	uint bytes = bitWidth & 0xF;
	uint v = 0;
	uint p = 0;
	for (uint y = 0; y < bitHeight; y++) {
		for (uint x = 0; x < words; x++) {
			v = stream.peekBits<32>();
			stream.skip(16);
			v >>= 16 - (stream.pos() % 8);
			data[p] = (v >> 8) & 0xff; p++;
			data[p] = v & 0xff; p++;
		}
		if (bytes) {
			v = stream.getBits(bytes);
			v <<= 16 - bytes;
			data[p] = (v >> 8) & 0xff; p++;
			data[p] = v & 0xff; p++;
		}
	}

}

void ImageAsset::decodePPIC1(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	decodeHuffGraphic(PPIC1Huff, stream, data, bitHeight, bitWidth, rowBytes);
}

void ImageAsset::decodePPIC2(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	decodeHuffGraphic(PPIC2Huff, stream, data, bitHeight, bitWidth, rowBytes);
}

void ImageAsset::decodePPIC3(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	// We need to load the huffman from the PPIC itself
	PPICHuff huff;
	uint16 v, bits;
	uint16 load = 0;
	while ((bits = loadBits[load++]) != 0xFF) {
		v = stream.getBits(bits);
		while ((bits = loadBits[load++]) != 0xFF) {
			huff.symbols[loadBits[load++]] = v % bits;
			v = (bits != 0) ? (v / bits) : 0;
		}
		huff.symbols[loadBits[load++]] = v;
	}
	huff.symbols[0x10] = 0;
	for (uint i = 0x10; i > 0; i--) {
		for (uint j = i; j <= 0x10; j++) {
			if (huff.symbols[j] >= huff.symbols[i - 1]) {
				huff.symbols[j]++;
			}
		}
	}

	for (int i = 0x10; i >= 0; i--) {
		if (huff.symbols[i] == 0x10) {
			huff.symbols[i] = 0xff;
			break;
		}
	}

	bits = stream.getBits<2>() + 1;
	uint16 mask = 0;
	for (uint i = 0; i < 0xf; i++) {
		if (i) {
			while (!stream.getBit()) {
				bits++;
			}
		}
		huff.lens[i] = bits;
		huff.masks[i] = mask;
		mask += 1 << (16 - bits);
	}
	huff.masks[0xf] = mask;
	while (mask&(1 << (16 - bits))) {
		bits++;
	}
	huff.masks[0x10] = mask | (1 << (16 - bits));
	huff.lens[0xf] = bits;
	huff.lens[0x10] = bits;

	decodeHuffGraphic(huff, stream, data, bitHeight, bitWidth, rowBytes);
}

void ImageAsset::decodeHuffGraphic(const PPICHuff &huff, Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	byte flags = 0;
	_walkRepeat = 0;
	_walkLast = 0;
	if (bitWidth & 3) {
		flags = stream.getBits<5>();
	} else {
		flags = stream.getBits<4>() << 1;
	}

	byte odd = 0;
	byte blank = bitWidth & 0xf;
	if (blank) {
		blank >>= 2;
		odd = blank & 1;
		blank = 2 - (blank >> 1);
	}

	uint16 pos = 0;
	for (uint y = 0; y < bitHeight; y++) {
		uint16 x = 0;
		for (; x < bitWidth >> 3; x++) {
			byte hi = walkHuff(huff, stream) << 4;
			data[pos++] = walkHuff(huff, stream) | hi;
		}
		if (odd) {
			data[pos] = walkHuff(huff, stream) << 4;
		}
		pos += blank;
	}

	uint16 edge = bitWidth & 3;
	if (edge) {
		pos = rowBytes - blank;
		uint16 bits = 0;
		uint16 val = 0;
		uint16 v;
		for (uint y = 0; y < bitHeight; y++) {
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
			pos += rowBytes;
		}
	}
	if (flags & 8) {
		pos = 0;
		for (uint y = 0; y < bitHeight; y++) {
			uint16 v = 0;
			if (flags & 2) {
				for (uint x = 0; x < rowBytes; x++) {
					data[pos] ^= v;
					v = data[pos];
					pos++;
				}
			} else {
				for (uint x = 0; x < rowBytes; x++) {
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
		uint16 delta = rowBytes * 4;
		if (flags & 2) {
			delta *= 2;
		}
		pos = 0;
		uint q = delta;
		for (uint i = 0; i < bitHeight * rowBytes - delta; i++) {
			data[q] ^= data[pos];
			q++;
			pos++;
		}
	}
}

byte ImageAsset::walkHuff(const PPICHuff &huff, Common::BitStream32BEMSB &stream) {
	if (_walkRepeat) {
		_walkRepeat--;
		_walkLast = ((_walkLast << 8) & 0xFF00) | (_walkLast >> 8);
		return _walkLast & 0xFF;
	}
	uint16 dw = stream.peekBits<16>();
	uint16 i = 0;
	for (;i < 16; i++) {
		if (huff.masks[i + 1] > dw) {
			break;
		}
	}
	stream.skip(huff.lens[i]);
	uint8 val = huff.symbols[i];
	if (val == 0xFF) {
		if (!stream.getBit()) {
			_walkLast &= 0xFF;
			_walkLast |= _walkLast << 8;
		}
		_walkRepeat = stream.getBits<3>();
		if (_walkRepeat < 3) {
			_walkRepeat <<= 4;
			_walkRepeat |= stream.getBits<4>();
			if (_walkRepeat < 8) {
				_walkRepeat <<= 8;
				_walkRepeat |= stream.getBits<8>();
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

void ImageAsset::blitInto(Graphics::ManagedSurface *target, int x, int y, BlitMode mode) {
	if (mode == kBlitDirect) {
		blitDirect(target, x, y, _imgData, _imgBitHeight, _imgBitWidth, _imgRowBytes);
	} else if (mode < kBlitXOR) {
		if (_container->getItemByteSize(_mask)) { // Has mask
			switch (mode) {
			case MacVenture::kBlitBIC:
				blitBIC(target, x, y, _maskData, _maskBitHeight, _maskBitWidth, _maskRowBytes);
				break;
			case MacVenture::kBlitOR:
				blitOR(target, x, y, _maskData, _maskBitHeight, _maskBitWidth, _maskRowBytes);
				break;
			default:
				break;
			}
		} else if (_container->getItemByteSize(_id)) {
			switch (mode) {
			case MacVenture::kBlitBIC:
				target->fillRect(Common::Rect(x, y, x + _imgBitWidth, y + _imgBitHeight), kColorWhite);
				break;
			case MacVenture::kBlitOR:
				target->fillRect(Common::Rect(x, y, x + _imgBitWidth, y + _imgBitHeight), kColorBlack);
				break;
			default:
				break;
			}
		}

		if (_container->getItemByteSize(_id) && mode > 0) {
			blitXOR(target, x, y, _imgData, _imgBitHeight, _imgBitWidth, _imgRowBytes);
		}
	}
}

bool ImageAsset::isPointInside(Common::Point point) {
	if (point.x >= _maskBitWidth || point.y >= _maskBitHeight) {
		return false;
	}
	if (_maskData.empty()) {
		return false;
	}
	// We see if the point lands on the mask.
	uint pix = _maskData[(point.y * _maskRowBytes) + (point.x >> 3)] & (1 << (7 - (point.x & 7)));
	return pix != 0;
}

bool ImageAsset::isRectInside(Common::Rect rect) {
	if (_maskData.empty()) {
		return (rect.width() > 0 && rect.height() > 0);
	}

	for (int y = rect.top; y < rect.top + rect.height(); y++) {
		uint bmpofs = y * _maskRowBytes;
		byte pix;
		for (int x = rect.left; x < rect.left + rect.width(); x++) {
			pix = _maskData[bmpofs + (x >> 3)] & (1 << (7 - (x & 7)));
			if (pix) {
				return true;
			}
		}
	}
	return false;
}

int ImageAsset::getWidth() {
	if (_imgData.size() == 0) {
		return 0;
	}
	return MAX(0, (int)_imgBitWidth);
}

int ImageAsset::getHeight() {
	if (_imgData.size() == 0) {
		return 0;
	}
	return MAX(0, (int)_imgBitHeight);
}

void ImageAsset::blitDirect(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	uint sx, sy, w, h;
	calculateSectionToDraw(target, ox, oy, bitWidth, bitHeight, sx, sy, w, h);

	for (uint y = 0; y < h; y++) {
		uint bmpofs = (y + sy) * rowBytes;
		byte pix = 0;
		for (uint x = 0; x < w; x++) {
			assert(ox + x <= (uint)target->w);
			assert(oy + y <= (uint)target->h);
			pix = data[bmpofs + ((x + sx) >> 3)] & (1 << (7 - ((x + sx) & 7)));
			pix = pix ? kColorBlack : kColorWhite;
			*((byte *)target->getBasePtr(ox + x, oy + y)) = pix;
		}
	}
}

void ImageAsset::blitBIC(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	uint sx, sy, w, h;
	calculateSectionToDraw(target, ox, oy, bitWidth, bitHeight, sx, sy, w, h);

	for (uint y = 0; y < h; y++) {
		uint bmpofs = (y + sy) * rowBytes;
		byte pix = 0;
		for (uint x = 0; x < w; x++) {
			assert(ox + x <= (uint)target->w);
			assert(oy + y <= (uint)target->h);
			pix = data[bmpofs + ((x + sx) >> 3)] & (1 << (7 - ((x + sx) & 7)));
			if (pix) {
				*((byte *)target->getBasePtr(ox + x, oy + y)) = kColorWhite;
			}
		}
	}
}

void ImageAsset::blitOR(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	uint sx, sy, w, h;
	calculateSectionToDraw(target, ox, oy, bitWidth, bitHeight, sx, sy, w, h);

	for (uint y = 0; y < h; y++) {
		uint bmpofs = (y + sy) * rowBytes;
		byte pix = 0;
		for (uint x = 0; x < w; x++) {
			assert(ox + x <= (uint)target->w);
			assert(oy + y <= (uint)target->h);
			pix = data[bmpofs + ((x + sx) >> 3)] & (1 << (7 - ((x + sx) & 7)));
			if (pix) {
				*((byte *)target->getBasePtr(ox + x, oy + y)) = kColorBlack;
			}
		}
	}
}

void ImageAsset::blitXOR(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes) {
	uint sx, sy, w, h;
	calculateSectionToDraw(target, ox, oy, bitWidth, bitHeight, sx, sy, w, h);

	for (uint y = 0; y < h; y++) {
		uint bmpofs = (y + sy) * rowBytes;
		byte pix = 0;
		for (uint x = 0; x < w; x++) {
			pix = data[bmpofs + ((x + sx) >> 3)] & (1 << (7 - ((x + sx) & 7)));
			if (pix) { // We need to xor
				assert(ox + x <= (uint)target->w);
				assert(oy + y <= (uint)target->h);
				byte p = *((byte *)target->getBasePtr(ox + x, oy + y));
				*((byte *)target->getBasePtr(ox + x, oy + y)) =
					(p == kColorWhite) ? kColorBlack : kColorWhite;
			}
		}
	}
}

void ImageAsset::calculateSectionToDraw(Graphics::ManagedSurface *target, int &ox, int &oy, uint bitWidth, uint bitHeight, uint &sx, uint &sy, uint &w, uint &h) {

	calculateSectionInDirection(target->w, bitWidth, ox, sx, w);
	calculateSectionInDirection(target->h, bitHeight, oy, sy, h);

	assert(w <= (uint)target->w);
	assert((int)w >= 0);
	assert(w <= bitWidth);
	assert(h <= (uint)target->h);
	assert((int)h >= 0);
	assert(h <= bitHeight);
}

void ImageAsset::calculateSectionInDirection(uint targetWhole, uint originWhole, int &originPosition, uint &startPosition, uint &blittedWhole) {
	startPosition = 0;
	blittedWhole = originWhole;
	if (originPosition < 0) {
		if (ABS(originPosition) > (int)blittedWhole) {
			blittedWhole = 0;
		} else {
			blittedWhole -= -originPosition;
		}
		startPosition = -originPosition;
		originPosition = 0;
	}
	if (originPosition + blittedWhole > targetWhole) {
		if (originPosition > (int)targetWhole) {
			blittedWhole = 0;
		} else {
			blittedWhole = targetWhole - originPosition;
		}
	}
}

} // End of namespace MacVenture
