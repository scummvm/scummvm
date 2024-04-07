/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_game_object_mouse.h"
#include "qd_game_object_animated.h"
#include "qd_game_scene.h"
#include "qd_game_dispatcher.h"
#include "qd_setup.h"
#include "qd_grid_zone.h"
#include "qd_animation_set.h"

#include <stdio.h>
/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#ifdef _QUEST_EDITOR
bool qdGameObjectAnimated::fast_state_merge_ = false;
#endif

qdGameObjectAnimated::qdGameObjectAnimated() : cur_state_(-1),
	inventory_type_(0),
	last_state_(NULL),
	default_r_(0,0,0),
	grid_r_(0,0,0),
	grid_size_(0,0),
	queued_state_(NULL),
	last_frame_(NULL),
	inventory_cell_index_(-1),
	shadow_alpha_(QD_NO_SHADOW_ALPHA),
	shadow_color_(0)
{
	set_animation(NULL);

	set_bound(Vect3f(10,10,10));

	last_screen_depth_ = 0;

	lastShadowColor_ = 0;
	lastShadowAlpha_ = QD_NO_SHADOW_ALPHA;

	if (NULL != qdGameDispatcher::get_dispatcher())
		last_chg_time_ = qdGameDispatcher::get_dispatcher()->time();
	else
		last_chg_time_ = 0;
}

qdGameObjectAnimated::qdGameObjectAnimated(const qdGameObjectAnimated& obj) : qdGameObject(obj),
	cur_state_(-1),
	queued_state_(NULL),
	inventory_type_(obj.inventory_type_),
	bound_(obj.bound_),
	radius_(obj.radius_),
	default_r_(obj.default_r_),
	grid_r_(0,0,0),
	grid_size_(0,0),
	inventory_name_(obj.inventory_name_),
	last_state_(NULL),
	inventory_cell_index_(-1),
	last_frame_(NULL),
	last_chg_time_(obj.last_chg_time_),
	shadow_alpha_(obj.shadow_alpha_),
	shadow_color_(obj.shadow_color_)
{
	last_screen_depth_ = 0;

	lastShadowColor_ = 0;
	lastShadowAlpha_ = QD_NO_SHADOW_ALPHA;

	qdGameObjectStateVector::const_iterator it;
	FOR_EACH(obj.states,it){
		if(!(*it) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
			add_state((*it) -> clone());
		else 
			add_state(*it);
	}
}

qdGameObjectAnimated::~qdGameObjectAnimated()
{
	clear_states();
}

qdGameObjectAnimated& qdGameObjectAnimated::operator = (const qdGameObjectAnimated& obj)
{
	if(this == &obj) return *this;

	*static_cast<qdGameObject*>(this) = obj;

	cur_state_ = -1;
	queued_state_ = NULL;

	inventory_type_ = obj.inventory_type_;

	animation_.clear();

	set_bound(obj.bound_);

	default_r_ = obj.default_r_;

	grid_r_ = obj.grid_r_;
	grid_size_ = obj.grid_size_;

	inventory_name_ = obj.inventory_name_;

	last_state_ = NULL;

	inventory_cell_index_ = -1;

	last_frame_ = NULL;
	last_screen_region_ = grScreenRegion::EMPTY;
	last_screen_depth_ = 0;

	lastShadowColor_ = 0;
	lastShadowAlpha_ = QD_NO_SHADOW_ALPHA;

	shadow_alpha_ = obj.shadow_alpha_;
	shadow_color_ = obj.shadow_color_;

	last_chg_time_ = obj.last_chg_time();

	clear_states();

	qdGameObjectStateVector::const_iterator it;
	FOR_EACH(obj.states,it){
		if(!(*it) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
			add_state((*it) -> clone());
		else 
			add_state(*it);
	}

	return *this;
}

void qdGameObjectAnimated::clear_states()
{
	qdGameObjectStateVector::iterator it;
	FOR_EACH(states,it){
		(*it) -> dec_reference_count();

		if(!(*it) -> reference_count()){
			delete (*it);
			*it = NULL;
		}
	}
}

void qdGameObjectAnimated::set_animation(qdAnimation* p,const qdAnimationInfo* inf)
{ 
	if(p){
//		if(!p -> is_resource_loaded())
//			p -> load_resources();

		p -> create_reference(&animation_,inf);
		animation_.start(); 
	}
	else
		animation_.clear(); 
}

void qdGameObjectAnimated::set_animation_info(qdAnimationInfo* inf)
{
	if(inf){
		if(qdAnimation* p = inf -> animation())
			set_animation(p,inf);
		else
			animation_.clear(); 
	}
	else
		animation_.clear(); 
}

void qdGameObjectAnimated::set_screen_rotation(float target_angle, float speed)
{
	target_transform_.set_angle(target_angle);
	transform_speed_.set_angle(speed);
}

float qdGameObjectAnimated::screen_rotation() const
{
	return current_transform_.angle();
}

void qdGameObjectAnimated::set_screen_scale(const Vect2f& scale, const Vect2f& speed)
{
	target_transform_.set_scale(scale);
	transform_speed_.set_scale(speed);
}

const Vect2f& qdGameObjectAnimated::screen_scale() const
{
	return current_transform_.scale();
}

void qdGameObjectAnimated::redraw(int offs_x,int offs_y) const
{
	if(!animation_.is_empty()){
		Vect2i r = screen_pos() + Vect2i(offs_x,offs_y);

		if(current_transform_()){
/*
			Vect2i delta = (cur_state_ != -1) ? states[cur_state_] -> center_offset() : Vect2i(0,0);
			if(delta.x || delta.y){
				r -= delta;

				Vect2f scale(current_transform_.scale());

				delta.x = round(float(delta.x) * scale.x);
				delta.y = round(float(delta.y) * scale.y);

				float angle = current_transform_.angle();

				r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
				r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			}
*/
			if(current_transform_.has_scale())
				animation_.redraw_rot(r.x,r.y,screen_depth(),current_transform_.angle(),current_transform_.scale());
			else
				animation_.redraw_rot(r.x,r.y,screen_depth(),current_transform_.angle());
		}
		else
			animation_.redraw(r.x,r.y,screen_depth());
	}

	if(shadow_alpha() != QD_NO_SHADOW_ALPHA)
		draw_shadow(offs_x, offs_y, shadow_color(), shadow_alpha());
}

bool qdGameObjectAnimated::need_redraw() const
{
	unsigned color = (cur_state_ == -1) ? 0 : states[cur_state_]->shadow_color();
	int alpha = (cur_state_ == -1) ? QD_NO_SHADOW_ALPHA : states[cur_state_]->shadow_alpha();

	return (animation_.get_cur_frame() != last_frame_) ||
	(last_screen_depth_ != screen_depth()) ||
	(screen_region() != last_screen_region_) ||
	(last_transform_ != current_transform_) ||
	lastShadowColor_ != color || lastShadowAlpha_ != alpha;
}

void qdGameObjectAnimated::quant(float dt)
{
	Vect3f beg_r = R();

	qdGameObject::quant(dt);

	get_animation() -> quant(dt);

	if(target_transform_ != current_transform_)
		current_transform_.change(dt, target_transform_, transform_speed_);

#ifndef _QUEST_EDITOR
	for(qdGameObjectStateVector::iterator it = states.begin(); it != states.end(); ++it)
		(*it) -> quant(dt);
#endif // _QUEST_EDITOR

	if(cur_state_ != -1){
#ifdef _QUEST_EDITOR
		states[cur_state_]->quant(dt);
#endif // _QUEST_EDITOR
		if(!states[cur_state_] -> coords_animation() -> is_empty())
			states[cur_state_] -> coords_animation() -> quant(dt);

#ifndef _QUEST_EDITOR
		if(!states[cur_state_] -> is_in_triggers()){
			for(qdGameObjectStateVector::iterator it = states.begin(); it != states.end(); ++it){
				if(states[cur_state_] != *it)
					(*it) -> set_prev_state(states[cur_state_]);
			}
		}

		if(!check_flag(QD_OBJ_HIDDEN_FLAG) && !check_flag(QD_OBJ_IS_IN_TRIGGER_FLAG)){
			if(!check_flag(QD_OBJ_STATE_CHANGE_FLAG)){
				StateStatus status = state_status(states[cur_state_]);
				for(qdGameObjectStateVector::iterator it = states.begin(); it != states.end(); ++it){
					if(!(*it) -> is_in_triggers()){
						if(states[cur_state_] != *it || status == STATE_DONE){
							if((*it) -> check_conditions()){
								set_state(*it);
								set_flag(QD_OBJ_STATE_CHANGE_FLAG);
								status = state_status(states[cur_state_]);
								break;
							}
						}
					}
				}
			}
			if(state_status(states[cur_state_]) == STATE_DONE)
				handle_state_end();
		}
#endif
	}

	if(queued_state())
		set_state(queued_state());

	drop_flag(QD_OBJ_STATE_CHANGE_FLAG);

	// Если текущ. позиция не соответствует той, что была в начале кванта, то
	// объект изменился
	if (R() != beg_r)
		last_chg_time_ = qdGameDispatcher::get_dispatcher() -> time();

//	appLog::default_log() << name() << " " << animation_.cur_time() << " " << R().x << " " << R().y << " " << R().z << "\r\n";
}

bool qdGameObjectAnimated::load_script_body(const xml::tag* p)
{
	qdGameObject::load_script_body(p);

	Vect3f v;
	qdGameObjectState* st;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
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
			st -> load_script(&*it);
			add_state(st);
			break;
		case QDSCR_OBJECT_STATE_WALK:
			st = new qdGameObjectStateWalk;
			st -> load_script(&*it);
			add_state(st);
			break;
		case QDSCR_OBJECT_STATE_MASK:
			st = new qdGameObjectStateMask;
			st -> load_script(&*it);
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
			set_inventory_name(it -> data());
			break;
		}
	}

	set_default_pos(R());

	return true;
}

