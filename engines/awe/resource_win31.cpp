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

#include "awe/resource_win31.h"
#include "awe/detection.h"

namespace Awe {

static const uint8 _shuffleTable[256] = {
	0xB2, 0x91, 0x49, 0xEE, 0x8C, 0xBC, 0x16, 0x0D, 0x07, 0x87, 0xCD, 0xB6, 0x4C, 0x44, 0x22, 0xB3,
	0xAE, 0x96, 0xDF, 0x18, 0x7B, 0x28, 0x17, 0x9A, 0x74, 0x3C, 0x2E, 0x59, 0x69, 0x56, 0x38, 0x82,
	0x7F, 0x25, 0x41, 0xC6, 0xE8, 0x8A, 0x86, 0x7A, 0xB5, 0x8B, 0xA7, 0xB1, 0x2C, 0x53, 0xF0, 0x3B,
	0x20, 0xCB, 0x6F, 0x9E, 0xD9, 0x05, 0x54, 0x08, 0x4F, 0xFE, 0x32, 0x31, 0xF9, 0x50, 0xBD, 0x37,
	0x45, 0xDA, 0x46, 0x33, 0x01, 0xC5, 0x27, 0xEC, 0xE5, 0x14, 0x98, 0x70, 0xB0, 0xF8, 0x93, 0xC9,
	0xAC, 0xEB, 0xE4, 0xE1, 0xE6, 0xF7, 0xAF, 0x76, 0x0E, 0x63, 0x80, 0x83, 0x1E, 0x57, 0x47, 0x9F,
	0xC2, 0x42, 0xA5, 0xFF, 0x5B, 0xBF, 0x12, 0xFA, 0x61, 0x5E, 0x5D, 0xC8, 0x21, 0xA8, 0xB9, 0x5A,
	0x9D, 0x30, 0xD5, 0x09, 0xB7, 0x0B, 0x2F, 0xED, 0x6E, 0xA2, 0x5F, 0x6C, 0xA0, 0x95, 0x00, 0x55,
	0x75, 0x7D, 0x89, 0x97, 0x6A, 0xFB, 0x1A, 0x58, 0xDE, 0x8D, 0x4E, 0xE3, 0x4B, 0x3D, 0x15, 0x67,
	0x11, 0x5C, 0x1C, 0x71, 0x73, 0x1B, 0xD3, 0x13, 0xE7, 0x77, 0x4D, 0xD6, 0x9C, 0x1D, 0x1F, 0xEF,
	0xBB, 0x66, 0x99, 0xF6, 0x3F, 0x02, 0x7E, 0xCF, 0x2B, 0x35, 0x88, 0xBA, 0xA4, 0x40, 0x19, 0x23,
	0xC1, 0xD4, 0xD7, 0x43, 0x52, 0x34, 0xE9, 0xDC, 0x60, 0x24, 0x94, 0x6B, 0x81, 0x03, 0xC0, 0x39,
	0xBE, 0x90, 0x65, 0xFD, 0xE0, 0x2D, 0x7C, 0xEA, 0x04, 0xA6, 0xDB, 0xF3, 0xCE, 0xB4, 0xA9, 0xAA,
	0xAD, 0x64, 0xF2, 0x72, 0xD2, 0x84, 0x8E, 0xD1, 0x26, 0xA3, 0xCA, 0x4A, 0x48, 0x06, 0x0F, 0x36,
	0x85, 0xD0, 0x51, 0x6D, 0xC4, 0x3E, 0x92, 0xF1, 0xC7, 0x62, 0x79, 0xA1, 0x9B, 0x68, 0xF5, 0xE2,
	0xAB, 0x0C, 0xCC, 0x78, 0xFC, 0x2A, 0xD8, 0x3A, 0xDD, 0x8F, 0x10, 0x29, 0xF4, 0x0A, 0xB8, 0xC3
};

static uint16 decode(uint8 *p, int size, uint16 key) {
	for (int i = 0; i < size; ++i) {
		const uint8 dl = 1 + (key >> 8);
		const uint8 al = _shuffleTable[dl];
		const uint8 dh = al ^ (key & 255);
		p[i] ^= al;
		key = (dh << 8) | dl;
	}
	return key;
}

struct Bitstream {
	Common::File *_f;
	int _size;
	uint16 _bits;
	int _len;

	Bitstream()
		: _f(nullptr), _size(0), _bits(0), _len(0) {
	}

	void reset(Common::File *f, int size) {
		_f = f;
		_size = size;
		_bits = 0;
		_len = 0;
	}

