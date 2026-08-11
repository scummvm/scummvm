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

#ifndef QDENGINE_MINIGAMES_MASKY_ORDER_H
#define QDENGINE_MINIGAMES_MASKY_ORDER_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

const int pieceCoordsFront[24] = {
	280,  93, 284, 163, 394, 140, 505, 120, 511, 225,
	392, 255, 296, 288, 444, 369, 309, 406, 446, 479,
	289, 478, 548, 420,
};

const int pieceCoordsBack[24] = {
	516,  94, 511, 162, 398, 139, 293, 120, 286, 227,
	404, 247, 500, 287, 353, 363, 488, 407, 354, 476,
	508, 478, 247, 420,
};


class qdMaskyOrderMiniGame : public qdMiniGameInterface {
public:
	qdMaskyOrderMiniGame() {
		for (uint i = 0; i < ARRAYSIZE(_pieces); i++)
			_pieces[i] = nullptr;
	}
	~qdMaskyOrderMiniGame() {};

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "MaskyOrder::init()");

		_engine = engine_interface;
		_scene = _engine->current_scene_interface();
		if (!_scene)
			return false;

		for (int i = 0; i < 12; i++)
			_pieces[i] = _scene->object_interface(_scene->minigame_parameter(Common::String::format("object_%i", i + 1).c_str()));

		_finalObj = _scene->object_interface(_scene->minigame_parameter("final"));
		_doneObj = _scene->object_interface(_scene->minigame_parameter("done_object"));
		_object3Flag = _scene->object_interface(_scene->minigame_parameter("object_3_flag"));
		_loadGameObj = _scene->object_interface(_scene->minigame_parameter("loadgame"));

		_wasInited = false;

        _isFinal = false;
        _minDepthPiece = findMinDepthPiece();

		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "MaskyOrder::quant(%f)", dt);

		if (!_wasInited && _loadGameObj->is_state_active("\xed\xe5\xf2")) {	// "нет"
			_rotatingPiece = -1;
			_currentPieceState = -1;
			_pieceIsPut = false;
			_isFinal = false;
			_flipped = false;
			_wasInited = true;
			_loadGameObj->set_state("\xe4\xe0");	// "да"
		}

		if ( _object3Flag->is_state_active("\xe4\xe0")) {	// "да"
			_flipped = true;
			_pieces[2]->set_state("back");
			_object3Flag->set_state("\xed\xe5\xf2");	// "нет"

			_minDepthPiece = findMinDepthPiece();

			mgVect3f coords = _scene->screen2world_coords(_pieces[2]->screen_R(), _minDepthPiece);
			_pieces[2]->set_R(coords);
		}

  		if (_isFinal)
			return true;

		if (!checkSolution() || _scene->mouse_object_interface()) {
			qdMinigameObjectInterface *mouseObj = _scene->mouse_object_interface();

			if (_pieceIsPut) {
				for (int i = 0; i < 12; i++) {
					if (_pieces[i]->is_state_active("to_inv_flag_back")
							|| _pieces[i]->is_state_active("to_inv_flag_front")) {

						if (_pieces[i]->is_state_active("to_inv_flag_back"))
							_currentPieceState = _pieces[i]->state_index("inv_back");
						else if (_pieces[i]->is_state_active("to_inv_flag_front"))
							_currentPieceState = _pieces[i]->state_index("inv_front");

						_rotatingPiece = i;
						_pieces[i]->set_state("to_inv");
						_pieceIsPut = false;
					}
				}
			}

			if (mouseObj) {
				if (_rotatingPiece != -1) {
					_pieces[_rotatingPiece]->set_state(_currentPieceState);
					_currentPieceState = -1;
					_rotatingPiece = -1;
				}
			}

			if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN)) {
				mgVect2i mousePos = _engine->mouse_cursor_position();
				qdMinigameObjectInterface *obj = _scene->mouse_object_interface();

				if (obj) {
					if (obj->is_state_active("inv_back"))
						obj->set_state("back");
					else if (obj->is_state_active("inv_front"))
						obj->set_state("front");

					_minDepthPiece -= 60.0;

					mgVect3f coords = _scene->screen2world_coords(mousePos, _minDepthPiece);
					obj->set_R(coords);

					snapPieces();
				} else {
					_pieceIsPut = true;
				}
			}
		}

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN)) {
			qdMinigameObjectInterface *obj = _scene->mouse_object_interface();

			if (obj) {
				if (obj->is_state_active("inv_back"))
					obj->set_state("inv_front");
				else if (obj->is_state_active("inv_front"))
					obj->set_state("inv_back");
			}
		}

		return true;
	}

	bool checkSolution() {
		if (_scene->mouse_object_interface())
			return false;

		if (_pieces[0]->is_state_active("front")) {
			for (int i = 0; i < 12; i++)
				if (!_pieces[0]->is_state_active("front"))
					return false;
		} else if (_pieces[0]->is_state_active("back")) {
			for (int i = 0; i < 12; i++)
				if (!_pieces[0]->is_state_active("back"))
					return false;
		} else {
			return false;
		}

		mgVect2i piecePos;

		if (_pieces[0]->is_state_active("front"))
			for (int i = 0; i < 12; i++) {
				piecePos = _pieces[i]->screen_R();

				if (ABS(pieceCoordsFront[i * 2 + 0] - piecePos.x) > 10 ||
					ABS(pieceCoordsFront[i * 2 + 1] - piecePos.y) > 10)
					return false;
			}

		if (_pieces[0]->is_state_active("back"))
			for (int i = 0; i < 12; i++) {
				piecePos = _pieces[i]->screen_R();

				if (ABS(pieceCoordsBack[i * 2 + 0] - piecePos.x) > 10 ||
					ABS(pieceCoordsBack[i * 2 + 1] - piecePos.y) > 10)
					return false;
			}

		_isFinal = true;
		_finalObj->set_state("\xe4\xe0");	// "да"

		setPiecePositions();

		return true;
	}

	void snapPieces() {
		mgVect2i piecePos;
		mgVect3f newPiecePos;

		for (int i = 0; i < 12; i++) {
			piecePos = _pieces[i]->screen_R();
			float depth = _scene->screen_depth(_pieces[i]->R());
			bool modified = false;

			if (_pieces[i]->is_state_active("front")) {
				if (ABS(pieceCoordsFront[i * 2 + 0] - piecePos.x) <= 10 &&
						ABS(pieceCoordsFront[i * 2 + 1] - piecePos.y) <= 10) {
					piecePos.x = pieceCoordsFront[i * 2 + 0];
					piecePos.y = pieceCoordsFront[i * 2 + 1];

					modified = true;
				}
			}

			if (_pieces[i]->is_state_active("back")) {
				if (ABS(pieceCoordsBack[i * 2 + 0] - piecePos.x) <= 10 &&
						ABS(pieceCoordsBack[i * 2 + 1] - piecePos.y) <= 10) {
					piecePos.x = pieceCoordsBack[i * 2 + 0];
					piecePos.y = pieceCoordsBack[i * 2 + 1];

					modified = true;
				}
			}

			if (modified) {
				newPiecePos = _scene->screen2world_coords(piecePos, depth);
				_pieces[i]->set_R(newPiecePos);
			}
		}
	}

	bool finit() {
		debugC(1, kDebugMinigames, "MaskyOrder::finit()");

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
	float findMinDepthPiece() {
		float min = 100000.0;

		for (int i = 0; i < 12; i++) {
			float depth = _scene->screen_depth(_pieces[i]->R());

			if (min > depth)
				min = depth;
		}

		return min;
	}

	void setPiecePositions() {
		mgVect3f coords(1000.0, 1000.0, 0.0);

		for (int i = 0; i < 12; i++) {
			_pieces[i]->set_R(coords);
		}

		if (_pieces[0]->is_state_active("back"))
			_doneObj->set_state("back");
		else
			_doneObj->set_state("front");
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_pieces[12];

	qdMinigameObjectInterface *_finalObj = nullptr;
	qdMinigameObjectInterface *_doneObj = nullptr;
	qdMinigameObjectInterface *_object3Flag = nullptr;
	qdMinigameObjectInterface *_loadGameObj = nullptr;

	bool _wasInited = false;
	bool _isFinal = false;
	bool _flipped = false;

	float _minDepthPiece = 0.0;
	int _rotatingPiece = -1;
	bool _pieceIsPut = true;
	int _currentPieceState = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_MASKY_ORDER_H
