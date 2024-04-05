#ifndef __QD_GAME_OBJECT_MOVING_H__
#define __QD_GAME_OBJECT_MOVING_H__

#include "xml_fwd.h"

#include "qd_game_object_animated.h"

class qdInterfaceButton;

const int QD_MOVING_OBJ_PATH_LENGTH	= 200;

//! Персонаж.
class qdGameObjectMoving : public qdGameObjectAnimated
{
public:
	qdGameObjectMoving();
	qdGameObjectMoving(const qdGameObjectMoving& obj);
#ifdef _QUEST_EDITOR
	qdGameObjectMoving(const qdGameObjectAnimated& obj);
#endif // _QUEST_EDITOR
	~qdGameObjectMoving();

	qdGameObjectMoving& operator = (const qdGameObjectMoving& obj);

	int named_object_type() const { return QD_NAMED_OBJECT_MOVING_OBJ; }

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
		CONTROL_MOUSE		= 0x01,
		//! можно рулить с клавиатуры
		CONTROL_KEYBOARD	= 0x02,
		//! можно толкать другим персонажем
		CONTROL_COLLISION	= 0x04,
		//! автоматически избегать столкновений с другими персонажами
		CONTROL_AVOID_COLLISION	= 0x10,
		//! автоматически двигаться
		CONTROL_AUTO_MOVE	= 0x20,
		//! сгонять с пути других персонажей, если блокируют дорогу
		CONTROL_CLEAR_PATH	= 0x40,
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
	enum follow_condition_t 
	{
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

	bool has_control_type(control_type_t type) const { if(control_types_ & type) return true; return false; }
	void add_control_type(control_type_t type){ control_types_ |= type; }
	void remove_control_type(control_type_t type){ control_types_ &= ~type; }

	qdGameObjectStateWalk::movement_type_t movement_type() const;

	bool is_walkable(const Vect3f& pos) const;
	bool is_walkable(const Vect2s& pos) const;

	const Vect3f& bound(bool perspective_correction = true) const;
	Vect3f calc_bound_in_pos(Vect3f pos, bool perspective_correction = true);
	bool calc_walk_grid(Vect2s& center, Vect2s& size) const;
	//! Возвращает текущее положение сетки движения, и положение сетки через dt
	bool calc_cur_and_future_walk_grid(float dt, Vect2s& cen_cur, Vect2s& size_cur, 
		                                       Vect2s& cen_next, Vect2s& size_next);
	float radius() const;

	bool adjust_z();

	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);

	float direction_angle() const { return direction_angle_; }
	float calc_direction_angle(const Vect3f& target) const;
	float animate_rotation(float dt);
	float rotation_angle() const { return rotation_angle_; }

	float rotation_angle_per_quant() const { return rotation_angle_per_quant_; }
	void set_rotation_angle_per_quant(float ang) { rotation_angle_per_quant_ = ang; }

	bool set_direction(float angle);
	int get_direction(float angle) const;

	float default_direction_angle() const { return default_direction_angle_; }
	void set_default_direction(float ang){ default_direction_angle_ = ang; }

	void set_state(int st);
	void set_state(qdGameObjectState* p);

#ifndef _QUEST_EDITOR
	void set_last_walk_state(qdGameObjectState* p){ last_walk_state_ = p; }
	qdGameObjectState* last_walk_state(){ return last_walk_state_; }
#endif // _QUEST_EDITOR

	qdGameObjectState* get_default_state();
	const qdGameObjectState* get_default_state() const;

	void merge(qdGameObjectMoving* p);
	void split(qdGameObjectMoving* p);

	void set_button(qdInterfaceButton* p){ button_ = p; }
	qdInterfaceButton* button() const { return button_; }

	bool move(const Vect3f& target,bool lock_target = false);
	bool move(const Vect3f& target,float angle,bool lock_target = false);

	bool move2position(const Vect3f target);

	bool skip_movement();
	bool stop_movement();

	bool is_moving() const { return check_flag(QD_OBJ_MOVING_FLAG); }

	bool can_move() const;

	bool is_in_position(const Vect3f pos) const;
	bool is_in_position(const Vect3f pos,float angle) const;

