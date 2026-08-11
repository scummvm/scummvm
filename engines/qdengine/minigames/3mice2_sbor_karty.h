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

#ifndef QDENGINE_MINIGAMES_3MICE2_SBOR_KARTY_H
#define QDENGINE_MINIGAMES_3MICE2_SBOR_KARTY_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2SborKartyMiniGame : public qdMiniGameInterface {
public:
	qd3mice2SborKartyMiniGame() {}
	~qd3mice2SborKartyMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2SborKarty::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		for (int i = 1; i <= 10; i++)
			_objects[i] = _scene->object_interface(Common::String::format("object@%i#", i).c_str());

		for (int i = 1; i <= 10; i++)
			_objects[11 + i] = _scene->object_interface(Common::String::format("inv_object@%i#", i).c_str());

		_objDone = _scene->object_interface("$done");
		_objDrop = _scene->object_interface("$\xf1\xe1\xf0\xee\xf1\xe8\xf2\xfc\x20\xf1\x20\xec\xfb\xf8\xe8");	// "$сбросить с мыши"
		_timePassed = 0.0;

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2SborKarty::quant(%f)", dt);

		_timePassed += dt;

		qdMinigameObjectInterface *obj = _scene->mouse_click_object_interface();

		if (obj) {
			const char *name = obj->name();

			if (strstr(name, "object@") && obj->is_state_active("base") && !_scene->mouse_object_interface()) {
				obj->set_state("hide");

				int num = getObjNum(name);
				debugC(4, kDebugMinigames, "to_inv: num is: %d for name: '%s'", num, name);
				_objects[num + 11]->set_state("to_inv");
			}

			_scene->release_object_interface(obj);
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN) || !_objDrop->is_state_active("\xed\xe5\xf2")) { // "нет"
			obj = _scene->mouse_object_interface();

			if (obj) {
				const char *name = obj->name();

				obj->set_state("del");

				int num = getObjNum(name);
				debugC(4, kDebugMinigames, "base: num is: %d for name: '%s'", num, name);

				_objects[num]->set_state("base");

				_objDrop->set_state("\xed\xe5\xf2"); // "нет"
				_scene->release_object_interface(obj);
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN)) {
			obj = _scene->mouse_object_interface();

			if (obj) {
				qdMinigameObjectInterface *obj2 = _scene->mouse_hover_object_interface();

				if (obj2) {
					const char *name = obj->name();

					if (strstr(name, "@")) {
						int num = getObjNum(name);
						debugC(4, kDebugMinigames, "part1: num is: %d for name: '%s'", num, name);

						name = obj2->name();
						int num2 = getObjNum(name);
						debugC(4, kDebugMinigames, "part2: num2 is: %d for name: '%s'", num2, name);

						if (num == num2) {
							obj->set_state("del");
							_objects[num]->set_state("karta");
						}

						_scene->release_object_interface(obj);
					}
				}
			}
		}

		if (_timePassed > 1.0) {
			if (checkSolution())
				_objDone->set_state("\xe4\xe0");	// "да"

			_timePassed = 0.0;
		}

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2SborKarty::finit()");

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
	bool checkSolution() {
		for (int i = 1; i <= 10; i++)
			if (!_objects[i]->is_state_active("kovrik"))
				return false;

		return true;
	}

	int getObjNum(const char *name) {
		const char *from = strstr(name, "@");
		const char *to = strstr(name, "#");
		char tmp[20];

		Common::strlcpy(tmp, from + 1, to - from);

		return atol(tmp);
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_objects[22] = { nullptr };
	qdMinigameObjectInterface *_objDone = nullptr;
	qdMinigameObjectInterface *_objDrop = nullptr;

	float _timePassed = 0.0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_SBOR_KARTY_H