int qdGameObjectAnimated::idle_time() const
{ 
	return qdGameDispatcher::get_dispatcher() -> time() - last_chg_time_; 
}

int qdGameObjectAnimated::shadow_color() const
{
	if(shadow_alpha_ != QD_NO_SHADOW_ALPHA)
		return shadow_color_;

	if(cur_state_ != -1)
		return states[cur_state_]->shadow_color();

	return 0;
}

int qdGameObjectAnimated::shadow_alpha() const
{
	if(shadow_alpha_ != QD_NO_SHADOW_ALPHA)
		return shadow_alpha_;

	if(cur_state_ != -1)
		return states[cur_state_]->shadow_alpha();

	return QD_NO_SHADOW_ALPHA;
}

bool qdGameObjectAnimated::insert_state(int iBefore, qdGameObjectState* p)
{
	p -> set_owner(this);
	p -> inc_reference_count();

	states.insert(states.begin() + iBefore, p);
	
	if(!p -> name()){
		XBuffer name_str(64);
		name_str < "Состояние " <= max_state();
		p -> set_name(name_str.c_str());
	}
	return true;
}

bool qdGameObjectAnimated::add_state(qdGameObjectState* p)
{
	p -> set_owner(this);
	p -> inc_reference_count();

	states.push_back(p);

	if(!p -> name()){
		XBuffer name_str(64);
		name_str < "Состояние " <= max_state();
		p -> set_name(name_str.c_str());
	}

	return true;
}

qdGameObjectState* qdGameObjectAnimated::remove_state(int state_num)
{				   
	if(state_num < 0 || state_num >= max_state()) return 0;

	qdGameObjectStateVector::iterator it = states.begin() + state_num;

	qdGameObjectState* p = *it;
	states.erase(it);

	p -> dec_reference_count();

#ifdef _QUEST_EDITOR
	if(cur_state_ >= max_state())
		set_state(--cur_state_);
#else
	if(cur_state_ >= max_state())
		--cur_state_;
#endif

	return p;
}

bool qdGameObjectAnimated::remove_state(qdGameObjectState* p)
{
	qdGameObjectStateVector::iterator it = std::find(states.begin(), states.end(), p);
	if(it != states.end()) {
		states.erase(it);
		p -> dec_reference_count();
		
		if(cur_state_ >= max_state())
			set_state(--cur_state_);
		
		return true;
	}
	return false;
}

bool qdGameObjectAnimated::save_script_body(XStream& fh,int indent) const
{
	qdGameObject::save_script_body(fh,indent);

	qdGameObjectStateVector::const_iterator is = states.begin();
	FOR_EACH(states,is){
		if(!(*is) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER) || owner() -> named_object_type() == QD_NAMED_OBJECT_DISPATCHER)
			(*is) -> save_script(fh,indent + 1);
		else {
			appLog::default_log() << "state not saved: ";
			if(owner() && owner() -> name())
			appLog::default_log() << owner() -> name() << "::";

			appLog::default_log() << name() << "::" << (*is) -> name() << "\r\n";
		}
	}

	if(inventory_type_){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<inventory_cell_type>" <= inventory_type_ < "</inventory_cell_type>\r\n";
	}

	if(!inventory_name_.empty()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<inventory>" < inventory_name_.c_str() < "</inventory>\r\n";
	}

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<bound>" <= bound_.x < " " <= bound_.y < " " <= bound_.z < "</bound>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<default_pos>" <= default_r_.x < " " <= default_r_.y < " " <= default_r_.z < "</default_pos>\r\n";

	return true;
}

