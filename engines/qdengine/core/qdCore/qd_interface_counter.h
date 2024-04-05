#ifndef __QD_INTERFACE_COUNTER__
#define __QD_INTERFACE_COUNTER__

#include "qd_named_object_reference.h"
#include "qd_screen_text.h"

#include "qd_interface_element.h"

//! Интерфейсный элемент для отображения значения счётчика.

class qdInterfaceCounter : public qdInterfaceElement
{
public:
	qdInterfaceCounter();
	qdInterfaceCounter(const qdInterfaceCounter& counter);
	~qdInterfaceCounter();

	qdInterfaceCounter& operator = (const qdInterfaceCounter& counter);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const { return qdInterfaceElement::EL_COUNTER; }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);

	grScreenRegion screen_region() const;

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	bool redraw() const;
	bool need_redraw() const;
	bool post_redraw();

	//! Возвращает размер элемента по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер элемента по вертикали в пикселах.
	int size_y() const;

	/// количество отображаемых разрядов
	int digits() const { return digits_; }
	void setDigits(int digits) { digits_ = digits; }

	const qdScreenTextFormat& textFormat() const { return textFormat_; }
	void setTextFormat(const qdScreenTextFormat& text_format){ textFormat_ = text_format; }

	const qdCounter* counter() const { return counter_; }
	void setCounter(const qdCounter* counter);

	const char* counterName() const;
	void setCounterName(const char* name);

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);

	const char* data() const;

private:

	qdScreenTextFormat textFormat_;

	int digits_;

	std::string counterName_;
	const qdCounter* counter_;

	int lastValue_;
};

#endif /* __QD_INTERFACE_COUNTER__ */
