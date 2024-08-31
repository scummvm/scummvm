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

#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_coords_animation_point.h"

namespace QDEngine {

const float qdCoordsAnimationPoint::NO_DIRECTION = -1.f;
qdCoordsAnimationPoint::qdCoordsAnimationPoint() : _pos(0, 0, 0),
	_direction_angle(NO_DIRECTION) {
	_path_length = _passed_path_length = 0.0f;
}

qdCoordsAnimationPoint::~qdCoordsAnimationPoint() {
}

void qdCoordsAnimationPoint::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_DEST_POS:
			xml::tag_buffer(*it) > _pos.x > _pos.y > _pos.z;
			break;
		case QDSCR_OBJECT_DIRECTION:
			xml::tag_buffer(*it) > _direction_angle;
			break;
		}
	}
}

bool qdCoordsAnimationPoint::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<coords_animation_point");
	fh.writeString(Common::String::format(" dest_pos=\"%f %f %f\"", _pos.x, _pos.y, _pos.z));

	if (_direction_angle >= 0.0f) {
		fh.writeString(Common::String::format(" object_direction=\"%f\"", _direction_angle));
	}

	fh.writeString("/>\r\n");

	return true;
}

void qdCoordsAnimationPoint::calc_path(const qdCoordsAnimationPoint &p, const Vect3f &shift) const {
	Vect3f dr = _pos - shift - p.dest_pos();
	_path_length = dr.norm();
}

float qdCoordsAnimationPoint::passed_path() const {
	if (_path_length < 0.01f) return 1.0f;

	return _passed_path_length / _path_length;
}

bool qdCoordsAnimationPoint::load_data(Common::SeekableReadStream &fh, int save_version) {
	_path_length = fh.readFloatLE();
	_passed_path_length = fh.readFloatLE();

	return true;
}

bool qdCoordsAnimationPoint::save_data(Common::WriteStream &fh) const {
	fh.writeFloatLE(_path_length);
	fh.writeFloatLE(_passed_path_length);

	return true;
}
} // namespace QDEngine
