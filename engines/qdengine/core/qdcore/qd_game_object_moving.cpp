/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_rnd.h"
#include "gr_dispatcher.h"
#include "xml_tag_buffer.h"

#include "qd_setup.h"
#include "qd_sound.h"
#include "qdscr_parser.h"
#include "qd_game_object_moving.h"
#include "qd_game_scene.h"
#include "qd_grid_zone.h"
#include "qd_animation_set.h"
#include "qd_game_dispatcher.h"
#include "qd_interface_button.h"

#include "AIAStar_API.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const int EIGHT_DIRS_OPT_ITER_MAX = 10;    // Макс. кол-во итераций спрямления в
										  // optimize_path_eight_dirs

qdGameObjectMoving::qdGameObjectMoving() : 
	collision_radius_(0.0f),
	collision_delay_(0.0f),
	collision_path_(20.0f),
	follow_min_radius_(0.0f),
	follow_max_radius_(0.0f),
	follow_condition_(FOLLOW_DONE),
	attacher_(NULL),
	attach_shift_(Vect2s(0,0)),
	disable_control_(false),
	scale_(1.0f),
	direction_angle_(0.0f),
	default_direction_angle_(0.0f),
	rotation_angle_(0.0f),
	rotation_angle_per_quant_(1.0f),
	last_move_order_(Vect3f::ZERO),
	target_r_(0,0,0),
	path_length_(0),
	cur_path_index_(0),
#ifndef _QUEST_EDITOR
	walk_grid_size_(0,0),
	last_walk_state_(NULL),
#endif
	impulse_movement_mode_(false),
	speed_delta_(0.0f),
	target_angle_(-1.0f),
	impulse_timer_(0.0f),
	impulse_start_timer_(0.0f),
	impulse_direction_(-1.0f),
	control_types_(CONTROL_MOUSE),
	button_(NULL)
{
	ignore_personages_ = false;
	is_selected_ = false;
	set_flag(QD_OBJ_HAS_BOUND_FLAG);
	movement_mode_ = MOVEMENT_MODE_STOP;
	movement_mode_time_ = movement_mode_time_current_ = 0.f;
}

qdGameObjectMoving::qdGameObjectMoving(const qdGameObjectMoving& obj) : qdGameObjectAnimated(obj),
    follow_min_radius_(obj.follow_min_radius()),
	follow_max_radius_(obj.follow_min_radius()),
	follow_condition_(obj.follow_condition()),
	attacher_(obj.attacher()),
	attacher_ref_(obj.attacher_ref()),
	attach_shift_(obj.attach_shift()),
	disable_control_(false),
	scale_(obj.scale_),
	direction_angle_(obj.direction_angle_),
	default_direction_angle_(obj.default_direction_angle_),
	rotation_angle_(obj.rotation_angle()),
	rotation_angle_per_quant_(obj.rotation_angle_per_quant()),
	last_move_order_(Vect3f::ZERO),
	target_r_(0,0,0),
	path_length_(0),
	cur_path_index_(0),
#ifndef _QUEST_EDITOR
	walk_grid_size_(obj.walk_grid_size_),
	last_walk_state_(NULL),
#endif
	impulse_movement_mode_(false),
	speed_delta_(0.0f),
	target_angle_(-1.0f),
	impulse_timer_(0.0f),
	impulse_start_timer_(0.0f),
	impulse_direction_(-1.0f),
	control_types_(obj.control_types_),
	button_(NULL)
{
	ignore_personages_ = false;
	is_selected_ = false;
	set_flag(QD_OBJ_HAS_BOUND_FLAG);

	movement_mode_ = MOVEMENT_MODE_STOP;
	movement_mode_time_ = movement_mode_time_current_ = 0.f;
    
	circuit_objs_ = obj.const_ref_circuit_objs();
}

#ifdef _QUEST_EDITOR
qdGameObjectMoving::qdGameObjectMoving(const qdGameObjectAnimated& obj)
:	qdGameObjectAnimated(obj),
	follow_min_radius_(0.0f),
	follow_max_radius_(0.0f),
	follow_condition_(FOLLOW_DONE),
	attacher_(NULL),
	attach_shift_(0,0),
	disable_control_(false),
	scale_(1.0f),
	direction_angle_(0.0f),
	default_direction_angle_(0.0f),
	rotation_angle_(0.0f),
	rotation_angle_per_quant_(1.0f),
	last_move_order_(Vect3f::ZERO),
	target_r_(0,0,0),
	path_length_(0),
	cur_path_index_(0),
	impulse_movement_mode_(false),
	speed_delta_(0.0f),
	target_angle_(-1.0f),
	impulse_timer_(0.0f),
	impulse_start_timer_(0.0f),
	impulse_direction_(-1.0f),
	control_types_(CONTROL_MOUSE),
	button_(NULL)
{
	ignore_personages_ = false;
	is_selected_ = false;
	set_flag(QD_OBJ_HAS_BOUND_FLAG);
}
#endif // _QUEST_EDITOR

qdGameObjectMoving::~qdGameObjectMoving()
{
}

qdGameObjectMoving& qdGameObjectMoving::operator = (const qdGameObjectMoving& obj)
{
	if(this == &obj) return *this;

	*static_cast<qdGameObjectAnimated*>(this) = obj;

	follow_min_radius_ = obj.follow_min_radius();
	follow_max_radius_ = obj.follow_max_radius();
	follow_condition_ = obj.follow_condition();

	circuit_objs_ = obj.const_ref_circuit_objs();

	attacher_ = obj.attacher();
	attacher_ref_ = obj.attacher_ref();
	attach_shift_ = obj.attach_shift();

	scale_ = obj.scale_;
	direction_angle_ = obj.direction_angle_;
	default_direction_angle_ = obj.default_direction_angle_;
	rotation_angle_ = obj.rotation_angle();
	rotation_angle_per_quant_ = obj.rotation_angle_per_quant();
	speed_delta_ = 0.0f;

	last_move_order_ = obj.last_move_order();

#ifndef _QUEST_EDITOR
	walk_grid_size_ = obj.walk_grid_size_;
#endif

	movement_mode_ = obj.movement_mode_;
	movement_mode_time_ = obj.movement_mode_time_;
	movement_mode_time_current_ = obj.movement_mode_time_current_;
    
	return *this;
}

bool qdGameObjectMoving::load_script_body(const xml::tag* p)
{
	qdGameObjectAnimated::load_script_body(p);

	xml::tag::subtag_iterator is;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_OBJECT_MOVEMENT_STATES:
			for(is = it -> subtags_begin(); is != it -> subtags_end(); ++is){
				qdGameObjectStateWalk* p = new qdGameObjectStateWalk;
				p -> load_script(&*is);
				add_state(p);
			}
			break;
		case QDSCR_OBJECT_DIRECTION:
			xml::tag_buffer(*it) > direction_angle_;
			break;
		case QDSCR_OBJECT_DEFAULT_DIRECTION:
			xml::tag_buffer(*it) > direction_angle_;
			default_direction_angle_ = direction_angle_;
			break;
		case QDSCR_ROTATION_ANGLE_PER_QUANT:
			xml::tag_buffer(*it) > rotation_angle_per_quant_;
			break;
		case QDSCR_COLLISION_DELAY:
			xml::tag_buffer(*it) > collision_delay_;
			break;
		case QDSCR_COLLISION_RADIUS:
			xml::tag_buffer(*it) > collision_radius_;
			break;
		case QDSCR_COLLISION_PATH:
			xml::tag_buffer(*it) > collision_path_;
			break;
		case QDSCR_FOLLOW_MIN_RADIUS:
			xml::tag_buffer(*it) > follow_min_radius_;
			break;
		case QDSCR_FOLLOW_MAX_RADIUS:
			xml::tag_buffer(*it) > follow_max_radius_;
			break;
		case QDSCR_NAMED_OBJECT:
			attacher_ref_.load_script(&*it);
			break;
		case QDSCR_ATTACH_SHIFT:
			xml::tag_buffer(*it) > attach_shift_.x > attach_shift_.y;
			break;
		case QDSCR_PERSONAGE_CONTROL:
			xml::tag_buffer(*it) > control_types_;
			break;
		}
	}

	return true;
}

bool qdGameObjectMoving::save_script_body(XStream& fh,int indent) const
{
	qdGameObjectAnimated::save_script_body(fh,indent);

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<object_direction>" <= direction_angle_ < "</object_direction>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<default_direction>" <= default_direction_angle_ < "</default_direction>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<rotation_angle_per_quant>" <= rotation_angle_per_quant_ < "</rotation_angle_per_quant>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<collision_path>" <= collision_path_ < "</collision_path>\r\n";

	if(collision_radius_ > FLT_EPS){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<collision_radius>" <= collision_radius_ < "</collision_radius>\r\n";
	}

	if(collision_delay_ > FLT_EPS){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<collision_delay>" <= collision_delay_ < "</collision_delay>\r\n";
	}

	if(follow_min_radius_ > FLT_EPS){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<follow_min_radius>" <= follow_min_radius_ < "</follow_min_radius>\r\n";
	}

	if(follow_max_radius_ > FLT_EPS){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<follow_max_radius>" <= follow_max_radius_ < "</follow_max_radius>\r\n";
	}

	if (NULL != attacher_)
		attacher_ref_.save_script(fh,indent + 1);
	
	if ((0 != attach_shift_.x) || (0 != attach_shift_.y))
	{
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<attach_shift>" <= attach_shift_.x < " " <= attach_shift_.y < "</attach_shift>\r\n";
	}

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<control>" <= control_types_ < "</control>\r\n";

	return true;
}

bool qdGameObjectMoving::load_script(const xml::tag* p)
{
	return load_script_body(p);
}

bool qdGameObjectMoving::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<moving_object name=\"" < qdscr_XML_string(name()) < "\">\r\n";

	save_script_body(fh,indent);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</moving_object>\r\n";

	return true;
}

bool qdGameObjectMoving::move(const Vect3f& target,bool lock_target)
{
	set_last_move_order(target);
	if (false == enough_far_target(target))  
		return true;
#ifndef _QUEST_EDITOR
	if(is_selected_ && has_control_type(CONTROL_CLEAR_PATH)){
		if(!find_path(target,true)){
			toggle_ignore_personages(true);
			bool ret = find_path(target,true);
			toggle_ignore_personages(false);
			if(ret)	
				return true;

			if(lock_target) 
				return false;
		}
	}
#endif
	switch(movement_mode_){
	case MOVEMENT_MODE_STOP:
	case MOVEMENT_MODE_END:
		movement_mode_ = MOVEMENT_MODE_TURN;
		break;
	}
	return find_path(target,lock_target);
}

bool qdGameObjectMoving::move(const Vect3f& target,float angle,bool lock_target)
{
	if(move(target,lock_target)){
		target_angle_ = angle;
		return true;
	}

	return false;
}

bool qdGameObjectMoving::enough_far_target(const Vect3f& dest) const
{
	Vect3f del = dest-R();
	if (del.norm2() < 0.5f) return false;
	else return true;
}

template<class V>
void dump_vect(const V& vect)
{
	appLog::default_log() << " -------------------- \r\n";

	appLog::default_log() << vect.size() << "\r\n";

	for(int i = 0; i < vect.size(); i++){
		appLog::default_log() << vect[i].x << " " << vect[i].y << "\r\n";
	}

	appLog::default_log() << " -------------------- \r\n";
}

