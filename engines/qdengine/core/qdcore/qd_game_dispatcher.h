
#ifndef __QD_GAME_DISPATCHER_H__
#define __QD_GAME_DISPATCHER_H__

#include "xml_fwd.h"

#include "WinVideo.h"
#include "mouse_input.h"
#include "gr_screen_region.h"

#include "qd_camera_mode.h"
#include "qd_object_list_container.h"
#include "qd_game_dispatcher_base.h"
#include "qd_resource_dispatcher.h"
#include "qd_screen_text_dispatcher.h"

#include "qd_interface_dispatcher.h"

#include "qd_inventory.h"
#include "qd_font_info.h"
#include "qd_file_owner.h"

class grFont;
class qdGameDispatcher : public qdGameDispatcherBase, public qdResourceDispatcher<qdNamedObject>, public qdFileOwner
{
public:
	//! Идентификаторы наборов текстов.
	enum {
		//! Фразы диалогов, отображаемые в данный момент на экране
		TEXT_SET_DIALOGS = 0
	};

	//! Флаги
	enum {
		EXIT_FLAG			= 0x01,
		RESTART_FLAG			= 0x02,
		OBJECT_CLICK_FLAG		= 0x04,
		DIALOG_CLICK_FLAG		= 0x08,
		FULLSCREEN_REDRAW_FLAG		= 0x10,
		SKIP_REDRAW_FLAG		= 0x20,
		MAIN_MENU_FLAG			= 0x40,
		INTRO_MODE_FLAG			= 0x80,
		CLICK_FAILED_FLAG		= 0x100,
		OBJECT_CLICK_FAILED_FLAG	= 0x200,
		CLICK_WAS_FAILED_FLAG		= 0x400,
		OBJECT_CLICK_WAS_FAILED_FLAG	= 0x800,
		LOAD_GAME_FLAG		= 0x1000,
		SAVE_GAME_FLAG		= 0x2000,
		FADE_IN_FLAG		= 0x4000,
		FADE_OUT_FLAG		= 0x8000,
		NEXT_FRAME_FLAG		= 0x10000
	};

	qdGameDispatcher();
	~qdGameDispatcher();

	int named_object_type() const { return QD_NAMED_OBJECT_DISPATCHER; }

	qdGameObjectMouse* mouse_object(){ return mouse_obj_; }
	qdGameObjectMouse const* mouse_object () const { return mouse_obj_; }
	bool drop_mouse_object();

	int CD_count() const;

	int resource_compression() const { return resource_compression_; }
	void set_resource_compression(int compression){ resource_compression_ = compression; }
	
	//! Просит вставить диск с игрой.
	void request_CD(const qdFileOwner& file_owner) const;
	//! Просит вставить диск номер cd_id (считая от нуля).
	void request_CD(int cd_id) const;

	void request_file_package(const qdFileOwner& file_owner) const;
	const char* find_file(const char* file_name,const qdFileOwner& file_owner) const;
	void startup_check() const;

	qdLoadingProgressFnc set_scene_loading_progress_callback(qdLoadingProgressFnc p,void* dp = 0){
		qdLoadingProgressFnc old_fnc = scene_loading_progress_fnc_;
		scene_loading_progress_fnc_ = p;
		scene_loading_progress_data_ = dp;

		return old_fnc;
	}

	qdLoadingProgressFnc get_scene_loading_progress_callback(){
		return scene_loading_progress_fnc_;
	}

	void* get_scene_loading_progress_data(){
		return scene_loading_progress_data_;
	}

	void set_time(int tm){ timer_ = tm; }
	int time() const { return timer_; }

	bool start_intro_videos();

	void quant();
	void quant(float dt);
	void pre_redraw();
	void redraw();
	void post_redraw();

	void update_time();

	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	bool keyboard_handler(int vkey,bool event);

	bool add_game_end(qdGameEnd* p);
	bool rename_game_end(qdGameEnd* p,const char* name);
	bool remove_game_end(qdGameEnd* p);
	qdGameEnd* get_game_end(const char* name);
	bool is_game_end_in_list(const char* name);
	bool is_game_end_in_list(qdGameEnd* p);

	bool add_counter(qdCounter* p);
	bool rename_counter(qdCounter* p,const char* name);
	bool remove_counter(qdCounter* p);
	qdCounter* get_counter(const char* name);
	bool is_counter_in_list(const char* name);
	bool is_counter_in_list(qdCounter* p);

