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
#include "common/file.h"
#include "common/memstream.h"
#include "common/endian.h"

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

namespace {
// A simple implementation of memmem, which is a non-standard GNU extension.
const void *local_memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len) {
	if (needle_len == 0) {
		return haystack;
	}
	if (haystack_len < needle_len) {
		return nullptr;
	}
	const char *h = (const char *)haystack;
	for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
		if (memcmp(h + i, needle, needle_len) == 0) {
			return h + i;
		}
	}
	return nullptr;
}
} // namespace

Common::SeekableReadStream *DrillerEngine::decryptFileAtariVirtualWorlds(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename)) {
		error("Failed to open %s", filename.toString().c_str());
	}
	const int size = file.size();
	byte *data = (byte *)malloc(size);
	file.read(data, size);

	int start = 0;
	int valid_offset = -1;
	int chunk_size = 0;

	while (true) {
		const byte *found = (const byte *)local_memmem(data + start, size - start, "CBCP", 4);
		if (!found) break;

		int idx = found - data;
		if (idx + 8 <= size) {
			int sz = READ_BE_UINT32(data + idx + 4);
			if (sz > 0 && sz < size + 0x20000) {
				valid_offset = idx;
				chunk_size = sz;
			}
		}
		start = idx + 1;
	}

	if (valid_offset == -1) {
		error("No valid CBCP chunk found in %s", filename.toString().c_str());
	}

	const byte *payload = data + valid_offset + 8;
	const int payload_size = chunk_size;

	if (payload_size < 12) {
		error("Payload too short in %s", filename.toString().c_str());
	}

	uint32 bit_buf_init = READ_BE_UINT32(payload + payload_size - 12);
	uint32 checksum_init = READ_BE_UINT32(payload + payload_size - 8);
	uint32 decoded_size = READ_BE_UINT32(payload + payload_size - 4);

	byte *out_buffer = (byte *)malloc(decoded_size);
	int dst_idx = decoded_size;

	struct BitStream {
		const byte *_src_data;
		int _src_idx;
		uint32 _bit_buffer;
		uint32 _checksum;
		int _refill_carry;

		BitStream(const byte *src_data, int start_idx, uint32 bit_buffer, uint32 checksum) :
			_src_data(src_data), _src_idx(start_idx), _bit_buffer(bit_buffer), _checksum(checksum), _refill_carry(0) {}

		void refill() {
			if (_src_idx < 0) {
				_refill_carry = 0;
				_bit_buffer = 0x80000000;
				return;
			}
			uint32 val = READ_BE_UINT32(_src_data + _src_idx);
			_src_idx -= 4;
			_checksum ^= val;
			_refill_carry = val & 1;
			_bit_buffer = (val >> 1) | 0x80000000;
		}

		int getBits(int count) {
			uint32 result = 0;
			for (int i = 0; i < count; ++i) {
				int carry = _bit_buffer & 1;
				_bit_buffer >>= 1;
				if (_bit_buffer == 0) {
					refill();
					carry = _refill_carry;
				}
				result = (result << 1) | carry;
			}
			return result;
		}
	};

	int src_idx = payload_size - 16;
	uint32 checksum = checksum_init ^ bit_buf_init;
	BitStream bs(payload, src_idx, bit_buf_init, checksum);

	while (dst_idx > 0) {
		if (bs.getBits(1) == 0) {
			if (bs.getBits(1) == 1) {
				int offset = bs.getBits(8);
				for (int i = 0; i < 2; ++i) {
					dst_idx--;
					if (dst_idx >= 0) {
						out_buffer[dst_idx] = out_buffer[dst_idx + offset];
					}
				}
			} else {
				int count = bs.getBits(3) + 1;
				for (int i = 0; i < count; ++i) {
					dst_idx--;
					if (dst_idx >= 0) {
						out_buffer[dst_idx] = bs.getBits(8);
					}
				}
			}
		} else {
			int tag = bs.getBits(2);
			if (tag == 3) {
				int count = bs.getBits(8) + 9;
				for (int i = 0; i < count; ++i) {
					dst_idx--;
					if (dst_idx >= 0) {
						out_buffer[dst_idx] = bs.getBits(8);
					}
				}
			} else if (tag == 2) {
				int length = bs.getBits(8) + 1;
				int offset = bs.getBits(12);
				for (int i = 0; i < length; ++i) {
					dst_idx--;
					if (dst_idx >= 0) {
						out_buffer[dst_idx] = out_buffer[dst_idx + offset];
					}
				}
			} else {
				int bits_offset = 9 + tag;
				int length = 3 + tag;
				int offset = bs.getBits(bits_offset);
				for (int i = 0; i < length; ++i) {
					dst_idx--;
					if (dst_idx >= 0) {
						out_buffer[dst_idx] = out_buffer[dst_idx + offset];
					}
				}
			}
		}
	}
	free(data);
	return new Common::MemoryReadStream(out_buffer, decoded_size);
}