bool qdGameObjectAnimated::load_script(const xml::tag* p)
{
	return load_script_body(p);
}

bool qdGameObjectAnimated::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<animated_object name=\"" < qdscr_XML_string(name()) < "\">\r\n";

	save_script_body(fh,indent);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</animated_object>\r\n";

	return true;
}

bool qdGameObjectAnimated::load_resources()
{
	if(cur_state_ != -1){
		states[cur_state_] -> register_resources();
		states[cur_state_] -> load_resources();

		if(animation_.parent()){
			int fl = animation_.flags();
			float tm = animation_.cur_time_rel();
			animation_.parent() -> create_reference(&animation_);
			animation_.clear_flags();
			animation_.set_flag(fl);
			animation_.set_time_rel(tm);
		}
	}
	else
		set_default_state();

	for(int i = 0; i < max_state(); i++){
		if(i != cur_state_ && states[i] -> forced_load()){
			states[i] -> register_resources();
			states[i] -> load_resources();
		}
	}

	init_grid_zone();
	return true;
}

void qdGameObjectAnimated::free_resources()
{
	for(int i = 0; i < max_state(); i++){
		states[i] -> unregister_resources();
		states[i] -> free_resources();
	}
}

void qdGameObjectAnimated::set_state(int st)
{
	// Указание на смену состояния => объект меняется (устанавливаем время изм.)
	last_chg_time_ = qdGameDispatcher::get_dispatcher()->time();

	if(max_state() && st >= 0 && st <= max_state()){
		qdGameObjectState* p = states[st];

#ifndef _QUEST_EDITOR
		if(p -> activation_delay() > 0.001f){
			appLog::default_log() << appLog::default_log().time_string() << " состояние ждет: " << name() << "/" << get_state(st) -> name() << "\r\n";

			if(!p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER)){
				p -> set_activation_timer();
				p -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
				set_queued_state(p);
				return;
			}

			if(!p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END))
				return;

			set_queued_state(NULL);
		}

		p -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);

		if(p -> is_in_triggers())
			set_flag(QD_OBJ_IS_IN_TRIGGER_FLAG);

		if(p -> has_camera_mode() && owner())
			static_cast<qdGameScene*>(owner()) -> set_camera_mode(p -> camera_mode(),this);
#endif

		if(cur_state_ != -1 && cur_state_ < max_state()){
			states[cur_state_] -> stop_sound();
			if(!states[cur_state_] -> forced_load() && cur_state_ != st){
				states[cur_state_] -> unregister_resources();
				p -> register_resources();
				states[cur_state_] -> free_resources();
			}
			else
				p -> register_resources();
		}
		else
			p -> register_resources();

		qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
		assert(dp);

#ifdef _QUEST_EDITOR
		//возвращаем начальное положение объекта,
		//если оно вдруг было изменено предыдущим состоянием
		set_pos(default_R());
#else
		if(cur_state_ != -1 && cur_state_ < max_state() && states[cur_state_] -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY) && !p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY)){
			dp -> remove_from_inventory(this);

			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);
			set_pos(default_R());
		}
#endif

		if(cur_state_ != st){
			set_last_state(get_cur_state());
			set_last_inventory_state(get_cur_state());
			set_cur_state(st);
		}

		p -> load_resources();
		p -> start();

#ifdef _QUEST_EDITOR
		dp -> screen_texts_dispatcher().clear_texts();
		if(p -> has_full_text() ){
			dp -> screen_texts_dispatcher().add_text(
				qdGameDispatcher::TEXT_SET_DIALOGS,
				qdScreenText(p -> full_text(),p -> text_format(),Vect2i(0,0),p));
		}
		else if (p -> has_short_text()){
			dp -> screen_texts_dispatcher().add_text(
				qdGameDispatcher::TEXT_SET_DIALOGS,
				qdScreenText(p -> short_text(),p -> text_format(),Vect2i(0,0),p));
		}
#else
		dp -> screen_texts_dispatcher().clear_texts(this);
		if(p -> has_text() && !p -> has_text_delay() && !p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DIALOG_PHRASE))
			dp -> screen_texts_dispatcher().add_text(qdGameDispatcher::TEXT_SET_DIALOGS,qdScreenText(p -> text(),p -> text_format(),Vect2i(0,0),p));
#endif

#ifndef _QUEST_EDITOR
		p -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
		drop_flag(QD_OBJ_HIDDEN_FLAG);
#endif

		p -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
		p -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);

#ifndef _QUEST_EDITOR
		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_ZONE)){
			dp -> remove_from_inventory(this);
			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);

			// Ставим объект по клику мыши
			Vect2f v2 = qdGameDispatcher::get_dispatcher()->get_active_scene()->
				        mouse_click_pos();
			Vect3f v3 = Vect3f(v2.x, v2.y, 0);
			set_pos(v3);
		}
		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_ON_OBJECT)){
			dp -> remove_from_inventory(this);
			drop_flag(QD_OBJ_SCREEN_COORDS_FLAG);

			// Ставим объект, состояние которого активируется, в центр объекта по 
			// которому был произведен клик (для этого ищем его в текущей сцене)
			Vect2f mouse_pos = qdGameDispatcher::get_dispatcher()->mouse_cursor_pos();
			qdGameObject* pObj = qdGameDispatcher::get_dispatcher()->
				                 get_active_scene()->
								 get_hitted_obj(mouse_pos.x, mouse_pos.y);
			if (NULL != pObj)							
				set_pos(pObj->R());			
		}

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_SAVE))
			dp->set_auto_save(p->autosave_slot());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_LOAD))
			dp->set_auto_load(p->autosave_slot());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_IN))
			dp->set_fade(true, p->fade_time());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_OUT))
			dp->set_fade(false, p->fade_time());
