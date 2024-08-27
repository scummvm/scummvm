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

#ifndef QDENGINE_MINIGAMES_PUZZLE_ALL_H
#define QDENGINE_MINIGAMES_PUZZLE_ALL_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

const int puzzle_ep01[] = {
	220, 193, 340, 186, 464, 193, 577, 186, 227, 306,
	340, 306, 465, 305, 585, 306, 220, 419, 340, 426,
	464, 418, 577, 426
};

const int puzzle_ep02[] = {
	218, 197, 335, 194, 465, 178, 579, 181, 220, 305,
	347, 304, 473, 300, 582, 298, 222, 410, 342, 416,
	469, 426, 579, 427
};

const int puzzle_ep04[] = {
	205, 224, 298, 224, 404, 195, 503, 189, 601, 204,
	218, 319, 306, 308, 416, 287, 504, 263, 587, 285,
	218, 400, 313, 408, 418, 386, 504, 387, 588, 384
};

const int puzzle_ep05[] = {
	205, 223, 297, 223, 404, 194, 503, 189, 601, 203,
	217, 316, 305, 306, 415, 287, 503, 262, 586, 284,
	217, 397, 312, 405, 417, 384, 503, 384, 587, 382
};

const int puzzle_ep07[] = {
	199, 188, 276, 185, 362, 177, 444, 172, 531, 185,
	601, 183, 202, 263, 282, 258, 362, 245, 440, 248,
	524, 254, 598, 265, 200, 342, 282, 341, 357, 342,
	439, 341, 519, 344, 595, 340, 203, 423, 276, 420,
	359, 425, 441, 421, 525, 419, 602, 414
};

const int puzzle_ep08[] = {
	217, 188, 337, 181, 462, 188, 575, 181, 224, 302,
	338, 301, 462, 301, 582, 301, 217, 415, 337, 422,
	462, 414, 575, 421
};

const int puzzle_ep13[] = {
	196, 194, 273, 194, 362, 169, 445, 164, 538, 188,
	606, 188, 197, 266, 283, 255, 365, 231, 438, 235,
	533, 246, 593, 271, 197, 345, 283, 341, 354, 344,
	437, 341, 516, 348, 592, 340, 197, 425, 270, 424,
	357, 430, 440, 427, 526, 416, 601, 412
};

