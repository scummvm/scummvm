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
#include "ultima/shared/actions/pass.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/gfx/info.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/u1gfx/status.h"
#include "ultima/ultima1/u1gfx/viewport_dungeon.h"
#include "ultima/ultima1/u1gfx/viewport_map.h"
#include "ultima/ultima1/actions/move.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/gfx/bitmap.h"

namespace Ultima {
namespace Ultima1 {
namespace U6Gfx {

BEGIN_MESSAGE_MAP(GameView, Shared::Gfx::VisualContainer)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

GameView::GameView(TreeItem *parent) : Shared::Gfx::VisualContainer("GameView", Rect(0, 0, 320, 200), parent) {
	_info = nullptr;
	_status = new U1Gfx::Status(this);

	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	_actions[0] = new Actions::Move(this);
	//_actions[1] = new Actions::Climb(this);
	//_actions[2] = new Actions::Enter(this);
	_actions[3] = new Shared::Actions::Pass(this, game->_res->ACTION_NAMES[15]);
	loadBackground();
}

GameView::~GameView() {
	delete _info;
	delete _status;
	for (int idx = 0; idx < 4; ++idx)
		delete _actions[idx];
}

void GameView::loadBackground() {
	// Load in the Ultima 6 background
	Shared::Gfx::Bitmap pic;
	pic.load("paper.bmp");
	_background.copyFrom(pic);

	// The scroll area in Ultima 6 is too big for the Ultima 1 status area, so we first have to remove it
	// Erase bottom edge of scroll
	_background.blitFrom(_background, Common::Rect(8, 190, 160, 200), Common::Point(168, 190));

	// Erase right edge of scroll
	pic.create(8, 86);
	pic.blitFrom(_background, Common::Rect(312, 16, 320, 102), Common::Point(0, 0));
	_background.blitFrom(pic, Common::Point(312, 105));

	// Erase bottom right-corner of scroll
	pic.create(8, 12);
	pic.blitFrom(_background, Common::Rect(0, 188, 8, 200), Common::Point(0, 0));
	pic.flipHorizontally();
	_background.blitFrom(pic, Common::Point(312, 188));

	// Clear off the rest of the scroll
	byte bgColor = *(const byte *)_background.getBasePtr(8, 8);
	_background.fillRect(Common::Rect(8, 8, 312, 192), bgColor);

	// Render the status and info areas
	Scroll scroll;
	scroll.draw(_background, Common::Rect(247, 159, 320, 200));
	scroll.draw(_background, Common::Rect(0, 159, 255, 200));
}

void GameView::draw() {
	Shared::Gfx::VisualSurface s = getSurface();
	s.blitFrom(_background);

	_status->draw();
}

bool GameView::KeypressMsg(CKeypressMsg &msg) {
	switch (msg._keyState.keycode) {
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4: {
		Shared::CMoveMsg move(Shared::Maps::DIR_LEFT);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6: {
		Shared::CMoveMsg move(Shared::Maps::DIR_RIGHT);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8: {
		Shared::CMoveMsg move(Shared::Maps::DIR_UP);
		move.execute(this);
		break;
	}
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2: {
		Shared::CMoveMsg move(Shared::Maps::DIR_DOWN);
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
	default:
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------*/

GameView::Scroll::Scroll() {
	Shared::Gfx::Bitmap b;
	b.load("newmagic.bmp");
	copyFrom(b);
}

void GameView::Scroll::draw(Graphics::ManagedSurface &dest, const Common::Rect &r) {
	const byte bgColor = *(const byte *)getBasePtr(16, 16);

	// To allow for increasing and/or decreasing horizontal and vertical, we rendering into
	// the four corners, allowing overlapping in the source if needed to increase the width
	// Top left
	dest.transBlitFrom(*this, Common::Rect(0, 0, r.width() / 2 + 1, r.height() / 2 + 1),
		Common::Point(r.left, r.top), bgColor);
	// Top right
	dest.transBlitFrom(*this, Common::Rect(this->w - r.width() / 2, 0, this->w, r.height() / 2 + 1),
		Common::Point(r.left + r.width() / 2, r.top), bgColor);
	// Bottom left
	dest.transBlitFrom(*this, Common::Rect(0, this->h - r.height() / 2, r.width() / 2 + 1, this->h),
		Common::Point(r.left, r.top + r.height() / 2), bgColor);
	// Bottom right
	dest.transBlitFrom(*this, Common::Rect(this->w - r.width() / 2, this->h - r.height() / 2,
		this->w, this->h), Common::Point(r.left + r.width() / 2, r.top + r.height() / 2), bgColor);
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
