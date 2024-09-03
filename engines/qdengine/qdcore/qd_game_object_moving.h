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

#ifndef QDENGINE_QDCORE_QD_GAME_OBJECT_MOVING_H
#define QDENGINE_QDCORE_QD_GAME_OBJECT_MOVING_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_game_object_animated.h"

namespace QDEngine {

class qdInterfaceButton;

const int QD_MOVING_OBJ_PATH_LENGTH = 200;

//! Персонаж.
class qdGameObjectMoving : public qdGameObjectAnimated {
public:
	qdGameObjectMoving();
	qdGameObjectMoving(const qdGameObjectMoving &obj);
	~qdGameObjectMoving();

	qdGameObjectMoving &operator = (const qdGameObjectMoving &obj);

	int named_object_type() const {
		return QD_NAMED_OBJECT_MOVING_OBJ;
	}

	enum movement_mode_t {
		MOVEMENT_MODE_STOP,
		MOVEMENT_MODE_TURN,
		MOVEMENT_MODE_START,
		MOVEMENT_MODE_MOVE,
		MOVEMENT_MODE_END
	};

	//! режимы управления персонажем
	enum control_type_t {
		//! можно указывать мышью точки куда идти
		CONTROL_MOUSE       = 0x01,
		//! можно рулить с клавиатуры
		CONTROL_KEYBOARD    = 0x02,
		//! можно толкать другим персонажем
		CONTROL_COLLISION   = 0x04,
		//! автоматически избегать столкновений с другими персонажами
		CONTROL_AVOID_COLLISION = 0x10,
		//! автоматически двигаться
		CONTROL_AUTO_MOVE   = 0x20,
		//! сгонять с пути других персонажей, если блокируют дорогу
		CONTROL_CLEAR_PATH  = 0x40,
		//! Персонаж стремится не отходить от активного персонажа более чем на некоторый радиус
		CONTROL_FOLLOW_ACTIVE_PERSONAGE = 0x80,
		//! Персонаж пытается двигаться в ту же сторону, что и активный
		CONTROL_REPEAT_ACTIVE_PERSONAGE_MOVEMENT = 0x100,
		//! Жесткая привязка персонажа к заданному персонажу
		CONTROL_ATTACHMENT_WITH_DIR_REL = 0x200,
		/** Жесткая привязка персонажа к заданному персонажу _без учета направления движения_
		    персонажа к которому привязываемся */
		CONTROL_ATTACHMENT_WITHOUT_DIR_REL = 0x400,
		// Подходить к точке привязки относительно активного
		CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING = 0x800,
		//! Режим реагирования на клик активному персонажу (персонаж бежит туда же)
		CONTROL_ACTIVE_CLICK_REACTING = 0x1000,
		//! Режим с анимацией поворота
		CONTROL_ANIMATED_ROTATION = 0x2000
	};

	//! флаги следования
	enum follow_condition_t {
		//! Для персонажа нужно просчитать путь следования
		FOLLOW_UPDATE_PATH = 0x01,
		//! Все ок
		FOLLOW_DONE = 0x02,
		//! Персонаж ждет, когда можно будет возобновить попытку следования
		FOLLOW_WAIT = 0x03,
		//! Персонаж ждет остановки всех следующих персонажей, чтобы продолжить следование
		FOLLOW_FULL_STOP_WAIT = 0x04,
		//! Персонаж следует
		FOLLOW_MOVING = 0x05
	};

	bool has_control_type(control_type_t type) const {
		if (_control_types & type) return true;
		return false;
	}
	void add_control_type(control_type_t type) {
		_control_types |= type;
	}
	void remove_control_type(control_type_t type) {
		_control_types &= ~type;
	}

	qdGameObjectStateWalk::movement_type_t movement_type() const;

	bool is_walkable(const Vect3f &pos) const;
	bool is_walkable(const Vect2s &pos) const;

	const Vect3f &bound(bool perspective_correction = true) const;
	Vect3f calc_bound_in_pos(Vect3f pos, bool perspective_correction = true);
	bool calc_walk_grid(Vect2s &center, Vect2s &size) const;
	//! Возвращает текущее положение сетки движения, и положение сетки через dt
	bool calc_cur_and_future_walk_grid(float dt, Vect2s &cen_cur, Vect2s &size_cur,
	                                   Vect2s &cen_next, Vect2s &size_next);
	float radius() const;

	bool adjust_z();

	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);

	float direction_angle() const {
		return _direction_angle;
	}
	float calc_direction_angle(const Vect3f &target) const;
	float animate_rotation(float dt);
	float rotation_angle() const {
		return _rotation_angle;
	}

	float rotation_angle_per_quant() const {
		return _rotation_angle_per_quant;
	}
	void set_rotation_angle_per_quant(float ang) {
		_rotation_angle_per_quant = ang;
	}

