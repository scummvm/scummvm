/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_fwd.h"

#include "qd_minigame.h"
#include "qd_counter.h"
#include "qd_minigame_interface.h"
#include "qd_engine_interface.h"

#include "gr_dispatcher.h"

#include "qd_rnd.h"

#include "qd_game_dispatcher.h"
#include "qd_game_scene.h"
#include "qd_game_object_mouse.h"
#include "qd_game_object_animated.h"
#include "qd_game_object_moving.h"

#include "qd_interface_dispatcher.h"
#include "qd_interface_screen.h"
#include "qd_interface_text_window.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */

namespace qdmg {

class qdMinigameObjectInterfaceImplBase : public qdMinigameObjectInterface
{
public:
	qdMinigameObjectInterfaceImplBase(qdGameObjectAnimated* object) : object_(object) { assert(object_); }

	/// Имя объекта.
	const char* name() const;

	//! Возвращает true, если у объекта есть состояние с именем state_name.
	bool has_state(const char* state_name) const;
	//! Возвращает имя активного в данный момент состояния.
	//! Вернёт 0, если активного состояния нету.
	const char* current_state_name() const;
	//! Возвращает true, если состояние с именем state_name включено в данный момент.
	bool is_state_active(const char* state_name) const;
	//! Возвращает true, если состояние с именем state_name в данный момент ожидает активации.
	bool is_state_waiting_activation(const char* state_name) const;
	//! Возвращает номер включенного в данный момент состояния.
	int current_state_index() const;
	//! Включает состояние с именем state_name.
	bool set_state(const char* state_name);
	//! Включает состояние номер state_index (отсчитывается от нуля).
	bool set_state(int state_index);
	//! Возвращает номер состояния с именем state_name.
	/**
	Отсчитывается от нуля, если такого состояния нет, то
	возвращает -1.
	*/
	int state_index(const char* state_name) const;

	//! Возвращает координаты объекта в мировой системе координат.
	mgVect3f R() const;
	//! Устанавливает координаты объекта в мировой системе координат.
	void set_R(const mgVect3f& r);

	/// Проверка, попадает ли точка с экранными координатами pos в объект
	bool hit_test(const mgVect2i& pos) const;

	//! Возвращает координаты объекта в экранной системе координат.
	mgVect2i screen_R() const;
	//! Обновляет координаты объекта в экранной системе координат.
	bool update_screen_R();
	//! Возвращает текущие экранные размеры объекта в пикселах.
	mgVect2i screen_size() const;

	//! Устанавливает поворот картинки объекта.
	//! angle - угол, на который должна быть повёрнута картинка, в радианах
	//! speed - скорость поворота, в радианах в секунду
	void set_screen_rotation(float angle, float speed);
	//! Возвращает поворот картинки объекта в радианах.
	float screen_rotation() const;
	//! Устанавливает масштабирование картинки объекта.
	void set_screen_scale(const mgVect2f& scale, const mgVect2f& speed);
	//! Возвращает масштаб картинки объекта.
	mgVect2f screen_scale() const;

	//! Возвращает цвет затенения.
	int shadow_color() const;
	//! Возвращает прозрачность затенения, значения - [0, 255], если равно -1, то затенения нет.
	int shadow_alpha() const;
	//! Устанавливает затенение.
	bool set_shadow(int shadow_color, int shadow_alpha);

	//! Возвращает true, если объект не спрятан.
	virtual bool is_visible() const;

	//! Возвращает размеры объекта в мировой системе координат.
	mgVect3f bound() const;

private:

	qdGameObjectAnimated* object_;
};

class qdMinigameObjectInterfaceImpl : public qdMinigameObjectInterfaceImplBase
{
public:
	qdMinigameObjectInterfaceImpl(qdGameObjectAnimated* object) : qdMinigameObjectInterfaceImplBase(object) { }

