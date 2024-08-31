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

#ifndef QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H
#define QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {


class qdEmptyMiniGameInterface : public qdMiniGameInterface {
public:
	typedef qdEmptyMiniGameInterface *(*interface_open_proc)(const char *game_name);
	typedef bool (*interface_close_proc)(qdEmptyMiniGameInterface *p);

	~qdEmptyMiniGameInterface() { };

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "qdEmptyMinigameInterface: This is a placeholder class");
		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "qdEmptyMiniGameInterface::quant()");
		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "qdEmptyMiniGameInterface::finit()");
		return true;
	}

	bool new_game(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "qdEmptyMiniGameInterface::new_game()");
		return true;
	}

	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		debugC(1, kDebugMinigames, "qdEmptyMiniGameInterface::save_game()");
		return 0;
	}

	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		debugC(1, kDebugMinigames, "qdEmptyMiniGameInterface::load_game()");
		return 0;
	}

	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_QD_EMPTY_INTERFACE_H
