
#ifndef __QD_INTERFACE_SCREEN_H__
#define __QD_INTERFACE_SCREEN_H__

#include "mouse_input.h"
#include "xml_fwd.h"

#include "gr_screen_region.h"

#include "qd_interface_object_base.h"
#include "qd_object_list_container.h"
#include "qd_resource_dispatcher.h"
#include "qd_music_track.h"

class qdInterfaceElement;
class qdInterfaceElementState;

//! Интерфейсный экран.
class qdInterfaceScreen : public qdInterfaceObjectBase
{
public:
	qdInterfaceScreen();
	~qdInterfaceScreen();

	//! Запись данных в скрипт.
	bool save_script(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);

	//! Отрисовка экрана.
	bool redraw(int dx = 0,int dy = 0) const;

	bool pre_redraw(bool force_full_redraw = false);
	bool post_redraw();

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Добавление элемента.
	bool add_element(qdInterfaceElement* p);
	//! Изменение имени элемента.
	bool rename_element(qdInterfaceElement* p,const char* name);
	//! Удаление элемента из списка.
	bool remove_element(qdInterfaceElement* p);
	//! Поиск элемента по имени.
	qdInterfaceElement* get_element(const char* el_name);
	//! Возвращает true, если элемент есть в списке.
	bool is_element_in_list(const qdInterfaceElement* el) const;

	typedef std::list<qdInterfaceElement*> element_list_t;
	//! Возвращает список элементов экрана.
	const element_list_t& element_list() const { return elements_.get_list(); }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);
	bool char_input_handler(int vkey);

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource* add_resource(const char* file_name,const qdInterfaceElementState* res_owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const char* file_name,const qdInterfaceElementState* res_owner);
	//! Возвращает true, если на ресурс есть ссылки.
	bool has_references(const qdResource* p) const { return resources_.is_registered(p); }

	//! Прячет элемент.
	bool hide_element(const char* element_name,bool temporary_hide = true);
	//! Прячет элемент.
	bool hide_element(qdInterfaceElement* p,bool temporary_hide = true);
	//! Показывает элемент.
	bool show_element(const char* element_name);
	//! Показывает элемент.
	bool show_element(qdInterfaceElement* p);

	//! Инициализация экрана при заходе на него.
	bool init(bool is_game_active = true);

	//! Строит сортированный список видимых элементов.
	bool build_visible_elements_list();

	//! Устанавливает, что ресурсы экрана не надо выгружать при выходе с него.
	void lock_resources(){ is_locked_ = true; }
	//! Устанавливает, что ресурсы экрана надо выгружать при выходе с него.
	void unlock_resources(){ is_locked_ = false; }
	//! Возвращает true, если ресурсы экрана не надо выгружать при выходе с него.
	bool is_locked() const { return is_locked_; }

	//! Включает кнопки, связанные с персонажем p, выключает кнопки остальных персонажей.
	void activate_personage_buttons(const qdNamedObject* p);

	void update_personage_buttons();

	float autohide_time() const { return autohide_time_; }
	void set_autohide_time(float time){ autohide_time_ = time; }

	const Vect2i& autohide_offset() const { return autohide_offset_; }
	void set_autohide_offset(const Vect2i& offset){ autohide_offset_ = offset; }

	//! Фоновая музыка.
	const qdMusicTrack& music_track() const { return music_track_; }
	//! Устанавливает фоновую музыку.
	void set_music_track(const qdMusicTrack& track){ music_track_ = track; }
	//! Возвращает true, если экрану задана фоновая музыка.
	bool has_music_track() const { return music_track_.has_file_name(); }

#ifdef _QUEST_EDITOR
	bool select_element(qdInterfaceElement* pele, bool bselect);
	bool is_selected(const qdInterfaceElement* p) const;
	const element_list_t& selected_element_list() const{ return selected_elements_;}
	void clear_selection();
#endif //_QUEST_EDITOR

	void set_autohide_phase(float ph){ autohide_phase_ = ph; }

	// Указатель на объект, последним вызвавший данный как модальный экран
	qdInterfaceObjectBase* modal_caller() { return modal_caller_; }
	void set_modal_caller(qdInterfaceObjectBase* caller) { modal_caller_ = caller; }

private:

#ifdef _QUEST_EDITOR
	//! выбранные элементы
	element_list_t selected_elements_;
#endif // _QUEST_EDITOR

	//! Список интерфейсных элементов экрана.
	qdObjectListContainer<qdInterfaceElement> elements_;

	//! Ресурсы, на которые ссылается экран.
	qdResourceDispatcher<qdInterfaceElementState> resources_;

	typedef std::vector<qdInterfaceElement*> sorted_element_list_t;
	//! Отсортированный по глубине список элементов экрана.
	sorted_element_list_t sorted_elements_;

	//! Сортирует элементы по глубине.
	bool sort_elements();

	//! true, если ресурсы экрана не надо выгружать при выходе с него.
	bool is_locked_;

	//! Время всплывания экрана в секундах.
	float autohide_time_;
	//! Смещение экрана, когда он минимизирован.
	Vect2i autohide_offset_;

	//! Текущая фаза всплывания экрана.
	float autohide_phase_;

	//! Фоновая музыка.
	qdMusicTrack music_track_;

	//! Указатель на объект, который последним вызвал данный экран в качестве модального 
	qdInterfaceObjectBase* modal_caller_;
};

#endif /* __QD_INTERFACE_SCREEN_H__ */
