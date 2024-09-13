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

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_grid_zone.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

namespace QDEngine {

qdGameObjectAnimated::qdGameObjectAnimated() : _cur_state(-1),
	_inventory_type(0),
	_last_state(NULL),
	_default_r(0, 0, 0),
	_grid_r(0, 0, 0),
	_grid_size(0, 0),
	_queued_state(NULL),
	_last_frame(NULL),
	_inventory_cell_index(-1),
	_last_inventory_state(nullptr),
	_shadow_alpha(QD_NO_SHADOW_ALPHA),
	_shadow_color(0) {
	set_animation(NULL);

	set_bound(Vect3f(10, 10, 10));

	_last_screen_depth = 0;

	_lastShadowColor = 0;
	_lastShadowAlpha = QD_NO_SHADOW_ALPHA;

	if (NULL != qdGameDispatcher::get_dispatcher())
		_last_chg_time = qdGameDispatcher::get_dispatcher()->get_time();
	else
		_last_chg_time = 0;
}

qdGameObjectAnimated::qdGameObjectAnimated(const qdGameObjectAnimated &obj) : qdGameObject(obj),
	_cur_state(-1),
	_queued_state(NULL),
	_inventory_type(obj._inventory_type),
	_bound(obj._bound),
	_radius(obj._radius),
	_default_r(obj._default_r),
	_grid_r(0, 0, 0),
	_grid_size(0, 0),
	_inventory_name(obj._inventory_name),
	_last_state(NULL),
	_inventory_cell_index(-1),
	_last_frame(NULL),
	_last_chg_time(obj._last_chg_time),
	_shadow_alpha(obj._shadow_alpha),
	_shadow_color(obj._shadow_color) {
	_last_screen_depth = 0;

	_lastShadowColor = 0;
	_lastShadowAlpha = QD_NO_SHADOW_ALPHA;

	for (auto &it : obj._states) {
		if (!it->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
			add_state(it->clone());
		else
			add_state(it);
	}
}

qdGameObjectAnimated::~qdGameObjectAnimated() {
	clear_states();
}

qdGameObjectAnimated &qdGameObjectAnimated::operator = (const qdGameObjectAnimated &obj) {
	if (this == &obj) return *this;

	*static_cast<qdGameObject *>(this) = obj;

	_cur_state = -1;
	_queued_state = NULL;

	_inventory_type = obj._inventory_type;

	_animation.clear();

	set_bound(obj._bound);

	_default_r = obj._default_r;

	_grid_r = obj._grid_r;
	_grid_size = obj._grid_size;

	_inventory_name = obj._inventory_name;

	_last_state = NULL;

	_inventory_cell_index = -1;

	_last_frame = NULL;
	_last_screen_region = grScreenRegion_EMPTY;
	_last_screen_depth = 0;

	_lastShadowColor = 0;
	_lastShadowAlpha = QD_NO_SHADOW_ALPHA;

	_shadow_alpha = obj._shadow_alpha;
	_shadow_color = obj._shadow_color;

	_last_chg_time = obj.last_chg_time();

	clear_states();

	for (auto &it : obj._states) {
		if (!(it->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))) {
			add_state(it->clone());
		} else {
			add_state(it);
		}
	}

	return *this;
}

void qdGameObjectAnimated::clear_states() {
	for (auto &it : _states) {
		it->dec_reference_count();

		if (!it->reference_count()) {
			delete it;
			it = nullptr;
		}
	}
}

void qdGameObjectAnimated::set_animation(qdAnimation *p, const qdAnimationInfo *inf) {
	if (p) {
		// FIXME: HACK: The original has these lines commented
		if (!p->is_resource_loaded())
			p->load_resources();

		p->create_reference(&_animation, inf);
		_animation.start();
	} else
		_animation.clear();
}

void qdGameObjectAnimated::set_animation_info(qdAnimationInfo *inf) {
	if (inf) {
		if (qdAnimation * p = inf->animation())
			set_animation(p, inf);
		else
			_animation.clear();
	} else
		_animation.clear();
}

void qdGameObjectAnimated::set_screen_rotation(float target_angle, float speed) {
	_target_transform.set_angle(target_angle);
	_transform_speed.set_angle(speed);
}

float qdGameObjectAnimated::screen_rotation() const {
	return _current_transform.angle();
}

void qdGameObjectAnimated::set_screen_scale(const Vect2f &scale, const Vect2f &speed) {
	_target_transform.set_scale(scale);
	_transform_speed.set_scale(speed);
}

const Vect2f &qdGameObjectAnimated::screen_scale() const {
	return _current_transform.scale();
}

void qdGameObjectAnimated::redraw(int offs_x, int offs_y) const {

	if (!_animation.is_empty()) {
		debugC(2, kDebugGraphics, "qdGameObjectAnimated::redraw([%d, %d]), name: '%s'", offs_x, offs_y, transCyrillic(name()));
		Vect2i r = screen_pos() + Vect2i(offs_x, offs_y);

		if (_current_transform()) {
			/*
			            Vect2i delta = (_cur_state != -1) ? _states[_cur_state]->center_offset() : Vect2i(0,0);
			            if(delta.x || delta.y){
			                r -= delta;

			                Vect2f scale(_current_transform.scale());

			                delta.x = round(float(delta.x) * scale.x);
			                delta.y = round(float(delta.y) * scale.y);

			                float angle = _current_transform.angle();

			                r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
			                r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			            }
			*/
			if (_current_transform.has_scale())
				_animation.redraw_rot(r.x, r.y, screen_depth(), _current_transform.angle(), _current_transform.scale());
			else
				_animation.redraw_rot(r.x, r.y, screen_depth(), _current_transform.angle());
		} else
			_animation.redraw(r.x, r.y, screen_depth());
	}

	if (shadow_alpha() != QD_NO_SHADOW_ALPHA)
		draw_shadow(offs_x, offs_y, shadow_color(), shadow_alpha());

	if (g_engine->_debugDraw)
		debug_redraw();
}

bool qdGameObjectAnimated::need_redraw() const {
	uint32 color = (_cur_state == -1) ? 0 : _states[_cur_state]->shadow_color();
	int alpha = (_cur_state == -1) ? QD_NO_SHADOW_ALPHA : _states[_cur_state]->shadow_alpha();

	return (_animation.get_cur_frame() != _last_frame) ||
	       (_last_screen_depth != screen_depth()) ||
	       (screen_region() != _last_screen_region) ||
	       (_last_transform != _current_transform) ||
	       _lastShadowColor != color || _lastShadowAlpha != alpha;
}

void qdGameObjectAnimated::quant(float dt) {
	Vect3f beg_r = R();

	qdGameObject::quant(dt);

	get_animation()->quant(dt);

	if (_target_transform != _current_transform)
		_current_transform.change(dt, _target_transform, _transform_speed);

	for (qdGameObjectStateVector::iterator it = _states.begin(); it != _states.end(); ++it)
		(*it)->quant(dt);

	debugC(1, kDebugQuant, "qdGameObjectAnimated::quant(): obj: '%s', state: %d  flags: %s", transCyrillic(name()), _cur_state, flag2str(flags()).c_str());

	if (_cur_state != -1) {
		if (!_states[_cur_state]->coords_animation()->is_empty())
			_states[_cur_state]->coords_animation()->quant(dt);

		if (!_states[_cur_state]->is_in_triggers()) {
			for (qdGameObjectStateVector::iterator it = _states.begin(); it != _states.end(); ++it) {
				if (_states[_cur_state] != *it)
					(*it)->set_prev_state(_states[_cur_state]);
			}
		}

		if (!check_flag(QD_OBJ_HIDDEN_FLAG) && !check_flag(QD_OBJ_IS_IN_TRIGGER_FLAG)) {
			if (!check_flag(QD_OBJ_STATE_CHANGE_FLAG)) {
				StateStatus status = state_status(_states[_cur_state]);
				for (qdGameObjectStateVector::iterator it = _states.begin(); it != _states.end(); ++it) {
					if (!(*it)->is_in_triggers()) {
						if (_states[_cur_state] != *it || status == STATE_DONE) {
							if ((*it)->check_conditions()) {
								set_state(*it);
								set_flag(QD_OBJ_STATE_CHANGE_FLAG);
								status = state_status(_states[_cur_state]);
								break;
							}
						}
					}
				}
			}
			if (state_status(_states[_cur_state]) == STATE_DONE)
				handle_state_end();
		}
	}

	if (queued_state())
		set_state(queued_state());

	drop_flag(QD_OBJ_STATE_CHANGE_FLAG);

	// Если текущ. позиция не соответствует той, что была в начале кванта, то
	// объект изменился
	if (R().x != beg_r.x || R().y != beg_r.y || R().z != beg_r.z)
		_last_chg_time = qdGameDispatcher::get_dispatcher()->get_time();

// debugC(3, kDebugLog, "%s %d %d %d %d", name(), _animation.cur_time(), R().x, R().y, R().z);
}

bool qdGameObjectAnimated::load_script_body(const xml::tag *p) {
	qdGameObject::load_script_body(p);

	Vect3f v;
	qdGameObjectState *st;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_BOUND:
			xml::tag_buffer(*it) > v.x > v.y > v.z;
			set_bound(v);
			break;
		case QDSCR_OBJECT_STATE:
		case QDSCR_OBJECT_STATE_STATIC:
			st = new qdGameObjectStateStatic;
			st->load_script(&*it);
			add_state(st);
			break;
		case QDSCR_OBJECT_STATE_WALK:
			st = new qdGameObjectStateWalk;
			st->load_script(&*it);
			add_state(st);
			break;
		case QDSCR_OBJECT_STATE_MASK:
			st = new qdGameObjectStateMask;
			st->load_script(&*it);
			add_state(st);
			break;
		case QDSCR_OBJECT_DEFAULT_POS:
			xml::tag_buffer(*it) > v.x > v.y > v.z;
			set_default_pos(v);
			set_pos(v);
			break;
		case QDSCR_INVENTORY_CELL_TYPE:
			set_inventory_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INVENTORY:
			set_inventory_name(it->data());
			break;
		}
	}

	set_default_pos(R());

	return true;
}

