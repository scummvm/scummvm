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

			_invClickObj = _scene->object_interface("$inv_click_flag");
			_invDescObj = _scene->object_interface("%%inv_desc");
			_invDescCloseupObj = _scene->object_interface("%%inv_desc_closeup");
			_invActiveFlagObj = _scene->object_interface("$inv_active_flag");
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

	qdMinigameObjectInterface *_invClickObj = nullptr;
	qdMinigameObjectInterface *_invDescObj = nullptr;
	qdMinigameObjectInterface *_invDescCloseupObj = nullptr;
	qdMinigameObjectInterface *_invActiveFlagObj = nullptr;
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
	bool  _shveikIsMoving = false;
	float _time = 0;
	int _timeout = 0;
	mgVect2f _oldShveikPos;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_INV_POPUP_H
