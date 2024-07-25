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

#ifndef QDENGINE_QDCORE_QD_GAME_OBJECT_ANIMATED_H
#define QDENGINE_QDCORE_QD_GAME_OBJECT_ANIMATED_H

#include <vector>
#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_coords_animation.h"
#include "qdengine/qdcore/qd_game_object.h"
#include "qdengine/qdcore/qd_game_object_state.h"

namespace QDEngine {

//! Dynamic object
/**
Paticularly dynamic object, actors and the mouse.
*/
class qdGameObjectAnimated : public qdGameObject {
public:
	enum StateStatus {
		STATE_INACTIVE,
		STATE_QUEUED,
		STATE_ACTIVE,
		STATE_DONE
	};

	qdGameObjectAnimated();
	qdGameObjectAnimated(const qdGameObjectAnimated &obj);
	~qdGameObjectAnimated();

	qdGameObjectAnimated &operator = (const qdGameObjectAnimated &obj);

	int named_object_type() const {
		return QD_NAMED_OBJECT_ANIMATED_OBJ;
	}

	//! Возвращает баунд объекта.
	virtual const Vect3f &bound(bool perspective_correction = true) const;

#ifdef _QUEST_EDITOR
	//! Всегда возвращает баунд объекта(независимо от выбранного состояния)
	virtual const Vect3f &obj_bound() const;
#endif // _QUEST_EDITOR
	//! Устанавливает баунд объекта по текущему состоянию.
	bool auto_bound();
	//! Возвращает радиус объекта.
	virtual float radius() const;
	//! Устанавливает баунд объекта.
	void set_bound(const Vect3f &b);
	//! Возвращает true, если у объекта выставлен баунд.
	bool has_bound() const {
		if (check_flag(QD_OBJ_HAS_BOUND_FLAG)) return true;
		if (cur_state_ != -1 && states[cur_state_]->has_bound()) return true;
		return false;
	}
	//! Отрисовка баунда (для отладки).
	void draw_bound() const;
	void draw_bound(Vect3f r, Vect3f const &bound, int const color) const;
	//! Пересекается ли баунд, расположенный в точке с заданным баундом с центром в cen
	bool inters_with_bound(Vect3f bnd, Vect3f cen, bool perspective_correction = true) const;

	//! Возвращает номер текущего состояния объекта.
	int cur_state() const {
		return cur_state_;
	}
	//! Устанавливает номер текущего состояния объекта.
	void set_cur_state(int st) {
		cur_state_ = st;
	}
	//! Возвращает количество состояний объекта.
	int max_state() const {
		return states.size();
	}
	//! Возвращает номер состояния или -1 если не может такое состояние найти.
	int get_state_index(const qdGameObjectState *p) const;

	//! Установка владельца состояний.
	void set_states_owner();

	//! Возвращает true, если состояние с именем state_name активно.
	bool is_state_active(const char *state_name) const;
	//! Возвращает true, если состояние с именем state_name было активировано.
	bool was_state_active(const char *state_name) const;

	//! Возвращает true, если состояние state активно.
	bool is_state_active(const qdGameObjectState *p) const {
		if (cur_state_ != -1 && states[cur_state_] == p)
			return true;

		return false;
	}

	//! Возвращает true, если состояние с именем state_name было активно перед активным в данный момент состоянием.
	bool was_state_previous(const char *state_name) const;
	//! Возвращает true, если состояние p было активно перед активным в данный момент состоянием.
	bool was_state_previous(const qdGameObjectState *p) const {
		return (last_state_ == p);
	}

	//! Возвращает true, если состояние state было активировано.
	bool was_state_active(const qdGameObjectState *p) const {
		return p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_WAS_ACTIVATED);
	}

	bool is_state_waiting(const qdGameObjectState *p) const {
		if (queued_state_ == p) return true;
		return false;
	}

	bool is_state_waiting(const char *state_name) const;

	//! Возвращает состояния объекта.
	const qdGameObjectStateVector &state_vector() const {
		return states;
	}

	//! Возвращает true, если объект в данный момент может менять состояние.
	virtual bool can_change_state(const qdGameObjectState *state = NULL) const;

	qdGameObjectState *get_state(const char *state_name);
	qdGameObjectState *get_state(int state_index);
	qdGameObjectState *get_cur_state() {
		return get_state(cur_state());
	}

	const qdGameObjectState *get_state(const char *state_name) const;
	const qdGameObjectState *get_state(int state_index) const;
	const qdGameObjectState *get_cur_state() const {
		return get_state(cur_state());
	}

	void set_queued_state(qdGameObjectState *st) {
		queued_state_ = st;
	}

	qdGameObjectState *queued_state() {
		return queued_state_;
	}
	const qdGameObjectState *queued_state() const {
		return queued_state_;
	}

	void merge_states(qdGameObjectAnimated *p);
	void split_states(qdGameObjectAnimated *p);

	StateStatus state_status(const qdGameObjectState *p) const;

	bool add_state(qdGameObjectState *p);
	bool insert_state(int iBefore, qdGameObjectState *p);
	qdGameObjectState *remove_state(int state_num);
	bool remove_state(qdGameObjectState *p);

	virtual void set_state(int st);
	virtual void set_state(qdGameObjectState *p);
	void restore_state();

	bool has_camera_mode() const;
	const qdCameraMode &camera_mode() const;

	//! Возвращает количество имеющихся у объекта направлений.
	int num_directions() const;

	const Vect3f &default_R() const {
		return default_r_;
	}
	void set_default_pos(const Vect3f &r) {
		default_r_ = r;
	}

	void set_default_state();
	virtual qdGameObjectState *get_default_state();
	virtual const qdGameObjectState *get_default_state() const;