int qdGameObjectAnimated::idle_time() const {
	return qdGameDispatcher::get_dispatcher()->get_time() - _last_chg_time;
}

int qdGameObjectAnimated::shadow_color() const {
	if (_shadow_alpha != QD_NO_SHADOW_ALPHA)
		return _shadow_color;

	if (_cur_state != -1)
		return _states[_cur_state]->shadow_color();

	return 0;
}

int qdGameObjectAnimated::shadow_alpha() const {
	if (_shadow_alpha != QD_NO_SHADOW_ALPHA)
		return _shadow_alpha;

	if (_cur_state != -1)
		return _states[_cur_state]->shadow_alpha();

	return QD_NO_SHADOW_ALPHA;
}

bool qdGameObjectAnimated::insert_state(int iBefore, qdGameObjectState *p) {
	p->set_owner(this);
	p->inc_reference_count();

	_states.insert(_states.begin() + iBefore, p);

	if (!p->name()) {
		Common::String nameStr;
		nameStr += Common::String::format("\xd1\xee\xf1\xf2\xee\xff\xed\xe8\xe5 %d", max_state()); //  "Состояние %d" -- "State %d"
		p->set_name(nameStr.c_str());
	}
	return true;
}

bool qdGameObjectAnimated::add_state(qdGameObjectState *p) {
	p->set_owner(this);
	p->inc_reference_count();

	_states.push_back(p);

	if (!p->name()) {
		Common::String nameStr;
		nameStr += Common::String::format("\xd1\xee\xf1\xf2\xee\xff\xed\xe8\xe5 %d", max_state()); //  "Состояние %d" -- "State %d"
		p->set_name(nameStr.c_str());
	}

	return true;
}

qdGameObjectState *qdGameObjectAnimated::remove_state(int state_num) {
	if (state_num < 0 || state_num >= max_state()) return 0;

	qdGameObjectStateVector::iterator it = _states.begin() + state_num;

	qdGameObjectState *p = *it;
	_states.erase(it);

	p->dec_reference_count();

	if (_cur_state >= max_state())
		--_cur_state;

	return p;
}

bool qdGameObjectAnimated::remove_state(qdGameObjectState *p) {
	qdGameObjectStateVector::iterator it = Common::find(_states.begin(), _states.end(), p);
	if (it != _states.end()) {
		_states.erase(it);
		p->dec_reference_count();

		if (_cur_state >= max_state())
			set_state(--_cur_state);

		return true;
	}
	return false;
}

