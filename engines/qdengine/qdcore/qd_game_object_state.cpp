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

#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_textdb.h"
#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/qdcore/qd_game_object_state.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_resource_dispatcher.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace QDEngine {

qdScreenTransform qdScreenTransform::ID;

bool qdScreenTransform::operator == (const qdScreenTransform &trans) const {
	return fabs(_angle - trans._angle) < FLT_EPS &&
	       fabs(_scale.x - trans._scale.x) < FLT_EPS && fabs(_scale.y - trans._scale.y) < FLT_EPS;
}

bool qdScreenTransform::change(float dt, const qdScreenTransform &target_trans, const qdScreenTransform &speed) {
	qdScreenTransform delta(getDeltaAngle(target_trans.angle(), angle()),
	                        target_trans.scale() - scale());

	qdScreenTransform delta_max = speed * dt;
	delta._angle = CLIP(delta._angle, -delta_max._angle, delta_max._angle);
	delta._scale.x = CLIP(delta._scale.x, -delta_max._scale.x, delta_max._scale.x);
	delta._scale.y = CLIP(delta._scale.y, -delta_max._scale.y, delta_max._scale.y);

	*this += delta;

	return (*this == target_trans);
}

qdGameObjectState::qdGameObjectState(qdGameObjectState::StateType tp) : _center_offset(0, 0),
	_state_type(tp),
	_prev_state(0),
	_mouse_cursor_id(CURSOR_UNASSIGNED),
	_work_time(0.0f),
	_cur_time(0.0f),
	_reference_count(0),
	_bound(Vect3f(3.f, 3.f, 3.f)),
	_activation_delay(0.0f),
	_activation_timer(0.0f),
	_is_sound_started(false),
	_is_text_shown(false),
	_radius(0.0f),
	_sound_delay(0.f),
	_text_delay(0.f),
	_rnd_move_radius(0.0f),
	_rnd_move_speed(0.0f),
	_autosave_slot(0),
	_fade_time(0.1f),
	_shadow_color(0),
	_shadow_alpha(QD_NO_SHADOW_ALPHA) {
	_coords_animation.set_owner(this);
	_sound_handle.set_owner(this);
}

qdGameObjectState::qdGameObjectState(const qdGameObjectState &st) : qdConditionalObject(st),
	_center_offset(st._center_offset),
	_state_type(st._state_type),
	_mouse_cursor_id(st._mouse_cursor_id),
	_work_time(st._work_time),
	_cur_time(st._cur_time),
	_sound_info(st._sound_info),
	_coords_animation(st._coords_animation),
	_prev_state(0),
	_text_ID(st._text_ID),
	_short_text_ID(st._short_text_ID),
	_reference_count(st._reference_count),
	_bound(st._bound),
	_activation_delay(st._activation_delay),
	_activation_timer(st._activation_timer),
	_radius(st._radius),
	_camera_mode(st._camera_mode),
	_is_sound_started(st._is_sound_started),
	_is_text_shown(false),
	_text_delay(st._text_delay),
	_sound_delay(st._sound_delay),
	_rnd_move_radius(st._rnd_move_radius),
	_rnd_move_speed(st._rnd_move_speed),
	_autosave_slot(st._autosave_slot),
	_text_format(st._text_format),
	_transform(st._transform),
	_transform_speed(st._transform_speed),
	_fade_time(st._fade_time),
	_shadow_color(st._shadow_color),
	_shadow_alpha(st._shadow_alpha) {
	_coords_animation.set_owner(this);
	_sound_handle.set_owner(this);
}

qdGameObjectState::~qdGameObjectState() {
}

bool qdGameObjectState::is_state_empty() const {
	return (!_sound_info.name() || !strlen(_sound_info.name()));
}

qdGameObjectState &qdGameObjectState::operator = (const qdGameObjectState &st) {
	if (this == &st) return *this;

	*static_cast<qdConditionalObject *>(this) = st;

	_center_offset = st._center_offset;
	_state_type = st._state_type;
	_mouse_cursor_id = st._mouse_cursor_id;
	_sound_info = st._sound_info;
	_text_delay = st._text_delay;
	_sound_delay = st._sound_delay;

	_prev_state = 0;

	_coords_animation = st._coords_animation;

	_coords_animation.set_owner(this);
	_sound_handle.set_owner(this);

	_text_ID = st._text_ID;
	_short_text_ID = st._short_text_ID;

	_work_time = st._work_time;
	_cur_time = st._cur_time;

	_reference_count = st._reference_count;

	_activation_delay = st._activation_delay;
	_activation_timer = st._activation_timer;

	_bound = st._bound;
	_radius = st._radius;
	_camera_mode = st._camera_mode;

	_rnd_move_radius = st._rnd_move_radius;
	_rnd_move_speed = st._rnd_move_speed;

	_autosave_slot = st._autosave_slot;
	_fade_time = st._fade_time;

	_shadow_color = st._shadow_color;
	_shadow_alpha = st._shadow_alpha;

	_transform = st._transform;
	_transform_speed = st._transform_speed;

	_text_format = st._text_format;

	_is_sound_started = st._is_sound_started;
	_is_text_shown = st._is_text_shown;

	return *this;
}

bool qdGameObjectState::register_resources() {
	if (qdSound *p = sound()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->register_resource(p, this);
	}

	return true;
}

bool qdGameObjectState::unregister_resources() {
	if (qdSound *p = sound()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->unregister_resource(p, this);
	}

	return true;
}

bool qdGameObjectState::load_resources() {
	if (qdSound *p = sound()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->load_resource(p, this);
	}

	return true;
}

bool qdGameObjectState::free_resources() {
	if (qdSound *p = sound()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->release_resource(p, this);
	}

	return true;
}