#endif

		restore_grid_zone();
		init_grid_zone();
		
		switch(p -> state_type()){
			case qdGameObjectState::STATE_STATIC:
				set_animation_info(static_cast<qdGameObjectStateStatic*>(p) -> animation_info());
				break;
			case qdGameObjectState::STATE_MASK:
				animation_.clear();
				break;
		}

		if(!p -> coords_animation() -> is_empty()){
			p -> coords_animation() -> start();
			get_animation() -> set_time_rel(p -> coords_animation() -> animation_phase());
		}

		if(p->has_transform()){
			set_screen_rotation(p->transform().angle(), p->transform_speed().angle());
			set_screen_scale(p->transform().scale(), p->transform_speed().scale());

			if(!last_state_)
				current_transform_ = target_transform_;
		}

		if(!p -> has_sound_delay())
			p -> play_sound();
		return;
	}

	if(st == -1){
		if(cur_state_ != -1 && cur_state_ < max_state()){
			states[cur_state_] -> stop_sound();
			states[cur_state_] -> unregister_resources();
			states[cur_state_] -> free_resources();
		}

		set_cur_state(st);
		get_animation() -> clear();
	}
}

void qdGameObjectAnimated::set_state(qdGameObjectState* p)
{
	for(int i = 0; i < max_state(); i ++){
		if(states[i] == p){
			set_state(i);
			return;
		}
	}
}

bool qdGameObjectAnimated::is_visible() const
{
	if(max_state() && cur_state_ != -1){
		if(states[cur_state_] -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDDEN))
			return false;
	}

	return qdGameObject::is_visible();
}

void qdGameObjectAnimated::set_bound(const Vect3f& b)
{ 
	bound_ = b;
	Vect3f b2 = b / 2.0f;
	radius_ = b2.norm();
}

bool qdGameObjectAnimated::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	if(ev == mouseDispatcher::EV_LEFT_DOWN)
		return true;

	return false;
}

void qdGameObjectAnimated::debug_redraw() const
{
	if(!is_visible()) return;
	
	if(has_bound()){
		draw_bound();
//		draw_grid_zone(grid_size_);
	}

	Vect3f pos = R();
	Vect2s scr_pos = qdCamera::current_camera() -> global2scr(pos);

	const int NET_PROJ_SIZE	= 10;
	const int OBJ_CENTER_SIZE = 6;

	pos.z = qdCamera::current_camera() -> get_grid_center().z;
	Vect2s proj_pos = qdCamera::current_camera() -> global2scr(pos);
	//прорисовываем проекцию на сетку
	grDispatcher::instance()->Rectangle(
				proj_pos.x - (NET_PROJ_SIZE>>1), 
				proj_pos.y - (NET_PROJ_SIZE>>1), 
				NET_PROJ_SIZE, NET_PROJ_SIZE, 
				0x00FF0000, 0x000000FF, GR_FILLED);

	//прорисовываем центр объекта
	grDispatcher::instance()->Rectangle(
				scr_pos.x - (OBJ_CENTER_SIZE>>1), 
				scr_pos.y - (OBJ_CENTER_SIZE>>1), 
				OBJ_CENTER_SIZE, OBJ_CENTER_SIZE, 
				0x00FF0000, 0x00FF0000, GR_FILLED);
	grDispatcher::instance()->Line(proj_pos.x, proj_pos.y, scr_pos.x, scr_pos.y, 0x00FF0000);

	Vect2s ssz = screen_size();

	if(!current_transform_()){
		grDispatcher::instance()->Rectangle(
					scr_pos.x - ssz.x/2, 
					scr_pos.y - ssz.y/2, 
					ssz.x, ssz.y, 
					0x00FF0000, 0x000000FF, GR_OUTLINED);
	}
	else {
			Vect2f scale(current_transform_.scale());

			ssz.x = round(float(ssz.x) * scale.x);
			ssz.y = round(float(ssz.y) * scale.y);

			float sn = sinf(-current_transform_.angle());
			float cs = cosf(-current_transform_.angle());

			Vect2i v0, v1, p0, p1;

			v0 = Vect2i(-ssz.x/2, -ssz.y/2);
			v1 = Vect2i(-ssz.x/2, +ssz.y/2);

			p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
			p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

			p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
			p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

			v0 = Vect2i(-ssz.x/2, +ssz.y/2);
			v1 = Vect2i(+ssz.x/2, +ssz.y/2);

			p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
			p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

			p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
			p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

			v0 = Vect2i(+ssz.x/2, +ssz.y/2);
			v1 = Vect2i(+ssz.x/2, -ssz.y/2);

			p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
			p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

			p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
			p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);

			v0 = Vect2i(+ssz.x/2, -ssz.y/2);
			v1 = Vect2i(-ssz.x/2, -ssz.y/2);

			p0.x = screen_pos().x + round(float(v0.x) * cs + float(v0.y) * sn);
			p0.y = screen_pos().y + round(float(-v0.x) * sn + float(v0.y) * cs);

			p1.x = screen_pos().x + round(float(v1.x) * cs + float(v1.y) * sn);
			p1.y = screen_pos().y + round(float(-v1.x) * sn + float(v1.y) * cs);

			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, 0x000000FF);
	}

	grDispatcher::instance()->Rectangle(
				screen_pos().x - 2, 
				screen_pos().y - 2, 
				4, 4, 
				0x00FF0000, 0x000000FF, GR_FILLED);

	static XBuffer buf(1024);
	buf.init();
	if(get_debug_info(buf))
		grDispatcher::instance() -> DrawText(scr_pos.x,scr_pos.y - 20,grDispatcher::instance() -> make_rgb888(255,255,255),buf.c_str());
/*
	for(int i = 0; i < qdGameConfig::get_config().screen_sy(); i++){
		for(int j = 0; j < qdGameConfig::get_config().screen_sx(); j++){
			if(hit(j,i))
				grDispatcher::instance()->SetPixel(j, i, 0x0000FF00);
		}
	}
*/
//	appLog::default_log() << name() << " screen_pos:"  << screen_pos().x << " " << screen_pos().y << "\r\n";

//	if(const qdGameObjectStateMask* st = dynamic_cast<const qdGameObjectStateMask*>(get_cur_state()))
//		st->draw_mask(grDispatcher::instance()->make_rgb(255,255,255));
}

bool qdGameObjectAnimated::hit(int x,int y) const
{
	if(!is_visible()) return false;

	Vect2s sr = screen_pos();
	if(cur_state_ != -1){
		if(states[cur_state_] -> state_type() == qdGameObjectState::STATE_MASK)
			return static_cast<qdGameObjectStateMask*>(states[cur_state_]) -> hit(x,y);
		else {
			if(current_transform_()){
				x -= sr.x;
				y -= sr.y;

				float cs = cosf(current_transform_.angle());
				float sn = sinf(current_transform_.angle());

				int xx = round(1.f/current_transform_.scale().x * (float(x) * cs + float(y) * sn));
				int yy = round(1.f/current_transform_.scale().y * (float(-x) * sn + float(y) * cs));

				return animation_.hit(xx,yy);
			}
			else
				return animation_.hit(x - sr.x,y - sr.y);
		}
	}

	return false;
}