bool qdGameObjectMoving::find_path(const Vect3f target,bool lock_target)
{
	Vect3f trg = target;

	if(!adjust_position(trg))
		return false;

	set_grid_zone_attributes(sGridCell::CELL_SELECTED);

	target_angle_ = -1.0f;

	if(!is_walkable(trg)){
		if(lock_target || check_grid_zone_attributes(sGridCell::CELL_IMPASSABLE)) return false;
		
		Vect2s pt;
		if(allowed_directions_count() <= 2)
			pt = get_nearest_walkable_point(qdCamera::current_camera() -> get_cell_index(trg.x,trg.y,false));
		else
			// Для движения с двумя степенями свободы смотрим последюнюю доступную
			// потому как в случае неудачи мы все равно проверим все подходящие нам для подхода.
			// Но зато получим выигрышь в оптимальности нахождения максимально близкого пути.
			pt = get_pre_last_walkable_point(qdCamera::current_camera() -> get_cell_index(trg.x,trg.y,false));
		if(pt.x == -1){
			drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
			return false;
		}

		target_angle_ = calc_direction_angle(target);
		trg = qdCamera::current_camera() -> get_cell_coords(pt.x,pt.y);
	}

	if(allowed_directions_count() <= 2){
		if(is_path_walkable(R(),trg)){
			path_length_ = 0;
			move2position(trg);

			drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
			return true;
		}
		else
			return false;
	}

	Vect2s cell_idx = qdCamera::current_camera() -> get_cell_index(R().x,R().y);
	if(cell_idx.x == -1){
		drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
		return false;
	}

	qdHeuristic phobj;
	phobj.set_camera(qdCamera::current_camera());
	phobj.set_object(this);
	phobj.init(trg);

	qdAStar pfobj;
	pfobj.Init(qdCamera::current_camera() -> get_grid_sx(),qdCamera::current_camera() -> get_grid_sy());

	int dirs_count = (allowed_directions_count() > 4) ? 8 : 4;

	std::vector<Vect2i> path_vect;
	pfobj.FindPath(cell_idx,&phobj,path_vect,dirs_count);

	int idx = 0;
	bool correct = true;
	for(std::vector<Vect2i>::const_iterator it = path_vect.begin(); it != path_vect.end(); ++it){
		if(!is_walkable(Vect2s(it -> x,it -> y))){
			correct = false;
			break;
		}

		idx ++;
	}
	if (0 == idx) correct = false;
	
	// Если нужно - пытаемся считать путь еще раз для ближайшей конечной точки
	while ((false == lock_target) && (false == correct))
	{
		// Пересчитываем конечную точку
		Vect2s pt = get_pre_last_walkable_point(qdCamera::current_camera() -> get_cell_index(trg.x,trg.y,false));
		if(pt.x == -1){
			drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
			return false;
		}
		target_angle_ = calc_direction_angle(target);
		trg = qdCamera::current_camera() -> get_cell_coords(pt.x,pt.y);
		
		// Считаем путь с новым концом
		phobj.init(trg);
		pfobj.FindPath(cell_idx,&phobj,path_vect,dirs_count);

		// Проверяем путь на проходимость
		correct = true;
		idx = 0;
		for(std::vector<Vect2i>::const_iterator it = path_vect.begin(); it != path_vect.end(); ++it){
			if(!is_walkable(Vect2s(it -> x,it -> y))){
				drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
				correct = false;
				break;
			}
			idx ++;
		}
		if (0 == idx) correct = false;
	}

	// Окончательно утверждаем путь
	if((false == correct) || (idx > QD_MOVING_OBJ_PATH_LENGTH) || !idx){
		drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
		return false;
	}

	__QDBG(appLog::default_log() << "найденный путь" << "\r\n");
	__QDBG(dump_vect(path_vect));

	optimize_path(path_vect);

	__QDBG(appLog::default_log() << "оптимизированный путь" << "\r\n");
	__QDBG(dump_vect(path_vect));

	if(path_vect.size() >= 2 && movement_type() == qdGameObjectStateWalk::MOVEMENT_FOUR_DIRS || movement_type() == qdGameObjectStateWalk::MOVEMENT_EIGHT_DIRS){
		std::vector<Vect3f> final_path;
		finalize_path(R(), trg, path_vect, final_path);

		for(int i = 0; i < final_path.size(); i++)
			path_[i] = final_path[i];

		idx = final_path.size();

		__QDBG(appLog::default_log() << "финальный путь" << "\r\n");
		__QDBG(dump_vect(final_path));
	}
	else {
		idx = 0;
		for(std::vector<Vect2i>::const_iterator it = path_vect.begin(); it != path_vect.end(); ++it){
			path_[idx] = qdCamera::current_camera() -> get_cell_coords(it -> x,it -> y);
			idx ++;
		}
		path_[idx - 1] = trg;
	}

	cur_path_index_ = (idx > 1) ? 1 : 0;
	path_length_ = idx;
	move2position(path_[cur_path_index_++]);

	if(cur_path_index_ >= path_length_)
		path_length_ = 0;

	drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
	return true;
}


bool qdGameObjectMoving::stop_movement()
{
	if(check_flag(QD_OBJ_MOVING_FLAG)){
		drop_flag(QD_OBJ_MOVING_FLAG);

		if(cur_state() == -1) return true;

		qdGameObjectState* st = get_state(cur_state());
//		if(movement_mode_ == MOVEMENT_MODE_MOVE && is_movement_finished() && st -> state_type() == qdGameObjectState::STATE_WALK){
		if(is_movement_finished() && st -> state_type() == qdGameObjectState::STATE_WALK){
			qdGameObjectStateWalk* wst = static_cast<qdGameObjectStateWalk*>(st);

			if(qdAnimationSet* set = wst->animation_set()){
				qdAnimationInfo* inf = set->get_stop_animation_info(direction_angle_);
				if(qdAnimation* anm = inf->animation()){
					movement_mode_ = MOVEMENT_MODE_END;
					float phase = get_animation()->cur_time_rel();
					movement_mode_time_ = anm->length() * (1.f - phase);
					movement_mode_time_current_ = 0.0f;
					set_animation_info(inf);
					get_animation() -> set_time_rel(phase);
					return true;
				}
			}

			set_direction(direction_angle_);
			st -> stop_sound();
		}

		return true;
	}

	return false;
}

bool qdGameObjectMoving::move2position(const Vect3f target)
{
	float tm = 0.0f;

	change_direction_angle(calc_direction_angle(target));
	target_r_ = target;

	return set_walk_animation();
}

bool qdGameObjectMoving::skip_movement()
{
	if(!check_flag(QD_OBJ_MOVING_FLAG)) return false;

	bool flag = true;

	do {
		flag = false;
		float sp = speed();

		if(sp > FLT_EPS){
			Vect3f dr = target_r_ - R();
			dr.z = 0.0f;
			float dist = dr.norm();
			float time = dist / sp;
			get_animation() -> advance_time(time);
		}

		target_r_.z = R().z;
		set_pos(target_r_);
		adjust_z();

		if(path_length_){
			move2position(path_[cur_path_index_ ++]);
			if(cur_path_index_ >= path_length_)
			path_length_ = 0;
			flag = true;
		}
	}
	while(path_length_ || flag);

	return true;
}

float qdGameObjectMoving::calc_direction_angle(const Vect3f& target) const
{
	Vect3f dr = target - R();
	dr.z = 0.0f;
	// Точки практически совпадают - угол неизменен
	if(dr.norm2() <= 0.01f) return direction_angle_;

	float angle = dr.psi() + qdCamera::current_camera() -> get_z_angle() * M_PI / 180.0f;

	if(fabs(angle) >= M_PI * 2.0f) angle = fmodf(angle,M_PI * 2.0f);
	if(angle < 0.0f) angle += M_PI * 2.0f;

	return angle;
}

float qdGameObjectMoving::animate_rotation(float dt)
{
	 // Второе значение - на сколько повернуться за квант
	float work_dt = fabs(rotation_angle_/rotation_angle_per_quant());
	if (work_dt <= FLT_EPS) return dt;   // Поворачиваться не нужно
	// Считаем на сколько можем повернуться и сколько после этого останется квантов
	if (work_dt > dt)
	{
		work_dt = dt;
		dt = 0;
	}
	else
		dt = dt - work_dt;

	float delta = (rotation_angle_ < 0) ? -work_dt*rotation_angle_per_quant() : work_dt*rotation_angle_per_quant();
	// Поворачиваемся
	direction_angle_ += delta;
	rotation_angle_ -= delta; // Уже меньше нужно поворачиваться

	// Циклим угол. Иначе могут происходить накрутки из-за rotation_angle_, 
	// установленного постоянно в одном (скажем положительном) направлении
	while (direction_angle_ > 2*M_PI) direction_angle_ -= 2*M_PI;
    while (direction_angle_ < 0.0f) direction_angle_ += 2*M_PI;

	set_walk_animation();
	return dt;
}

