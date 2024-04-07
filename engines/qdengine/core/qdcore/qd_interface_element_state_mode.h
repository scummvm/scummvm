
#ifndef __QD_INTERFACE_ELEMENT_STATE_MODE_H__
#define __QD_INTERFACE_ELEMENT_STATE_MODE_H__

#include "qd_contour.h"

//! Событие, привязанное к состоянию интерфейсного элемента.
/**
Включает анимацию и звук при выполнении некоторых условий (например при наведении
мыши на элемент интерфейса.
*/
class qdInterfaceElementStateMode
{
public:
	qdInterfaceElementStateMode();
	qdInterfaceElementStateMode(const qdInterfaceElementStateMode& ev);
	~qdInterfaceElementStateMode();

	qdInterfaceElementStateMode& operator = (const qdInterfaceElementStateMode& ev);

	//! Устанавливает имя файла звука.
	/**
	Если надо убрать звук - передать NULL в качестве имени файла.
	*/
	void set_sound_file(const char* name);
	//! Возвращает имя файла звука.
	const char* sound_file() const { return sound_file_.c_str(); }
	//! Устанавливает звук события.
	void set_sound(const qdSound* p){ sound_ = p; }
	//! Возвращает звук события.
	const qdSound* sound() const { return sound_; }
	//! Возвращает true, если к событию привязан звук.
	bool has_sound() const { return !sound_file_.empty(); }

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_animation_file(const char* name);
	//! Возвращает имя файла для анимации.
	const char* animation_file() const { return animation_file_.c_str(); }
	//! Возвращает флаги анимации.
	int animation_flags() const { return animation_flags_; }

	//! Устанавливает флаг анимации.
	void set_animation_flag(int fl){ animation_flags_ |= fl; }
	//! Скидывает флаг анимации.
	void drop_animation_flag(int fl){ animation_flags_ &= ~fl; }
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_animation_flag(int fl) const { if(animation_flags_ & fl) return true; return false; }

	//! Устанавливает указатель на анимацию события.
	void set_animation(const qdAnimation* p){ animation_ = p; }
	//! Возвращает указатель на анимацию события.
	const qdAnimation* animation() const { return animation_; }
	//! Возвращает true, если к состоянию привязана анимация.
	bool has_animation() const { return !animation_file_.empty(); }
	//! Возвращает true, если у состояния задан контур.
	bool has_contour() const { return !contour_.is_contour_empty(); }
	//! Возвращает true, если точка с экранными координатами (x,у) попадает внутрь контура.
	bool hit_test(int x,int y) const { return contour_.is_inside(Vect2s(x,y)); }

	//! Записывает контур в cnt.
	bool get_contour(qdContour& cnt) const { cnt = contour_; return true; }
	//! Устанавливает контур.
	bool set_contour(const qdContour& cnt){ contour_ = cnt; return true; }

	//! Запись данных в скрипт.
	/**
	type_id - тип события
	*/
	bool save_script(XStream& fh,int type_id = 0,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);

private:

	//! Имя файла звука.
	std::string sound_file_;

	//! Звук, включаемый событием.
	const qdSound* sound_;

	//! Имя файла для анимации
	std::string animation_file_;
	//! Флаги для анимации.
	int animation_flags_;

	//! Анимация, включаемая событием.
	const qdAnimation* animation_;

	//! Контур.
	qdContour contour_;
};

#endif /* __QD_INTERFACE_ELEMENT_STATE_MODE_H__ */
