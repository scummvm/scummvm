/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <algorithm>

#include "gr_dispatcher.h"
#include "snd_dispatcher.h"
#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_textdb.h"
#include "qd_sound.h"
#include "qd_animation_set.h"
#include "qd_game_object_state.h"
#include "qd_game_object_animated.h"
#include "qd_game_scene.h"
#include "qd_resource_dispatcher.h"
#include "qd_game_dispatcher.h"

#ifdef _QUEST_EDITOR
#include "qd_game_object_mouse.h"
#endif // _QUEST_EDITOR
/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const Vect3f qdGameObjectState::DEFAULT_BOUND(3.f, 3.f, 3.f);

qdScreenTransform qdScreenTransform::ID;

bool qdScreenTransform::operator == (const qdScreenTransform& trans) const
{
	return fabs(angle_ - trans.angle_) < FLT_EPS &&
		fabs(scale_.x - trans.scale_.x) < FLT_EPS && fabs(scale_.y - trans.scale_.y) < FLT_EPS;
}

bool qdScreenTransform::change(float dt, const qdScreenTransform& target_trans, const qdScreenTransform& speed)
{
	qdScreenTransform delta(getDeltaAngle(target_trans.angle(), angle()),
		target_trans.scale() - scale());

	qdScreenTransform delta_max = speed * dt;
	delta.angle_ = clamp(delta.angle_, -delta_max.angle_, delta_max.angle_);
	delta.scale_.x = clamp(delta.scale_.x, -delta_max.scale_.x, delta_max.scale_.x);
	delta.scale_.y = clamp(delta.scale_.y, -delta_max.scale_.y, delta_max.scale_.y);

	*this += delta;

	return (*this == target_trans);
}

XStream& operator < (XStream& fh, const qdScreenTransform& trans)
{
	fh < trans.angle_ < trans.scale_;
	return fh;
}

XStream& operator > (XStream& fh, qdScreenTransform& trans)
{
	fh > trans.angle_ > trans.scale_;
	return fh;
}

qdGameObjectState::qdGameObjectState(qdGameObjectState::StateType tp) : center_offset_(0,0),
	state_type_(tp),
	prev_state_(0),
	mouse_cursor_id_(CURSOR_UNASSIGNED),
	work_time_(0.0f),
	cur_time_(0.0f),
	reference_count_(0),
	bound_(DEFAULT_BOUND),
	activation_delay_(0.0f),
	activation_timer_(0.0f),
	is_sound_started_(false),
	is_text_shown_(false),
	radius_(0.0f),
	sound_delay_(0.f),
	text_delay_(0.f),
	rnd_move_radius_(0.0f),
	rnd_move_speed_(0.0f),
	autosave_slot_(0),
	fade_time_(0.1f),
	shadow_color_(0),
	shadow_alpha_(QD_NO_SHADOW_ALPHA)
{ 
	coords_animation_.set_owner(this);
	sound_handle_.set_owner(this);
}

qdGameObjectState::qdGameObjectState(const qdGameObjectState& st) : qdConditionalObject(st),
	center_offset_(st.center_offset_),
	state_type_(st.state_type_),
	mouse_cursor_id_(st.mouse_cursor_id_),
	work_time_(st.work_time_),
	cur_time_(st.cur_time_),
	sound_info_(st.sound_info_),
	coords_animation_(st.coords_animation_),
	prev_state_(0),
	text_ID_(st.text_ID_),
	short_text_ID_(st.short_text_ID_),
	reference_count_(st.reference_count_),
	bound_(st.bound_),
	activation_delay_(st.activation_delay_),
	activation_timer_(st.activation_timer_),
	radius_(st.radius_),
	camera_mode_(st.camera_mode_),
	is_sound_started_(st.is_sound_started_),
	is_text_shown_(false),
	text_delay_(st.text_delay_),
	sound_delay_(st.sound_delay_),
	rnd_move_radius_(st.rnd_move_radius_),
	rnd_move_speed_(st.rnd_move_speed_),
	autosave_slot_(st.autosave_slot_),
	text_format_(st.text_format_),
	transform_(st.transform_),
	transform_speed_(st.transform_speed_),
	fade_time_(st.fade_time_),
	shadow_color_(st.shadow_color_),
	shadow_alpha_(st.shadow_alpha_)
#ifdef _QUEST_EDITOR
	, cursor_name_(st.cursor_name_)
#endif // _QUEST_EDITOR
{
	coords_animation_.set_owner(this);
	sound_handle_.set_owner(this);
}

qdGameObjectState::~qdGameObjectState()
{
}

bool qdGameObjectState::is_state_empty() const
{
	return (!sound_info_.name()||!strlen(sound_info_.name()));
}

qdGameObjectState& qdGameObjectState::operator = (const qdGameObjectState& st)
{
	if(this == &st) return *this;

	*static_cast<qdConditionalObject*>(this) = st;

	center_offset_ = st.center_offset_;
	state_type_ = st.state_type_;
	mouse_cursor_id_ = st.mouse_cursor_id_;
	sound_info_ = st.sound_info_;
	text_delay_ = st.text_delay_;
	sound_delay_ = st.sound_delay_;

	prev_state_ = 0;

	coords_animation_ = st.coords_animation_;

	coords_animation_.set_owner(this);
	sound_handle_.set_owner(this);

	text_ID_ = st.text_ID_;
	short_text_ID_ = st.short_text_ID_;

	work_time_ = st.work_time_;
	cur_time_ = st.cur_time_;

	reference_count_ = st.reference_count_;

	activation_delay_ = st.activation_delay_;
	activation_timer_ = st.activation_timer_;

	bound_ = st.bound_;
	radius_ = st.radius_;
	camera_mode_ = st.camera_mode_;

	rnd_move_radius_ = st.rnd_move_radius_;
	rnd_move_speed_ = st.rnd_move_speed_;

	autosave_slot_ = st.autosave_slot_;
	fade_time_ = st.fade_time_;

	shadow_color_ = st.shadow_color_;
	shadow_alpha_ = st.shadow_alpha_;

	transform_ = st.transform_;
	transform_speed_ = st.transform_speed_;

	text_format_ = st.text_format_;

	is_sound_started_ = st.is_sound_started_;
	is_text_shown_ = st.is_text_shown_;

#ifdef _QUEST_EDITOR
	cursor_name_ = st.cursor_name_;
#endif // _QUEST_EDITOR

	return *this;
}