	bool set_direction(float angle);
	int get_direction(float angle) const;

	float default_direction_angle() const {
		return _default_direction_angle;
	}
	void set_default_direction(float ang) {
		_default_direction_angle = ang;
	}

	void set_state(int st);
	void set_state(qdGameObjectState *p);

	void set_last_walk_state(qdGameObjectState *p) {
		_last_walk_state = p;
	}
	qdGameObjectState *last_walk_state() {
		return _last_walk_state;
	}

	qdGameObjectState *get_default_state();
	const qdGameObjectState *get_default_state() const;

	void merge(qdGameObjectMoving *p);
	void split(qdGameObjectMoving *p);

	void set_button(qdInterfaceButton *p) {
		_button = p;
	}
	qdInterfaceButton *button() const {
		return _button;
	}

	bool move(const Vect3f &target, bool lock_target = false);
	bool move(const Vect3f &target, float angle, bool lock_target = false);

	bool move2position(const Vect3f target);

	bool skip_movement();
	bool stop_movement();

	bool is_moving() const {
		return check_flag(QD_OBJ_MOVING_FLAG);
	}

	bool can_move() const;

	bool is_in_position(const Vect3f pos) const;
	bool is_in_position(const Vect3f pos, float angle) const;

	bool is_moving2position(const Vect3f pos) const;
	bool is_moving2position(const Vect3f pos, float angle) const;

	//! Текущая точка, к которой движется персонаж.
	Vect3f local_target_position() const {
		if (is_moving())
			return _target_r;
		else
			return R();
	}

	//! Точка, к которой движется персонаж.
	Vect3f target_position() const {
		if (is_moving())
			return ((_path_length) ? _path[_path_length] : _target_r);
		else
			return R();
	}

