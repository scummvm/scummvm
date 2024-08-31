/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_counter.h"
#include "qdengine/qdcore/qd_minigame_interface.h"
#include "qdengine/qdcore/qd_engine_interface.h"
#include "qdengine/qdcore/qd_rnd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/input/keyboard_input.h"

#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_object_moving.h"

#include "qdengine/qdcore/qd_interface_dispatcher.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_text_window.h"


namespace QDEngine {

namespace qdmg {

class qdMinigameObjectInterfaceImplBase : public qdMinigameObjectInterface {
public:
	qdMinigameObjectInterfaceImplBase(qdGameObjectAnimated *object) : _object(object) {
		assert(_object);
	}

	/// Имя объекта.
	const char *name() const;

	//! Возвращает true, если у объекта есть состояние с именем state_name.
	bool has_state(const char *state_name) const;
	//! Возвращает имя активного в данный момент состояния.
	//! Вернёт 0, если активного состояния нету.
	const char *current_state_name() const;
	//! Возвращает true, если состояние с именем state_name включено в данный момент.
	bool is_state_active(const char *state_name) const;
	//! Возвращает true, если состояние с именем state_name в данный момент ожидает активации.
	bool is_state_waiting_activation(const char *state_name) const;
	//! Возвращает номер включенного в данный момент состояния.
	int current_state_index() const;
	//! Включает состояние с именем state_name.
	bool set_state(const char *state_name);
	//! Включает состояние номер state_index (отсчитывается от нуля).
	bool set_state(int state_index);
	//! Возвращает номер состояния с именем state_name.
	/**
	Отсчитывается от нуля, если такого состояния нет, то
	возвращает -1.
	*/
	int state_index(const char *state_name) const;

	//! Возвращает координаты объекта в мировой системе координат.
	mgVect3f R() const;
	//! Устанавливает координаты объекта в мировой системе координат.
	void set_R(const mgVect3f &r);

	/// Проверка, попадает ли точка с экранными координатами pos в объект
	bool hit_test(const mgVect2i &pos) const;

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
	void set_screen_scale(const mgVect2f &scale, const mgVect2f &speed);
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

	qdGameObjectAnimated *_object;
};

class qdMinigameObjectInterfaceImpl : public qdMinigameObjectInterfaceImplBase {
public:
	qdMinigameObjectInterfaceImpl(qdGameObjectAnimated *object) : qdMinigameObjectInterfaceImplBase(object) { }

	//! Команда персонажу идти к точке target_position.
	/**
	Если второй параметр равен false, то если target_position непроходима
	персонаж идёт к ближайшей от target_position проходимой точке.
	*/
	bool move(const mgVect3f &target_position, bool disable_target_change = false) {
		return false;
	}

	float direction_angle() const {
		return 0.f;
	}
	bool set_direction_angle(float direction) {
		return false;
	}
};

//! Интерфейс к персонажу.
class qdMinigamePersonageInterfaceImpl : public qdMinigameObjectInterfaceImplBase {
public:
	qdMinigamePersonageInterfaceImpl(qdGameObjectMoving *p);

	//! Команда персонажу идти к точке target_position.
	/**
	Если второй параметр равен false, то если target_position непроходима
	персонаж идёт к ближайшей от target_position проходимой точке.
	*/
	bool move(const mgVect3f &target_position, bool disable_target_change = false);

	float direction_angle() const;
	bool set_direction_angle(float direction);

private:

	qdGameObjectMoving *_personage_object;
};

//! Интерфейс к сцене.
class qdMinigameSceneInterfaceImpl : public qdMinigameSceneInterface {
public:
	qdMinigameSceneInterfaceImpl(qdGameScene *scene);

	/// Имя сцены.
	const char *name() const;

	//! Создаёт интерфейс к объекту с именем object_name.
	qdMinigameObjectInterface *object_interface(const char *object_name);
	//! Создаёт интерфейс к персонажу с именем personage_name.
	qdMinigameObjectInterface *personage_interface(const char *personage_name);
	//! Активация персонажа с именем personage_name.
	bool activate_personage(const char *personage_name);

	//! Преобразование из экранных координат в мировые.
	mgVect3f screen2world_coords(const mgVect2i &screen_pos, float screen_depth = 0) const;
	//! Преобразование из мировых координат в экранные.
	mgVect2i world2screen_coords(const mgVect3f &world_pos) const;
	//! Возвращает "глубину" точки с координатами pos в мировой системе координат.
	float screen_depth(const mgVect3f &pos) const;
	//! Возвращает мировые координаты точки на сетке по её экранным координатам.
	mgVect3f screen2grid_coords(const mgVect2i &screen_pos) const;

