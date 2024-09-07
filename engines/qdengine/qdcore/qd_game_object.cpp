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

#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_object.h"
#include "qdengine/qdcore/qd_camera.h"


namespace QDEngine {

qdGameObject::qdGameObject() : _r(0, 0, 0),
	_parallax_offset(0.0f, 0.0f),
	_screen_r(0, 0),
	_screen_depth(0.0f), _tempPosInList(0) {
}

qdGameObject::qdGameObject(const qdGameObject &obj) : qdNamedObject(obj),
	_r(obj._r),
	_parallax_offset(obj._parallax_offset),
	_screen_r(obj._screen_r),
	_screen_depth(obj._screen_depth), _tempPosInList(obj._tempPosInList) {
}

qdGameObject::~qdGameObject() {
}

qdGameObject &qdGameObject::operator = (const qdGameObject &obj) {
	if (this == &obj) return *this;

	*static_cast<qdNamedObject *>(this) = obj;

	_r = obj._r;
	_parallax_offset = obj._parallax_offset;

	_screen_r = obj._screen_r;
	_screen_depth = obj._screen_depth;

	return *this;
}

bool qdGameObject::load_script_body(const xml::tag *p) {
	int fl;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_POS2D:
			xml::tag_buffer(*it) > _r.x > _r.y;
			_r.z = 0.0f;
			break;
		case QDSCR_POS3D:
			xml::tag_buffer(*it) > _r.x > _r.y > _r.z;
			break;
		case QDSCR_FLAG:
			xml::tag_buffer(*it) > fl;
			set_flag(fl);
			break;
		case QDSCR_PARALLAX_OFFSET:
			xml::tag_buffer(*it) > _parallax_offset.x > _parallax_offset.y;
			break;
		}
	}

	return true;
}

bool qdGameObject::save_script_body(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<pos_3d>%f %f %f</pos_3d>\r\n", _r.x, _r.y, _r.z));

	if (_parallax_offset.x || _parallax_offset.y) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<parallax_offset>%d %d</parallax_offset>\r\n", _parallax_offset.x, _parallax_offset.y));
	}

	if (flags()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<flag>%d</flag>\r\n", flags()));
	}

	return true;
}

void qdGameObject::quant(float dt) {
}

bool qdGameObject::update_screen_pos() {
	if (!check_flag(QD_OBJ_SCREEN_COORDS_FLAG)) {
		if (const qdCamera * cp = qdCamera::current_camera()) {
			Vect3f v = cp->global2camera_coord(R());

			if (check_flag(QD_OBJ_FIXED_SCREEN_COORDS_FLAG)) {
				_screen_r = cp->camera_coord2rscr(v);
				_screen_r.x += cp->get_scr_sx() / 2;
				_screen_r.y = cp->get_scr_sy() / 2 - _screen_r.y;

				_screen_depth = 0.0f;
			} else {
				_screen_r = cp->camera_coord2scr(v);

				Vect3f rr = R();
				rr.z = cp->get_grid_center().z;
				_screen_depth = cp->global2camera_coord(rr).z;
			}

			if (_parallax_offset.x || _parallax_offset.y) {
				_screen_r.x += round(float(_parallax_offset.x) * cp->scrolling_phase_x());
				_screen_r.y += round(float(_parallax_offset.y) * cp->scrolling_phase_y());
			}
		} else
			return false;
	} else {
		_screen_r = Vect2i(R().xi(), R().yi());
		_screen_depth = 0.0f;
	}

	return true;
}

bool qdGameObject::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	debugC(3, kDebugSave, "  qdGameObject::load_data(): before: %d", (int)fh.pos());

	if (!qdNamedObject::load_data(fh, saveVersion)) {
		return false;
	}

	_r.x = fh.readFloatLE();
	_r.y = fh.readFloatLE();
	_r.z = fh.readFloatLE();

	debugC(3, kDebugSave, "  qdGameObject::load_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdGameObject::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdGameObject::save_data(): before: %d", (int)fh.pos());
	if (!qdNamedObject::save_data(fh)) {
		return false;
	}

	fh.writeFloatLE(_r.x);
	fh.writeFloatLE(_r.y);
	fh.writeFloatLE(_r.z);

	debugC(3, kDebugSave, "  qdGameObject::save_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdGameObject::init() {
	drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);
	drop_flag(QD_OBJ_STATE_CHANGE_FLAG | QD_OBJ_IS_IN_TRIGGER_FLAG | QD_OBJ_IS_IN_INVENTORY_FLAG);
	drop_flag(QD_OBJ_HIDDEN_FLAG);
	return true;
}

#define defFlag(x) { x, #x }

struct FlagsList {
	int f;
	const char *s;
} static flagList[] = {
	defFlag(QD_OBJ_MOVING_FLAG),
	defFlag(QD_OBJ_HIDDEN_FLAG),
	defFlag(QD_OBJ_INVERSE_PERSPECTIVE_FLAG),
	defFlag(QD_OBJ_SHOW_NAME_FLAG),
	defFlag(QD_OBJ_NO_SCALE_FLAG),
	defFlag(QD_OBJ_SCREEN_COORDS_FLAG),
	defFlag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG),
	defFlag(QD_OBJ_HAS_BOUND_FLAG),
	defFlag(QD_OBJ_DISABLE_MOVEMENT_FLAG),
	defFlag(QD_OBJ_DISABLE_MOUSE_FLAG),
	defFlag(QD_OBJ_IS_IN_TRIGGER_FLAG),
	defFlag(QD_OBJ_STATE_CHANGE_FLAG),
	defFlag(QD_OBJ_IS_IN_INVENTORY_FLAG),
	defFlag(QD_OBJ_KEYBOARD_CONTROL_FLAG),
	defFlag(QD_OBJ_FIXED_SCREEN_COORDS_FLAG),
};

Common::String qdGameObject::flag2str(int fl) const {
	Common::String res;

	for (int i = 0; i < ARRAYSIZE(flagList); i++) {
		if (fl & flagList[i].f) {
			if (!res.empty())
				res += " | ";

			res += flagList[i].s;

			fl &= ~flagList[i].f;
		}
	}

	if (fl)
		res += Common::String::format(" | %x", fl);

	return res;
}

} // namespace QDEngine