	bool add_minigame(qdMiniGame* p);
	bool rename_minigame(qdMiniGame* p,const char* name);
	bool remove_minigame(const char* name);
	bool remove_minigame(qdMiniGame* p);
	qdMiniGame* get_minigame(const char* name);
	bool is_minigame_in_list(const char* name);
	bool is_minigame_in_list(qdMiniGame* p);

	void set_mouse_click_state(const qdGameObjectState* p){ mouse_click_state_ = p; }

	bool add_inventory(qdInventory* p){ 
		if(inventories.add_object(p)){ p -> set_owner(this); return 1; } 
		return 0; 
	}
	bool remove_inventory(const char* name){ return inventories.remove_object(name); }
	bool remove_inventory(qdInventory* p){ return inventories.remove_object(p); }
	qdInventory* get_inventory(const char* name){ return inventories.get_object(name); }
	bool is_inventory_in_list(const char* name){ return inventories.is_in_list(name); }
	bool is_inventory_in_list(qdInventory* p){ return inventories.is_in_list(p); }
	bool rename_inventory(qdInventory* p,const char* name);

	const qdInventoryCellType* add_inventory_cell_type(const qdInventoryCellType& tp){ 
		//проверяем уникальность идентификатора, вставляемого типа
		assert(find_inventory_cell_type(tp.type()) == 
									inventory_cell_types_.end());
		inventory_cell_types_.push_back(tp); 

		return &inventory_cell_types_.back(); 
	}
	bool remove_inventory_cell_type(int type){
		typedef qdInventoryCellTypeVector::iterator i_t;
		i_t res = find_inventory_cell_type(type);
		if (res != inventory_cell_types_.end()){
			inventory_cell_types_.erase(res);
			return true;
		}
		return false;
	}
	const qdInventoryCellType* get_inventory_cell_type(int type) const {
		typedef qdInventoryCellTypeVector::const_iterator ci_t;
		ci_t res = find_inventory_cell_type(type);
		if (res != inventory_cell_types_.end())
			return &*res;
		return NULL;
	}
	int get_unique_inventory_cell_type() const{
		if (inventory_cell_types_.empty()) return 0;
		return inventory_cell_types_.back().type() + 1;
	}
	bool set_inventory_cell_type(const qdInventoryCellType& tp){
		typedef qdInventoryCellTypeVector::iterator i_t;
		i_t res = find_inventory_cell_type(tp.type());
		if (res != inventory_cell_types_.end()){
			*res = tp;
			return true;
		}
		return false;
	}
	const qdInventoryCellTypeVector& inventory_cell_types() const{
		return inventory_cell_types_;
	}

	qdInventoryCellTypeVector& not_const_inventory_cell_types() {
		return inventory_cell_types_;
	}

	//! Операции со шрифтами
	/**
	Для создания типов всех добавляемых шрифтов нужно 
	использовать get_unique_font_info_type().
	Самостоятельное задание типов шрифтов крайне не рекомендуется - 
	возможны проблемы с уникальностью типов шрифтов и извлечением шрифта по типу.
	*/
	bool add_font_info(qdFontInfo* fi);
	bool remove_font_info(qdFontInfo* fi);
	bool rename_font_info(qdFontInfo* fi, char const* name);

	const qdFontInfo* get_font_info(int type) const;
	int get_unique_font_info_type() const;
	bool set_font_info(const qdFontInfo& fi);
	const qdFontInfoList& fonts_list() const{
		return fonts_.get_list();
	}

	int default_font() const { return default_font_; }
	void set_default_font(int font) { default_font_ = font; }

	bool toggle_inventory(bool state);
	bool toggle_inventory(){
		if(!cur_inventory_) return toggle_inventory(true);
		else return toggle_inventory(false);
	}
	bool put_to_inventory(qdGameObjectAnimated* p);
	bool remove_from_inventory(qdGameObjectAnimated* p);
	bool is_in_inventory(const qdGameObjectAnimated* p) const;
	bool is_on_mouse(const qdGameObjectAnimated* p) const;

	const qdInventory* cur_inventory() const { return cur_inventory_; }

	bool add_video(qdVideo* p, qdVideo const* before = NULL);
	bool remove_video(const char* name);
	bool remove_video(qdVideo* p);
	qdVideo* get_video(const char* name);
	bool is_video_in_list(const char* name);
	bool is_video_in_list(qdVideo* p);
	bool rename_video(qdVideo* p,const char* name);

