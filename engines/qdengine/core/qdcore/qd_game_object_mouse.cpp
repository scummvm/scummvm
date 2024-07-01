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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/debug.h"
#include "common/stream.h"
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_rnd.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/system/graphics/gr_dispatcher.h"
#include "qdengine/core/qdcore/qd_game_object_mouse.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGameObjectMouse::qdGameObjectMouse() : object_(NULL),
	object_screen_region_(grScreenRegion::EMPTY) {
	set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
	set_name("Мышь");

	screen_pos_offset_ = screen_pos_offset_delta_ = Vect2f(0, 0);

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		default_cursors_[i] = 0;
}

qdGameObjectMouse::qdGameObjectMouse(const qdGameObjectMouse &obj) : qdGameObjectAnimated(obj),
	object_(NULL),
	object_screen_region_(grScreenRegion::EMPTY) {
	set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
	set_name("Мышь");

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		default_cursors_[i] = 0;
}

qdGameObjectMouse::~qdGameObjectMouse() {
}

qdGameObjectMouse &qdGameObjectMouse::operator = (const qdGameObjectMouse &obj) {
	if (this == &obj) return *this;

	*static_cast<qdGameObjectAnimated *>(this) = obj;

	for (int i = 0; i < MAX_CURSOR_ID; i++)
		default_cursors_[i] = obj.default_cursors_[i];

	return *this;
}

bool qdGameObjectMouse::load_script_body(const xml::tag *p) {
	qdGameObjectAnimated::load_script_body(p);

	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_MOUSE_DEFAULT_CURSORS: {
			xml::tag_buffer buf(*it);
			for (int i = 0; i < MAX_CURSOR_ID; i ++) buf > default_cursors_[i];
		}
		break;
		}
	}

	for (int i = 0; i < max_state(); i++) {
		if (get_state(i) -> state_type() == qdGameObjectState::STATE_STATIC) {
			static_cast<qdGameObjectStateStatic *>(get_state(i)) -> animation_info() -> set_flag(QD_ANIMATION_FLAG_LOOP);
		}
	}

	return true;
}

bool qdGameObjectMouse::save_script_body(Common::SeekableWriteStream &fh, int indent) const {
	return qdGameObjectAnimated::save_script_body(fh, indent);
}

bool qdGameObjectMouse::save_script_body(XStream &fh, int indent) const {
	return qdGameObjectAnimated::save_script_body(fh, indent);
}

bool qdGameObjectMouse::load_script(const xml::tag *p) {
	return load_script_body(p);
}

bool qdGameObjectMouse::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(" default_cursors=\"");
	for (int i = 0; i < MAX_CURSOR_ID; i ++) {
		if (i) fh.writeString(" ");
		fh.writeString(Common::String::format("%d", default_cursors_[i]));
	}

	fh.writeString("\"");
	fh.writeString(">\r\n");

	save_script_body(fh, indent);

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</mouse_object>\r\n");

	return true;

}

bool qdGameObjectMouse::save_script(XStream &fh, int indent) const {
	warning("STUB: qdGameObjectMouse::save_script(XStream)");
	return true;
}

bool qdGameObjectMouse::load_data(qdSaveStream &fh, int save_version) {
	if (!qdGameObjectAnimated::load_data(fh, save_version))
		return false;

	int fl;
	fh > fl;

	if (fl) {
		qdNamedObjectReference ref;
		if (!ref.load_data(fh, save_version))
			return false;

		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
			object_ = static_cast<qdGameObjectAnimated * >(p -> get_named_object(&ref));

		if (!object_) return false;
	}

	return true;
}

bool qdGameObjectMouse::save_data(Common::SeekableWriteStream &fh) {

	if (object_) {
		fh.writeUint32LE(1);
		qdNamedObjectReference ref(object_);
		if (!ref.save_data(fh)) {
			return false;
		}
	} else {
		fh.writeUint32LE(0);
	}

	return true;

}

bool qdGameObjectMouse::save_data(qdSaveStream &fh) const {
	if (!qdGameObjectAnimated::save_data(fh))
		return false;

	if (object_) {
		fh < (int)1;
		qdNamedObjectReference ref(object_);
		if (!ref.save_data(fh))
			return false;
	} else
		fh < (int)0;

	return true;
}

