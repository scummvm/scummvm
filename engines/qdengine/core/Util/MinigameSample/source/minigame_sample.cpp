
#include <list>
#include <fstream>

#include <math.h>
#include <stdio.h>

#include "qd_minigame_interface.h"

class minigameSample : public qdMiniGameInterface
{
public:
	minigameSample();
	~minigameSample(){ }

	/// Инициализация игры.
	bool init(const qdEngineInterface* p);

	/// Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt);

	/// Деинициализация игры.
	bool finit();
private:

	qdMinigameObjectInterface* ball_object_;
	qdMinigameObjectInterface* board_object_;
	qdMinigameObjectInterface* field_object_;

	enum {
		BALL_LOCKED,
		BALL_FREE
	} ball_movement_mode_;

	mgVect2f ball_speed_;
	mgVect2f ball_speed_initial_;
	mgVect2f ball_speed_initial_rnd_;

	typedef std::list<qdMinigameObjectInterface*> brick_objects_container_t;
	brick_objects_container_t brick_objects_;

	const qdEngineInterface* engine_;
	qdMinigameSceneInterface* scene_;

	bool constrain_object_position(qdMinigameObjectInterface* object, const qdMinigameObjectInterface* bound_object);
	bool ball_collision(qdMinigameObjectInterface* object);
	bool vertical_edge_collision(const mgVect3f& v0, const mgVect3f& v1) const;
	bool horizontal_edge_collision(const mgVect3f& v0, const mgVect3f& v1) const;
};

extern "C" {
__declspec(dllexport) qdMiniGameInterface* open_game_interface(const char*);
__declspec(dllexport) bool close_game_interface(qdMiniGameInterface* p);
};

qdMiniGameInterface* open_game_interface(const char*)
{
	return new minigameSample;
}

bool close_game_interface(qdMiniGameInterface* p)
{
	delete p;
	return true;
}

minigameSample::minigameSample() : ball_object_(0),
	board_object_(0),
	field_object_(0),
	ball_movement_mode_(BALL_LOCKED)
{
}

bool minigameSample::init(const qdEngineInterface* p)
{
	engine_ = p;
	if(scene_ = p -> current_scene_interface()){
		ball_object_ = scene_ -> object_interface(scene_ -> minigame_parameter("ball_object_name"));
		board_object_ = scene_ -> object_interface(scene_ -> minigame_parameter("board_object_name"));
		field_object_ = scene_ -> object_interface(scene_ -> minigame_parameter("field_object_name"));

		sscanf(scene_ -> minigame_parameter("ball_speed"),"%f%f",&ball_speed_initial_.x,&ball_speed_initial_.y);
		sscanf(scene_ -> minigame_parameter("ball_speed_rnd"),"%f%f",&ball_speed_initial_rnd_.x,&ball_speed_initial_rnd_.y);

		float x = field_object_ -> R().x;
		float y = field_object_ -> R().y - field_object_ -> bound().y/2;

		board_object_ -> set_R(mgVect3f(x,y,0));

		y += ball_object_ -> bound().y/2 + board_object_ -> bound().y/2;
		ball_object_ -> set_R(mgVect3f(x,y,0));

		ball_movement_mode_ = BALL_LOCKED;

		char name[256];
		mgVect3f v0 = field_object_ -> R() - field_object_ -> bound()/2;
		v0 += mgVect3f(16,field_object_ -> bound().y - 32,0);

		for(int i = 0; i < 10; i ++){
			sprintf(name,"%s%.4d",scene_ -> minigame_parameter("brick_object"),i);
			if(qdMinigameObjectInterface* obj = scene_ -> object_interface(name)){
				obj -> set_state(3);
				obj -> set_R(v0 + obj -> bound()/2);
				v0.x += obj -> bound().x + 6;

				brick_objects_.push_back(obj);
			}
		}
	}
	else
		return false;

	return true;
}

bool minigameSample::quant(float dt)
{
	mgVect3f r = board_object_ -> R();
	r.x = scene_ -> screen2grid_coords(engine_ -> mouse_cursor_position()).x;
	board_object_ -> set_R(r);
	constrain_object_position(board_object_, field_object_);

	switch(ball_movement_mode_){
	case BALL_LOCKED:
		r = board_object_ -> R();
		r.y += ball_object_ -> bound().y/2 + board_object_ -> bound().y/2;
		ball_object_ -> set_R(r);
		if(engine_->is_mouse_event_active(qdEngineInterface::MOUSE_EV_LEFT_DOWN)){
			ball_movement_mode_ = BALL_FREE;

			ball_speed_ = ball_speed_initial_;
			ball_speed_.x += engine_ -> frnd(ball_speed_initial_rnd_.x);
			ball_speed_.y += engine_ -> frnd(ball_speed_initial_rnd_.y);
		}
		break;
	case BALL_FREE:
		r = ball_object_ -> R();
		r.x += ball_speed_.x * dt;
		r.y += ball_speed_.y * dt;
		ball_object_ -> set_R(r);

		ball_collision(board_object_);
		for(brick_objects_container_t::iterator it = brick_objects_.begin(); it != brick_objects_.end(); ++it){
			if((*it) -> is_visible() && ball_collision(*it)){
				int idx = (*it) -> current_state_index();
				(*it) -> set_state(idx ? idx - 1 : 0);
			}
		}

		if(r.x > field_object_ -> R().x + field_object_ -> bound().x/2 - ball_object_ -> bound().x/2 && ball_speed_.x > 0)
			ball_speed_.x = -ball_speed_.x + engine_ -> frnd(1);
		if(r.x < field_object_ -> R().x - field_object_ -> bound().x/2 + ball_object_ -> bound().x/2 && ball_speed_.x < 0)
			ball_speed_.x = -ball_speed_.x + engine_ -> frnd(1);

		if(r.y > field_object_ -> R().y + field_object_ -> bound().y/2 - ball_object_ -> bound().y/2 && ball_speed_.y > 0)
			ball_speed_.y = -ball_speed_.y + engine_ -> frnd(1);
		if(r.y < field_object_ -> R().y - field_object_ -> bound().y/2 - ball_object_ -> bound().y)
			ball_movement_mode_ = BALL_LOCKED;
		break;
	}

	return true;
}