bool qdGameObjectState::is_active() const {
	if (owner() && static_cast<qdGameObjectAnimated *>(owner())->get_cur_state() == this)
		return true;

	return false;
}

bool qdGameObjectState::is_default() const {
	if (owner() && static_cast<qdGameObjectAnimated *>(owner())->get_default_state() == this)
		return true;

	return false;
}

#define defFlag(x) { qdGameObjectState::x, #x }

struct FlagsList {
	int f;
	const char *s;
} static flagList[] = {
	defFlag(QD_OBJ_STATE_FLAG_HIDDEN),
	defFlag(QD_OBJ_STATE_FLAG_NOT_IN_TRIGGERS),
	defFlag(QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE),
	defFlag(QD_OBJ_STATE_FLAG_HIDE_OBJECT),
	defFlag(QD_OBJ_STATE_FLAG_GLOBAL_OWNER),
	defFlag(QD_OBJ_STATE_FLAG_INVENTORY),
	defFlag(QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY),
	defFlag(QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED),
	defFlag(QD_OBJ_STATE_FLAG_HAS_BOUND),
	defFlag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER),
	defFlag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END),
	defFlag(QD_OBJ_STATE_FLAG_DIALOG_PHRASE),
	defFlag(QD_OBJ_STATE_FLAG_SOUND_SYNC),
	defFlag(QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT),
	defFlag(QD_OBJ_STATE_FLAG_WAS_ACTIVATED),
	defFlag(QD_OBJ_STATE_FLAG_DISABLE_WALK_INTERRUPT),
	defFlag(QD_OBJ_STATE_FLAG_MOUSE_STATE),
	defFlag(QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE),
	defFlag(QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY),
	defFlag(QD_OBJ_STATE_FLAG_FORCED_LOAD),
	defFlag(QD_OBJ_STATE_FLAG_ENABLE_SKIP),
	defFlag(QD_OBJ_STATE_FLAG_MOVE_TO_ZONE),
	defFlag(QD_OBJ_STATE_FLAG_MOVE_ON_OBJECT),
	defFlag(QD_OBJ_STATE_FLAG_ACTIVATE_PERSONAGE),
	defFlag(QD_OBJ_STATE_FLAG_AUTO_LOAD),
	defFlag(QD_OBJ_STATE_FLAG_AUTO_SAVE),
	defFlag(QD_OBJ_STATE_FLAG_FADE_IN),
	defFlag(QD_OBJ_STATE_FLAG_FADE_OUT),
};

