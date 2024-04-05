/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_coords_animation.h"
#include "qd_game_object_animated.h"
#include "qd_game_object_moving.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdCoordsAnimation::qdCoordsAnimation() : status_(false),
	is_finished_(false),
	type_(CA_INTERPOLATE_COORDS),
	animation_phase_(0.0f),
	speed_(100.0f),
	start_object_(NULL),
	cur_point_(0)
#ifdef _QUEST_EDITOR
	,animation_scroll_phase_(0.f)
#endif
{
}

qdCoordsAnimation::qdCoordsAnimation(const qdCoordsAnimation& anm) : qdNamedObject(anm),
	status_(false),
	is_finished_(false),
	type_(anm.type_),
	animation_phase_(anm.animation_phase_),
	speed_(anm.speed_),
	start_object_(anm.start_object_),
	start_object_ref_(anm.start_object_ref()),
	points_(anm.points_),
	cur_point_(0)
#ifdef _QUEST_EDITOR
	,animation_scroll_phase_(anm.animation_scroll_phase_)
#endif
{
}

qdCoordsAnimation& qdCoordsAnimation::operator = (const qdCoordsAnimation& anm)
{
	qdNamedObject::operator=(anm);

	status_ = false;
	is_finished_ = false;
	type_ = anm.type_;
	animation_phase_ = anm.animation_phase_;
	speed_ = anm.speed_;
	// Внутри функции устанавливается и start_object_ref_ 
	set_start_object(anm.start_object());
	points_ = anm.points_;
	cur_point_ = 0;
	
#ifdef _QUEST_EDITOR
	animation_scroll_phase_ = anm.animation_scroll_phase_;
#endif
	return *this;
}

qdCoordsAnimation::~qdCoordsAnimation()
{
	points_.clear();
}

void qdCoordsAnimation::set_start_object(const qdGameObject* p_obj)
{
	start_object_ = p_obj; 
	if (NULL != p_obj)
	{
		qdNamedObjectReference ref(p_obj);
		start_object_ref_ = ref;
	}
	else
		start_object_ref_.clear();
}

void qdCoordsAnimation::calc_paths() const
{
	for(size_t i = 1; i < points_.size(); i ++)
		points_[i].calc_path(points_[i - 1]);
}

void qdCoordsAnimation::add_point(const qdCoordsAnimationPoint* p)
{
	points_.push_back(*p);
	calc_paths();
}

void qdCoordsAnimation::insert_point(const qdCoordsAnimationPoint* p, int insert_pos)
{
	points_.insert(points_.begin()+insert_pos, *p);
	calc_paths();
}

void qdCoordsAnimation::remove_point(int num)
{
	assert(0 <= num && num < points_.size());

	points_.erase(points_.begin() + num);

	if(cur_point_ >= points_.size())
		cur_point_ = points_.size() - 1;

	calc_paths();
}

void qdCoordsAnimation::clear()
{
	if(!points_.empty())
		set_cur_point(0);
	cur_point_ = -1;
	points_.clear();
	clear_flags();
}

void qdCoordsAnimation::start() const
{
	cur_point_ = 0;

	if(!points_.empty())
	{
		qdGameObjectAnimated* p = object();
		if(!p) return;
		
		// Объект будет двигаться по шаблону =>
		// он выходит из состояния следования
		qdGameObjectMoving* obj = dynamic_cast<qdGameObjectMoving*>(p);
		if (NULL != obj)
			obj->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);

		// Если траектория должна выполняться относительно текущего положения (или
		// от центра заданного объекта), то считаем дельту 
		if (true == check_flag(QD_COORDS_ANM_RELATIVE_FLAG))
		{
			if (NULL != start_object_)
				del_ = points_[0].dest_pos() - start_object_->R();
			else del_ = points_[0].dest_pos() - p->R();
		}
		else
		{
			del_.x = 0;
			del_.y = 0;
			del_.z = 0;
		};

#ifdef _QUEST_EDITOR
		start_point_.set_dest_pos(points_[0].dest_pos());
		p -> set_pos(points_[0].dest_pos());
#else
		if(check_flag(QD_COORDS_ANM_OBJECT_START_FLAG))
			start_point_.set_dest_pos(p -> R());
		else
		{
			start_point_.set_dest_pos(points_[0].dest_pos() - del_);
			// Задана коорд. анимация с перемещением в точку и задан угол =>
			// устанавливаем направление для движущегося объекта
			qdGameObjectMoving* mov_obj = dynamic_cast<qdGameObjectMoving*>(obj);
			if ((NULL != mov_obj) && 
				(qdCoordsAnimationPoint::NO_DIRECTION != points_[0].direction_angle()))
				mov_obj->set_direction(points_[0].direction_angle());
		}
#endif //_QUEST_EDITOR

		points_[0].calc_path(start_point_, del_);

		points_[0].start();
		status_ = true;

		p -> set_pos(cur_pos());
	}

	is_finished_ = false;
}

void qdCoordsAnimation::stop() const
{
	status_ = false;
}

