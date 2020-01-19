/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/u6gfx/game_view.h"
#include "ultima/shared/core/map.h"
#include "ultima/shared/gfx/info.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/gfx/drawing_support.h"
#include "ultima/ultima1/gfx/status.h"
#include "ultima/ultima1/gfx/viewport_dungeon.h"
#include "ultima/ultima1/gfx/viewport_map.h"
#include "ultima/ultima1/actions/move.h"
#include "ultima/ultima1/actions/climb.h"
#include "ultima/ultima1/actions/enter.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U6Gfx {

BEGIN_MESSAGE_MAP(GameView, Shared::Gfx::VisualContainer)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

GameView::GameView(TreeItem *parent) : Shared::Gfx::VisualContainer("GameView", Rect(0, 0, 320, 200), parent) {
	_info = nullptr;
	_background.load("newmagic.bmp");
	_actions[0] = new Actions::Move(this);
	_actions[1] = new Actions::Climb(this);
	_actions[2] = new Actions::Enter(this);
}

GameView::~GameView() {
	delete _info;
	for (int idx = 0; idx < 3; ++idx)
		delete _actions[idx];
}

void GameView::draw() {
	Shared::Gfx::VisualSurface s = getSurface();
	s.blitFrom(_background);
}

bool GameView::KeypressMsg(CKeypressMsg &msg) {
	if (msg._keyState.keycode == Common::KEYCODE_LEFT || msg._keyState.keycode == Common::KEYCODE_KP4) {
		Shared::CMoveMsg move(Shared::DIR_LEFT);
		move.execute(this);
	} else if (msg._keyState.keycode == Common::KEYCODE_RIGHT || msg._keyState.keycode == Common::KEYCODE_KP6) {
		Shared::CMoveMsg move(Shared::DIR_RIGHT);
		move.execute(this);
	} else if (msg._keyState.keycode == Common::KEYCODE_UP || msg._keyState.keycode == Common::KEYCODE_KP8) {
		Shared::CMoveMsg move(Shared::DIR_UP);
		move.execute(this);
	} else if (msg._keyState.keycode == Common::KEYCODE_DOWN || msg._keyState.keycode == Common::KEYCODE_KP2) {
		Shared::CMoveMsg move(Shared::DIR_DOWN);
		move.execute(this);
	} else if (msg._keyState.keycode == Common::KEYCODE_e) {
		Shared::CEnterMsg enter;
		enter.execute(this);
	} else if (msg._keyState.keycode == Common::KEYCODE_k) {
		Shared::CClimbMsg climb;
		climb.execute(this);
	} else {
		return false;
	}

	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