Common::String qdGameObjectState::flag2str(int fl) const {
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

qdConditionalObject::trigger_start_mode qdGameObjectState::trigger_start() {
	if (!owner()) return qdConditionalObject::TRIGGER_START_FAILED;

	qdGameObjectAnimated *op = static_cast<qdGameObjectAnimated *>(owner());

	if (!op->check_flag(QD_OBJ_STATE_CHANGE_FLAG) && op->can_change_state(this)) {
		op->set_state(this);
		op->set_flag(QD_OBJ_STATE_CHANGE_FLAG | QD_OBJ_IS_IN_TRIGGER_FLAG);

		switch (op->state_status(this)) {
		case qdGameObjectAnimated::STATE_QUEUED:
			return qdConditionalObject::TRIGGER_START_WAIT;
		case qdGameObjectAnimated::STATE_INACTIVE:
			return qdConditionalObject::TRIGGER_START_FAILED;
		default:
			return qdConditionalObject::TRIGGER_START_ACTIVATE;
		}
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}

bool qdGameObjectState::trigger_can_start() const {
	if (!qdConditionalObject::trigger_can_start()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
			if (qdGameObjectAnimated *op = static_cast<qdGameObjectAnimated *>(owner())) {
				if (dp->is_on_mouse(op) || dp->is_in_inventory(op))
					return true;
			}
		}

		return false;
	} else
		return true;
}

bool qdGameObjectState::load_script_body(const xml::tag *p) {
	load_conditions_script(p);

	Vect2s vs;
	Vect3f vf;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_OBJECT_STATE_CURSOR_ID:
			set_mouse_cursor_ID(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_CAMERA_MODE:
			_camera_mode.load_script(&*it);
			break;
		case QDSCR_LENGTH:
			set_work_time(xml::tag_buffer(*it).get_float());
			break;
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_COORDS_ANIMATION:
			_coords_animation.load_script(&*it);
			break;
		case QDSCR_SOUND:
			set_sound_name(it->data());
			if (const xml::tag *tg = it->search_subtag(QDSCR_FLAG))
				_sound_info.set_flag(xml::tag_buffer(*tg).get_int());
			break;
		case QDSCR_OBJECT_STATE_CENTER_OFFSET:
			xml::tag_buffer(*it) > vs.x > vs.y;
			set_center_offset(vs);
			break;
		case QDSCR_BOUND:
			xml::tag_buffer(*it) > vf.x > vf.y > vf.z;
			set_bound(vf);
			set_flag(QD_OBJ_STATE_FLAG_HAS_BOUND);
			break;
		case QDSCR_OBJECT_STATE_DELAY:
			set_activation_delay(xml::tag_buffer(*it).get_float());
			break;
		case QDSCR_SHORT_TEXT:
			set_short_text_ID(it->data());
			break;
		case QDSCR_TEXT:
			set_full_text_ID(it->data());
			break;
		case QDSCR_OBJECT_STATE_SOUND_DELAY:
			xml::tag_buffer(*it) > _sound_delay;
			break;
		case QDSCR_OBJECT_STATE_TEXT_DELAY:
			xml::tag_buffer(*it) > _text_delay;
			break;
		case QDSCR_OBJECT_STATE_RND_MOVE:
			xml::tag_buffer(*it) > _rnd_move_radius > _rnd_move_speed;
			break;
		case QDSCR_TEXT_COLOR:
			_text_format.set_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_HOVER_COLOR:
			_text_format.set_hover_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_ALIGN:
			_text_format.set_arrangement(qdScreenTextFormat::arrangement_t(xml::tag_buffer(*it).get_int()));
			break;
		case QDSCR_SCREEN_TEXT_FORMAT:
			_text_format.load_script(&*it);
			break;
		case QDSCR_SCENE_SAVE_SLOT:
			xml::tag_buffer(*it) > _autosave_slot;
			break;
		case QDSCR_FADE_TIME:
			xml::tag_buffer(*it) > _fade_time;
			break;
		case QDSCR_SCREEN_TRANSFORM: {
			float angle, angle_sp;
			Vect2f scale, scale_sp;
			xml::tag_buffer(*it) > angle > angle_sp > scale.x > scale.y > scale_sp.x > scale_sp.y;

			_transform.set_angle(G2R(angle));
			_transform.set_scale(scale);

			_transform_speed.set_angle(G2R(angle_sp));
			_transform_speed.set_scale(scale_sp);
		}
		break;
		case QDSCR_GRID_ZONE_SHADOW_COLOR:
			xml::tag_buffer(*it) > _shadow_color;
			break;
		case QDSCR_GRID_ZONE_SHADOW_ALPHA:
			xml::tag_buffer(*it) > _shadow_alpha;
			break;
		}
	}

	drop_flag(QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
	return true;
}
bool qdGameObjectState::save_script_body(Common::WriteStream &fh, int indent) const {
	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	if (_mouse_cursor_id != CURSOR_UNASSIGNED) {
		fh.writeString(Common::String::format(" cursor_id=\"%d\"", _mouse_cursor_id));
	}

	fh.writeString(Common::String::format(" save_slot=\"%d\"", _autosave_slot));

	if (flags()) {
		if (debugChannelSet(-1, kDebugLog))
			fh.writeString(Common::String::format(" flags=\"%s\"", flag2str(flags()).c_str()));
		else
			fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (_work_time > 0.001f) {
		fh.writeString(Common::String::format(" length=\"%f\"", _work_time));
	}

	if (_activation_delay > 0.001f) {
		fh.writeString(Common::String::format(" state_activation_delay=\"%f\"", _activation_delay));
	}

	if (!_short_text_ID.empty()) {
		fh.writeString(Common::String::format(" short_text=\"%s\"", qdscr_XML_string(_short_text_ID.c_str())));
	}

	if (!_text_ID.empty()) {
		fh.writeString(Common::String::format(" text=\"%s\"", qdscr_XML_string(_text_ID.c_str())));
	}

	if (_center_offset.x || _center_offset.y) {
		fh.writeString(Common::String::format(" center_offset=\"%d %d\"", _center_offset.x, _center_offset.y));
	}

	if (has_bound()) {
		fh.writeString(Common::String::format(" bound=\"%f %f %f\"", _bound.x, _bound.y, _bound.z));
	}

	if (has_sound_delay()) {
		fh.writeString(Common::String::format(" sound_delay=\"%f\"", _sound_delay));
	}

	if (has_text_delay()) {
		fh.writeString(Common::String::format(" text_delay=\"%f\"", _text_delay));
	}

	if (_rnd_move_radius > FLT_EPS && _rnd_move_speed > FLT_EPS) {
		fh.writeString(Common::String::format(" rnd_move=\"%f %f\"", _rnd_move_radius, _rnd_move_speed));
	}

	if (fabs(_fade_time - 0.1f) > FLT_EPS) {
		fh.writeString(Common::String::format(" fade_time=\"%f\"", _fade_time));
	}

	if (_shadow_color) {
		fh.writeString(Common::String::format(" shadow_color=\"%u\"", _shadow_color));
	}

	if (_shadow_alpha != QD_NO_SHADOW_ALPHA) {
		fh.writeString(Common::String::format(" shadow_alpha=\"%d\"", _shadow_alpha));
	}

	fh.writeString(">\r\n");

	if (!_coords_animation.is_empty()) {
		_coords_animation.save_script(fh, indent + 1);
	}

	if (has_camera_mode()) {
		_camera_mode.save_script(fh, indent + 1);
	}

	if (has_sound()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<sound");

		if (_sound_info.flags()) {
			fh.writeString(Common::String::format(" flags=\"%d\"", _sound_info.flags()));
		}

		fh.writeString(Common::String::format(">%s</sound>\r\n", qdscr_XML_string(_sound_info.name())));
	}

	if (has_transform()) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<screen_transform>");
		fh.writeString(Common::String::format(" %f %f", R2G(_transform.angle()), R2G(_transform_speed.angle())));
		fh.writeString(Common::String::format(" %f %f", _transform.scale().x, _transform.scale().y));
		fh.writeString(Common::String::format(" %f %f", _transform_speed.scale().x, _transform_speed.scale().y));
		fh.writeString("</screen_transform>\r\n");
	}

	_text_format.save_script(fh, indent + 1);

	save_conditions_script(fh, indent);

	return true;
}

bool qdGameObjectState::init() {
	if (!qdConditionalObject::init()) return false;
	// Ищем по ссылке и инициализируем найденным стартовый объект координатной анимации
	qdNamedObject *nam_obj = qdGameDispatcher::get_dispatcher()->
	                         get_named_object(&_coords_animation.start_object_ref());
	_coords_animation.set_start_object(dynamic_cast<const qdGameObject *>(nam_obj));


//	drop_flag(QD_OBJ_STATE_FLAG_GLOBAL_OWNER);

	drop_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
	drop_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);
	drop_flag(QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
	drop_flag(QD_OBJ_STATE_FLAG_WAS_ACTIVATED);

	return true;
}

bool qdGameObjectState::check_conditions() {
	if (has_conditions())
		return qdConditionalObject::check_conditions();

	return is_in_triggers();
}

float qdGameObjectState::work_time() const {
	if (check_flag(QD_OBJ_STATE_FLAG_SOUND_SYNC)) {
		if (qdSound *p = sound())
			return p->length() + _sound_delay;

		return 0.0f;
	}

	return _work_time;
}

void qdGameObjectState::quant(float dt) {
	if (is_active()) {
		_cur_time += dt;

		if (has_sound() && (state_type() != STATE_WALK || static_cast<qdGameObject *>(owner())->check_flag(QD_OBJ_MOVING_FLAG))) {
			if (!_is_sound_started) {
				if (!has_sound_delay() || _cur_time >= _sound_delay) {
					play_sound();
				}
			} else {
				if (check_sound_flag(qdSoundInfo::LOOP_SOUND_FLAG) && !is_sound_playing())
					play_sound();
			}
		}

		if (!_is_text_shown && has_text() && has_text_delay()) {
			if (_cur_time >= _text_delay) {
				qdGameDispatcher::get_dispatcher()->screen_texts_dispatcher().add_text(qdGameDispatcher::TEXT_SET_DIALOGS, qdScreenText(text(), text_format(), Vect2i(0, 0), this));
				_is_text_shown = true;
			}
		}
	}

	conditions_quant(dt);

	if (check_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER) && !check_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END)) {
		_activation_timer -= dt;
		if (_activation_timer <= 0.0f) {
			_activation_timer = 0.0f;
			set_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);
		}
	}
}