	uint8 readByte() {
		if (_len < 8) {
			_bits <<= 8;
			assert(_size > 0);
			--_size;
			_bits |= _f->readByte();
			_len += 8;
		}
		_len -= 8;
		return (_bits >> _len) & 255;
	}
	int readBit() {
		if (_len == 0) {
			assert(_size > 0);
			--_size;
			_bits = _f->readByte();
			_len = 8;
		}
		--_len;
		return (_bits & (1 << _len)) != 0 ? 1 : 0;
	}
};

struct LzHuffman {

	enum {
		kCharsCount = 314,
		kTableSize = kCharsCount * 2 - 1,
		kHuffmanRoot = kTableSize - 1,
		kMaxFreq = 0x8000
	};

	Bitstream _stream;
	int _child[kTableSize];
	int _freq[628];
	int _parent[943];
	unsigned char _historyBuffer[4096];

	LzHuffman() {
		memset(_child, 0, sizeof(_child));
		memset(_freq, 0, sizeof(_freq));
		memset(_parent, 0, sizeof(_parent));
		memset(_historyBuffer, 0, sizeof(_historyBuffer));
	}

	void resetHuffTables() {
		for (int i = 0; i < kCharsCount; ++i) {
			_freq[i] = 1;
			_child[i] = kTableSize + i;
			_parent[kTableSize + i] = i;
		}
		for (int i = 0, j = kCharsCount; j <= kHuffmanRoot; i += 2, ++j) {
			_freq[j] = _freq[i] + _freq[i + 1];
			_child[j] = i;
			_parent[i] = _parent[i + 1] = j;
		}
		_freq[kTableSize] = 0xFFFF;
		_parent[kHuffmanRoot] = 0;
	}
	int getHuffCode() {
		static const int base[] = { 0, 1, 4, 12, 24, 48 };
		static const int count[] = { 0, 2, 5, 9, 12, 15 };
		static const int length[] = { 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5 };
		int index = _stream.readByte();
		const int len = length[index >> 4];
		const int code = base[len] + (index - count[len] * 16) / (1 << (5 - len));
		for (int i = 0; i <= len; ++i) {
			index = (index << 1) | _stream.readBit();
		}
		return (index & 63) | (code << 6);
	}
	int decodeChar() {
		int i = _child[kHuffmanRoot];
		while (i < kTableSize) {
			i += _stream.readBit();

			if (i < kTableSize)
				i = _child[i];
		}
		i -= kTableSize;
		update(i);
		return i;
	}
	void update(int num) {
		if (_freq[kHuffmanRoot] == kMaxFreq) {
			for (int j = 0, i = 0; j < kTableSize; ++j) {
				if (_child[j] >= kTableSize) {
					_freq[i] = (_freq[j] + 1) >> 1;
					_child[i] = _child[j];
					++i;
				}
			}
			for (int j = 0, i = kCharsCount; i < kTableSize; j += 2, ++i) {
				const int f = _freq[i] = _freq[j] + _freq[j + 1];
				int index = i - 1;
				while (_freq[index] > f) {
					--index;
				}
				++index;
				const int copySize = (i - index) * sizeof(int);
				memmove(_freq + index + 1, _freq + index, copySize);
				_freq[index] = f;

				if (index + 1 < kTableSize)
					memmove(_child + index + 1, _child + index, copySize);

				_child[index] = j;
			}
			for (int i = 0; i < kTableSize; ++i) {
				const int j = _child[i];
				if (j >= kTableSize) {
					_parent[j] = i;
				} else {
					_parent[j] = _parent[j + 1] = i;
				}
			}
		}
		int p = _parent[kTableSize + num];
		do {
			++_freq[p];
			const int i = _freq[p];
			int index = p + 1;
			if (_freq[index] < i) {
				while (_freq[++index] < i) {
				}
				--index;
				_freq[p] = _freq[index];
				_freq[index] = i;
				const int k = _child[p];
				_parent[k] = index;
				if (k < kTableSize) {
					_parent[k + 1] = index;
				}
				const int j = _child[index];
				_child[index] = k;
				_parent[j] = p;
				if (j < kTableSize) {
					_parent[j + 1] = p;
				}
				_child[p] = j;
				p = index;
			}
			p = _parent[p];
		} while (p != 0);
	}
	bool decode(uint8 *out, int uncompressedSize) {
		resetHuffTables();
		memset(_historyBuffer, ' ', sizeof(_historyBuffer));
		int offset = 4078;
		int currentSize = 0;
		while (currentSize < uncompressedSize) {
			int chr = decodeChar();
			if (chr < 256) {
				*out++ = chr & 255;
				_historyBuffer[offset++] = chr;
				offset &= 0xFFF;
				++currentSize;
			} else {
				const int baseOffset = (offset - getHuffCode() - 1) & 0xFFF;
				const int size = chr - 253;
				for (int i = 0; i < size; ++i) {
					chr = _historyBuffer[(baseOffset + i) & 0xFFF];
					*out++ = chr & 255;
					_historyBuffer[offset++] = chr;
					offset &= 0xFFF;
					++currentSize;
				}
			}
		}
		return currentSize == uncompressedSize;
	}