bool qdGameObjectState::register_resources()
{
	if(qdSound* p = sound()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> register_resource(p,this);
	}

	return true;
}

bool qdGameObjectState::unregister_resources()
{
	if(qdSound* p = sound()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> unregister_resource(p,this);
	}

	return true;
}

bool qdGameObjectState::load_resources()
{
	if(qdSound* p = sound()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> load_resource(p,this);
	}

	return true;
}

bool qdGameObjectState::free_resources()
{
	if(qdSound* p = sound()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> release_resource(p,this);
	}

	return true;
}

bool qdGameObjectState::is_active() const
{
	if(owner() && static_cast<qdGameObjectAnimated*>(owner()) -> get_cur_state() == this)
		return true;

	return false;
}

bool qdGameObjectState::is_default() const
{
	if(owner() && static_cast<qdGameObjectAnimated*>(owner()) -> get_default_state() == this)
		return true;

	return false;
}

qdConditionalObject::trigger_start_mode qdGameObjectState::trigger_start()
{
	if(!owner()) return qdConditionalObject::TRIGGER_START_FAILED;

	qdGameObjectAnimated* op = static_cast<qdGameObjectAnimated*>(owner());

	if(!op -> check_flag(QD_OBJ_STATE_CHANGE_FLAG) && op -> can_change_state(this)){
		op -> set_state(this);
		op -> set_flag(QD_OBJ_STATE_CHANGE_FLAG | QD_OBJ_IS_IN_TRIGGER_FLAG);

		switch(op -> state_status(this)){
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

bool qdGameObjectState::trigger_can_start() const
{
	if(!qdConditionalObject::trigger_can_start()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			if(qdGameObjectAnimated* op = static_cast<qdGameObjectAnimated*>(owner())){
				if(dp -> is_on_mouse(op) || dp -> is_in_inventory(op))
					return true;
			}
		}

		return false;
	}
	else
		return true;
}

bool qdGameObjectState::load_script_body(const xml::tag* p)
{
	load_conditions_script(p);

	Vect2s vs;
	Vect3f vf;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_FLAG:
				set_flag(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_OBJECT_STATE_CURSOR_ID:
				set_mouse_cursor_ID(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_CAMERA_MODE:
				camera_mode_.load_script(&*it);
				break;
			case QDSCR_LENGTH:
				set_work_time(xml::tag_buffer(*it).get_float());
				break;
			case QDSCR_NAME:
				set_name(it -> data());
				break;
			case QDSCR_COORDS_ANIMATION:
				coords_animation_.load_script(&*it);
				break;
			case QDSCR_SOUND:
				set_sound_name(it -> data());
				if(const xml::tag* tg = it -> search_subtag(QDSCR_FLAG))
					sound_info_.set_flag(xml::tag_buffer(*tg).get_int());
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
				set_short_text_ID(it -> data());
				break;
			case QDSCR_TEXT:
				set_full_text_ID(it -> data());
				break;
			case QDSCR_OBJECT_STATE_SOUND_DELAY:
				xml::tag_buffer(*it) > sound_delay_;
				break;
			case QDSCR_OBJECT_STATE_TEXT_DELAY:
				xml::tag_buffer(*it) > text_delay_;
				break;
			case QDSCR_OBJECT_STATE_RND_MOVE:
				xml::tag_buffer(*it) > rnd_move_radius_ > rnd_move_speed_;
				break;
			case QDSCR_TEXT_COLOR:
				text_format_.set_color(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_TEXT_HOVER_COLOR:
				text_format_.set_hover_color(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_TEXT_ALIGN:
				text_format_.set_arrangement(qdScreenTextFormat::arrangement_t(xml::tag_buffer(*it).get_int()));
				break;
			case QDSCR_SCREEN_TEXT_FORMAT:
				text_format_.load_script(&*it);
				break;
			case QDSCR_SCENE_SAVE_SLOT:
				xml::tag_buffer(*it) > autosave_slot_;
				break;
			case QDSCR_FADE_TIME:
				xml::tag_buffer(*it) > fade_time_;
				break;
			case QDSCR_SCREEN_TRANSFORM: {
					float angle, angle_sp;
					Vect2f scale, scale_sp;
					xml::tag_buffer(*it) > angle > angle_sp > scale.x > scale.y > scale_sp.x > scale_sp.y;

					transform_.set_angle(G2R(angle));
					transform_.set_scale(scale);

					transform_speed_.set_angle(G2R(angle_sp));
					transform_speed_.set_scale(scale_sp);
				}
				break;
		case QDSCR_GRID_ZONE_SHADOW_COLOR:
			xml::tag_buffer(*it) > shadow_color_;
			break;
		case QDSCR_GRID_ZONE_SHADOW_ALPHA:
			xml::tag_buffer(*it) > shadow_alpha_;
			break;
		}
	}

#ifdef _QUEST_EDITOR
	if (mouse_cursor_ID() != CURSOR_UNASSIGNED)
	{
		qdGameObjectMouse const& mouseObject = *qd_get_game_dispatcher()->mouse_object();
		if (mouse_cursor_ID() < mouseObject.state_vector().size())
			set_cursor_name(mouseObject.state_vector()[mouse_cursor_ID()]->name());
	}
#endif // _QUEST_EDITOR

	drop_flag(QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
	return true;
}

bool qdGameObjectState::save_script_body(XStream& fh,int indent) const
{
	if(name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	if(mouse_cursor_id_ != CURSOR_UNASSIGNED)
		fh < " cursor_id=\"" <= mouse_cursor_id_ < "\"";

	fh < " save_slot=\"" <= autosave_slot_ < "\"";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(work_time_ > 0.001f)
		fh < " length=\"" <= work_time_ < "\"";

	if(activation_delay_ > 0.001f)
		fh < " state_activation_delay=\"" <= activation_delay_ < "\"";

	if(!short_text_ID_.empty())
		fh < " short_text=\"" < qdscr_XML_string(short_text_ID_.c_str()) < "\"";

	if(!text_ID_.empty())
		fh < " text=\"" < qdscr_XML_string(text_ID_.c_str()) < "\"";

	if(center_offset_.x || center_offset_.y)
		fh < " center_offset=\"" <= center_offset_.x < " " <= center_offset_.y < "\"";

	if(has_bound())
		fh < " bound=\"" <= bound_.x < " " <= bound_.y < " " <= bound_.z < "\"";

	if(has_sound_delay())
		fh < " sound_delay=\"" <= sound_delay_ < "\"";

	if(has_text_delay())
		fh < " text_delay=\"" <= text_delay_ < "\"";

	if(rnd_move_radius_ > FLT_EPS && rnd_move_speed_ > FLT_EPS)
		fh < " rnd_move=\"" <= rnd_move_radius_ < " " <= rnd_move_speed_ < "\"";

	if(fabs(fade_time_ - 0.1f) > FLT_EPS)
		fh < " fade_time=\"" <= fade_time_ < "\"";

	if(shadow_color_)
		fh < " shadow_color=\"" <= shadow_color_ < "\"";

	if(shadow_alpha_ != QD_NO_SHADOW_ALPHA)
		fh < " shadow_alpha=\"" <= shadow_alpha_ < "\"";

	fh < ">\r\n";

	if(!coords_animation_.is_empty())
		coords_animation_.save_script(fh,indent + 1);

	if(has_camera_mode())
		camera_mode_.save_script(fh,indent + 1);

	if(has_sound()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<sound";

		if(sound_info_.flags())
			fh < " flags=\"" <= sound_info_.flags() < "\"";

		fh < ">" < qdscr_XML_string(sound_info_.name()) < "</sound>\r\n";
	}

	if(has_transform()){
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "<screen_transform>";

		fh < " " <= R2G(transform_.angle()) < " " <= R2G(transform_speed_.angle());
		fh < " " <= transform_.scale().x < " " <= transform_.scale().y;
		fh < " " <= transform_speed_.scale().x < " " <= transform_speed_.scale().y;

		fh < "</screen_transform>\r\n";
	}

	text_format_.save_script(fh, indent + 1);

	save_conditions_script(fh,indent);

	return true;
}

bool qdGameObjectState::init()
{
	if(!qdConditionalObject::init()) return false;
	// Ищем по ссылке и инициализируем найденным стартовый объект координатной анимации
	qdNamedObject* nam_obj = qdGameDispatcher::get_dispatcher()->
		                       get_named_object(&coords_animation_.start_object_ref());
	coords_animation_.set_start_object(dynamic_cast<const qdGameObject*>(nam_obj));


#ifndef _QUEST_EDITOR
//	drop_flag(QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
#endif

	drop_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
	drop_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);
	drop_flag(QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
	drop_flag(QD_OBJ_STATE_FLAG_WAS_ACTIVATED);

	return true;
}

bool qdGameObjectState::check_conditions()
{
	if(has_conditions())
		return qdConditionalObject::check_conditions();

	return is_in_triggers();
}

float qdGameObjectState::work_time() const 
{ 
	if(check_flag(QD_OBJ_STATE_FLAG_SOUND_SYNC)){
		if(qdSound* p = sound())
			return p -> length() + sound_delay_;

		return 0.0f;
	}

	return work_time_; 
}

void qdGameObjectState::quant(float dt)
{
	if(is_active()){
		cur_time_ += dt;

		if(has_sound() && (state_type() != STATE_WALK || static_cast<qdGameObject*>(owner()) -> check_flag(QD_OBJ_MOVING_FLAG))){
			if(!is_sound_started_){
				if(!has_sound_delay() || cur_time_ >= sound_delay_){
					play_sound();
				}
			}
			else {
				if(check_sound_flag(qdSoundInfo::LOOP_SOUND_FLAG) && !is_sound_playing())
					play_sound();
			}
		}

#ifndef _QUEST_EDITOR
		if(!is_text_shown_ && has_text() && has_text_delay()){
			if(cur_time_ >= text_delay_){
				qdGameDispatcher::get_dispatcher() -> screen_texts_dispatcher().add_text(qdGameDispatcher::TEXT_SET_DIALOGS,qdScreenText(text(),text_format(),Vect2i(0,0),this));
				is_text_shown_ = true;
			}
		}
#endif
	}

	conditions_quant(dt);

	if(check_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER) && !check_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END)){
		activation_timer_ -= dt;
		if(activation_timer_ <= 0.0f){
			activation_timer_ = 0.0f;
			set_flag(QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);
		}
	}
}

qdSound* qdGameObjectState::sound() const
{
	if(sound_info_.name()){
		if(qdGameScene* p = static_cast<qdGameScene*>(owner(QD_NAMED_OBJECT_SCENE))){
			if(qdSound* snd = p -> get_sound(sound_info_.name()))
				return snd;
		}

		if(qdGameDispatcher* p = qd_get_game_dispatcher())
			return p -> get_sound(sound_info_.name());
	}

	return NULL;
}

bool qdGameObjectState::play_sound(float position)
{
	if(qdSound* p = sound()){
		p -> stop(sound_handle());
		is_sound_started_ = true;
		return p -> play(sound_handle(),check_sound_flag(qdSoundInfo::LOOP_SOUND_FLAG),position);
	}

	return false;
}

bool qdGameObjectState::set_sound_frequency(float frequency_coeff) const
{
	if(qdSound* p = sound())
		return p -> set_frequency(sound_handle(),frequency_coeff);

	return false;
}

bool qdGameObjectState::is_sound_finished() const
{
	if(sound_delay_ > 0.01f && cur_time_ <= sound_delay_) return false;

	if(qdSound* p = sound())
		return p -> is_stopped(sound_handle());

	return true;
}

bool qdGameObjectState::is_sound_playing() const
{
	if(qdSound* p = sound())
		return !p -> is_stopped(sound_handle());

	return false;
}

bool qdGameObjectState::stop_sound() const
{
	if(qdSound* p = sound()){
		return p -> stop(sound_handle());
	}

	return false;
}

void qdGameObjectState::set_bound(const Vect3f& b)
{
	bound_ = b;
	Vect3f b2 = b / 2.0f;
	radius_ = b2.norm();
}

bool qdGameObjectState::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdConditionalObject::load_data(fh,save_version)) return false;

	fh > cur_time_;

	int idx;
	fh > idx;

	if(idx != -1)
		prev_state_ = static_cast<qdGameObjectAnimated*>(owner()) -> get_state(idx);
	else
		prev_state_ = NULL;

	char cidx;
	fh > cidx;
	if(cidx){
		fh > cidx;
		if(cidx){
			float pos;
			fh > pos;

			if(qdSound* snd = sound()){
				if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
					dp -> load_resource(snd,this);
					play_sound(pos);
				}
			}

			is_sound_started_ = true;
		}

		fh > cidx;
		is_sound_started_ = (cidx) ? true : false;

		if(!coords_animation_.is_empty()){
			if(!coords_animation_.load_data(fh,save_version))
				return false;
		}
	}

	return true;
}

bool qdGameObjectState::save_data(qdSaveStream& fh) const
{
	if(!qdConditionalObject::save_data(fh)) return false;

	fh < cur_time_;

	int idx = -1;
	if(prev_state_ && owner())
		idx = static_cast<qdGameObjectAnimated*>(owner()) -> get_state_index(prev_state_);
	fh < idx;

	if(is_active()){
		fh < char(1);

		if(const qdSound* snd = sound()){
			if(!snd -> is_stopped(&sound_handle_)){
				float pos = snd -> position(&sound_handle_);
				fh < char(1) < pos;
			}
			else
				fh < char(0);
		}
		else
			fh < char(0);

		fh < char(is_sound_started_);

		if(!coords_animation_.is_empty()){
			if(!coords_animation_.save_data(fh))
				return false;
		}
	}
	else 
		fh < char(0);

	return true;
}

bool qdGameObjectState::need_sound_restart() const
{
	if(sndDispatcher* p = sndDispatcher::get_dispatcher()){
		if(p -> sound_status(&sound_handle_) != sndSound::SOUND_PLAYING)
			return true;
	}

	return false;
}

const char* qdGameObjectState::full_text() const
{
	return qdTextDB::instance().getText(text_ID_.c_str());
}

const char* qdGameObjectState::short_text() const
{
	return qdTextDB::instance().getText(short_text_ID_.c_str());
}

#ifdef _QUEST_EDITOR
//! Выдает имя курсора. Пустая строка, если установлен курсор по умолчанию
std::string const& qdGameObjectState::cursor_name() const{
	return cursor_name_;
}
//! Установка имени курсора
void qdGameObjectState::set_cursor_name(std::string const& cursor_name){
	cursor_name_ = cursor_name;
}
//! По имени курсора определяет его номер
void qdGameObjectState::update_cursor_id(qdGameDispatcher const& gameDispatcher)
{
	if (cursor_name_.empty()
		||mouse_cursor_ID() == CURSOR_UNASSIGNED)
		return;

	qdGameObjectMouse const& mouseObject = *gameDispatcher.mouse_object();
	qdGameObjectStateVector const& v = mouseObject.state_vector();
	qdGameObjectStateVector::const_iterator i = v.begin(), e = v.end();
	for(; i != e; ++i)
	{
		qdGameObjectState const& state = **i;
		if (cursor_name_ == state.name())
		{
			set_mouse_cursor_ID(std::distance(v.begin(), i));
			return;
		}
	}
	cursor_name_.clear();
	set_mouse_cursor_ID(CURSOR_UNASSIGNED);
}
#endif // _QUEST_EDITOR

/* ------------------------ qdGameObjectStateStatic ------------------------- */

qdGameObjectStateStatic::qdGameObjectStateStatic() : qdGameObjectState(qdGameObjectState::STATE_STATIC)
{ 
	animation_info_.set_owner(this);
}

qdGameObjectStateStatic::qdGameObjectStateStatic(const qdGameObjectStateStatic& st) : qdGameObjectState(st),
	animation_info_(st.animation_info_)
{
	animation_info_.set_owner(this);
}

qdGameObjectStateStatic::~qdGameObjectStateStatic()
{
}

qdGameObjectStateStatic& qdGameObjectStateStatic::operator = (const qdGameObjectStateStatic& st)
{
	if(this == &st) return *this;
	qdGameObjectState::operator = (st);

	animation_info_ = st.animation_info_;
	animation_info_.set_owner(this);

	return *this;
}

qdGameObjectState& qdGameObjectStateStatic::operator = (const qdGameObjectState& st)
{
	if(this == &st) return *this;

	assert(st.state_type() == STATE_STATIC);
	qdGameObjectState::operator = (st);

//	*static_cast<qdGameObjectState*>(this) = st;
	const qdGameObjectStateStatic* ss =
		static_cast<const qdGameObjectStateStatic*>(&st);

	animation_info_ = ss->animation_info_;
	animation_info_.set_owner(this);

	return *this;
}

bool qdGameObjectStateStatic::is_state_empty() const
{
	if (qdGameObjectState::is_state_empty()) {
		return (!animation_info_.animation_name()||strlen(animation_info_.animation_name()));
	}
	return false;
}

bool qdGameObjectStateStatic::load_script(const xml::tag* p)
{
	load_script_body(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_ANIMATION_INFO:
				animation_info_.load_script(&*it);
				break;
		}
	}

	return true;
}

