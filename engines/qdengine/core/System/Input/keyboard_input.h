#ifndef __KEYBOARD_INPUT_H__
#define __KEYBOARD_INPUT_H__

//! Обработчик ввода с клавиатуры.
class keyboardDispatcher
{
public:
	//! Обработчик нажатий/отжатий кнопок.
	/**
	Возвращает true при успешной обработке события.
	*/
	typedef bool (*event_handler_t)(int key_vcode, bool event);

	keyboardDispatcher();
	~keyboardDispatcher();

	//! Возвращает true, если кнопка с кодом vkey в данный момент нажата.
	bool is_pressed(int vkey) const { 
		assert(vkey >= 0 && vkey < 256);
		return key_states_[vkey];
	}

	//! Устанавливает обработчик нажатий/отжатий кнопок.
	event_handler_t set_handler(event_handler_t h){
		event_handler_t old_h = handler_;
		handler_ = h;
		return old_h;
	}

	//! Обрабатывает нажатие (event == true) или отжатие (event == false) кнопки с кодом vkey.
	bool handle_event(int vkey, bool event){
		key_states_[vkey] = event;
		if(handler_) return (*handler_)(vkey,event);
		return false;
	}

	//! Возвращает диспетчер по-умолчанию.
	static keyboardDispatcher* instance();

private:

	//! Обработчик нажатий/отжатий кнопок.
	event_handler_t handler_;

	//! Состояния кнопок - key_states_[vkey] == true если кнопка с кодом vkey нажата.
	bool key_states_[256];
};

#endif /* __KEYBOARD_INPUT_H__ */

