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

#ifndef QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H
#define QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdArkadaAvtomatMiniGame : public qdMiniGameInterface {
public:
	qdArkadaAvtomatMiniGame() {}
	~qdArkadaAvtomatMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "ArkadaAvtomat::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_patronMouseObj  = _scene->object_interface(_scene->minigame_parameter("patron_mouse"));
		_patronTomatoObj = _scene->object_interface(_scene->minigame_parameter("patron_tomato"));
		_killObj         = _scene->object_interface(_scene->minigame_parameter("kill"));
		_fazaObj         = _scene->object_interface(_scene->minigame_parameter("faza"));
		_menObj          = _scene->object_interface(_scene->minigame_parameter("men"));
		_bloodObj        = _scene->object_interface(_scene->minigame_parameter("blood"));
		_shotsEggObj     = _scene->object_interface(_scene->minigame_parameter("shots_egg"));
		_shotsTomatoObj  = _scene->object_interface(_scene->minigame_parameter("shots_tomato"));
		_shotsBananObj   = _scene->object_interface(_scene->minigame_parameter("shots_banan"));
		_doneObj         = _scene->object_interface(_scene->minigame_parameter("done"));
		_livesObj        = _scene->object_interface(_scene->minigame_parameter("lives"));
		_bublObj         = _scene->object_interface(_scene->minigame_parameter("bubl"));
		_walkFlagObj     = _scene->object_interface(_scene->minigame_parameter("walk_flag"));
		_jumpFlagObj     = _scene->object_interface(_scene->minigame_parameter("jump_flag"));

		_goingLeft = false;
		_goingRight = false;
		_manSpeed = 2;
		_doingJump = false;
		_stepsToJump = -1;

		// stand(time(0));

		_jumpNextPhase = -1;
		_shotsTomatoCounter = 9;
		_shotsBananaCounter = 10;
		_shoteEggCounter = 10;
		_livesCounter = 5;
		_isFinal = false;
		_jumpPhase = 2;
		_doingWalk = false;
		_jumpFlag = false;

   		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "ArkadaAvtomat::quant(%f)", dt);

		mgVect2i menCoords;
		int randomCond = 0;

		menCoords = _scene->world2screen_coords(_menObj->R());
		_bublObj->set_R(_scene->screen2world_coords(menCoords, -5000.0));

		if (_isFinal) {
			updateWalkState();
			return true;
		}

		if (!_shotsTomatoCounter && !_shotsBananaCounter && !_shoteEggCounter) {
			_shotsTomatoCounter = 9;
			_shotsBananaCounter = 10;
			_shoteEggCounter = 10;
			_livesCounter = 5;

			_patronMouseObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");	// "помидор"

			updateStats();

			_manSpeed = 2;
		}

		if (!_doingJump) {
			if (_scene->world2screen_coords(_menObj->R()).x < _jumpingPhase3Y
					&& _menObj->is_state_active("\xf1\xf2\xee\xe8\xf2")) { // "стоит"
				_menObj->set_R(_menCoords);

				_bloodObj->set_R(_scene->screen2world_coords(_scene->world2screen_coords(_menCoords), 0.0)); // check 0.0 v96
				_stepsToJump = -1;
			}
		}

		if (_menObj->is_state_active("\xf1\xf2\xee\xe8\xf2")) { // "стоит"
			_goingLeft = false;
			_goingRight = false;
			_doingJump = false;
		}

		if (!_goingRight) {
			if (_goingLeft || _doingJump || _menObj->is_state_active("\xf1\xf2\xee\xe8\xf2")) { // "стоит"
LABEL_44:
				if (!_goingRight)
					goto LABEL_47;
				goto LABEL_45;
			}
			_stepsToLeft = -1;
			_stepsToRight = -1;

			switch (qd_rnd(5)) {
			case 0:
			case 3: {
					_goingRight = true;
					int maxSteps = (500 - _menObj->screen_R().x) / _manSpeed;

					if (maxSteps < 10)
						maxSteps = 10;

					_stepsToRight = qd_rnd(maxSteps);
					_menObj->set_state("\xe8\xe4\xe5\xf2 \xe2\xef\xf0\xe0\xe2\xee"); // "идет вправо"
					goto LABEL_44;
				}
			case 1:
			case 4: {
					_goingLeft = true;
					int maxSteps = (_menObj->screen_R().x - 300) / _manSpeed;
					if (maxSteps < 10)
						maxSteps = 10;

					_stepsToLeft = qd_rnd(maxSteps);
					_menObj->set_state("\xe8\xe4\xe5\xf2 \xe2\xeb\xe5\xe2\xee"); // "идет влево"
					goto LABEL_44;
			}
			case 2:
				_doingJump = true;
				_stepsToJump = 30;
				_menCoords = _menObj->R();
				{
					int y = _scene->world2screen_coords(_menObj->R()).y;

					_manOrigPosY = y;
					_jumpingPhase3Y = y;
					_jumpHeight = y - 60;
				}

				randomCond = qd_rnd(3);

				switch (_jumpPhase) {
				case 1:
					if (!randomCond) {
						_jumpingPhase3Y = _manOrigPosY;
						goto LABEL_43;
					}
					if (randomCond == 1)
						goto LABEL_29;
					if (randomCond != 2)
						goto LABEL_43;
					break;
				case 2:
					if (!randomCond) {
						_jumpingPhase3Y = _manOrigPosY;
						goto LABEL_43;
					}
					if (randomCond == 1) {
						_jumpingPhase3Y = 307;
						_jumpNextPhase = 1;
						goto LABEL_43;
					}
					if (randomCond != 2)
						goto LABEL_43;
					break;
				case 3:
					if (!randomCond) {
						_jumpingPhase3Y = _manOrigPosY;
						goto LABEL_43;
					}
					if (randomCond != 1) {
						if (randomCond == 2) {
							_jumpingPhase3Y = 307;
							_jumpNextPhase = 1;
						}
						goto LABEL_43;
					}
		LABEL_29:
					_jumpingPhase3Y = 332;
					_jumpNextPhase = 2;
					goto LABEL_43;
				default:
		LABEL_43:
					_jumpPhaseIsGoingDown = false;
					goto LABEL_44;
				}

				_jumpingPhase3Y = 357;
				_jumpNextPhase = 3;
				goto LABEL_43;
			default:
				goto LABEL_44;
			}
		}