bool qdGameObjectStateStatic::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<object_state_static";

	save_script_body(fh,indent);

	if(animation_info_.animation_name())
		animation_info_.save_script(fh,indent + 1);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</object_state_static>\r\n";

	return true;
}

bool qdGameObjectStateStatic::register_resources()
{
	qdGameObjectState::register_resources();

	if(qdAnimation* p = animation()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			dp -> register_resource(p,this);
		}
	}

	return true;
}

bool qdGameObjectStateStatic::unregister_resources()
{
	qdGameObjectState::unregister_resources();

	if(qdAnimation* p = animation()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			dp -> unregister_resource(p,this);
		}
	}

	return true;
}

bool qdGameObjectStateStatic::load_resources()
{
	qdGameObjectState::load_resources();

	if(qdAnimation* p = animation()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> load_resource(p,this);
	}

	return true;
}

bool qdGameObjectStateStatic::free_resources()
{
	qdGameObjectState::free_resources();

	if(qdAnimation* p = animation()){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> release_resource(p,this);
	}

	return true;
}

bool qdGameObjectStateStatic::auto_bound()
{
	qdAnimation* ap = animation();
	if(ap){
		bool res_flag = false;
		if(!ap -> is_resource_loaded()){
			ap -> load_resource();
			res_flag = true;
		}

		set_bound(Vect3f(ap -> picture_size_x(),ap -> picture_size_x(),ap -> picture_size_y()));

		if(res_flag)
			ap -> free_resource();

		return true;
	}

	return false;
}