qdSound *qdGameObjectState::sound() const {
	if (_sound_info.name()) {
		if (qdGameScene *p = static_cast<qdGameScene *>(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdSound *snd = p->get_sound(_sound_info.name()))
				return snd;
		}

		if (qdGameDispatcher *p = qd_get_game_dispatcher())
			return p->get_sound(_sound_info.name());
	}

	return NULL;
}

bool qdGameObjectState::play_sound() {
	if (qdSound *p = sound()) {
		p->stop(sound_handle());
		_is_sound_started = true;
		return p->play(sound_handle(), check_sound_flag(qdSoundInfo::LOOP_SOUND_FLAG));
	}

	return false;
}

bool qdGameObjectState::set_sound_frequency(float frequency_coeff) const {
	if (qdSound *p = sound())
		return p->set_frequency(sound_handle(), frequency_coeff);

	return false;
}

bool qdGameObjectState::is_sound_finished() const {
	if (_sound_delay > 0.01f && _cur_time <= _sound_delay) return false;

	if (qdSound *p = sound())
		return p->is_stopped(sound_handle());

	return true;
}

bool qdGameObjectState::is_sound_playing() const {
	if (qdSound *p = sound())
		return !p->is_stopped(sound_handle());

	return false;
}

bool qdGameObjectState::stop_sound() const {
	if (qdSound *p = sound()) {
		return p->stop(sound_handle());
	}

	return false;
}

void qdGameObjectState::set_bound(const Vect3f &b) {
	_bound = b;
	Vect3f b2 = b / 2.0f;
	_radius = b2.norm();
}

bool qdGameObjectState::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdGameObjectState::load_data before: %ld", fh.pos());
	if (!qdConditionalObject::load_data(fh, save_version)) return false;

	_cur_time = fh.readFloatLE();

	int idx = fh.readSint32LE();

	if (idx != -1)
		_prev_state = static_cast<qdGameObjectAnimated *>(owner())->get_state(idx);
	else
		_prev_state = NULL;

	char cidx = fh.readByte();
	if (cidx) {
		cidx = fh.readByte();
		if (cidx) {
			/* float pos = */fh.readFloatLE();

			if (qdSound *snd = sound()) {
				if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
					dp->load_resource(snd, this);
					play_sound();
				}
			}

			_is_sound_started = true;
		}

		cidx = fh.readByte();
		_is_sound_started = (cidx) ? true : false;

		if (!_coords_animation.is_empty()) {
			if (!_coords_animation.load_data(fh, save_version))
				return false;
		}
	}

	debugC(3, kDebugSave, "  qdGameObjectState::load_data after: %ld", fh.pos());
	return true;
}

bool qdGameObjectState::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdGameObjectState::save_data before: %ld", fh.pos());
	if (!qdConditionalObject::save_data(fh)) return false;

	fh.writeFloatLE(_cur_time);

	int idx = -1;
	if (_prev_state && owner())
		idx = static_cast<qdGameObjectAnimated *>(owner())->get_state_index(_prev_state);
	fh.writeSint32LE(idx);

	if (is_active()) {
		fh.writeByte(1);

		if (const qdSound *snd = sound()) {
			if (!snd->is_stopped(&_sound_handle)) {
				float pos = 0.0;
				fh.writeByte(1);
				fh.writeFloatLE(pos);
			} else
				fh.writeByte(0);
		} else
			fh.writeByte(0);

		fh.writeByte(_is_sound_started);

		if (!_coords_animation.is_empty()) {
			if (!_coords_animation.save_data(fh))
				return false;
		}
	} else
		fh.writeByte(0);

	debugC(3, kDebugSave, "  qdGameObjectState::save_data after: %ld", fh.pos());
	return true;
}

bool qdGameObjectState::need_sound_restart() const {
	if (sndDispatcher *p = sndDispatcher::get_dispatcher()) {
		if (p->sound_status(&_sound_handle) != sndSound::SOUND_PLAYING)
			return true;
	}

	return false;
}

const char *qdGameObjectState::full_text() const {
	return qdTextDB::instance().getText(_text_ID.c_str());
}

