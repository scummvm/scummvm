#ifndef __QD_COORDS_ANIMATION_H__
#define __QD_COORDS_ANIMATION_H__

#include "xml_fwd.h"

#include "qd_named_object.h"
#include "qd_coords_animation_point.h"
#include "qd_named_object_reference.h"

const QD_COORDS_ANM_OBJECT_START_FLAG	= 0x01;
const QD_COORDS_ANM_LOOP_FLAG			= 0x02;
const QD_COORDS_ANM_RELATIVE_FLAG		= 0x04; // Флаг перемещения траектории в текущую точку нахождения объекта

class qdCoordsAnimation : public qdNamedObject
{
public:
	enum qdCoordsAnimationType
	{
		CA_INTERPOLATE_COORDS,
		CA_WALK
	};

	qdCoordsAnimation();
	qdCoordsAnimation(const qdCoordsAnimation& anm);
	~qdCoordsAnimation();

	qdCoordsAnimation& operator = (const qdCoordsAnimation& anm);

	int named_object_type() const { return QD_NAMED_OBJECT_COORDS_ANIMATION; }

	float speed() const { return speed_; }
	void set_speed(float sp){ speed_ = sp; }

	const qdGameObject* start_object() const { return start_object_; }
	void set_start_object(const qdGameObject* p_obj);
	const qdNamedObjectReference& start_object_ref() const{ return start_object_ref_; }

	bool is_playing() const { return status_; }
	bool is_finished() const { return is_finished_; }

	qdCoordsAnimationType type() const { return type_; }
	void set_type(qdCoordsAnimationType tp){ type_ = tp; }

	float animation_phase() const { 
#ifdef _QUEST_EDITOR
		return animation_phase_ + animation_scroll_phase_; 
#else
		return animation_phase_; 
#endif
	}
	void set_animation_phase(float p){ animation_phase_ = p; }

	void start() const;
	void stop() const;
	void quant(float dt) const;

	bool reset_cur_point() const;
	bool set_cur_point(int point_num) const;
	int get_cur_point() const { return cur_point_; }

	void set_time_rel(float tm);

	void add_point(const qdCoordsAnimationPoint* p);
	void insert_point(const qdCoordsAnimationPoint* p, int insert_pos);

	void remove_point(int num);
	void clear();
	
	qdCoordsAnimationPoint* get_point(int index = 0){ return &points_[index]; };
	const qdCoordsAnimationPoint* get_point(int index = 0) const { return &points_[index]; };

	int size() const { return points_.size(); }
	
	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;
	
	bool is_empty() const { if(points_.empty()) return true; return false; }
	
#ifdef _QUEST_EDITOR
	// direction: true = next frame, false = prev frame
	bool change_animation_frame(bool direction = true) const; 
	void reset_scroll() const{
		animation_scroll_phase_ = 0.f;
	}
#endif
	
	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

private:

	qdCoordsAnimationType type_;
	qdCoordsAnimationPointVector points_;
	float animation_phase_;
	float speed_;

	// Для относительного перемещения персонажа (QD_COORDS_ANM_RELATIVE_FLAG):
	// объект, относительно коориднат которого будет работать траектория движения (если не задано, то работаем
	// относительно координат объекта-владельца координатной анимации)
	const qdGameObject* start_object_;
	qdNamedObjectReference start_object_ref_;

#ifdef _QUEST_EDITOR
	//исопользуется для прокрутки анимации
	mutable float animation_scroll_phase_;
#endif
	mutable bool status_;
	mutable bool is_finished_;
	mutable qdCoordsAnimationPoint start_point_;
	mutable int cur_point_;

	Vect3f cur_pos() const;
	class qdGameObjectAnimated* object() const;
	
	void calc_paths() const;

	// Дельта (вектор) смещения координат анимации
	mutable Vect3f del_;
};

//typedef std::list<qdCoordsAnimation*> qdCoordsAnimationList;

#endif /* __QD_COORDS_ANIMATION_H__ */

