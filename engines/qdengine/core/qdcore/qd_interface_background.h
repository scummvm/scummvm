
#ifndef __QD_INTERFACE_BACKGROUND_H__
#define __QD_INTERFACE_BACKGROUND_H__

#include "qd_interface_element.h"
#include "qd_interface_element_state.h"

//! Интерфейсный элемент - фон.
class qdInterfaceBackground : public qdInterfaceElement
{
public:
	qdInterfaceBackground();
	qdInterfaceBackground(const qdInterfaceBackground& bk);
	~qdInterfaceBackground();

	qdInterfaceBackground& operator = (const qdInterfaceBackground& bk);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const { return qdInterfaceElement::EL_BACKGROUND; }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_animation_file(const char* name){ state_.set_animation_file(name); }
	//! Возвращает имя файла для анимации.
	const char* animation_file() const { return state_.animation_file(); }
	//! Возвращает флаги анимации.
	int animation_flags() const { return state_.animation_flags(); }

	//! Устанавливает флаг анимации.
	void set_animation_flag(int fl){ state_.set_animation_flag(fl); }
	//! Скидывает флаг анимации.
	void drop_animation_flag(int fl){ state_.drop_animation_flag(fl); }
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_animation_flag(int fl) const { return state_.check_animation_flag(fl); }

	//! Возвращает true, если к фону привязана анимация.
	bool has_animation() const { return state_.has_animation(); }

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);

private:

	//! Состояние, в котором хранятся все необходимые настройки.
	qdInterfaceElementState state_;
};

#endif /* __QD_INTERFACE_BACKGROUND_H__ */