Vect3f qdGameObjectMoving::get_future_r(float dt, bool& end_movement, bool real_moving)
{
	if(!check_flag(QD_OBJ_MOVING_FLAG)){
		end_movement = true;
		return R();
	}

	qdGameObjectState* st;

	switch(movement_mode_){
	case MOVEMENT_MODE_START:
		if(real_moving){
			movement_mode_time_current_ += dt;
			if(movement_mode_time_current_ >= movement_mode_time_){
				movement_mode_ = MOVEMENT_MODE_MOVE;
				set_walk_animation();
				get_animation() -> set_time_rel(0.f);
			}
		}
		return R();
	case MOVEMENT_MODE_TURN:
		if(has_control_type(CONTROL_ANIMATED_ROTATION)){
			if(real_moving)
				dt = animate_rotation(dt);
			// Считаем не для настоящего движения - после просчета восстанавл. направления
			else
			{
				float dir_buf = direction_angle_;
				float rot_buf = rotation_angle_;
				dt = animate_rotation(dt);
				direction_angle_ = dir_buf;
				rotation_angle_ = rot_buf;
			}
			// Не осталось квантов на движение. Возвращаем текущую позицию
			end_movement = false;
			if(dt <= FLT_EPS)
				return R();
		}

		st = get_state(cur_state());
		if(st && st -> state_type() == qdGameObjectState::STATE_WALK){
			qdGameObjectStateWalk* wst = static_cast<qdGameObjectStateWalk*>(st);

			if(qdAnimationSet* set = wst->animation_set()){
				qdAnimationInfo* inf = set->get_start_animation_info(direction_angle_);
				if(qdAnimation* anm = inf->animation()){
					if(real_moving){
						movement_mode_ = MOVEMENT_MODE_START;
						movement_mode_time_ = anm->length();
						movement_mode_time_current_ = 0.0f;
						set_walk_animation();
						get_animation() -> set_time_rel(0.f);
					}
					return R();
				}
			}
		}

		if(real_moving){
			movement_mode_ = MOVEMENT_MODE_MOVE;
			set_walk_animation();
			get_animation() -> set_time_rel(0.f);
		}
		return R();
	case MOVEMENT_MODE_MOVE:
		if(has_control_type(CONTROL_ANIMATED_ROTATION)){
			if(real_moving)
				dt = animate_rotation(dt);
			// Считаем не для настоящего движения - после просчета восстанавл. направления
			else
			{
				float dir_buf = direction_angle_;
				float rot_buf = rotation_angle_;
				dt = animate_rotation(dt);
				direction_angle_ = dir_buf;
				rotation_angle_ = rot_buf;
			}
			// Не осталось квантов на движение. Возвращаем текущую позицию
			end_movement = false;
			if(dt <= FLT_EPS) return R(); 
		}
		break;
	case MOVEMENT_MODE_END:
		if(real_moving){
			movement_mode_time_current_ += dt;
			if(movement_mode_time_current_ >= movement_mode_time_){
				movement_mode_ = MOVEMENT_MODE_STOP;
				end_movement = true;
			}
		}
		return R();
	}

	movement_mode_ = MOVEMENT_MODE_MOVE;

	float sp,a,sp_max;
	get_speed_parameters(sp,sp_max,a);

	Vect3f r = R();
	if(sp > FLT_EPS){
		end_movement = false;

		if(!impulse_movement_mode_){ 
			speed_delta_ += a * dt;
			sp += speed_delta_;
			if (false == real_moving)
				speed_delta_ -= a * dt;

			if(sp_max > FLT_EPS && sp > sp_max)
				sp = sp_max;

			if(sp < 0.0f){
				sp = 0.01f;
				end_movement = true;
			}

			float dist = sp * dt;

			Vect3f dr = target_r_ - R();
			dr.z = 0.0f;

			if(dr.norm2() <= dist * dist){
				r = target_r_;
				r.z = R().z;
				end_movement = true;
			}
			else {
				dr.normalize(dist);
				r = R() + dr;
			}
		}
		else 
			if(impulse_timer_ > FLT_EPS || has_control_type(CONTROL_AUTO_MOVE)){
				float time = dt;
				if(!has_control_type(CONTROL_AUTO_MOVE)){
					if(impulse_timer_ < dt){
						time = impulse_timer_;
						if (real_moving)
							impulse_timer_ = 0.0f;
						end_movement = true;
					}
					else
						if (real_moving) 
							impulse_timer_ -= dt;
				}

				speed_delta_ += a * time;
				sp += speed_delta_;
				if (false == real_moving)
					speed_delta_ -= a * time;

				if(sp_max > FLT_EPS && sp > sp_max)
					sp = sp_max;

				if(sp < 0.0f){
					sp = 0.01f;
					end_movement = true;
				}

				float dist = sp * time;
				float angle = direction_angle_ + qdCamera::current_camera() -> get_z_angle() * M_PI / 180.0f;
				r.x += dist * cos(angle);
				r.y += dist * sin(angle);
		
				set_grid_zone_attributes(sGridCell::CELL_SELECTED);

				if(!is_walkable(r)){
					r = R();
					end_movement = true;
				}

				drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
			}
			else
				end_movement = true;
	}
	// sp <= 0 - нет скорости объекта
	else
		end_movement = true;

	return r;
}

bool qdGameObjectMoving::future_pos_correct(float dt)
{
	// Считаем текущую и последующую позицию на сетке
	Vect2s cur_cen, cur_size, next_cen, next_size;
	calc_cur_and_future_walk_grid(dt, cur_cen, cur_size, next_cen, next_size);
	
	// Не будем учитывать заняые сейчас персонажем ячейки
	qdCamera::current_camera()->set_grid_attributes(
		cur_cen,
		cur_size,
		sGridCell::CELL_SELECTED);
	
	int incorr_num = qdCamera::current_camera()->cells_num_with_exact_attributes(next_cen, next_size, sGridCell::CELL_IMPASSABLE);
	bool all_ok;
	if ((incorr_num >= 1 && (next_size.x <= 1 || next_size.y <= 1)) || 
		(incorr_num > max(next_size.x,next_size.y)))
		all_ok = false;
	else
		all_ok = true;
	
	qdCamera::current_camera()->drop_grid_attributes(
		cur_cen,
		cur_size,
		sGridCell::CELL_SELECTED);
	
	return all_ok;	
}

bool qdGameObjectMoving::is_movement_finished() const
{
	return (!path_length_ && (cur_state() == -1 || !get_cur_state()->coords_animation()->is_playing()));
}

void qdGameObjectMoving::quant(float dt)
{
	Vect3f beg_r = R();

	if(can_change_state())
		enable_control();

	if(impulse_start_timer_ > FLT_EPS){
		impulse_start_timer_ -= dt;
		if(impulse_start_timer_ <= 0.0f)
			movement_impulse();
	}

#ifndef _QUEST_EDITOR
	if(has_control_type(CONTROL_AUTO_MOVE))
		start_auto_move();
#endif

	if(check_flag(QD_OBJ_MOVING_FLAG)){		
		if(future_pos_correct(dt)){
			bool end_movement = false;
			Vect3f r = get_future_r(dt, end_movement, true);

			set_pos(r);

			if(end_movement){
				impulse_movement_mode_ = false;
				speed_delta_ = 0.0f;

				if(path_length_){
					move2position(path_[cur_path_index_ ++]);
					if(cur_path_index_ >= path_length_)
					path_length_ = 0;
				}
				else {
					if(target_angle_ >= 0.0f)
						direction_angle_ = target_angle_;

					stop_movement();
/*
					drop_flag(QD_OBJ_MOVING_FLAG);

					set_direction(direction_angle_);
		
					if(get_cur_state())
						get_cur_state() -> stop_sound();

					if(is_movement_finished())
						movement_mode_ = MOVEMENT_MODE_STOP;
*/
				}
			}
		}
		else
			stop_movement();
	}
	else {
		speed_delta_ = 0.0f;

		if(movement_mode_ == MOVEMENT_MODE_END){
			movement_mode_time_current_ += dt;
			if(movement_mode_time_current_ >= movement_mode_time_){
				movement_mode_ = MOVEMENT_MODE_STOP;
				movement_mode_time_current_ = movement_mode_time_ = 0.f;
				set_direction(direction_angle_);
				if(get_cur_state())
					get_cur_state() -> stop_sound();
			}
		}
		else if(movement_mode_ == MOVEMENT_MODE_MOVE){
			movement_mode_ = MOVEMENT_MODE_STOP;
			movement_mode_time_current_ = movement_mode_time_ = 0.f;
		}

//		movement_mode_ = MOVEMENT_MODE_TURN;
	}

	qdGameObjectAnimated::quant(dt);

	adjust_z();

	// Если текущ. позиция не соответствует той, что была в начале кванта, то
	// объект изменился
	if (R() != beg_r)
		set_last_chg_time(qdGameDispatcher::get_dispatcher()->time());
}

bool qdGameObjectMoving::load_resources()
{
	float sc = 1.0f;
	
	if(qdGameDispatcher* dsp = qd_get_game_dispatcher())
		dsp -> get_object_scale(name(),sc);

	set_scale(sc);

	qdGameObjectAnimated::load_resources();

	button_ = NULL;

	return true;
}

bool qdGameObjectMoving::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	if(!check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
		qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
		if(dp && !dp -> check_flag(qdGameDispatcher::OBJECT_CLICK_FLAG | qdGameDispatcher::DIALOG_CLICK_FLAG)){
			if(owner() && static_cast<qdGameScene*>(owner()) -> get_active_personage() != this){
				static_cast<qdGameScene*>(owner()) -> set_active_personage(this);
				return true;
			}
		}
	}

	return false;
}

bool qdGameObjectMoving::is_walkable(const Vect3f& pos) const
{
	Vect2s v = qdCamera::current_camera() -> get_cell_index(pos.x,pos.y);
	return is_walkable(v);
}

bool qdGameObjectMoving::is_walkable(const Vect2s& pos) const
{
	Vect2s size = walk_grid_size(pos);

#ifdef _QUEST_EDITOR
	return qdCamera::current_camera() -> is_walkable(pos,size);
#else
	return qdCamera::current_camera() -> is_walkable(pos,size,ignore_personages_);
#endif
}

bool qdGameObjectMoving::is_path_walkable(const Vect3f& src,const Vect3f& trg) const
{
	Vect2s src_ = qdCamera::current_camera() -> get_cell_index(src.x,src.y);
	Vect2s trg_ = qdCamera::current_camera() -> get_cell_index(trg.x,trg.y);

	if(src_.x == -1 || trg_.x == -1) return false;

	return is_path_walkable(src_.x,src_.y,trg_.x,trg_.y);
}

void qdGameObjectMoving::set_path_attributes(int attr) const
{
	if(check_flag(QD_OBJ_MOVING_FLAG)){
		qdCamera* cp = qdCamera::current_camera();
		assert(cp);

		Vect2s v0 = cp -> get_cell_index(R().x,R().y);
		Vect2s v1 = cp -> get_cell_index(target_r_.x,target_r_.y);

		cp -> set_grid_line_attributes(v0,v1,grid_size(),attr);

		v0 = v1;

		for(int i = cur_path_index_; i < path_length_; i ++){
			v1 = cp -> get_cell_index(path_[i].x,path_[i].y);
			cp -> set_grid_line_attributes(v0,v1,grid_size(),attr);
			v0 = v1;
		}
	}
}

void qdGameObjectMoving::clear_path_attributes(int attr) const
{
	if(check_flag(QD_OBJ_MOVING_FLAG)){
		qdCamera* cp = qdCamera::current_camera();
		assert(cp);

		Vect2s v0 = cp -> get_cell_index(R().x,R().y);
		Vect2s v1 = cp -> get_cell_index(target_r_.x,target_r_.y);

		cp -> drop_grid_line_attributes(v0,v1,grid_size(),attr);

		v0 = v1;

		for(int i = cur_path_index_; i < path_length_; i ++){
			v1 = cp -> get_cell_index(path_[i].x,path_[i].y);
			cp -> drop_grid_line_attributes(v0,v1,grid_size(),attr);
			v0 = v1;
		}
	}
}

bool qdGameObjectMoving::is_path_walkable(int x1,int y1,int x2,int y2) const
{
	if(x1 == x2 && y1 == y2){
		if(!is_walkable(Vect2s(x1,y1))) 
			return false;

		return true;
	}

	Vect2f r(x1,y1);

	Vect2f dr(x2 - x1,y2 - y1);
	dr.normalize(0.2f);

	if(abs(x2 - x1) > abs(y2 - y1)){
		int dx = round(float(x2 - x1)/dr.x);
		do {
			if(!is_walkable(Vect2s(r.xi(),r.yi()))) 
				return false;

			r += dr;
		} while(--dx >= 0);
	}
	else {
		int dy = round(float(y2 - y1)/dr.y);
		do {
			if(!is_walkable(Vect2s(r.xi(),r.yi()))) 
				return false;
			
			r += dr;
		} while(--dy >= 0);
	}

	return true;
}

