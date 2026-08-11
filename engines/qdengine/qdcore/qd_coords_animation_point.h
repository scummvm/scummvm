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

#ifndef QDENGINE_QDCORE_QD_COORDS_ANIMATION_POINT_H
#define QDENGINE_QDCORE_QD_COORDS_ANIMATION_POINT_H

#include "qdengine/parser/xml_fwd.h"


namespace QDEngine {

class qdCoordsAnimationPoint {
public:
	qdCoordsAnimationPoint();
	~qdCoordsAnimationPoint();

	static const float NO_DIRECTION;

	const Vect3f &dest_pos() const {
		return _pos;
	}
	void set_dest_pos(const Vect3f &r) {
		_pos = r;
	}

	float direction_angle() const {
		return _direction_angle;
	}
	void set_direction_angle(float ang) {
		_direction_angle = ang;
	}

	void start() const {
		_passed_path_length = 0.0f;
	}

	bool move(float &path) const {
		_passed_path_length += path;
		if (_passed_path_length >= _path_length) {
			path = _passed_path_length - _path_length;
			return true;
		}
		return false;
	}

	void calc_path(const qdCoordsAnimationPoint &p, const Vect3f &shift = Vect3f(0, 0, 0)) const;
	float passed_path() const;
	float path_length() const {
		return _path_length;
	}

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

private:

	Vect3f _pos;
	float _direction_angle;

	mutable float _path_length;
	mutable float _passed_path_length;
};

typedef Std::vector<qdCoordsAnimationPoint> qdCoordsAnimationPointVector;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_COORDS_ANIMATION_POINT_H
