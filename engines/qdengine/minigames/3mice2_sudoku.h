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

#ifndef QDENGINE_MINIGAMES_3MICE2_SUDOKU_H
#define QDENGINE_MINIGAMES_3MICE2_SUDOKU_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2SudokuMiniGame : public qdMiniGameInterface {
public:
	qd3mice2SudokuMiniGame() {}
	~qd3mice2SudokuMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2Sudoku::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_figures[0] = _scene->object_interface("figure@1#");
		_figures[1] = _scene->object_interface("figure@2#");
		_figures[2] = _scene->object_interface("figure@3#");
		_figures[3] = _scene->object_interface("figure@4#");
		_figures[4] = _scene->object_interface("figure@5#");
		_figures[5] = _scene->object_interface("figure@6#");
		_figures[6] = _scene->object_interface("figure@7#");
		_figures[7] = _scene->object_interface("figure@8#");
		_figures[8] = _scene->object_interface("figure@9#");
		_figures[9] = _scene->object_interface("figure@10#");
		_figures[10] = _scene->object_interface("figure@11#");
		_figures[11] = _scene->object_interface("figure@12#");
		_figures[12] = _scene->object_interface("figure@13#");

		_figures[13] = _scene->object_interface("figure@1#inv");
		_figures[14] = _scene->object_interface("figure@2#inv");
		_figures[15] = _scene->object_interface("figure@3#inv");
		_figures[16] = _scene->object_interface("figure@4#inv");
		_figures[17] = _scene->object_interface("figure@5#inv");
		_figures[18] = _scene->object_interface("figure@6#inv");
		_figures[19] = _scene->object_interface("figure@7#inv");
		_figures[20] = _scene->object_interface("figure@8#inv");
		_figures[21] = _scene->object_interface("figure@9#inv");
		_figures[22] = _scene->object_interface("figure@10#inv");
		_figures[23] = _scene->object_interface("figure@11#inv");
		_figures[24] = _scene->object_interface("figure@12#inv");
		_figures[25] = _scene->object_interface("figure@13#inv");

		_pos[0] = 20;
		_pos[1] = 10;
		_pos[2] = 26;
		_pos[3] = 1;
		_pos[4] = 3;
		_pos[5] = 17;
		_pos[6] = 8;
		_pos[7] = 20;
		_pos[8] = 6;
		_pos[9] = 16;
		_pos[10] = 17;
		_pos[11] = 14;
		_pos[12] = 18;

		_objDone = _scene->object_interface("$done");
		_objResult = _scene->object_interface("$результат");
		_objSettled = _scene->object_interface("$разложено");
		_objReset = _scene->object_interface("$обнулить");
		_objTarget = _scene->object_interface("target");
		_objLoading = _scene->object_interface("$загрузка");

		if (_objLoading->is_state_active("no")) {
			memset(_someArray, 0xFFu, sizeof(_someArray));

			_objLoading->set_state("yes");

			return true;
		} else {
			if (_objLoading->is_state_active("yes")) {
				for (int i = 0; i < 13; i++) {
					_objTarget->set_state(Common::String::format("%d", i + 1).c_str());
					_objTarget->update_screen_R();

					_pieceCoords[i] = _objTarget->screen_R();

					_someArray[i] = -1;
				}

				for (int i = 0; i < 13; i++) {
					if (_figures[i]->screen_R().x != _pieceCoords[i].x &&
						_figures[i]->screen_R().y != _pieceCoords[i].y)
						_someArray[i] = i;
				}
			}
		}

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2Sudoku::quant(%f)", dt);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2Sudoku::finit()");

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

	qdMinigameObjectInterface *_figures[26];
	int _pos[13];
	mgVect2i _pieceCoords[13];
	int _someArray[13];

	qdMinigameObjectInterface *_objDone = nullptr;
	qdMinigameObjectInterface *_objResult = nullptr;
	qdMinigameObjectInterface *_objSettled = nullptr;
	qdMinigameObjectInterface *_objReset = nullptr;
	qdMinigameObjectInterface *_objTarget = nullptr;
	qdMinigameObjectInterface *_objLoading = nullptr;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_SUDOKU_H