	//! Команда персонажу идти к точке target_position.
	/**
	Если второй параметр равен false, то если target_position непроходима
	персонаж идёт к ближайшей от target_position проходимой точке.
	*/
	bool move(const mgVect3f& target_position,bool disable_target_change = false){ return false; }

	float direction_angle() const { return 0.f; }
	bool set_direction_angle(float direction){ return false; }
};

//! Интерфейс к персонажу.
class qdMinigamePersonageInterfaceImpl : public qdMinigameObjectInterfaceImplBase
{
public:
	qdMinigamePersonageInterfaceImpl(qdGameObjectMoving* p);

	//! Команда персонажу идти к точке target_position.
	/**
	Если второй параметр равен false, то если target_position непроходима
	персонаж идёт к ближайшей от target_position проходимой точке.
	*/
	bool move(const mgVect3f& target_position,bool disable_target_change = false);

	float direction_angle() const;
	bool set_direction_angle(float direction);

private:

	qdGameObjectMoving* personage_object_;
};

//! Интерфейс к сцене.
class qdMinigameSceneInterfaceImpl : public qdMinigameSceneInterface
{
public:
	qdMinigameSceneInterfaceImpl(qdGameScene* scene);

	/// Имя сцены.
	const char* name() const;

	//! Создаёт интерфейс к объекту с именем object_name.
	qdMinigameObjectInterface* object_interface(const char* object_name);
	//! Создаёт интерфейс к персонажу с именем personage_name.
	qdMinigameObjectInterface* personage_interface(const char* personage_name);
	//! Активация персонажа с именем personage_name.
	bool activate_personage(const char* personage_name);

	//! Преобразование из экранных координат в мировые.
	mgVect3f screen2world_coords(const mgVect2i& screen_pos,float screen_depth = 0) const;
	//! Преобразование из мировых координат в экранные.
	mgVect2i world2screen_coords(const mgVect3f& world_pos) const;
	//! Возвращает "глубину" точки с координатами pos в мировой системе координат.
	float screen_depth(const mgVect3f& pos) const;
	//! Возвращает мировые координаты точки на сетке по её экранным координатам.
	mgVect3f screen2grid_coords(const mgVect2i& screen_pos) const;

	//! Создаёт интерфейс к объекту, который взят мышью в данный момент.
	qdMinigameObjectInterface* mouse_object_interface() const;
	//! Создаёт интерфейс к объекту, по которому кликнули мышью.
	qdMinigameObjectInterface* mouse_click_object_interface() const;
	//! Создаёт интерфейс к объекту, по которому кликнули правой кнопкой мыши.
	qdMinigameObjectInterface* mouse_right_click_object_interface() const;
	//! Создаёт интерфейс к объекту, над которым находится мышиный курсор.
	qdMinigameObjectInterface* mouse_hover_object_interface() const;

	const char* minigame_parameter(const char* parameter_name) const;

	void release_object_interface(qdMinigameObjectInterface* p) const;

private:

	qdGameScene* scene_;
};

/// Интерфейс к счётчику.
class qdMinigameCounterInterfaceImpl : public qdMinigameCounterInterface
{
public:
	qdMinigameCounterInterfaceImpl(qdCounter* counter) : counter_(counter) { assert(counter_); }

	/// возвращает текущее значение счётчика
	int value() const;
	/// устанавливает текущее значение счётчика
	void set_value(int value);
	/// добавляет к текущему значению счётчика value_delta
	void add_value(int value_delta);
private:

