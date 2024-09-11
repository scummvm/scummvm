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

#ifndef QDENGINE_MINIGAMES_SHVEIK_SHKATULKA_H
#define QDENGINE_MINIGAMES_SHVEIK_SHKATULKA_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdShveikShkatulkaMiniGame : public qdMiniGameInterface {
public:
	qdShveikShkatulkaMiniGame() {}
	~qdShveikShkatulkaMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "ShveikShkatulka::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_stones[0] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31"); // "камень1"
		_stones[1] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x32"); // "камень2"
		_stones[2] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x33"); // "камень3"
		_stones[3] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x34"); // "камень4"
		_stones[4] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x35"); // "камень5"
		_stones[5] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x36"); // "камень6"
		_stones[6] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x37"); // "камень7"
		_stones[7] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x38"); // "камень8"
		_stones[8] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x39"); // "камень9"
		_stones[9] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31\x30"); // "камень10"
		_stones[10] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31\x31"); // "камень11"
		_stones[11] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31\x32"); // "камень12"
		_stones[12] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31\x33"); // "камень13"
		_stones[13] = _scene->object_interface("\xea\xe0\xec\xe5\xed\xfc\x31\x34"); // "камень14"

		_cursorObj = _scene->object_interface("\xea\xf3\xf0\xf1\xee\xf0"); // "курсор"
		_doneObj = _scene->object_interface("$done");
		_startObj = _scene->object_interface("\x24\xe7\xe0\xef\xf3\xf1\xea"); // "$запуск"
		_jumpSoundObj = _scene->object_interface("\xe7\xe2\xf3\xea\x20\xef\xe5\xf0\xe5\xf1\xea\xee\xea\xe0"); // "звук перескока"

		_someVar1 = 0;
		_cursorTakenFlag = 0;
		_someVar3 = 0;
		_someVar4 = 0;

		if (_startObj->is_state_active("\xed\xe5\x20\xe1\xfb\xeb")) { // "не был"
			resetStones();
			_startObj->set_state("\xe1\xfb\xeb"); // "был"
		}

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ShveikShkatulka::quant(%f)", dt);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "ShveikShkatulka::finit()");

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

	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	void resetStones() {
		_stones[0]->set_state("15");
		_stones[1]->set_state("16");
		_stones[2]->set_state("17");
		_stones[3]->set_state("110");
		_stones[4]->set_state("11");
		_stones[5]->set_state("12");
		_stones[6]->set_state("14");
		_stones[7]->set_state("26");
		_stones[8]->set_state("27");
		_stones[9]->set_state("210");
		_stones[10]->set_state("211");
		_stones[11]->set_state("21");
		_stones[12]->set_state("22");
		_stones[13]->set_state("24");
  	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	int _someVar1 = 0;
	bool _cursorTakenFlag = false;
	int _someVar3 = 0;
	int _someVar4 = 0;

	qdMinigameObjectInterface *_stones[14];
	qdMinigameObjectInterface *_cursorObj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;
	qdMinigameObjectInterface *_startObj = nullptr;
	qdMinigameObjectInterface *_jumpSoundObj = nullptr;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_SHVEIK_SHKATULKA_H