bool minigameSample::finit()
{
	engine_ -> release_scene_interface(scene_);
	return true;
}

bool minigameSample::constrain_object_position(qdMinigameObjectInterface* object, const qdMinigameObjectInterface* bound_object)
{
	bool ret = false;

	mgVect3f r = object -> R();
	mgVect3f bound = object -> bound();

	mgVect3f r0 = bound_object -> R();
	mgVect3f bound0 = bound_object -> bound();

	if(r.x - bound.x/2 < r0.x - bound0.x/2){
		ret = true;
		r.x = r0.x - bound0.x/2 + bound.x/2;
	}
	if(r.x + bound.x/2 > r0.x + bound0.x/2){
		ret = true;
		r.x = r0.x + bound0.x/2 - bound.x/2;
	}

	if(r.y - bound.y/2 < r0.y - bound0.y/2){
		ret = true;
		r.y = r0.y - bound0.y/2 + bound.y/2;
	}
	if(r.y + bound.y/2 > r0.y + bound0.y/2){
		ret = true;
		r.y = r0.y + bound0.y/2 - bound.y/2;
	}

	if(ret)
		object -> set_R(r);

	return ret;
}

bool minigameSample::ball_collision(qdMinigameObjectInterface* object)
{
	bool ret = false;

	mgVect3f d = object -> R() - ball_object_ -> R();
	mgVect3f bound = ball_object_ -> bound()/2 + object -> bound()/2;

	if(fabs(d.x) < bound.x && fabs(d.y) < bound.y){
		if(d.x < 0 && ball_speed_.x < 0){
			mgVect3f v0 = object -> R() + object->bound()/2;
			mgVect3f v1 = object -> R() + object->bound()/2;

			v1.y -= object -> bound().y;

			if(vertical_edge_collision(v0, v1)){
				ret = true;
				ball_speed_.x = -ball_speed_.x + engine_ -> frnd(1);
			}
		}
		if(d.x > 0 && ball_speed_.x > 0){
			mgVect3f v0 = object -> R() - object->bound()/2;
			mgVect3f v1 = object -> R() - object->bound()/2;

			v1.y += object -> bound().y;

			if(vertical_edge_collision(v0, v1)){
				ret = true;
				ball_speed_.x = -ball_speed_.x + engine_ -> frnd(1);
			}
		}

		if(d.y < 0 && ball_speed_.y < 0){
			mgVect3f v0 = object -> R() + object->bound()/2;
			mgVect3f v1 = object -> R() + object->bound()/2;

			v1.x -= object -> bound().x;

			if(horizontal_edge_collision(v0, v1)){
				ret = true;
				ball_speed_.y = -ball_speed_.y + engine_ -> frnd(1);
			}
		}
		if(d.y > 0 && ball_speed_.y > 0){
			mgVect3f v0 = object -> R() - object->bound()/2;
			mgVect3f v1 = object -> R() - object->bound()/2;

			v1.x += object -> bound().x;

			if(horizontal_edge_collision(v0, v1)){
				ret = true;
				ball_speed_.y = -ball_speed_.y + engine_ -> frnd(1);
			}
		}
	}

	return ret;
}

bool minigameSample::vertical_edge_collision(const mgVect3f& v0, const mgVect3f& v1) const
{
	float y = ball_object_ -> R().y;
	float y0 = (v0.y < v1.y) ? v0.y : v1.y;
	float y1 = (v0.y < v1.y) ? v1.y : v0.y;

	float r = ball_object_ -> bound().x/2;
	float r2 = r * r;

	float dx = fabs(ball_object_ -> R().x - v0.x);

	if(y < y0){
		return (dx * dx + (y - y0) * (y - y0) < r2);
	}
	else {
		if(y < y1){
			return (dx < r);
		}
		else {
			return (dx * dx + (y - y1) * (y - y1) < r2);
		}
	}

	return false;
}

bool minigameSample::horizontal_edge_collision(const mgVect3f& v0, const mgVect3f& v1) const
{
	float x = ball_object_ -> R().x;
	float x0 = (v0.x < v1.x) ? v0.x : v1.x;
	float x1 = (v0.x < v1.x) ? v1.x : v0.x;

	float r = ball_object_ -> bound().x/2;
	float r2 = r * r;

	float dy = fabs(ball_object_ -> R().y - v0.y);

	if(x < x0){
		return (dy * dy + (x - x0) * (x - x0) < r2);
	}
	else {
		if(x < x1){
			return (dy < r);
		}
		else {
			return (dy * dy + (x - x1) * (x - x1) < r2);
		}
	}

	return false;
}
