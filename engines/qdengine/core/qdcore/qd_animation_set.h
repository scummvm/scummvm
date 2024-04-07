#ifndef __QD_ANIMATION_SET_H__
#define __QD_ANIMATION_SET_H__

#include "xml_fwd.h"

#include "qd_named_object.h"
#include "qd_animation.h"
#include "qd_animation_info.h"

class qdAnimationSet : public qdNamedObject
{
public:
	qdAnimationSet();
	qdAnimationSet(const qdAnimationSet& set);
	~qdAnimationSet();

	qdAnimationSet& operator = (const qdAnimationSet& set);

	int named_object_type() const { return QD_NAMED_OBJECT_ANIMATION_SET; }

	int size() const { return animations_.size(); }
	void resize(int sz);

	//! выдает индекс походки и остановки, соответствующих углу
	int get_angle_index(float direction_angle) const;

	//! выдает угол походки и остановки, соответствующих индексу
	float get_index_angle(int direction_index) const;

	//! Возвращает ближайшее к angle направление, для которого есть анимация.
	float adjust_angle(float angle) const;

	qdAnimationInfo* get_animation_info(int index = 0);
	qdAnimationInfo* get_animation_info(float direction_angle);

	qdAnimationInfo* get_static_animation_info(int index = 0);
	qdAnimationInfo* get_static_animation_info(float direction_angle);

	qdAnimationInfo* get_start_animation_info(int index = 0);
	qdAnimationInfo* get_start_animation_info(float direction_angle);

	qdAnimationInfo* get_stop_animation_info(int index = 0);
	qdAnimationInfo* get_stop_animation_info(float direction_angle);

	qdAnimation* get_turn_animation() const;
	qdAnimationInfo* get_turn_animation_info(){ return &turn_animation_; }
	void set_turn_animation(const char* animation_name){ turn_animation_.set_animation_name(animation_name); }

	const float walk_sound_frequency(int direction_index) const;
	const float walk_sound_frequency(float direction_angle) const;
	void set_walk_sound_frequency(int direction_index,float freq);

	bool load_animations(const qdNamedObject* res_owner);
	bool free_animations(const qdNamedObject* res_owner);
	//! Регистрация ресурсов набора в диспетчере ресурсов.
	bool register_resources(const qdNamedObject* res_owner);
	//! Отмена регистрации ресурсов набора в диспетчере ресурсов.
	bool unregister_resources(const qdNamedObject* res_owner);

	bool scale_animations(float coeff_x,float coeff_y);

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;
	
	float start_angle() const{
		return start_angle_;
	}
	void set_start_angle(float v) {
		start_angle_ = v;
	}

private:

	//! Угол, соответствующий первому направлению в походке.
	/**
	Если нулевой - первое направление вправо.
	*/
	float start_angle_;

	qdAnimationInfoVector animations_;
	qdAnimationInfoVector static_animations_;
	qdAnimationInfoVector start_animations_;
	qdAnimationInfoVector stop_animations_;

	/// анимация поворота, полный оборот начиная с направления вправо
	qdAnimationInfo turn_animation_;

	//! Коэффициенты для частоты звука походки.
	std::vector<float> walk_sound_frequency_;

	static int get_angle_index(float direction_angle, int dir_count);
	static float get_index_angle(int index, int dir_count);
};

#endif /* __QD_ANIMATION_SET_H__ */
