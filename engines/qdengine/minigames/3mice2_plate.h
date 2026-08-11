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

#ifndef QDENGINE_MINIGAMES_3MICE2_PLATE_H
#define QDENGINE_MINIGAMES_3MICE2_PLATE_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2PlateMiniGame : public qdMiniGameInterface {
public:
	qd3mice2PlateMiniGame() {}
	~qd3mice2PlateMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2Plate::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		for (int i = 1; i <= 5; i++) {
			_figures[i] = _scene->object_interface(Common::String::format("object@%i#", i).c_str());
			_figures[i + 5] = _scene->object_interface(Common::String::format("inv_object@%i#", i).c_str());
		}

		_objDone = _scene->object_interface("$done");
		_objZoneTarget = _scene->object_interface("zone_target");
		_objZoneFull = _scene->object_interface("zone_full");
		_objFading = _scene->object_interface("\xE7\xE0\xF2\xE5\xEC\xED\xE5\xED\xE8\xE5"); // "затемнение"
		_objHasStarted = _scene->object_interface("$\xE7\xE0\xEF\xF3\xF1\xEA \xE1\xFB\xEB"); // "$запуск был"

		_initialCoords[1] = 54;
		_initialCoords[2] = 99;
		_initialCoords[3] = 693;
		_initialCoords[4] = 673;
		_initialCoords[5] = 706;
		_initialCoords[7] = 452;
		_initialCoords[8] = 98;
		_initialCoords[9] = 436;
		_initialCoords[10] = 93;
		_initialCoords[11] = 254;

		mgVect2i pos;

		if (_objHasStarted->is_state_active("\xed\xe5\xf2")) { // "нет"
			for (int i = 0; i < 5; i++) {
				pos.x = _initialCoords[i + 1];
				pos.y = _initialCoords[i + 7];

				_figures[i + 1]->set_R(_scene->screen2world_coords(pos, 0.0));
			}

			_zoneDepth = -100;
			_objHasStarted->set_state("\xe4\xe0");	// "да"
		} else {
			float depth = 0.0;

			for (int i = 0; i < 5; i++) {
				if (_scene->screen_depth(_figures[i + 1]->R()) < depth)
					depth = _scene->screen_depth(_figures[i + 1]->R());
			}

			_zoneDepth = depth - 100;
		}

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2Plate::quant(%f)", dt);

		qdMinigameObjectInterface *mouseObj = _scene->mouse_object_interface();
		qdMinigameObjectInterface *clickObj = _scene->mouse_click_object_interface();
		qdMinigameObjectInterface *hoverObj = _scene->mouse_hover_object_interface();

		const char *name;

		if (clickObj && !mouseObj) {
			name = clickObj->name();

			if (strstr(name, "object@") && clickObj->is_state_active("base")) {
				int num = getObjNum(name);

				clickObj->set_state("hide");

				debugC(2, kDebugMinigames, "to_inv: num is: %d for name: '%s'", num, name);

				_figures[num + 5]->set_state("to_inv");
				_objZoneFull->set_state("\xD4\xEE\xED - \xEC\xE0\xF1\xEA\xE0"); // "Фон - маска"

				_objZoneFull->set_R(_scene->screen2world_coords(_objZoneFull->screen_R(), _zoneDepth - 500));
				_objZoneTarget->set_R(_scene->screen2world_coords(_objZoneTarget->screen_R(), _zoneDepth - 1500));
				_objFading->set_R(_scene->screen2world_coords(_objFading->screen_R(), _zoneDepth - 3500));
			} else if (!strcmp(name, "zone_target")) {
				int minDepth = 32768.0;
				int num = -1;

				for (int i = 0; i < 5; i++) {
					float depth = _scene->screen_depth(_figures[i + 1]->R());
					if (depth < minDepth && _figures[i + 1]->is_state_active("plate")) {
						minDepth = depth;
						num = i + 1;
					}
				}

				debugC(2, kDebugMinigames, "zone_target: num is: %d", num);

				if (num > -1) {
					_figures[num]->set_state("hide");
					_figures[num + 5]->set_state("to_inv");
					_objZoneFull->set_state("\xD4\xEE\xED - \xEC\xE0\xF1\xEA\xE0"); // "Фон - маска"

					_objZoneFull->set_R(_scene->screen2world_coords(_objZoneFull->screen_R(), _zoneDepth - 1500));
					_objZoneTarget->set_R(_scene->screen2world_coords(_objZoneTarget->screen_R(), _zoneDepth - 2500));
				}
			}

			_objFading->set_R(_scene->screen2world_coords(_objFading->screen_R(), _zoneDepth - 3500));

			_scene->release_object_interface(clickObj);
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN) && mouseObj) {
			name = mouseObj->name();

			int num = getObjNum(name);

			debugC(2, kDebugMinigames, "base: num is: %d for name: '%s'", num, name);

			mouseObj->set_state("del");

			_figures[num]->set_state("base");

			mgVect2i pos;

			pos.x = _initialCoords[num];
			pos.y = _initialCoords[num + 6];

			_figures[num]->set_R(_scene->screen2world_coords(pos, 0));

			_scene->release_object_interface(mouseObj);

			_objZoneFull->set_state("no");

			_objFading->set_R(_scene->screen2world_coords(_objFading->screen_R(), _zoneDepth - 3500));
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN) && mouseObj && hoverObj) {
			name = mouseObj->name();

			int num = getObjNum(name);

			debugC(2, kDebugMinigames, "plate: num is: %d for name: '%s'", num, name);

			if (!strcmp(hoverObj->name(), "zone_target")) {
				mouseObj->set_state("del");

				_figures[num]->set_state("plate");

				_figures[num]->set_R(_scene->screen2world_coords(mgVect2i(400, 300), _zoneDepth));

				_zoneDepth -= 500;
			} else if (!strcmp(hoverObj->name(), "zone_full")) {
				mouseObj->set_state("del");

				_figures[num]->set_state("base");

				mgVect2i pos;

				pos.x = _initialCoords[num];
				pos.y = _initialCoords[num + 6];

				_figures[num]->set_R(_scene->screen2world_coords(pos, 0));
			}

			_objFading->set_R(_scene->screen2world_coords(_objFading->screen_R(), _zoneDepth - 3500));

			_scene->release_object_interface(mouseObj);

			_objZoneFull->set_state("no");
		}

		if (checkSolution())
			_objDone->set_state("\xe4\xe0");	// "да"
		else
			_objDone->set_state("\xed\xe5\xf2"); // "нет"

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2Plate::finit()");

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
		for (int i = 0; i < 5; i++) {
			if (!_figures[i + 1]->is_state_active("plate"))
				return false;

			if (i == 4)
				break;

			if (_scene->screen_depth(_figures[i + 1]->R()) < _scene->screen_depth(_figures[i + 2]->R()))
				return false;
		}

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

	qdMinigameObjectInterface *_figures[12] = { nullptr };
	qdMinigameObjectInterface *_objDone = nullptr;
	qdMinigameObjectInterface *_objZoneTarget = nullptr;
	qdMinigameObjectInterface *_objZoneFull = nullptr;
	qdMinigameObjectInterface *_objFading = nullptr;
	qdMinigameObjectInterface *_objHasStarted = nullptr;
	int _initialCoords[12] = { 0 };
	int _zoneDepth = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_PLATE_H