	bool decompressEntry(Common::File &f, const Win31BankEntry *e, uint8 *out) {
		f.seek(e->offset);
		_stream.reset(&f, e->packedSize);
		return decode(out, e->size);
	}
};

const char *ResourceWin31::FILENAME = "BANK";

ResourceWin31::ResourceWin31() {
	if (!_f.open(FILENAME))
		error("Could not open BANK");
}

ResourceWin31::~ResourceWin31() {
	free(_entries);
	free(_textBuf);
}

bool ResourceWin31::readEntries() {
	uint8 buf[32];
	const int count = _f.read(buf, sizeof(buf));

	if (count == 32 && memcmp(buf, "NL\00\00", 4) == 0) {
		_entriesCount = READ_LE_UINT16(buf + 4);
		debugC(kDebugResource, "Read %d entries in win31 '%s'", _entriesCount, FILENAME);
		_entries = (Win31BankEntry *)calloc(_entriesCount, sizeof(Win31BankEntry));
		if (_entries) {
			uint16 key = READ_LE_UINT16(buf + 0x14);
			for (int i = 0; i < _entriesCount; ++i) {
				_f.read(buf, sizeof(buf));
				key = decode(buf, sizeof(buf), key);
				Win31BankEntry *e = &_entries[i];
				memcpy(e->name, buf, 16);
				const uint16 flags = READ_LE_UINT16(buf + 16);
				e->type = buf[19];
				e->size = READ_LE_UINT32(buf + 20);
				e->offset = READ_LE_UINT32(buf + 24);
				e->packedSize = READ_LE_UINT32(buf + 28);
				debugC(kDebugResource, "Res #%03d '%s' type %d size %d (%d) offset 0x%x", i, e->name, e->type, e->size, e->packedSize, e->offset);
				assert(e->size == 0 || flags == 0x80);
			}
			readStrings();
		}
	}
	return _entries != nullptr;
}

uint8 *ResourceWin31::loadFile(int num, uint8 *dst, uint32 *size) {
	bool allocated = false;

	if (num > 0 && num < _entriesCount) {
		Win31BankEntry *e = &_entries[num];
		*size = e->size;
		if (!dst) {
			allocated = true;
			dst = (uint8 *)malloc(e->size);
			if (!dst) {
				warning("Unable to allocate %d bytes", e->size);
				return nullptr;
			}
		}
		// check for unpacked data
		char name[32];
		snprintf(name, sizeof(name), "%03d_%s", num, e->name);
		Common::File f;
		if (f.open(name) && f.size() == e->size) {
			f.read(dst, e->size);
			return dst;
		}
		LzHuffman lzHuf;
		if (lzHuf.decompressEntry(_f, e, dst)) {
			return dst;
		}
	}
	warning("Unable to load resource #%d", num);

	if (allocated)
		free(dst);

	return nullptr;
}

void ResourceWin31::readStrings() {
	uint32 len, offset = 0;
	_textBuf = loadFile(148, nullptr, &len);
	while (true) {
		const uint32 sep = READ_LE_UINT32(_textBuf + offset); offset += 4;
		const uint16 num = sep >> 16;
		if (num == 0xFFFF) {
			break;
		}
		if (num < ARRAYSIZE(_stringsTable) && _stringsTable[num] == nullptr) {
			_stringsTable[num] = (const char *)_textBuf + offset;
		}
		while (offset < len && _textBuf[offset++] != 0)
			;
		// strings are not always '\0' terminated
		if (_textBuf[offset + 1] != 0) {
			--offset;
		}
	}
}

const char *ResourceWin31::getString(int num) const {
	return _stringsTable[num];
}

const char *ResourceWin31::getMusicName(int num) const {
	switch (num) {
	case 7:
		return "y.mid";
	case 138:
		return "X.mid";
	default:
		break;
	}
	return nullptr;
}

} // namespace Awe
