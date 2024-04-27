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

#include "common/std/algorithm.h"
#include "ags/shared/font/wfn_font.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

static const char *WFN_FILE_SIGNATURE = "WGT Font File  ";
static const size_t  WFN_FILE_SIG_LENGTH = 15;
static const size_t  MinCharDataSize = sizeof(uint16_t) * 2;

WFNChar::WFNChar()
	: Width(0)
	, Height(0)
	, Data(nullptr) {
}

void WFNChar::RestrictToBytes(size_t bytes) {
	if (bytes < GetRequiredPixelSize())
		Height = static_cast<uint16_t>(bytes / GetRowByteCount());
}

const WFNChar &WFNFont::GetChar(uint16_t code) const {
	return code < _refs.size() ? *_refs[code] : _G(emptyChar);
}

void WFNFont::Clear() {
	_refs.clear();
	_items.clear();
	_pixelData.clear();
}

WFNError WFNFont::ReadFromFile(Stream *in, const soff_t data_size) {
	Clear();

	const soff_t used_data_size = data_size > 0 ? data_size : in->GetLength();

	// Read font header
	char sig[WFN_FILE_SIG_LENGTH];
	in->Read(sig, WFN_FILE_SIG_LENGTH);
	if (strncmp(sig, WFN_FILE_SIGNATURE, WFN_FILE_SIG_LENGTH) != 0) {
		Debug::Printf(kDbgMsg_Error, "\tWFN: bad format signature");
		return kWFNErr_BadSignature; // bad format
	}

	const soff_t table_addr = static_cast<uint16_t>(in->ReadInt16()); // offset table relative address
	if (table_addr < (soff_t)(WFN_FILE_SIG_LENGTH + sizeof(uint16_t)) || table_addr >= used_data_size) {
		Debug::Printf(kDbgMsg_Error, "\tWFN: bad table address: %llu (%llu - %llu)", static_cast<int64>(table_addr),
			static_cast<int64>(WFN_FILE_SIG_LENGTH + sizeof(uint16_t)), static_cast<int64>(used_data_size));
		return kWFNErr_BadTableAddress; // bad table address
	}

	const soff_t offset_table_size = used_data_size - table_addr;
	const soff_t raw_data_offset = WFN_FILE_SIG_LENGTH + sizeof(uint16_t);
	const size_t total_char_data = static_cast<size_t>(table_addr - raw_data_offset);
	const size_t char_count = static_cast<size_t>(offset_table_size / sizeof(uint16_t));

	// We process character data in three steps:
	// 1. For every character store offset of character item, excluding
	//    duplicates.
	// 2. Allocate memory for character items and pixel array and copy
	//    appropriate data; test for possible format corruption.
	// 3. Create array of references from characters to items; same item may be
	//    referenced by many characters.
	WFNError err = kWFNErr_NoError;

	if (total_char_data == 0u || char_count == 0u)
		return kWFNErr_NoError; // no items

	// Read character data array
	std::vector<uint8_t> raw_data; raw_data.resize(total_char_data);
	in->Read(&raw_data.front(), total_char_data);

	// Read offset table
	std::vector<uint16_t> offset_table;	offset_table.resize(char_count);
	in->ReadArrayOfInt16(reinterpret_cast<int16_t *>(&offset_table.front()), char_count);

	// Read all referenced offsets in an unsorted vector
	std::vector<uint16_t> offs;
	offs.reserve(char_count); // reserve max possible offsets
	for (size_t i = 0; i < char_count; ++i) {
		const uint16_t off = offset_table[i];
		if (off < raw_data_offset || (soff_t)(off + MinCharDataSize) > table_addr) {
			Debug::Printf("\tWFN: character %d -- bad item offset: %d (%d - %d, +%d)",
			              i, off, raw_data_offset, table_addr, MinCharDataSize);
			err = kWFNErr_HasBadCharacters; // warn about potentially corrupt format
			continue; // bad character offset
		}
		offs.push_back(off);
	}
	// sort offsets vector and remove any duplicates
	std::sort(offs.begin(), offs.end());
#if AGS_PLATFORM_SCUMMVM
	// TODO: See if this works correctly
	std::unique(offs.begin(), offs.end());
#else
	std::vector<uint16_t>(offs.begin(), std::unique(offs.begin(), offs.end())).swap(offs);
#endif

	// Now that we know number of valid character items, parse and store character data
	WFNChar init_ch;
	_items.resize(offs.size());
	size_t total_pixel_size = 0;
	for (size_t i = 0; i < _items.size(); ++i) {
		const uint8_t *p_data = &raw_data[offs[i] - raw_data_offset];
		init_ch.Width = Memory::ReadInt16LE(p_data);
		init_ch.Height = Memory::ReadInt16LE(p_data + sizeof(uint16_t));
		total_pixel_size += init_ch.GetRequiredPixelSize();
		_items[i] = init_ch;
	}

	// Now that we know actual size of pixels in use, create pixel data array;
	// since the items are sorted, the pixel data will be stored sequentially as well.
	// At this point offs and _items have related elements in the same order.
	_pixelData.resize(total_pixel_size);
	std::vector<uint8_t>::iterator pixel_it = _pixelData.begin(); // write ptr
	for (size_t i = 0; i < _items.size(); ++i) {
		const size_t pixel_data_size = _items[i].GetRequiredPixelSize();
		if (pixel_data_size == 0) {
			Debug::Printf("\tWFN: item at off %d -- null size", offs[i]);
			err = kWFNErr_HasBadCharacters;
			continue; // just an empty character
		}
		const uint16_t raw_off = offs[i] - raw_data_offset + MinCharDataSize; // offset in raw array
		size_t src_size = pixel_data_size;
		if (i + 1 != _items.size() && (soff_t)(raw_off + src_size) > offs[i + 1] - raw_data_offset) {   // character pixel data overlaps next character
			Debug::Printf("\tWFN: item at off %d -- pixel data overlaps next known item (at %d, +%d)",
			              offs[i], offs[i + 1], MinCharDataSize + src_size);
			err = kWFNErr_HasBadCharacters; // warn about potentially corrupt format
			src_size = offs[i + 1] - offs[i] - MinCharDataSize;
		}

		if (raw_off + src_size > total_char_data) {   // character pixel data overflow buffer
			Debug::Printf("\tWFN: item at off %d -- pixel data exceeds available data (at %d, +%d)",
			              offs[i], table_addr, MinCharDataSize + src_size);
			err = kWFNErr_HasBadCharacters; // warn about potentially corrupt format
			src_size = total_char_data - raw_off;
		}
		_items[i].RestrictToBytes(src_size);

		assert(pixel_it + pixel_data_size <= _pixelData.end()); // should not normally fail
		Common::copy(raw_data.begin() + raw_off, raw_data.begin() + (raw_off + src_size), pixel_it);
		_items[i].Data = &(*pixel_it);
		pixel_it += pixel_data_size;
	}

	// Create final reference array
	_refs.resize(char_count);
	for (size_t i = 0; i < char_count; ++i) {
		const uint16_t off = offset_table[i];
		// if bad character offset - reference empty character
		if (off < raw_data_offset || (soff_t)(off + MinCharDataSize) > table_addr) {
			_refs[i] = &_G(emptyChar);
		} else {
			// in usual case the offset table references items in strict order
			if (i < _items.size() && offs[i] == off)
				_refs[i] = &_items[i];
			else {
				// we know beforehand that such item must exist
				std::vector<uint16_t>::const_iterator at = std::lower_bound(offs.begin(), offs.end(), off);
				assert(at != offs.end() && *at == off && // should not normally fail
				       at - offs.begin() >= 0 && static_cast<size_t>(at - offs.begin()) < _items.size());
				_refs[i] = &_items[at - offs.begin()]; // set up reference to item
			}
		}
	}

	return err;
}

} // namespace AGS3
