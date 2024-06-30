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

#ifndef QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_FRAME_H
#define QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_FRAME_H

#include <vector>
#include "qdengine/core/parser/xml_fwd.h"


namespace QDEngine {

class XStream;

class qdCoordsAnimationPoint {
public:
	qdCoordsAnimationPoint();
	~qdCoordsAnimationPoint();

	static const float NO_DIRECTION;

	const Vect3f &dest_pos() const {
		return pos_;
	}
	void set_dest_pos(const Vect3f &r) {
		pos_ = r;
	}

	float direction_angle() const {
		return direction_angle_;
	}
	void set_direction_angle(float ang) {
		direction_angle_ = ang;
	}

	void start() const {
		passed_path_length_ = 0.0f;
	}

	bool move(float &path) const {
		passed_path_length_ += path;
		if (passed_path_length_ >= path_length_) {
			path = passed_path_length_ - path_length_;
			return true;
		}
		return false;
	}

	void calc_path(const qdCoordsAnimationPoint &p, const Vect3f &shift = Vect3f::ZERO) const;
	float passed_path() const;
	float path_length() const {
		return path_length_;
	}

	void load_script(const xml::tag *p);
	bool save_script(XStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;

private:

	Vect3f pos_;
	float direction_angle_;

	mutable float path_length_;
	mutable float passed_path_length_;
};

typedef std::vector<qdCoordsAnimationPoint> qdCoordsAnimationPointVector;

} // namespace QDEngine

#endif /* QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_FRAME_H */