	void set_scale(float sc) {
		_scale = sc;
	}
	float scale() const {
		return _scale;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	bool load_resources();

	//! Возвращает true, если объект в данный момент может менять состояние.
	bool can_change_state(const qdGameObjectState *state = NULL) const;

	//! Инициализация объекта, вызывается при старте и перезапуске игры.
	bool init();

	Vect3f get_future_r(float dt, bool &end_movement, bool real_moving = false);
	void quant(float dt);

	void redraw(int offs_x = 0, int offs_y = 0) const;
	void debug_redraw() const;
	void draw_contour(uint32 color) const;
	void draw_shadow(int offs_x, int offs_y, uint32 color, int alpha) const;

	bool get_debug_info(Common::String &buf) const;

	grScreenRegion screen_region() const;

	bool hit(int x, int y) const;

	bool update_screen_pos();
	Vect2s screen_size() const;

	void disable_control() {
		_disable_control = true;
	}
	void enable_control() {
		_disable_control = false;
	}
	bool is_control_disabled() const {
		return _disable_control;
	}

	bool keyboard_move();

	bool set_movement_impulse(float dir_angle);

	float collision_radius() const {
		if (_collision_radius > FLT_EPS)
			return _collision_radius;
		else
			return radius();
	}

	void set_collision_radius(float r) {
		_collision_radius = r;
	}

	float collision_delay() const {
		return _collision_delay;
	}
	void set_collision_delay(float r) {
		_collision_delay = r;
	}

	float collision_path() const {
		return _collision_path;
	}
	void set_collision_path(float path) {
		_collision_path = path;
	}

	float follow_min_radius() const {
		return _follow_min_radius;
	}
	void set_follow_min_radius(float fmr) {
		_follow_min_radius = fmr;
	}

	float follow_max_radius() const {
		return _follow_max_radius;
	}
	void set_follow_max_radius(float fmr) {
		_follow_max_radius = fmr;
	}

	int follow_condition() const {
		return _follow_condition;
	};
	void set_follow_condition(int cond) {
		_follow_condition = cond;
	};

	const Std::vector<const qdGameObjectMoving *> &const_ref_circuit_objs() const {
		return _circuit_objs;
	};
	Std::vector<const qdGameObjectMoving *> &ref_circuit_objs() {
		return _circuit_objs;
	};

	// Для CONTROL_ATTACHMENT
	const qdGameObjectMoving *attacher() const {
		return _attacher;
	}
	void set_attacher(const qdGameObjectMoving *mov_obj);
	const qdNamedObjectReference &attacher_ref() const {
		return _attacher_ref;
	}
	Vect2s attach_shift() const {
		return _attach_shift;
	}
	void set_attach_shift(Vect2s shift) {
		_attach_shift = shift;
	}

	Vect3f last_move_order() const {
		return _last_move_order;
	};
	void set_last_move_order(const Vect3f &pnt) {
		_last_move_order = pnt;
	};

	bool avoid_collision(const qdGameObjectMoving *p);
	bool move_from_personage_path();

	bool toggle_grid_zone(bool make_walkable = false);
	void toggle_selection(bool state) {
		_is_selected = state;
	}

	void set_path_attributes(int attr) const;
	void clear_path_attributes(int attr) const;

protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;

private:

	//! Дистанция, на котрой персонаж взаимодействует с другими персонажами.
	/**
	Если не установлен, высчитывается по баунду персонажа.
	*/
	float _collision_radius;
	//! Задержка от момента столкновения с другим персонажем до начала движения.
	float _collision_delay;
	//! Путь, который персонаж проходит после того, как другой персонаж его толкает.
	float _collision_path;

	//! Минимальный и максимальный радиусы следования
	float _follow_min_radius;
	float _follow_max_radius;

	//! Состояние следования
	int _follow_condition;
	//! Объекты, который текущий объект пытается обойти
	Std::vector<const qdGameObjectMoving *> _circuit_objs;

	//! Для CONTROL_HARD_ATTACHMENT
	const qdGameObjectMoving *_attacher;   // Объект, который присоединяет к себе наш объект
	qdNamedObjectReference _attacher_ref;
	Vect2s _attach_shift;            // Позиция нашего объекта - смещение от центра attacher'а

	//! Режимы управления персонажем - комбинация значений control_type_t.
	int _control_types;

	bool _disable_control;

	bool _impulse_movement_mode;
	float _impulse_timer;
	float _impulse_start_timer;
	float _impulse_direction;

	movement_mode_t _movement_mode;
	float _movement_mode_time;
	float _movement_mode_time_current;

	float _scale;
	float _direction_angle;
	float _rotation_angle;
	float _rotation_angle_per_quant;

	float _default_direction_angle;

	float _speed_delta;

	Vect3f _last_move_order; //! Точка, заданная последним приказом на движение

	Vect3f _target_r;
	int _path_length;
	int _cur_path_index;
	float _target_angle;
	Vect3f _path[QD_MOVING_OBJ_PATH_LENGTH];

	Vect2s _walk_grid_size;
	qdGameObjectState *_last_walk_state;

	bool _ignore_personages;
	bool _is_selected;

	mutable qdInterfaceButton *_button;

	Vect2s get_nearest_walkable_point(const Vect2s &target) const;
	//! Возвращает доступную точку, предшествующую последней до target пустОте
	Vect2s get_pre_last_walkable_point(const Vect2s &target) const;
	bool is_path_walkable(int x1, int y1, int x2, int y2) const;
	bool is_path_walkable(const Vect2i &src, const Vect2i &trg) const {
		return is_path_walkable(src.x, src.y, trg.x, trg.y);
	}
	bool is_path_walkable(const Vect3f &src, const Vect3f &trg) const;
	bool enough_far_target(const Vect3f &dest) const;

	void toggle_ignore_personages(bool state) {
		_ignore_personages = state;
	}

	bool find_path(const Vect3f target, bool lock_target = false);

	void optimize_path(Std::vector<Vect2i> &path) const;

	void optimize_path_four_dirs(Std::list<Vect2i> &path) const;
	// Спрямление четырех точек для пути с восемью направлениями
	bool four_pts_eight_dir_straight(Std::list<Vect2i> &path,
	                                 Std::list<Vect2i>::reverse_iterator cur) const;
	// Удаляем точки, лежащие внутри прямых отрезков пути
	bool del_coll_pts(Std::list<Vect2i> &path) const;
	void optimize_path_eight_dirs(Std::list<Vect2i> &path) const;
	void optimize_path_smooth(Std::list<Vect2i> &path) const;
	void finalize_path(const Vect3f &from, const Vect3f &to, const Std::vector<Vect2i> &path, Std::vector<Vect3f> &out_path) const;

	bool adjust_position(Vect3f &pos) const;
	bool adjust_direction_angle(float &angle);

	void change_direction_angle(float angle);

	bool is_direction_allowed(float angle) const;
	int allowed_directions_count() const;

	float calc_scale() const {
		return calc_scale(R());
	}
	float calc_scale(const Vect3f &r) const;

	bool set_walk_animation();
	bool movement_impulse();

	float speed();
	bool get_speed_parameters(float &speed, float &speed_max, float &acceleration);

	const qdGameObjectStateWalk *current_walk_state() const;

	bool adjust_position(Vect3f &pos, float dir_angle) const;

	Vect2s walk_grid_size(const Vect3f &r) const;
	Vect2s walk_grid_size(const Vect2s &r) const;
	Vect2s walk_grid_size() const {
		return walk_grid_size(R());
	}

	bool start_auto_move();

	//! Проверяет, является ли корректной последующая позиция при движении персонажа
	bool future_pos_correct(float dt);

	/// Проверка, закончилось ли движение.
	bool is_movement_finished() const;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_OBJECT_MOVING_H
