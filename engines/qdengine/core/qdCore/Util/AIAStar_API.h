#ifndef __AIASTAR_API_H__
#define __AIASTAR_API_H__

#include "AIAStar.h"

class qdCamera;
class qdGameObjectMoving;

//! Ёвристика дл€ поиска пути.
class qdHeuristic
{
public:
	qdHeuristic();
	~qdHeuristic();

	int GetH(int x,int y);
	int GetG(int x1,int y1,int x2,int y2);
	bool IsEndPoint(int x,int y){ return (x == target_.x && y == target_.y); }

	void init(const Vect3f trg);
	void set_camera(const qdCamera* cam){ camera_ptr_ = cam; }
	void set_object(const qdGameObjectMoving* obj){ object_ptr_ = obj; }

private:

	Vect2i target_;
	Vect3f target_f_;

	const qdCamera* camera_ptr_;
	const qdGameObjectMoving* object_ptr_;
};

typedef AIAStar<qdHeuristic,int> qdAStar;

#endif /* __AIASTAR_API_H__ */