const char *qdGameObjectState::short_text() const {
	return qdTextDB::instance().getText(_short_text_ID.c_str());
}

/* ------------------------ qdGameObjectStateStatic ------------------------- */

qdGameObjectStateStatic::qdGameObjectStateStatic() : qdGameObjectState(qdGameObjectState::STATE_STATIC) {
	_animation_info.set_owner(this);
}

qdGameObjectStateStatic::qdGameObjectStateStatic(const qdGameObjectStateStatic &st) : qdGameObjectState(st),
	_animation_info(st._animation_info) {
	_animation_info.set_owner(this);
}

qdGameObjectStateStatic::~qdGameObjectStateStatic() {
}

qdGameObjectStateStatic &qdGameObjectStateStatic::operator = (const qdGameObjectStateStatic &st) {
	if (this == &st) return *this;
	qdGameObjectState::operator = (st);

	_animation_info = st._animation_info;
	_animation_info.set_owner(this);

	return *this;
}

qdGameObjectState &qdGameObjectStateStatic::operator = (const qdGameObjectState &st) {
	if (this == &st) return *this;

	assert(st.state_type() == STATE_STATIC);
	qdGameObjectState::operator = (st);

//	*static_cast<qdGameObjectState*>(this) = st;
	const qdGameObjectStateStatic *ss =
	    static_cast<const qdGameObjectStateStatic *>(&st);

	_animation_info = ss->_animation_info;
	_animation_info.set_owner(this);

	return *this;
}

bool qdGameObjectStateStatic::is_state_empty() const {
	if (qdGameObjectState::is_state_empty()) {
		return (!_animation_info.animation_name() || strlen(_animation_info.animation_name()));
	}
	return false;
}

bool qdGameObjectStateStatic::load_script(const xml::tag *p) {
	load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_ANIMATION_INFO:
			_animation_info.load_script(&*it);
			break;
		}
	}

	return true;
}

bool qdGameObjectStateStatic::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<object_state_static");

	save_script_body(fh, indent);

	if (_animation_info.animation_name()) {
		_animation_info.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</object_state_static>\r\n");

	return true;
}

bool qdGameObjectStateStatic::register_resources() {
	qdGameObjectState::register_resources();

	if (qdAnimation *p = animation()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
			dp->register_resource(p, this);
		}
	}

	return true;
}

bool qdGameObjectStateStatic::unregister_resources() {
	qdGameObjectState::unregister_resources();

	if (qdAnimation *p = animation()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
			dp->unregister_resource(p, this);
		}
	}

	return true;
}

bool qdGameObjectStateStatic::load_resources() {
	qdGameObjectState::load_resources();

	if (qdAnimation *p = animation()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->load_resource(p, this);
	}

	return true;
}

bool qdGameObjectStateStatic::free_resources() {
	qdGameObjectState::free_resources();

	if (qdAnimation *p = animation()) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->release_resource(p, this);
	}

	return true;
}

bool qdGameObjectStateStatic::auto_bound() {
	qdAnimation *ap = animation();
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

	return false;
}

/* -------------------------- qdGameObjectStateWalk ------------------------- */

qdGameObjectStateWalk::qdGameObjectStateWalk() : qdGameObjectState(qdGameObjectState::STATE_WALK),
	_direction_angle(-1.0f),
	_acceleration(0.0f),
	_max_speed(0.0f),
	_movement_type(MOVEMENT_EIGHT_DIRS) {
	_animation_set_info.set_owner(this);
	coords_animation()->set_type(qdCoordsAnimation::CA_WALK);
}

qdGameObjectStateWalk::qdGameObjectStateWalk(const qdGameObjectStateWalk &st) : qdGameObjectState(st),
	_animation_set_info(st._animation_set_info),
	_direction_angle(st._direction_angle),
	_acceleration(st._acceleration),
	_max_speed(st._max_speed),
	_movement_type(st._movement_type),
	_center_offsets(st._center_offsets),
	_static_center_offsets(st._static_center_offsets),
	_start_center_offsets(st._start_center_offsets),
	_stop_center_offsets(st._stop_center_offsets) {
}

qdGameObjectStateWalk::~qdGameObjectStateWalk() {
}

qdGameObjectStateWalk &qdGameObjectStateWalk::operator = (const qdGameObjectStateWalk &st) {
	if (this == &st) return *this;
	qdGameObjectState::operator = (st);

	_animation_set_info = st._animation_set_info;
	_direction_angle = st._direction_angle;

	_center_offsets = st._center_offsets;
	_static_center_offsets = st._static_center_offsets;
	_start_center_offsets = st._start_center_offsets;
	_stop_center_offsets = st._stop_center_offsets;

	_acceleration = st._acceleration;
	_max_speed = st._max_speed;

	_movement_type = st._movement_type;

	return *this;
}

qdGameObjectState &qdGameObjectStateWalk::operator = (const qdGameObjectState &st) {
	if (this == &st) return *this;

	assert(st.state_type() == STATE_WALK);
	qdGameObjectState::operator = (st);

	const qdGameObjectStateWalk *sw = static_cast<const qdGameObjectStateWalk *>(&st);

	_animation_set_info = sw->_animation_set_info;
	_direction_angle = sw->_direction_angle;

	_center_offsets = sw->_center_offsets;
	_static_center_offsets = sw->_static_center_offsets;
	_start_center_offsets = sw->_start_center_offsets;
	_stop_center_offsets = sw->_stop_center_offsets;

	_acceleration = sw->_acceleration;
	_max_speed = sw->_max_speed;
	_movement_type = sw->_movement_type;

	return *this;
}

