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

#ifndef QDENGINE_QDCORE_QD_GAME_SCENE_H
#define QDENGINE_QDCORE_QD_GAME_SCENE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_camera.h"
#include "qdengine/qdcore/qd_conditional_object.h"
#include "qdengine/qdcore/qd_game_dispatcher_base.h"
#include "qdengine/qdcore/qd_object_map_container.h"
#include "qdengine/qdcore/qd_object_list_container.h"
#include "qdengine/qdcore/qd_file_owner.h"
#include "qdengine/qdcore/util/fps_counter.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/graphics/gr_screen_region.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdMiniGame;

//! Сцена.
class qdGameScene : public qdGameDispatcherBase, public qdFileOwner {
public:
	qdGameScene();
	~qdGameScene();

	int named_object_type() const {
		return QD_NAMED_OBJECT_SCENE;
	}

	//! флаги
	enum {
		/// зациклить сцену по горизонтали
		CYCLE_X     = 0x01,
		/// зациклить сцену по вертикали
		CYCLE_Y     = 0x02,

		/// перезапускать триггера при заходе на сцену
		RESET_TRIGGERS_ON_LOAD = 0x04,

		/// не переключать персонажей с клавиатуры
		DISABLE_KEYBOARD_PERSONAGE_SWITCH = 0x08,

		/// не давать выйти в главное меню по ESC
		DISABLE_MAIN_MENU = 0x10
	};

	qdNamedObject *mouse_click_object() {
		return _mouse_click_object;
	}
	void set_mouse_click_object(qdNamedObject *p) {
		_mouse_click_object = p;
	}

	qdNamedObject *mouse_right_click_object() {
		return _mouse_right_click_object;
	}
	void set_mouse_right_click_object(qdNamedObject *p) {
		_mouse_right_click_object = p;
	}

	qdNamedObject *mouse_hover_object() {
		return _mouse_hover_object;
	}
	void set_mouse_hover_object(qdNamedObject *p) {
		_mouse_hover_object = p;
	}

	const Vect2f mouse_click_pos() const {
		return _mouse_click_pos;
	}

	bool need_to_redraw_inventory(const char *inventory_name) const;

	const char *minigame_name() const {
		return _minigame_name.c_str();
	}
	void set_minigame_name(const char *name) {
		_minigame_name = name;
	}
	bool has_minigame() const {
		return !_minigame_name.empty();
	}

	const qdMiniGame *minigame() const {
		return _minigame;
	}
	void start_minigame();

	bool restart_minigame_on_load() const {
		return _restart_minigame_on_load;
	}
	void toggle_restart_minigame_on_load(bool state) {
		_restart_minigame_on_load = state;
	}

	bool add_object(qdGameObject *p);
	bool rename_object(qdGameObject *p, const char *name);
	bool remove_object(const char *name);
	bool remove_object(qdGameObject *p);
	qdGameObject *get_object(const char *name);
	bool is_object_in_list(const char *name);
	bool is_object_in_list(qdGameObject *p);

	bool is_active() const;

	void set_active_personage(qdGameObjectMoving *p);
	void set_active_object(qdGameObjectAnimated *p);
	qdGameObjectMoving *get_active_personage() {
		return _selected_object;
	}
	bool change_active_personage(void);

	bool set_personage_button(qdInterfaceButton *p);

	bool add_grid_zone(qdGridZone *p);
	bool rename_grid_zone(qdGridZone *p, const char *name);
	bool remove_grid_zone(const char *name);
	bool remove_grid_zone(qdGridZone *p);
	qdGridZone *get_grid_zone(const char *name);
	bool is_grid_zone_in_list(const char *name);
	bool is_grid_zone_in_list(qdGridZone *p);

	bool is_any_personage_in_zone(const qdGridZone *p) const;

	bool add_music_track(qdMusicTrack *p);
	bool rename_music_track(qdMusicTrack *p, const char *name);
	bool remove_music_track(const char *name);
	bool remove_music_track(qdMusicTrack *p);
	qdMusicTrack *get_music_track(const char *name);
	bool is_music_track_in_list(const char *name) const;
	bool is_music_track_in_list(qdMusicTrack *p) const;

	void init_objects_grid();
	void quant(float dt);

