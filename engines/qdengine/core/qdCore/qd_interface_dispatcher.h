
#ifndef __QD_INTERFACE_DISPATCHER_H__
#define __QD_INTERFACE_DISPATCHER_H__

#include "qd_resource_container.h"
#include "qd_interface_object_base.h"
#include "qd_object_list_container.h"

class qdInterfaceScreen;
class qdInterfaceElementState;

class qdInterfaceDispatcher : public qdInterfaceObjectBase
{
public:
	qdInterfaceDispatcher();
	~qdInterfaceDispatcher();

	/// для чего вызван модальный экран
	enum ModalScreenMode
	{
		MODAL_SCREEN_SAVE_OVERWRITE,
		MODAL_SCREEN_SAVE_NAME_EDIT,
		MODAL_SCREEN_OTHER
	};

	//! Отрисовка интерфейса.
	bool redraw(int dx = 0,int dy = 0) const;
	bool pre_redraw();
	bool post_redraw();

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Добавление экрана.
	bool add_screen(qdInterfaceScreen* scr);
	//! Изменение имени экрана.
	bool rename_screen(qdInterfaceScreen* scr,const char* name);
	//! Удаление экрана из списка.
	bool remove_screen(qdInterfaceScreen* scr);
	//! Поиск экрана по имени.
	qdInterfaceScreen* get_screen(const char* screen_name);
	//! Возвращает true, если экран есть в списке.
	bool is_screen_in_list(const qdInterfaceScreen* scr);

	typedef std::list<qdInterfaceScreen*> screen_list_t;
	//! Возвращает список экранов.
	const screen_list_t& screen_list() const { return screens_.get_list(); }

	//! Установка активного экрана.
	/**
	Имя можно передавать нулевое, для сброса активного экрана.
	*/
	bool select_screen(const char* screen_name,bool lock_resources = false);
	/// Установка фонового экрана.
	bool select_background_screen(qdInterfaceScreen* p);

	//! Включает внутриигровой интерфейс.
	bool select_ingame_screen(bool inventory_state = false);

	//! Включает кнопки, связанные с персонажем p, выключает кнопки остальных персонажей.
	void update_personage_buttons();

	qdInterfaceScreen* selected_screen() const{ return cur_screen_;}

	//! Возвращает true, если экран p в данный момент активен.
	bool is_screen_active(const qdInterfaceScreen* p) const { return (cur_screen_ == p); }

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource* add_resource(const char* file_name,const qdInterfaceElementState* owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const char* file_name,const qdInterfaceElementState* owner);
	//! Возвращает указатель на ресурс file_name.
	qdResource* get_resource(const char* file_name) const { return resources_.get_resource(file_name); }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);
	bool char_input_handler(int vkey);

	//! Запись данных в скрипт.
	bool save_script(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);

	//! Установка имени экрана с главным меню игры.
	void set_main_menu_screen(const char* name);
	//! Возвращает имя экрана с главным меню игры.
	const char* main_menu_screen_name() const { return main_menu_screen_name_.c_str(); }
	//! Возвращает true, если установлено имя экрана с главным меню игры.
	bool has_main_menu() const { return !main_menu_screen_name_.empty(); }

	//! Установка имени внутриигрового экрана.
	void set_ingame_screen(const char* name,bool inventory_state = false);
	//! Возвращает имя внутриигрового экрана.
	const char* ingame_screen_name(bool inventory_state = false) const { return ingame_screen_names_[inventory_state].c_str(); }
	//! Возвращает true, если установлено имя внутриигрового экрана.
	bool has_ingame_screen(bool inventory_state = false) const { return !ingame_screen_names_[inventory_state].empty(); }

	//! Имя модального экрана подсказки: перезаписывать сэйв или нет.
	const char* save_prompt_screen_name() const { return save_prompt_screen_name_.c_str(); }
	void set_save_prompt_screen_name(const char* str) { if(str) save_prompt_screen_name_ = str; else save_prompt_screen_name_ = ""; }
	bool has_save_prompt_screen() const { return !save_prompt_screen_name_.empty(); }

	const char* save_title_screen_name() const { return save_title_screen_name_.c_str(); }
	void set_save_title_screen_name(const char* name){ save_title_screen_name_ = name; }
	bool has_save_title_screen() const { return !save_title_screen_name_.empty(); }

	//! Нужно ли выводить скриншот к сохраненке
	bool need_save_screenshot() const { return need_save_screenshot_; }
	void toggle_save_screenshot(bool state) { need_save_screenshot_ = state; }
	//! Нужно ли выводить дату и время при отображении сэйва
	bool need_show_save_time() const { return need_show_save_time_; }
	void toggle_show_save_time(bool state) { need_show_save_time_ = state; }
	bool need_show_save_title() const { return need_show_save_title_; }
	void toggle_show_save_title(bool state) { need_show_save_title_ = state; }
	//! Тип шрифт, которым выводится текст сэйва (в частности дата и время)
	int save_font_type() const { return save_font_type_; }
	void set_save_font_type(int type) { save_font_type_ = type; }
	//! Цвет, которым выводится текст сэйва (в частности дата и время)
	int save_font_color() const { return save_font_color_; }
	void set_save_font_color(int clr) { save_font_color_ = clr; }

	//! Возвращает true, если интерфейс отрисовывается поверх сцены.
	bool need_scene_redraw() const { return need_scene_redraw_; }
	//! Устанавливает, надо ли если интерфейсу отрисовываться поверх сцены.
	void set_scene_redraw(bool state){ need_scene_redraw_ = state; }

	static void set_dispatcher(qdInterfaceDispatcher* p){ dispatcher_ = p; }
	static qdInterfaceDispatcher* get_dispatcher(){ return dispatcher_; }

	void activate(){ is_active_ = true; }
	void deactivate(){ is_active_ = false; }
	bool is_active() const { return is_active_; }

	bool is_mouse_hover() const { return is_mouse_hover_; }
	void toggle_mouse_hover(){ is_mouse_hover_ = true; }
	bool is_autohide_enabled() const { return !autohide_disable_; }
	void disable_autohide(){ autohide_disable_ = true; }

	ModalScreenMode modalScreenMode() const { return modalScreenMode_; }
	void setModalScreenMode(ModalScreenMode mode){ modalScreenMode_ = mode; }

	bool set_save_title(const char* title);
	const char* get_save_title() const;

	/** 
	Обработка события. 
	sender - тот, кто послал событие на выполнение. NULL - sender не известен. 
	*/
	bool handle_event(int event_code,const char* event_data,qdInterfaceObjectBase* sender=NULL);

	bool adjust_pack_files(const char* pack_dir, bool can_overwrite = false);
	//! Пишет список всех файлов ресурсов в list.
	bool get_file_list(qdFileNameList& files_to_copy, qdFileNameList& files_to_pack) const;

	void toggle_end_game_mode(bool state){ end_game_mode_ = state; }

