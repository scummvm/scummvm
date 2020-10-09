/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 *
 */

#include "common/endian.h"
#include "common/stream.h"

#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/resource.h"
#include "engines/grim/debug.h"

namespace Grim {

template class ObjectPtr<LipSync>;


LipSync::LipSync(const Common::String &filename, Common::SeekableReadStream *data) :
		Object(), _fname(filename) {

	if (data->readUint32BE() != MKTAG('L','I','P','!')) {
		error("Invalid file format in %s", _fname.c_str());
	}

	_numEntries = (data->size() - 8) / 4;

	// There are cases where the lipsync file has no entries
	if (_numEntries == 0) {
		_entries = nullptr;
		return;
	}

	data->readUint32LE();
	Debug::debug(Debug::Lipsync, "Reading LipSync %s, %d entries\n", filename.c_str(), _numEntries);
	_entries = new LipEntry[_numEntries];
	for (int i = 0; i < _numEntries; i++) {
		_entries[i].frame = data->readUint16LE();
		uint16 readPhoneme = data->readUint16LE();
		int j;

		// Look for the animation corresponding to the phoneme
		for (j = 0; j < _animTableSize; j++) {
			if (readPhoneme == _animTable[j].phoneme) {
				_entries[i].anim = _animTable[j].anim;
				break;
			}
		}

		if (j >= _animTableSize) {
			Debug::warning(Debug::Lipsync, "Unknown phoneme: 0x%X in file %s", readPhoneme, _fname.c_str());
			_entries[i].anim = 1;
		}
	}
}

LipSync::~LipSync() {
	delete[] _entries;
	g_resourceloader->uncacheLipSync(this);
}

int LipSync::getAnim(int pos) {
	int frame1, frame2;

	for (int i = 0; i < _numEntries; i++) {
		frame1 = _entries[i].frame;
		if ((i + 1) < _numEntries) {
			frame2 = _entries[i + 1].frame;
		} else {
			frame2 = (unsigned int) - 1L;
		}
		if ((pos >= frame1) && (pos < frame2)) {
			return _entries[i].anim;
		}
	}

	return -1;
}

// TODO: Figure out which one is 0x0000, use 0 for now.
const LipSync::PhonemeAnim LipSync::_animTable[] = {
	{0x005F, 0}, {0x0251, 1}, {0x0061, 1}, {0x00E6, 1}, {0x028C, 8},
	{0x0254, 1}, {0x0259, 1}, {0x0062, 6}, {0x02A7, 2}, {0x0064, 2},
	{0x00F0, 5}, {0x025B, 8}, {0x0268, 8}, {0x025A, 9}, {0x025D, 9},
	{0x0065, 1}, {0x0066, 4}, {0x0067, 8}, {0x0261, 8}, {0x0068, 8},
	{0x026A, 8}, {0x0069, 3}, {0x02A4, 2}, {0x006B, 2}, {0x006C, 5},
	{0x026B, 5}, {0x006D, 6}, {0x006E, 8}, {0x014B, 8}, {0x006F, 7},
	{0x0070, 6}, {0x0072, 2}, {0x027B, 2}, {0x0279, 2}, {0x0073, 2},
	{0x0283, 2}, {0x0074, 2}, {0x027E, 2}, {0x03B8, 5}, {0x028A, 9},
	{0x0075, 9}, {0x0076, 4}, {0x0077, 9}, {0x006A, 8}, {0x007A, 2},
	{0x0292, 2}, {0x002E, 2}, {0x0000, 0}
};

const int LipSync::_animTableSize = sizeof(LipSync::_animTable) / sizeof(LipSync::_animTable[0]);

} // end of namespace Grim