LABEL_45:
		{
			mgVect2i pos = _scene->world2screen_coords(_menObj->R());
			pos.x += _manSpeed;
			_menObj->set_R(_scene->screen2world_coords(pos, 0.0));
		}
		_stepsToRight--;

		if (_stepsToRight <= 0)
			_goingRight = false;

LABEL_47:
		if (_goingLeft) {
			mgVect2i pos = _scene->world2screen_coords(_menObj->R());
			pos.x -= _manSpeed;
			_menObj->set_R(_scene->screen2world_coords(pos, 0.0));
			_stepsToLeft--;

			if (_stepsToLeft <= 0)
				_goingLeft = false;
		}

		if (_doingJump) {
			mgVect2i pos = _scene->world2screen_coords(_menObj->R());

			if (pos.y >= _manOrigPosY - 10) {
				if (!_menObj->is_state_active("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x31") && !_jumpPhaseIsGoingDown)		// "прыгает1"
					_menObj->set_state("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x31");	// "прыгает1"
			}

			if (pos.y <= _manOrigPosY - 10) {
				if (_menObj->is_state_active("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x31") && !_jumpPhaseIsGoingDown)		// "прыгает1"
					_menObj->set_state("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x32");							// "прыгает2"
			}

			if (pos.y >= _jumpingPhase3Y - 10) {
				if (_menObj->is_state_active("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x32") && _jumpPhaseIsGoingDown)			// "прыгает2"
					_menObj->set_state("\xef\xf0\xfb\xe3\xe0\xe5\xf2\x33");							// "прыгает3"
			}

			if (pos.y <= _jumpHeight)
				goto LABEL_176;

			if (!_jumpPhaseIsGoingDown) {
				pos.y -= 4;
LABEL_75:
LABEL_76:
				if (pos.y < _jumpingPhase3Y) {
LABEL_79:
					_menObj->set_R(_scene->screen2world_coords(pos, 0.0));
					goto LABEL_80;
				}
LABEL_77:
				if (_jumpPhaseIsGoingDown) {
					_doingJump = false;
					_jumpPhase = _jumpNextPhase;
				}
				goto LABEL_79;
			}
			if (pos.y > _jumpHeight) {
				if (!_jumpPhaseIsGoingDown)
					goto LABEL_76;
				} else {
LABEL_176:
				if (!_jumpPhaseIsGoingDown) {
					_jumpPhaseIsGoingDown = true;
					goto LABEL_76;
				}
			}
			if (pos.y > _jumpingPhase3Y)
				goto LABEL_77;

			pos.y += 4;
			goto LABEL_75;
		}