	//! Создаёт интерфейс к объекту, который взят мышью в данный момент.
	qdMinigameObjectInterface *mouse_object_interface() const;
	//! Создаёт интерфейс к объекту, по которому кликнули мышью.
	qdMinigameObjectInterface *mouse_click_object_interface() const;
	//! Создаёт интерфейс к объекту, по которому кликнули правой кнопкой мыши.
	qdMinigameObjectInterface *mouse_right_click_object_interface() const;
	//! Создаёт интерфейс к объекту, над которым находится мышиный курсор.
	qdMinigameObjectInterface *mouse_hover_object_interface() const;

	const char *minigame_parameter(const char *parameter_name) const;

	void release_object_interface(qdMinigameObjectInterface *p) const;

private:

	qdGameScene *_scene;
};

/// Интерфейс к счётчику.
class qdMinigameCounterInterfaceImpl : public qdMinigameCounterInterface {
public:
	qdMinigameCounterInterfaceImpl(qdCounter *counter) : _counter(counter) {
		assert(_counter);
	}

	/// возвращает текущее значение счётчика
	int value() const;
	/// устанавливает текущее значение счётчика
	void set_value(int value);
	/// добавляет к текущему значению счётчика value_delta
	void add_value(int value_delta);
private:

	qdCounter *_counter;
};

} // namespace qdmg

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace qdmg {

qdMinigameSceneInterfaceImpl::qdMinigameSceneInterfaceImpl(qdGameScene *scene) : _scene(scene) {
	assert(_scene);
}

const char *qdMinigameSceneInterfaceImpl::minigame_parameter(const char *parameter_name) const {
	if (const qdMiniGame * p = _scene->minigame())
		return p->config_parameter_value(parameter_name);
	return NULL;
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::mouse_object_interface() const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
		if (dp->mouse_object()->object())
			return new qdMinigameObjectInterfaceImpl(dp->mouse_object()->object());
	}

	return NULL;
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::mouse_click_object_interface() const {
	if (qdNamedObject * p = _scene->mouse_click_object()) {
		if (qdGameObjectAnimated * obj = dynamic_cast<qdGameObjectAnimated * >(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::mouse_right_click_object_interface() const {
	if (qdNamedObject * p = _scene->mouse_right_click_object()) {
		if (qdGameObjectAnimated * obj = dynamic_cast<qdGameObjectAnimated * >(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::mouse_hover_object_interface() const {
	if (qdNamedObject * p = _scene->mouse_hover_object()) {
		if (qdGameObjectAnimated * obj = dynamic_cast<qdGameObjectAnimated * >(p))
			return new qdMinigameObjectInterfaceImpl(obj);
	}
	return 0;
}

mgVect3f qdMinigameSceneInterfaceImpl::screen2world_coords(const mgVect2i &screen_pos, float screen_depth) const {
	const qdCamera *cp = _scene->get_camera();
	Vect3f pos = cp->rscr2global(cp->scr2rscr(Vect2s(screen_pos.x, screen_pos.y)), screen_depth);

	return mgVect3f(pos.x, pos.y, pos.z);
}

mgVect2i qdMinigameSceneInterfaceImpl::world2screen_coords(const mgVect3f &world_pos) const {
	const qdCamera *cp = _scene->get_camera();
	Vect3f v = cp->global2camera_coord(Vect3f(world_pos.x, world_pos.y, world_pos.z));
	Vect2i screen_pos = cp->camera_coord2scr(v);

	return mgVect2i(screen_pos.x, screen_pos.y);
}

float qdMinigameSceneInterfaceImpl::screen_depth(const mgVect3f &pos) const {
	const qdCamera *cp = _scene->get_camera();
	Vect3f v = cp->global2camera_coord(Vect3f(pos.x, pos.y, pos.z));

	return v.z;
}

mgVect3f qdMinigameSceneInterfaceImpl::screen2grid_coords(const mgVect2i &screen_pos) const {
	const qdCamera *cp = _scene->get_camera();
	Vect3f pos = cp->scr2plane(Vect2s(screen_pos.x, screen_pos.y));

	return mgVect3f(pos.x, pos.y, pos.z);
}

void qdMinigameSceneInterfaceImpl::release_object_interface(qdMinigameObjectInterface *p) const {
	delete p;
}

const char *qdMinigameSceneInterfaceImpl::name() const {
	return _scene->name();
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::object_interface(const char *object_name) {
	if (qdGameObjectAnimated * p = dynamic_cast<qdGameObjectAnimated * >(_scene->get_object(object_name)))
		return new qdMinigameObjectInterfaceImpl(p);

	return NULL;
}

qdMinigameObjectInterface *qdMinigameSceneInterfaceImpl::personage_interface(const char *personage_name) {
	if (qdGameObjectMoving * p = dynamic_cast<qdGameObjectMoving * >(_scene->get_object(personage_name)))
		return new qdMinigamePersonageInterfaceImpl(p);

	return NULL;
}

bool qdMinigameSceneInterfaceImpl::activate_personage(const char *personage_name) {
	if (qdGameObjectMoving * p = dynamic_cast<qdGameObjectMoving * >(_scene->get_object(personage_name))) {
		_scene->set_active_personage(p);
		return true;
	}

	return false;
}

qdMinigamePersonageInterfaceImpl::qdMinigamePersonageInterfaceImpl(qdGameObjectMoving *object) : qdMinigameObjectInterfaceImplBase(object), _personage_object(object) {
	assert(_personage_object);
}

bool qdMinigamePersonageInterfaceImpl::move(const mgVect3f &target_position, bool disable_target_change) {
	if (_personage_object) {
		Vect3f target(target_position.x, target_position.y, target_position.z);
		return _personage_object->move(target, disable_target_change);
	}

	return false;
}

float qdMinigamePersonageInterfaceImpl::direction_angle() const {
	if (_personage_object)
		return _personage_object->direction_angle();

	return 0.f;
}

bool qdMinigamePersonageInterfaceImpl::set_direction_angle(float direction) {
	if (_personage_object)
		return _personage_object->set_direction(direction);

	return false;
}

mgVect3f qdMinigameObjectInterfaceImplBase::bound() const {
	if (_object) {
		Vect3f b = _object->bound();
		return mgVect3f(b.x, b.y, b.z);
	}

	return mgVect3f(0, 0, 0);
}

const char *qdMinigameObjectInterfaceImplBase::name() const {
	if (_object)
		return _object->name();

	return 0;
}

bool qdMinigameObjectInterfaceImplBase::has_state(const char *state_name) const {
	if (_object && _object->get_state(state_name))
		return true;

	return false;
}

const char *qdMinigameObjectInterfaceImplBase::current_state_name() const {
	if (_object) {
		if (const qdGameObjectState * p = _object->get_cur_state())
			return p->name();
	}

	return 0;
}

bool qdMinigameObjectInterfaceImplBase::is_state_active(const char *state_name) const {
	return _object->is_state_active(state_name);
}

bool qdMinigameObjectInterfaceImplBase::is_state_waiting_activation(const char *state_name) const {
	return _object->is_state_waiting(state_name);
}

int qdMinigameObjectInterfaceImplBase::current_state_index() const {
	return _object->cur_state();
}

bool qdMinigameObjectInterfaceImplBase::set_state(const char *state_name) {
	int idx = state_index(state_name);
	if (idx != -1) {
		_object->set_state(idx);
		return true;
	}

	return false;
}

bool qdMinigameObjectInterfaceImplBase::set_state(int state_index) {
	_object->set_state(state_index);
	return true;
}

int qdMinigameObjectInterfaceImplBase::state_index(const char *state_name) const {
	if (const qdGameObjectState * p = _object->get_state(state_name))
		return _object->get_state_index(p);

	return -1;
}

mgVect3f qdMinigameObjectInterfaceImplBase::R() const {
	Vect3f r = _object->R();
	return mgVect3f(r.x, r.y, r.z);
}

void qdMinigameObjectInterfaceImplBase::set_R(const mgVect3f &r) {
	Vect3f rr(r.x, r.y, r.z);
	_object->set_pos(rr);
}

bool qdMinigameObjectInterfaceImplBase::hit_test(const mgVect2i &pos) const {
	return _object->hit(pos.x, pos.y);
}

mgVect2i qdMinigameObjectInterfaceImplBase::screen_R() const {
	return mgVect2i(_object->screen_pos().x, _object->screen_pos().y);
}

bool qdMinigameObjectInterfaceImplBase::update_screen_R() {
	_object->update_screen_pos();
	return true;
}

mgVect2i qdMinigameObjectInterfaceImplBase::screen_size() const {
	Vect2s sz = _object->screen_size();
	return mgVect2i(sz.x, sz.y);
}

void qdMinigameObjectInterfaceImplBase::set_screen_rotation(float angle, float speed) {
	_object->set_screen_rotation(angle, speed);
}

float qdMinigameObjectInterfaceImplBase::screen_rotation() const {
	return _object->screen_rotation();
}

void qdMinigameObjectInterfaceImplBase::set_screen_scale(const mgVect2f &scale, const mgVect2f &speed) {
	_object->set_screen_scale(Vect2f(scale.x, scale.y), Vect2f(speed.x, speed.y));
}

mgVect2f qdMinigameObjectInterfaceImplBase::screen_scale() const {
	return mgVect2f(_object->screen_scale().x, _object->screen_scale().y);
}

int qdMinigameObjectInterfaceImplBase::shadow_color() const {
	return _object->shadow_color();
}

int qdMinigameObjectInterfaceImplBase::shadow_alpha() const {
	return _object->shadow_alpha();
}

bool qdMinigameObjectInterfaceImplBase::set_shadow(int shadow_color, int shadow_alpha) {
	_object->set_shadow(shadow_color, shadow_alpha);
	return true;
}

bool qdMinigameObjectInterfaceImplBase::is_visible() const {
	return _object->is_visible();
}

const qdEngineInterfaceImpl &qdEngineInterfaceImpl::instance() {
	static qdEngineInterfaceImpl qdi;
	return qdi;
}

qdMinigameSceneInterface *qdEngineInterfaceImpl::current_scene_interface() const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		return new qdMinigameSceneInterfaceImpl(dp->get_active_scene());

	return NULL;
}

qdMinigameSceneInterface *qdEngineInterfaceImpl::scene_interface(qdGameScene *scene) const {
	return new qdMinigameSceneInterfaceImpl(scene);
}

mgVect2i qdEngineInterfaceImpl::screen_size() const {
	if (grDispatcher * dp = grDispatcher::instance())
		return mgVect2i(dp->get_SizeX(), dp->get_SizeY());

	return mgVect2i(0, 0);
}

void qdEngineInterfaceImpl::release_scene_interface(qdMinigameSceneInterface *p) const {
	delete p;
}

qdMinigameCounterInterface *qdEngineInterfaceImpl::counter_interface(const char *counter_name) const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		return new qdMinigameCounterInterfaceImpl(dp->get_counter(counter_name));

	return 0;
}

void qdEngineInterfaceImpl::release_counter_interface(qdMinigameCounterInterface *p) const {
	delete p;
}

bool qdEngineInterfaceImpl::is_key_pressed(int vkey) const {
	return keyboardDispatcher::instance()->is_pressed(vkey);
}

bool qdEngineInterfaceImpl::is_mouse_event_active(qdMinigameMouseEvent event_id) const {
	return mouseDispatcher::instance()->is_event_active(mouseDispatcher::mouseEvent(event_id));
}

mgVect2i qdEngineInterfaceImpl::mouse_cursor_position() const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
		Vect2f pos = dp->mouse_cursor_pos();
		return mgVect2i(pos.x, pos.y);
	}

	return mgVect2i(0, 0);
}

bool qdEngineInterfaceImpl::add_hall_of_fame_entry(int score) const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		return dp->add_hall_of_fame_entry(score);

	return false;
}

bool qdEngineInterfaceImpl::set_interface_text(const char *screen_name, const char *control_name, const char *text) const {
	if (qdInterfaceDispatcher * dp = qdInterfaceDispatcher::get_dispatcher()) {
		qdInterfaceScreen *scr = screen_name ? dp->get_screen(screen_name) : dp->selected_screen();

		if (scr) {
			qdInterfaceElement *el = scr->get_element(control_name);
			if (el && el->get_element_type() == qdInterfaceElement::EL_TEXT_WINDOW) {
				static_cast<qdInterfaceTextWindow *>(el)->set_input_string(text);
				return true;
			}
		}
	}

	return false;
}

int qdMinigameCounterInterfaceImpl::value() const {
	if (_counter)
		return _counter->value();

	return 0;
}

void qdMinigameCounterInterfaceImpl::set_value(int value) {
	if (_counter)
		_counter->set_value(value);
}

void qdMinigameCounterInterfaceImpl::add_value(int value_delta) {
	if (_counter)
		_counter->add_value(value_delta);
}

} // namespace qdmg
} // namespace QDEngine
