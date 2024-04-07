#ifndef __QD_GAME_SCENE_H__
#define __QD_GAME_SCENE_H__

#include "xml_fwd.h"
#include "mouse_input.h"

#include "gr_screen_region.h"

#include "qd_camera.h"
#include "qd_conditional_object.h"
#include "qd_game_dispatcher_base.h"

#include "qd_object_map_container.h"
#include "qd_object_list_container.h"

#include "qd_file_owner.h"

#include "fps_counter.h"

class qdMiniGame;

//! Сцена.
class qdGameScene : public qdGameDispatcherBase, public qdFileOwner
{
public:
	qdGameScene();
	~qdGameScene();

	int named_object_type() const { return QD_NAMED_OBJECT_SCENE; }

	//! флаги
	enum {
		/// зациклить сцену по горизонтали
		CYCLE_X		= 0x01,
		/// зациклить сцену по вертикали
		CYCLE_Y		= 0x02,

		/// перезапускать триггера при заходе на сцену
		RESET_TRIGGERS_ON_LOAD = 0x04,

		/// не переключать персонажей с клавиатуры 
		DISABLE_KEYBOARD_PERSONAGE_SWITCH = 0x08,

		/// не давать выйти в главное меню по ESC
		DISABLE_MAIN_MENU = 0x10
	};

#ifdef _QUEST_EDITOR
	void activate_object(qdGameObject* p){ active_object_ = p; }
	qdGameObject* get_active_object() const { return active_object_; }
#endif

	qdNamedObject* mouse_click_object(){ return mouse_click_object_; }
	void set_mouse_click_object(qdNamedObject* p){ mouse_click_object_ = p; }

	qdNamedObject* mouse_right_click_object(){ return mouse_right_click_object_; }
	void set_mouse_right_click_object(qdNamedObject* p){ mouse_right_click_object_ = p; }

	qdNamedObject* mouse_hover_object(){ return mouse_hover_object_; }
	void set_mouse_hover_object(qdNamedObject* p){ mouse_hover_object_ = p; }

	const Vect2f mouse_click_pos() const { return mouse_click_pos_; }

	bool need_to_redraw_inventory(const char* inventory_name) const;

	const char* minigame_name() const { return minigame_name_.c_str(); }
	void set_minigame_name(const char* name){ 
		minigame_name_ = name;
	}
	bool has_minigame() const { return !minigame_name_.empty(); }

#ifndef _QUEST_EDITOR
	const qdMiniGame* minigame() const { return minigame_; }
	void start_minigame();
#endif

	bool restart_minigame_on_load() const { return restart_minigame_on_load_; }
	void toggle_restart_minigame_on_load(bool state){ restart_minigame_on_load_ = state; }

	bool add_object(qdGameObject* p);
	bool rename_object(qdGameObject* p,const char* name);
	bool remove_object(const char* name);
	bool remove_object(qdGameObject* p);
	qdGameObject* get_object(const char* name);
	bool is_object_in_list(const char* name);
	bool is_object_in_list(qdGameObject* p);

	bool is_active() const;

	void set_active_personage(qdGameObjectMoving* p);
	void set_active_object(qdGameObjectAnimated* p);
	qdGameObjectMoving* get_active_personage(){ return selected_object_; }
	bool change_active_personage(void);

	bool set_personage_button(qdInterfaceButton* p);

	bool add_grid_zone(qdGridZone* p);
	bool rename_grid_zone(qdGridZone* p,const char* name);
	bool remove_grid_zone(const char* name);
	bool remove_grid_zone(qdGridZone* p);
	qdGridZone* get_grid_zone(const char* name);
	bool is_grid_zone_in_list(const char* name);
	bool is_grid_zone_in_list(qdGridZone* p);

#ifdef _QUEST_EDITOR
	bool insert_grid_zone(qdGridZone* p, const qdGridZone* before);
#endif // _QUEST_EDITOR

	bool is_any_personage_in_zone(const qdGridZone* p) const;