/* -------------------------- qdGameObjectStateWalk ------------------------- */

qdGameObjectStateWalk::qdGameObjectStateWalk() : qdGameObjectState(qdGameObjectState::STATE_WALK),
	direction_angle_(-1.0f),
	acceleration_(0.0f),
	max_speed_(0.0f),
	movement_type_(MOVEMENT_EIGHT_DIRS)
{
	animation_set_info_.set_owner(this);
	coords_animation() -> set_type(qdCoordsAnimation::CA_WALK);
}

qdGameObjectStateWalk::qdGameObjectStateWalk(const qdGameObjectStateWalk& st) : qdGameObjectState(st),
	animation_set_info_(st.animation_set_info_),
	direction_angle_(st.direction_angle_),
	acceleration_(st.acceleration_),
	max_speed_(st.max_speed_),
	movement_type_(st.movement_type_),
	center_offsets_(st.center_offsets_),
	static_center_offsets_(st.static_center_offsets_),
	start_center_offsets_(st.start_center_offsets_),
	stop_center_offsets_(st.stop_center_offsets_)
{
}

qdGameObjectStateWalk::~qdGameObjectStateWalk()
{
}

qdGameObjectStateWalk& qdGameObjectStateWalk::operator = (const qdGameObjectStateWalk& st)
{
	if(this == &st) return *this;
	qdGameObjectState::operator = (st);

	animation_set_info_ = st.animation_set_info_;
	direction_angle_ = st.direction_angle_;

	center_offsets_ = st.center_offsets_;
	static_center_offsets_ = st.static_center_offsets_;
	start_center_offsets_ = st.start_center_offsets_;
	stop_center_offsets_ = st.stop_center_offsets_;

	acceleration_ = st.acceleration_;
	max_speed_ = st.max_speed_;

	movement_type_ = st.movement_type_;

	return *this;
}