	qdGameObjectState *get_inventory_state();
	qdGameObjectState *get_mouse_state();
	qdGameObjectState *get_mouse_hover_state();

	bool update_screen_pos();

	// Animation
	qdAnimation *get_animation() {
		return &animation_;
	}
	const qdAnimation *get_animation() const {
		return &animation_;
	}

	void set_animation(qdAnimation *p, const qdAnimationInfo *inf = NULL);
	void set_animation_info(qdAnimationInfo *inf);
	Vect2s screen_size() const {
		return Vect2s(animation_.size_x(), animation_.size_y());
	}

	void set_screen_rotation(float target_angle, float speed);
	float screen_rotation() const;

	void set_screen_scale(const Vect2f &scale, const Vect2f &speed);
	const Vect2f &screen_scale() const;

	bool has_screen_transform() const {
		return current_transform_();
	}

	// Inventory
	int inventory_type() const {
		return inventory_type_;
	}
	void set_inventory_type(int tp) {
		inventory_type_ = tp;
	}

	// Logic
	bool hit(int x, int y) const;
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	void quant(float dt);
	//! Обработка окончания текущего состояния.
	bool handle_state_end();

	// Redraw
	void redraw(int offs_x = 0, int offs_y = 0) const;
	bool need_redraw() const;

	void post_redraw();
	void draw_shadow(int offs_x, int offs_y, unsigned color, int alpha) const;

	bool get_debug_info(Common::String &buf) const;

	void debug_redraw() const;
	void draw_contour(unsigned color) const;
	void draw_grid_zone(const Vect2s sz) const;

	int mouse_cursor_ID() const;

	bool load_script(const xml::tag *p);
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::SeekableWriteStream &fh) const;

	bool load_resources();
	void free_resources();

	//! Инициализация объекта, вызывается при старте и перезапуске игры.
	bool init();

	bool is_visible() const;

	const Vect2s &grid_size() const {
		return grid_size_;
	}

	bool init_grid_zone();
	virtual bool toggle_grid_zone(bool make_walkable = false);
	bool save_grid_zone();
	bool restore_grid_zone();
	bool set_grid_zone_attributes(int attr) const;
	bool check_grid_zone_attributes(int attr) const;
	bool drop_grid_zone_attributes(int attr) const;
	bool set_grid_zone_attributes(const Vect2f &r, int attr) const;
	bool check_grid_zone_attributes(const Vect2f &r, int attr) const;
	bool drop_grid_zone_attributes(const Vect2f &r, int attr) const;

	const char *inventory_name() const {
		return inventory_name_.c_str();
	}
	void set_inventory_name(const char *name) {
		if (name)
			inventory_name_ = name;
		else
			inventory_name_.clear();
	}
	bool has_inventory_name() const {
		return !inventory_name_.empty();
	}

#ifdef _QUEST_EDITOR
	//! Удаляет пустые края анимации.
	bool remove_animation_edges(Vect2i &full_offset, Vect2i &anim_offset);
#endif //_QUEST_EDITOR

	const grScreenRegion &last_screen_region() const {
		return last_screen_region_;
	}
	virtual grScreenRegion screen_region() const;

	int inventory_cell_index() const {
		return inventory_cell_index_;
	}
	void set_inventory_cell_index(int idx) {
		inventory_cell_index_ = idx;
	}

	int last_chg_time() const {
		return last_chg_time_;
	}
	void set_last_chg_time(int time) {
		last_chg_time_ = time;
	}
	int idle_time() const;

#ifdef _QUEST_EDITOR
	static void toggle_fast_state_merge(bool state) {
		fast_state_merge_ = state;
	}
#endif

	int shadow_color() const;
	int shadow_alpha() const;
	void set_shadow(unsigned color, int alpha) {
		shadow_color_ = color;
		shadow_alpha_ = alpha;
	}
	void clear_shadow() {
		set_shadow(0, QD_NO_SHADOW_ALPHA);
	}

protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(Common::SeekableWriteStream &fh, int indent = 0) const;

	void set_last_state(qdGameObjectState *p) {
		if (!p || !p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE | qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE))
			last_state_ = p;
	}

	void set_last_inventory_state(qdGameObjectState *p) {
		if (!p || (p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY) && !p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_STATE | qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE)))
			last_inventory_state_ = p;
	}

private:

	int cur_state_;
	qdGameObjectStateVector states;

	qdGameObjectState *queued_state_;
	qdGameObjectState *last_inventory_state_;

	int inventory_type_;

	qdAnimation animation_;

	Vect3f bound_;
	float radius_;

	Vect3f default_r_;

	Vect3f grid_r_;
	Vect2s grid_size_;

	std::string inventory_name_;

	qdScreenTransform current_transform_;
	qdScreenTransform target_transform_;
	qdScreenTransform transform_speed_;

	qdScreenTransform last_transform_;

	qdGameObjectState *last_state_;

	//! Индекс ячейки инвентори, в которой лежал объект.
	int inventory_cell_index_;

	const qdAnimationFrame *last_frame_;
	grScreenRegion last_screen_region_;
	float last_screen_depth_;

	unsigned lastShadowColor_;
	int lastShadowAlpha_;

	//! Последнее время изменения объекта
	int last_chg_time_;

	//! Цвет затенения.
	unsigned shadow_color_;
	//! Прозрачность затенения, значения - [0, 255], если равно QD_NO_SHADOW_ALPHA, то персонаж не затеняется.
	int shadow_alpha_;

#ifdef _QUEST_EDITOR
	/// если true, то глобальные состояния добавляются по-быстрому
	static bool fast_state_merge_;
#endif

	void clear_states();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_OBJECT_ANIMATED_H

