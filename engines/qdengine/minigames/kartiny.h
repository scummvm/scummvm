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

#ifndef QDENGINE_MINIGAMES_KARTINY_H
#define QDENGINE_MINIGAMES_KARTINY_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdKartinyMiniGame : public qdMiniGameInterface {
public:
	qdKartinyMiniGame() {}
	~qdKartinyMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "Kartiny::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return 0;

		_bg_clickObj = _scene->object_interface(_scene->minigame_parameter("zad_click_flag_name"));
		_bg1_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad1_l2_object_name"));
		_bg2_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad2_l2_object_name"));
		_bg3_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad3_l2_object_name"));
		_bg4_l2Obj   = _scene->object_interface(_scene->minigame_parameter("zad4_l2_object_name"));
		_doneObj     = _scene->object_interface(_scene->minigame_parameter("done"));
		_startObj    = _scene->object_interface(_scene->minigame_parameter("start"));
		_activateObj = _scene->object_interface(_scene->minigame_parameter("activate"));
		_zFlagObj    = _scene->object_interface(_scene->minigame_parameter("z_flag"));
		_startObj2   = _scene->object_interface(_scene->minigame_parameter("start"));

		_wasInited = 0;
		_artDepth = -1.0;
		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "Kartiny::quant(%f)", dt);

		mgVect3f newPiecePos;
		mgVect2i piecePos;

		float depth1 = _scene->screen_depth(_bg1_l2Obj->R());
		float depth2 = _scene->screen_depth(_bg2_l2Obj->R());
		float depth3 = _scene->screen_depth(_bg3_l2Obj->R());
		float depth4 = _scene->screen_depth(_bg4_l2Obj->R());

		if (depth4 >= depth3 || depth3 >= depth2 || depth2 >= depth1)
			_zFlagObj->set_state("\xed\xe5\xf2");	// "нет"
		else
			_zFlagObj->set_state("\xe4\xe0");		// "да"

		if (!_wasInited) {
			if (_startObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\xe8 \xee\xef\xf3\xf9\xe5\xed\xfb"))	// "задники опущены"
				_wasInited = 1;

			if (!_wasInited) {
				if (_startObj->is_state_active("\xed\xe5\xf2")) { // "нет"
					_bg1_l2Obj->set_R(_scene->screen2world_coords(mgVect2i(400, -300), 600.0));
					_bg2_l2Obj->set_R(_scene->screen2world_coords(mgVect2i(200, -300), 400.0));
					_bg3_l2Obj->set_R(_scene->screen2world_coords(mgVect2i(400, -300), 200.0));
					_bg4_l2Obj->set_R(_scene->screen2world_coords(mgVect2i(600, -300), 0.0));

					_stage = 4;
					_artDepth = -50.0;

					_activateObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\x34"); // "задник4"
					_startObj->set_state("\xe4\xe0");	// "да"
				} else {
					if (_startObj->is_state_active("\xe4\xe0")) { // "да"
						if (_bg1_l2Obj->screen_R().y < 100) {
							piecePos.x = 400;
							piecePos.y = _bg1_l2Obj->screen_R().y + 3;
							_bg1_l2Obj->set_R(_scene->screen2world_coords(piecePos, 600.0));
						}

						if (_bg2_l2Obj->screen_R().y < 50) {
							piecePos.x = 200;
							piecePos.y = _bg2_l2Obj->screen_R().y + 2;
							_bg2_l2Obj->set_R(_scene->screen2world_coords(piecePos, 400.0));
						}

						if (_bg3_l2Obj->screen_R().y < 85) {
							piecePos.x = 400;
							piecePos.y = _bg3_l2Obj->screen_R().y + 2;
							_bg3_l2Obj->set_R(_scene->screen2world_coords(piecePos, 200.0));
						}

						if (_bg4_l2Obj->screen_R().y < 0) {
							piecePos.x = 600;
							piecePos.y = _bg4_l2Obj->screen_R().y + 2;
							_bg4_l2Obj->set_R(_scene->screen2world_coords(piecePos, 0.0));
						}

						if (_bg1_l2Obj->screen_R().y >= 100
								&& _bg2_l2Obj->screen_R().y >= 50
								&& _bg3_l2Obj->screen_R().y >= 85
								&& _bg4_l2Obj->screen_R().y >= 0) {
							_wasInited = 1;
							_startObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\xe8 \xee\xef\xf3\xf9\xe5\xed\xfb");	// "задники опущены"
						}
					}

					if (_startObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\xe8 \xee\xef\xf3\xf9\xe5\xed\xfb")	// "задники опущены"
							&& _artDepth == -1.0) {
						_artState[0].depth = _scene->screen_depth(_bg1_l2Obj->R());
						_artState[0].num = 1;
						_artState[1].depth = _scene->screen_depth(_bg2_l2Obj->R());
						_artState[1].num = 2;
						_artState[2].depth = _scene->screen_depth(_bg3_l2Obj->R());
						_artState[2].num = 3;
						_artState[3].depth = _scene->screen_depth(_bg4_l2Obj->R());
						_artState[3].num = 4;

						// Sort
						for (int i = 0; i < 3; i++) {
							while (_artState[i + 1].depth < _artState[i].depth) {
								int num = _artState[i].num;
								float depth = _artState[i].depth;

								_artState[i].depth = _artState[i + 1].depth;
								_artState[i].num = _artState[i + 1].num;
								_artState[i + 1].depth = depth;
								_artState[i + 1].num = num;

								i = 0;
							}
						}

						_wasInited = 1;

						_stage = _artState[0].num;
						_artDepth = _artState[0].depth;
					}
				}
			}
		}

		if (!checkSolution() && _wasInited) {
			_doneObj->set_state("false");

			if (_engine->is_key_pressed(VK_LEFT)
					|| _engine->is_key_pressed(VK_RIGHT)
					|| _engine->is_key_pressed(VK_UP)
					|| _engine->is_key_pressed(VK_DOWN)) {
				++_keyDownCounter;
			} else {
				_keyDownCounter = 0;
			}

			if (_engine->is_key_pressed(VK_LEFT)) {
				switch (_stage) {
				case 1:
					moveLeft(_bg1_l2Obj);
					break;
				case 2:
					moveLeft(_bg2_l2Obj);
					break;
				case 3:
					moveLeft(_bg3_l2Obj);
					break;
				case 4:
					moveLeft(_bg4_l2Obj);
					break;
				default:
					break;
				}
			}

			if (_engine->is_key_pressed(VK_RIGHT)) {
				switch (_stage) {
				case 1:
					moveRight(_bg1_l2Obj);
					break;
				case 2:
					moveRight(_bg2_l2Obj);
					break;
				case 3:
					moveRight(_bg3_l2Obj);
					break;
				case 4:
					moveRight(_bg4_l2Obj);
					break;
				default:
					break;
				}
			}

			if (_engine->is_key_pressed(VK_UP)) {
				switch (_stage) {
				case 1:
					moveUp(_bg1_l2Obj);
					break;
				case 2:
					moveUp(_bg2_l2Obj);
					break;
				case 3:
					moveUp(_bg3_l2Obj);
					break;
				case 4:
					moveUp(_bg4_l2Obj);
					break;
				default:
					break;
				}
			}

			if (_engine->is_key_pressed(VK_DOWN)) {
				switch (_stage) {
				case 1:
					moveDown(_bg1_l2Obj);
					break;
				case 2:
					moveDown(_bg2_l2Obj);
					break;
				case 3:
					moveDown(_bg3_l2Obj);
					break;
				case 4:
					moveDown(_bg4_l2Obj);
					break;
				default:
					break;
				}
			}

			if ((_bg_clickObj->is_state_active("zad1_level2")
					|| _activateObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\x31")) && _stage != 1) { // "задник1"
				_stage = 1;
				_activateObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\x31"); // "задник1"
				_artDepth -= 200.0;
				_bg1_l2Obj->set_R(_scene->screen2world_coords(_bg1_l2Obj->screen_R(), _artDepth));

				snapPieces();
			}

			if ((_bg_clickObj->is_state_active("zad2_level2")
					|| _activateObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\x32")) && _stage != 2) { // "задник2"
				_stage = 2;
				_activateObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\x32"); // "задник2"
				_artDepth -= 200.0;
				_bg2_l2Obj->set_R(_scene->screen2world_coords(_bg2_l2Obj->screen_R(), _artDepth));

				snapPieces();
			}

			if ((_bg_clickObj->is_state_active("zad3_level2")
					|| _activateObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\x33")) && _stage != 3) { // "задник3"
				_stage = 3;
				_activateObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\x33"); // "задник3"
				_artDepth -= 200.0;
				_bg3_l2Obj->set_R(_scene->screen2world_coords(_bg3_l2Obj->screen_R(), _artDepth));

				snapPieces();
			}

			if ((_bg_clickObj->is_state_active("zad4_level2")
					|| _activateObj->is_state_active("\xe7\xe0\xe4\xed\xe8\xea\x34")) && _stage != 4) { // "задник4"
				_stage = 4;
				_activateObj->set_state("\xe7\xe0\xe4\xed\xe8\xea\x34"); // "задник4"
				_artDepth -= 200.0;
				_bg4_l2Obj->set_R(_scene->screen2world_coords(_bg4_l2Obj->screen_R(), _artDepth));

				snapPieces();
			}
		}

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "Kartiny::finit()");

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
	void snapPieces() {
		mgVect2i piecePos;
		mgVect3f newPiecePos;
		float depth;

		piecePos = _bg1_l2Obj->screen_R();
		depth = _scene->screen_depth(_bg1_l2Obj->R());

		if (ABS(399 - ABS(piecePos.x)) <= 10 && ABS(278 - ABS(piecePos.y)) <= 10) {
			piecePos.x = 399;
			piecePos.y = 278;

			newPiecePos = _scene->screen2world_coords(piecePos, depth);
			_bg1_l2Obj->set_R(newPiecePos);
		}

		piecePos = _bg2_l2Obj->screen_R();
		depth = _scene->screen_depth(_bg2_l2Obj->R());

		if (piecePos.x >= 387 && piecePos.x <= 440 && ABS(267 - ABS(piecePos.y)) <= 20) {
			piecePos.x = 408;
			piecePos.y = 267;

			newPiecePos = _scene->screen2world_coords(piecePos, depth);
			_bg2_l2Obj->set_R(newPiecePos);
		}

		piecePos = _bg3_l2Obj->screen_R();
		depth = _scene->screen_depth(_bg3_l2Obj->R());

		if (ABS(ABS(piecePos.x) - 413) < 25 && ABS(ABS(piecePos.y) - 43) < 40) {
			piecePos.x = 406;
			piecePos.y = -43;

			newPiecePos = _scene->screen2world_coords(piecePos, depth);
			_bg3_l2Obj->set_R(newPiecePos);
		}
	}

	void moveDown(qdMinigameObjectInterface *obj) {
		int speed = MAX(10, _keyDownCounter / 10 + 1);
		int maxCoords;

		if (obj == _bg1_l2Obj)
			maxCoords = 279;
		else if (obj == _bg2_l2Obj)
			maxCoords = 267;
		else if (obj == _bg3_l2Obj)
			maxCoords = 258;
		else
			maxCoords = 258;

		mgVect2i objPos = obj->screen_R();

		if (objPos.y + speed < maxCoords)
			objPos.y += speed;

		obj->set_R(_scene->screen2world_coords(objPos, _artDepth));
	}

	void moveUp(qdMinigameObjectInterface *obj) {
		int speed = MAX(10, _keyDownCounter / 10 + 1);
		mgVect2i objPos = obj->screen_R();

		if (objPos.y > -100)
			objPos.y -= speed;

		obj->set_R(_scene->screen2world_coords(objPos, _artDepth));
	}

	void moveRight(qdMinigameObjectInterface *obj) {
		int speed = MAX(10, _keyDownCounter / 10 + 1);
		mgVect2i objPos = obj->screen_R();

		if (objPos.x < 900)
			objPos.x += speed;

		obj->set_R(_scene->screen2world_coords(objPos, _artDepth));
	}

	void moveLeft(qdMinigameObjectInterface *obj) {
		int speed = MAX(10, _keyDownCounter / 10 + 1);
		mgVect2i objPos = obj->screen_R();

		if (objPos.x > -100)
			objPos.x -= speed;

		obj->set_R(_scene->screen2world_coords(objPos, _artDepth));
	}

	bool checkSolution() {
		if (_scene->screen_depth(_bg1_l2Obj->R()) < _scene->screen_depth(_bg2_l2Obj->R()))
			return false;

		if (_scene->screen_depth(_bg2_l2Obj->R()) < _scene->screen_depth(_bg3_l2Obj->R()))
			return false;

		if (_scene->screen_depth(_bg3_l2Obj->R()) < _scene->screen_depth(_bg4_l2Obj->R()))
			return false;

		if (ABS(_bg1_l2Obj->screen_R().x - 399) > 5
				|| ABS(_bg1_l2Obj->screen_R().y - 278) > 5
				|| (ABS(_bg2_l2Obj->screen_R().x - 407) > 10
				&& ABS(_bg2_l2Obj->screen_R().x - 420) > 10)) // copy/paste error in the originaL?
			return false;

		if (ABS(_bg2_l2Obj->screen_R().y - 267) > 10)
			return false;

		if (_bg2_l2Obj->screen_R().x - _bg3_l2Obj->screen_R().x > 6)
			return false;

		if (_bg3_l2Obj->screen_R().x - _bg2_l2Obj->screen_R().x > 5)
			return false;

		if (ABS(_bg3_l2Obj->screen_R().y + 43) > 10)
			return false;

		if (_bg4_l2Obj->screen_R().x - _bg3_l2Obj->screen_R().x > 5)
			return false;

		if (_bg3_l2Obj->screen_R().x - _bg4_l2Obj->screen_R().x > 3)
			return false;

		if (ABS(_bg4_l2Obj->screen_R().y + 42) > 10)
			return false;

		if (ABS(_bg4_l2Obj->screen_R().y) - ABS(_bg3_l2Obj->screen_R().y) > 3)
			return false;


		_doneObj->set_state("true");

		return true;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_bg_clickObj = nullptr;
	qdMinigameObjectInterface *_bg1_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg2_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg3_l2Obj = nullptr;
	qdMinigameObjectInterface *_bg4_l2Obj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;
	qdMinigameObjectInterface *_startObj = nullptr;
	qdMinigameObjectInterface *_activateObj = nullptr;
	qdMinigameObjectInterface *_zFlagObj = nullptr;
	qdMinigameObjectInterface *_startObj2 = nullptr;

	bool _wasInited = false;
	float _artDepth = -1.0;
	int _keyDownCounter = 0;

	int _stage = 0;

	struct {
		float depth = 0;
		int num = 0;
	} _artState[4];
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_KARTINY_H
