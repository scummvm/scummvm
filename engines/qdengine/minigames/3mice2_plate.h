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

		for (int i = 1; i <= 5; i++) {
			_figures[i] = _scene->object_interface(Common::String::format("object@%i#", i).c_str());
			_figures[i + 6] = _scene->object_interface(Common::String::format("inv_object@%i#", i).c_str());
		}

		_objDone = _scene->object_interface("$done");
		_objZoneTarget = _scene->object_interface("zone_target");
		_objZoneFull = _scene->object_interface("zone_full");
		_objeFading = _scene->object_interface("затемнение");
		_objHasStarted = _scene->object_interface("$запуск был");

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

		if (_objHasStarted->is_state_active("нет")) {
			for (int i = 0; i < 5; i++) {
				pos.x = _initialCoords[i + 1];
				pos.y = _initialCoords[i + 7];

				_figures[i + 1]->set_R(_scene->screen2world_coords(pos, 0.0));
			}

			_zoneDepth = -100;
			_objHasStarted->set_state("да");
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
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_figures[12] = { nullptr };
	qdMinigameObjectInterface *_objDone = nullptr;
	qdMinigameObjectInterface *_objZoneTarget = nullptr;
	qdMinigameObjectInterface *_objZoneFull = nullptr;
	qdMinigameObjectInterface *_objeFading = nullptr;
	qdMinigameObjectInterface *_objHasStarted = nullptr;
	int _initialCoords[12] = { 0 };
	int _zoneDepth = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_PLATE_H
