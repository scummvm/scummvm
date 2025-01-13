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

#include "got/data/sd_data.h"
#include "got/utils/file.h"

namespace Got {

#define SD_DATA_SIZE 61440

SdData::SdData() {
	_data = new byte[SD_DATA_SIZE];
}

SdData::~SdData() {
	delete[] _data;
}

void SdData::load() {
	Common::String fname = Common::String::format("SDAT%d", _area);
	res_read(fname, _data);
}

void SdData::setArea(int area) {
	if (area != _area) {
		_area = area;
		load();
	}
}

void SdData::sync(Common::Serializer &s) {
	s.syncBytes(_data, SD_DATA_SIZE);
}

} // namespace Got
