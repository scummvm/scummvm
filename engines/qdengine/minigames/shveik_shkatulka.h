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

		_direction = kDirNone;
		_cursorTakenFlag = 0;
		_someVar3 = 0;
		_needSnap = false;

		if (_startObj->is_state_active("\xed\xe5\x20\xe1\xfb\xeb")) { // "не был"
			resetStones();
			_startObj->set_state("\xe1\xfb\xeb"); // "был"
		}

		return true;
	}

	enum {
		kDirNone,
		kDirUp,
		kDirRight,
		kDirDown,
		kDirLeft
	};

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ShveikShkatulka::quant(%f)", dt);

		if (checkSolution()) {
			_doneObj->set_state("\xe4\xe0");	// "да"
			_cursorObj->set_state("nottaken");
			_draggedStone = 0;
			_direction = kDirNone;
			_cursorTakenFlag = 0;
			_someVar3 = 0;
			_needSnap = 0;

			return true;
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN)
				&& _scene->mouse_click_object_interface()) {
			const char *state = _scene->mouse_click_object_interface()->current_state_name();

			if (state[0] == '1' || state[0] == '2') {
				_draggedStone = _scene->mouse_click_object_interface();

				_mousePos = _engine->mouse_cursor_position();
				_cursorTakenFlag = 1;
				_someVar3 = 1;
			} else if (state[0] == '0') {
				resetStones();
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_UP) && _cursorTakenFlag) {
			_draggedStone = 0;
			_cursorTakenFlag = 0;
			_someVar3 = 0;
			_needSnap = false;
			_direction = kDirNone;
		}

		mgVect2i curPos = _engine->mouse_cursor_position();

		if (_cursorTakenFlag) {
			if (!_needSnap) {
				_mouseDelta.x = curPos.x - _mousePos.x;
				_mouseDelta.y = curPos.y - _mousePos.y;

				if (ABS(_mouseDelta.x) <= ABS(_mouseDelta.y)) { // Going down
					const char *state = _draggedStone->current_state_name();
					int draggedStonePos;

					if (state[2] == '\0')
						draggedStonePos = state[1] - '0';
					else if (state[2] == '0')
						draggedStonePos = 10;
					else
						draggedStonePos = 11;

					int off = 60;
					if (draggedStonePos == 3 || draggedStonePos == 8) {
						int delta = 0;

						if (curPos.y <= _mousePos.y) {
							if (draggedStonePos == 3)
								delta = 186;
							else
								delta = 0;
						} else {
							if (draggedStonePos == 8)
								delta = 196;
							else
								delta = 0;
						}
						off = delta + 150;
					}

					if (ABS(_mouseDelta.y) > off) {
						if (curPos.y <= _mousePos.y)
							_direction = kDirUp;
						else
							_direction = kDirDown;

						_needSnap = true;
					}
				} else if (ABS(_mouseDelta.x) > 60) {
					if (curPos.x <= _mousePos.x)
						_direction = kDirLeft;
					else
						_direction = kDirRight;

					_needSnap = true;
				}
			}

			if (_cursorTakenFlag && _needSnap) {
				const char *state = _draggedStone->current_state_name();
				int draggedStonePos;

				if (state[2] == '\0')
					draggedStonePos = state[1] - '0';
				else if (state[2] == '0')
					draggedStonePos = 10;
				else
					draggedStonePos = 11;

				int side = state[0] - '0';
				int pos = -1;

				if (side == 1) {
					switch (_direction) {
					case kDirUp:
						if (draggedStonePos != 8 || checkStonePosition(3, 1) || checkStonePosition(8, 2))
							break;

						pos = 3;
						break;
					case kDirRight:
						if (draggedStonePos == 4
								|| draggedStonePos == 11
								|| checkStonePosition(draggedStonePos + 1, 1)
								|| checkStonePosition(draggedStonePos, 2))
							break;

						pos = draggedStonePos + 1;
						break;
					case kDirDown:
						if (draggedStonePos != 3 || checkStonePosition(8, 1) || checkStonePosition(8, 2))
							break;

						pos = 8;
						break;
					case kDirLeft:
						if (draggedStonePos == 1
								|| draggedStonePos == 5
								|| checkStonePosition(draggedStonePos - 1, 1)
								|| checkStonePosition(draggedStonePos - 1, 2))
							break;

						pos = draggedStonePos - 1;
						break;

					default:
						break;
					}
				} else {
					switch (_direction) {
					case kDirUp:
						if (draggedStonePos != 8 || checkStonePosition(3, 1) || checkStonePosition(3, 2))
							break;

						pos = 3;
						break;

					case kDirRight:
						if (draggedStonePos == 4
								|| draggedStonePos == 11
								|| checkStonePosition(draggedStonePos + 1, 1)
								|| checkStonePosition(draggedStonePos + 1, 2))
							break;

						pos = draggedStonePos + 1;
						break;

					case kDirDown:
						if (draggedStonePos != 3 || checkStonePosition(8, 2) || checkStonePosition(3, 1))
							break;

						pos = 8;
						break;

					case kDirLeft:
						if (draggedStonePos == 1
								|| draggedStonePos == 5
								|| checkStonePosition(draggedStonePos, 1)
								|| checkStonePosition(draggedStonePos - 1, 2))
							break;

						pos = draggedStonePos - 1;
						break;

					default:
						break;
					}
				}

				if (pos == -1) {
					_cursorTakenFlag = 0;
					_someVar3 = 0;
				} else {
					_draggedStone->set_state(Common::String::format("%d%d", side, pos).c_str());
					_mousePos = curPos;
					_jumpSoundObj->set_state("\xe4\xe0");	// "да"
				}

				_direction = kDirNone;
				_needSnap = false;
			}
		}

		if (_cursorTakenFlag)
			_cursorObj->set_state("taken");
		else
			_cursorObj->set_state("nottaken");

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

	bool checkSolution() {
		return
			   _stones[0]->is_state_active("110")
			&& _stones[1]->is_state_active("16")
			&& _stones[2]->is_state_active("17")
			&& _stones[3]->is_state_active("18")
			&& _stones[4]->is_state_active("15")
			&& _stones[5]->is_state_active("111")
			&& _stones[6]->is_state_active("19")
			&& _stones[7]->is_state_active("26")
			&& _stones[8]->is_state_active("28")
			&& _stones[9]->is_state_active("210")
			&& _stones[10]->is_state_active("211")
			&& _stones[11]->is_state_active("29")
			&& _stones[12]->is_state_active("27")
			&& _stones[13]->is_state_active("25");
	}

	bool checkStonePosition(int targetPos, int leftStones) {
		int n = leftStones == 1 ? 0 : 7;

		for (int i = 0; i < 7; i++) {
			const char *state = _stones[n + i]->current_state_name();
			int pos;

			if (state[2] == '\0')
				pos = state[1] - '0';
			else if (state[2] == '0')
				pos = 10;
			else
				pos = 11;

			if (pos == targetPos)
				return true;
		}

		return false;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	int _direction = kDirNone;
	bool _cursorTakenFlag = false;

	mgVect2i _mousePos;
	mgVect2i _mouseDelta;
	int _someVar3 = 0;
	bool _needSnap = false;

	qdMinigameObjectInterface *_stones[14];
	qdMinigameObjectInterface *_draggedStone = nullptr;
	qdMinigameObjectInterface *_cursorObj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;
	qdMinigameObjectInterface *_startObj = nullptr;
	qdMinigameObjectInterface *_jumpSoundObj = nullptr;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_SHVEIK_SHKATULKA_H