bool qdGameObjectAnimated::save_script_body(Common::WriteStream &fh, int indent) const {
	qdGameObject::save_script_body(fh, indent);

	for (auto &is : _states) {
		if (!is->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER) || owner()->named_object_type() == QD_NAMED_OBJECT_DISPATCHER) {
			is->save_script(fh, indent + 1);
		} else {
			debugC(3, kDebugLog, "State not saved: ");
			if (owner() && owner()->name()) {
				debugC(3, kDebugLog, "%s::", transCyrillic(owner()->name()));
			}

			debugC(3, kDebugLog, "%s::%s", transCyrillic(name()), transCyrillic(is->name()));
		}
	}

	if (_inventory_type) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<inventory_cell_type>%d</inventory_cell_type>\r\n", _inventory_type));
	}

	if (!_inventory_name.empty()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<inventory>%s</inventory>\r\n", qdscr_XML_string(_inventory_name.c_str())));
	}

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<bound>%f %f %f</bound>\r\n", _bound.x, _bound.y, _bound.z));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<default_pos>%f %f %f</default_pos>\r\n", _default_r.x, _default_r.y, _default_r.z));

	return true;
}

bool qdGameObjectAnimated::load_script(const xml::tag *p) {
	return load_script_body(p);
}

bool qdGameObjectAnimated::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<animated_object name=\"%s\">\r\n", qdscr_XML_string(name())));

	save_script_body(fh, indent);

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</animated_object>\r\n");

	return true;
}

bool qdGameObjectAnimated::load_resources() {
	debugC(4, kDebugLoad, "qdGameObjectAnimated::load_resources(): name: %s _cur_state: %d max_state: %d", transCyrillic(name()), _cur_state, max_state());
	if (_cur_state != -1) {
		_states[_cur_state]->register_resources();
		_states[_cur_state]->load_resources();

		if (_animation.parent()) {
			int fl = _animation.flags();
			float tm = _animation.cur_time_rel();
			_animation.parent()->create_reference(&_animation);
			_animation.clear_flags();
			_animation.set_flag(fl);
			_animation.set_time_rel(tm);
		}
	} else
		set_default_state();

	for (int i = 0; i < max_state(); i++) {
		debugC(1, kDebugTemp, "i: %d forced_load: %d", i, _states[i]->forced_load());
		if (i != _cur_state && _states[i]->forced_load()) {
			_states[i]->register_resources();
			_states[i]->load_resources();
		}
	}

	init_grid_zone();
	return true;
}

void qdGameObjectAnimated::free_resources() {
	for (int i = 0; i < max_state(); i++) {
		_states[i]->unregister_resources();
		_states[i]->free_resources();
	}
}

void qdGameObjectAnimated::set_state(int st) {
	debugC(3, kDebugGraphics, "qdGameObjectAnimated::set_state(%d)", st);
	// Указание на смену состояния => объект меняется (устанавливаем время изм.)
	_last_chg_time = qdGameDispatcher::get_dispatcher()->get_time();

	if (max_state() && st >= 0 && st <= max_state()) {
		qdGameObjectState *p = _states[st];

		if (p->activation_delay() > 0.001f) {
			debugC(3, kDebugGraphics, "[%d] state waits %s %s", g_system->getMillis(), transCyrillic(p->name()), transCyrillic(get_state(st)->name()));

			if (!p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER)) {
				p->set_activation_timer();
				p->set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
				set_queued_state(p);
				return;
			}

			if (!p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END))
				return;

			set_queued_state(NULL);
		}

		p->set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);

		if (p->is_in_triggers())
			set_flag(QD_OBJ_IS_IN_TRIGGER_FLAG);

		if (p->has_camera_mode() && owner())
			static_cast<qdGameScene * >(owner())->set_camera_mode(p->camera_mode(), this);

		if (_cur_state != -1 && _cur_state < max_state()) {
			_states[_cur_state]->stop_sound();
			if (!_states[_cur_state]->forced_load() && _cur_state != st) {
				_states[_cur_state]->unregister_resources();
				p->register_resources();
				_states[_cur_state]->free_resources();
			} else
				p->register_resources();
		} else
			p->register_resources();

		qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
		assert(dp);

		if (_cur_state != -1 && _cur_state < max_state() && _states[_cur_state]->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY) && !p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY)) {
			dp->remove_from_inventory(this);

			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);
			set_pos(default_R());
		}

		if (_cur_state != st) {
			set_last_state(get_cur_state());
			set_last_inventory_state(get_cur_state());
			set_cur_state(st);
		}

		p->load_resources();
		p->start();

		dp->screen_texts_dispatcher().clear_texts(this);
		if (p->has_text() && !p->has_text_delay() && !p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DIALOG_PHRASE))
			dp->screen_texts_dispatcher().add_text(qdGameDispatcher::TEXT_SET_DIALOGS, qdScreenText(p->text(), p->text_format(), Vect2i(0, 0), p));

		p->drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
		drop_flag(QD_OBJ_HIDDEN_FLAG);

		p->drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
		p->drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_ZONE)) {
			dp->remove_from_inventory(this);
			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);

			// Ставим объект по клику мыши
			Vect2f v2 = qdGameDispatcher::get_dispatcher()->get_active_scene()->
			            mouse_click_pos();
			Vect3f v3 = Vect3f(v2.x, v2.y, 0);
			set_pos(v3);
		}
		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_ON_OBJECT)) {
			dp->remove_from_inventory(this);
			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);

			// Ставим объект, состояние которого активируется, в центр объекта по
			// которому был произведен клик (для этого ищем его в текущей сцене)
			Vect2f mouse_pos = qdGameDispatcher::get_dispatcher()->mouse_cursor_pos();
			qdGameObject *pObj = qdGameDispatcher::get_dispatcher()->
			                     get_active_scene()->
			                     get_hitted_obj(mouse_pos.x, mouse_pos.y);
			if (NULL != pObj)
				set_pos(pObj->R());
		}

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_SAVE))
			dp->set_auto_save(p->autosave_slot());

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_LOAD))
			dp->set_auto_load(p->autosave_slot());

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_IN))
			dp->set_fade(true, p->fade_time());

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_OUT))
			dp->set_fade(false, p->fade_time());

		restore_grid_zone();
		init_grid_zone();

		switch (p->state_type()) {
		case qdGameObjectState::STATE_STATIC:
			set_animation_info(static_cast<qdGameObjectStateStatic *>(p)->animation_info());
			break;
		case qdGameObjectState::STATE_MASK:
			_animation.clear();
			break;
		default:
			break;
		}

		if (!p->coords_animation()->is_empty()) {
			p->coords_animation()->start();
			get_animation()->set_time_rel(p->coords_animation()->animation_phase());
		}

		if (p->has_transform()) {
			set_screen_rotation(p->transform().angle(), p->transform_speed().angle());
			set_screen_scale(p->transform().scale(), p->transform_speed().scale());

			if (!_last_state)
				_current_transform = _target_transform;
		}

		if (!p->has_sound_delay())
			p->play_sound();
		return;
	}

	if (st == -1) {
		if (_cur_state != -1 && _cur_state < max_state()) {
			_states[_cur_state]->stop_sound();
			_states[_cur_state]->unregister_resources();
			_states[_cur_state]->free_resources();
		}

		set_cur_state(st);
		get_animation()->clear();
	}
}