void qdCoordsAnimation::quant(float dt) const
{
	if(!status_ || !points_.size()) return;

	if(type_ == CA_INTERPOLATE_COORDS){
		float path = speed_ * dt;
		while(points_[cur_point_].move(path)){
			if(++cur_point_ >= points_.size()){
				is_finished_ = true;
				if(!check_flag(QD_COORDS_ANM_LOOP_FLAG)){
					cur_point_ --;
					stop();
					break;
				}

				start_point_.set_dest_pos(points_[points_.size() - 1].dest_pos() - del_);
				// Расстояние считаем, переместившись в глобальне координаты, так как points_ в глобальных
				points_[0].calc_path(start_point_, del_);

				cur_point_ = 0;
			}
			points_[cur_point_].start();
		}

		qdGameObjectAnimated* obj = object();
		if(obj)	obj -> set_pos(cur_pos());
	}
	else {
		if(type_ == CA_WALK){
			qdGameObjectAnimated* obj = object();
			if(obj -> named_object_type() != QD_NAMED_OBJECT_MOVING_OBJ) return;
			qdGameObjectMoving* p = static_cast<qdGameObjectMoving*>(obj);

			if(p -> is_in_position(points_[cur_point_].dest_pos() - del_)){
				if(++cur_point_ >= points_.size()){
					is_finished_ = true;
					if(!check_flag(QD_COORDS_ANM_LOOP_FLAG)){
						stop();
						return;
					}

					start_point_.set_dest_pos(points_[points_.size() - 1].dest_pos() - del_);
					// Расстояние считаем, переместившись в глобальне координаты
					points_[0].calc_path(start_point_, del_);

					cur_point_ = 0;
				}

				points_[cur_point_].start();
			}

			if(!p -> is_moving())
				p -> move(points_[cur_point_].dest_pos() - del_,points_[cur_point_].direction_angle());
		}
	}
}

void qdCoordsAnimation::load_script(const xml::tag* p)
{
	int v;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_NAME:
				set_name(it -> data());
				break;
			case QDSCR_COORDS_ANIMATION_POINT: {
					qdCoordsAnimationPoint fp;
					fp.load_script(&*it);
					add_point(&fp);
				}
				break;
			case QDSCR_TYPE:
				xml::tag_buffer(*it) > v;
				set_type((qdCoordsAnimationType)v);
				break;
			case QDSCR_FLAG:
				xml::tag_buffer(*it) > v;
				set_flag(v);
				break;
			case QDSCR_SPEED:
				xml::tag_buffer(*it) > speed_;
				break;
			case QDSCR_ANIMATION_PHASE:
				xml::tag_buffer(*it) > animation_phase_;
				break;
			case QDSCR_NAMED_OBJECT:
				start_object_ref_.load_script(&*it);
				break;
		}
	}
}

bool qdCoordsAnimation::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<coords_animation";

	if(name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";
	else
		fh < " name=\" \"";

	fh < " type=\"" <= type_ < "\"";
	fh < " speed=\"" <= speed_ < "\"";
	fh < " animation_phase=\"" <= animation_phase_ < "\"";

	fh < ">\r\n";

	qdCoordsAnimationPointVector::const_iterator it;
	FOR_EACH(points_,it)
		it -> save_script(fh,indent + 1);
	
	if(flags()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<flag>" <= flags() < "</flag>\r\n";
	}

	if (NULL != start_object_)
		start_object_ref_.save_script(fh,indent + 1);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</coords_animation>\r\n";

	return true;
}

Vect3f qdCoordsAnimation::cur_pos() const
{
	// В cur_pos() учитываем дельту относительных координат
	qdCoordsAnimationPoint p0;
	if (0 != cur_point_)
	{
		p0 = points_[cur_point_ - 1];
		p0.set_dest_pos(p0.dest_pos() - del_); // Перемещаемся в относительные координаты
	}
	else p0 = start_point_;
	qdCoordsAnimationPoint p1 = points_[cur_point_];
	p1.set_dest_pos(p1.dest_pos() - del_);     // Перемещаемся в относительные координаты

	Vect3f v = p0.dest_pos() + p1.passed_path() * (p1.dest_pos() - p0.dest_pos());
	return v;
}

