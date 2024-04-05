
#ifndef __QD_INTERFACE_SAVE_H__
#define __QD_INTERFACE_SAVE_H__

#include "xml_fwd.h"

#include "qd_interface_element.h"
#include "qd_interface_element_state.h"

//! Интерфейсный элемент - сэйв.
class qdInterfaceSave : public qdInterfaceElement
{
public:
	qdInterfaceSave();
	qdInterfaceSave(const qdInterfaceSave& sv);
	~qdInterfaceSave();

	qdInterfaceSave& operator = (const qdInterfaceSave& sv);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const { return qdInterfaceElement::EL_SAVE; }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Отрисовка элемента.
	bool redraw() const;

	grScreenRegion screen_region() const;

	//! Возвращает размер элемента по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер элемента по вертикали в пикселах.
	int size_y() const;

	//! Возвращает размер картики-превью по горизонтали в пикселах.
	int qdInterfaceSave::thumbnail_size_x() const {	return thumbnail_size_x_; }
	//! Возвращает размер картики-превью по вертикали в пикселах.
	int qdInterfaceSave::thumbnail_size_y() const {	return thumbnail_size_y_; }

	//! Назначает размер картики-превью по горизонтали в пикселах.
	void qdInterfaceSave::set_thumbnail_size_x(int sx){ thumbnail_size_x_ = sx; }
	//! Назначает размер картики-превью по вертикали в пикселах.
	void qdInterfaceSave::set_thumbnail_size_y(int sy){ thumbnail_size_y_ = sy; }

	//! Смещение текста, выводимого в сэйве, относительно левого-верхнего угла
	int text_dx() const { return text_dx_; }
	int text_dy() const { return text_dy_; }
	void set_test_dx(int val) { text_dx_ = val; }
	void set_test_dy(int val) { text_dy_ = val; }

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x,int y) const;

	//! Возвращает номер сэйва.
	int save_ID() const { return save_ID_; }
	//! Устанавливает номер сэйва.
	void set_save_ID(int id){ save_ID_ = id; }

	//! Возвращает имя сэйв-файла.
	const char* save_file() const;
	//! Возвращает имя файла картинки-превью сэйва.
	const char* thumbnail_file() const;
	//! Возвращает имя файла описания сэйва.
	const char* description_file() const;

	//! Устанавливает режим работы - записывать (state == true) или загружать (state == false) сэйвы.
	static void set_save_mode(bool state){ save_mode_ = state; }

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_frame_animation_file(const char* name,qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE){ frame_.set_animation_file(name,mode); }
	//! Возвращает имя файла для анимации.
	const char* frame_animation_file(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const { return frame_.animation_file(mode); }
	//! Возвращает флаги анимации.
	int frame_animation_flags(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const { return frame_.animation_flags(mode); }

	//! Устанавливает флаг анимации.
	void set_frame_animation_flag(int fl,qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE){ frame_.set_animation_flag(fl,mode); }
	//! Скидывает флаг анимации.
	void drop_frame_animation_flag(int fl,qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE){ frame_.drop_animation_flag(fl,mode); }
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_frame_animation_flag(int fl,qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const { return frame_.check_animation_flag(fl,mode); }

	//! Возвращает true, если к сейву привязана анимация рамки.
	bool frame_has_animation(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const { return frame_.has_animation(mode); }

	void set_frame_sound_file(const char* name,qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE){ frame_.set_sound_file(name,mode);}
	const char* frame_sound_file(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const { return frame_.sound_file(mode);}

	bool isAutosaveSlot() const { return isAutosaveSlot_; }
	void setAutosaveSlot(bool state){ isAutosaveSlot_ = state; }

	void set_title(const char* title){ save_title_ = title; }
	const char* title() const { return save_title_.c_str(); }

	//! Осуществить сохранение текущего состояния игры в ячейку сэйва.
	bool perform_save();

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);

private:

	//! Горизонтальный размер картинки-превью сэйва.
	int thumbnail_size_x_;
	//! Вертикальный размер картинки-превью сэйва.
	int thumbnail_size_y_;

	//! Смещение текста, выводимого в сэйве, относительно левого-верхнего угла
	int text_dx_;
	int text_dy_;

	//! Номер сэйва, имя файла имеет вид saveNNN.dat, где NNN - save_ID_.
	int save_ID_;

	/// если true, то игрок сам не может записать игру в этот слот
	bool isAutosaveSlot_;

	//! Превью сэйва.
	qdInterfaceElementState thumbnail_;

	std::string save_title_;

	//! Опциональная рамка вокруг картинки-превью сэйва.
	qdInterfaceElementState frame_;

	//! Режим работы сэйвов - чтение/запись (== false/true).
	static bool save_mode_;
	
	//! Текущий номер сэйва.
	static int current_save_ID_;
};

#endif /* __QD_INTERFACE_SAVE_H__ */

