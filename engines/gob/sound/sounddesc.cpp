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

#include "common/util.h"

#include "gob/sound/sounddesc.h"

namespace Gob {

SoundDesc::SoundDesc() {
	_data = _dataPtr = 0;
	_size = 0;

	_type = SOUND_SND;
	_source = SOUND_FILE;

	_repCount = 0;
	_frequency = 0;
	_flag = 0;
	_id = 0;
}

SoundDesc::~SoundDesc() {
	free();
}

void SoundDesc::set(SoundType type, SoundSource src,
		byte *data, uint32 dSize) {

	free();

	_type = type;
	_source = src;
	_data = _dataPtr = data;
	_size = dSize;
}

void SoundDesc::load(SoundType type, SoundSource src,
		byte *data, uint32 dSize) {

	free();

	_source = src;
	switch (type) {
	case SOUND_ADL:
		loadADL(data, dSize);
		break;
	case SOUND_SND:
		loadSND(data, dSize);
		break;
	}
}

void SoundDesc::free() {
	if (_source != SOUND_TOT)
		delete[] _data;
	_data = _dataPtr = 0;
	_id = 0;
}

void SoundDesc::convToSigned() {
	if ((_type == SOUND_SND) && _data && _dataPtr)
		for (uint32 i = 0; i < _size; i++)
			_dataPtr[i] ^= 0x80;
}

int16 SoundDesc::calcFadeOutLength(int16 frequency) {
	return (10 * (_size / 2)) / frequency;
}

uint32 SoundDesc::calcLength(int16 repCount, int16 frequency, bool fade) {
	uint32 fadeSize = fade ? _size / 2 : 0;
	return ((_size * repCount - fadeSize) * 1000) / frequency;
}

void SoundDesc::loadSND(byte *data, uint32 dSize) {
	assert(dSize > 6);

	_type = SOUND_SND;
	_data = data;
	_dataPtr = data + 6;
	_frequency = MAX((int16) READ_BE_UINT16(data + 4), (int16) 4700);
	_flag = data[0] ? (data[0] & 0x7F) : 8;
	data[0] = 0;
	_size = MIN(READ_BE_UINT32(data), dSize - 6);
}

void SoundDesc::loadADL(byte *data, uint32 dSize) {
	_type = SOUND_ADL;
	_data = _dataPtr = data;
	_size = dSize;
}

} // End of namespace Gob