void qdGameObjectAnimated::set_state(qdGameObjectState *p) {
	for (int i = 0; i < max_state(); i++) {
		if (_states[i] == p) {
			set_state(i);
			return;
		}
	}
}

bool qdGameObjectAnimated::is_visible() const {
	if (max_state() && _cur_state != -1) {
		if (_states[_cur_state]->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDDEN))
			return false;
	}

	return qdGameObject::is_visible();
}

void qdGameObjectAnimated::set_bound(const Vect3f &b) {
	_bound = b;
	Vect3f b2 = b / 2.0f;
	_radius = b2.norm();
}

bool qdGameObjectAnimated::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	if (ev == mouseDispatcher::EV_LEFT_DOWN)
		return true;

	return false;
}

void qdGameObjectAnimated::debug_redraw() const {
	if (!is_visible()) return;

	if (has_bound()) {
		draw_bound();
//		draw_grid_zone(_grid_size);
	}

	Vect3f pos = R();
	Vect2s scr_pos = qdCamera::current_camera()->global2scr(pos);

	const int NET_PROJ_SIZE = 10;
	const int OBJ_CENTER_SIZE = 6;

	pos.z = qdCamera::current_camera()->get_grid_center().z;
	Vect2s proj_pos = qdCamera::current_camera()->global2scr(pos);
	//прорисовываем проекцию на сетку
	grDispatcher::instance()->rectangle(
	    proj_pos.x - (NET_PROJ_SIZE >> 1),
	    proj_pos.y - (NET_PROJ_SIZE >> 1),
	    NET_PROJ_SIZE, NET_PROJ_SIZE,
	    0x00FF0000, 0x000000FF, GR_FILLED);

	//прорисовываем центр объекта
	grDispatcher::instance()->rectangle(
	    scr_pos.x - (OBJ_CENTER_SIZE >> 1),
	    scr_pos.y - (OBJ_CENTER_SIZE >> 1),
	    OBJ_CENTER_SIZE, OBJ_CENTER_SIZE,
	    0x00FF0000, 0x00FF0000, GR_FILLED);
	grDispatcher::instance()->line(proj_pos.x, proj_pos.y, scr_pos.x, scr_pos.y, 0x00FF0000);

	Vect2s ssz = screen_size();

	if (!_current_transform()) {
		grDispatcher::instance()->rectangle(
		    scr_pos.x - ssz.x / 2,
		    scr_pos.y - ssz.y / 2,
		    ssz.x, ssz.y,
		    0x00FF0000, 0x000000FF, GR_OUTLINED);
	} else {
		Vect2f scale(_current_transform.scale());

		ssz.x = round(float(ssz.x) * scale.x);
		ssz.y = round(float(ssz.y) * scale.y);

		float sn = sinf(-_current_transform.angle());
		float cs = cosf(-_current_transform.angle());

		Vect2i v0, v1, p0, p1;

		v0 = Vect2i(-ssz.x / 2, -ssz.y / 2);
		v1 = Vect2i(-ssz.x / 2, +ssz.y / 2);

		p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
		p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

		p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
		p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

		grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

		v0 = Vect2i(-ssz.x / 2, +ssz.y / 2);
		v1 = Vect2i(+ssz.x / 2, +ssz.y / 2);

		p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
		p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

		p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
		p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

		grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

		v0 = Vect2i(+ssz.x / 2, +ssz.y / 2);
		v1 = Vect2i(+ssz.x / 2, -ssz.y / 2);

		p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
		p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

		p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
		p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

		grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

		v0 = Vect2i(+ssz.x / 2, -ssz.y / 2);
		v1 = Vect2i(-ssz.x / 2, -ssz.y / 2);

		p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
		p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

		p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
		p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

		grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);
	}

	grDispatcher::instance()->rectangle(
	    screen_pos().x - 2,
	    screen_pos().y - 2,
	    4, 4,
	    0x00FF0000, 0x000000FF, GR_FILLED);

	static Common::String buf;
	if (get_debug_info(buf))
		grDispatcher::instance()->drawText(scr_pos.x, scr_pos.y - 20, grDispatcher::instance()->make_rgb888(255, 255, 255), buf.c_str());

	if (const qdGameObjectStateMask* st = dynamic_cast<const qdGameObjectStateMask*>(get_cur_state()))
		st->draw_mask(grDispatcher::instance()->make_rgb(255,255,255));

}

bool qdGameObjectAnimated::hit(int x, int y) const {
	if (!is_visible()) return false;

	Vect2s sr = screen_pos();
	if (_cur_state != -1) {
		if (_states[_cur_state]->state_type() == qdGameObjectState::STATE_MASK)
			return static_cast<qdGameObjectStateMask * >(_states[_cur_state])->hit(x, y);
		else {
			if (_current_transform()) {
				x -= sr.x;
				y -= sr.y;

				float cs = cosf(_current_transform.angle());
				float sn = sinf(_current_transform.angle());

				int xx = round(1.f / _current_transform.scale().x * (float(x) * cs + float(y) * sn));
				int yy = round(1.f / _current_transform.scale().y * (float(-x) * sn + float(y) * cs));

				return _animation.hit(xx, yy);
			} else
				return _animation.hit(x - sr.x, y - sr.y);
		}
	}

	return false;
}