qdGameObjectState* qdGameObjectAnimated::get_state(const char* state_name)
{
	qdGameObjectStateVector::iterator it;
	FOR_EACH(states,it){
		qdGameObjectState* p = *it;
		if((*it) -> name() && !strcmp((*it) -> name(),state_name))
			return *it;
	}

	return NULL;
}

const qdGameObjectState* qdGameObjectAnimated::get_state(const char* state_name) const
{
	qdGameObjectStateVector::const_iterator it;
	FOR_EACH(states,it){
		const qdGameObjectState* p = *it;
		if((*it) -> name() && !strcmp((*it) -> name(),state_name))
			return p;
	}

	return NULL;
}

qdGameObjectState* qdGameObjectAnimated::get_state(int state_index)
{
	if(state_index >= 0 && state_index < max_state())
		return states[state_index];

	return NULL;
}

const qdGameObjectState* qdGameObjectAnimated::get_state(int state_index) const
{
	if(state_index >= 0 && state_index < max_state())
		return states[state_index];

	return NULL;
}

qdGameObjectAnimated::StateStatus qdGameObjectAnimated::state_status(const qdGameObjectState* p) const
{
	if(queued_state_ == p)
		return STATE_QUEUED;

	if(cur_state_ != -1 && states[cur_state_] == p){
		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED))
			return STATE_ACTIVE;

		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_SKIP)){
			if(mouseDispatcher::instance() -> check_event(mouseDispatcher::EV_LEFT_DOWN) || mouseDispatcher::instance() -> check_event(mouseDispatcher::EV_RIGHT_DOWN))
				return STATE_DONE;
		}

		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_SOUND_SYNC)){
			if(!p -> is_sound_finished())
				return STATE_ACTIVE;
		}

		if(p -> work_time() > 0.001f){
			if(p -> cur_time() >= p -> work_time())
				return STATE_DONE;
			else
				return STATE_ACTIVE;
		}

		if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDDEN))
			return STATE_DONE;

		if(!p -> coords_animation() -> is_empty()){ 
			if(p -> coords_animation() -> size() > 1){
				if(!p -> coords_animation() -> is_finished())
					return STATE_ACTIVE;

				return STATE_DONE;
			}
		}

		switch(p -> state_type()){
			case qdGameObjectState::STATE_STATIC: {
					const qdGameObjectStateStatic* sp = static_cast<const qdGameObjectStateStatic*>(p);
					if(const qdAnimation* anm = sp -> animation()){
						if(anm -> is_reference(get_animation())){
							if(!animation_.is_finished())
								return STATE_ACTIVE;
						}
						else
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

void qdGameObjectAnimated::draw_contour(unsigned color) const
{
	if(!get_animation() -> is_empty()){
		Vect2s scrCoord = screen_pos();
		get_animation() -> draw_contour(scrCoord.x,scrCoord.y,color);
	}
}

int qdGameObjectAnimated::mouse_cursor_ID() const
{
	if(cur_state_ != -1)
		return states[cur_state_] -> mouse_cursor_ID();

	return qdGameObjectState::CURSOR_UNASSIGNED;
}

void qdGameObjectAnimated::restore_state()
{
	if(qdGameObjectState* st = states[cur_state_] -> prev_state()){
		states[cur_state_] -> set_prev_state(0);
		set_state(st);
	}
}

void qdGameObjectAnimated::merge_states(qdGameObjectAnimated* p)
{
	if(states.empty() || (!p -> states.empty() && states[0] != p -> states[0])){
		for(int i = 0; i < p -> max_state(); i ++){
			insert_state(i,p -> states[i]);
			p -> states[i] -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
#ifdef _QUEST_EDITOR
			p -> states[i] -> set_ref_owner(p);
#endif
		}
	}

#ifdef _QUEST_EDITOR
		if(!fast_state_merge_)
			set_default_state();
#endif
}

void qdGameObjectAnimated::split_states(qdGameObjectAnimated* p)
{
#ifndef _QUEST_EDITOR
	int st = cur_state_;
#endif

	if(!states.empty() && !p -> states.empty() && states[0] == p -> states[0]){
		for(int i = 0; i < p -> max_state(); i ++){
			remove_state(0);
			p -> states[i] -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER);
			p -> states[i] -> set_owner(p);
		}
	}

#ifndef _QUEST_EDITOR
	cur_state_ = st;
#endif
}

bool qdGameObjectAnimated::init_grid_zone()
{
	if(has_bound() && owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_SCENE){
		Vect3f b = bound(false);
		grid_size_.x = b.xi() / static_cast<qdGameScene*>(owner()) -> get_camera() -> get_cell_sx();
		grid_size_.y = b.yi() / static_cast<qdGameScene*>(owner()) -> get_camera() -> get_cell_sy();

		if(grid_size_.x < 1) grid_size_.x = 1;
		if(grid_size_.y < 1) grid_size_.y = 1;

		return true;
	}
	return false;
}

bool qdGameObjectAnimated::toggle_grid_zone(bool make_walkable)
{
	if(make_walkable)
		return drop_grid_zone_attributes(sGridCell::CELL_OCCUPIED);
	else
		return set_grid_zone_attributes(sGridCell::CELL_OCCUPIED);
}

bool qdGameObjectAnimated::save_grid_zone()
{
	grid_r_ = R();
	return true;
}

bool qdGameObjectAnimated::restore_grid_zone()
{
	return toggle_grid_zone(true);
}

qdGameObjectState* qdGameObjectAnimated::get_inventory_state()
{
#ifndef _QUEST_EDITOR
	if(last_inventory_state_) return last_inventory_state_;

	if(states.size()){
		qdGameObjectStateVector::iterator it;
		FOR_EACH(states,it){
			if((*it) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
				return *it;
		}
	}
#endif
	return NULL;
}

#ifdef _QUEST_EDITOR
const Vect3f& qdGameObjectAnimated::obj_bound() const
{
	return bound_;
}
#endif // _QUEST_EDITOR

const Vect3f& qdGameObjectAnimated::bound(bool perspective_correction) const
{
	if(cur_state_ != -1 && states[cur_state_] -> has_bound())
		return states[cur_state_] -> bound();

	return bound_;
}

float qdGameObjectAnimated::radius() const
{
	if(cur_state_ != -1 && states[cur_state_] -> has_bound())
		return states[cur_state_] -> radius();

	return radius_;
}

void qdGameObjectAnimated::draw_bound(Vect3f r, 
									  Vect3f const& bound,
									  int const color) const
{
	float const bx = bound.x / 2.0f;
	float const by = bound.y / 2.0f;
	float const bz = bound.z / 2.0f;
	if(const qdCamera* cp = qdCamera::current_camera()){
		if(named_object_type() == QD_NAMED_OBJECT_ANIMATED_OBJ)
			r.z = bz + cp->get_grid_center().z;

		Vect2s v0 = cp -> global2scr(Vect3f(r.x - bx,r.y - by,r.z - bz));
		Vect2s v1 = cp -> global2scr(Vect3f(r.x - bx,r.y + by,r.z - bz));
		Vect2s v2 = cp -> global2scr(Vect3f(r.x + bx,r.y + by,r.z - bz));
		Vect2s v3 = cp -> global2scr(Vect3f(r.x + bx,r.y - by,r.z - bz));

		Vect2s _v0 = cp -> global2scr(Vect3f(r.x - bx,r.y - by,r.z + bz));
		Vect2s _v1 = cp -> global2scr(Vect3f(r.x - bx,r.y + by,r.z + bz));
		Vect2s _v2 = cp -> global2scr(Vect3f(r.x + bx,r.y + by,r.z + bz));
		Vect2s _v3 = cp -> global2scr(Vect3f(r.x + bx,r.y - by,r.z + bz));

		grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,color,4,true);
		grDispatcher::instance() -> Line(v1.x,v1.y,v2.x,v2.y,color,4,true);
		grDispatcher::instance() -> Line(v2.x,v2.y,v3.x,v3.y,color,4,true);
		grDispatcher::instance() -> Line(v3.x,v3.y,v0.x,v0.y,color,4,true);

		grDispatcher::instance() -> Line(_v0.x,_v0.y,_v1.x,_v1.y,color,4,true);
		grDispatcher::instance() -> Line(_v1.x,_v1.y,_v2.x,_v2.y,color,4,true);
		grDispatcher::instance() -> Line(_v2.x,_v2.y,_v3.x,_v3.y,color,4,true);
		grDispatcher::instance() -> Line(_v3.x,_v3.y,_v0.x,_v0.y,color,4,true);

		grDispatcher::instance() -> Line(v0.x,v0.y,_v0.x,_v0.y,color,4,true);
		grDispatcher::instance() -> Line(v1.x,v1.y,_v1.x,_v1.y,color,4,true);
		grDispatcher::instance() -> Line(v2.x,v2.y,_v2.x,_v2.y,color,4,true);
		grDispatcher::instance() -> Line(v3.x,v3.y,_v3.x,_v3.y,color,4,true);
	}
}

void qdGameObjectAnimated::draw_bound() const
{
	const int cl = 0xFFFFFF;
	draw_bound(R(), bound(), cl);
}

bool qdGameObjectAnimated::inters_with_bound(Vect3f bnd, Vect3f cen, 
											 bool perspective_correction) const
{
	Vect3f bnd_a = cen - bnd/2;
	Vect3f bnd_b = cen + bnd/2;
	Vect3f obj_a = R() - bound(perspective_correction)/2;
	Vect3f obj_b = R() + bound(perspective_correction)/2;

	if ((max(bnd_a.x, obj_a.x) <= min(bnd_b.x, obj_b.x)) &&
		(max(bnd_a.y, obj_a.y) <= min(bnd_b.y, obj_b.y)) &&
		(max(bnd_a.z, obj_a.z) <= min(bnd_b.z, obj_b.z)))
		return true;
	else return false;
}

int qdGameObjectAnimated::num_directions() const
{
	if(cur_state_ != -1 && states[cur_state_] -> state_type() == qdGameObjectState::STATE_WALK){
		qdAnimationSet* p = static_cast<qdGameObjectStateWalk*>(states[cur_state_]) -> animation_set();
		if(p)
			return p -> size();
	}

	return 0;
}

bool qdGameObjectAnimated::auto_bound()
{
	if(cur_state_ != -1){
		qdAnimation* ap = NULL;

		if(get_cur_state() -> state_type() == qdGameObjectState::STATE_WALK){
			ap = static_cast<qdGameObjectStateWalk*>(get_cur_state()) -> static_animation(3.0f / 2.0f * M_PI);
		}
		else {
			if(get_cur_state() -> state_type() == qdGameObjectState::STATE_STATIC)
				ap = static_cast<qdGameObjectStateStatic*>(get_cur_state()) -> animation();
		}

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
	}

	return false;
}

bool qdGameObjectAnimated::handle_state_end()
{
	qdGameObjectState* sp = states[cur_state_];

	if(sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE))
		restore_state();

	if(sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_HIDE_OBJECT)){
		if(qdGameDispatcher* gp = qd_get_game_dispatcher()){
			if(gp -> mouse_object() -> object() == this){
				gp -> mouse_object() -> take_object(NULL);
			}
			else
				gp -> remove_from_inventory(this);
		}
		sp -> stop_sound();
		animation_.clear();
		set_flag(QD_OBJ_HIDDEN_FLAG);
	}

	if(sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY) && !sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED)){
		if(qdGameObjectState* p = get_inventory_state()){
			qdGameDispatcher* gp = qd_get_game_dispatcher();
			if(!gp || !gp -> put_to_inventory(this))
				sp -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED);
		}
	}

	if(sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_SOUND_SYNC | qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_SKIP) && !sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE)){
		sp -> stop_sound();
		get_animation() -> stop();
		get_animation() -> set_time_rel(0.0f);
	}

	if(sp -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATE_PERSONAGE)){
		if(owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_SCENE)
			static_cast<qdGameScene*>(owner()) -> set_active_object(this);
	}

	return true;
}