	bool add_music_track(qdMusicTrack* p);
	bool rename_music_track(qdMusicTrack* p,const char* name);
	bool remove_music_track(const char* name);
	bool remove_music_track(qdMusicTrack* p);
	qdMusicTrack* get_music_track(const char* name);
	bool is_music_track_in_list(const char* name) const;
	bool is_music_track_in_list(qdMusicTrack* p) const;

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

	void add_redraw_region(const grScreenRegion& reg) const;

	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	int load_resources();
	void free_resources();

	qdCamera* get_camera() { return &camera; }
	const qdCamera* get_camera() const { return &camera; }

	const qdGameObjectList& object_list() const { return objects.get_list(); }
	const qdGridZoneList& grid_zone_list() const { return grid_zones.get_list(); }
	const qdMusicTrackList& music_track_list() const { return music_tracks.get_list(); }

	bool merge_global_objects(qdGameObject* obj = NULL);
	bool split_global_objects(qdGameObject* obj = NULL);

	int get_resources_size();

	void inc_zone_update_count(){ zone_update_count_ ++; }
	unsigned int zone_update_count() const { return zone_update_count_; }

	qdConditionalObject::trigger_start_mode trigger_start();

	static fpsCounter& fps_counter(){ return fps_counter_; }

	int autosave_slot() const {	return autosave_slot_; }
	void set_autosave_slot(int slot){ autosave_slot_ = slot; }

	bool has_interface_screen() const { return !interface_screen_name_.empty(); }
	const char* interface_screen_name() const { return interface_screen_name_.c_str(); }
	void set_interface_screen_name(const char* name) { interface_screen_name_ = name; }

	bool adjust_files_paths(const char* copy_dir, const char* pack_dir, bool canoverwrite = false);
	bool get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const;

	qdGameObject* get_hitted_obj(int x, int y);

	bool set_camera_mode(const qdCameraMode& mode,qdGameObjectAnimated* object);

#ifdef __QD_DEBUG_ENABLE__
	bool get_resources_info(qdResourceInfoContainer& infos) const;
#endif

protected:
#ifdef _QUEST_EDITOR
	void reload_personage_list();
#endif // _QUEST_EDITOR
private:

	int autosave_slot_;

	qdObjectMapContainer<qdGameObject> objects;
	qdObjectListContainer<qdGridZone> grid_zones;
	qdObjectListContainer<qdMusicTrack> music_tracks;

	qdCamera camera;

	//! текущий персонаж
	qdGameObjectMoving* selected_object_;

#ifdef _QUEST_EDITOR
	qdGameObject* active_object_;		// Активный объект
#else
	/// миниигра, управляющая сценой
	qdMiniGame* minigame_;
#endif
	/// true если нужен перезапуск миниигры при загрузке из сэйва
	bool restart_minigame_on_load_;

	typedef std::vector<qdGameObjectMoving*> personages_container_t;
	//! список персонажей сцены
	personages_container_t personages_;

	//! кликнутый мышью объект
	qdNamedObject* mouse_click_object_;
	//! кликнутый правой кнопкой мыши объект
	qdNamedObject* mouse_right_click_object_;
	//! объект, над кторым мышиный курсор
	qdNamedObject* mouse_hover_object_;

	//! кликнутая мышью точка на плоскости сцены
	Vect2f mouse_click_pos_;

	/// имя миниигры, управляющей сценой
	std::string minigame_name_;

	/// используемый для сцены интерфейсный экран
	std::string interface_screen_name_;

	unsigned int zone_update_count_;

	static std::vector<qdGameObject*> visible_objects_;

	static fpsCounter fps_counter_;
	static grScreenRegion fps_region_;
	static grScreenRegion fps_region_last_;
	static char fps_string_[255];

	bool init_visible_objects_list();
	void update_mouse_cursor();

	void personages_quant();

	//! Инициализирует персонажей, участвующих в следовании
	void follow_pers_init(int follow_cond);
	//! Пытается найти путь к точке следования для pObj. Если идти нужно, но не удастся, то false.
	bool follow_path_seek(qdGameObjectMoving* pObj, bool lock_target);

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

#ifndef _QUEST_EDITOR
	void create_minigame_objects();
#endif // _QUEST_EDITOR
};

#endif /* __QD_GAME_SCENE_H__ */
