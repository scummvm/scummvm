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

#ifndef QDENGINE_MINIGAMES_TETRIS_H
#define QDENGINE_MINIGAMES_TETRIS_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdTetrisMiniGame : public qdMiniGameInterface {
public:
	qdTetrisMiniGame() {
		_flyingObjs = _hiddenObjs = nullptr;
	}
	~qdTetrisMiniGame() {
		free(_flyingObjs);
		free(_hiddenObjs);
	}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "Tetris::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		if (!_flyingObjs) {
			_flyingObjs = (qdMinigameObjectInterface **)malloc(5 * sizeof(qdMinigameObjectInterface *));

			if (!_flyingObjs)
				return false;

			for (int i = 0; i < 5; i++) {
				Common::String name = Common::String::format("\xeb\xe5\xf2\xe8\xf2%d", i + 1); // "летит%i"
				_flyingObjs[i] = _scene->object_interface(_scene->minigame_parameter(name.c_str()));
			}
		}

		if (!_hiddenObjs) {
			_hiddenObjs = (qdMinigameObjectInterface **)malloc(5 * sizeof(qdMinigameObjectInterface *));

			if (!_hiddenObjs)
				return false;

			for (int i = 0; i < 5; i++) {
				Common::String name = Common::String::format("\xf1\xea\xf0\xfb\xf2%d", i + 1); // "скрыт%i"
				_hiddenObjs[i] = _scene->object_interface(_scene->minigame_parameter(name.c_str()));
			}
		}

		_lastRowObj = _scene->object_interface(_scene->minigame_parameter("last_row"));

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "Tetris::quant(%f)", dt);

		for (int i = 0; i < 5; i++ ) {
			if (_flyingObjs[i]->is_state_active("\xeb\xe5\xf2\xe8\xf2")		// "летит"
					&& _hiddenObjs[i]->is_state_active("\xed\xe5\xf2")) {	// "нет"

				mgVect2i pos = _flyingObjs[i]->screen_R();

				if (pos.x <= 251) {
					int activeColumn = getActiveColumn();
					if (activeColumn < 12)
						activeColumn = 12;
					if (pos.y > 535 - 10 * activeColumn)
						_hiddenObjs[i]->set_state("\xe4\xe0");	// "да"
				} else {
					if (pos.y > 535 - 10 * getActiveColumn())
						_hiddenObjs[i]->set_state("\xe4\xe0");	// "да"
				}
			}
		}

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "Tetris::finit()");

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
	int getActiveColumn() {
		if (_lastRowObj->is_state_active("0"))
			return 0;
		if (_lastRowObj->is_state_active("1"))
			return 1;
		if (_lastRowObj->is_state_active("2"))
			return 2;
		if (_lastRowObj->is_state_active("3"))
			return 3;
		if (_lastRowObj->is_state_active("4"))
			return 4;
		if (_lastRowObj->is_state_active("5"))
			return 5;
		if (_lastRowObj->is_state_active("6"))
			return 6;
		if (_lastRowObj->is_state_active("7"))
			return 7;
		if (_lastRowObj->is_state_active("8"))
			return 8;
		if (_lastRowObj->is_state_active("9"))
			return 9;
		if (_lastRowObj->is_state_active("10"))
			return 10;
		if (_lastRowObj->is_state_active("11"))
			return 11;
		if (_lastRowObj->is_state_active("12"))
			return 12;
		if (_lastRowObj->is_state_active("13"))
			return 13;
		if (_lastRowObj->is_state_active("14"))
			return 14;
		if (_lastRowObj->is_state_active("15"))
			return 15;
		if (_lastRowObj->is_state_active("16"))
			return 16;
		if (_lastRowObj->is_state_active("17"))
			return 17;
		if (_lastRowObj->is_state_active("18"))
			return 18;
		if (_lastRowObj->is_state_active("19"))
			return 19;
		if (_lastRowObj->is_state_active("20"))
			return 20;
		if (_lastRowObj->is_state_active("21"))
			return 21;
		if (_lastRowObj->is_state_active("22"))
			return 22;
		if (_lastRowObj->is_state_active("23"))
			return 23;
		if (_lastRowObj->is_state_active("24"))
			return 24;

		return 0;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface **_flyingObjs = nullptr;
	qdMinigameObjectInterface **_hiddenObjs = nullptr;
	qdMinigameObjectInterface *_lastRowObj  = nullptr;

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_TETRIS_H