	qdCounter* counter_;
};

}; // namespace qdmg

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace qdmg {

qdMinigameSceneInterfaceImpl::qdMinigameSceneInterfaceImpl(qdGameScene* scene) : scene_(scene)
{
	assert(scene_);
}

const char* qdMinigameSceneInterfaceImpl::minigame_parameter(const char* parameter_name) const
{
#ifndef _QUEST_EDITOR
	if(const qdMiniGame* p = scene_ -> minigame())
		return p -> config_parameter_value(parameter_name);
#endif
	return NULL;
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::mouse_object_interface() const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		if(dp -> mouse_object() -> object())
			return new qdMinigameObjectInterfaceImpl(dp -> mouse_object() -> object());
	}

	return NULL;
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::mouse_click_object_interface() const
{
	if(qdNamedObject* p = scene_->mouse_click_object()){
		if(qdGameObjectAnimated* obj = dynamic_cast<qdGameObjectAnimated*>(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::mouse_right_click_object_interface() const
{
	if(qdNamedObject* p = scene_->mouse_right_click_object()){
		if(qdGameObjectAnimated* obj = dynamic_cast<qdGameObjectAnimated*>(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::mouse_hover_object_interface() const
{
	if(qdNamedObject* p = scene_->mouse_hover_object()){
		if(qdGameObjectAnimated* obj = dynamic_cast<qdGameObjectAnimated*>(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

mgVect3f qdMinigameSceneInterfaceImpl::screen2world_coords(const mgVect2i& screen_pos,float screen_depth) const
{
	const qdCamera* cp = scene_ -> get_camera();
	Vect3f pos = cp -> rscr2global(cp -> scr2rscr(Vect2s(screen_pos.x,screen_pos.y)),screen_depth);

	return mgVect3f(pos.x,pos.y,pos.z);
}

mgVect2i qdMinigameSceneInterfaceImpl::world2screen_coords(const mgVect3f& world_pos) const
{
	const qdCamera* cp = scene_ -> get_camera();
	Vect3f v = cp -> global2camera_coord(Vect3f(world_pos.x,world_pos.y,world_pos.z));
	Vect2i screen_pos = cp -> camera_coord2scr(v);

	return mgVect2i(screen_pos.x,screen_pos.y);
}

float qdMinigameSceneInterfaceImpl::screen_depth(const mgVect3f& pos) const
{
	const qdCamera* cp = scene_ -> get_camera();
	Vect3f v = cp -> global2camera_coord(Vect3f(pos.x,pos.y,pos.z));

	return v.z;
}

mgVect3f qdMinigameSceneInterfaceImpl::screen2grid_coords(const mgVect2i& screen_pos) const
{
	const qdCamera* cp = scene_ -> get_camera();
	Vect3f pos = cp -> scr2plane(Vect2s(screen_pos.x,screen_pos.y));

	return mgVect3f(pos.x,pos.y,pos.z);
}

void qdMinigameSceneInterfaceImpl::release_object_interface(qdMinigameObjectInterface* p) const
{
	delete p;
}

const char* qdMinigameSceneInterfaceImpl::name() const
{
	return scene_->name();
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::object_interface(const char* object_name)
{
	if(qdGameObjectAnimated* p = dynamic_cast<qdGameObjectAnimated*>(scene_ -> get_object(object_name)))
		return new qdMinigameObjectInterfaceImpl(p);

	return NULL;
}

qdMinigameObjectInterface* qdMinigameSceneInterfaceImpl::personage_interface(const char* personage_name)
{
	if(qdGameObjectMoving* p = dynamic_cast<qdGameObjectMoving*>(scene_ -> get_object(personage_name)))
		return new qdMinigamePersonageInterfaceImpl(p);

	return NULL;
}

bool qdMinigameSceneInterfaceImpl::activate_personage(const char* personage_name)
{
	if(qdGameObjectMoving* p = dynamic_cast<qdGameObjectMoving*>(scene_ -> get_object(personage_name))){
		scene_->set_active_personage(p);
		return true;
	}

	return false;
}

qdMinigamePersonageInterfaceImpl::qdMinigamePersonageInterfaceImpl(qdGameObjectMoving* object) : qdMinigameObjectInterfaceImplBase(object), personage_object_(object)
{
	assert(personage_object_);
}

bool qdMinigamePersonageInterfaceImpl::move(const mgVect3f& target_position,bool disable_target_change)
{
	if(personage_object_){
		Vect3f target(target_position.x,target_position.y,target_position.z);
		return personage_object_ -> move(target,disable_target_change);
	}

	return false;
}

float qdMinigamePersonageInterfaceImpl::direction_angle() const
{
	if(personage_object_)
		return personage_object_->direction_angle();

	return 0.f;
}

bool qdMinigamePersonageInterfaceImpl::set_direction_angle(float direction)
{
	if(personage_object_)
		return personage_object_->set_direction(direction);

	return false;
}

mgVect3f qdMinigameObjectInterfaceImplBase::bound() const
{
	if(object_){
		Vect3f b = object_ -> bound();
		return mgVect3f(b.x,b.y,b.z);
	}

	return mgVect3f(0,0,0);
}

const char* qdMinigameObjectInterfaceImplBase::name() const
{
	if(object_)
		return object_->name();

	return 0;
}

bool qdMinigameObjectInterfaceImplBase::has_state(const char* state_name) const
{
	if(object_ && object_ -> get_state(state_name))
		return true;

	return false;
}

const char* qdMinigameObjectInterfaceImplBase::current_state_name() const
{
	if(object_){
		if(const qdGameObjectState* p = object_->get_cur_state())
			return p->name();
	}

	return 0;
}

bool qdMinigameObjectInterfaceImplBase::is_state_active(const char* state_name) const
{
	return object_ -> is_state_active(state_name);
}

bool qdMinigameObjectInterfaceImplBase::is_state_waiting_activation(const char* state_name) const
{
	return object_ -> is_state_waiting(state_name);
}

int qdMinigameObjectInterfaceImplBase::current_state_index() const
{
	return object_ -> cur_state();
}

bool qdMinigameObjectInterfaceImplBase::set_state(const char* state_name)
{
	int idx = state_index(state_name);
	if(idx != -1){
		object_ -> set_state(idx);
		return true;
	}

	return false;
}

bool qdMinigameObjectInterfaceImplBase::set_state(int state_index)
{
	object_ -> set_state(state_index);
	return true;
}

int qdMinigameObjectInterfaceImplBase::state_index(const char* state_name) const
{
	if(const qdGameObjectState* p = object_ -> get_state(state_name))
		return object_ -> get_state_index(p);

	return -1;
}

mgVect3f qdMinigameObjectInterfaceImplBase::R() const
{
	Vect3f r = object_ -> R();
	return mgVect3f(r.x,r.y,r.z);
}

void qdMinigameObjectInterfaceImplBase::set_R(const mgVect3f& r)
{
	Vect3f rr(r.x,r.y,r.z);
	object_ -> set_pos(rr);
}

bool qdMinigameObjectInterfaceImplBase::hit_test(const mgVect2i& pos) const
{
	return object_->hit(pos.x, pos.y);
}

mgVect2i qdMinigameObjectInterfaceImplBase::screen_R() const
{
	return mgVect2i(object_ -> screen_pos().x,object_ -> screen_pos().y);
}

bool qdMinigameObjectInterfaceImplBase::update_screen_R()
{
	object_ -> update_screen_pos();
	return true;
}

mgVect2i qdMinigameObjectInterfaceImplBase::screen_size() const
{
	Vect2s sz = object_ -> screen_size();
	return mgVect2i(sz.x,sz.y);
}

void qdMinigameObjectInterfaceImplBase::set_screen_rotation(float angle, float speed)
{
	object_->set_screen_rotation(angle, speed);
}

float qdMinigameObjectInterfaceImplBase::screen_rotation() const
{
	return object_->screen_rotation();
}

void qdMinigameObjectInterfaceImplBase::set_screen_scale(const mgVect2f& scale, const mgVect2f& speed)
{
	object_->set_screen_scale(Vect2f(scale.x, scale.y), Vect2f(speed.x, speed.y));
}

mgVect2f qdMinigameObjectInterfaceImplBase::screen_scale() const
{
	return mgVect2f(object_->screen_scale().x, object_->screen_scale().y);
}

int qdMinigameObjectInterfaceImplBase::shadow_color() const
{
	return object_->shadow_color();
}

int qdMinigameObjectInterfaceImplBase::shadow_alpha() const
{
	return object_->shadow_alpha();
}

bool qdMinigameObjectInterfaceImplBase::set_shadow(int shadow_color, int shadow_alpha)
{
	object_->set_shadow(shadow_color, shadow_alpha);
	return true;
}

bool qdMinigameObjectInterfaceImplBase::is_visible() const
{
	return object_ -> is_visible();
}

const qdEngineInterfaceImpl& qdEngineInterfaceImpl::instance()
{
	static qdEngineInterfaceImpl qdi;
	return qdi;
}

qdMinigameSceneInterface* qdEngineInterfaceImpl::current_scene_interface() const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
		return new qdMinigameSceneInterfaceImpl(dp -> get_active_scene());

	return NULL;
}

qdMinigameSceneInterface* qdEngineInterfaceImpl::scene_interface(qdGameScene* scene) const
{
	return new qdMinigameSceneInterfaceImpl(scene);
}

mgVect2i qdEngineInterfaceImpl::screen_size() const
{
	if(grDispatcher* dp = grDispatcher::instance())
		return mgVect2i(dp -> Get_SizeX(),dp -> Get_SizeY());

	return mgVect2i(0,0);
}

void qdEngineInterfaceImpl::release_scene_interface(qdMinigameSceneInterface* p) const
{
	delete p;
}

qdMinigameCounterInterface* qdEngineInterfaceImpl::counter_interface(const char* counter_name) const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
		return new qdMinigameCounterInterfaceImpl(dp -> get_counter(counter_name));

	return 0;
}

void qdEngineInterfaceImpl::release_counter_interface(qdMinigameCounterInterface* p) const
{
	delete p;
}

bool qdEngineInterfaceImpl::is_key_pressed(int vkey) const
{
	return keyboardDispatcher::instance() -> is_pressed(vkey);
}

bool qdEngineInterfaceImpl::is_mouse_event_active(qdMinigameMouseEvent event_id) const
{
	return mouseDispatcher::instance() -> is_event_active(mouseDispatcher::mouseEvent(event_id));
}

mgVect2i qdEngineInterfaceImpl::mouse_cursor_position() const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		Vect2f pos = dp -> mouse_cursor_pos();
		return mgVect2i(pos.x,pos.y);
	}

	return mgVect2i(0,0);
}

bool qdEngineInterfaceImpl::add_hall_of_fame_entry(int score) const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
		return dp->add_hall_of_fame_entry(score);

	return false;
}

bool qdEngineInterfaceImpl::set_interface_text(const char* screen_name, const char* control_name, const char* text) const
{
	if(qdInterfaceDispatcher* dp = qdInterfaceDispatcher::get_dispatcher()){
		qdInterfaceScreen* scr = screen_name ? dp->get_screen(screen_name) : dp->selected_screen();

		if(scr){
			qdInterfaceElement* el = scr->get_element(control_name);
			if(el && el->get_element_type() == qdInterfaceElement::EL_TEXT_WINDOW){
				static_cast<qdInterfaceTextWindow*>(el)->set_input_string(text);
				return true;
			}
		}
	}

	return false;
}

int qdMinigameCounterInterfaceImpl::value() const
{
	if(counter_)
		return counter_->value();

	return 0;
}

void qdMinigameCounterInterfaceImpl::set_value(int value)
{
	if(counter_)
		counter_->set_value(value);
}

void qdMinigameCounterInterfaceImpl::add_value(int value_delta)
{
	if(counter_)
		counter_->add_value(value_delta);
}

}; // namespace qdmg
