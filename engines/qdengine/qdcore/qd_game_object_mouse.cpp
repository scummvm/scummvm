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

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_rnd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace QDEngine {

qdGameObjectMouse::qdGameObjectMouse() : _object(NULL),
	_object_screen_region(grScreenRegion_EMPTY) {
	set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
	set_name("\xcc\xfb\xf8\xfc"); // "Мышь" -- mouse

	_screen_pos_offset = _screen_pos_offset_delta = Vect2f(0, 0);

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		_default_cursors[i] = 0;
}

qdGameObjectMouse::qdGameObjectMouse(const qdGameObjectMouse &obj) : qdGameObjectAnimated(obj),
	_object(NULL),
	_object_screen_region(grScreenRegion_EMPTY) {
	set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
	set_name("\xcc\xfb\xf8\xfc"); // "Мышь" -- mouse

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		_default_cursors[i] = 0;
}

qdGameObjectMouse::~qdGameObjectMouse() {
}

qdGameObjectMouse &qdGameObjectMouse::operator = (const qdGameObjectMouse &obj) {
	if (this == &obj) return *this;

	*static_cast<qdGameObjectAnimated *>(this) = obj;

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		_default_cursors[i] = obj._default_cursors[i];

	return *this;
}

bool qdGameObjectMouse::load_script_body(const xml::tag *p) {
	qdGameObjectAnimated::load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_MOUSE_DEFAULT_CURSORS: {
			xml::tag_buffer buf(*it);
			for (int i = 0; i < MAX_CURSOR_ID; i++) buf > _default_cursors[i];
		}
		break;
		}
	}

	for (int i = 0; i < max_state(); i++) {
		if (get_state(i)->state_type() == qdGameObjectState::STATE_STATIC) {
			static_cast<qdGameObjectStateStatic *>(get_state(i))->animation_info()->set_flag(QD_ANIMATION_FLAG_LOOP);
		}
	}

	return true;
}

bool qdGameObjectMouse::save_script_body(Common::WriteStream &fh, int indent) const {
	return qdGameObjectAnimated::save_script_body(fh, indent);
}

bool qdGameObjectMouse::load_script(const xml::tag *p) {
	return load_script_body(p);
}

bool qdGameObjectMouse::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<mouse_object name=\"%s\"", qdscr_XML_string(name())));

	fh.writeString(" default_cursors=\"");
	for (int i = 0; i < MAX_CURSOR_ID; i++) {
		if (i) fh.writeString(" ");
		fh.writeString(Common::String::format("%d", _default_cursors[i]));
	}

	fh.writeString("\">\r\n");

	save_script_body(fh, indent);

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</mouse_object>\r\n");

	return true;

}

bool qdGameObjectMouse::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdGameObjectMouse::load_data before: %ld", fh.pos());
	if (!qdGameObjectAnimated::load_data(fh, save_version))
		return false;

	int fl = fh.readSint32LE();

	if (fl) {
		qdNamedObjectReference ref;
		if (!ref.load_data(fh, save_version))
			return false;

		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
			_object = static_cast<qdGameObjectAnimated * >(p->get_named_object(&ref));

		if (!_object) return false;
	}

	debugC(3, kDebugSave, "  qdGameObjectMouse::load_data after: %ld", fh.pos());
	return true;
}

bool qdGameObjectMouse::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdGameObjectMouse::save_data before: %ld", fh.pos());

	if (!qdGameObjectAnimated::save_data(fh))
		return false;

	if (_object) {
		fh.writeUint32LE(1);
		qdNamedObjectReference ref(_object);
		if (!ref.save_data(fh)) {
			return false;
		}
	} else {
		fh.writeUint32LE(0);
	}

	debugC(3, kDebugSave, "  qdGameObjectMouse::save_data after: %ld", fh.pos());
	return true;
}

