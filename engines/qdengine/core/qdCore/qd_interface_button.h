
#ifndef __QD_INTERFACE_BUTTON_H__
#define __QD_INTERFACE_BUTTON_H__

#include "qd_interface_element.h"
#include "qd_interface_element_state.h"

#ifdef _QUEST_EDITOR
#define _QD_INTERFACE_BUTTON_PTR_CONTAINER
#endif

//! Интерфейсный элемент - кнопка.
class qdInterfaceButton : public qdInterfaceElement
{
public:
	qdInterfaceButton();
	qdInterfaceButton(const qdInterfaceButton& bt);
	~qdInterfaceButton();

	qdInterfaceButton& operator = (const qdInterfaceButton& bt);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const { return qdInterfaceElement::EL_BUTTON; }

	//! Возвращает количество состояний кнопки.
	int num_states() const { return states_.size(); }

	//! Включает состояние номер state_num.
	bool activate_state(int state_num);
	//! Включает состояние с именем state_name.
	bool activate_state(const char* state_name);

	int option_value() const { return cur_state_; }
	bool set_option_value(int value);

	//! Возвращает указатель на состояние кнопки.
	const qdInterfaceElementState* get_state(int state_num) const {
		assert(state_num >= 0 && state_num < states_.size());
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		return &states_[state_num];
#else
		return &*states_[state_num];
#endif
	}
	//! Возвращает указатель на состояние кнопки.
	qdInterfaceElementState* get_state(int state_num){
		assert(state_num >= 0 && state_num < states_.size());
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		return &states_[state_num];
#else
		return &*states_[state_num];
#endif
	}

	//! Добавляет состояние кнопки.
	/**
	Параметры из состояния st копируются во внутренние данные.
	*/
	bool add_state(const qdInterfaceElementState& st);

	//! Добавляет состояние кнопки.
	/**
	Новое состояние добавляется в список перед состоянием номер insert_before.
	Параметры из состояния st копируются во внутренние данные.
	*/
	bool insert_state(int insert_before,const qdInterfaceElementState& st);

	//! Удаляет состояние номер state_num.
	bool erase_state(int state_num);

	//! Модифицирует состояние кнопки.
	/** 
	Соответствующему состоянию присваивается значение st.
	*/
	bool modify_state(int state_num,const qdInterfaceElementState& st){
		assert(state_num >= 0 && state_num < states_.size());
#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		*states_[state_num] = st;
		states_[state_num]->set_owner(this);
#else
		states_[state_num] = st;
		states_[state_num].set_owner(this);
#endif
		return true;
	}

	//! Переключает состояние кнопки.
	/**
	Если direction == true включается следующее состояние, если false - предыдущее.
	*/
	bool change_state(bool direction = true);

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x,int y) const;

	const qdInterfaceEvent* find_event(qdInterfaceEvent::event_t type) const;
	bool has_event(qdInterfaceEvent::event_t type,const char* ev_data = NULL) const;

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);

private:

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	typedef std::vector<qdInterfaceElementState*> state_container_t;
#else
	typedef std::vector<qdInterfaceElementState> state_container_t;
#endif

	//! Состояния кнопки.
	state_container_t states_;

	//! Текущее состояние кнопки.
	int cur_state_;

};

#endif /* __QD_INTERFACE_BUTTON_H__ */