bool qdGameObjectMoving::update_screen_pos()
{
	if(qdGameObject::update_screen_pos()){
		if(cur_state() != -1){
			Vect2s offs = get_cur_state() -> center_offset();

			if(get_cur_state() -> state_type() == qdGameObjectState::STATE_WALK){
				qdGameObjectStateWalk::OffsetType offs_type = qdGameObjectStateWalk::OFFSET_WALK;
				switch(movement_mode_){
				case MOVEMENT_MODE_STOP:
					offs_type = qdGameObjectStateWalk::OFFSET_STATIC;
					break;
				case MOVEMENT_MODE_TURN:
					offs_type = qdGameObjectStateWalk::OFFSET_STATIC;
					break;
				case MOVEMENT_MODE_START:
					offs_type = qdGameObjectStateWalk::OFFSET_START;
					break;
				case MOVEMENT_MODE_MOVE:
					offs_type = qdGameObjectStateWalk::OFFSET_WALK;
					break;
				case MOVEMENT_MODE_END:
					offs_type = qdGameObjectStateWalk::OFFSET_END;
					break;
				}

				offs += static_cast<qdGameObjectStateWalk*>(get_cur_state()) -> center_offset(direction_angle_, offs_type);
			}

			if(offs.x || offs.y){
				float scale = calc_scale();
				offs.x = round(float(offs.x) * scale);
				offs.y = round(float(offs.y) * scale);

				set_screen_R(get_screen_R() + offs);
			}
		}

		return true;
	}

	return false;
}

Vect2s qdGameObjectMoving::screen_size() const
{
	float scale = calc_scale();

	return Vect2s(round(static_cast<float>(get_animation() -> size_x()) * scale),round(static_cast<float>(get_animation() -> size_y()) * scale));
}

float qdGameObjectMoving::radius() const
{
	return qdGameObjectAnimated::radius() * calc_scale();
}

void qdGameObjectMoving::debug_redraw() const
{
	if(check_flag(QD_OBJ_MOVING_FLAG)){
		const cl = grDispatcher::instance()->make_rgb(255, 255, 255);
		Vect3f r = R();
		r.z = 0;
		Vect2s v0 = qdCamera::current_camera() -> global2scr(r);
		Vect2s v1 = qdCamera::current_camera() -> global2scr(target_r_);
		grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,cl,2);
		v0 = v1;

		for(int i = cur_path_index_; i < path_length_; i ++){
			v1 = qdCamera::current_camera() -> global2scr(path_[i]);
			grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,cl,2);
			v0 = v1;
		}
	}

	XBuffer str;
	str < "movement_mode: " <= (int)movement_mode_;
	grDispatcher::instance() -> DrawText(10,110,grDispatcher::instance() -> make_rgb888(255,255,255),str);

//	draw_bound();

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

	draw_grid_zone(walk_grid_size());
/*
	if(!get_animation() -> is_empty()){
		const cl = 0xFFFFFF;
		Vect2s v = screen_pos();
		Vect2s sz = screen_size();

		grDispatcher::instance() -> Line(v.x - sz.x/2,v.y - sz.y/2,v.x + sz.x/2,v.y - sz.y/2,cl);
		grDispatcher::instance() -> Line(v.x - sz.x/2,v.y + sz.y/2,v.x + sz.x/2,v.y + sz.y/2,cl);

		grDispatcher::instance() -> Line(v.x - sz.x/2,v.y - sz.y/2,v.x - sz.x/2,v.y + sz.y/2,cl);
		grDispatcher::instance() -> Line(v.x + sz.x/2,v.y - sz.y/2,v.x + sz.x/2,v.y + sz.y/2,cl);

		grDispatcher::instance() -> Line(v.x - sz.x/2,v.y - sz.y/2,v.x + sz.x/2,v.y + sz.y/2,cl);
		grDispatcher::instance() -> Line(v.x + sz.x/2,v.y - sz.y/2,v.x - sz.x/2,v.y + sz.y/2,cl);
	}
*/

	qdGameObjectAnimated::debug_redraw();
}

bool qdGameObjectMoving::is_in_position(const Vect3f pos) const
{
	if(!check_flag(QD_OBJ_MOVING_FLAG)){
		Vect3f pos1 = pos;
		if(!adjust_position(pos1))
			return false;

		Vect3f dr = R() - pos1;
		dr.z = 0.0f;

		if(dr.norm2() <= 0.01f) return true;
	}

	return false;
}

bool qdGameObjectMoving::is_in_position(const Vect3f pos,float angle) const
{
	if(!is_in_position(pos)) return false;
	if(fabs(direction_angle_ - angle) <= 0.01f) return true;

	return false;
}

bool qdGameObjectMoving::is_moving2position(const Vect3f pos) const
{
	if(check_flag(QD_OBJ_MOVING_FLAG)){
		Vect3f target = (path_length_) ? path_[path_length_] : target_r_;

		if(!adjust_position(target))
			return false;

		Vect3f target_pos = pos;
		if(!adjust_position(target_pos))
			return false;

		Vect3f dr = target - target_pos;
		dr.z = 0.0f;

		if (dr.norm2() <= 0.01f)
			return true;

		return false;
	}

	return false;
}

bool qdGameObjectMoving::is_moving2position(const Vect3f pos,float angle) const
{
	if(!is_moving2position(pos)) return false;

	if(fabs(angle - target_angle_) <= 0.01f) 
		return true;

	return false;
}

bool qdGameObjectMoving::can_move() const
{
	if(is_control_disabled() || check_flag(QD_OBJ_HIDDEN_FLAG | QD_OBJ_STATE_CHANGE_FLAG) || !can_change_state()) return false;

	if(const qdGameObjectState* p = queued_state()){
		if(p -> need_to_walk())
			return !p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DISABLE_WALK_INTERRUPT);
	}

	return true;
}

const Vect3f& qdGameObjectMoving::bound(bool perspective_correction) const
{
	// Случай с инверсной перспективой не обрабатывается (как атавизм)
	static Vect3f b;

	b = qdGameObjectAnimated::bound();	

	// Есле хоть один параметр альтернативной перспективы задан, то считаем
	// z границы (баунда) через calc_scale()
	if(qdCamera::current_camera() && perspective_correction && qdCamera::current_camera() -> need_perspective_correction()){
		Vect3f rr = R();
		rr.z = qdCamera::current_camera() -> get_grid_center().z;
		b = b * calc_scale(rr);
	}
	else
		b = b * scale_;

	return b;
}

Vect3f qdGameObjectMoving::calc_bound_in_pos(Vect3f pos, bool perspective_correction)
{
	Vect3f buf_r, bnd;
	buf_r = R();
	set_pos(pos);
	bnd = bound(perspective_correction);
	set_pos(buf_r);

	return bnd;
}

Vect2s qdGameObjectMoving::walk_grid_size(const Vect3f& r) const
{
#ifdef _QUEST_EDITOR
	Vect2s size = grid_size();
#else
	Vect2s size = walk_grid_size_;
#endif

	if(qdCamera::current_camera()&&	qdCamera::current_camera() -> need_perspective_correction()){
		float scale = calc_scale(r);
		size.x = round(float(size.x) * scale);
		if (size.x < 1) size.x = 1;
		size.y = round(float(size.y) * scale);
		if (size.y < 1) size.y = 1;
	}

	return size;
}

Vect2s qdGameObjectMoving::walk_grid_size(const Vect2s& r) const
{
#ifdef _QUEST_EDITOR
	Vect2s size = grid_size();
#else
	Vect2s size = walk_grid_size_;
#endif

	if(qdCamera::current_camera()&&	qdCamera::current_camera() -> need_perspective_correction()){
		Vect3f rr = qdCamera::current_camera() -> get_cell_coords(r.x,r.y);
		float scale = calc_scale(rr);
		size.x = round(float(size.x) * scale);
		if (size.x < 1) size.x = 1;
		size.y = round(float(size.y) * scale);
		if (size.y < 1) size.y = 1;
	}

	return size;
}

bool qdGameObjectMoving::calc_walk_grid(Vect2s& center, Vect2s& size) const
{
	const qdCamera* cp = qdCamera::current_camera();
	if (NULL == cp) return false;

	center = cp -> get_cell_index(R());
	size = walk_grid_size();
	return true;
}

bool qdGameObjectMoving::calc_cur_and_future_walk_grid(float dt, Vect2s& cen_cur, Vect2s& size_cur, 
		                                                       Vect2s& cen_next, Vect2s& size_next)
{
	const qdCamera* cp = qdCamera::current_camera();
	if (NULL == cp) return false;

	cen_cur = cp->get_cell_index(R());
	size_cur = walk_grid_size(R());

	// НЕ для движения считаем следующую позицию без изменений в объекте
	bool nope;
	Vect3f next_r = get_future_r(dt, nope, false);

	cen_next = cp->get_cell_index(next_r);
	size_next = walk_grid_size(next_r);
	
	return true;
}

bool qdGameObjectMoving::hit(int x,int y) const
{
	if(!is_visible()) return false;

	Vect2s sr = screen_pos();

	if(!check_flag(QD_OBJ_NO_SCALE_FLAG))
		return get_animation() -> hit(x - sr.x,y - sr.y,calc_scale());
	else 
		return get_animation() -> hit(x - sr.x,y - sr.y);
}

void qdGameObjectMoving::set_state(int st)
{
	// Указание на смену состояния => объект меняется (устанавливаем время изм.)
	set_last_chg_time(qdGameDispatcher::get_dispatcher()->time());

	if(max_state() && st >= 0 && st <= max_state()){
		qdGameObjectState* p = get_state(st);

#ifndef _QUEST_EDITOR
		if(p -> activation_delay() > 0.01f){
			__QDBG(appLog::default_log() << appLog::default_log().time_string() << " состояние ждет: " << name() << "/" << get_state(st) -> name() << "\r\n");

			if(!p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER)){
				p -> set_activation_timer();
				p -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
				set_queued_state(p);
				return;
			}

			if(!p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END))
				return;
		}

		if(p -> need_to_walk()){
			if(!can_change_state(p)) return;

			if(is_moving2position(p -> start_pos())){
				__QDBG(appLog::default_log() << appLog::default_log().time_string() << " состояние ждет: " << name() << "/" << get_state(st) -> name() << "\r\n");
				__QDBG(appLog::default_log() << appLog::default_log().time_string() << " pos " << R().x << " " << R().y << "/" << p -> start_pos().x << " " << p -> start_pos().y << "\r\n");
				return;
			}

			if(!is_in_position(p -> start_pos())){
				if(move(p -> start_pos(),p -> start_direction_angle(),true)){
					__QDBG(appLog::default_log() << appLog::default_log().time_string() << " состояние поставлено в очередь: " << name() << "/" << get_state(st) -> name() << "\r\n");
					set_queued_state(p);
				}
				
				return;
			}
		}
		set_queued_state(NULL);
		drop_flag(QD_OBJ_HIDDEN_FLAG);

		p -> set_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);

		if(p -> is_in_triggers())
			set_flag(QD_OBJ_IS_IN_TRIGGER_FLAG);

		if(p -> has_camera_mode() && owner())
			static_cast<qdGameScene*>(owner()) -> set_camera_mode(p -> camera_mode(),this);
#endif
		appLog::default_log() << appLog::default_log().time_string() << " старт состояния: " << name() << "/" << p -> name() << "\r\n";

		p -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER);
		p -> drop_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END);

		restore_grid_zone();
		
		if(cur_state() != -1 && cur_state() < max_state()){
			get_cur_state() -> stop_sound();

			if(!get_cur_state() -> forced_load() && cur_state() != st){
				get_cur_state() -> unregister_resources();
				p -> register_resources();
				get_cur_state() -> free_resources();
			}
			else
				p -> register_resources();
		}
		else
			p -> register_resources();