qdGameObjectState *qdGameObjectAnimated::get_state(const char *state_name) {
	for (auto &it : _states) {
		if (it-> name() && !strcmp(it->name(), state_name)) {
			return it;
		}
	}

	return NULL;
}

const qdGameObjectState *qdGameObjectAnimated::get_state(const char *state_name) const {
	for (auto &it : _states) {
		const qdGameObjectState *p = it;
		if (it->name() && !strcmp(it->name(), state_name))
			return p;
	}

	return NULL;
}

qdGameObjectState *qdGameObjectAnimated::get_state(int state_index) {
	if (state_index >= 0 && state_index < max_state())
		return _states[state_index];

	return NULL;
}

const qdGameObjectState *qdGameObjectAnimated::get_state(int state_index) const {
	if (state_index >= 0 && state_index < max_state())
		return _states[state_index];

	return NULL;
}

qdGameObjectAnimated::StateStatus qdGameObjectAnimated::state_status(const qdGameObjectState *p) const {
	if (_queued_state == p)
		return STATE_QUEUED;

	if (_cur_state != -1 && _states[_cur_state] == p) {
		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED))
			return STATE_ACTIVE;

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_SKIP)) {
			if (mouseDispatcher::instance()->check_event(mouseDispatcher::EV_LEFT_DOWN) || mouseDispatcher::instance()->check_event(mouseDispatcher::EV_RIGHT_DOWN))
				return STATE_DONE;
		}

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_SOUND_SYNC)) {
			if (!p->is_sound_finished())
				return STATE_ACTIVE;
		}

		if (p->work_time() > 0.001f) {
			if (p->cur_time() >= p->work_time())
				return STATE_DONE;
			else
				return STATE_ACTIVE;
		}

		if (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDDEN))
			return STATE_DONE;

		if (!p->coords_animation()->is_empty()) {
			if (p->coords_animation()->size() > 1) {
				if (!p->coords_animation()->is_finished())
					return STATE_ACTIVE;

				return STATE_DONE;
			}
		}

		switch (p->state_type()) {
		case qdGameObjectState::STATE_STATIC: {
				const qdGameObjectStateStatic *sp = static_cast<const qdGameObjectStateStatic *>(p);
				if (const qdAnimation * anm = sp->animation()) {
					if (anm->is_reference(get_animation())) {
						if (!_animation.is_finished())
							return STATE_ACTIVE;
					} else
						return STATE_INACTIVE;
				}
			}
			return STATE_DONE;
		case qdGameObjectState::STATE_WALK:
			return STATE_DONE;
		case qdGameObjectState::STATE_MASK:
			return STATE_DONE;
		}
		return STATE_DONE;
	}

	return STATE_INACTIVE;
}

void qdGameObjectAnimated::draw_contour(uint32 color) const {
	if (!get_animation()->is_empty()) {
		Vect2s scrCoord = screen_pos();
		get_animation()->draw_contour(scrCoord.x, scrCoord.y, color);
	}
}

int qdGameObjectAnimated::mouse_cursor_ID() const {
	if (_cur_state != -1)
		return _states[_cur_state]->mouse_cursor_ID();

	return qdGameObjectState::CURSOR_UNASSIGNED;
}

void qdGameObjectAnimated::restore_state() {
	if (qdGameObjectState * st = _states[_cur_state]->prev_state()) {
		_states[_cur_state]->set_prev_state(0);
		set_state(st);
	}
}

void qdGameObjectAnimated::merge_states(qdGameObjectAnimated *p) {
	if (_states.empty() || (!p->_states.empty() && _states[0] != p->_states[0])) {
		for (int i = 0; i < p->max_state(); i++) {
			insert_state(i, p->_states[i]);
			p->_states[i]->set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
		}
	}
}

void qdGameObjectAnimated::split_states(qdGameObjectAnimated *p) {
	int st = _cur_state;

	if (!_states.empty() && !p->_states.empty() && _states[0] == p->_states[0]) {
		for (int i = 0; i < p->max_state(); i++) {
			remove_state(0);
			p->_states[i]->drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
			p->_states[i]->set_owner(p);
		}
	}

	_cur_state = st;
}

bool qdGameObjectAnimated::init_grid_zone() {
	if (has_bound() && owner() && owner()->named_object_type() == QD_NAMED_OBJECT_SCENE) {
		Vect3f b = bound(false);
		_grid_size.x = b.xi() / static_cast<qdGameScene *>(owner())->get_camera()->get_cell_sx();
		_grid_size.y = b.yi() / static_cast<qdGameScene *>(owner())->get_camera()->get_cell_sy();

		if (_grid_size.x < 1) _grid_size.x = 1;
		if (_grid_size.y < 1) _grid_size.y = 1;

		return true;
	}
	return false;
}

bool qdGameObjectAnimated::toggle_grid_zone(bool make_walkable) {
	if (make_walkable)
		return drop_grid_zone_attributes(sGridCell::CELL_OCCUPIED);
	else
		return set_grid_zone_attributes(sGridCell::CELL_OCCUPIED);
}

bool qdGameObjectAnimated::save_grid_zone() {
	_grid_r = R();
	return true;
}

bool qdGameObjectAnimated::restore_grid_zone() {
	return toggle_grid_zone(true);
}