	bool add_trigger_chain(qdTriggerChain* p);
	bool remove_trigger_chain(const char* name);
	bool remove_trigger_chain(qdTriggerChain* p);
	qdTriggerChain* get_trigger_chain(const char* name);
	bool is_trigger_chain_in_list(const char* name);
	bool is_trigger_chain_in_list(qdTriggerChain* p);
	bool rename_trigger_chain(qdTriggerChain* p,const char* name);

	bool add_scene(qdGameScene* p);
	bool rename_scene(qdGameScene* p,const char* name);
	bool remove_scene(qdGameScene* p);
	qdGameScene* get_scene(const char* name);
	bool is_scene_in_list(const char* name);
	bool is_scene_in_list(const qdGameScene* p);

	bool add_global_object(qdGameObject* p);
	bool rename_global_object(qdGameObject* p,const char* name);
	bool remove_global_object(const char* name);
	bool remove_global_object(qdGameObject* p);
	qdGameObject* get_global_object(const char* name);
	bool is_global_object_in_list(const char* name);
	bool is_global_object_in_list(qdGameObject* p);

	bool merge_global_objects(qdGameObject* obj = NULL);
	bool split_global_objects(qdGameObject* obj = NULL);
#ifndef _QUEST_EDITOR
	bool update_walk_state(const char* object_name,qdGameObjectState* p);
	qdGameObjectState* get_walk_state(const char* object_name);
#endif // _QUEST_EDITOR

	// Управление видеовставками
	bool play_video(const char* vid_name);
	bool play_video(qdVideo* p);
	bool pause_video();
	bool stop_video();
	bool close_video();
	bool is_video_finished();
	bool is_video_playing(){ if(!cur_video_) return false; return true; }
	bool is_video_playing(qdVideo* p){ return (cur_video_ == p); }

	bool play_music_track(const qdMusicTrack* p,bool interface_mode = false);
	const qdMusicTrack* current_music() const { return cur_music_track_; }
	bool stop_music();

	bool select_scene(const char* s_name);
	bool select_scene(qdGameScene* sp,bool resources_flag = true);

	//! Делает активными все линки, идущие от объекта в триггерах.
	bool activate_trigger_links(const qdNamedObject* p);
	//! Делает неактивными все триггера сцены.
	bool deactivate_scene_triggers(const qdGameScene* p);

	void set_next_scene(qdGameScene* p){ next_scene_ = p; }
	
	int load_resources();
	void free_resources();
	int get_resources_size();

	void load_script(const char* fname);
	void load_script(const xml::tag* p);
	bool save_script(XStream& fh) const;
	bool save_script(const char* fname) const;

	//! Загрузка данных из сэйва.
	bool load_data(const char* fname);
	//! Запись данных в сэйв.
	bool save_data(const char* fname) const;

	bool load_game(int slot_id);
	bool save_game(int slot_id) const;

	//! Инициализация данных, вызывается один раз сразу после загрузки скрипта.
	bool init();

	bool init_triggers();
	bool reset_triggers();

	const qdVideoList& video_list() const { return videos.get_list(); }
	const qdTriggerChainList& trigger_chain_list() const { return trigger_chains.get_list(); }
	const qdGameObjectList& global_object_list() const { return global_objects.get_list(); }
	const qdInventoryList& inventory_list() const { return inventories.get_list(); }
	const qdMiniGameList& minigame_list() const { return minigames.get_list(); }
	const qdGameEndList& game_end_list() const { return game_ends_.get_list(); }
	const qdGameSceneList& scene_list() const { return scenes_.get_list(); }
	const qdCounterList& counter_list() const { return counters_.get_list(); }

	qdGameScene* get_active_scene() const { return cur_scene_; }

	qdSound* get_sound(const char* name);
	qdAnimation* get_animation(const char* name);
	qdAnimationSet* get_animation_set(const char* name);
	qdGameObject* get_object(const char* name);
	qdGameObjectMoving* get_active_personage();
	qdNamedObject* get_named_object(const qdNamedObjectReference* ref);

	qdScaleInfo* get_scale_info(const char* p);

	bool check_condition(qdCondition* cnd);

	void pause();
	void resume();
	bool is_paused(){ return is_paused_; }

	bool restart();

	qdScreenTextDispatcher& screen_texts_dispatcher(){ return screen_texts; }