#ifdef _QUEST_EDITOR
		//возвращаем начальное положение объекта,
		//если оно вдруг было изменено предыдущим состоянием
		set_pos(default_R());
		set_direction(default_direction_angle());
		drop_flag(QD_OBJ_MOVING_FLAG);
#endif
		
		if(cur_state() != st){
			set_last_state(get_cur_state());
			set_last_inventory_state(get_cur_state());
			set_cur_state(st);

			speed_delta_ = 0.0f;
		}

		p -> load_resources();
		p -> start();

		qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
		assert(dp);

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

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_SAVE))
			dp->set_auto_save(p->autosave_slot());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_AUTO_LOAD))
			dp->set_auto_load(p->autosave_slot());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_IN))
			dp->set_fade(true, p->fade_time());

		if(p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_FADE_OUT))
			dp->set_fade(false, p->fade_time());
#endif
		
		init_grid_zone();

		qdGameObjectStateWalk* wp;

		switch(p -> state_type()){
			case qdGameObjectState::STATE_STATIC:
				path_length_ = 0;
				drop_flag(QD_OBJ_MOVING_FLAG);
				set_animation_info(static_cast<qdGameObjectStateStatic*>(p) -> animation_info());
				if(!p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT))
					disable_control();
#ifndef _QUEST_EDITOR
				if(!p -> coords_animation() -> is_empty() && (!p -> need_to_walk() || p -> coords_animation() -> size() > 1)){
					p -> coords_animation() -> start();
					get_animation() -> set_time_rel(p -> coords_animation() -> animation_phase());
				}
#else
				if(!p -> coords_animation() -> is_empty()){
					p -> coords_animation() -> start();
					get_animation() -> set_time_rel(p -> coords_animation() -> animation_phase());
				}
#endif
				if(!p -> has_sound_delay())
					p -> play_sound();
				break;
			case qdGameObjectState::STATE_WALK:
				wp = static_cast<qdGameObjectStateWalk*>(p);

				if(!impulse_movement_mode_){
					if(allowed_directions_count() <= 4)
						direction_angle_ = wp -> adjust_direction_angle(direction_angle_);

					if(!check_flag(QD_OBJ_MOVING_FLAG)){
						if(wp -> direction_angle() >= 0.0f)
							direction_angle_ = wp -> direction_angle();

						set_animation_info(static_cast<qdGameObjectStateWalk*>(p) -> static_animation_info(direction_angle_));
						p -> stop_sound();
					}
					else
						move2position(target_r_);
				}
				else {
					if(check_flag(QD_OBJ_MOVING_FLAG)){
						if(!is_direction_allowed(direction_angle_)){
							drop_flag(QD_OBJ_MOVING_FLAG);
							direction_angle_ = wp -> adjust_direction_angle(direction_angle_);
							set_direction(direction_angle_);
							p -> stop_sound();
						}
					}
					else {
						if(allowed_directions_count() <= 4)
							direction_angle_ = wp -> adjust_direction_angle(direction_angle_);

						set_direction(direction_angle_);
						p -> stop_sound();
					}
				}

				if(!p -> coords_animation() -> is_empty())
					p -> coords_animation() -> start();
#ifndef _QUEST_EDITOR
				else {
 					last_walk_state_ = p;
					if(p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
						dp -> update_walk_state(name(),p);

					walk_grid_size_ = grid_size();
				}

				if(has_control_type(CONTROL_AUTO_MOVE))
					start_auto_move();
#endif
				break;
		}
#ifdef _QUEST_EDITOR
		adjust_z();
#endif
		return;
	}

	if(st == -1){
		if(cur_state() != -1 && cur_state() < max_state()){
			get_state(cur_state()) -> stop_sound();
			get_state(cur_state()) -> unregister_resources();
			get_state(cur_state()) -> free_resources();
		}

		set_cur_state(st);
		get_animation() -> clear();
	}
}

void qdGameObjectMoving::set_state(qdGameObjectState* p)
{
	for(int i = 0; i < max_state(); i ++){
		if(get_state(i) == p){
			set_state(i);
			return;
		}
	}
}

Vect2s qdGameObjectMoving::get_pre_last_walkable_point(const Vect2s& target) const
{
	Vect2s trg = target;
	Vect2s src = qdCamera::current_camera() -> get_cell_index(R().x,R().y);

	if(src.x == -1 || target.x == -1 || !qdCamera::current_camera() -> clip_grid_line(src,trg))
		return Vect2s(-1,-1);

	int x1 = src.x;
	int y1 = src.y;

	int x2 = trg.x;
	int y2 = trg.y;

	if(x1 == x2 && y1 == y2)
		return Vect2s(-1,-1);

	Vect2f r(trg.x,trg.y);

	Vect2f dr(x2 - x1,y2 - y1);
	int delta = qdCamera::current_camera() -> get_cell_sx()/4;
	if(delta < 1) delta = 1; 
	dr.normalize(float(delta));
	// Идем с конца. Если натыкаемся на проходимую точку, отличную от начальной
	bool fir_step = true;
	if(abs(x2 - x1) > abs(y2 - y1)){
		int dx = int(float(x2 - x1)/dr.x);
		// Пропускаем все проходимые и доходим до непроходимой
		do {
			if(false == is_walkable(Vect2s(r.xi(),r.yi())))
				break;
			r -= dr;
		} while(--dx >= 0);
		// Доходим до первой проходимой
		do {
			if(true == is_walkable(Vect2s(r.xi(),r.yi())))			
				return Vect2s(r.xi(),r.yi());
			r -= dr;
		} while(--dx >= 0);
	}
	else {
		int dy = int(float(y2 - y1)/dr.y);
		// Пропускаем все проходимые и доходим до непроходимой
		do {
			if(false == is_walkable(Vect2s(r.xi(),r.yi())))
				break;
			r -= dr;
		} while(--dy >= 0);
		// Доходим до первой проходимой
		do {
			if(true == is_walkable(Vect2s(r.xi(),r.yi())))			
				return Vect2s(r.xi(),r.yi());
			r -= dr;
		} while(--dy >= 0);
	}

	return Vect2s(-1, -1); // не нашли
}

Vect2s qdGameObjectMoving::get_nearest_walkable_point(const Vect2s& target) const
{
	Vect2s trg = target;
	Vect2s src = qdCamera::current_camera() -> get_cell_index(R().x,R().y);

	if(src.x == -1 || target.x == -1 || !qdCamera::current_camera() -> clip_grid_line(src,trg))
		return Vect2s(-1,-1);

	int x1 = src.x;
	int y1 = src.y;

	int x2 = trg.x;
	int y2 = trg.y;

	if(x1 == x2 && y1 == y2)
		return Vect2s(-1,-1);

	Vect2f r(src.x,src.y);

	Vect2f dr(x2 - x1,y2 - y1);
	int delta = qdCamera::current_camera() -> get_cell_sx()/4;
	if(delta < 1) delta = 1; 
	dr.normalize(float(delta));
	// Идем с конца. Если натыкаемся на проходимую точку, отличную от начальной
	bool fir_step = true;
	if(abs(x2 - x1) > abs(y2 - y1)){
		int dx = int(float(x2 - x1)/dr.x);
		do {
			if(false == is_walkable(Vect2s(r.xi(),r.yi())))
			{
				// Если только первый шаг, то неудача
				if (fir_step) return Vect2s(-1, -1);
				r -= dr;
				return Vect2s(r.xi(),r.yi());
			}

			fir_step = false;
			r += dr;
		} while(--dx >= 0);
	}
	else {
		int dy = int(float(y2 - y1)/dr.y);
		do {
			if(false == is_walkable(Vect2s(r.xi(),r.yi())))
			{
				if (fir_step) return Vect2s(-1, -1);
				r -= dr;
				return Vect2s(r.xi(),r.yi());
			}
			
			fir_step = false;
			r += dr;
		} while(--dy >= 0);
	}

	// Если шаг так и не был сделан
	if (fir_step) return trg;

	r -= dr;
	return Vect2s(r.xi(), r.yi());
}


void qdGameObjectMoving::draw_contour(unsigned color) const
{
	Vect2s scrCoord = screen_pos();

	if(!check_flag(QD_OBJ_NO_SCALE_FLAG)){
		if(!get_animation() -> is_empty())
			get_animation() -> draw_contour(scrCoord.x,scrCoord.y,color,calc_scale());
	}
	else {
		if(!get_animation() -> is_empty())
			get_animation() -> draw_contour(scrCoord.x,scrCoord.y,color);
	}
}

float qdGameObjectMoving::calc_scale(const Vect3f& r) const
{
	if(!check_flag(QD_OBJ_NO_SCALE_FLAG)){
		float scale;

		if(!check_flag(QD_OBJ_INVERSE_PERSPECTIVE_FLAG))
			scale = qdCamera::current_camera() -> get_scale(r) * scale_;
		else
			scale = scale_ / qdCamera::current_camera() -> get_scale(r);

		return scale;
	}

	return 1.0f;
}

qdGameObjectState* qdGameObjectMoving::get_default_state()
{
#ifndef _QUEST_EDITOR
	if(last_walk_state_) return last_walk_state_;
#endif

	for(int i = 0; i < max_state(); i ++){
		if(get_state(i) -> state_type() == qdGameObjectState::STATE_WALK && !get_state(i) -> is_in_triggers() && get_state(i) -> coords_animation() -> is_empty())
			return get_state(i);
	}
	for(int i = 0; i < max_state(); i ++){
		if(get_state(i) -> state_type() == qdGameObjectState::STATE_WALK && !get_state(i) -> is_in_triggers())
			return get_state(i);
	}
	for(int i = 0; i < max_state(); i ++){
		if(!get_state(i) -> is_in_triggers() && !get_state(i) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY | qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
			return get_state(i);
	}

	return qdGameObjectAnimated::get_default_state();
}

const qdGameObjectState* qdGameObjectMoving::get_default_state() const
{
#ifndef _QUEST_EDITOR
	if(last_walk_state_) return last_walk_state_;
#endif

	for(int i = 0; i < max_state(); i ++){
		if(get_state(i) -> state_type() == qdGameObjectState::STATE_WALK && !get_state(i) -> is_in_triggers() && get_state(i) -> coords_animation() -> is_empty())
			return get_state(i);
	}
	for(int i = 0; i < max_state(); i ++){
		if(get_state(i) -> state_type() == qdGameObjectState::STATE_WALK && !get_state(i) -> is_in_triggers())
			return get_state(i);
	}
	for(int i = 0; i < max_state(); i ++){
		if(!get_state(i) -> is_in_triggers() && !get_state(i) -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY | qdGameObjectState::QD_OBJ_STATE_FLAG_GLOBAL_OWNER))
			return get_state(i);
	}

	return qdGameObjectAnimated::get_default_state();
}

bool qdGameObjectMoving::set_direction(float angle)
{
	angle = fmodf(angle,2.0f * M_PI);
	if(angle < 0.0f) angle += 2.0f * M_PI;

	direction_angle_ = angle;

	if(!check_flag(QD_OBJ_MOVING_FLAG)){
		qdGameObjectState* st = get_state(cur_state());
		if(st && st -> state_type() == qdGameObjectState::STATE_WALK)
			set_animation_info(static_cast<qdGameObjectStateWalk*>(st) -> static_animation_info(direction_angle_));

		return true;
	}

	return false;
}

