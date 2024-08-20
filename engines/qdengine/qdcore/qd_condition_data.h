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

#ifndef QDENGINE_QDCORE_QD_CONDITION_DATA_H
#define QDENGINE_QDCORE_QD_CONDITION_DATA_H

#include "common/std/vector.h"

#include "qdengine/parser/xml_fwd.h"

namespace QDEngine {

class qdConditionData {
public:
	enum data_t {
		DATA_INT,
		DATA_FLOAT,
		DATA_STRING
	};

	qdConditionData();
	qdConditionData(data_t data_type, int data_size = 0);
	qdConditionData(const qdConditionData &data);
	~qdConditionData();

	qdConditionData &operator = (const qdConditionData &data);

	data_t type() const {
		return _type;
	}
	void set_type(data_t tp) {
		_type = tp;
	}

	int get_int(int index = 0) const {
		return reinterpret_cast<const int32 *>(&*_data.begin())[index];
	}
	bool put_int(int value, int index = 0) {
		if (static_cast<int>(_data.size()) >= static_cast<int>((index - 1) * sizeof(int32))) {
			reinterpret_cast<int32 *>(&*_data.begin())[index] = value;
			return true;
		}

		return false;
	}

	float get_float(int index = 0) const {
		return reinterpret_cast<const float *>(&*_data.begin())[index];
	}
	bool put_float(float value, int index = 0) {
		if (static_cast<int>(_data.size()) >=
		        static_cast<int>((index - 1) * sizeof(float))) {
			reinterpret_cast<float *>(&*_data.begin())[index] = value;
			return true;
		}

		return false;
	}

	const char *get_string() const {
		if (!_data.empty())
			return &*_data.begin();
		else
			return NULL;
	}

	bool put_string(const char *str) {
		if (alloc_data(strlen(str) + 1)) {
			Common::strlcpy(&*_data.begin(), str, _data.size());
			return true;
		}

		return false;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool alloc_data(int size);

private:

	data_t _type;
	Std::vector<char> _data;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CONDITION_DATA_H
