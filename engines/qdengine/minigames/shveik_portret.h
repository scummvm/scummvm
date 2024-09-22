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

		_clickObj = _scene->object_interface("$клики объектом");
		_maskOutsideObj = _scene->object_interface("_МАСКА место за полем");
		_objectClickObj = _scene->object_interface("$клики по объекту");
		_doneObj = _scene->object_interface("$done");
		_wasStartedObj = _scene->object_interface("$запуск_был");
		_oneAbsentObj = _scene->object_interface("$one_absent");
		_exitClickObj = _scene->object_interface("$клик по выходу");
		_completePicObj = _scene->object_interface("готовая картинка");

		_currentPieceRow = -1;
		_currentPieceCol = -1;

		if (_wasStartedObj->is_state_active("нет")) {
			// srand(time(0))

			for (int i = 1; i <= 24; i++) {
				switch (qd_rnd(3)) {
				case 0:
					_objArray[i].obj->set_state("лежит за полем 0");
					break;
				case 1:
					_objArray[i].obj->set_state("лежит за полем 90");
					break;
				case 2:
					_objArray[i].obj->set_state("лежит за полем 180");
					break;
				case 3:
					_objArray[i].obj->set_state("лежит за полем 270");
					break;
				default:
					break;
				}
			}

			_wasStartedObj->set_state("да");

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

			if (!_doneObj->is_state_active("да"))
				processState();
		}

		_solutionState[1].pieceNum = 1;
		_solutionState[1].angle = 0;
		_solutionState[2].pieceNum = 2;
		_solutionState[2].angle = 0;
		_solutionState[3].pieceNum = 3;
		_solutionState[3].angle = 0;
		_solutionState[4].pieceNum = 4;
		_solutionState[4].angle = 0;
		_solutionState[5].pieceNum = 5;
		_solutionState[5].angle = 0;
		_solutionState[6].pieceNum = 6;
		_solutionState[6].angle = 0;
		_solutionState[8].pieceNum = 7;
		_solutionState[8].angle = 0;
		_solutionState[9].pieceNum = 8;
		_solutionState[9].angle = 0;
		_solutionState[10].pieceNum = 9;
		_solutionState[10].angle = 0;
		_solutionState[11].pieceNum = 10;
		_solutionState[11].angle = 0;
		_solutionState[12].pieceNum = 11;
		_solutionState[12].angle = 0;
		_solutionState[13].pieceNum = 12;
		_solutionState[13].angle = 0;
		_solutionState[15].pieceNum = 13;
		_solutionState[15].angle = 0;
		_solutionState[16].pieceNum = 14;
		_solutionState[16].angle = 0;
		_solutionState[17].pieceNum = 15;
		_solutionState[17].angle = 0;
		_solutionState[18].pieceNum = 16;
		_solutionState[18].angle = 0;
		_solutionState[19].pieceNum = 17;
		_solutionState[19].angle = 0;
		_solutionState[20].pieceNum = 18;
		_solutionState[20].angle = 0;
		_solutionState[22].pieceNum = 19;
		_solutionState[22].angle = 0;
		_solutionState[23].pieceNum = 20;
		_solutionState[23].angle = 0;
		_solutionState[24].pieceNum = 21;
		_solutionState[24].angle = 0;
		_solutionState[25].pieceNum = 22;
		_solutionState[25].angle = 0;
		_solutionState[26].pieceNum = 23;
		_solutionState[26].angle = 0;
		_solutionState[27].pieceNum = 24;
		_solutionState[27].angle = 0;

		_exitClickObj->set_state("нет");
		_draggedInvObjectState = 0;
		_draggedObjectState = 0;

		setPiecesDepth();
		setPiecesPos();

		if (_oneAbsentObj->is_state_active("нет")) {
			_objArray[2].obj->set_R(_scene->screen2world_coords(mgVect2i(700, 500), 0.0));
			_oneAbsentObj->set_state("флаг отработан");
		}

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ShveikPortret::quant(%f)", dt);

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
			if (_objArray[i].obj->is_state_active("лежит на поле 0")
					|| _objArray[i].obj->is_state_active("лежит на поле 90")
					|| _objArray[i].obj->is_state_active("лежит на поле 180")
					|| _objArray[i].obj->is_state_active("лежит на поле 270")) {
				mgVect2i pos = _objArray[i].obj->screen_R();

				_objArray[i].x = (pos.x - 204) / 99 + 1;
				_objArray[i].y = (pos.y - 4) / 99 + 1;
				_fieldState[_objArray[i].x][_objArray[i].y].pieceNum = i;

				if (_objArray[i].obj->is_state_active("лежит на поле 0")) {
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 0;
				} else if (_objArray[i].obj->is_state_active("лежит на поле 90")) {
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 90;
				} else if (_objArray[i].obj->is_state_active("лежит на поле 180")) {
					_fieldState[_objArray[i].x][_objArray[i].y].angle = 180;
				} else if (_objArray[i].obj->is_state_active("лежит на поле 270")) {
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
	PieceState _solutionState[28];

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_SHVEIK_PORTRET_H
