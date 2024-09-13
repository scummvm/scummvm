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


#ifndef QDENGINE_QDCORE_QD_GAME_DISPATCHER_H
#define QDENGINE_QDCORE_QD_GAME_DISPATCHER_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/qdcore/qd_camera_mode.h"
#include "qdengine/qdcore/qd_object_list_container.h"
#include "qdengine/qdcore/qd_game_dispatcher_base.h"
#include "qdengine/qdcore/qd_resource_dispatcher.h"
#include "qdengine/qdcore/qd_screen_text_dispatcher.h"
#include "qdengine/qdcore/qd_interface_dispatcher.h"
#include "qdengine/qdcore/qd_inventory.h"
#include "qdengine/qdcore/qd_font_info.h"
#include "qdengine/qdcore/qd_file_owner.h"
#include "qdengine/qdcore/util/WinVideo.h"

namespace QDEngine {

class grFont;
class qdGameDispatcher : public qdGameDispatcherBase, public qdResourceDispatcher<qdNamedObject>, public qdFileOwner {
public:
	//! Идентификаторы наборов текстов.
	enum {
		//! Фразы диалогов, отображаемые в данный момент на экране
		TEXT_SET_DIALOGS = 0
	};

	//! Флаги
	enum {
		EXIT_FLAG           = 0x01,
		RESTART_FLAG            = 0x02,
		OBJECT_CLICK_FLAG       = 0x04,
		DIALOG_CLICK_FLAG       = 0x08,
		FULLSCREEN_REDRAW_FLAG      = 0x10,
		SKIP_REDRAW_FLAG        = 0x20,
		MAIN_MENU_FLAG          = 0x40,
		INTRO_MODE_FLAG         = 0x80,
		CLICK_FAILED_FLAG       = 0x100,
		OBJECT_CLICK_FAILED_FLAG    = 0x200,
		CLICK_WAS_FAILED_FLAG       = 0x400,
		OBJECT_CLICK_WAS_FAILED_FLAG    = 0x800,
		LOAD_GAME_FLAG      = 0x1000,
		SAVE_GAME_FLAG      = 0x2000,
		FADE_IN_FLAG        = 0x4000,
		FADE_OUT_FLAG       = 0x8000,
		NEXT_FRAME_FLAG     = 0x10000
	};

	qdGameDispatcher();
	~qdGameDispatcher();

	int named_object_type() const {
		return QD_NAMED_OBJECT_DISPATCHER;
	}

	qdGameObjectMouse *mouse_object() {
		return _mouse_obj;
	}
	qdGameObjectMouse const *mouse_object() const {
		return _mouse_obj;
	}
	bool drop_mouse_object();

	int CD_count() const;

	int resource_compression() const {
		return _resource_compression;
	}
	void set_resource_compression(int compression) {
		_resource_compression = compression;
	}

	void request_file_package(const qdFileOwner &file_owner) const;
	Common::Path find_file(const Common::Path file_name, const qdFileOwner &file_owner) const;
	void startup_check() const;

	qdLoadingProgressFnc set_scene_loading_progress_callback(qdLoadingProgressFnc p, void *dp = 0) {
		qdLoadingProgressFnc old_fnc = _scene_loading_progress_fnc;
		_scene_loading_progress_fnc = p;
		_scene_loading_progress_data = dp;

		return old_fnc;
	}

	qdLoadingProgressFnc get_scene_loading_progress_callback() {
		return _scene_loading_progress_fnc;
	}

	void *get_scene_loading_progress_data() {
		return _scene_loading_progress_data;
	}

	void set_time(int tm) {
		_timer = tm;
	}
	int get_time() const {
		return _timer;
	}

	bool start_intro_videos();

	void quant();
	void quant(float dt);
	void pre_redraw();
	void redraw();
	void post_redraw();

	void update_time();

	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	bool keyboard_handler(Common::KeyCode vkey, bool event);

	bool add_game_end(qdGameEnd *p);
	bool rename_game_end(qdGameEnd *p, const char *name);
	bool remove_game_end(qdGameEnd *p);
	qdGameEnd *get_game_end(const char *name);
	bool is_game_end_in_list(const char *name);
	bool is_game_end_in_list(qdGameEnd *p);

