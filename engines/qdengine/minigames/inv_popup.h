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

#ifndef QDENGINE_MINIGAMES_INV_POPUP_H
#define QDENGINE_MINIGAMES_INV_POPUP_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qdInvPopupMiniGame : public qdMiniGameInterface {
public:
	qdInvPopupMiniGame() {}
	~qdInvPopupMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "InvPopup::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		//_invClickObj = _scene->object_interface("$inv_click_flag");
		_invDescObj = _scene->object_interface("%inv_desc");
		_invDescCloseupObj = _scene->object_interface("%inv_desc_closeup");
		//_invActiveFlagObj = _scene->object_interface("$inv_active_flag");
		_blockPersObj = _scene->object_interface("\x24\xe1\xeb\xee\xea\xe8\xf0\xee\xe2\xea\xe0\x5f\xef\xe5\xf0\xf1\xee\xed\xe0\xe6\xe0");	// "$блокировка_персонажа"
		_blockPersFlagObj = _scene->object_interface("\x24\xe1\xeb\xee\xea\xe8\xf0\xee\xe2\xea\xe0\x5f\xef\xe5\xf0\xf1\xee\xed\xe0\xe6\xe0\x5f\xf4\xeb\xe0\xe3");	// "$блокировка_персонажа_флаг"

		_shveikObj = _scene->personage_interface("\xd8\xe2\xe5\xe9\xea");	// "Швейк"

		_shveikLookObj = _scene->object_interface("\x24\xf8\xe2\xe5\xe9\xea\x20\xf1\xec\xee\xf2\xf0\xe8\xf2");	// "$швейк смотрит"
		_shveikMoveObj = _scene->object_interface("\x25\xF3\xEF\xF0\xE0\xE2\xEB\xE5\xED\xE8\xE5\x20\xEE\xE6\xE8\xE2\xEB\xFF\xE6\xE5\xEC\x20\xD8\xE2\xE5\xE9\xEA\xE0");	// "%управление оживляжем Швейка"
		_shveikMoveEnabledObj = _scene->object_interface("\x25\xEE\xE6\xE8\xE2\xEB\xFF\xE6\x20\xD8\xE2\xE5\xE9\xEA\xE0\x20\xF0\xE0\xE7\xF0\xE5\xF8\xE5\xED");	// "%оживляж Швейка разрешен"

		_someFlag = false;
		_scene->activate_personage("\xd8\xe2\xe5\xe9\xea");	// "Швейк"

		// srand(time(0));
		_shveikIsMoving = 0;
		_time = -1.0;
		_timeout = -1;

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "InvPopup::quant(%f)", dt);

		if (_blockPersObj->is_state_active("\xe2\xea\xeb\xfe\xf7\xe8\xf2\xfc")) { // "включить"
			_scene->activate_personage("Lock");
		} else if (_blockPersObj->is_state_active("\xe2\xfb\xea\xeb\xfe\xf7\xe8\xf2\xfc")) { // "выключить"
			_scene->activate_personage("\xd8\xe2\xe5\xe9\xea");	// "Швейк"
			_blockPersObj->set_state("\xe2\xfb\xea\xeb\xfe\xf7\xe5\xed\xe0");	// "выключена"
		}

		qdMinigameObjectInterface *obj;
		char buf[5];
		const char *state = nullptr;

		if (_scene->mouse_hover_object_interface())
			state = _scene->mouse_hover_object_interface()->current_state_name();

		if (!_scene->mouse_hover_object_interface() || _scene->mouse_object_interface()) {
			_invDescObj->set_state("00");
			obj = _invDescCloseupObj;
			obj->set_state("00");
		} else if (!strstr(state, "#inv#")) {
			obj = _invDescObj;
			obj->set_state("00");
		} else {
			const char *pos = strstr(state, "#inv#");
			char buf2[5];
			Common::strlcpy(buf2, pos + 5, 3);
			Common::strlcpy(buf, _invDescCloseupObj->current_state_name(), 3);

			if (buf2[0] != buf[0] || buf2[1] != buf[1]) {
				_invDescObj->set_state(buf2);
				_hoverObjectPos = _scene->mouse_hover_object_interface()->screen_R();
				if (_hoverObjectPos.x + _invDescObj->screen_size().x / 2 >= 800
						|| (_hoverObjectPos.x - _invDescObj->screen_size().x / 2 <= 0)) {
					if (_hoverObjectPos.x + _invDescObj->screen_size().x / 2 < 800) {
						if (_hoverObjectPos.x - _invDescObj->screen_size().x / 2 <= 0)
							_invDescPos.x = _invDescObj->screen_size().x / 2 + 10;
						} else {
							_invDescPos.x = 790 - _invDescObj->screen_size().x / 2;
						}
					} else {
						_invDescPos.x = _hoverObjectPos.x;
				}
				_invDescPos.y = _invDescObj->screen_size().y / 2 + 73;
				_invDescObj->set_R(_scene->screen2world_coords(_invDescPos, -1000.0));
				_invDescCloseupObj->set_state("00");
			}
		}

		if (_scene->mouse_right_click_object_interface()) {
			if (!_scene->mouse_object_interface()) {
				state = _scene->mouse_hover_object_interface()->current_state_name();

				if (strstr(state, "#closeup#")) {
					const char *pos = strstr(state, "#inv#");
					Common::strlcpy(buf, pos + 5, 3);
					buf[2] = 0;

					_invDescCloseupObj->set_state(buf);
					_invDescPos.y = 300;
					_invDescPos.x = 400;
					_invDescCloseupObj->set_R( _scene->screen2world_coords(_invDescPos, -1000.0));
					_invDescObj->set_state("00");
				}
			}
		}

		if (Common::String(_blockPersFlagObj->current_state_name()) == "\xe4\xe0") { // "да"
			if (!_blockPersObj->is_state_active("\xe2\xea\xeb\xfe\xf7\xe8\xf2\xfc")) // "включить"
				_blockPersObj->set_state("\xe2\xea\xeb\xfe\xf7\xe8\xf2\xfc"); // "включить"
		} else if (_blockPersObj->is_state_active("\xe2\xea\xeb\xfe\xf7\xe8\xf2\xfc")) { // "включить"
			_blockPersObj->set_state("\xe2\xfb\xea\xeb\xfe\xf7\xe8\xf2\xfc"); // "выключить"
		}

		_direction = -10;
		int angle = (int)(_shveikObj->direction_angle() * 180.0 * 0.3183098865475127);

		if (angle > 220 && angle < 230)
			_direction = 1;
		else if (angle > 265 && angle < 275)
			_direction = 2;
		else if (angle > 310 && angle < 320)
			_direction = 3;
		else if (angle > 175 && angle < 185)
			_direction = 4;
		else if (angle >= 0 && angle < 5)
			_direction = 6;
		else if (angle > 40 && angle < 50)
			_direction = 9;
		else if (angle > 85 && angle < 95)
			_direction = 8;
		else if (angle > 130 && angle < 140)
			_direction = 7;

		Common::String curState = _shveikObj->current_state_name();
		if (curState == "#1#") {
			_direction = 1;
		} else if (curState == "#2#") {
			_direction = 2;
		} else if (curState == "#3#") {
			_direction = 3;
		} else if (curState == "#4#") {
			_direction = 4;
		} else if (curState == "#6#") {
			_direction = 6;
		} else if (curState == "#7#") {
			_direction = 7;
		} else if (curState == "#8#") {
			_direction = 8;
		} else if (curState == "#9#") {
			_direction = 9;
		}

		switch (_direction) {
		case -10:
			_shveikLookObj->set_state("0");
			break;
		case 1:
			_shveikLookObj->set_state("1");
			break;
		case 2:
			_shveikLookObj->set_state("2");
			break;
		case 3:
			_shveikLookObj->set_state("3");
			break;
		case 4:
			_shveikLookObj->set_state("4");
			break;
		case 6:
			_shveikLookObj->set_state("6");
			break;
		case 7:
			_shveikLookObj->set_state("7");
			break;
		case 8:
			_shveikLookObj->set_state("8");
			break;
		case 9:
			_shveikLookObj->set_state("9");
			break;
		default:
			break;
		}

		if (curState == "?1?") {
			_direction = 10;
		} else if (curState == "?2?") {
			_direction = 20;
		} else if (curState == "?3?") {
			_direction = 30;
		} else if (curState == "?4?") {
			_direction = 40;
		} else if (curState == "?6?") {
			_direction = 60;
		} else if (curState == "?7?") {
			_direction = 70;
		} else if (curState == "?8?") {
			_direction = 80;
		} else if (curState == "?9?") {
			_direction = 90;
		}

		switch (_direction) {
		case 10:
			_shveikObj->set_direction_angle(5 * M_PI / 4);
			break;
		case 20:
			_shveikObj->set_direction_angle(6 * M_PI / 4);
			break;
		case 30:
			_shveikObj->set_direction_angle(7 * M_PI / 4);
			break;
		case 40:
			_shveikObj->set_direction_angle(M_PI);
			break;
		case 60:
			_shveikObj->set_direction_angle(0.0);
			break;
		case 70:
			_shveikObj->set_direction_angle(3 * M_PI / 4);
			break;
		case 80:
			_shveikObj->set_direction_angle(2 * M_PI / 4);
			break;
		case 90:
			_shveikObj->set_direction_angle(1 * M_PI / 4);
			break;
		default:
			break;
		}

		if (strstr(_shveikMoveEnabledObj->current_state_name(), "\xe4\xe0")) { // "да"
			if (!_shveikIsMoving) {
				_shveikIsMoving = 1;
				_time = 0.0;
				_timeout = qd_rnd(10) + 5;
			}

			if (_timeout >= _time || _time == -1.0) {
				_time = dt + _time;
			} else {
				_shveikIsMoving = 2;
				_time = -1.0;
				_shveikMoveObj->set_state("\xe2\xfb\xef\xee\xeb\xed\xe8\xf2\xfc"); // "выполнить"
			}
		}

		if ((strstr(_shveikMoveEnabledObj->current_state_name(), "\xed\xe5\xf2")	// "нет"
				|| _shveikMoveObj->is_state_active("\xee\xf2\xf0\xe0\xe1\xee\xf2\xe0\xed"))		// "отработан"
				&& _shveikIsMoving > 0) {
			_shveikIsMoving = 0;
			_time = -1.0;
			_timeout = -1;
			_shveikMoveObj->set_state("\xee\xe6\xe8\xe4\xe0\xed\xe8\xe5"); // "ожидание"
		}

		if ((_shveikObj->R().x != _oldShveikPos.x
				|| _shveikObj->R().y != _oldShveikPos.y)
				&& _shveikIsMoving > 0) {
			_shveikIsMoving = 0;
			_time = -1.0;
			_timeout = -1;
			_shveikMoveObj->set_state("\xee\xe6\xe8\xe4\xe0\xed\xe8\xe5"); // "ожидание"
		}

		_oldShveikPos = _shveikObj->R();

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "InvPopup::finit()");

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

//	qdMinigameObjectInterface *_invClickObj = nullptr;
	qdMinigameObjectInterface *_invDescObj = nullptr;
	qdMinigameObjectInterface *_invDescCloseupObj = nullptr;
//	qdMinigameObjectInterface *_invActiveFlagObj = nullptr;
	qdMinigameObjectInterface *_shveikObj = nullptr;
	qdMinigameObjectInterface *_blockPersObj = nullptr;
	qdMinigameObjectInterface *_blockPersFlagObj = nullptr;
	qdMinigameObjectInterface *_shveikMoveObj = nullptr;
	qdMinigameObjectInterface *_shveikMoveEnabledObj = nullptr;
	qdMinigameObjectInterface *_shveikLookObj = nullptr;

	mgVect2i _hoverObjectPos;
	mgVect2i _invDescPos;
	bool _someFlag = false;
	int _direction = 0;
	int _shveikIsMoving = 0;
	float _time = 0;
	int _timeout = 0;
	mgVect3f _oldShveikPos;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_INV_POPUP_H