float qdGameObjectStateWalk::adjust_direction_angle(float angle) const {
	angle = cycleAngle(angle);

	switch (_movement_type) {
	case MOVEMENT_LEFT:
		angle = M_PI;
		break;
	case MOVEMENT_UP:
		angle = M_PI / 2.0f;
		break;
	case MOVEMENT_RIGHT:
		angle = 0.0f;
		break;
	case MOVEMENT_DOWN:
		angle = M_PI / 2.0f * 3.0f;
		break;
	case MOVEMENT_UP_LEFT:
		angle = M_PI / 4.0f * 3.0f;
		break;
	case MOVEMENT_UP_RIGHT:
		angle = M_PI / 4.0f * 1.0f;
		break;
	case MOVEMENT_DOWN_RIGHT:
		angle = M_PI / 4.0f * 7.0f;
		break;
	case MOVEMENT_DOWN_LEFT:
		angle = M_PI / 4.0f * 5.0f;
		break;
	case MOVEMENT_HORIZONTAL:
		angle = (fabs(getDeltaAngle(0.0f, angle)) < fabs(getDeltaAngle(M_PI, angle))) ? 0.0f : M_PI;
		break;
	case MOVEMENT_VERTICAL:
		angle = (fabs(getDeltaAngle(M_PI / 2.0f, angle)) < fabs(getDeltaAngle(M_PI / 2.0f * 3.0f, angle))) ? M_PI / 2.0f : M_PI / 2.0f * 3.0f;
		break;
	case MOVEMENT_FOUR_DIRS: {
		float dist0 = fabs(getDeltaAngle(0.0f, angle));
		float angle0 = 0.0f;

		for (int i = 1; i < 4; i++) {
			float angle1 = float(i) * M_PI / 2.0f;
			float dist1 = fabs(getDeltaAngle(angle1, angle));
			if (dist1 < dist0) {
				dist0 = dist1;
				angle0 = angle1;
			}
		}

		angle = angle0;
	}
	break;
	case MOVEMENT_EIGHT_DIRS: {
		float dist0 = fabs(getDeltaAngle(0.0f, angle));
		float angle0 = 0.0f;

		for (int i = 1; i < 8; i++) {
			float angle1 = float(i) * M_PI / 4.0f;
			float dist1 = fabs(getDeltaAngle(angle1, angle));
			if (dist1 < dist0) {
				dist0 = dist1;
				angle0 = angle1;
			}
		}

		angle = angle0;
	}
	break;
	default:
		if (qdAnimationSet *p = animation_set())
			angle = p->adjust_angle(angle);
		break;
	}

	return angle;
}

bool qdGameObjectStateWalk::is_state_empty() const {
	if (qdGameObjectState::is_state_empty())
		return (!_animation_set_info.name() || strlen(_animation_set_info.name()));

	return false;
}

bool qdGameObjectStateWalk::update_sound_frequency(float direction_angle) const {
	float coeff = 1.0f;
	if (qdAnimationSet *set = animation_set())
		coeff *= set->walk_sound_frequency(direction_angle);

	return set_sound_frequency(coeff * walk_sound_frequency(direction_angle));
}

qdAnimationSet *qdGameObjectStateWalk::animation_set() const {
	if (_animation_set_info.name()) {
		if (qdGameScene * p = static_cast<qdGameScene * >(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdAnimationSet * set = p->get_animation_set(_animation_set_info.name()))
				return set;
		}

		if (qdGameDispatcher * p = qd_get_game_dispatcher())
			return p->get_animation_set(_animation_set_info.name());
	}

	return 0;
}

qdAnimation *qdGameObjectStateWalk::animation(float direction_angle) {
	if (qdAnimationInfo * inf = animation_info(direction_angle))
		return inf->animation();

	return 0;
}

qdAnimation *qdGameObjectStateWalk::static_animation(float direction_angle) {
	if (qdAnimationInfo * inf = static_animation_info(direction_angle))
		return inf->animation();

	return 0;
}

qdAnimationInfo *qdGameObjectStateWalk::animation_info(float direction_angle) {
	if (qdAnimationSet * set = animation_set())
		return set->get_animation_info(direction_angle);

	return 0;
}

qdAnimationInfo *qdGameObjectStateWalk::static_animation_info(float direction_angle) {
	if (qdAnimationSet * set = animation_set())
		return set->get_static_animation_info(direction_angle);

	return 0;
}

const Vect2i &qdGameObjectStateWalk::center_offset(int direction_index, OffsetType offset_type) const {
	const Std::vector<Vect2i> *vect = &_center_offsets;

	switch (offset_type) {
	case OFFSET_STATIC:
		vect = &_static_center_offsets;
		break;
	case OFFSET_WALK:
		vect = &_center_offsets;
		break;
	case OFFSET_START:
		vect = &_start_center_offsets;
		break;
	case OFFSET_END:
		vect = &_stop_center_offsets;
		break;
	}

	if (direction_index < 0 || direction_index >= (int)vect->size()) {
		static Vect2i v(0, 0);
		return v;
	} else
		return (*vect)[direction_index];
}

const Vect2i &qdGameObjectStateWalk::center_offset(float direction_angle, OffsetType offset_type) const {
	int index = 0;
	if (qdAnimationSet * p = animation_set())
		index = p->get_angle_index(direction_angle);

	return center_offset(index, offset_type);
}

void qdGameObjectStateWalk::set_center_offset(int direction_index, const Vect2i &offs, OffsetType offset_type) {
	assert(direction_index >= 0);

	Std::vector<Vect2i> *vect = &_center_offsets;

	switch (offset_type) {
	case OFFSET_STATIC:
		vect = &_static_center_offsets;
		break;
	case OFFSET_WALK:
		vect = &_center_offsets;
		break;
	case OFFSET_START:
		vect = &_start_center_offsets;
		break;
	case OFFSET_END:
		vect = &_stop_center_offsets;
		break;
	}

	if (direction_index >= (int)vect->size())
		vect->resize(direction_index + 1, Vect2i(0, 0));

	(*vect)[direction_index] = offs;
}