	bool add_counter(qdCounter *p);
	bool rename_counter(qdCounter *p, const char *name);
	bool remove_counter(qdCounter *p);
	qdCounter *get_counter(const char *name);
	bool is_counter_in_list(const char *name);
	bool is_counter_in_list(qdCounter *p);

	bool add_minigame(qdMiniGame *p);
	bool rename_minigame(qdMiniGame *p, const char *name);
	bool remove_minigame(const char *name);
	bool remove_minigame(qdMiniGame *p);
	qdMiniGame *get_minigame(const char *name);
	bool is_minigame_in_list(const char *name);
	bool is_minigame_in_list(qdMiniGame *p);

	void set_mouse_click_state(const qdGameObjectState *p) {
		_mouse_click_state = p;
	}

	bool add_inventory(qdInventory *p) {
		if (_inventories.add_object(p)) {
			p->set_owner(this);
			return 1;
		}
		return 0;
	}
	bool remove_inventory(const char *name) {
		return _inventories.remove_object(name);
	}
	bool remove_inventory(qdInventory *p) {
		return _inventories.remove_object(p);
	}
	qdInventory *get_inventory(const char *name) {
		return _inventories.get_object(name);
	}
	bool is_inventory_in_list(const char *name) {
		return _inventories.is_in_list(name);
	}
	bool is_inventory_in_list(qdInventory *p) {
		return _inventories.is_in_list(p);
	}
	bool rename_inventory(qdInventory *p, const char *name);

	const qdInventoryCellType *add_inventory_cell_type(const qdInventoryCellType &tp) {
		//проверяем уникальность идентификатора, вставляемого типа
		assert(find_inventory_cell_type(tp.type()) == _inventory_cell_types.end());
		_inventory_cell_types.push_back(tp);

		return &_inventory_cell_types.back();
	}
	bool remove_inventory_cell_type(int type) {
		typedef qdInventoryCellTypeVector::iterator i_t;
		i_t res = find_inventory_cell_type(type);
		if (res != _inventory_cell_types.end()) {
			_inventory_cell_types.erase(res);
			return true;
		}
		return false;
	}
	const qdInventoryCellType *get_inventory_cell_type(int type) const {
		typedef qdInventoryCellTypeVector::const_iterator ci_t;
		ci_t res = find_inventory_cell_type(type);
		if (res != _inventory_cell_types.end())
			return &*res;
		return NULL;
	}
	int get_unique_inventory_cell_type() const {
		if (_inventory_cell_types.empty()) return 0;
		return _inventory_cell_types.back().type() + 1;
	}
	bool set_inventory_cell_type(const qdInventoryCellType &tp) {
		typedef qdInventoryCellTypeVector::iterator i_t;
		i_t res = find_inventory_cell_type(tp.type());
		if (res != _inventory_cell_types.end()) {
			*res = tp;
			return true;
		}
		return false;
	}
	const qdInventoryCellTypeVector &inventory_cell_types() const {
		return _inventory_cell_types;
	}

	qdInventoryCellTypeVector &not_const_inventory_cell_types() {
		return _inventory_cell_types;
	}

	//! Операции со шрифтами
	/**
	Для создания типов всех добавляемых шрифтов нужно
	использовать get_unique_font_info_type().
	Самостоятельное задание типов шрифтов крайне не рекомендуется -
	возможны проблемы с уникальностью типов шрифтов и извлечением шрифта по типу.
	*/
	bool add_font_info(qdFontInfo *fi);
	bool remove_font_info(qdFontInfo *fi);
	bool rename_font_info(qdFontInfo *fi, char const *name);

	const qdFontInfo *get_font_info(int type) const;
	int get_unique_font_info_type() const;
	bool set_font_info(const qdFontInfo &fi);
	const qdFontInfoList &fonts_list() const {
		return _fonts.get_list();
	}

	int default_font() const {
		return _default_font;
	}
	void set_default_font(int font) {
		_default_font = font;
	}

	bool toggle_inventory(bool state);
	bool toggle_inventory() {
		if (!_cur_inventory) return toggle_inventory(true);
		else return toggle_inventory(false);
	}
	bool put_to_inventory(qdGameObjectAnimated *p);
	bool remove_from_inventory(qdGameObjectAnimated *p);
	bool is_in_inventory(const qdGameObjectAnimated *p) const;
	bool is_on_mouse(const qdGameObjectAnimated *p) const;