	bool toggle_main_menu(){ return toggle_main_menu(!interface_dispatcher_.is_active()); }
	bool toggle_main_menu(bool state,const char* screen_name = NULL);
	bool has_main_menu() const { return interface_dispatcher_.has_main_menu(); }
	bool is_main_menu_exit_enabled() const;

	void set_game_end(const qdGameEnd* p){ game_end_ = p; }
	bool end_game(const qdGameEnd* p);

	bool add_redraw_region(const grScreenRegion& reg);

	//! Ставит флаг, что надо выйти из игры.
	void toggle_exit(){ set_flag(EXIT_FLAG); }
	//! Возвращает true, если надо выйти из игры.
	bool need_exit() const { return check_flag(EXIT_FLAG); }

	//! Ставит флаг, что надо перезапустить игру.
	void toggle_restart(){ set_flag(RESTART_FLAG); }
	//! Возвращает true, если надо перезапустить игру.
	bool need_restart() const { return check_flag(RESTART_FLAG); }

	void toggle_full_redraw(){ set_flag(FULLSCREEN_REDRAW_FLAG); }
	bool need_full_redraw() const { return check_flag(FULLSCREEN_REDRAW_FLAG); }

	static void set_dispatcher(qdGameDispatcher* p);
	static qdGameDispatcher* get_dispatcher(){ return dispatcher_; }

	//! Возвращает имя стартовой сцены игры.
	const char* startup_scene() const { return startup_scene_.c_str(); }
	//! Устанавливает имя стартовой сцены игры.
	/**
	Передавать NULL, чтобы скинуть установку.
	*/
	void set_startup_scene(const char* name){
		if(name)
			startup_scene_ = name;
		else
			startup_scene_.clear();
	}
	//! Возвращает true, если задана стартовая сцена игры.
	bool has_startup_scene() const { return !startup_scene_.empty(); }

	bool game_screenshot(const char* file_name,int sx,int sy) const;

	void scan_files(qdLoadingProgressFnc progress_fnc,void* context_ptr);
	//! Правит пути файлов глобальных объектов: файлы без упаковки в copy_dir, пакетные в pack_dir
	bool adjust_global_object_files_paths(const char* copy_dir, const char* pack_dir, bool can_overwrite = false);
	/** 
		Собирает все файлы находящиеся НЕ внутри рабочей дирректории в collect_dir,
		исправляя пути файлов в соответствующих им объектах
	*/
	bool collect_all_external_files(const char* collector_dir);
	bool get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const;

	void set_game_title(const char* p){ game_title_ = p; }
	const char* game_title() const { return game_title_.c_str(); }

	void set_texts_database(const char* file_name){ texts_database_ = file_name; }
	const char* texts_database() const { return texts_database_.c_str(); }

	void set_cd_key(const char* key){ cd_key_ = key; }
	const char* cd_key() const { return cd_key_.c_str(); }

	bool add_dialog_state(qdGameObjectState* p);

	const Vect2f& mouse_cursor_pos() const { return mouse_cursor_pos_; }

	void set_auto_save(int save_slot){ set_flag(SAVE_GAME_FLAG); autosave_slot_ = save_slot; }
	void set_auto_load(int save_slot){ set_flag(LOAD_GAME_FLAG); autosave_slot_ = save_slot; }

	enum SaveFileType
	{
		SAVE_FILE,
		SAVE_THUMBNAIL,
		SAVE_DESCRIPTION
	};

	//! Возвращает имя файла сэйва или имя файла превью сэйва.
	static const char* get_save_name(int slot_id, SaveFileType file_type = SAVE_FILE);

	static grFont* create_font(int font_idx);
	static void free_font(grFont* fnt);

	//! Поиск шрифтов
	qdFontInfo* find_font_info(int type);
	const qdFontInfo* find_font_info(int type) const;
	const grFont* find_font(int type) const;

	//! Копирование ресурсов в папку и из папки
	bool copy_resources_to_folder(const char* dest_dir,const char* file_extension,qdLoadingProgressFnc callback = NULL,void* callback_context = NULL);
	bool copy_resources_from_folder(const char* src_dir,const char* file_extension,qdLoadingProgressFnc callback = NULL,void* callback_context = NULL);

	bool set_fade(bool fade_in, float duration);

	bool write_resource_stats(const char* file_name) const;

	int hall_of_fame_size() const { return hall_of_fame_size_; }
	void set_hall_of_fame_size(int size){ hall_of_fame_size_ = size; }