class qdPuzzleAllMiniGame : public qdMiniGameInterface {
public:
	qdPuzzleAllMiniGame(Common::String dll, Common::Language language) : _dll(dll), _language(language) {
		for (uint i = 0; i < ARRAYSIZE(_pieces); i++)
			_pieces[i] = nullptr;
	}
	~qdPuzzleAllMiniGame() {};

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "PuzzleAll::init(), dll: %s lang: %d", _dll.c_str(), _language);

		_engine = engine_interface;
		_scene = _engine->current_scene_interface();
		if (!_scene)
			return 0;

		if (_dll == "DLL\\Puzzle_ep01.dll") { // worm
			_numPieces = 12;
			_pieceCoords = puzzle_ep01;
		} else if (_dll == "DLL\\Puzzle_ep02.dll") { // mushrooms
			_numPieces = 12;
			_pieceCoords = puzzle_ep02;
		} else if (_dll == "DLL\\Puzzle_ep04.dll") { // bird
			_numPieces = 15;
			_pieceCoords = puzzle_ep04;
		} else if (_dll == "DLL\\Puzzle_ep05.dll") { // forest
			_numPieces = 15;
			_pieceCoords = puzzle_ep05;
		} else if (_dll == "DLL\\Puzzle_ep07.dll") { // bears
			_numPieces = 24;
			_pieceCoords = puzzle_ep07;
		} else if (_dll == "DLL\\Puzzle_ep08.dll") { // house
			_numPieces = 12;
			_pieceCoords = puzzle_ep08;
		} else if (_dll == "DLL\\Puzzle_ep13.dll") { // harbor
			_numPieces = 24;
			_pieceCoords = puzzle_ep13;
		}

		for (int i = 0; i < _numPieces; i++)
			_pieces[i] = _scene->object_interface(Common::String::format("\xee\xe1\xfa\xe5\xea\xf2_%i", i + 1).c_str());	// "объект_%i"

		_objFinal = _scene->object_interface("$\xf4\xe8\xed\xe0\xeb"); // "$финал"
		_objRan = _scene->object_interface("$\xe7\xe0\xef\xf3\xf1\xf2\xe8\xeb\xe8");	// "$запустили""
		_wasInited = false;

		if (_objFinal->is_state_active("\xed\xe5\xf2")) { // "нет"
			_isFinal = false;

			_minDepthPiece = findMinDepthPiece();
			return true;
		} else {
			if (_objFinal->is_state_active("\xe4\xe0"))	// "да"
				_isFinal = true;

			_minDepthPiece = findMinDepthPiece();
			return true;
		}

		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		debugC(3, kDebugMinigames, "PuzzleAll::quant(%f)", dt);

		if (!_wasInited && _objRan->is_state_active("\xed\xe5\xf2")) {	// "нет"
			_rotatingPiece = -1;
			_currentPieceState = -1;
			_pieceIsPut = false;
			_isFinal = false;
			_wasInited = 1;
			_objRan->set_state("\xe4\xe0");	// "да"
		}

		if (_isFinal)
			return true;

		if (!checkSolution() || _scene->mouse_object_interface()) {
			qdMinigameObjectInterface *mouseObj = _scene->mouse_object_interface();

			if (_pieceIsPut) {
				for (int i = 0; i < _numPieces; i++) {
					if (_pieces[i]->is_state_active("to_inv_flag_0")
							|| _pieces[i]->is_state_active("to_inv_flag_90")
							|| _pieces[i]->is_state_active("to_inv_flag_180")
							|| _pieces[i]->is_state_active("to_inv_flag_270")) {

						if (_pieces[i]->is_state_active("to_inv_flag_0"))
							_currentPieceState = _pieces[i]->state_index("inv_0");
						else if (_pieces[i]->is_state_active("to_inv_flag_90"))
							_currentPieceState = _pieces[i]->state_index("inv_90");
						else if (_pieces[i]->is_state_active("to_inv_flag_180"))
							_currentPieceState = _pieces[i]->state_index("inv_180");
						else if (_pieces[i]->is_state_active("to_inv_flag_270"))
							_currentPieceState = _pieces[i]->state_index("inv_270");

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
					if (obj->is_state_active("inv_0"))
						obj->set_state("0");
					else if (obj->is_state_active("inv_90"))
						obj->set_state("90");
					else if (obj->is_state_active("inv_180"))
						obj->set_state("180");
					else if (obj->is_state_active("inv_270"))
						obj->set_state("270");

					_minDepthPiece -= 250.0;

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
				if (obj->is_state_active("inv_0"))
					obj->set_state("inv_270");
				else if (obj->is_state_active("inv_90"))
					obj->set_state("inv_0");
				else if (obj->is_state_active("inv_180"))
					obj->set_state("inv_90");
				else if (obj->is_state_active("inv_270"))
					obj->set_state("inv_180");
			}
		}

		return true;
	}

	bool checkSolution() {
		if (_scene->mouse_object_interface())
			return false;

		for (int i = 0; i < _numPieces; i++)
			if (!_pieces[0]->is_state_active("0"))
				return false;

		mgVect2i piecePos;

		for (int i = 0; i < _numPieces; i++) {
			piecePos = _pieces[i]->screen_R();

			if (ABS(_pieceCoords[i * 2 + 0] - piecePos.x) > 10 ||
				ABS(_pieceCoords[i * 2 + 1] - piecePos.y) > 10)
				return false;
		}

		_isFinal = true;
		_objFinal->set_state("\xe4\xe0");	// "да"

		_minDepthPiece -= 250.0;

		mgVect3f coords = _scene->screen2world_coords(_objFinal->screen_R(), _minDepthPiece);
		_objFinal->set_R(coords);

		return true;
	}

	void snapPieces() {
		mgVect2i piecePos;
		mgVect3f newPiecePos;

		for (int i = 0; i < _numPieces; i++) {
			piecePos = _pieces[i]->screen_R();
			float depth = _scene->screen_depth(_pieces[i]->R());

			if (_pieces[i]->is_state_active("0")) {
				if (ABS(_pieceCoords[i * 2 + 0] - piecePos.x) <= 10 &&
						ABS(_pieceCoords[i * 2 + 1] - piecePos.y) <= 10) {
					piecePos.x = _pieceCoords[i * 2 + 0];
					piecePos.y = _pieceCoords[i * 2 + 1];

					newPiecePos = _scene->screen2world_coords(piecePos, depth);
					_pieces[i]->set_R(newPiecePos);
				}
			}
		}
	}

	//! Деинициализация игры.
	bool finit() {
		debugC(1, kDebugMinigames, "PuzzleAll::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	/// Версия интерфейса игры, трогать не надо.
	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	float findMinDepthPiece() {
		float min = 100000.0;

		for (int i = 0; i < _numPieces; i++) {
			float depth = _scene->screen_depth(_pieces[i]->R());

			if (min > depth)
				min = depth;
		}

		return min;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_pieces[24];
	const int *_pieceCoords = nullptr;

	qdMinigameObjectInterface *_objFinal = nullptr;
	qdMinigameObjectInterface *_objRan = nullptr;

	bool _wasInited = false;
	bool _isFinal = false;

	float _minDepthPiece = 0.0;
	int _rotatingPiece = -1;
	bool _pieceIsPut = true;
	int _currentPieceState = 0;

	int _numPieces = 12;

	Common::String _dll;
	Common::Language _language;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_PUZZLE_ALL_H
