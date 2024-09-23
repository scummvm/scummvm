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

#ifndef QDENGINE_MINIGAMES_SHVEIK_PORTRET_H
#define QDENGINE_MINIGAMES_SHVEIK_PORTRET_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdShveikPortretMiniGame : public qdMiniGameInterface {
public:
	qdShveikPortretMiniGame() {}
	~qdShveikPortretMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "ShveikPortret::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_objArray[1].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31"); // "объект1"
		_objArray[1].x = -1;
		_objArray[1].y = -1;
		_objArray[2].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32"); // "объект2"
		_objArray[2].x = -1;
		_objArray[2].y = -1;
		_objArray[3].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x33"); // "объект3"
		_objArray[3].x = -1;
		_objArray[3].y = -1;
		_objArray[4].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x34"); // "объект4"
		_objArray[4].x = -1;
		_objArray[4].y = -1;
		_objArray[5].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x35"); // "объект5"
		_objArray[5].x = -1;
		_objArray[5].y = -1;
		_objArray[6].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x36"); // "объект6"
		_objArray[6].x = -1;
		_objArray[6].y = -1;
		_objArray[7].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x37"); // "объект7"
		_objArray[7].x = -1;
		_objArray[7].y = -1;
		_objArray[8].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x38"); // "объект8"
		_objArray[8].x = -1;
		_objArray[8].y = -1;
		_objArray[9].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x39"); // "объект9"
		_objArray[9].x = -1;
		_objArray[9].y = -1;
		_objArray[10].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x30"); // "объект10"
		_objArray[10].x = -1;
		_objArray[10].y = -1;
		_objArray[11].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x31"); // "объект11"
		_objArray[11].x = -1;
		_objArray[11].y = -1;
		_objArray[12].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x32"); // "объект12"
		_objArray[12].x = -1;
		_objArray[12].y = -1;
		_objArray[13].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x33"); // "объект13"
		_objArray[13].x = -1;
		_objArray[13].y = -1;
		_objArray[14].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x34"); // "объект14"
		_objArray[14].x = -1;
		_objArray[14].y = -1;
		_objArray[15].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x35"); // "объект15"
		_objArray[15].x = -1;
		_objArray[15].y = -1;
		_objArray[16].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x36"); // "объект16"
		_objArray[16].x = -1;
		_objArray[16].y = -1;
		_objArray[17].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x37"); // "объект17"
		_objArray[17].x = -1;
		_objArray[17].y = -1;
		_objArray[18].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x38"); // "объект18"
		_objArray[18].x = -1;
		_objArray[18].y = -1;
		_objArray[19].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x31\x39"); // "объект19"
		_objArray[19].x = -1;
		_objArray[19].y = -1;
		_objArray[20].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32\x30"); // "объект20"
		_objArray[20].x = -1;
		_objArray[20].y = -1;
		_objArray[21].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32\x31"); // "объект21"
		_objArray[21].x = -1;
		_objArray[21].y = -1;
		_objArray[22].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32\x32"); // "объект22"
		_objArray[22].x = -1;
		_objArray[22].y = -1;
		_objArray[23].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32\x33"); // "объект23"
		_objArray[23].x = -1;
		_objArray[23].y = -1;
		_objArray[24].obj = _scene->object_interface("\xee\xe1\xfa\xe5\xea\xf2\x32\x34"); // "объект24"
		_objArray[24].x = -1;
		_objArray[24].y = -1;

		_clickObj = _scene->object_interface("\x24\xea\xeb\xe8\xea\xe8\x20\xee\xe1\xfa\xe5\xea\xf2\xee\xec"); // "$клики объектом"
		_maskOutsideObj = _scene->object_interface("\x5f\xcc\xc0\xd1\xca\xc0\x20\xec\xe5\xf1\xf2\xee\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec"); // "_МАСКА место за полем"
		_objectClickObj = _scene->object_interface("\x24\xea\xeb\xe8\xea\xe8\x20\xef\xee\x20\xee\xe1\xfa\xe5\xea\xf2\xf3"); // "$клики по объекту"
		_doneObj = _scene->object_interface("$done");
		_wasStartedObj = _scene->object_interface("\x24\xe7\xe0\xef\xf3\xf1\xea\x5f\xe1\xfb\xeb"); // "$запуск_был"
		_oneAbsentObj = _scene->object_interface("$one_absent");
		_exitClickObj = _scene->object_interface("\x24\xea\xeb\xe8\xea\x20\xef\xee\x20\xe2\xfb\xf5\xee\xe4\xf3"); // "$клик по выходу"
		_completePicObj = _scene->object_interface("\xe3\xee\xf2\xee\xe2\xe0\xff\x20\xea\xe0\xf0\xf2\xe8\xed\xea\xe0"); // "готовая картинка"

		_currentPieceRow = -1;
		_currentPieceCol = -1;

		if (_wasStartedObj->is_state_active("\xed\xe5\xf2")) { // "нет"
			// srand(time(0))

			for (int i = 1; i <= 24; i++) {
				switch (qd_rnd(3)) {
				case 0:
					_objArray[i].obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 0"); // "лежит за полем 0")
					break;
				case 1:
					_objArray[i].obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 90"); // "лежит за полем 90")
					break;
				case 2:
					_objArray[i].obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 180"); // "лежит за полем 180")
					break;
				case 3:
					_objArray[i].obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 270"); // "лежит за полем 270")
					break;
				default:
					break;
				}
			}

			_wasStartedObj->set_state("\xe4\xe0"); // "да"

			for (int i = 0; i < 7; i++)
				for (int j = 0; j < 6; j++) {
					_fieldState[i][j].angle = -1;
					_fieldState[i][j].pieceNum = -1;
				}

		} else {
			for (int i = 0; i < 7; i++)
				for (int j = 0; j < 6; j++) {
					_fieldState[i][j].angle = -1;
					_fieldState[i][j].pieceNum = -1;
				}

			if (!_doneObj->is_state_active("\xe4\xe0")) // "да"
				processState();
		}

		_exitClickObj->set_state("\xed\xe5\xf2"); // "нет"
		_draggedInvObjectState = 0;
		_draggedObjectState = 0;

		setPiecesDepth();
		setPiecesPos();

		if (_oneAbsentObj->is_state_active("\xed\xe5\xf2")) { // "нет"
			_objArray[2].obj->set_R(_scene->screen2world_coords(mgVect2i(700, 500), 0.0));
			_oneAbsentObj->set_state("\xf4\xeb\xe0\xe3\x20\xee\xf2\xf0\xe0\xe1\xee\xf2\xe0\xed"); // "флаг отработан"
		}

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ShveikPortret::quant(%f)", dt);

		if (_engine->is_key_pressed(VK_F1)) {
			for (int j = 0; j < 7; j++) {
				for (int i = 0; i < 6; i++) {
					debugN("%02d ", _fieldState[j][i].pieceNum);
				}

				debugN("   ");

				for (int i = 0; i < 6; i++) {
					debugN("%3d ", _fieldState[j][i].angle);
				}

				debugN("\n");
			}

			debug("\n");
		}

		if (!_doneObj->is_state_active("\xe4\xe0")) { // "да"
			if (checkSolution()) {
				_doneObj->set_state("\xe4\xe0"); // "да"
				_completePicObj->set_state("\xeb\xe8\xf6\xe5\xe2\xe0\xff\x20\xf1\xf2\xee\xf0\xee\xed\xe0"); // "лицевая сторона"

				for (int i = 1; i <= 24; i++)
					_objArray[i].obj->set_R(mgVect3f(-1000.0, -1000.0, -100.0));

				_scene->activate_personage("\xd8\xe2\xe5\xe9\xea"); // "Швейк"

				return true;
			}
		}

		if (_exitClickObj->is_state_active("\xe4\xe0")) // "да"
			_exitClickObj->set_state("\xe2\xfb\xf5\xee\xe4\x20\xf0\xe0\xe7\xf0\xe5\xf8\xe5\xed"); // "выход разрешен"

		mgVect2i curPos = _engine->mouse_cursor_position();

		if (_scene->mouse_object_interface()) {
			if (_draggedObjectState) {
				_objArray[_draggedObjectState].obj->set_state(_draggedInvObjectState);
				_draggedInvObjectState = 0;
				_draggedObjectState = 0;
			}
		}

		int state = _clickObj->current_state_index();
		qdMinigameObjectInterface *obj;

		if (state > 24) {
			obj = _objArray[state - 24].obj;

			if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 0")) { // "inv на мыши 0"
				obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 0"); // "лежит за полем 0"
			} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 90")) { // "inv на мыши 90"
				obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 90"); // "лежит за полем 90"
			} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 180")) { // "inv на мыши 180"
				obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 180"); // "лежит за полем 180"
			} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 270")) { // "inv на мыши 270"
				obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec 270"); // "лежит за полем 270"
			}

			if (_engine->mouse_cursor_position().x > 205
					|| _engine->mouse_cursor_position().x <= 155) {
				if (_engine->mouse_cursor_position().x < 600
						|| _engine->mouse_cursor_position().x >= 649) {
					obj->set_R(_scene->screen2world_coords(_engine->mouse_cursor_position(), 0.0));
				} else {
					mgVect2i pos = _engine->mouse_cursor_position();
					pos.x = 649;
					obj->set_R(_scene->screen2world_coords(pos, 0.0));
				}
			} else {
				mgVect2i pos = _engine->mouse_cursor_position();
				pos.x = 155;
				obj->set_R(_scene->screen2world_coords(pos, 0.0));
			}

			obj->update_screen_R();
			_objArray[state - 24].depth = 25.0f;

			setPiecesPos();

			_clickObj->set_state("\xed\xe5\xf2"); // "нет"
			_maskOutsideObj->set_state("\x21\xec\xe0\xf1\xea\xe0"); // "!маска"
		} else if (state > 0) {
			_currentPieceRow = getPieceNumber(curPos.x, 204, 4, 99);
			_currentPieceCol = getPieceNumber(curPos.y, 4, 6, 99);

			obj = _objArray[state].obj;

			if (_fieldState[_currentPieceRow][_currentPieceCol].pieceNum == -1) {
				if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 0")) { // "inv на мыши 0"
					obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 0"); // "лежит на поле 0"
					_fieldState[_currentPieceRow][_currentPieceCol].angle = 0;
				} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 90")) { // "inv на мыши 90"
					obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 90"); // "лежит на поле 90"
					_fieldState[_currentPieceRow][_currentPieceCol].angle = 90;
				} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 180")) { // "inv на мыши 180"
					obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 180"); // "лежит на поле 180"
					_fieldState[_currentPieceRow][_currentPieceCol].angle = 180;
				} else if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 270")) { // "inv на мыши 270"
					obj->set_state("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 270"); // "лежит на поле 270"
					_fieldState[_currentPieceRow][_currentPieceCol].angle = 270;
				}

				mgVect2i pos;

				pos.x = 99 * _currentPieceRow + 154;
				pos.y = 99 * _currentPieceCol - 46;

				obj->set_R(_scene->screen2world_coords(pos, 0.0));

				_objArray[state].x = _currentPieceRow;
				_objArray[state].y = _currentPieceCol;
				_fieldState[_currentPieceRow][_currentPieceCol].pieceNum = state;
			}

			_clickObj->set_state("\xed\xe5\xf2"); // "нет"
			_maskOutsideObj->set_state("\x21\xec\xe0\xf1\xea\xe0"); // "!маска"
		}

		state = _objectClickObj->current_state_index();

		if (state > 0) {
			obj = _objArray[state].obj;

			if (obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 0") // "лежит на поле 0"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 90") // "лежит на поле 90"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 180") // "лежит на поле 180"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 270")) { // "лежит на поле 270"
				_fieldState[_objArray[state].x][_objArray[state].y].pieceNum = -1;
				_fieldState[_objArray[state].x][_objArray[state].y].angle = -1;

				_objArray[state].x = -1;
				_objArray[state].y = -1;
			}

			if (obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 0") // "лежит на поле 0"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec\x20\x30")) { // "лежит за полем 0"
				_draggedInvObjectState = obj->state_index("inv \xed\xe0\x20\xec\xfb\xf8\xe8 0"); // "inv на мыши 0"
			}
			if (obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 90") // "лежит на поле 90"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec\x20\x39\x30")) { // "лежит за полем 90"
				_draggedInvObjectState = obj->state_index("inv \xed\xe0\x20\xec\xfb\xf8\xe8 90"); // "inv на мыши 90"
			}
			if (obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 180") // "лежит на поле 180"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec\x20\x31\x38\x30")) { // "лежит за полем 180"
				_draggedInvObjectState = obj->state_index("inv \xed\xe0\x20\xec\xfb\xf8\xe8 180"); // "inv на мыши 180"
			}
			if (obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5 270") // "лежит на поле 270"
					|| obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xe7\xe0\x20\xef\xee\xeb\xe5\xec\x20\x32\x37\x30")) { // "лежит за полем 270"
				_draggedInvObjectState = obj->state_index("inv \xed\xe0\x20\xec\xfb\xf8\xe8 270"); // "inv на мыши 270"
			}

			_draggedObjectState = state;

			obj->set_state("to_inv");

			_objectClickObj->set_state("\xed\xe5\xf2"); // "нет"
			_maskOutsideObj->set_state("\xd4\xee\xed\x20\x2d\x20\xec\xe0\xf1\xea\xe0"); // Фон - маска"
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN)
				|| _engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DBLCLICK)) {
			obj =_scene->mouse_object_interface();

			if (obj) {
				if (obj->has_state("\xf3\xe4\xe0\xeb\xe8\xf2\xfc")) { // "удалить"
					if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 0")) { // "inv на мыши 0"
						obj->set_state("inv \xed\xe0\x20\xec\xfb\xf8\xe8 90"); // "inv на мыши 90"
						return true;
					}
					if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 90")) { // "inv на мыши 90"
						obj->set_state("inv \xed\xe0\x20\xec\xfb\xf8\xe8 180"); // "inv на мыши 180"
						return true;
					}
					if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 180")) { // "inv на мыши 180"
						obj->set_state("inv \xed\xe0\x20\xec\xfb\xf8\xe8 270"); // "inv на мыши 270"
						return true;
					}
					if (obj->is_state_active("inv \xed\xe0\x20\xec\xfb\xf8\xe8 270")) { // "inv на мыши 270"
						obj->set_state("inv \xed\xe0\x20\xec\xfb\xf8\xe8 0"); // "inv на мыши 0"
						return true;
					}
				}
			}
		}

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "ShveikPortret::finit()");

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
	void processState() {
		for (int i = 1; i <= 24; i++) {
			if (_objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x30") // "лежит на поле 0"
					|| _objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x39\x30") // "лежит на поле 90"
					|| _objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x31\x38\x30") // "лежит на поле 180"
					|| _objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x32\x37\x30")) { // "лежит на поле 270"
				mgVect2i pos = _objArray[i].obj->screen_R();

				_objArray[i].x = (pos.x - 204) / 99 + 1;
				_objArray[i].y = (pos.y - 4) / 99 + 1;
				_fieldState[_objArray[i].x][_objArray[i].y].pieceNum = i;

				if (_objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x30")) { // "лежит на поле 0"
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 0;
				} else if (_objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x39\x30")) { // "лежит на поле 90"
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 90;
				} else if (_objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x31\x38\x30")) { // "лежит на поле 180"
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 180;
				} else if (_objArray[i].obj->is_state_active("\xeb\xe5\xe6\xe8\xf2\x20\xed\xe0\x20\xef\xee\xeb\xe5\x20\x32\x37\x30")) { // "лежит на поле 270"
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 270;
				}
			} else {
				_objArray[i].x = -1;
				_objArray[i].y = -1;
			}
		}
	}

	void setPiecesDepth() {
		for (int i = 1; i <= 24; i++) {
			_objArray[i].depth = i;
			_objArray[i].obj->set_R(_scene->screen2world_coords(_objArray[i].obj->screen_R(), 0.0));
		}
	}

	void setPiecesPos() {
		int target = -1;

		for (int j = 1; j <= 24; j++) {
			int i;
			for (i = 1; i <= 24; ++i) {
				if (j == _objArray[i].depth)
					i = 26;
			}

			if (i == 25) {
				target = j;
				break;
			}
		}

		if (target != -1) {
			for (int i = 1; i <= 24; i++) {
				if (target < _objArray[i].depth)
					_objArray[i].depth -= 1.0;
			}

			for (int i = 1; i <= 24; i++) {
				_objArray[i].obj->set_R(_scene->screen2world_coords(_objArray[i].obj->screen_R(), 100.0 - _objArray[i].depth * 100.0));
			}
		}
	}

	bool checkSolution() {
		int n = 1;

		// The original was comparing with the static array
		for (int j = 1; j < 6; j++)
			for (int i = 0; i < 6; i++) {
				if ((j == 1 && i == 0) || (j == 5 && i > 0))
					continue;

				if (_fieldState[j][i].pieceNum != n || _fieldState[j][i].angle != 0)
					return false;

				n++;
			}

		return true;
	}

	int getPieceNumber(int left, int right, int dimSize, int step) {
		if (dimSize < 1)
			return dimSize;

		int res = dimSize;
		int curPos = step + 204;

		for (int i = 1; i <= dimSize; i++) {
			if (right <= left && curPos > left)
				res = i;

			right += step;
			curPos += step;
		}

		return res;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	struct PieceState {
		int pieceNum = -1;
		int angle = -1;
	};

	struct ObjectStruct {
		qdMinigameObjectInterface *obj = nullptr;
		int x = 0;
		int y = 0;
		float depth = 0.0;
	};

	ObjectStruct _objArray[25];

	int _draggedObjectState = 0;
	int _draggedInvObjectState = 0;

	qdMinigameObjectInterface *_clickObj = nullptr;
	qdMinigameObjectInterface *_objectClickObj = nullptr;
	qdMinigameObjectInterface *_maskOutsideObj = nullptr;
	qdMinigameObjectInterface *_wasStartedObj = nullptr;
	qdMinigameObjectInterface *_oneAbsentObj = nullptr;
	qdMinigameObjectInterface *_exitClickObj = nullptr;
	qdMinigameObjectInterface *_completePicObj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;

	int _currentPieceRow = 0;
	int _currentPieceCol = 0;

	PieceState _fieldState[7][6];
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_SHVEIK_PORTRET_H