	//! Вызывается при заходе на сцену.
	bool activate();
	//! Вызывается при выходе со сцены.
	bool deactivate();

	//! Инициализация данных, вызывается при старте и перезапуске игры.
	bool init();

	void pre_redraw();
	void redraw();
	void post_redraw();
	void debug_redraw();

	void add_redraw_region(const grScreenRegion &reg) const;

	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	int load_resources();
	void free_resources();

	qdCamera *get_camera() {
		return &_camera;
	}
	const qdCamera *get_camera() const {
		return &_camera;
	}

	const qdGameObjectList &object_list() const {
		return _objects.get_list();
	}
	const qdGridZoneList &grid_zone_list() const {
		return _grid_zones.get_list();
	}
	const qdMusicTrackList &music_track_list() const {
		return _music_tracks.get_list();
	}

	bool merge_global_objects(qdGameObject *obj = NULL);
	bool split_global_objects(qdGameObject *obj = NULL);

	int get_resources_size();

	void inc_zone_update_count() {
		_zone_update_count++;
	}
	uint32 zone_update_count() const {
		return _zone_update_count;
	}

	qdConditionalObject::trigger_start_mode trigger_start();

	static fpsCounter *fps_counter();

	int autosave_slot() const {
		return _autosave_slot;
	}
	void set_autosave_slot(int slot) {
		_autosave_slot = slot;
	}

	bool has_interface_screen() const {
		return !_interface_screen_name.empty();
	}
	const char *interface_screen_name() const {
		return _interface_screen_name.c_str();
	}
	void set_interface_screen_name(const char *name) {
		_interface_screen_name = name;
	}

	qdGameObject *get_hitted_obj(int x, int y);

	bool set_camera_mode(const qdCameraMode &mode, qdGameObjectAnimated *object);

#ifdef __QD_DEBUG_ENABLE__
	bool get_resources_info(qdResourceInfoContainer &infos) const;
#endif

private:

	int _autosave_slot;

	qdObjectMapContainer<qdGameObject> _objects;
	qdObjectListContainer<qdGridZone> _grid_zones;
	qdObjectListContainer<qdMusicTrack> _music_tracks;

	qdCamera _camera;

	//! текущий персонаж
	qdGameObjectMoving *_selected_object;

	/// миниигра, управляющая сценой
	qdMiniGame *_minigame;

	/// true если нужен перезапуск миниигры при загрузке из сэйва
	bool _restart_minigame_on_load;

	typedef Std::vector<qdGameObjectMoving *> personages_container_t;
	//! список персонажей сцены
	personages_container_t _personages;

	//! кликнутый мышью объект
	qdNamedObject *_mouse_click_object;
	//! кликнутый правой кнопкой мыши объект
	qdNamedObject *_mouse_right_click_object;
	//! объект, над кторым мышиный курсор
	qdNamedObject *_mouse_hover_object;

	//! кликнутая мышью точка на плоскости сцены
	Vect2f _mouse_click_pos;

	/// имя миниигры, управляющей сценой
	Common::String _minigame_name;

	/// используемый для сцены интерфейсный экран
	Common::String _interface_screen_name;

	uint32 _zone_update_count;

	static Std::vector<qdGameObject *> _visible_objects;

	static grScreenRegion _fps_region;
	static grScreenRegion _fps_region_last;
	static char _fps_string[255];

	bool init_visible_objects_list();
	void update_mouse_cursor();

	void personages_quant();

	//! Инициализирует персонажей, участвующих в следовании
	void follow_pers_init(int follow_cond);
	//! Пытается найти путь к точке следования для pObj. Если идти нужно, но не удастся, то false.
	bool follow_path_seek(qdGameObjectMoving *pObj, bool lock_target);

	//! Реализует движение всех "следующих", кто достаточно далеко от точки следования
	void follow_implement_update_path();
	//! Снимает состояние FOLLOW_WAIT и заставляет пытаться следовать снова
	void follow_wakening();
	/** Обработка пересечений и попытка первого перса обойти второго, разрешив
	    таким образом пересечение */
	void follow_circuit(float dt);
	//! Останавливает дошедших следующих персонажей
	void follow_end_moving();
	//! Квант следования
	void follow_quant(float dt);

	void collision_quant();

	void create_minigame_objects();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_SCENE_H
