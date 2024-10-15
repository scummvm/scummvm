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

#ifndef QDENGINE_MINIGAMES_3MICE2_RASKR1_H
#define QDENGINE_MINIGAMES_3MICE2_RASKR1_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2Raskr1MiniGame : public qdMiniGameInterface {
public:
	qd3mice2Raskr1MiniGame() {}
	~qd3mice2Raskr1MiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2Raskr1::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		for (int i = 1; i <= 311; i++)
			_zones[i] = _scene->object_interface(Common::String::format("zone@%i#", i).c_str());

		_objColor = _scene->object_interface("$color");
		_objColorSel = _scene->object_interface("$color_sel");
		_objLoaded = _scene->object_interface("$loaded");
		_objDone = _scene->object_interface("$done");

		if (_objLoaded->is_state_active("no")) {
			for (int i = 1; i <= 311; i++)
				_zones[i]->set_shadow(0xFEFEFF, 0);

			_objColorSel->set_state("!\xEC\xE0\xF1\xEA\xE0"); // "!маска"
			_objColor->set_shadow(0xFEFEFE, 0);
			_objLoaded->set_state("yes");
		}

		_timePassed = 0;

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2Raskr1::quant(%f)", dt);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2Raskr1::finit()");

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
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_zones[312] = { nullptr };
	qdMinigameObjectInterface *_objColor = nullptr;
	qdMinigameObjectInterface *_objColorSel = nullptr;
	qdMinigameObjectInterface *_objLoaded = nullptr;
	qdMinigameObjectInterface *_objDone = nullptr;

	float _timePassed = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_RASKR1_H