qdGameObjectState& qdGameObjectStateWalk::operator = (const qdGameObjectState& st)
{
	if(this == &st) return *this;

	assert(st.state_type() == STATE_WALK);
	qdGameObjectState::operator = (st);

	const qdGameObjectStateWalk* sw = static_cast<const qdGameObjectStateWalk*>(&st);

	animation_set_info_ = sw -> animation_set_info_;
	direction_angle_ = sw -> direction_angle_;

	center_offsets_ = sw -> center_offsets_;
	static_center_offsets_ = sw -> static_center_offsets_;
	start_center_offsets_ = sw -> start_center_offsets_;
	stop_center_offsets_ = sw -> stop_center_offsets_;

	acceleration_ = sw -> acceleration_;
	max_speed_ = sw -> max_speed_;
	movement_type_ = sw -> movement_type_;

	return *this;
}

float qdGameObjectStateWalk::adjust_direction_angle(float angle) const
{
	angle = cycleAngle(angle);

	switch(movement_type_){
	case MOVEMENT_LEFT:
		angle = M_PI;
		break;
	case MOVEMENT_UP:
		angle = M_PI/2.0f;
		break;
	case MOVEMENT_RIGHT:
		angle = 0.0f;
		break;
	case MOVEMENT_DOWN:
		angle = M_PI/2.0f*3.0f;
		break;
	case MOVEMENT_UP_LEFT:
		angle = M_PI/4.0f*3.0f;
		break;
	case MOVEMENT_UP_RIGHT:
		angle = M_PI/4.0f*1.0f;
		break;
	case MOVEMENT_DOWN_RIGHT:
		angle = M_PI/4.0f*7.0f;
		break;
	case MOVEMENT_DOWN_LEFT:
		angle = M_PI/4.0f*5.0f;
		break;
	case MOVEMENT_HORIZONTAL:
		angle = (fabs(getDeltaAngle(0.0f,angle)) < fabs(getDeltaAngle(M_PI,angle))) ? 0.0f : M_PI;
		break;
	case MOVEMENT_VERTICAL:
		angle = (fabs(getDeltaAngle(M_PI/2.0f,angle)) < fabs(getDeltaAngle(M_PI/2.0f*3.0f,angle))) ? M_PI/2.0f : M_PI/2.0f*3.0f;
		break;
	case MOVEMENT_FOUR_DIRS: {
			float dist0 = fabs(getDeltaAngle(0.0f,angle));
			float angle0 = 0.0f;

			for(int i = 1; i < 4; i ++){
				float angle1 = float(i)*M_PI/2.0f;
				float dist1 = fabs(getDeltaAngle(angle1,angle));
				if(dist1 < dist0){
					dist0 = dist1;
					angle0 = angle1;
				}
			}

			angle = angle0;
		}
		break;
	case MOVEMENT_EIGHT_DIRS: {
			float dist0 = fabs(getDeltaAngle(0.0f,angle));
			float angle0 = 0.0f;

			for(int i = 1; i < 8; i ++){
				float angle1 = float(i)*M_PI/4.0f;
				float dist1 = fabs(getDeltaAngle(angle1,angle));
				if(dist1 < dist0){
					dist0 = dist1;
					angle0 = angle1;
				}
			}

			angle = angle0;
		}
		break;
	default:
		if(qdAnimationSet* p = animation_set())
			angle = p -> adjust_angle(angle);
		break;
	}

	return angle;
}