Common::SeekableReadStream *DrillerEngine::decryptFileAtari(const Common::Path &filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	byte *a6 = encryptedBuffer + 0x118;
	byte *a5 = encryptedBuffer + size - 4;
	uint64 key = 0xb9f11bce;

	while (a6 <= a5) {
		uint64 d0 = (a6[0] << 24) | (a6[1] << 16) | (a6[2] << 8) | a6[3];
		d0 += key;
		d0 = uint32(d0);

		a6[0] = byte((d0 >> 24) & 0xFF);
		a6[1] = byte((d0 >> 16) & 0xFF);
		a6[2] = byte((d0 >> 8) & 0xFF);
		a6[3] = byte(d0 & 0xFF);

		key += 0x51684624;
		key = uint32(key);
		a6 += 4;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

void DrillerEngine::loadAssetsAtariFullGame() {
	Common::SeekableReadStream *stream = nullptr;
	if (_variant & GF_ATARI_RETAIL) {
		stream = decryptFileAtari("x.prg");

		_border = loadAndConvertNeoImage(stream, 0x14b96);
		_borderExtra = loadAndConvertNeoImage(stream, 0x1c916);
		_title = loadAndConvertNeoImage(stream, 0x3f6);

		loadFonts(stream, 0x8a92);
		Common::Array<Graphics::ManagedSurface *> chars;
		chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, stream, 0x8a92 + 112 * 33 + 1, 100);
		_fontSmall = Font(chars);
		_fontSmall.setCharWidth(5);

		loadMessagesFixedSize(stream, 0xda22, 14, 20);
		loadGlobalObjects(stream, 0xd116, 8);
		load8bitBinary(stream, 0x2afb8, 16);
		loadPalettes(stream, 0x2ab76);
		loadSoundsFx(stream, 0x30da6 + 0x147c, 25);
	} else if (_variant & GF_ATARI_BUDGET) {
		Common::File file;
		file.open("x.prg");

		if (!file.isOpen()) {
			stream = decryptFileAtariVirtualWorlds("dril.all");
		} else
			stream = &file;

		if (isSpaceStationOblivion()) {
			_border = loadAndConvertNeoImage(&file, 0x13544);
			byte palette[16 * 3];
			for (int i = 0; i < 16; i++) { // gray scale palette
				palette[i * 3 + 0] = i * (255 / 16);
				palette[i * 3 + 1] = i * (255 / 16);
				palette[i * 3 + 2] = i * (255 / 16);
			}
			_title = loadAndConvertNeoImage(&file, 0x10, palette);

			loadFonts(&file, 0x8a32 - 0x1d6);
			loadMessagesFixedSize(&file, 0xc5d8 - 0x1da, 14, 20);
			loadGlobalObjects(&file, 0xbccc - 0x1da, 8);
			load8bitBinary(&file, 0x29b3c - 0x1d6, 16);
			loadPalettes(&file, 0x296fa - 0x1d6);
			loadSoundsFx(&file, 0x30da6 - 0x1d6, 25);
		} else {
			_border = loadAndConvertNeoImage(stream, 0x1371a);
			_title = loadAndConvertNeoImage(stream, 0x396);

			loadFonts(stream, 0x8a32);

			Common::Array<Graphics::ManagedSurface *> chars;
			chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, stream, 0x8a32 + 112 * 33 + 1, 100);
			_fontSmall = Font(chars);
			_fontSmall.setCharWidth(5);

			loadMessagesFixedSize(stream, 0xc5d8, 14, 20);
			loadGlobalObjects(stream, 0xbccc, 8);
			load8bitBinary(stream, 0x29b3c, 16);
			loadPalettes(stream, 0x296fa);
			loadSoundsFx(stream, 0x30da6, 25);
		}
	} else
		error("Unknown Atari ST Driller variant");

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 14);
	}

	_indicators.push_back(loadBundledImage("driller_tank_indicator_0_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_1_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_2_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_3_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_ship_indicator_Amiga", false));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void DrillerEngine::loadAssetsAtariDemo() {
	Common::File file;
	file.open("lift.neo");
	if (!file.isOpen())
		error("Failed to open 'lift.neo' file");

	_title = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("console.neo");
	if (!file.isOpen())
		error("Failed to open 'console.neo' file");

	_border = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("demo.cmd");
	if (!file.isOpen())
		error("Failed to open 'demo.cmd' file");

	loadDemoData(&file, 0, 0x1000);

	file.close();
	if (_variant & GF_ATARI_MAGAZINE_DEMO) {
		file.open("auto_x.prg");
		if (!file.isOpen())
			error("Failed to open 'auto_x.prg' file");
		_demoMode = false;
	} else {
		file.open("x.prg");
		if (!file.isOpen())
			error("Failed to open 'x.prg' file");
	}

	if (_variant & GF_ATARI_MAGAZINE_DEMO) {
		loadMessagesFixedSize(&file, 0x40d2, 14, 20);
		loadGlobalObjects(&file, 0x3e88, 8);

		loadFonts(&file, 0x7ee);
		Common::Array<Graphics::ManagedSurface *> chars;
		chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, &file, 0x7ee + 112 * 33 + 1, 100);
		_fontSmall = Font(chars);
		_fontSmall.setCharWidth(5);
	} else {
		loadFonts(&file, 0x7bc);
		loadMessagesFixedSize(&file, 0x3b90, 14, 20);
		loadGlobalObjects(&file, 0x3946, 8);
	}

	file.close();
	file.open("data");

	if (!file.isOpen())
		error("Failed to open 'data' file");

	load8bitBinary(&file, 0x442, 16);
	loadPalettes(&file, 0x0);

	file.close();
	file.open("soundfx");
	if (!file.isOpen())
		error("Failed to open 'soundfx' executable for AtariST demo");

	loadSoundsFx(&file, 0, 25);

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 14);
	}

	_indicators.push_back(loadBundledImage("driller_tank_indicator_0_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_1_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_2_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_3_Amiga", false));
	_indicators.push_back(loadBundledImage("driller_ship_indicator_Amiga", false));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

} // End of namespace Freescape