qdGameObjectState *qdGameObjectAnimated::get_inventory_state() {
	if (_last_inventory_state) return _last_inventory_state;

	if (_states.size()) {
		for (auto &it : _states) {
			if (it->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
				return it;
		}
	}
	return NULL;
}

const Vect3f &qdGameObjectAnimated::bound(bool perspective_correction) const {
	if (_cur_state != -1 && _states[_cur_state]->has_bound())
		return _states[_cur_state]->bound();

	return _bound;
}

float qdGameObjectAnimated::radius() const {
	if (_cur_state != -1 && _states[_cur_state]->has_bound())
		return _states[_cur_state]->radius();

	return _radius;
}

void qdGameObjectAnimated::draw_bound(Vect3f r,
                                      Vect3f const &bound,
                                      int const color) const {
	float const bx = bound.x / 2.0f;
	float const by = bound.y / 2.0f;
	float const bz = bound.z / 2.0f;
	if (const qdCamera * cp = qdCamera::current_camera()) {
		if (named_object_type() == QD_NAMED_OBJECT_ANIMATED_OBJ)
			r.z = bz + cp->get_grid_center().z;

		Vect2s v0 = cp->global2scr(Vect3f(r.x - bx, r.y - by, r.z - bz));
		Vect2s v1 = cp->global2scr(Vect3f(r.x - bx, r.y + by, r.z - bz));
		Vect2s v2 = cp->global2scr(Vect3f(r.x + bx, r.y + by, r.z - bz));
		Vect2s v3 = cp->global2scr(Vect3f(r.x + bx, r.y - by, r.z - bz));

		Vect2s _v0 = cp->global2scr(Vect3f(r.x - bx, r.y - by, r.z + bz));
		Vect2s _v1 = cp->global2scr(Vect3f(r.x - bx, r.y + by, r.z + bz));
		Vect2s _v2 = cp->global2scr(Vect3f(r.x + bx, r.y + by, r.z + bz));
		Vect2s _v3 = cp->global2scr(Vect3f(r.x + bx, r.y - by, r.z + bz));

		grDispatcher::instance()->line(v0.x, v0.y, v1.x, v1.y, color, 4, true);
		grDispatcher::instance()->line(v1.x, v1.y, v2.x, v2.y, color, 4, true);
		grDispatcher::instance()->line(v2.x, v2.y, v3.x, v3.y, color, 4, true);
		grDispatcher::instance()->line(v3.x, v3.y, v0.x, v0.y, color, 4, true);

		grDispatcher::instance()->line(_v0.x, _v0.y, _v1.x, _v1.y, color, 4, true);
		grDispatcher::instance()->line(_v1.x, _v1.y, _v2.x, _v2.y, color, 4, true);
		grDispatcher::instance()->line(_v2.x, _v2.y, _v3.x, _v3.y, color, 4, true);
		grDispatcher::instance()->line(_v3.x, _v3.y, _v0.x, _v0.y, color, 4, true);

		grDispatcher::instance()->line(v0.x, v0.y, _v0.x, _v0.y, color, 4, true);
		grDispatcher::instance()->line(v1.x, v1.y, _v1.x, _v1.y, color, 4, true);
		grDispatcher::instance()->line(v2.x, v2.y, _v2.x, _v2.y, color, 4, true);
		grDispatcher::instance()->line(v3.x, v3.y, _v3.x, _v3.y, color, 4, true);
	}
}

void qdGameObjectAnimated::draw_bound() const {
	const int cl = 0xFFFFFF;
	draw_bound(R(), bound(), cl);
}

bool qdGameObjectAnimated::inters_with_bound(Vect3f bnd, Vect3f cen,
        bool perspective_correction) const {
	Vect3f bnd_a = cen - bnd / 2;
	Vect3f bnd_b = cen + bnd / 2;
	Vect3f obj_a = R() - bound(perspective_correction) / 2;
	Vect3f obj_b = R() + bound(perspective_correction) / 2;

	if ((MAX(bnd_a.x, obj_a.x) <= MIN(bnd_b.x, obj_b.x)) &&
	        (MAX(bnd_a.y, obj_a.y) <= MIN(bnd_b.y, obj_b.y)) &&
	        (MAX(bnd_a.z, obj_a.z) <= MIN(bnd_b.z, obj_b.z)))
		return true;
	else return false;
}

int qdGameObjectAnimated::num_directions() const {
	if (_cur_state != -1 && _states[_cur_state]->state_type() == qdGameObjectState::STATE_WALK) {
		qdAnimationSet *p = static_cast<qdGameObjectStateWalk *>(_states[_cur_state])->animation_set();
		if (p)
			return p->size();
	}

	return 0;
}

bool qdGameObjectAnimated::auto_bound() {
	if (_cur_state != -1) {
		qdAnimation *ap = NULL;

		if (get_cur_state()->state_type() == qdGameObjectState::STATE_WALK) {
			ap = static_cast<qdGameObjectStateWalk *>(get_cur_state())->static_animation(3.0f / 2.0f * M_PI);
		} else {
			if (get_cur_state()->state_type() == qdGameObjectState::STATE_STATIC)
				ap = static_cast<qdGameObjectStateStatic * >(get_cur_state())->animation();
		}

		if (ap) {
			bool res_flag = false;
			if (!ap->is_resource_loaded()) {
				ap->load_resource();
				res_flag = true;
			}

			set_bound(Vect3f(ap->picture_size_x(), ap->picture_size_x(), ap->picture_size_y()));

			if (res_flag)
				ap->free_resource();

			return true;
		}
	}

	return false;
}

bool qdGameObjectAnimated::handle_state_end() {
	qdGameObjectState *sp = _states[_cur_state];

	if (sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE))
		restore_state();

	if (sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDE_OBJECT)) {
		if (qdGameDispatcher * gp = qd_get_game_dispatcher()) {
			if (gp->mouse_object()->object() == this) {
				gp->mouse_object()->take_object(NULL);
			} else
				gp->remove_from_inventory(this);
		}
		sp->stop_sound();
		_animation.clear();
		set_flag(QD_OBJ_HIDDEN_FLAG);
	}

	if (sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY) && !sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED)) {
		if (/*qdGameObjectState * p = */get_inventory_state()) {
			qdGameDispatcher *gp = qd_get_game_dispatcher();
			if (!gp || !gp->put_to_inventory(this))
				sp->set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
		}
	}

	if (sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_SOUND_SYNC | qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_SKIP) && !sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE)) {
		sp->stop_sound();
		get_animation()->stop();
		get_animation()->set_time_rel(0.0f);
	}

	if (sp->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATE_PERSONAGE)) {
		if (owner() && owner()->named_object_type() == QD_NAMED_OBJECT_SCENE)
			static_cast<qdGameScene * >(owner())->set_active_object(this);
	}

	return true;
}