bool qdGameObjectStateWalk::is_state_empty() const
{
	if(qdGameObjectState::is_state_empty())
		return (!animation_set_info_.name() || strlen(animation_set_info_.name()));

	return false;
}

bool qdGameObjectStateWalk::update_sound_frequency(float direction_angle) const
{
	float coeff = 1.0f;
	if(qdAnimationSet* set = animation_set())
		coeff *= set -> walk_sound_frequency(direction_angle);

	return set_sound_frequency(coeff * walk_sound_frequency(direction_angle));
}

qdAnimationSet* qdGameObjectStateWalk::animation_set() const
{
	if(animation_set_info_.name()){
		if(qdGameScene* p = static_cast<qdGameScene*>(owner(QD_NAMED_OBJECT_SCENE))){
			if(qdAnimationSet* set = p -> get_animation_set(animation_set_info_.name()))
				return set;
		}

		if(qdGameDispatcher* p = qd_get_game_dispatcher())
			return p -> get_animation_set(animation_set_info_.name());
	}

	return 0;
}

qdAnimation* qdGameObjectStateWalk::animation(float direction_angle)
{
	if(qdAnimationInfo* inf = animation_info(direction_angle))
		return inf -> animation();

	return 0;
}

qdAnimation* qdGameObjectStateWalk::static_animation(float direction_angle)
{
	if(qdAnimationInfo* inf = static_animation_info(direction_angle))
		return inf -> animation();

	return 0;
}

qdAnimationInfo* qdGameObjectStateWalk::animation_info(float direction_angle)
{
	if(qdAnimationSet* set = animation_set())
		return set -> get_animation_info(direction_angle);

	return 0;
}

qdAnimationInfo* qdGameObjectStateWalk::static_animation_info(float direction_angle)
{
	if(qdAnimationSet* set = animation_set())
		return set -> get_static_animation_info(direction_angle);

	return 0;
}

const Vect2i& qdGameObjectStateWalk::center_offset(int direction_index, OffsetType offset_type) const
{
    const std::vector<Vect2i>* vect = &center_offsets_;

	switch(offset_type){
	case OFFSET_STATIC:
		vect = &static_center_offsets_;
		break;
	case OFFSET_WALK:
		vect = &center_offsets_;
		break;
	case OFFSET_START:
		vect = &start_center_offsets_;
		break;
	case OFFSET_END:
		vect = &stop_center_offsets_;
		break;
	}

	if(direction_index < 0 || direction_index >= vect->size()){
		static Vect2i v(0,0);
		return v;
	}
	else
		return (*vect)[direction_index];
}

const Vect2i& qdGameObjectStateWalk::center_offset(float direction_angle, OffsetType offset_type) const
{
	int index = 0;
	if(qdAnimationSet* p = animation_set())
		index = p -> get_angle_index(direction_angle);

	return center_offset(index, offset_type);
}

void qdGameObjectStateWalk::set_center_offset(int direction_index, const Vect2i& offs, OffsetType offset_type)
{
	assert(direction_index >= 0);

    std::vector<Vect2i>* vect = &center_offsets_;

	switch(offset_type){
	case OFFSET_STATIC:
		vect = &static_center_offsets_;
		break;
	case OFFSET_WALK:
		vect = &center_offsets_;
		break;
	case OFFSET_START:
		vect = &start_center_offsets_;
		break;
	case OFFSET_END:
		vect = &stop_center_offsets_;
		break;
	}

	if(direction_index >= vect->size())
		vect->resize(direction_index + 1,Vect2i(0,0));

	(*vect)[direction_index] = offs;
}

