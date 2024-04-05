#ifndef __QD_COORDS_ANIMATION_FRAME_H__
#define __QD_COORDS_ANIMATION_FRAME_H__

#include <vector>

#include "xml_fwd.h"

class XStream;

class qdCoordsAnimationPoint
{
public:
	qdCoordsAnimationPoint();
	~qdCoordsAnimationPoint();

	static const float NO_DIRECTION;
	
	const Vect3f& dest_pos() const { return pos_; }
	void set_dest_pos(const Vect3f& r){ pos_ = r; }

	float direction_angle() const { return direction_angle_; }
	void set_direction_angle(float ang){ direction_angle_ = ang; }

	void start() const { passed_path_length_ = 0.0f; }

	bool move(float& path) const { 
		passed_path_length_ += path;
		if(passed_path_length_ >= path_length_){
			path = passed_path_length_ - path_length_;
			return true;
		}
		return false;
	}

	void calc_path(const qdCoordsAnimationPoint& p, const Vect3f& shift = Vect3f::ZERO) const;
	float passed_path() const;
	float path_length() const { return path_length_; }

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

private:

	Vect3f pos_;
	float direction_angle_;

	mutable float path_length_;
	mutable float passed_path_length_;
};

typedef std::vector<qdCoordsAnimationPoint> qdCoordsAnimationPointVector;

#endif /* __QD_COORDS_ANIMATION_FRAME_H__ */

