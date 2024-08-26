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

#ifndef QDENGINE_MINIGAMES_KARTINY_H
#define QDENGINE_MINIGAMES_KARTINY_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdKartinyMiniGameInterface : public qdMiniGameInterface {
public:
	qdKartinyMiniGameInterface() {}
	~qdKartinyMiniGameInterface() {}

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "Kartiny::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return 0;

		_bg_clickObj = _scene->object_interface(_scene->minigame_parameter("zad_click_flag_name"));
		_bg1_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad1_l2_object_name"));
		_bg2_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad2_l2_object_name"));
		_bg3_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad3_l2_object_name"));
		_bg4_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad4_l2_object_name"));
		_doneObj     = _scene->object_interface(_scene->minigame_parameter("done"));
		_startObj    = _scene->object_interface(_scene->minigame_parameter("start"));
		_activateObj = _scene->object_interface(_scene->minigame_parameter("activate"));
		_zFlagObj    = _scene->object_interface(_scene->minigame_parameter("z_flag"));
		_startObj2   = _scene->object_interface(_scene->minigame_parameter("start"));

		_wasInited = 0;
		_artDepth = -1.0;
		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		debugC(3, kDebugMinigames, "Kartiny::quant(%f)", dt);

		return true;
	}

	//! Деинициализация игры.
	bool finit() {
		debugC(1, kDebugMinigames, "Kartiny::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	/// Версия интерфейса игры, трогать не надо.
	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_bg_clickObj = nullptr;
	qdMinigameObjectInterface *_bg1_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg2_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg3_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg4_l2Obj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;
	qdMinigameObjectInterface *_startObj = nullptr;
	qdMinigameObjectInterface *_activateObj = nullptr;
	qdMinigameObjectInterface *_zFlagObj = nullptr;
	qdMinigameObjectInterface *_startObj2 = nullptr;

	bool _wasInited = false;
	float _artDepth = -1.0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_KARTINY_H