int qdGameObjectMoving::get_direction(float angle) const
{
	if(const qdGameObjectState* st = get_state(cur_state())){
		if(st -> state_type() == qdGameObjectState::STATE_WALK){
			if(const qdAnimationSet* set = reinterpret_cast<const qdGameObjectStateWalk*>(st) -> animation_set())
				return set -> get_angle_index(angle);
		}
	}

	return -1;
}

bool qdGameObjectMoving::adjust_z()
{
	qdCamera * ptrCam = static_cast<qdGameScene*>(owner()) -> get_camera();
	assert(ptrCam);

	Vect3f dr = R();
	dr.z = bound().z / 2.0f + ptrCam -> get_grid_center().z;

	const sGridCell* cp = ptrCam -> get_cell(dr.x,dr.y);
	if(cp)
		dr.z += cp -> height();

	set_pos(dr);

	return true;
}

void qdGameObjectMoving::merge(qdGameObjectMoving* p)
{
	merge_states(p);

#ifndef _QUEST_EDITOR
	if(p -> last_walk_state_)
		last_walk_state_ = p -> last_walk_state_;
#endif
}

void qdGameObjectMoving::split(qdGameObjectMoving* p)
{
#ifndef _QUEST_EDITOR
	if(last_walk_state_){
		for(int i = 0; i < p -> max_state(); i ++){
			if(last_walk_state_ == p -> get_state(i)){
				p -> last_walk_state_ = last_walk_state_;
				break;
			}
		}
	}
#endif
	
	split_states(p);
}

bool qdGameObjectMoving::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdGameObjectAnimated::load_data(fh,save_version)) return false;

#ifndef _QUEST_EDITOR
	int idx;
	fh > idx > walk_grid_size_.x > walk_grid_size_.y;

	if(idx != -1)
		last_walk_state_ = get_state(idx);
	else
		last_walk_state_ = NULL;
#endif

	char mode;
	fh > scale_ > direction_angle_ > rotation_angle_ > target_r_.x > target_r_.y > target_r_.z > target_angle_;
	fh > path_length_ > cur_path_index_ > mode > impulse_timer_ > impulse_direction_ > speed_delta_;

	if(save_version >= 104){
		char rot_mode;
		fh > rot_mode;
		if(save_version >= 105){
			movement_mode_ = movement_mode_t(rot_mode);
			fh > movement_mode_time_ > movement_mode_time_current_;
		}
		else {
			if(rot_mode)
				movement_mode_ = MOVEMENT_MODE_TURN;
			else
				movement_mode_ = MOVEMENT_MODE_STOP;
		}
	}

	impulse_movement_mode_ = mode;
	is_selected_ = false;

	for(int i = 0; i < path_length_; i ++)
		fh > path_[i].x > path_[i].y > path_[i].z;

	fh > follow_condition_;

	int num;
	fh > num;
	circuit_objs_.clear();
	for (int i = 0; i < num; i++)
	{
		qdNamedObjectReference circ_ref;
		circ_ref.load_data(fh, save_version);
		circuit_objs_.push_back(dynamic_cast<qdGameObjectMoving*>(
			                      qdGameDispatcher::get_dispatcher()->get_named_object(&circ_ref)));
	}

	return true;
}

bool qdGameObjectMoving::save_data(qdSaveStream& fh) const
{
	if(!qdGameObjectAnimated::save_data(fh)) return false;

#ifndef _QUEST_EDITOR
	int idx = -1;
	if(last_walk_state_) idx = get_state_index(last_walk_state_);
	fh < idx < walk_grid_size_.x < walk_grid_size_.y;
#endif

	fh < scale_ < direction_angle_ < rotation_angle_ < target_r_.x < target_r_.y < target_r_.z < target_angle_;
	fh < path_length_ < cur_path_index_ < char(impulse_movement_mode_) < impulse_timer_ < impulse_direction_ < speed_delta_ < char(movement_mode_);
	fh < movement_mode_time_ < movement_mode_time_current_;

	for(int i = 0; i < path_length_; i ++)
		fh < path_[i].x < path_[i].y < path_[i].z;

	fh < follow_condition_;

	fh < circuit_objs_.size();
	for (int i = 0; i < circuit_objs_.size(); i++)
	{
		qdNamedObjectReference circ_ref(circuit_objs_[i]);
		circ_ref.save_data(fh);
	}

	return true;
}

bool qdGameObjectMoving::init()
{
	if(!qdGameObjectAnimated::init()) return false;

#ifndef _QUEST_EDITOR
	walk_grid_size_ = Vect2s(0,0);
	last_walk_state_ = NULL;
#endif

	impulse_movement_mode_ = false;
	impulse_timer_ = 0.0f;
	impulse_start_timer_ = 0.0f;
	impulse_direction_ = -1.0f;

	drop_flag(QD_OBJ_MOVING_FLAG);

	direction_angle_ = default_direction_angle_;
	path_length_ = cur_path_index_ = 0;

	// Грузим attacher_ по attacher_ref_
	qdNamedObject* nam_obj = qdGameDispatcher::get_dispatcher()->
		                       get_named_object(&attacher_ref());
	set_attacher(dynamic_cast<const qdGameObjectMoving*>(nam_obj));

	return true;
}

grScreenRegion qdGameObjectMoving::screen_region() const
{
	if(is_visible()){
		grScreenRegion reg = (check_flag(QD_OBJ_NO_SCALE_FLAG)) ? get_animation() -> screen_region() : get_animation() -> screen_region(0,calc_scale());
		reg.move(screen_pos().x,screen_pos().y);

		return reg;
	}
	else
		return grScreenRegion::EMPTY;
}

void qdGameObjectMoving::redraw(int offs_x,int offs_y) const
{
	if(get_animation() -> is_empty())
		return;

	Vect2i r = screen_pos() + Vect2i(offs_x,offs_y);

	if(!check_flag(QD_OBJ_NO_SCALE_FLAG))
		get_animation() -> redraw(r.x,r.y,screen_depth(),calc_scale());
	else 
		get_animation() -> redraw(r.x,r.y,0);

	if(shadow_alpha() != QD_NO_SHADOW_ALPHA)
		draw_shadow(offs_x,offs_y,shadow_color(),shadow_alpha());
}

bool qdGameObjectMoving::keyboard_move()
{
	if(!is_control_disabled() && has_control_type(CONTROL_KEYBOARD)){
		bool keypress = false;

		static int vkeys[6] = { VK_DOWN, VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT };
		bool key_state[6];

		for(int i = 0; i < 6; i++){
			key_state[i] = keyboardDispatcher::instance() -> is_pressed(vkeys[i]);
			if(key_state[i]) keypress = true;
		}

		if(keypress){
			float direction = 0.0f;
			for(int i = 1; i < 5; i++){
				if(key_state[i]){
					direction = M_PI / 2.0f * float(i - 1);
						if(key_state[i - 1])
							direction -= M_PI / 4.0f;
						else if(key_state[i + 1])
							direction += M_PI / 4.0f;
				}
			}

			if(is_direction_allowed(direction))
				set_movement_impulse(direction);
		}
		else {
			if(impulse_movement_mode_ && is_moving() && !has_control_type(CONTROL_AUTO_MOVE)){
				stop_movement();
				impulse_movement_mode_ = false;
			}
		}

		return true;
	}

	return false;
}

bool qdGameObjectMoving::set_walk_animation()
{
	float tm = 0.0f;
	
	if(check_flag(QD_OBJ_MOVING_FLAG))
		tm = get_animation() -> cur_time_rel();

#ifndef _QUEST_EDITOR
	if(cur_state() == -1 || get_state(cur_state()) -> state_type() != qdGameObjectState::STATE_WALK){
		if(!last_walk_state_){
			qdGameObjectState* p = get_default_state();
			if(p && p -> state_type() == qdGameObjectState::STATE_WALK)
				set_state(p);
		}
		else
			set_state(last_walk_state_);
	}
#endif

	if(cur_state() == -1 || get_cur_state() -> state_type() != qdGameObjectState::STATE_WALK) return false;

	qdGameObjectState* st = get_state(cur_state());
	if(st -> state_type() == qdGameObjectState::STATE_WALK){
		qdGameObjectStateWalk* wst = static_cast<qdGameObjectStateWalk*>(st);

		switch(movement_mode_){
			case MOVEMENT_MODE_TURN:
				if(qdAnimationSet* set = wst->animation_set()){
					if(qdAnimation* anm = set->get_turn_animation()){
						set_animation_info(set->get_turn_animation_info());
						get_animation()->set_time_rel(cycleAngle(direction_angle_)/(2.f*M_PI));
						set_flag(QD_OBJ_MOVING_FLAG);
						return true;
					}
				}
				break;
			case MOVEMENT_MODE_START:
				if(qdAnimationSet* set = wst->animation_set()){
					qdAnimationInfo* inf = set->get_start_animation_info(direction_angle_);
					if(inf->animation()){
						set_animation_info(inf);
						get_animation() -> set_time_rel(tm);
						set_flag(QD_OBJ_MOVING_FLAG);
						return true;
					}
				}
				break;
			case MOVEMENT_MODE_END:
				if(qdAnimationSet* set = wst->animation_set()){
					qdAnimationInfo* inf = set->get_stop_animation_info(direction_angle_);
					if(inf->animation()){
						set_animation_info(inf);
						get_animation() -> set_time_rel(tm);
						set_flag(QD_OBJ_MOVING_FLAG);
						return true;
					}
				}
				break;
		}

		set_animation_info(wst -> animation_info(direction_angle_));
		get_animation() -> set_time_rel(tm);

		if(!check_flag(QD_OBJ_MOVING_FLAG))
			st -> play_sound();

		wst -> update_sound_frequency(direction_angle_);
	}

	set_flag(QD_OBJ_MOVING_FLAG);
	return true;
}

bool qdGameObjectMoving::set_movement_impulse(float dir_angle)
{
	if(impulse_direction_ >= 0.0f) return false;

	adjust_direction_angle(dir_angle);
	impulse_direction_ = dir_angle;

	if(collision_delay_ > FLT_EPS)
		impulse_start_timer_ = collision_delay_;
	else
		return movement_impulse();

	return true;
}

bool qdGameObjectMoving::movement_impulse()
{
	if(impulse_direction_ < 0.0f || !is_direction_allowed(impulse_direction_) || !can_move() || (is_moving() && !impulse_movement_mode_) || check_grid_zone_attributes(sGridCell::CELL_IMPASSABLE)){
		impulse_direction_ = -1.0f;
		return false;
	}

	change_direction_angle(impulse_direction_);

//	direction_angle_ = impulse_direction_;
	impulse_direction_ = -1.0f;
	target_angle_ = -1.0f;

	if(movement_mode_ == MOVEMENT_MODE_STOP || movement_mode_ == MOVEMENT_MODE_END)
		movement_mode_ = MOVEMENT_MODE_TURN;

	set_flag(QD_OBJ_MOVING_FLAG);

	float sp = speed();
	if(sp > FLT_EPS) impulse_timer_ = collision_path_ / sp;
	else impulse_timer_ = 0.0f;

	impulse_movement_mode_ = true;
	
	return true;
}