LABEL_80:

		_patronMouseObj->set_R(_scene->screen2world_coords(_engine->mouse_cursor_position(), -5000.0));

		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_RIGHT_DOWN)) {
			if (_patronMouseObj->is_state_active("\xef\xee\xec\xe8\xe4\xee\xf0")) {		// "помидор"
				if (_shoteEggCounter <= 0) {
					if (_shotsBananaCounter > 0) {
						_patronMouseObj->set_state("\xe1\xe0\xed\xe0\xed");				// "банан"

						if (_shotsTomatoCounter > 0)
							_shotsTomatoCounter++;

						--_shotsBananaCounter;
					}
					goto LABEL_107;
				}
				_patronMouseObj->set_state("\xff\xe9\xf6\xee");		// "яйцо"

				if (_shotsTomatoCounter > 0)
					_shotsTomatoCounter++;
			} else {
				if (_patronMouseObj->is_state_active("\xff\xe9\xf6\xee")) {				// "яйцо"
					if (_shotsBananaCounter <= 0) {
						if (_shotsTomatoCounter > 0) {
							_patronMouseObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");	// "помидор"

							if (_shoteEggCounter > 0)
								_shoteEggCounter++;

							--_shotsTomatoCounter;
						}
					} else {
						_patronMouseObj->set_state("\xe1\xe0\xed\xe0\xed");		// "банан"

						if (_shoteEggCounter > 0)
							_shoteEggCounter++;

						--_shotsBananaCounter;
					}
					goto LABEL_107;
				}

				if (!_patronMouseObj->is_state_active("\xe1\xe0\xed\xe0\xed"))		// "банан"
					goto LABEL_107;

				if (_shotsTomatoCounter > 0) {
					_patronMouseObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");	// "помидор"

					if (_shotsBananaCounter > 0)
						_shotsBananaCounter++;

					--_shotsTomatoCounter;
					goto LABEL_107;
				}
				if (_shoteEggCounter <= 0) {
LABEL_107:
					updateStats();
					goto LABEL_108;
				}
				_patronMouseObj->set_state("\xff\xe9\xf6\xee");		// "яйцо"

				if (_shotsBananaCounter > 0)
					_shotsBananaCounter = _shotsBananaCounter + 1;
			}
			--_shoteEggCounter;
			goto LABEL_107;
		}