	const qdInventory *cur_inventory() const {
		return _cur_inventory;
	}

	bool add_video(qdVideo *p, qdVideo const *before = NULL);
	bool remove_video(const char *name);
	bool remove_video(qdVideo *p);
	qdVideo *get_video(const char *name);
	bool is_video_in_list(const char *name);
	bool is_video_in_list(qdVideo *p);
	bool rename_video(qdVideo *p, const char *name);

	bool add_trigger_chain(qdTriggerChain *p);
	bool remove_trigger_chain(const char *name);
	bool remove_trigger_chain(qdTriggerChain *p);
	qdTriggerChain *get_trigger_chain(const char *name);
	bool is_trigger_chain_in_list(const char *name);
	bool is_trigger_chain_in_list(qdTriggerChain *p);
	bool rename_trigger_chain(qdTriggerChain *p, const char *name);

	bool add_scene(qdGameScene *p);
	bool rename_scene(qdGameScene *p, const char *name);
	bool remove_scene(qdGameScene *p);
	qdGameScene *get_scene(const char *name);
	bool is_scene_in_list(const char *name);
	bool is_scene_in_list(const qdGameScene *p);

	bool add_global_object(qdGameObject *p);
	bool rename_global_object(qdGameObject *p, const char *name);
	bool remove_global_object(const char *name);
	bool remove_global_object(qdGameObject *p);
	qdGameObject *get_global_object(const char *name);
	bool is_global_object_in_list(const char *name);
	bool is_global_object_in_list(qdGameObject *p);

	bool merge_global_objects(qdGameObject *obj = NULL);
	bool split_global_objects(qdGameObject *obj = NULL);
	bool update_walk_state(const char *object_name, qdGameObjectState *p);
	qdGameObjectState *get_walk_state(const char *object_name);

	// Управление видеовставками
	bool play_video(const char *vid_name);
	bool play_video(qdVideo *p);
	bool pause_video();
	bool stop_video();
	bool close_video();
	bool is_video_finished();
	bool is_video_playing() {
		if (!_cur_video) return false;
		return true;
	}

	void continueVideo();
	bool is_video_playing(qdVideo *p) {
		return (_cur_video == p);
	}

	bool play_music_track(const qdMusicTrack *p, bool interface_mode = false);
	const qdMusicTrack *current_music() const {
		return _cur_music_track;
	}
	bool stop_music();

	bool select_scene(const char *s_name);
	bool select_scene(qdGameScene *sp, bool resources_flag = true);

	//! Делает активными все линки, идущие от объекта в триггерах.
	bool activate_trigger_links(const qdNamedObject *p);
	//! Делает неактивными все триггера сцены.
	bool deactivate_scene_triggers(const qdGameScene *p);

	void set_next_scene(qdGameScene *p) {
		_next_scene = p;
	}

	int load_resources();
	void free_resources();
	int get_resources_size();

	void load_script(const char *fname);
	void load_script(const xml::tag *p);
	bool save_script(Common::SeekableWriteStream &fh) const;
	bool save_script(const char *fname) const;

	//! Загрузка данных из сэйва.
	bool load_save(Common::SeekableReadStream *fh);
	//! Запись данных в сэйв.
	bool save_save(Common::WriteStream *stream) const;

	bool save_game(int slot_id) const;

	//! Инициализация данных, вызывается один раз сразу после загрузки скрипта.
	bool init();

	bool init_triggers();
	bool reset_triggers();

	const qdVideoList &video_list() const {
		return _videos.get_list();
	}
	const qdTriggerChainList &trigger_chain_list() const {
		return _trigger_chains.get_list();
	}
	const qdGameObjectList &global_object_list() const {
		return _global_objects.get_list();
	}
	const qdInventoryList &inventory_list() const {
		return _inventories.get_list();
	}
	const qdMiniGameList &minigame_list() const {
		return _minigames.get_list();
	}
	const qdGameEndList &game_end_list() const {
		return _game_ends.get_list();
	}
	const qdGameSceneList &scene_list() const {
		return _scenes.get_list();
	}
	const qdCounterList &counter_list() const {
		return _counters.get_list();
	}