float qdGameObjectMoving::speed()
{
	if(qdGameObjectState* st = get_cur_state()){
		if(st -> state_type() == qdGameObjectState::STATE_WALK){
			if(qdAnimationInfo* inf = static_cast<qdGameObjectStateWalk*>(st) -> animation_info(direction_angle_))
				return inf -> speed() * scale();
		}
	}

	return 0.0f;
}

void qdGameObjectMoving::set_attacher(const qdGameObjectMoving* mov_obj)
{
	attacher_ = mov_obj; 
	if (NULL != mov_obj)
	{
		qdNamedObjectReference ref(mov_obj);
		attacher_ref_ = ref;
	}
	else
		attacher_ref_.clear();
}


bool qdGameObjectMoving::avoid_collision(const qdGameObjectMoving* p)
{
	if(!can_move() || is_moving()) return false;

	float direction = p -> direction_angle() + qdCamera::current_camera() -> get_z_angle() * M_PI / 180.0f;
	float angle = p -> calc_direction_angle(R());

	direction += (getDist(direction,angle,2.0f*M_PI) < 0) ? M_PI / 2.0f : -M_PI / 2.0f;

	float dist = (radius() + p -> radius()) * 0.7f;

	Vect3f r(R());
	r.x += dist * cos(direction);
	r.y += dist * sin(direction);

	if(move(r,true)) return true;

//	r = R();
//	r.x += dist * cos(angle);
//	r.y += dist * sin(angle);

//	if(init_movement(r,true)) return true;
/*
	const num_angles = 4;
	for(int i = 0; i < num_angles; i++){
		float dir = direction + M_PI / 2.0f / float(num_angles) * float(i) + p -> direction_angle() + qdCamera::current_camera() -> get_z_angle() * M_PI / 180.0f;

		Vect3f r(R());
		r.x += dist * cos(dir);
		r.y += dist * sin(dir);

		if(init_movement(r,true)) return true;

		dir = direction - M_PI / 2.0f / float(num_angles) * float(i) + p -> direction_angle() + qdCamera::current_camera() -> get_z_angle() * M_PI / 180.0f;

		r = R();
		r.x += dist * cos(dir);
		r.y += dist * sin(dir);

		if(init_movement(r,true)) return true;
	}
*/
	return false;
}

void qdGameObjectMoving::optimize_path(std::vector<Vect2i>& path) const
{
	std::list<Vect2i> opt_path;
	opt_path.insert(opt_path.end(),path.begin(),path.end());
	opt_path.unique();

	std::list<Vect2i>::iterator it = opt_path.begin();
	
	while(it != opt_path.end()){
		std::list<Vect2i>::iterator it1 = it;
		++it1;

		if(it1 != opt_path.end()){
			std::list<Vect2i>::iterator it2 = it1;
			++it2;

			if(it2 != opt_path.end()){
				if((it -> x == it1 -> x && it -> x == it2 -> x) || (it -> y == it1 -> y && it -> y == it2 -> y))
					opt_path.erase(it1);
				else
					++it;
			}
			else
				++it;
		}
		else
			++it;
	}

	switch(movement_type()){
	case qdGameObjectStateWalk::MOVEMENT_FOUR_DIRS:
		optimize_path_four_dirs(opt_path);
		break;
	case qdGameObjectStateWalk::MOVEMENT_EIGHT_DIRS:
		optimize_path_eight_dirs(opt_path);
		break;
	case qdGameObjectStateWalk::MOVEMENT_SMOOTH:
		optimize_path_smooth(opt_path);
		break;
	}

	path.clear();
	path.insert(path.end(),opt_path.begin(),opt_path.end());
}

void qdGameObjectMoving::optimize_path_four_dirs(std::list<Vect2i>& path) const
{
	std::list<Vect2i>::iterator it = path.begin();

	while(it != path.end()){
		std::list<Vect2i>::iterator it1 = it;
		++it1;

		if(it1 != path.end()){
			std::list<Vect2i>::iterator it2 = it1;
			++it2;

			if(it2 != path.end()){
				std::list<Vect2i>::iterator it3 = it2;
				++it3;

				if(it3 != path.end()){
					if(it -> y == it1 -> y){
						if(is_path_walkable(it -> x,it -> y,it3 -> x,it -> y) && is_path_walkable(it3 -> x,it -> y,it3 -> x,it3 -> y)){
							path.erase(it2);
							it1 -> x = it3 -> x;
						}
						else {
							if(is_path_walkable(it -> x,it3 -> y,it3 -> x,it3 -> y) && is_path_walkable(it -> x,it3 -> y,it -> x,it -> y)){
								path.erase(it1);
								it2 -> x = it -> x;
							}
							else
								++it;
						}
					}
					else {
						if(is_path_walkable(it -> x,it -> y,it -> x,it3 -> y) && is_path_walkable(it3 -> x,it3 -> y,it -> x,it3 -> y)){
							path.erase(it2);
							it1 -> y = it3 -> y;
						}
						else {
							if(is_path_walkable(it3 -> x,it -> y,it3 -> x,it3 -> y) && is_path_walkable(it3 -> x,it -> y,it -> x,it -> y)){
								path.erase(it1);
								it2 -> y = it -> y;
							}
							else
								++it;
						}
					}
				}
				else
					++it;
			}
			else
				++it;
		}
		else
			++it;
	}
}

double vec_cos(Vect2i v1, Vect2i v2)
{
	if ((0 == v1.norm2()) || (0 == v2.norm2())) return 0.0;
	return (v1.x*v2.x + v1.y*v2.y)/(sqrt((double)v1.norm2())*sqrt((double)v2.norm2()));
}

// Проверка векторов на коллинеарность
bool coll(const Vect2i v1, const Vect2i v2)
{
	if (((v1.x*v2.y == v2.x*v1.y) && (v1.x*v2.x + v1.y*v2.y != 0)))
		return true;
	else return false;
}

const double SQRT_2_DIV_2 = 0.7071067811865475244;

bool qdGameObjectMoving::del_coll_pts(std::list<Vect2i>& path) const
{
	bool is_del = false;

	// Пытаемся выделить три точки пути (если этого сделать нельзя, то выход)
	std::list<Vect2i>::iterator cur = path.begin();
	std::list<Vect2i>::iterator pre, pre_pre;
	if (cur != path.end())
	{
		pre_pre = cur;
		++cur;
	}
	else return false;
	if (cur != path.end())
	{
		pre = cur;
		++cur;
	}
	else return false;
	// если три точки коллинеарны, то среднюю (pre) удаляем
	while (cur != path.end())
	{
		if ( coll((*pre) - (*pre_pre), (*cur) - (*pre_pre)) )
		{
			is_del = true;
			path.erase(pre);
			pre = cur;
			++cur;
		}
		else
		{
			pre_pre = pre;
			pre = cur;
			++cur;
		}
	}

	return is_del;
}

// Вспомогательная функция - пытается спрямить отрезок пути из четырех точек, начиная с cur
/*
bool qdGameObjectMoving::four_pts_eight_dir_straight(std::list<Vect2i> path, 
						                             const std::list<Vect2i>::iterator cur) const
{
	// Извлекаем четыре точки
	Vect2i pts[4];
	std::list<Vect2i>::iterator buf = cur;
	for (int i = 0; i < 4; i++)
	{
		if (path.end() == buf) return false;
		pts[i] = (*buf);
		++buf;
	}
	// Проверяем - является ли четверка точек "вытянутым зигзагом"
	if ((fabs(vec_cos(pts[1] - pts[0], pts[2] - pts[1]) - SQRT_2_DIV_2) > 0.0001) ||
		(fabs(vec_cos(pts[2] - pts[1], pts[3] - pts[2]) - SQRT_2_DIV_2) > 0.0001) ||
		!coll(pts[1] - pts[0], pts[3] - pts[2]))
		return false;

	// Проверяем, проходИм ли новый, спрямленный путь
	Vect2i pnt;
	for (int i = 0; i < 2; i++)
	{
		if (0 == i) pnt = pts[3] - pts[2] + pts[1];
		else pnt = pts[0] + pts[2] - pts[1];

		if (is_path_walkable(pts[0], pnt) && 
			is_path_walkable(pnt, pts[3]))
		{
			// Удаляем две промежуточные
			buf = cur;
			path.erase(++buf);
			buf = cur;
			path.erase(++buf);	
			// Добавляем новую перед указателем на следующую
			buf = cur;
			++buf;
			path.insert(buf, pnt);
			return true;
		}
	}
		
	return false;
}
*/

bool qdGameObjectMoving::four_pts_eight_dir_straight(std::list<Vect2i>& path, std::list<Vect2i>::reverse_iterator cur) const
{
	// Извлекаем четыре точки
	Vect2i pts[4], opt_pts[4];
	std::list<Vect2i>::reverse_iterator buf = cur;
	for (int i = 0; i < 4; i++)
	{
		if (path.rend() == buf) return false;
		pts[i] = (*buf);
		opt_pts[i] = pts[i];
		++buf;
	}

	// Пробуем подвигать средние точки (в частных случаях может улучшить, в худших 
	// не сделает хуже)
	Vect2i new_pnt = pts[0] + (pts[2] - pts[1]);
	if (is_path_walkable(pts[0], new_pnt) &&
		is_path_walkable(new_pnt, pts[2]))
		opt_pts[1] = new_pnt;
	// Если первое "спрямление" привело к коллинеарным точкам, то уже отлично
	// иначе пробуем другое спрямление
	if (!coll(new_pnt - pts[0], pts[2] - new_pnt))
	{
		opt_pts[1] = pts[1]; // Потому как могла измениться ранее
		new_pnt = pts[3] - (pts[2] - pts[1]);

		if (is_path_walkable(pts[1], new_pnt) &&
			is_path_walkable(new_pnt, pts[3]))
			opt_pts[2] = new_pnt;
	}
	
	buf = cur;
	for (int i = 0; i < 4; i++)
	{
		if (path.rend() == buf) return false;
		(*buf) = opt_pts[i];
		++buf;
	}
	
	return true;
}

void qdGameObjectMoving::optimize_path_eight_dirs(std::list<Vect2i>& path) const
{
	// Спрямляем, пока спрямляется, но не более чем EIGHT_DIRS_OPT_ITER_MAX раз
    
	for (int i = 0; i < EIGHT_DIRS_OPT_ITER_MAX; i++)
	{
		for (std::list<Vect2i>::reverse_iterator it = path.rbegin(); it != path.rend(); ++it)
			four_pts_eight_dir_straight(path, it);

		if (!del_coll_pts(path)) break;
	}
}

void qdGameObjectMoving::optimize_path_smooth(std::list<Vect2i>& path) const
{
	std::list<Vect2i>::iterator it = path.begin();

	while(it != path.end()){
		std::list<Vect2i>::iterator it1 = it;
		++it1;

		if(it1 != path.end()){
			std::list<Vect2i>::iterator it2 = it1;
			++it2;

			if(it2 != path.end()){
				if(is_path_walkable(it -> x,it -> y,it2 -> x,it2 -> y)){
					path.erase(it1);
				}
				else
					++it;
			}
			else
				++it;
		}
		else
			++it;
	}
}