float qdGameObjectStateWalk::walk_sound_frequency(int direction_index) const {
	if (direction_index < 0 || direction_index >= (int)_walk_sound_frequency.size())
		return 1;
	else
		return _walk_sound_frequency[direction_index];
}

float qdGameObjectStateWalk::walk_sound_frequency(float direction_angle) const {
	int index = 0;
	if (qdAnimationSet * p = animation_set())
		index = p->get_angle_index(direction_angle);

	return walk_sound_frequency(index);
}

void qdGameObjectStateWalk::set_walk_sound_frequency(int direction_index, float freq) {
	assert(direction_index >= 0);

	if (direction_index >= (int)_walk_sound_frequency.size())
		_walk_sound_frequency.resize(direction_index + 1, 1);

	_walk_sound_frequency[direction_index] = freq;
}

bool qdGameObjectStateWalk::load_script(const xml::tag *p) {
	load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_ANIMATION_SET:
			_animation_set_info.set_name(it->data());
			break;
		case QDSCR_OBJECT_DIRECTION:
			xml::tag_buffer(*it) > _direction_angle;
			break;
		case QDSCR_STATE_CENTER_OFFSETS: {
			xml::tag_buffer buf(*it);
			_center_offsets.resize(it->data_size() / 2);
			for (int i = 0; i < it->data_size() / 2; i++)
				buf > _center_offsets[i].x > _center_offsets[i].y;
		}
		break;
		case QDSCR_STATE_STATIC_CENTER_OFFSETS: {
			xml::tag_buffer buf(*it);
			_static_center_offsets.resize(it->data_size() / 2);
			for (int i = 0; i < it->data_size() / 2; i++)
				buf > _static_center_offsets[i].x > _static_center_offsets[i].y;
		}
		break;
		case QDSCR_STATE_START_CENTER_OFFSETS: {
			xml::tag_buffer buf(*it);
			_start_center_offsets.resize(it->data_size() / 2);
			for (int i = 0; i < it->data_size() / 2; i++)
				buf > _start_center_offsets[i].x > _start_center_offsets[i].y;
		}
		break;
		case QDSCR_STATE_STOP_CENTER_OFFSETS: {
			xml::tag_buffer buf(*it);
			_stop_center_offsets.resize(it->data_size() / 2);
			for (int i = 0; i < it->data_size() / 2; i++)
				buf > _stop_center_offsets[i].x > _stop_center_offsets[i].y;
		}
		break;
		case QDSCR_OBJECT_STATE_WALK_SOUND_FREQUENCY: {
			xml::tag_buffer buf(*it);
			_walk_sound_frequency.resize(it->data_size());
			for (int i = 0; i < it->data_size(); i++)
				buf > _walk_sound_frequency[i];
		}
		break;
		case QDSCR_OBJECT_STATE_ACCELERATION:
			xml::tag_buffer(*it) > _acceleration > _max_speed;
			break;
		case QDSCR_PERSONAGE_MOVEMENT_TYPE:
			_movement_type = movement_type_t(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}
bool qdGameObjectStateWalk::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<object_state_walk");
	fh.writeString(Common::String::format(" movement=\"%d\"", _movement_type));

	save_script_body(fh, indent);

	if (_animation_set_info.name()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<animation_set>%s</animation_set>\r\n", qdscr_XML_string(_animation_set_info.name())));
	}

	if (_direction_angle > 0.0f) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<object_direction>%f</object_direction>\r\n", _direction_angle));
	}

	if (_acceleration > FLT_EPS || _max_speed > FLT_EPS) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<acceleration>%f %f</acceleration>\r\n", _acceleration, _max_speed));
	}

	if (_center_offsets.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<center_offsets>%u", _center_offsets.size() * 2));
		for (uint i = 0; i < _center_offsets.size(); i++) {
			fh.writeString(Common::String::format(" %d %d", _center_offsets[i].x, _center_offsets[i].y));
		}
		fh.writeString("</center_offsets>\r\n");
	}

	if (_static_center_offsets.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<static_center_offsets>%u", _static_center_offsets.size() * 2));
		for (uint i = 0; i < _static_center_offsets.size(); i++) {
			fh.writeString(Common::String::format(" %d %d", _static_center_offsets[i].x, _static_center_offsets[i].y));
		}
		fh.writeString("</static_center_offsets>\r\n");
	}

	if (_start_center_offsets.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<start_center_offsets>%u", _start_center_offsets.size() * 2));
		for (uint i = 0; i < _start_center_offsets.size(); i++){
			fh.writeString(Common::String::format(" %d %d", _start_center_offsets[i].x, _start_center_offsets[i].y));
		}
		fh.writeString("</start_center_offsets>\r\n");
	}

	if (_stop_center_offsets.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<stop_center_offsets>%u", _stop_center_offsets.size() * 2));
		for (uint i = 0; i < _stop_center_offsets.size(); i++) {
			fh.writeString(Common::String::format(" %d %d", _stop_center_offsets[i].x, _stop_center_offsets[i].y));
		}
		fh.writeString("</stop_center_offsets>\r\n");
	}

	if (_walk_sound_frequency.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<walk_sound_frequency>%u", _walk_sound_frequency.size()));
		for (uint i = 0; i < _walk_sound_frequency.size(); i++) {
			fh.writeString(Common::String::format(" %f", _walk_sound_frequency[i]));
		}
		fh.writeString("</walk_sound_frequency>\r\n");
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</object_state_walk>\r\n");

	return true;
}

bool qdGameObjectStateWalk::register_resources() {
	qdGameObjectState::register_resources();

	if (qdAnimationSet * p = animation_set())
		p->register_resources(this);

	return true;
}

