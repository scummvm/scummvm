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

class qd3mince2SborKartyMiniGame : public qdMiniGameInterface {
public:
	qd3mince2SborKartyMiniGame() {}
	~qd3mince2SborKartyMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mince2SborKarty::init()");

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
		debugC(3, kDebugMinigames, "3mince2SborKarty::quant(%f)", dt);


		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mince2SborKarty::finit()");

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

		return true;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_objects[22];
	qdMinigameObjectInterface *_objDone = nullptr;
	qdMinigameObjectInterface *_objDrop = nullptr;

	float _timePassed;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_SBOR_KARTY_H
