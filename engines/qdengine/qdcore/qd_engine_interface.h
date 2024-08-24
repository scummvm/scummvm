/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QDENGINE_QDCORE_QD_ENGINE_INTERFACE_H
#define QDENGINE_QDCORE_QD_ENGINE_INTERFACE_H

#include "qdengine/qdcore/qd_rnd.h"


namespace QDEngine {

class qdGameScene;

namespace qdmg {

/// Интерфейс к движку для миниигр, реализация.
class qdEngineInterfaceImpl : public qdEngineInterface {
public:
	static const qdEngineInterfaceImpl &instance();

	qdMinigameSceneInterface *current_scene_interface() const;
	qdMinigameSceneInterface *scene_interface(qdGameScene *scene) const;
	void release_scene_interface(qdMinigameSceneInterface *p) const;

	qdMinigameCounterInterface *counter_interface(const char *counter_name) const;
	void release_counter_interface(qdMinigameCounterInterface *p) const;

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

	bool set_interface_text(const char *screen_name, const char *control_name, const char *text) const;

	/// Инициализация rnd
	void rnd_init(int seed) const {
		qd_rnd_init(seed);
	}
	/// Возвращает случайное значение в интервале [0, m-1].
	uint32 rnd(uint32 m) const {
		return qd_rnd(m);
	}
	/// Возвращает случайное значение в интервале [-x, x].
	float frnd(float x) const {
		return qd_frnd(x);
	}
	/// Возвращает случайное значение в интервале [0, x].
	float fabs_rnd(float x) const {
		return qd_fabs_rnd(x);
	}
};

} // namespace qdmg

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_ENGINE_INTERFACE_H