void qdGameObjectMouse::redraw(int offs_x, int offs_y) const {
	if (_object && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
		update_object_position();
		const qdGameObjectState *p = _object-> get_cur_state();

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY) || !p->has_mouse_cursor_ID())
			_object->redraw(offs_x, offs_y);

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY) || p->has_mouse_cursor_ID())
			qdGameObjectAnimated::redraw(offs_x, offs_y);
	} else {
		qdGameObjectAnimated::redraw(offs_x, offs_y);
	}
}

void qdGameObjectMouse::set_cursor(cursor_ID_t id) {
	debugC(2, kDebugGraphics, "qdGameObjectMouse::set_cursor(%d)", id);
	if (cur_state() != _default_cursors[id])
		set_state(_default_cursors[id]);
}

void qdGameObjectMouse::take_object(qdGameObjectAnimated *p) {
	if (_object) {
		if (_object->get_cur_state() && _object->get_cur_state()->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE)) {
			if (qdGameObjectState * sp = _object->get_inventory_state())
				_object->set_state(sp);
		}

//		object_->drop_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
	}

	_object = p;

	if (_object) {
		_object->set_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
		if (qdGameObjectState * sp = _object->get_mouse_state()) {
			if (_object->get_cur_state() != sp) {
				qdGameObjectState *cur_st = _object->get_cur_state();
				if (cur_st && cur_st->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE))
					cur_st = cur_st->prev_state();

				sp->set_prev_state(cur_st);
				_object->set_state(sp);
			}
		}
	}
}

void qdGameObjectMouse::update_object_position() const {
	if (_object) {
		if (_object-> get_cur_state()->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY)) {
			if (const qdGameDispatcher * dp = static_cast<const qdGameDispatcher * >(owner())) {
				Vect2s pos = dp->cur_inventory()->cell_position(_object->inventory_cell_index());
				_object->set_pos(Vect3f(pos.x, pos.y, 0));
			}
		} else
			_object->set_pos(R());

		_object->update_screen_pos();
	}
}

void qdGameObjectMouse::pre_redraw() {
	qdGameDispatcher *dp = static_cast<qdGameDispatcher *>(owner());
	if (!dp) return;

	update_object_position();

	if (!dp->need_full_redraw()) {
		if (_object && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
			if (_object->need_redraw()) {
				dp->add_redraw_region(_object->last_screen_region());
				dp->add_redraw_region(_object->screen_region());
			}
		} else
			dp->add_redraw_region(_object_screen_region);

		dp->add_redraw_region(last_screen_region());
		dp->add_redraw_region(screen_region());
	}
}

void qdGameObjectMouse::post_redraw() {
	if (_object && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
		_object->post_redraw();
		_object_screen_region = _object->last_screen_region();
	} else
		_object_screen_region = grScreenRegion_EMPTY;

	qdGameObjectAnimated::post_redraw();
}

void qdGameObjectMouse::quant(float dt) {
	qdGameObjectAnimated::quant(dt);

	if (_object)
		_object->quant(dt);

	if (const qdGameObjectState * p = get_cur_state()) {
		if (p->rnd_move_radius() > FLT_EPS) {
			if (_screen_pos_offset.norm2() >= sqr(p->rnd_move_radius()) || (_screen_pos_offset_delta.x <= FLT_EPS && _screen_pos_offset_delta.y <= FLT_EPS)) {
				float angle = qd_fabs_rnd(M_PI * 2.0f);

				Vect2f r(p->rnd_move_radius() * cos(angle), p->rnd_move_radius() * sin(angle));
				_screen_pos_offset_delta = r - _screen_pos_offset;
				_screen_pos_offset_delta.normalize(p->rnd_move_speed());
			}

			_screen_pos_offset.x += _screen_pos_offset_delta.x * dt;
			_screen_pos_offset.y += _screen_pos_offset_delta.y * dt;
		} else
			_screen_pos_offset = _screen_pos_offset_delta = Vect2f(0, 0);
	}
}

bool qdGameObjectMouse::update_screen_pos() {
	if (qdGameObjectAnimated::update_screen_pos()) {
		set_screen_R(get_screen_R() + _screen_pos_offset);
		return true;
	}

	return false;
}
} // namespace QDEngine
