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

#include "ultima/ultima1/u1gfx/view_game.h"
#include "ultima/shared/actions/huh.h"
#include "ultima/shared/actions/pass.h"
#include "ultima/shared/core/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/u1gfx/info.h"
#include "ultima/ultima1/u1gfx/status.h"
#include "ultima/ultima1/u1gfx/viewport_dungeon.h"
#include "ultima/ultima1/u1gfx/viewport_map.h"
#include "ultima/ultima1/actions/move.h"
#include "ultima/ultima1/actions/climb.h"
#include "ultima/ultima1/actions/enter.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewGame, Shared::Gfx::VisualContainer)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

ViewGame::ViewGame(TreeItem *parent) : Shared::Gfx::VisualContainer("Game", Rect(0, 0, 320, 200), parent), _frameCtr(0) {
	_info = new Info(this);
	_status = new Status(this);
	_viewportDungeon = new ViewportDungeon(this);

	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	_viewportMap = new ViewportMap(this);
	_actions[0] = new Actions::Move(this);
	_actions[1] = new Actions::Climb(this);
	_actions[2] = new Actions::Enter(this);
	_actions[3] = new Shared::Actions::Pass(this, game->_res->PASS);
	_actions[4] = new Shared::Actions::Huh(this, game->_res->HUH);
}

ViewGame::~ViewGame() {
	delete _info;
	delete _status;
	delete _viewportDungeon;
	delete _viewportMap;
	for (int idx = 0; idx < 5; ++idx)
		delete _actions[idx];
}

void ViewGame::draw() {
	Shared::Gfx::VisualSurface s = getSurface();

	if (_isDirty) {
		// Draw the overal frame
		s.clear();
		DrawingSupport ds(s);
		ds.drawGameFrame();
		drawIndicators();
		_isDirty = false;
	}

	_info->draw();
	if (_status->isDirty())
		_status->draw();

	Map::Ultima1Map *map = static_cast<Map::Ultima1Map *>(getGame()->getMap());
	switch (map->_mapType) {
	case Map::MAP_DUNGEON:
		_viewportDungeon->draw();
		break;
	default:
		_viewportMap->draw();
		break;
	}
}

void ViewGame::drawIndicators() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Map::Ultima1Map *map = static_cast<Map::Ultima1Map *>(game->getMap());

	Shared::Gfx::VisualSurface s = getSurface();
	DrawingSupport ds(s);

	if (map->_mapType == Map::MAP_DUNGEON) {
		// Draw the dungeon level indicator
		ds.drawRightArrow(TextPoint(15, 0));
		s.writeString(game->_res->DUNGEON_LEVEL, TextPoint(16, 0), game->_textColor);
		s.writeString(Common::String::format("%2d", map->getLevel()), TextPoint(23, 0), game->_textColor);
		ds.drawLeftArrow(TextPoint(26, 0));

		// Draw the current direction
		const char *dir = game->_res->DIRECTION_NAMES[map->getDirection() - 1];
		ds.drawRightArrow(TextPoint(16, 19));
		s.writeString("       ", TextPoint(17, 19), game->_textColor);
		s.writeString(dir, TextPoint(19 - (7 - strlen(dir)) / 2, 19), game->_textColor);
		ds.drawLeftArrow(TextPoint(24, 19));
	}
}

#define FRAME_REDUCTION_RATE 5

bool ViewGame::FrameMsg(CFrameMsg &msg) {
	if (_frameCtr == FRAME_REDUCTION_RATE) {
		// Ignore frame message at the start of passing reduced frame rate to child views
		return false;
	} else if (++_frameCtr == FRAME_REDUCTION_RATE) {
		msg.execute(this, nullptr, Shared::MSGFLAG_SCAN);
		_frameCtr = 0;
	}

	return true;
}

bool ViewGame::KeypressMsg(CKeypressMsg &msg) {
	switch (msg._keyState.keycode) {
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4: {
		Shared::CMoveMsg move(Shared::DIR_LEFT);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6: {
		Shared::CMoveMsg move(Shared::DIR_RIGHT);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8: {
		Shared::CMoveMsg move(Shared::DIR_UP);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2: {
		Shared::CMoveMsg move(Shared::DIR_DOWN);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_e: {
		Shared::CEnterMsg enter;
		enter.execute(this);
		break;
	}
	case Common::KEYCODE_k: {
		Shared::CClimbMsg climb;
		climb.execute(this);
		break;
	}
	case Common::KEYCODE_SPACE: {
		Shared::CPassMsg pass;
		pass.execute(this);
		break;
	}
	default: {
		Shared::CHuhMsg huh;
		huh.execute(this);
		break;
	}
	}

	// End of turn
	getGame()->endOfTurn();

	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