bool qdGameObjectAnimated::update_screen_pos() {
	if (qdGameObject::update_screen_pos()) {
		Vect2i r = get_screen_R();
		Vect2i delta = (_cur_state != -1) ? Vect2i(_states[_cur_state]->center_offset()) : Vect2i(0, 0);

		if (_current_transform()) {
			if (delta.x || delta.y) {
				Vect2f scale(_current_transform.scale());

				delta.x = round(float(delta.x) * scale.x);
				delta.y = round(float(delta.y) * scale.y);

				float angle = _current_transform.angle();

				r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
				r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			}

			set_screen_R(r);
		} else
			set_screen_R(r + delta);

		return true;
	}

	return false;
}

void qdGameObjectAnimated::set_default_state() {
	if (qdGameObjectState * p = get_default_state())
		set_state(p);
	else {
		set_state(-1);
	}
}

qdGameObjectState *qdGameObjectAnimated::get_default_state() {
	for (uint i = 0; i < _states.size(); i++) {
		if (!_states[i]->is_in_triggers() && !_states[i]->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
			return _states[i];
	}

	return NULL;
}

const qdGameObjectState *qdGameObjectAnimated::get_default_state() const {
	for (uint i = 0; i < _states.size(); i++) {
		if (!_states[i]->is_in_triggers() && !_states[i]->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
			return _states[i];
	}

	return NULL;
}

int qdGameObjectAnimated::get_state_index(const qdGameObjectState *p) const {
	for (uint i = 0; i < _states.size(); i++) {
		if (_states[i] == p)
			return i;
	}

	return -1;
}

bool qdGameObjectAnimated::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(4, kDebugSave, "    qdGameObjectAnimated::load_data before: %d", (int)fh.pos());
	if (!qdGameObject::load_data(fh, save_version)) return false;

	_cur_state = fh.readSint32LE();

	for (uint i = 0; i < _states.size(); i++) {
		if (!_states[i]->load_data(fh, save_version))
			return false;
	}

	if (!_animation.load_data(fh, save_version)) return false;

	int idx = fh.readSint32LE();
	if (idx != -1)
		_queued_state = get_state(idx);
	else
		_queued_state = NULL;

	idx = fh.readSint32LE();
	if (idx != -1)
		_last_state = get_state(idx);
	else
		_last_state = NULL;

	if (save_version >= 103) {
		idx = fh.readSint32LE();
		if (idx != -1)
			_last_inventory_state = get_state(idx);
		else
			_last_inventory_state = NULL;
	}

	_inventory_cell_index = fh.readSint32LE();
	_last_chg_time = fh.readUint32LE();

	debugC(4, kDebugSave, "    qdGameObjectAnimated::load_data after: %d", (int)fh.pos());

	return true;
}

bool qdGameObjectAnimated::save_data(Common::WriteStream &fh) const {
	debugC(4, kDebugSave, "    qdGameObjectAnimated::save_data before: %d", (int)fh.pos());
	if (!qdGameObject::save_data(fh)) return false;

	fh.writeSint32LE(_cur_state);

	for (uint i = 0; i < _states.size(); i++) {
		if (!_states[i]->save_data(fh))
			return false;
	}

	if (!_animation.save_data(fh)) return false;

	int idx = -1;
	if (_queued_state)
		idx = get_state_index(_queued_state);
	fh.writeSint32LE(idx);

	idx = -1;
	if (_last_state)
		idx = get_state_index(_last_state);
	fh.writeSint32LE(idx);

	idx = -1;
	if (_last_inventory_state)
		idx = get_state_index(_last_inventory_state);
	fh.writeSint32LE(idx);

	fh.writeSint32LE(_inventory_cell_index);

	fh.writeSint32LE(_last_chg_time);

	debugC(4, kDebugSave, "    qdGameObjectAnimated::save_data after: %d", (int)fh.pos());
	return true;
}

bool qdGameObjectAnimated::is_state_active(const char *state_name) const {
	if (_cur_state != -1 && !strcmp(_states[_cur_state]->name(), state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::was_state_previous(const char *state_name) const {
	if (_last_state && !strcmp(_last_state->name(), state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::is_state_waiting(const char *state_name) const {
	if (_queued_state && !scumm_stricmp(_queued_state->name(), state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::was_state_active(const char *state_name) const {
	for (uint i = 0; i < _states.size(); i++) {
		if (!strcmp(_states[i]->name(), state_name))
			return _states[i]->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);
	}

	return false;
}

bool qdGameObjectAnimated::can_change_state(const qdGameObjectState *state) const {
	if (cur_state() != -1) {
		const qdGameObjectState *p = get_cur_state();
		if (state_status(p) == qdGameObjectAnimated::STATE_ACTIVE) {
			if (p->state_type() == qdGameObjectState::STATE_WALK && p->coords_animation()->size() > 1)
				return p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT);
			if (p->state_type() == qdGameObjectState::STATE_STATIC)
				return p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT);
		}
	}

	return true;
}

void qdGameObjectAnimated::draw_grid_zone(const Vect2s sz) const {
	const qdCamera *cp = qdCamera::current_camera();
	Vect2s gr = cp->get_cell_index(_grid_r.x, _grid_r.y);

	if (gr.x == -1) return;

	gr.x -= sz.x / 2;
	gr.y -= sz.y / 2;

	for (int y = 0; y < sz.y; y++) {
		for (int x = 0; x < sz.x; x++) {
			if (const sGridCell * cell = cp->get_cell(Vect2s(gr.x + x, gr.y + y))) {
				if (!cell->check_attribute(sGridCell::CELL_IMPASSABLE))
					cp->draw_cell(gr.x + x, gr.y + y, 0, 1, 0xFFFFFF);
				else
					cp->draw_cell(gr.x + x, gr.y + y, 0, 1, 0xFF0000);
			}
		}
	}
}

bool qdGameObjectAnimated::set_grid_zone_attributes(const Vect2f &r, int attr) const {
	debugC(5, kDebugMovement, "qdGameObjectAnimated::set_grid_zone_attributes(): has_bound: %d named_object_type: %d", has_bound(), owner() ? owner()->named_object_type() : -1);

	if (has_bound() && owner() && owner()->named_object_type() == QD_NAMED_OBJECT_SCENE) {
		qdCamera *cp = static_cast<qdGameScene *>(owner())->get_camera();
		Vect2s sr = cp->get_cell_index(r.x, r.y);

		if (sr.x == -1) return false;

		cp->set_grid_attributes(sr, _grid_size, attr);

		return true;
	}

	return false;
}

bool qdGameObjectAnimated::drop_grid_zone_attributes(const Vect2f &r, int attr) const {
	if (has_bound() && owner() && owner()->named_object_type() == QD_NAMED_OBJECT_SCENE) {
		qdCamera *cp = static_cast<qdGameScene *>(owner())->get_camera();
		Vect2s sr = cp->get_cell_index(r.x, r.y);

		if (sr.x == -1) return false;

		cp->drop_grid_attributes(sr, _grid_size, attr);

		return true;
	}

	return false;
}

bool qdGameObjectAnimated::check_grid_zone_attributes(const Vect2f &r, int attr) const {
	if (has_bound() && owner() && owner()->named_object_type() == QD_NAMED_OBJECT_SCENE) {
		qdCamera *cp = static_cast<qdGameScene *>(owner())->get_camera();
		Vect2s sr = cp->get_cell_index(r.x, r.y);

		if (sr.x == -1) return false;

		return cp->check_grid_attributes(sr, _grid_size, attr);
	}

	return false;
}

bool qdGameObjectAnimated::set_grid_zone_attributes(int attr) const {
	return set_grid_zone_attributes(_grid_r, attr);
}

bool qdGameObjectAnimated::drop_grid_zone_attributes(int attr) const {
	return drop_grid_zone_attributes(_grid_r, attr);
}

bool qdGameObjectAnimated::check_grid_zone_attributes(int attr) const {
	return check_grid_zone_attributes(_grid_r, attr);
}

bool qdGameObjectAnimated::has_camera_mode() const {
	if (_cur_state != -1)
		return _states[_cur_state]->has_camera_mode();

	return false;
}

const qdCameraMode &qdGameObjectAnimated::camera_mode() const {
	if (_cur_state != -1)
		return _states[_cur_state]->camera_mode();

	static qdCameraMode md;
	return md;
}

bool qdGameObjectAnimated::init() {
	if (!qdGameObject::init()) return false;

	_last_state = NULL;

	set_pos(default_R());
	set_cur_state(-1);

	_queued_state = NULL;
	_last_inventory_state = NULL;

	for (uint i = 0; i < _states.size(); i++)
		_states[i]->init();

	return true;
}

grScreenRegion qdGameObjectAnimated::screen_region() const {
	if (is_visible()) {
		grScreenRegion reg = grScreenRegion_EMPTY;
		if (_current_transform()) {
			Vect2i r = screen_pos();
			/*
			            Vect2i delta = (_cur_state != -1) ? _states[_cur_state]->center_offset() : Vect2i(0,0);
			            if(delta.x || delta.y){
			                r -= delta;

			                Vect2f scale(_current_transform.scale());

			                delta.x = round(float(delta.x) * scale.x);
			                delta.y = round(float(delta.y) * scale.y);

			                float angle = _current_transform.angle();

			                r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
			                r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			            }
			*/
			if (const qdAnimationFrame * fp = _animation.get_cur_frame()) {
				Vect2f scale(_current_transform.scale());
				Vect2f size(fp->size_x(), fp->size_y());

				float sn = sinf(_current_transform.angle());
				float cs = cosf(_current_transform.angle());

				int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
				int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

				reg = grScreenRegion(0, 0, sx, sy);
				reg.move(r.x, r.y);
			}
		} else {
			reg = _animation.screen_region();
			reg.move(screen_pos().x, screen_pos().y);
		}

		return reg;
	} else
		return grScreenRegion_EMPTY;
}

void qdGameObjectAnimated::post_redraw() {
	_last_screen_region = screen_region();
	_last_screen_depth = screen_depth();
	_last_frame = _animation.get_cur_frame();
	_last_transform = _current_transform;

	_lastShadowColor = shadow_color();
	_lastShadowAlpha = shadow_alpha();
}

void qdGameObjectAnimated::set_states_owner() {
	for (int i = 0; i < max_state(); i++)
		_states[i]->set_owner(this);
}

qdGameObjectState *qdGameObjectAnimated::get_mouse_state() {
	if (_states.size()) {
		for (auto &it : _states) {
			if (it->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE))
				return it;
		}
	}
	return NULL;
}

qdGameObjectState *qdGameObjectAnimated::get_mouse_hover_state() {
	if (_states.size()) {
		for (auto &it : _states) {
			if (it->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE))
				return it;
		}
	}
	return NULL;
}

void qdGameObjectAnimated::draw_shadow(int offs_x, int offs_y, uint32 color, int alpha) const {
	if (alpha == QD_NO_SHADOW_ALPHA || get_animation()->is_empty())
		return;

	if (!_animation.is_empty()) {
		Vect2i r = screen_pos() + Vect2i(offs_x, offs_y);
		if (_current_transform()) {
			/*          Vect2i delta = (_cur_state != -1) ? _states[_cur_state]->center_offset() : Vect2i(0,0);
			            if(delta.x || delta.y){
			                r -= delta;

			                Vect2f scale(_current_transform.scale());

			                delta.x = round(float(delta.x) * scale.x);
			                delta.y = round(float(delta.y) * scale.y);

			                float angle = _current_transform.angle();

			                r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
			                r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			            }
			*/
			if (_current_transform.has_scale())
				_animation.draw_mask_rot(r.x, r.y, screen_depth(), _current_transform.angle(), grDispatcher::instance()->make_rgb(color), alpha, _current_transform.scale());
			else
				_animation.draw_mask_rot(r.x, r.y, screen_depth(), _current_transform.angle(), grDispatcher::instance()->make_rgb(color), alpha);
		} else
			_animation.draw_mask(r.x, r.y, screen_depth(), grDispatcher::instance()->make_rgb(color), alpha);
	}
}

bool qdGameObjectAnimated::get_debug_info(Common::String &buf) const {
	qdGameObject::get_debug_info(buf);
#ifdef __QD_DEBUG_ENABLE__
	if (const qdGameObjectState * p = get_cur_state()) {
		buf += Common::String::format("%s %f", p->name(), _animation.cur_time());

		if (queued_state())
			buf += Common::String::format("wait %s", queued_state()->name());
	}
#endif
	return true;
}
} // namespace QDEngine