	qdGameScene *get_active_scene() const {
		return _cur_scene;
	}

	qdSound *get_sound(const char *name);
	qdAnimation *get_animation(const char *name);
	qdAnimationSet *get_animation_set(const char *name);
	qdGameObject *get_object(const char *name);
	qdGameObjectMoving *get_active_personage();
	qdNamedObject *get_named_object(const qdNamedObjectReference *ref);

	qdScaleInfo *get_scale_info(const char *p);

	bool check_condition(qdCondition *cnd);

	void pause();
	void resume();
	bool is_paused() {
		return _is_paused;
	}

	bool is_gameplay_paused() {
		return _is_gameplay_paused;
	}

	void pause_gameplay(bool pause) { _is_gameplay_paused = pause; }

	bool restart();

	qdScreenTextDispatcher &screen_texts_dispatcher() {
		return _screen_texts;
	}

	bool toggle_main_menu() {
		return toggle_main_menu(!_interface_dispatcher.is_active());
	}
	bool toggle_main_menu(bool state, const char *screen_name = NULL);
	bool has_main_menu() const {
		return _interface_dispatcher.has_main_menu();
	}
	bool is_main_menu_exit_enabled() const;

	void set_game_end(const qdGameEnd *p) {
		_game_end = p;
	}
	bool end_game(const qdGameEnd *p);

	bool add_redraw_region(const grScreenRegion &reg);

	//! Ставит флаг, что надо выйти из игры.
	void toggle_exit() {
		set_flag(EXIT_FLAG);
	}
	//! Возвращает true, если надо выйти из игры.
	bool need_exit() const {
		return check_flag(EXIT_FLAG);
	}

	//! Ставит флаг, что надо перезапустить игру.
	void toggle_restart() {
		set_flag(RESTART_FLAG);
	}
	//! Возвращает true, если надо перезапустить игру.
	bool need_restart() const {
		return check_flag(RESTART_FLAG);
	}

	void toggle_full_redraw() {
		set_flag(FULLSCREEN_REDRAW_FLAG);
	}
	bool need_full_redraw() const {
		return check_flag(FULLSCREEN_REDRAW_FLAG);
	}

	static void set_dispatcher(qdGameDispatcher *p);
	static qdGameDispatcher *get_dispatcher() {
		return _dispatcher;
	}

	//! Возвращает имя стартовой сцены игры.
	const char *startup_scene() const {
		return _startup_scene.c_str();
	}
	//! Устанавливает имя стартовой сцены игры.
	/**
	Передавать NULL, чтобы скинуть установку.
	*/
	void set_startup_scene(const char *name) {
		if (name)
			_startup_scene = name;
		else
			_startup_scene.clear();
	}
	//! Возвращает true, если задана стартовая сцена игры.
	bool has_startup_scene() const {
		return !_startup_scene.empty();
	}

	bool game_screenshot(Graphics::Surface &thumb) const;

	void set_game_title(const char *p) {
		_game_title = p;
	}
	const char *game_title() const {
		return _game_title.c_str();
	}

	void set_texts_database(const Common::Path file_name) {
		_texts_database = file_name;
	}
	const Common::Path texts_database() const {
		return _texts_database;
	}

	void set_cd_key(const char *key) {
		_cd_key = key;
	}
	const char *cd_key() const {
		return _cd_key.c_str();
	}

	bool add_dialog_state(qdGameObjectState *p);

	const Vect2f &mouse_cursor_pos() const {
		return _mouse_cursor_pos;
	}

	bool is_autosave_slot(int save_slot) {
		return save_slot == _autosave_slot;
	}

	void set_auto_save(int save_slot) {
		set_flag(SAVE_GAME_FLAG);
		_autosave_slot = save_slot;
	}
	void set_auto_load(int save_slot) {
		set_flag(LOAD_GAME_FLAG);
		_autosave_slot = save_slot;
	}

	static grFont *create_font(int font_idx);
	static void free_font(grFont *fnt);

	//! Поиск шрифтов
	qdFontInfo *find_font_info(int type);
	const qdFontInfo *find_font_info(int type) const;
	const grFont *find_font(int type) const;

	bool set_fade(bool fade_in, float duration);

	bool write_resource_stats(const char *file_name) const;