bool qdGameObjectStateWalk::unregister_resources() {
	qdGameObjectState::unregister_resources();

	if (qdAnimationSet * p = animation_set())
		p->unregister_resources(this);

	return true;
}

bool qdGameObjectStateWalk::load_resources() {
	qdGameObjectState::load_resources();

	if (qdAnimationSet * p = animation_set())
		p->load_animations(this);

	return true;
}

bool qdGameObjectStateWalk::free_resources() {
	qdGameObjectState::free_resources();

	if (qdAnimationSet * p = animation_set())
		p->free_animations(this);

	return true;
}

bool qdGameObjectStateWalk::auto_bound() {
	qdAnimation *ap = static_animation(3.0f / 2.0f * M_PI);
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

	return false;
}

bool qdGameObjectStateWalk::need_sound_restart() const {
	if (owner() && owner()->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
		if (!owner()->check_flag(QD_OBJ_MOVING_FLAG))
			return false;
	}

	return qdGameObjectState::need_sound_restart();
}

/* -------------------------- qdGameObjectStateMask ------------------------- */

qdGameObjectStateMask::qdGameObjectStateMask() : qdGameObjectState(qdGameObjectState::STATE_MASK), qdContour(qdContour::CONTOUR_POLYGON),
	_parent(NULL) {

}

qdGameObjectStateMask::qdGameObjectStateMask(const qdGameObjectStateMask &st) : qdGameObjectState(st), qdContour(st),
	_parent(st._parent),
	_parent_name(st._parent_name) {
}

qdGameObjectStateMask::~qdGameObjectStateMask() {
}

qdGameObjectStateMask &qdGameObjectStateMask::operator = (const qdGameObjectStateMask &st) {
	if (this == &st) return *this;
	qdGameObjectState::operator = (*static_cast<const qdGameObjectState *>(&st));
	qdContour::operator = (*static_cast<const qdContour *>(&st));

	_parent_name = st._parent_name;
	_parent = st._parent;
	return *this;
}

qdGameObjectState &qdGameObjectStateMask::operator = (const qdGameObjectState &st) {
	if (this == &st) return *this;

	assert(st.state_type() == STATE_MASK);
	qdGameObjectState::operator = (st);

	*static_cast<qdContour *>(this) =
	    *static_cast<const qdContour *>(static_cast<const qdGameObjectStateMask *>(&st));
	const qdGameObjectStateMask *sm =
	    static_cast<const qdGameObjectStateMask *>(&st);

	_parent_name = sm->_parent_name;
	_parent = sm->_parent;

	return *this;
}

bool qdGameObjectStateMask::is_state_empty() const {
	if (qdGameObjectState::is_state_empty()) {
		return _parent_name.empty();
	}
	return false;
}

bool qdGameObjectStateMask::load_script(const xml::tag *p) {
	load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_OBJECT_STATE_MASK_PARENT:
			set_parent_name(it->data());
			break;
		case QDSCR_CONTOUR_RECTANGLE:
			set_contour_type(qdContour::CONTOUR_RECTANGLE);
			qdContour::load_script(&*it);
			break;
		case QDSCR_CONTOUR_CIRCLE:
			set_contour_type(qdContour::CONTOUR_CIRCLE);
			qdContour::load_script(&*it);
			break;
		case QDSCR_CONTOUR_POLYGON:
		case QDSCR_OBJECT_STATE_MASK_CONTOUR:
			set_contour_type(qdContour::CONTOUR_POLYGON);
			qdContour::load_script(&*it);
			break;
		}
	}

	return true;
}

bool qdGameObjectStateMask::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<object_state_mask");

	save_script_body(fh, indent);

	if (!_parent_name.empty()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<state_mask_parent>%s</state_mask_parent>\r\n", qdscr_XML_string(_parent_name.c_str())));
	}

	if (contour_size()) {
		qdContour::save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</object_state_mask>\r\n");

	return true;
}

bool qdGameObjectStateMask::hit(int x, int y) const {
	const qdGameObject *p = parent();
	if (!p) return false;

	Vect2s scr_pos = p->screen_pos();
	x -= scr_pos.x;
	y -= scr_pos.y;

	return is_inside(Vect2s(x, y));
}

qdGameObject *qdGameObjectStateMask::parent() {
	if (_parent) return _parent;

	if (_parent_name.empty() || !owner()) return 0;

	qdNamedObject *p = owner()->owner();
	if (!p || p->named_object_type() != QD_NAMED_OBJECT_SCENE) return 0;

	_parent = static_cast<qdGameScene *>(p)->get_object(_parent_name.c_str());
	return _parent;
}

const qdGameObject *qdGameObjectStateMask::parent() const {
	if (_parent) return _parent;

	if (_parent_name.empty() || !owner()) return 0;

	qdNamedObject *p = owner()->owner();
	if (!p || p->named_object_type() != QD_NAMED_OBJECT_SCENE) return 0;

	return static_cast<qdGameScene *>(p)->get_object(_parent_name.c_str());
}

bool qdGameObjectStateMask::draw_mask(uint32 color) const {
	const qdGameObject *p = parent();
	if (!p) return false;

	Vect2s pos = p->screen_pos() + mask_pos();
	pos.x -= mask_size().x / 2;
	pos.y -= mask_size().y / 2;

	for (int y = 0; y < mask_size().y; y++) {
		for (int x = 0; x < mask_size().x; x++) {
			if (hit(pos.x + x, pos.y + y)) {
				grDispatcher::instance()->setPixel(pos.x + x, pos.y + y, color);
			}
		}
	}

	return true;
}

bool qdGameObjectStateMask::load_resources() {
	qdGameObjectState::load_resources();
	return true;
}
} // namespace QDEngine