qdGameObjectAnimated* qdCoordsAnimation::object() const
{
	if(owner() && owner() -> named_object_type() == QD_NAMED_OBJECT_OBJ_STATE){
		qdNamedObject* p = owner() -> owner();
		if(p && p -> named_object_type() == QD_NAMED_OBJECT_ANIMATED_OBJ || p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			return static_cast<qdGameObjectAnimated*>(p);
	}

	return 0;
}

bool qdCoordsAnimation::set_cur_point(int point_num) const
{
	if(!points_.size()) return false;

	start();

	if(type_ == CA_WALK){
		if(!object() || object() -> named_object_type() != QD_NAMED_OBJECT_MOVING_OBJ) return false;
		qdGameObjectMoving* p = static_cast<qdGameObjectMoving*>(object());
		p -> set_pos(cur_pos());
		p -> drop_flag(QD_OBJ_MOVING_FLAG);

		p -> move(points_[cur_point_].dest_pos());
		p -> skip_movement();

		for(int i = 0; i < point_num; i ++){
			if(++cur_point_ >= points_.size()){
				if(!check_flag(QD_COORDS_ANM_LOOP_FLAG)){
					stop();
					return false;
				}

				start_point_.set_dest_pos(points_[points_.size() - 1].dest_pos());
				points_[0].calc_path(start_point_);

				cur_point_ = 0;
			}

			points_[cur_point_].start();

			if(!p -> move(points_[cur_point_].dest_pos())) return false;
			if(!p -> skip_movement()) return false;
		}

		return true;
	}
	else {
		if(!object() || speed_ < 0.01f) return false;
		qdGameObjectAnimated* p = object();
		p -> get_animation() -> set_time_rel(animation_phase());
		p -> set_pos(cur_pos());

		for(int i = 0; i < point_num; i ++){
			if(++cur_point_ >= points_.size()){
				if(!check_flag(QD_COORDS_ANM_LOOP_FLAG)){
					stop();
#ifdef _QUEST_EDITOR
					cur_point_ = (points_.size())?points_.size()-1 : 0;
#endif // _QUEST_EDITOR
					if(p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
						static_cast<qdGameObjectMoving*>(p) -> adjust_z();

					return false;
				}

				start_point_.set_dest_pos(points_[points_.size() - 1].dest_pos());
				points_[0].calc_path(start_point_);

				cur_point_ = 0;
			}

			points_[cur_point_].start();

			p -> set_pos(points_[cur_point_].dest_pos());
			p -> get_animation() -> advance_time(points_[cur_point_].path_length() / speed_);
		}

		if(p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			static_cast<qdGameObjectMoving*>(p) -> adjust_z();

		return true;
	}
/*
		const float dt = 0.005f;
		while(cur_point_ < point_num || points_[cur_point_].passed_path() < 0.99f){
			quant(dt);
			p -> quant(dt);
		}

		return true;
	}
*/
	return false;
}

bool qdCoordsAnimation::reset_cur_point() const
{
	return set_cur_point(cur_point_);
}

#ifdef _QUEST_EDITOR
bool qdCoordsAnimation::change_animation_frame(bool direction) const
{
	qdGameObjectAnimated* p = object();
	if(p && !p -> get_animation() -> is_empty()){
		int frame_idx = p -> get_animation() -> get_cur_frame_number();
		frame_idx += (direction) ? 1 : -1;

		if(frame_idx < 0) frame_idx = p -> get_animation() -> num_frames() - 1;
		if(frame_idx >= p -> get_animation() -> num_frames()) frame_idx = p -> get_animation() -> num_frames() - 1;

		float phase = p -> get_animation() -> cur_time_rel();
		p -> get_animation() -> set_cur_frame(frame_idx);


		animation_scroll_phase_ += p -> get_animation() -> cur_time_rel() - phase;
		if (animation_scroll_phase_> 1.f)
			animation_scroll_phase_ = fmodf(animation_scroll_phase_, 1.f);
		else if (animation_scroll_phase_< 0.f) 
			animation_scroll_phase_ = 1.f + fmodf(animation_scroll_phase_, 1.f);


		return true;
	}

	return false;
}
#endif

void qdCoordsAnimation::set_time_rel(float tm)
{
	assert(tm >= 0.0f && tm <= 1.0f);

	if(!points_.size()) return;

	int pt = tm * float(points_.size() - 1);
	set_cur_point(pt);
}

bool qdCoordsAnimation::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdNamedObject::load_data(fh,save_version)) return false;

	int v;
	fh > v;
	status_ = (v) ? true : false;

	fh > v;
	is_finished_ = (v) ? true : false;

	fh > cur_point_ > v;
	if(points_.size() != v) return false;

	if(save_version >= 101)
		fh > del_.x > del_.y > del_.z;
	else
		del_ = Vect3f::ZERO;

	for(qdCoordsAnimationPointVector::iterator it = points_.begin(); it != points_.end(); ++it)
		it -> load_data(fh,save_version);

	start_point_.load_data(fh,save_version);

	Vect3f vec;
	fh > vec.x > vec.y > vec.z;
	start_point_.set_dest_pos(vec);

	return true;
}

bool qdCoordsAnimation::save_data(qdSaveStream& fh) const
{
	if(!qdNamedObject::save_data(fh)) return false;

	fh < static_cast<int>(status_) < static_cast<int>(is_finished_) < cur_point_ < points_.size();

	fh < del_.x < del_.y < del_.z;

	for(qdCoordsAnimationPointVector::const_iterator it = points_.begin(); it != points_.end(); ++it)
		it -> save_data(fh);

	start_point_.save_data(fh);
	fh < start_point_.dest_pos().x < start_point_.dest_pos().y < start_point_.dest_pos().z;

	return true;
}