LABEL_108:
		if (_engine->is_mouse_event_active(qdmg::qdEngineInterfaceImpl::MOUSE_EV_LEFT_DOWN)
				&& !_bloodObj->is_state_active("\xe5\xf1\xf2\xfc")						// "есть"
				&& !_patronTomatoObj->is_state_active("\xef\xee\xec\xe8\xe4\xee\xf0")	// "помидор"
				&& !_patronTomatoObj->is_state_active("\xff\xe9\xf6\xee")				// "яйцо"
				&& !_patronTomatoObj->is_state_active("\xe1\xe0\xed\xe0\xed")) {		// "банан"
			_cursorPos = _engine->mouse_cursor_position();

			_patronTomatoObj->set_R(_patronMouseObj->R());

			if (_patronMouseObj->is_state_active("\xef\xee\xec\xe8\xe4\xee\xf0")) {		// "помидор"
				_patronTomatoObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");			// "помидор"
				--_shotsTomatoCounter;
			} else if (_patronMouseObj->is_state_active("\xff\xe9\xf6\xee")) {		// "яйцо"
				_patronTomatoObj->set_state("\xff\xe9\xf6\xee");					// "яйцо"
				--_shoteEggCounter;
			} else if (_patronMouseObj->is_state_active("\xe1\xe0\xed\xe0\xed")) {		// "банан"
				_patronTomatoObj->set_state("\xe1\xe0\xed\xe0\xed");					// "банан"
				--_shotsBananaCounter;
			}

			_bloodObj->set_R(_patronMouseObj->R());
			updateStats();

			if (_shotsTomatoCounter <= 0
					&& _patronTomatoObj->is_state_active("\xef\xee\xec\xe8\xe4\xee\xf0")) {	// "помидор"
				if (_shoteEggCounter <= 0) {
					if (_shotsBananaCounter <= 0)
						_patronMouseObj->set_state("\xed\xe8\xf7\xe5\xe3\xee");			// "ничего"
					else
						_patronMouseObj->set_state("\xe1\xe0\xed\xe0\xed");					// "банан"
				} else {
					_patronMouseObj->set_state("\xff\xe9\xf6\xee");							// "яйцо"
				}
			}
			if (_shoteEggCounter <= 0 && _patronTomatoObj->is_state_active("\xff\xe9\xf6\xee")) {	// "яйцо"
				if (_shotsBananaCounter <= 0) {
					if (_shotsTomatoCounter <= 0)
						_patronMouseObj->set_state("\xed\xe8\xf7\xe5\xe3\xee");		// "ничего"
					else
						_patronMouseObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");	// "помидор"
				} else {
					_patronMouseObj->set_state("\xe1\xe0\xed\xe0\xed");				// "банан"
				}
			}
			if (_shotsBananaCounter <= 0 && _patronTomatoObj->is_state_active("\xe1\xe0\xed\xe0\xed")) {	// "банан"
				if (_shotsTomatoCounter <= 0) {
					if (_shoteEggCounter <= 0)
						_patronMouseObj->set_state("\xed\xe8\xf7\xe5\xe3\xee");	// "ничего"
					else
						_patronMouseObj->set_state("\xff\xe9\xf6\xee");			// "яйцо"
				} else {
					_patronMouseObj->set_state("\xef\xee\xec\xe8\xe4\xee\xf0");	// "помидор"
				}
			}
		}

		// Hit check
		if (_fazaObj->is_state_active("\xe4\xe0")		// "да"
				&& !_bloodObj->is_state_active("\xe5\xf1\xf2\xfc")) {	// "есть"
			mgVect2i pos = _menObj->screen_R();

			if (ABS(pos.x - _cursorPos.x) <= 15 && ABS(pos.y - _cursorPos.y) <= 35) {
				_killObj->set_state("\xe4\xe0");		// "да"
				--_livesCounter;

				updateStats();

				_doingJump = false;
				_manSpeed = 7 - _livesCounter;

				if (_manSpeed > 4)
					_manSpeed = 4;
			}
		}

		if (!_livesCounter) {
			_isFinal = true;
			_doneObj->set_state("\xe4\xe0");		// "да"
			_goingLeft = false;
			_goingRight = false;
			_doingJump = false;
		}

		updateWalkState();

		return true;
	}

	void updateWalkState() {
		if ((_goingLeft || _goingRight) && !_doingWalk) {
			_doingWalk = true;
			_walkFlagObj->set_state("\xe4\xe0");		// "да"

			_jumpFlag = false;
			_jumpFlagObj->set_state("\xed\xe5\xf2");	// "нет"

			return;
		}

		if (_doingJump) {
			_doingWalk = false;
			_walkFlagObj->set_state("\xed\xe5\xf2");	// "нет"

			_jumpFlag = true;
			_jumpFlagObj->set_state("\xe4\xe0");		// "да"

			return;
		}


		_doingWalk = false;
		_walkFlagObj->set_state("\xed\xe5\xf2");	// "нет"

		_jumpFlag = false;
		_jumpFlagObj->set_state("\xed\xe5\xf2");	// "нет"
	}

	bool finit() {
		debugC(1, kDebugMinigames, "ArkadaAvtomat::finit()");

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
	void  updateStats() {
		_shotsEggObj->set_state(Common::String::format("%i", _shoteEggCounter).c_str());
		_shotsBananObj->set_state(Common::String::format("%i", _shotsBananaCounter).c_str());
		_shotsTomatoObj->set_state(Common::String::format("%d", _shotsTomatoCounter).c_str());
		_livesObj->set_state(Common::String::format("%d", _livesCounter).c_str());
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_patronMouseObj;
	qdMinigameObjectInterface *_patronTomatoObj;
	qdMinigameObjectInterface *_killObj;
	qdMinigameObjectInterface *_fazaObj;
	qdMinigameObjectInterface *_menObj;
	qdMinigameObjectInterface *_bloodObj;
	qdMinigameObjectInterface *_shotsEggObj;
	qdMinigameObjectInterface *_shotsTomatoObj;
	qdMinigameObjectInterface *_shotsBananObj;
	qdMinigameObjectInterface *_doneObj;
	qdMinigameObjectInterface *_livesObj;
	qdMinigameObjectInterface *_bublObj;
	qdMinigameObjectInterface *_walkFlagObj;
	qdMinigameObjectInterface *_jumpFlagObj;

	bool _goingLeft = false;
	bool _goingRight = false;
	bool _doingJump = false;
	bool _doingWalk = false;
	bool _jumpFlag = false;

	int _stepsToLeft = 0;
	int _stepsToRight = 0;
	int _stepsToJump = -1;

	mgVect3f _menCoords;
	int _manSpeed = 2;

	int _shotsTomatoCounter = 9;
	int _shotsBananaCounter = 10;
	int _shoteEggCounter = 10;
	int _livesCounter = 5;

	bool _isFinal = false;

	mgVect2i _cursorPos;

	int _manOrigPosY = 0;
	int _jumpHeight = 0;
	int _jumpingPhase3Y = 0;

	bool _jumpPhaseIsGoingDown = false;

	int _jumpPhase = 2;
	int _jumpNextPhase = -1;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ARKADA_AVTOMAT_H