	const char* hall_of_fame_player_name(int place) const;
	void set_hall_of_fame_player_name(int place, const char* name);

	int hall_of_fame_player_score(int place) const;

	bool is_hall_of_fame_updated(int place) const;

	bool has_hall_of_fame() const { return hall_of_fame_size_ != 0; }
	bool load_hall_of_fame();
	bool save_hall_of_fame() const;
	bool add_hall_of_fame_entry(int score);

private:

	int timer_;

	bool is_paused_;

	bool enable_file_packages_;
	/// компрессия ресурсов в архивах, 0 - отключена, 9 - максимальная
	int resource_compression_;

	/// таблица рекордов, если нету - размер нулевой
	int hall_of_fame_size_;
	struct HallOfFameEntry {
		std::string player_name_;
		int score_;

		bool updated_;

		HallOfFameEntry(){ score_ = 0; updated_ = false; }
	};

	typedef std::vector<HallOfFameEntry> HallOfFame;
	HallOfFame hall_of_fame_;

	qdInterfaceDispatcher interface_dispatcher_;

	winVideo video_player_;
	qdVideo* cur_video_;
	qdObjectListContainer<qdVideo> videos;

	qdObjectListContainer<qdGameObject> global_objects;
	qdObjectListContainer<qdTriggerChain> trigger_chains;
	qdObjectListContainer<qdInventory> inventories;
	qdObjectListContainer<qdGameEnd> game_ends_;
	qdObjectListContainer<qdGameScene> scenes_;
	qdObjectListContainer<qdCounter> counters_;

	qdGameScene* cur_scene_;
	bool scene_saved_;
	int autosave_slot_;

	//! Если ненулевой - игра заканчивается с переходом на него.
	const qdGameEnd* game_end_;

	qdObjectListContainer<qdMiniGame> minigames;

	qdInventory* cur_inventory_;
	qdInventoryCellTypeVector inventory_cell_types_;
	//! Шрифты
	qdObjectListContainer<qdFontInfo> fonts_;

	//! Шрифт, используемый для отрисовки текстов, в формате который шрифт не задан.
	int default_font_;

	qdGameObjectMouse* mouse_obj_;
	qdGameObjectAnimated* mouse_click_obj_;
	const qdGameObjectState* mouse_click_state_;
	qdAnimation* mouse_animation_;
	Vect2f mouse_cursor_pos_;

	typedef std::vector<qdGameObjectState*> dialog_states_container_t;
	dialog_states_container_t dialog_states_;
	dialog_states_container_t dialog_states_last_;

	qdScreenTextDispatcher screen_texts;

	std::string startup_scene_;

	//! Файл с субтитрами.
	std::string texts_database_;

	//! Название игры.
	std::string game_title_;

	std::string cd_key_;

	void* scene_loading_progress_data_;
	qdLoadingProgressFnc scene_loading_progress_fnc_;

	qdGameScene* next_scene_;

	bool interface_music_mode_;
	const qdMusicTrack* cur_music_track_;
	const qdMusicTrack* cur_interface_music_track_;

	float fade_timer_;
	float fade_duration_;

	qdCameraMode default_camera_mode_;

	static qdGameDispatcher* dispatcher_;

	bool init_inventories();

	qdInventoryCellTypeVector::iterator find_inventory_cell_type(int type){
		return std::find(inventory_cell_types_.begin(), 
			inventory_cell_types_.end(), type);
	}

	qdInventoryCellTypeVector::const_iterator find_inventory_cell_type(int type) const {
		return std::find(inventory_cell_types_.begin(),inventory_cell_types_.end(),type);
	}

	void redraw(const grScreenRegion& reg);
	void redraw_scene(bool draw_interface = true);

	void hide_game_window() const;
	void restore_game_window() const;
	const char* cd_request_string(int cd_id) const;

	/// включает нужный экран внутриигрового интерфейса
	bool update_ingame_interface();

#ifdef __QD_DEBUG_ENABLE__
	unsigned write_resource_stats(XStream& fh,const qdResourceInfoContainer& info,const char* title = NULL,qdResource::file_format_t filter = qdResource::RES_UNKNOWN) const;
#endif

	/// вытаскивает из интерфейса имена игроков в таблице рекордов
	bool update_hall_of_fame_names();
};

qdGameDispatcher* qd_get_game_dispatcher();

#endif /* __QD_GAME_DISPATCHER_H__ */

