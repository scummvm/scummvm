#ifndef __QD_ENGINE_INTERFACE_H__
#define __QD_ENGINE_INTERFACE_H__

#include "qd_rnd.h"

class qdGameScene;

namespace qdmg {

/// Интерфейс к движку для миниигр, реализация.
class qdEngineInterfaceImpl : public qdEngineInterface
{
public:
	static const qdEngineInterfaceImpl& instance();

	qdMinigameSceneInterface* current_scene_interface() const;
	qdMinigameSceneInterface* scene_interface(qdGameScene* scene) const;
	void release_scene_interface(qdMinigameSceneInterface* p) const;

	qdMinigameCounterInterface* counter_interface(const char* counter_name) const;
	void release_counter_interface(qdMinigameCounterInterface* p) const;
	
	//! Возвращает размер экрана в пикселах.
	mgVect2i screen_size() const;

	//! Возвращает true, если на клавиатуре нажата кнопка vkey.
	bool is_key_pressed(int vkey) const;

	//! Возвращает true, если в данный момент произошло событие event_id.
	bool is_mouse_event_active(qdMinigameMouseEvent event_id) const;

	//! Возвращает текущие координаты мышиного курсора.
	mgVect2i mouse_cursor_position() const;

	/// Добавляет набранные очки в таблицу рекордов.
	/// Если очков оказалось достаточно, возвращает true.
	bool add_hall_of_fame_entry(int score) const;

	bool set_interface_text(const char* screen_name, const char* control_name, const char* text) const;

	/// Инициализация rnd
	void rnd_init(int seed) const { qd_rnd_init(seed); }
	/// Возвращает случайное значение в интервале [0, m-1].
	unsigned rnd(unsigned m) const { return qd_rnd(m); }
	/// Возвращает случайное значение в интервале [-x, x].
	float frnd(float x) const { return qd_frnd(x); }
	/// Возвращает случайное значение в интервале [0, x].
	float fabs_rnd(float x) const { return qd_fabs_rnd(x); }
};

}; // namespace qdmg

#endif /* __QD_ENGINE_INTERFACE_H__ */

