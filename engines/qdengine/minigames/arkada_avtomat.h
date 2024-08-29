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

#ifndef QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H
#define QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdArkadaAvtomatMiniGame : public qdMiniGameInterface {
public:
	qdArkadaAvtomatMiniGame() {}
	~qdArkadaAvtomatMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "ArkadaAvtomat::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_patronMouseObj  = _scene->object_interface(_scene->minigame_parameter("patron_mouse"));
		_patronTomatoObj = _scene->object_interface(_scene->minigame_parameter("patron_tomato"));
		_killObj         = _scene->object_interface(_scene->minigame_parameter("kill"));
		_fazaObj         = _scene->object_interface(_scene->minigame_parameter("faza"));
		_menObj          = _scene->object_interface(_scene->minigame_parameter("men"));
		_bloodObj        = _scene->object_interface(_scene->minigame_parameter("blood"));
		_shotsEggObj     = _scene->object_interface(_scene->minigame_parameter("shots_egg"));
		_shotsTomatoObj  = _scene->object_interface(_scene->minigame_parameter("shots_tomato"));
		_shotsBananObj   = _scene->object_interface(_scene->minigame_parameter("shots_banan"));
		_doneObj         = _scene->object_interface(_scene->minigame_parameter("done"));
		_livesObj        = _scene->object_interface(_scene->minigame_parameter("lives"));
		_bublObj         = _scene->object_interface(_scene->minigame_parameter("bubl"));
		_walkFlagObj     = _scene->object_interface(_scene->minigame_parameter("walk_flag"));
		_jumpFlagObj     = _scene->object_interface(_scene->minigame_parameter("jump_flag"));

		_flag1 = 0;
		_flag2 = 0;
		_field_64 = 2;
		_flag3 = 0;
		_field_54 = -1;

		// stand(time(0));

		_someSwitchBackup = -1;
		_shotsTomatoCounter = 9;
		_shotsBananaCounter = 10;
		_shoteEggCounter = 10;
		_livesCounter = 5;
		_someFlag5 = 0;
		_someSwitch = 2;
		_someFlag6 = 0;
		_jumpFlag = 0;

   		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ArkadaAvtomat::quant(%f)", dt);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "ArkadaAvtomat::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}

	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}

	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	enum { INTERFACE_VERSION = 99 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_patronMouseObj;
	qdMinigameObjectInterface *_patronTomatoObj;
	qdMinigameObjectInterface *_killObj;
	qdMinigameObjectInterface *_fazaObj;
	qdMinigameObjectInterface *_menObj;
	qdMinigameObjectInterface *_bloodObj;
	qdMinigameObjectInterface *_shotsEggObj;
	qdMinigameObjectInterface *_shotsTomatoObj;
	qdMinigameObjectInterface *_shotsBananObj;
	qdMinigameObjectInterface *_doneObj;
	qdMinigameObjectInterface *_livesObj;
	qdMinigameObjectInterface *_bublObj;
	qdMinigameObjectInterface *_walkFlagObj;
	qdMinigameObjectInterface *_jumpFlagObj;

	bool _flag1 = 0;
	bool _flag2 = 0;
	int _field_64 = 2;
	bool _flag3 = 0;
	int _field_54 = -1;

	int _someSwitch = 2;
	int _someSwitchBackup = -1;

	int _shotsTomatoCounter = 9;
	int _shotsBananaCounter = 10;
	int _shoteEggCounter = 10;
	int _livesCounter = 5;

	bool _someFlag5 = 0;
	bool _someFlag6 = 0;
	bool _jumpFlag = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H