	int hall_of_fame_size() const {
		return _hall_of_fame_size;
	}
	void set_hall_of_fame_size(int size) {
		_hall_of_fame_size = size;
	}

	const char *hall_of_fame_player_name(int place) const;
	void set_hall_of_fame_player_name(int place, const char *name);

	int hall_of_fame_player_score(int place) const;

	bool is_hall_of_fame_updated(int place) const;

	bool has_hall_of_fame() const {
		return _hall_of_fame_size != 0;
	}
	bool load_hall_of_fame();
	bool save_hall_of_fame() const;
	bool add_hall_of_fame_entry(int score);

private:

	int _timer;

	bool _is_paused;
	bool _is_gameplay_paused = false;

	bool _enable_file_packages;
	/// компрессия ресурсов в архивах, 0 - отключена, 9 - максимальная
	int _resource_compression;

	/// таблица рекордов, если нету - размер нулевой
	int _hall_of_fame_size;
	struct HallOfFameEntry {
		Common::String _player_name;
		int _score;

		bool _updated;

		HallOfFameEntry() {
			_score = 0;
			_updated = false;
		}
	};

	typedef Std::vector<HallOfFameEntry> HallOfFame;
	HallOfFame _hall_of_fame;

	qdInterfaceDispatcher _interface_dispatcher;

	winVideo _video_player;
	qdVideo *_cur_video;
	qdObjectListContainer<qdVideo> _videos;

	qdObjectListContainer<qdGameObject> _global_objects;
	qdObjectListContainer<qdTriggerChain> _trigger_chains;
	qdObjectListContainer<qdInventory> _inventories;
	qdObjectListContainer<qdGameEnd> _game_ends;
	qdObjectListContainer<qdGameScene> _scenes;
	qdObjectListContainer<qdCounter> _counters;

	qdGameScene *_cur_scene;
	bool _scene_saved;
	int _autosave_slot;

	//! Если ненулевой - игра заканчивается с переходом на него.
	const qdGameEnd *_game_end;

	qdObjectListContainer<qdMiniGame> _minigames;

	qdInventory *_cur_inventory;
	qdInventoryCellTypeVector _inventory_cell_types;
	//! Шрифты
	qdObjectListContainer<qdFontInfo> _fonts;

	//! Шрифт, используемый для отрисовки текстов, в формате который шрифт не задан.
	int _default_font;

	qdGameObjectMouse *_mouse_obj;
	qdGameObjectAnimated *_mouse_click_obj;
	const qdGameObjectState *_mouse_click_state;
	qdAnimation *_mouse_animation;
	Vect2f _mouse_cursor_pos;

	typedef Std::vector<qdGameObjectState *> dialog_states_container_t;
	dialog_states_container_t _dialog_states;
	dialog_states_container_t _dialog_states_last;

	qdScreenTextDispatcher _screen_texts;

	Common::String _startup_scene;

	//! Файл с субтитрами.
	Common::Path _texts_database;

	//! Название игры.
	Common::String _game_title;

	Common::String _cd_key;

	void *_scene_loading_progress_data;
	qdLoadingProgressFnc _scene_loading_progress_fnc;

	qdGameScene *_next_scene;

	bool _interface_music_mode;
	const qdMusicTrack *_cur_music_track;
	const qdMusicTrack *_cur_interface_music_track;

	float _fade_timer;
	float _fade_duration;

	qdCameraMode _default_camera_mode;

	static qdGameDispatcher *_dispatcher;

	bool init_inventories();

	qdInventoryCellTypeVector::iterator find_inventory_cell_type(int type) {
		return Common::find(_inventory_cell_types.begin(),
		                 _inventory_cell_types.end(), type);
	}

	qdInventoryCellTypeVector::const_iterator find_inventory_cell_type(int type) const {
		return Common::find(_inventory_cell_types.begin(), _inventory_cell_types.end(), type);
	}

	void redraw(const grScreenRegion &reg);
	void redraw_scene(bool draw_interface = true);

	/// включает нужный экран внутриигрового интерфейса
	bool update_ingame_interface();

	/// вытаскивает из интерфейса имена игроков в таблице рекордов
	bool update_hall_of_fame_names();
};

qdGameDispatcher *qd_get_game_dispatcher();

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_DISPATCHER_H
