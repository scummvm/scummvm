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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/game.h"
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

TeInputMgr::TeInputMgr() {
}

void TeInputMgr::handleEvent(const Common::Event &e) {
	switch (e.type) {
		case Common::EVENT_KEYDOWN:
			_keyDownSignal.call(e.kbd);
			break;
		case Common::EVENT_KEYUP:
			_keyUpSignal.call(e.kbd);
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseMoveSignal.call(e.mouse);
			_lastMousePos = e.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseLDownSignal.call(e.mouse);
			_lastMousePos = e.mouse;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseLUpSignal.call(e.mouse);
			_lastMousePos = e.mouse;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseRDownSignal.call(e.mouse);
			_lastMousePos = e.mouse;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseRUpSignal.call(e.mouse);
			_lastMousePos = e.mouse;
			break;
		case Common::EVENT_MAINMENU:
			g_engine->getGame()->_returnToMainMenu = true;
			break;
		default:
			break;
	}
}

} // end namespace Tetraedge