	bool is_moving2position(const Vect3f pos) const;
	bool is_moving2position(const Vect3f pos,float angle) const;
	
	//! Текущая точка, к которой движется персонаж.
	Vect3f local_target_position() const { 
		if(is_moving())
			return target_r_;
		else
			return R();
	}

	//! Точка, к которой движется персонаж.
	Vect3f target_position() const { 
		if(is_moving())
			return ((path_length_) ? path_[path_length_] : target_r_);
		else
			return R();
	}

	void set_scale(float sc){ scale_ = sc; }
	float scale() const { return scale_; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool load_resources();
	
	//! Возвращает true, если объект в данный момент может менять состояние.
	bool can_change_state(const qdGameObjectState* state = NULL) const;

	//! Инициализация объекта, вызывается при старте и перезапуске игры.
	bool init();

	Vect3f get_future_r(float dt, bool& end_movement, bool real_moving = false);
	void quant(float dt);

	void redraw(int offs_x = 0,int offs_y = 0) const;
	void debug_redraw() const;
	void draw_contour(unsigned color) const;
	void draw_shadow(int offs_x,int offs_y,unsigned color,int alpha) const;

	bool get_debug_info(XBuffer& buf) const;

	grScreenRegion screen_region() const;

	bool hit(int x,int y) const;

	bool update_screen_pos();
	Vect2s screen_size() const;

	void disable_control(){ disable_control_ = true; }
	void enable_control(){ disable_control_ = false; }
	bool is_control_disabled() const { return disable_control_; }

	bool keyboard_move();

	bool set_movement_impulse(float dir_angle);

#ifdef _QUEST_EDITOR
	float collision_radius() const { return collision_radius_; }
	void set_movement_mode(movement_mode_t mode){ movement_mode_ = mode; }
#else
	float collision_radius() const {
		if(collision_radius_ > FLT_EPS)
			return collision_radius_;
		else
			return radius();
	}
#endif
	void set_collision_radius(float r){ collision_radius_ = r; }

	float collision_delay() const { return collision_delay_; }
	void set_collision_delay(float r){ collision_delay_ = r; }

	float collision_path() const { return collision_path_; }
	void set_collision_path(float path){ collision_path_ = path; }

	float follow_min_radius() const { return follow_min_radius_; }
	void set_follow_min_radius(float fmr) { follow_min_radius_ = fmr; }
	
	float follow_max_radius() const { return follow_max_radius_; }
	void set_follow_max_radius(float fmr) { follow_max_radius_ = fmr; }

	int follow_condition() const { return follow_condition_; };
	void set_follow_condition(int cond) { follow_condition_ = cond; };

	const std::vector<const qdGameObjectMoving*>& const_ref_circuit_objs() const { return circuit_objs_; };
	std::vector<const qdGameObjectMoving*>& ref_circuit_objs() { return circuit_objs_; };

	// Для CONTROL_ATTACHMENT
	const qdGameObjectMoving* attacher() const { return attacher_; }
	void set_attacher(const qdGameObjectMoving* mov_obj);
	const qdNamedObjectReference& attacher_ref() const { return attacher_ref_; }
	Vect2s attach_shift() const { return attach_shift_; }
	void set_attach_shift(Vect2s shift) { attach_shift_ = shift; }

	Vect3f last_move_order() const { return last_move_order_; };
	void set_last_move_order(const Vect3f& pnt) { last_move_order_ = pnt; };

	bool avoid_collision(const qdGameObjectMoving* p);
	bool move_from_personage_path();

	bool toggle_grid_zone(bool make_walkable = false);
	void toggle_selection(bool state){ is_selected_ = state; }

	void set_path_attributes(int attr) const;
	void clear_path_attributes(int attr) const;

protected:

	bool load_script_body(const xml::tag* p);
	bool save_script_body(XStream& fh,int indent = 0) const;

private:

	//! Дистанция, на котрой персонаж взаимодействует с другими персонажами.
	/**
	Если не установлен, высчитывается по баунду персонажа.
	*/
	float collision_radius_;
	//! Задержка от момента столкновения с другим персонажем до начала движения.
	float collision_delay_;
	//! Путь, который персонаж проходит после того, как другой персонаж его толкает.
	float collision_path_;

	//! Минимальный и максимальный радиусы следования
	float follow_min_radius_;
	float follow_max_radius_;

	//! Состояние следования
	int follow_condition_;
	//! Объекты, который текущий объект пытается обойти
	std::vector<const qdGameObjectMoving*> circuit_objs_;

	//! Для CONTROL_HARD_ATTACHMENT
	const qdGameObjectMoving* attacher_;   // Объект, который присоединяет к себе наш объект
	qdNamedObjectReference attacher_ref_;
	Vect2s attach_shift_;            // Позиция нашего объекта - смещение от центра attacher'а 

	//! Режимы управления персонажем - комбинация значений control_type_t.
	int control_types_;

	bool disable_control_;

	bool impulse_movement_mode_;
	float impulse_timer_;
	float impulse_start_timer_;
	float impulse_direction_;

	movement_mode_t movement_mode_;
	float movement_mode_time_;
	float movement_mode_time_current_;

	float scale_;
	float direction_angle_;
	float rotation_angle_;
	float rotation_angle_per_quant_;

	float default_direction_angle_;

	float speed_delta_;

	Vect3f last_move_order_; //! Точка, заданная последним приказом на движение

	Vect3f target_r_;
	int path_length_;
	int cur_path_index_;
	float target_angle_;
	Vect3f path_[QD_MOVING_OBJ_PATH_LENGTH];

#ifndef _QUEST_EDITOR
	Vect2s walk_grid_size_;
	qdGameObjectState* last_walk_state_;
#endif

	bool ignore_personages_;
	bool is_selected_;

	mutable qdInterfaceButton* button_;

	Vect2s get_nearest_walkable_point(const Vect2s& target) const;
	//! Возвращает доступную точку, предшествующую последней до target пустОте
	Vect2s get_pre_last_walkable_point(const Vect2s& target) const;
	bool is_path_walkable(int x1,int y1,int x2,int y2) const;
	bool is_path_walkable(const Vect2i& src,const Vect2i& trg) const { return is_path_walkable(src.x,src.y,trg.x,trg.y); }
	bool is_path_walkable(const Vect3f& src,const Vect3f& trg) const;
	bool enough_far_target(const Vect3f& dest) const;

	void toggle_ignore_personages(bool state){ ignore_personages_ = state; }

	bool find_path(const Vect3f target,bool lock_target = false);

	void optimize_path(std::vector<Vect2i>& path) const;

	void optimize_path_four_dirs(std::list<Vect2i>& path) const;
	// Спрямление четырех точек для пути с восемью направлениями
	bool four_pts_eight_dir_straight(std::list<Vect2i>& path, 
		                             std::list<Vect2i>::reverse_iterator cur) const;
	// Удаляем точки, лежащие внутри прямых отрезков пути
	bool del_coll_pts(std::list<Vect2i>& path) const;
	void optimize_path_eight_dirs(std::list<Vect2i>& path) const;
	void optimize_path_smooth(std::list<Vect2i>& path) const;
	void finalize_path(const Vect3f& from, const Vect3f& to, const std::vector<Vect2i>& path, std::vector<Vect3f>& out_path) const;

	bool adjust_position(Vect3f& pos) const;
	bool adjust_direction_angle(float& angle);

	void change_direction_angle(float angle);

	bool is_direction_allowed(float angle) const;
	int allowed_directions_count() const;

	float calc_scale() const { return calc_scale(R()); }
	float calc_scale(const Vect3f& r) const;

	bool set_walk_animation();
	bool movement_impulse();

	float speed();
	bool get_speed_parameters(float& speed,float& speed_max,float& acceleration);

	const qdGameObjectStateWalk* current_walk_state() const;

	bool adjust_position(Vect3f& pos,float dir_angle) const;

	Vect2s walk_grid_size(const Vect3f& r) const;
	Vect2s walk_grid_size(const Vect2s& r) const;
	Vect2s walk_grid_size() const { return walk_grid_size(R()); }

	bool start_auto_move();

	//! Проверяет, является ли корректной последующая позиция при движении персонажа
	bool future_pos_correct(float dt);

	/// Проверка, закончилось ли движение.
	bool is_movement_finished() const;
};

#endif /* __QD_GAME_OBJECT_MOVING_H__ */
