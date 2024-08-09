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

#ifndef QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H
#define QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {


class qdEmptyMiniGameInterface : public qdMiniGameInterface {
public:
	//! Возвращает указатель на интерфейс к игре, параметр - имя конфигурационного файла.
	typedef qdEmptyMiniGameInterface *(*interface_open_proc)(const char *game_name);
	//! Очищает интерфейс к игре.
	typedef bool (*interface_close_proc)(qdEmptyMiniGameInterface *p);

	~qdEmptyMiniGameInterface() { };

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		warning("STUB: qdEmptyMinigameInterface: This is a placeholder class");
		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		warning("STUB: qdEmptyMiniGameInterface::quant()");
		return true;
	}

	//! Деинициализация игры.
	bool finit() {
		warning("STUB: qdEmptyMiniGameInterface::finit()");
		return true;
	}

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	bool new_game(const qdEngineInterface *engine_interface) {
		warning("STUB: qdEmptyMiniGameInterface::new_game()");
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		warning("STUB: qdEmptyMiniGameInterface::save_game()");
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		warning("STUB: qdEmptyMiniGameInterface::load_game()");
		return 0;
	}

	/// Версия интерфейса игры, трогать не надо.
	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H
