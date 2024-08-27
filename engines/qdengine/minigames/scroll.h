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

#ifndef QDENGINE_MINIGAMES_SCROLL_H
#define QDENGINE_MINIGAMES_SCROLL_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdScrollMiniGame : public qdMiniGameInterface {
public:
	qdScrollMiniGame() {}
	~qdScrollMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "Scroll::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_commandObj = _scene->object_interface(_scene->minigame_parameter("command_object"));

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "Scroll::quant(%f)", dt);

		mgVect2i pos = _engine->mouse_cursor_position();

		if (pos.x < 25)
			_commandObj->set_state("\xe2\xeb\xe5\xe2\xee");		// "влево"

		if (pos.x > 775)
			_commandObj->set_state("\xe2\xef\xf0\xe0\xe2\xee");	// "вправо"

		if (pos.x > 25 && pos.x < 775)
			_commandObj->set_state("\xed\xe5\xf2");				// "нет"

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "Scroll::finit()");

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

	qdMinigameObjectInterface *_commandObj  = nullptr;

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_SCROLL_H