void qdGameObjectMouse::redraw(int offs_x, int offs_y) const {
	debugC(1, kDebugTemp, "mouse redraw %p %d", object_, !qdInterfaceDispatcher::get_dispatcher()->is_active());
	if (object_ && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
		update_object_position();
		const qdGameObjectState *p = object_-> get_cur_state();

		if (p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY) || !p -> has_mouse_cursor_ID())
			object_ -> redraw(offs_x, offs_y);

		if (p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY) || p -> has_mouse_cursor_ID())
			qdGameObjectAnimated::redraw(offs_x, offs_y);
	} else {
		debugC(1, kDebugTemp, "mouse redraw 2");
		qdGameObjectAnimated::redraw(offs_x, offs_y);
		debugC(1, kDebugTemp, "mouse redraw 2 over");
	}
}

void qdGameObjectMouse::set_cursor(cursor_ID_t id) {
	debugC(2, kDebugGraphics, "qdGameObjectMouse::set_cursor(%d)", id);
	if (cur_state() != default_cursors_[id])
		set_state(default_cursors_[id]);
}

void qdGameObjectMouse::take_object(qdGameObjectAnimated *p) {
	if (object_) {
		if (object_ -> get_cur_state() && object_ -> get_cur_state() -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE)) {
			if (qdGameObjectState * sp = object_->get_inventory_state())
				object_ -> set_state(sp);
		}

//		object_ -> drop_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
	}

	object_ = p;

	if (object_) {
		object_ -> set_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
		if (qdGameObjectState * sp = object_ -> get_mouse_state()) {
			if (object_ -> get_cur_state() != sp) {
				qdGameObjectState *cur_st = object_ -> get_cur_state();
				if (cur_st && cur_st -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE))
					cur_st = cur_st -> prev_state();

				sp -> set_prev_state(cur_st);
				object_ -> set_state(sp);
			}
		}
	}
}

void qdGameObjectMouse::update_object_position() const {
	if (object_) {
		if (object_-> get_cur_state() -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY)) {
			if (const qdGameDispatcher * dp = static_cast<const qdGameDispatcher * >(owner())) {
				Vect2s pos = dp -> cur_inventory() -> cell_position(object_ -> inventory_cell_index());
				object_ -> set_pos(Vect3f(pos.x, pos.y, 0));
			}
		} else
			object_ -> set_pos(R());

		object_ -> update_screen_pos();
	}
}

void qdGameObjectMouse::pre_redraw() {
	qdGameDispatcher *dp = static_cast<qdGameDispatcher *>(owner());
	if (!dp) return;

	update_object_position();

	if (!dp -> need_full_redraw()) {
		if (object_ && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
			if (object_ -> need_redraw()) {
				dp -> add_redraw_region(object_ -> last_screen_region());
				dp -> add_redraw_region(object_ -> screen_region());
			}
		} else
			dp -> add_redraw_region(object_screen_region_);

		dp -> add_redraw_region(last_screen_region());
		dp -> add_redraw_region(screen_region());
	}
}

void qdGameObjectMouse::post_redraw() {
	if (object_ && !qdInterfaceDispatcher::get_dispatcher()->is_active()) {
		object_ -> post_redraw();
		object_screen_region_ = object_ -> last_screen_region();
	} else
		object_screen_region_ = grScreenRegion::EMPTY;

	qdGameObjectAnimated::post_redraw();
}

void qdGameObjectMouse::quant(float dt) {
	qdGameObjectAnimated::quant(dt);

	if (object_)
		object_ -> quant(dt);

#ifndef _QUEST_EDITOR
	if (const qdGameObjectState * p = get_cur_state()) {
		if (p -> rnd_move_radius() > FLT_EPS) {
			if (screen_pos_offset_.norm2() >= sqr(p -> rnd_move_radius()) || (screen_pos_offset_delta_.x <= FLT_EPS && screen_pos_offset_delta_.y <= FLT_EPS)) {
				float angle = qd_fabs_rnd(M_PI * 2.0f);

				Vect2f r(p -> rnd_move_radius() * cos(angle), p -> rnd_move_radius() * sin(angle));
				screen_pos_offset_delta_ = r - screen_pos_offset_;
				screen_pos_offset_delta_.normalize(p -> rnd_move_speed());
			}

			screen_pos_offset_.x += screen_pos_offset_delta_.x * dt;
			screen_pos_offset_.y += screen_pos_offset_delta_.y * dt;
		} else
			screen_pos_offset_ = screen_pos_offset_delta_ = Vect2f(0, 0);
	}
#endif
}

bool qdGameObjectMouse::update_screen_pos() {
	if (qdGameObjectAnimated::update_screen_pos()) {
#ifndef _QUEST_EDITOR
		set_screen_R(get_screen_R() + screen_pos_offset_);
#endif
		return true;
	}

	return false;
}
} // namespace QDEngine