#ifndef _QUEST_EDITOR
	//! Устанавливает следующий экран.
	void set_next_screen(const char* screen_name){ next_screen_ = screen_name; }
#endif

	static int option_value(int option_id,const char* option_data = NULL);
	static bool set_option_value(int option_id,int value,const char* oprtion_data = NULL);

#ifdef __QD_DEBUG_ENABLE__
	bool get_resources_info(qdResourceInfoContainer& infos) const;
#endif

private:

	//! Активный интерфейсный экран.
	qdInterfaceScreen* cur_screen_;

	/// Фоновый экран, рисуется под активным
	qdInterfaceScreen* background_screen_;
	/// true если ресурсы фонового экрана не надо выгружать
	bool background_screen_lock_;

	ModalScreenMode modalScreenMode_;

	//! Имя экрана с главным меню игры
	std::string main_menu_screen_name_;

	//! Имена экраноы с внутриигровым интерфейсом.
	/**
	Первый экран показывается когда инвентори спрятано, вотрой - когда оно активно.
	*/
	std::string ingame_screen_names_[2];

	/// Экран ввода имени сэйва
	std::string save_title_screen_name_;
	std::string save_title_;

	//! Экран, вызывающийся при подтверждении перезаписи файла сэйва
	std::string save_prompt_screen_name_;

	//! Нужно ли выводить скриншот к сохраненке
	bool need_save_screenshot_;
	//! Нужно ли выводить дату и время при отображении сэйва
	bool need_show_save_time_;
	//! Нужно ли выводить имя при отображении сэйва
	bool need_show_save_title_;
	//! Тип шрифта, которым выводится текст сэйва (в частности дата и время)
	int save_font_type_;
	//! Цвет, которым выводится текст сэйва (в частности дата и время)
	int save_font_color_;

	//! Интерфейсные экраны.
	qdObjectListContainer<qdInterfaceScreen> screens_;

	typedef qdResourceContainer<qdInterfaceElementState> resource_container_t;
	//! Интерфейсные ресурсы.
	resource_container_t resources_;

	//! Равно true, если интерфейс активен.
	bool is_active_;

	//! Равно true, если курсор мыши попадает в интерфейс.
	bool is_mouse_hover_;
	/// true если надо показывать 
	bool autohide_disable_;

	//! Равно true, если требуется полная отрисовка интерфейса, после отрисовки скидывается.
	bool need_full_redraw_;

	//! Равно true, если отрисовка сцены под интерфейсом.
	bool need_scene_redraw_;

	//! Режим окончания игры - после любого клика возврат с текущего экрана в главное меню.
	//! После загрузки меню скидывается в false.
	bool end_game_mode_;

#ifndef _QUEST_EDITOR
	//! Имя экрана, на который надо перейти.
	const char* next_screen_;
#endif

	//! Текущий диспетчер интерфейса.
	static qdInterfaceDispatcher* dispatcher_;
};

#endif /* __QD_INTERFACE_DISPATCHER_H__ */
