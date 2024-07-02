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


/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/qdcore/qd_coords_animation_point.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace QDEngine {

const float qdCoordsAnimationPoint::NO_DIRECTION = -1.f;
qdCoordsAnimationPoint::qdCoordsAnimationPoint() : pos_(0, 0, 0),
	direction_angle_(NO_DIRECTION) {
	path_length_ = passed_path_length_ = 0.0f;
}

qdCoordsAnimationPoint::~qdCoordsAnimationPoint() {
}

void qdCoordsAnimationPoint::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_DEST_POS:
			xml::tag_buffer(*it) > pos_.x > pos_.y > pos_.z;
			break;
		case QDSCR_OBJECT_DIRECTION:
			xml::tag_buffer(*it) > direction_angle_;
			break;
		}
	}
}

bool qdCoordsAnimationPoint::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<coords_animation_point");
	fh.writeString(Common::String::format(" dest_pos=\"%f %f %f\"", pos_.x, pos_.y, pos_.z));

	if (direction_angle_ >= 0.0f) {
		fh.writeString(Common::String::format(" object_direction=\"%f\"", direction_angle_));
	}

	fh.writeString("/>\r\n");

	return true;
}

bool qdCoordsAnimationPoint::save_script(XStream &fh, int indent) const {
	warning("STUB: qdCoordsAnimationPoint::save_script(XStream)");
	return true;
}

void qdCoordsAnimationPoint::calc_path(const qdCoordsAnimationPoint &p, const Vect3f &shift) const {
	Vect3f dr = pos_ - shift - p.dest_pos();
	path_length_ = dr.norm();
}

float qdCoordsAnimationPoint::passed_path() const {
	if (path_length_ < 0.01f) return 1.0f;

	return passed_path_length_ / path_length_;
}

bool qdCoordsAnimationPoint::load_data(qdSaveStream &fh, int save_version) {
	fh > path_length_ > passed_path_length_;

	return true;
}

bool qdCoordsAnimationPoint::save_data(qdSaveStream &fh) const {
	fh < path_length_ < passed_path_length_;

	return true;
}
} // namespace QDEngine