const float qdGameObjectStateWalk::walk_sound_frequency(int direction_index) const
{
	if(direction_index < 0 || direction_index >= walk_sound_frequency_.size())
		return 1;
	else
		return walk_sound_frequency_[direction_index];
}

const float qdGameObjectStateWalk::walk_sound_frequency(float direction_angle) const
{
	int index = 0;
	if(qdAnimationSet* p = animation_set())
		index = p -> get_angle_index(direction_angle);

	return walk_sound_frequency(index);
}

void qdGameObjectStateWalk::set_walk_sound_frequency(int direction_index,float freq)
{
	assert(direction_index >= 0);

	if(direction_index >= walk_sound_frequency_.size())
		walk_sound_frequency_.resize(direction_index + 1,1);

	walk_sound_frequency_[direction_index] = freq;
}

bool qdGameObjectStateWalk::load_script(const xml::tag* p)
{
	load_script_body(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_ANIMATION_SET:
			animation_set_info_.set_name(it -> data());
			break;
		case QDSCR_OBJECT_DIRECTION:
			xml::tag_buffer(*it) > direction_angle_;
			break;
		case QDSCR_STATE_CENTER_OFFSETS: {
				xml::tag_buffer buf(*it);
				center_offsets_.resize(it -> data_size()/2);
				for(int i = 0; i < it -> data_size()/2; i++)
					buf > center_offsets_[i].x > center_offsets_[i].y;
			}
			break;
		case QDSCR_STATE_STATIC_CENTER_OFFSETS: {
				xml::tag_buffer buf(*it);
				static_center_offsets_.resize(it -> data_size()/2);
				for(int i = 0; i < it -> data_size()/2; i++)
					buf > static_center_offsets_[i].x > static_center_offsets_[i].y;
			}
			break;
		case QDSCR_STATE_START_CENTER_OFFSETS: {
				xml::tag_buffer buf(*it);
				start_center_offsets_.resize(it -> data_size()/2);
				for(int i = 0; i < it -> data_size()/2; i++)
					buf > start_center_offsets_[i].x > start_center_offsets_[i].y;
			}
			break;
		case QDSCR_STATE_STOP_CENTER_OFFSETS: {
				xml::tag_buffer buf(*it);
				stop_center_offsets_.resize(it -> data_size()/2);
				for(int i = 0; i < it -> data_size()/2; i++)
					buf > stop_center_offsets_[i].x > stop_center_offsets_[i].y;
			}
			break;
		case QDSCR_OBJECT_STATE_WALK_SOUND_FREQUENCY: {
				xml::tag_buffer buf(*it);
				walk_sound_frequency_.resize(it -> data_size());
				for(int i = 0; i < it -> data_size(); i++)
					buf > walk_sound_frequency_[i];
			}
			break;
		case QDSCR_OBJECT_STATE_ACCELERATION:
			xml::tag_buffer(*it) > acceleration_ > max_speed_;
			break;
		case QDSCR_PERSONAGE_MOVEMENT_TYPE:
			movement_type_ = movement_type_t(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdGameObjectStateWalk::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<object_state_walk";

	fh < " movement=\"" <= movement_type_ < "\"";

	save_script_body(fh,indent);

	if(animation_set_info_.name()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<animation_set>" < qdscr_XML_string(animation_set_info_.name()) < "</animation_set>\r\n";
	}

	if(direction_angle_ > 0.0f){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<object_direction>" <= direction_angle_ < "</object_direction>\r\n";
	}

	if(acceleration_ > FLT_EPS || max_speed_ > FLT_EPS){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<acceleration>" <= acceleration_ < " " <= max_speed_ < "</acceleration>\r\n";
	}

	if(center_offsets_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<center_offsets>" <= center_offsets_.size() * 2;
		for(int i = 0; i < center_offsets_.size(); i++)
			fh < " " <= center_offsets_[i].x < " " <= center_offsets_[i].y;
		fh < "</center_offsets>\r\n";
	}

	if(static_center_offsets_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<static_center_offsets>" <= static_center_offsets_.size() * 2;
		for(int i = 0; i < static_center_offsets_.size(); i++)
			fh < " " <= static_center_offsets_[i].x < " " <= static_center_offsets_[i].y;
		fh < "</static_center_offsets>\r\n";
	}

	if(start_center_offsets_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<start_center_offsets>" <= start_center_offsets_.size() * 2;
		for(int i = 0; i < start_center_offsets_.size(); i++)
			fh < " " <= start_center_offsets_[i].x < " " <= start_center_offsets_[i].y;
		fh < "</start_center_offsets>\r\n";
	}

	if(stop_center_offsets_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<stop_center_offsets>" <= stop_center_offsets_.size() * 2;
		for(int i = 0; i < stop_center_offsets_.size(); i++)
			fh < " " <= stop_center_offsets_[i].x < " " <= stop_center_offsets_[i].y;
		fh < "</stop_center_offsets>\r\n";
	}

	if(walk_sound_frequency_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<walk_sound_frequency>" <= walk_sound_frequency_.size();
		for(int i = 0; i < walk_sound_frequency_.size(); i++)
			fh < " " <= walk_sound_frequency_[i];
		fh < "</walk_sound_frequency>\r\n";
	}

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</object_state_walk>\r\n";

	return true;
}

bool qdGameObjectStateWalk::register_resources()
{
	qdGameObjectState::register_resources();

	if(qdAnimationSet* p = animation_set())
		p -> register_resources(this);

	return true;
}

bool qdGameObjectStateWalk::unregister_resources()
{
	qdGameObjectState::unregister_resources();

	if(qdAnimationSet* p = animation_set())
		p -> unregister_resources(this);

	return true;
}

bool qdGameObjectStateWalk::load_resources()
{
	qdGameObjectState::load_resources();

	if(qdAnimationSet* p = animation_set())
		p -> load_animations(this);

	return true;
}

bool qdGameObjectStateWalk::free_resources()
{
	qdGameObjectState::free_resources();

	if(qdAnimationSet* p = animation_set())
		p -> free_animations(this);

	return true;
}

bool qdGameObjectStateWalk::auto_bound()
{
	qdAnimation* ap = static_animation(3.0f / 2.0f * M_PI);
	if(ap){
		bool res_flag = false;
		if(!ap -> is_resource_loaded()){
			ap -> load_resource();
			res_flag = true;
		}

		set_bound(Vect3f(ap -> picture_size_x(),ap -> picture_size_x(),ap -> picture_size_y()));

		if(res_flag)
			ap -> free_resource();

		return true;
	}

	return false;
}

bool qdGameObjectStateWalk::need_sound_restart() const
{
	if(owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
		if(!owner() -> check_flag(QD_OBJ_MOVING_FLAG))
			return false;
	}

	return qdGameObjectState::need_sound_restart();
}

/* -------------------------- qdGameObjectStateMask ------------------------- */

qdGameObjectStateMask::qdGameObjectStateMask() : qdGameObjectState(qdGameObjectState::STATE_MASK), qdContour(qdContour::CONTOUR_POLYGON),
	parent_(NULL)
{

}

qdGameObjectStateMask::qdGameObjectStateMask(const qdGameObjectStateMask& st) : qdGameObjectState(st), qdContour(st),
	parent_(st.parent_),
	parent_name_(st.parent_name_)
{
}

qdGameObjectStateMask::~qdGameObjectStateMask()
{
}

qdGameObjectStateMask& qdGameObjectStateMask::operator = (const qdGameObjectStateMask& st)
{
	if(this == &st) return *this;
	qdGameObjectState::operator = (*static_cast<const qdGameObjectState*>(&st));
	qdContour::operator = (*static_cast<const qdContour*>(&st));

	parent_name_ = st.parent_name_;
	parent_ = st.parent_;
	return *this;
}

qdGameObjectState& qdGameObjectStateMask::operator = (const qdGameObjectState& st)
{
	if(this == &st) return *this;

	assert(st.state_type() == STATE_MASK);
	qdGameObjectState::operator = (st);

	*static_cast<qdContour*>(this) = 
		*static_cast<const qdContour*>(static_cast<const qdGameObjectStateMask*>(&st));
	const qdGameObjectStateMask* sm = 
		static_cast<const qdGameObjectStateMask*>(&st);

	parent_name_ = sm->parent_name_;
	parent_ = sm->parent_;

	return *this;
}

bool qdGameObjectStateMask::is_state_empty() const
{
	if(qdGameObjectState::is_state_empty()) {
		return parent_name_.empty();
	}
	return false;
}

bool qdGameObjectStateMask::load_script(const xml::tag* p)
{
	load_script_body(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_OBJECT_STATE_MASK_PARENT:
				set_parent_name(it -> data());
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

bool qdGameObjectStateMask::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<object_state_mask";

	save_script_body(fh,indent);

	if(!parent_name_.empty()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<state_mask_parent>" < qdscr_XML_string(parent_name_.c_str()) < "</state_mask_parent>\r\n";
	}

	if(contour_size())
		qdContour::save_script(fh,indent + 1);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</object_state_mask>\r\n";

	return true;
}

bool qdGameObjectStateMask::hit(int x,int y) const
{
	const qdGameObject* p = parent();
	if(!p) return false;

	Vect2s scr_pos = p -> screen_pos();
	x -= scr_pos.x;
	y -= scr_pos.y;

	return is_inside(Vect2s(x,y));
}

qdGameObject* qdGameObjectStateMask::parent()
{
	if(parent_) return parent_;

	if(parent_name_.empty() || !owner()) return 0;

	qdNamedObject* p = owner() -> owner();
	if(!p || p -> named_object_type() != QD_NAMED_OBJECT_SCENE) return 0;

#ifndef _QUEST_EDITOR
	parent_ = static_cast<qdGameScene*>(p) -> get_object(parent_name_.c_str());
	return parent_;
#else
	return static_cast<qdGameScene*>(p) -> get_object(parent_name_.c_str());
#endif
}

const qdGameObject* qdGameObjectStateMask::parent() const
{
	if(parent_) return parent_;

	if(parent_name_.empty() || !owner()) return 0;

	qdNamedObject* p = owner() -> owner();
	if(!p || p -> named_object_type() != QD_NAMED_OBJECT_SCENE) return 0;

	return static_cast<qdGameScene*>(p) -> get_object(parent_name_.c_str());
}

bool qdGameObjectStateMask::draw_mask(unsigned color) const
{
	const qdGameObject* p = parent();
	if(!p) return false;

	Vect2s pos = p -> screen_pos() + mask_pos();
	pos.x -= mask_size().x/2;
	pos.y -= mask_size().y/2;

	for(int y = 0; y < mask_size().y; y ++){
		for(int x = 0; x < mask_size().x; x ++){
			if(hit(pos.x + x, pos.y + y)){
				grDispatcher::instance() -> SetPixel(pos.x + x,pos.y + y,color);
			}
		}
	}

	return true;
}

#ifdef _QUEST_EDITOR
void qdGameObjectStateMask::copy_contour(qdGameObjectStateMask const* source)
{
	set_contour(source->get_contour());
}
#endif // _QUEST_EDITOR


bool qdGameObjectStateMask::load_resources()
{
	qdGameObjectState::load_resources();
	return true;
}


