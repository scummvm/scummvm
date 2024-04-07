#ifndef __QD_INTERFACE_ELEMENT_H__
#define __QD_INTERFACE_ELEMENT_H__

#include "snd_sound.h"
#include "mouse_input.h"

#include "gr_screen_region.h"

#include "qd_animation.h"
#include "qd_interface_object_base.h"
#include "qd_sound_handle.h"

//class qdInterfaceScreen;
class qdInterfaceElementState;

//! Базовый класс для элементов GUI.
class qdInterfaceElement : public qdInterfaceObjectBase
{
public:
	qdInterfaceElement();
	qdInterfaceElement(const qdInterfaceElement& el);
	virtual ~qdInterfaceElement() = 0;

	qdInterfaceElement& operator = (const qdInterfaceElement& el);

	//! Идентификаторы настроек игры.
	enum option_ID_t {
		OPTION_NONE = 0,
		/// звук вкл/выкл, 1/0
		OPTION_SOUND,
		/// громкость звука, [0, 255]
		OPTION_SOUND_VOLUME,
		/// музыка вкл/выкл, 1/0
		OPTION_MUSIC,
		/// громкость музыки, [0, 255]
		OPTION_MUSIC_VOLUME,
		/// определенный персонаж персонаж активен/неактивен, 1/0
		OPTION_ACTIVE_PERSONAGE
	};
	
	//! Типы элементов интерфейса.
	enum element_type {
		//! фон
		EL_BACKGROUND,
		//! кнопка
		EL_BUTTON,
		//! слайдер
		EL_SLIDER,
		//! сэйв
		EL_SAVE,
		//! окно с текстами
		EL_TEXT_WINDOW,
		//! счётчик
		EL_COUNTER
	};

	//! Статус состояния.
	enum state_status_t {
		//! состояние неактивно
		STATE_INACTIVE,
		//! состояние активно
		STATE_ACTIVE,
		//! работа состояния закончилась
		STATE_DONE
	};

	//! Создание элемента интерфейса заданного типа.
	static qdInterfaceElement* create_element(element_type tp);
	//! Уничтожение элемента интерфейса.
	static void destroy_element(qdInterfaceElement* p);
	//! Возвращает тип элемента.
	virtual element_type get_element_type() const = 0;

	//! Возвращает идентификатор настройки игры, связанной с элементом.
	option_ID_t option_ID() const { return option_ID_; }
	//! Устанавливает идентификатор настройки игры, связанной с элементом.
	void set_option_ID(option_ID_t id){ option_ID_ = id; }
	//! Возвращает true, если с элементом связана настройка игры.
	bool linked_to_option() const { return (option_ID_ != OPTION_NONE); }
	//! Возвращает значение настройки игры, связанной с элементом.
	virtual int option_value() const { return -1; }
	//! Устанавливает значение настройки игры, связанной с элементом.
	virtual bool set_option_value(int value){ return false; }

	//! Возвращает экранные координаты центра элемента.
	virtual const Vect2i r() const { return r_ + screen_offset_; }
	//! Устанавливает экранные координаты центра элемента.
	void set_r(const Vect2i& v){ r_ = v; }

	//! Возвращает размер элемента по горизонтали в пикселах.
	virtual int size_x() const { return animation_.size_x(); }
	//! Возвращает размер элемента по вертикали в пикселах.
	virtual int size_y() const { return animation_.size_y(); }

	virtual grScreenRegion screen_region() const;
	const grScreenRegion& last_screen_region() const { return last_screen_region_; }

	//! Устанавливает экранную глубину элемента.
	void set_screen_depth(int v){ screen_depth_ = v; }
	//! Возвращает экранную глубину эелмента.
	int screen_depth() const { return screen_depth_; }

	//! Запись данных в скрипт.
	bool save_script(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);

	//! Отрисовка элемента.
	virtual bool redraw() const;
	virtual bool need_redraw() const;
	virtual bool post_redraw();

	//! Обработчик событий мыши.
	virtual bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev) = 0;
	//! Обработчик ввода с клавиатуры.
	virtual bool keyboard_handler(int vkey) = 0;
	//! Обработчик ввода символов с клавиатуры.
	virtual bool char_input_handler(int vkey){ return false; }

	virtual void hover_clear(){ }

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	virtual bool init(bool is_game_active = true) = 0;

	//! Обсчет логики, параметр - время в секундах.
	virtual bool quant(float dt);

	const qdAnimation& animation() const { return animation_; }

	//! Устанавливает анимацию для элемента.
	bool set_animation(const qdAnimation* anm,int anm_flags = 0);
	//! Включает состояние элемента.
	bool set_state(const qdInterfaceElementState* p);

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource* add_resource(const char* file_name,const qdInterfaceElementState* res_owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const char* file_name,const qdInterfaceElementState* res_owner);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	virtual bool hit_test(int x,int y) const;

	//! Возвращает статус состояния.
	state_status_t state_status(const qdInterfaceElementState* p) const;

	//! Прячет элемент.
	void hide(){ is_visible_ = false; }
	//! Показывает элемент.
	void show(){ is_visible_ = true; }
	//! Возвращает true, если элемент не спрятан.
	bool is_visible() const { return is_visible_; }

	//! Возвращает true, если элемент заблокирован.
	bool is_locked() const { return is_locked_; }
	//! Блокировка/разблокировка элемента.
	void set_lock(bool state){ is_locked_ = state; }

	static const Vect2i& screen_offset(){ return screen_offset_; }
	static void set_screen_offset(const Vect2i& offset){ screen_offset_ = offset; }

protected:

	//! Запись данных в скрипт.
	virtual bool save_script_body(XStream& fh,int indent = 0) const = 0;
	//! Загрузка данных из скрипта.
	virtual bool load_script_body(const xml::tag* p) = 0;

	void clear_screen_region(){ last_screen_region_ = grScreenRegion::EMPTY; }

private:

	//! Идентификатор настройки игры, связанной с элементом.
	option_ID_t option_ID_;
	//! Данные для настройки игры, связанной с элементом.
	std::string option_data_;

	//! Экранные координаты центра элемента.
	Vect2i r_;

	//! Экранная глубина. Чем больше значение - тем дальше элемент.
	int screen_depth_;

	//! Анимация элемента.
	qdAnimation animation_;

	//! Хэндл для управления звуками.
	qdSoundHandle sound_handle_;

	//! Равно false, если элемент спрятан.
	bool is_visible_;

	//! Равно true, если элемент заблокирован (не реагирует на клавиатуру/мышь).
	bool is_locked_;

	grScreenRegion last_screen_region_;
	const qdAnimationFrame* last_animation_frame_;

	static Vect2i screen_offset_;
};

#endif /* __QD_INTERFACE_ELEMENT_H__ */