bool qdGameObjectAnimated::update_screen_pos()
{
	if(qdGameObject::update_screen_pos()){
		Vect2i r = get_screen_R();
		Vect2i delta = (cur_state_ != -1) ? states[cur_state_] -> center_offset() : Vect2i(0,0);

		if(current_transform_()){
			if(delta.x || delta.y){
				Vect2f scale(current_transform_.scale());

				delta.x = round(float(delta.x) * scale.x);
				delta.y = round(float(delta.y) * scale.y);

				float angle = current_transform_.angle();

				r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
				r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			}

			set_screen_R(r);
		}
		else
			set_screen_R(r + delta);

		return true;
	}

	return false;
}

void qdGameObjectAnimated::set_default_state()
{
	if(qdGameObjectState* p = get_default_state())
		set_state(p);
	else {
#ifdef _QUEST_EDITOR
		if(!states.empty()){
			set_state((int)0);
			return;
		}
#endif
		set_state(-1);
	}
}

qdGameObjectState* qdGameObjectAnimated::get_default_state()
{
	for(int i = 0; i < states.size(); i ++){
		if(!states[i] -> is_in_triggers() && !states[i] -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
			return states[i];
	}

	return NULL;
}

const qdGameObjectState* qdGameObjectAnimated::get_default_state() const
{
	for(int i = 0; i < states.size(); i ++){
		if(!states[i] -> is_in_triggers() && !states[i] -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
			return states[i];
	}

	return NULL;
}

int qdGameObjectAnimated::get_state_index(const qdGameObjectState* p) const
{
	for(int i = 0; i < states.size(); i ++){
		if(states[i] == p)
			return i;
	}

	return -1;
}

bool qdGameObjectAnimated::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdGameObject::load_data(fh,save_version)) return false;

	fh > cur_state_;

	for(int i = 0; i < states.size(); i ++){
		if(!states[i] -> load_data(fh,save_version))
			return false;
	}

	if(!animation_.load_data(fh,save_version)) return false;

	int idx;
	fh > idx;
	if(idx != -1)
		queued_state_ = get_state(idx);
	else
		queued_state_ = NULL;

	fh > idx;
	if(idx != -1)
		last_state_ = get_state(idx);
	else
		last_state_ = NULL;

	if(save_version >= 103){
		fh > idx;
		if(idx != -1)
			last_inventory_state_ = get_state(idx);
		else
			last_inventory_state_ = NULL;
	}

	fh > inventory_cell_index_;

	fh > last_chg_time_;

	return true;
}