void qdGameObjectMoving::finalize_path(const Vect3f& from, const Vect3f& to, const std::vector<Vect2i>& path, std::vector<Vect3f>& out_path) const
{
	if(movement_type() != qdGameObjectStateWalk::MOVEMENT_FOUR_DIRS && movement_type() != qdGameObjectStateWalk::MOVEMENT_EIGHT_DIRS)
		return;

	qdCamera* cp = qdCamera::current_camera();

	out_path.resize(path.size(), Vect3f(0,0,0));
	for(int i = 0; i < path.size(); i++)
		out_path[i] = cp->get_cell_coords(path[i].x,path[i].y);

	Vect2f d = to - out_path.back();
	out_path[path.size() - 1] = to;

	std::vector<Vect2i>::const_reverse_iterator it = path.rbegin();
	std::vector<Vect3f>::reverse_iterator itp = out_path.rbegin();
	itp++;
	do {
		std::vector<Vect2i>::const_reverse_iterator it1 = it;
		it1++;
		if(it1 == path.rend())
			break;

		Vect2i delta = *it - *it1;

		if(delta.y){
			itp->x += d.x;
			if(!delta.x)
				d.y = 0;
		}

		if(delta.x){
			itp->y += d.y;
			if(!delta.y)
				d.x = 0;
		}

		if(fabs(d.x) <= FLT_EPS && fabs(d.y) <= FLT_EPS)
			break;

		it++;
		itp++;
	} while(it != path.rend());
}

bool qdGameObjectMoving::adjust_position(Vect3f& pos) const
{
	switch(movement_type()){
	case qdGameObjectStateWalk::MOVEMENT_LEFT:
		if(pos.x <= R().x && fabs(R().y - pos.y) <= bound().y/2.0f){
			pos.y = R().y;
			return true;
		}
		break;
	case qdGameObjectStateWalk::MOVEMENT_UP:
		if(pos.y >= R().y && fabs(R().x - pos.x) <= bound().x/2.0f){
			pos.x = R().x;
			return true;
		}
		break;
	case qdGameObjectStateWalk::MOVEMENT_RIGHT:
		if(pos.x >= R().x && fabs(R().y - pos.y) <= bound().y/2.0f){
			pos.y = R().y;
			return true;
		}
		break;
	case qdGameObjectStateWalk::MOVEMENT_DOWN:
		if(pos.y <= R().y && fabs(R().x - pos.x) <= bound().x/2.0f){
			pos.x = R().x;
			return true;
		}
		break;
	case qdGameObjectStateWalk::MOVEMENT_UP_LEFT:
		return adjust_position(pos,M_PI/4.0f*3.0f);
	case qdGameObjectStateWalk::MOVEMENT_UP_RIGHT:
		return adjust_position(pos,M_PI/4.0f*1.0f);
	case qdGameObjectStateWalk::MOVEMENT_DOWN_RIGHT:
		return adjust_position(pos,M_PI/4.0f*7.0f);
	case qdGameObjectStateWalk::MOVEMENT_DOWN_LEFT:
		return adjust_position(pos,M_PI/4.0f*5.0f);
	case qdGameObjectStateWalk::MOVEMENT_HORIZONTAL:
		if(fabs(R().y - pos.y) <= bound().y/2.0f){
			pos.y = R().y;
			return true;
		}
		break;
	case qdGameObjectStateWalk::MOVEMENT_VERTICAL:
		if(fabs(R().x - pos.x) <= bound().x/2.0f){
			pos.x = R().x;
			return true;
		}
		break;
	default:
		return true;
	}

	return false;
}

bool qdGameObjectMoving::adjust_position(Vect3f& pos,float dir_angle) const
{
	float d = -pos.x * sin(dir_angle) + pos.y * cos(dir_angle);

	if(fabs(d) <= radius()){
		float angle = calc_direction_angle(pos);
		float delta_angle = getDeltaAngle(angle,dir_angle);
		if(fabs(delta_angle) <= M_PI/2.0f){
			float d1 = sqrt(pos.x * pos.x + pos.y * pos.y - d * d);
			pos.x = d1 * cos(dir_angle);
			pos.y = d1 * sin(dir_angle);
			return true;
		}
	}

	return false;
}

bool qdGameObjectMoving::is_direction_allowed(float angle) const
{
	angle = cycleAngle(angle);

	switch(movement_type()){
	case qdGameObjectStateWalk::MOVEMENT_LEFT:
		if(fabs(angle - M_PI) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_UP:
		if(fabs(angle - M_PI/2.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_RIGHT:
		if(fabs(angle) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_DOWN:
		if(fabs(angle - M_PI/2.0f*3.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_UP_LEFT:
		if(fabs(angle - M_PI/4.0f*3.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_UP_RIGHT:
		if(fabs(angle - M_PI/4.0f*1.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_DOWN_RIGHT:
		if(fabs(angle - M_PI/4.0f*7.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_DOWN_LEFT:
		if(fabs(angle - M_PI/4.0f*5.0f) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_HORIZONTAL:
		if(fabs(angle) <= 0.01f) return true;
		if(fabs(angle - M_PI) <= 0.01f) return true;
		break;
	case qdGameObjectStateWalk::MOVEMENT_VERTICAL:
		if(fabs(angle - M_PI/2.0f) <= 0.01f) return true;
		if(fabs(angle - M_PI/2.0f*3.0f) <= 0.01f) return true;
		break;
	default:
		return true;
	}

	return false;
}

bool qdGameObjectMoving::adjust_direction_angle(float& angle)
{
	if(const qdGameObjectStateWalk* p = current_walk_state()){
		angle = p -> adjust_direction_angle(angle);
		return true;
	}

	return false;
}

void qdGameObjectMoving::change_direction_angle(float angle)
{
	if (!has_control_type(CONTROL_ANIMATED_ROTATION))
		direction_angle_ = angle;
	else
	{
		rotation_angle_ = angle - direction_angle_;	
		// Приводим к диапазону -PI..PI
		while (rotation_angle_ > 2*M_PI)
			rotation_angle_ -= 2*M_PI;
		while (rotation_angle_ < 0)
			rotation_angle_ += 2*M_PI;
		if (rotation_angle_	> M_PI) rotation_angle_ -= 2*M_PI;
	}
}

bool qdGameObjectMoving::get_speed_parameters(float& speed,float& speed_max,float& acceleration)
{
	if(qdGameObjectState* st = get_cur_state()){
		if(st -> state_type() == qdGameObjectState::STATE_WALK){
			float sc = scale();
			if(qdCamera::current_camera() && qdCamera::current_camera() -> need_perspective_correction())
				sc = calc_scale();

			qdGameObjectStateWalk* sw = static_cast<qdGameObjectStateWalk*>(st);

			speed_max = sw -> max_speed() * sc;
			acceleration = sw -> acceleration() * sc;

			if(qdAnimationInfo* inf = sw -> animation_info(direction_angle_))
				speed = inf -> speed() * sc;
		}
	}

	return false;
}

qdGameObjectStateWalk::movement_type_t qdGameObjectMoving::movement_type() const
{
	if(const qdGameObjectStateWalk* st = current_walk_state())
		return st -> movement_type();

	return qdGameObjectStateWalk::MOVEMENT_EIGHT_DIRS;
}

int qdGameObjectMoving::allowed_directions_count() const
{
	switch(movement_type()){
	case qdGameObjectStateWalk::MOVEMENT_LEFT:
	case qdGameObjectStateWalk::MOVEMENT_UP:
	case qdGameObjectStateWalk::MOVEMENT_RIGHT:
	case qdGameObjectStateWalk::MOVEMENT_DOWN:
	case qdGameObjectStateWalk::MOVEMENT_UP_LEFT:
	case qdGameObjectStateWalk::MOVEMENT_UP_RIGHT:
	case qdGameObjectStateWalk::MOVEMENT_DOWN_RIGHT:
	case qdGameObjectStateWalk::MOVEMENT_DOWN_LEFT:
		return 1;
	case qdGameObjectStateWalk::MOVEMENT_HORIZONTAL:
	case qdGameObjectStateWalk::MOVEMENT_VERTICAL:
		return 2;
	case qdGameObjectStateWalk::MOVEMENT_FOUR_DIRS:
		return 4;
	case qdGameObjectStateWalk::MOVEMENT_EIGHT_DIRS:
		return 8;
	case qdGameObjectStateWalk::MOVEMENT_SMOOTH:
		return 16;
	}

	return 0;
}

const qdGameObjectStateWalk* qdGameObjectMoving::current_walk_state() const
{
	const qdGameObjectState* st = get_cur_state();
	if(!st || st -> state_type() != qdGameObjectState::STATE_WALK){
#ifndef _QUEST_EDITOR
		st = last_walk_state_;
		if(!st || st -> state_type() != qdGameObjectState::STATE_WALK)
			st = get_default_state();
else
		st = get_default_state();
		if(!st) st = get_state(0);
#endif
	}

	if(st && st -> state_type() == qdGameObjectState::STATE_WALK)
		return static_cast<const qdGameObjectStateWalk*>(st);

	return NULL;
}

bool qdGameObjectMoving::start_auto_move()
{
	if(can_move()){
		float angle = direction_angle_;
		adjust_direction_angle(angle);
		set_movement_impulse(angle);
		impulse_timer_ = 0.0f;
		movement_impulse();

		return true;
	}

	return false;
}

bool qdGameObjectMoving::can_change_state(const qdGameObjectState* state) const
{
	if(!qdGameObjectAnimated::can_change_state(state)) return false;

	if(const qdGameObjectState* p = queued_state()){
		if(p != state && p -> need_to_walk())
			return !p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DISABLE_WALK_INTERRUPT);
	}

	return true;
}

bool qdGameObjectMoving::toggle_grid_zone(bool make_walkable)
{
	if(make_walkable)
		return drop_grid_zone_attributes(sGridCell::CELL_PERSONAGE_OCCUPIED);
	else 
		return set_grid_zone_attributes(sGridCell::CELL_PERSONAGE_OCCUPIED);
}

bool qdGameObjectMoving::move_from_personage_path()
{
	float dist = radius()/2.0f;

	const num_distances = 3;
	for(int i = 0; i < num_distances; i ++){
		const num_angles = 8;
		for(int j = 0; j < num_angles; j++){
			float dir = 2.0f * M_PI / float(num_angles) * float(j);

			Vect3f r(R());
			r.x += dist * cos(dir);
			r.y += dist * sin(dir);

			if(!check_grid_zone_attributes(Vect2f(r.x,r.y),sGridCell::CELL_PERSONAGE_PATH)){
				if(move(r,true))
					return true;
			}
		}

		dist += radius() / 2.0f;
	}

	return false;
}

void qdGameObjectMoving::draw_shadow(int offs_x,int offs_y,unsigned color,int alpha) const
{
	if(alpha == QD_NO_SHADOW_ALPHA || get_animation() -> is_empty())
		return;

	Vect2i r = screen_pos() + Vect2i(offs_x,offs_y);

	if(check_flag(QD_OBJ_NO_SCALE_FLAG))
		get_animation() -> draw_mask(r.x,r.y,screen_depth(),grDispatcher::instance() -> make_rgb(color),alpha);
	else
		get_animation() -> draw_mask(r.x,r.y,screen_depth(),grDispatcher::instance() -> make_rgb(color),alpha,calc_scale());
}

bool qdGameObjectMoving::get_debug_info(XBuffer& buf) const
{
	qdGameObjectAnimated::get_debug_info(buf);
#ifdef __QD_DEBUG_ENABLE__
	buf < "scale: " <= calc_scale() < "\n";
#endif
	return true;
}