bool qdGameObjectAnimated::save_data(qdSaveStream& fh) const
{
	if(!qdGameObject::save_data(fh)) return false;

	fh < cur_state_;

	for(int i = 0; i < states.size(); i ++){
		if(!states[i] -> save_data(fh))
			return false;
	}

	if(!animation_.save_data(fh)) return false;

	int idx = -1;
	if(queued_state_)
		idx = get_state_index(queued_state_);
	fh < idx;

	idx = -1;
	if(last_state_)
		idx = get_state_index(last_state_);
	fh < idx;

	idx = -1;
	if(last_inventory_state_)
		idx = get_state_index(last_inventory_state_);
	fh < idx;

	fh < inventory_cell_index_;

	fh < last_chg_time_;

	return true;
}
#ifdef _QUEST_EDITOR
bool qdGameObjectAnimated::remove_animation_edges(Vect2i& full_offset, 
												  Vect2i& anim_offset)
{
	if(!qdCamera::current_camera()) return false;

	if(cur_state_ != -1 && states[cur_state_] -> state_type() == qdGameObjectState::STATE_STATIC){
		if(qdAnimationInfo* inf = static_cast<qdGameObjectStateStatic*>(states[cur_state_]) -> animation_info()){
			if(qdAnimation* p = inf -> animation()){
				int const sx = p -> size_x();
				int const sy = p -> size_y();

				Vect2i offs = p -> remove_edges();

				if(p -> qda_file())
					p -> qda_save(p -> qda_file());

				offs.x = offs.x + p -> size_x()/2 - (sx>>1);
				offs.y = offs.y + p -> size_y()/2 - (sy>>1);

				if(inf -> check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
					offs.x = -offs.x;
				if(inf -> check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
					offs.y = -offs.y;

				Vect2i const& state_offset = states[cur_state_]->center_offset();
				anim_offset = offs;
				full_offset = offs + state_offset;
				states[cur_state_]->set_center_offset(full_offset);
				return true;
			}
		}
	}

	return false;
}
#endif//_QUEST_EDITOR

bool qdGameObjectAnimated::is_state_active(const char* state_name) const
{
	if(cur_state_ != -1 && !strcmp(states[cur_state_] -> name(),state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::was_state_previous(const char* state_name) const
{
	if(last_state_ && !strcmp(last_state_ -> name(),state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::is_state_waiting(const char* state_name) const
{
	if(queued_state_ && !stricmp(queued_state_ -> name(),state_name))
		return true;

	return false;
}

bool qdGameObjectAnimated::was_state_active(const char* state_name) const
{
	for(int i = 0; i < states.size(); i++){
		if(!strcmp(states[i] -> name(),state_name))
			return states[i] -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);
	}
	
	return false;
}

bool qdGameObjectAnimated::can_change_state(const qdGameObjectState* state) const
{
	if(cur_state() != -1){
		const qdGameObjectState* p = get_cur_state();
		if(state_status(p) == qdGameObjectAnimated::STATE_ACTIVE){
			if(p -> state_type() == qdGameObjectState::STATE_WALK && p -> coords_animation() -> size() > 1)
				return p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT);
			if(p -> state_type() == qdGameObjectState::STATE_STATIC)
				return p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT);
		}
	}

	return true;
}

void qdGameObjectAnimated::draw_grid_zone(const Vect2s sz) const
{
	const qdCamera* cp = qdCamera::current_camera();
	Vect2s gr = cp -> get_cell_index(grid_r_.x,grid_r_.y);

	if(gr.x == -1) return;
	
	gr.x -= sz.x/2;
	gr.y -= sz.y/2;

	for(int y = 0; y < sz.y; y++){
		for(int x = 0; x < sz.x; x++){
			if(const sGridCell* cell = cp -> get_cell(Vect2s(gr.x + x,gr.y + y))){
				if(!cell -> check_attribute(sGridCell::CELL_IMPASSABLE))
					cp -> draw_cell(gr.x + x,gr.y + y,0,1,0xFFFFFF);
				else
					cp -> draw_cell(gr.x + x,gr.y + y,0,1,0xFF0000);
			}
		}
	}
}

bool qdGameObjectAnimated::set_grid_zone_attributes(const Vect2f& r,int attr) const
{
	if(has_bound() && owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_SCENE){
		qdCamera* cp = static_cast<qdGameScene*>(owner()) -> get_camera();
		Vect2s sr = cp -> get_cell_index(r.x,r.y);

		if(sr.x == -1) return false;

		cp -> set_grid_attributes(sr,grid_size_,attr);
		
		return true;
	}

	return false;
}

bool qdGameObjectAnimated::drop_grid_zone_attributes(const Vect2f& r,int attr) const
{
	if(has_bound() && owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_SCENE){
		qdCamera* cp = static_cast<qdGameScene*>(owner()) -> get_camera();
		Vect2s sr = cp -> get_cell_index(r.x,r.y);
		
		if(sr.x == -1) return false;
		
		cp -> drop_grid_attributes(sr,grid_size_,attr);
		
		return true;
	}
	
	return false;
}

bool qdGameObjectAnimated::check_grid_zone_attributes(const Vect2f& r,int attr) const
{
	if(has_bound() && owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_SCENE){
		qdCamera* cp = static_cast<qdGameScene*>(owner()) -> get_camera();
		Vect2s sr = cp -> get_cell_index(r.x,r.y);
		
		if(sr.x == -1) return false;
		
		return cp -> check_grid_attributes(sr,grid_size_,attr);
	}
	
	return false;
}

bool qdGameObjectAnimated::set_grid_zone_attributes(int attr) const
{
	return set_grid_zone_attributes(grid_r_,attr);
}

bool qdGameObjectAnimated::drop_grid_zone_attributes(int attr) const
{
	return drop_grid_zone_attributes(grid_r_,attr);
}

bool qdGameObjectAnimated::check_grid_zone_attributes(int attr) const
{
	return check_grid_zone_attributes(grid_r_,attr);
}

bool qdGameObjectAnimated::has_camera_mode() const
{
	if(cur_state_ != -1)
		return states[cur_state_] -> has_camera_mode();

	return false;
}

const qdCameraMode& qdGameObjectAnimated::camera_mode() const
{
	if(cur_state_ != -1)
		return states[cur_state_] -> camera_mode();

	static qdCameraMode md;
	return md;
}

bool qdGameObjectAnimated::init()
{
	if(!qdGameObject::init()) return false;

	last_state_ = NULL;

	set_pos(default_R());
	set_cur_state(-1);

	queued_state_ = NULL;
	last_inventory_state_ = NULL;

	for(int i = 0; i < states.size(); i++)
		states[i] -> init();

	return true;
}

grScreenRegion qdGameObjectAnimated::screen_region() const
{
	if(is_visible()){
		grScreenRegion reg = grScreenRegion::EMPTY;
		if(current_transform_()){
			Vect2i r = screen_pos();
/*
			Vect2i delta = (cur_state_ != -1) ? states[cur_state_] -> center_offset() : Vect2i(0,0);
			if(delta.x || delta.y){
				r -= delta;

				Vect2f scale(current_transform_.scale());

				delta.x = round(float(delta.x) * scale.x);
				delta.y = round(float(delta.y) * scale.y);

				float angle = current_transform_.angle();

				r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
				r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			}
*/
			if(const qdAnimationFrame* fp = animation_.get_cur_frame()){
				Vect2f scale(current_transform_.scale());
				Vect2f size(fp->size_x(), fp->size_y());

				float sn = sinf(current_transform_.angle());
				float cs = cosf(current_transform_.angle());

				int sx = round(fabs(cs) * float(size.x) * scale.x + fabs(sn) * float(size.y) * scale.y) + 2;
				int sy = round(fabs(sn) * float(size.x) * scale.x + fabs(cs) * float(size.y) * scale.y) + 2;

				reg = grScreenRegion(0, 0, sx, sy);
				reg.move(r.x, r.y);
			}
		}
		else {
			reg = animation_.screen_region();
			reg.move(screen_pos().x,screen_pos().y);
		}

		return reg;
	}
	else
		return grScreenRegion::EMPTY;
}

void qdGameObjectAnimated::post_redraw()
{
	last_screen_region_ = screen_region();
	last_screen_depth_ = screen_depth();
	last_frame_ = animation_.get_cur_frame();
	last_transform_ = current_transform_;

	lastShadowColor_ = shadow_color();
	lastShadowAlpha_ = shadow_alpha();
}

void qdGameObjectAnimated::set_states_owner()
{
	for(int i = 0; i < max_state(); i++)
		states[i] -> set_owner(this);
}

qdGameObjectState* qdGameObjectAnimated::get_mouse_state()
{
#ifndef _QUEST_EDITOR
	if(states.size()){
		qdGameObjectStateVector::iterator it;
		FOR_EACH(states,it){
			if((*it) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE))
				return *it;
		}
	}
#endif
	return NULL;
}

qdGameObjectState* qdGameObjectAnimated::get_mouse_hover_state()
{
#ifndef _QUEST_EDITOR
	if(states.size()){
		qdGameObjectStateVector::iterator it;
		FOR_EACH(states,it){
			if((*it) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE))
				return *it;
		}
	}
#endif
	return NULL;
}

void qdGameObjectAnimated::draw_shadow(int offs_x,int offs_y,unsigned color,int alpha) const
{
	if(alpha == QD_NO_SHADOW_ALPHA || get_animation() -> is_empty())
		return;

	if(!animation_.is_empty()){
		Vect2i r = screen_pos() + Vect2i(offs_x,offs_y);
		if(current_transform_()){
/*			Vect2i delta = (cur_state_ != -1) ? states[cur_state_] -> center_offset() : Vect2i(0,0);
			if(delta.x || delta.y){
				r -= delta;

				Vect2f scale(current_transform_.scale());

				delta.x = round(float(delta.x) * scale.x);
				delta.y = round(float(delta.y) * scale.y);

				float angle = current_transform_.angle();

				r.x += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
				r.y += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
			}
*/
			if(current_transform_.has_scale())
				animation_.draw_mask_rot(r.x,r.y,screen_depth(),current_transform_.angle(),grDispatcher::instance() -> make_rgb(color),alpha,current_transform_.scale());
			else
				animation_.draw_mask_rot(r.x,r.y,screen_depth(),current_transform_.angle(),grDispatcher::instance() -> make_rgb(color),alpha);
		}
		else
			animation_.draw_mask(r.x,r.y,screen_depth(),grDispatcher::instance() -> make_rgb(color),alpha);
	}
}

bool qdGameObjectAnimated::get_debug_info(XBuffer& buf) const
{
	qdGameObject::get_debug_info(buf);
#ifdef __QD_DEBUG_ENABLE__
	if(const qdGameObjectState* p = get_cur_state()){
		buf < p -> name() < " " <= animation_.cur_time() < "\n";
		
		if(queued_state())
			buf < "wait: " < queued_state() -> name() < "\n";
	}
#endif
	return true;
}
