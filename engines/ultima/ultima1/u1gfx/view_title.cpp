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

#include "ultima/ultima1/u1gfx/view_title.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewTitle, Shared::Gfx::VisualContainer)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

ViewTitle::ViewTitle(Shared::TreeItem *parent) : Shared::Gfx::VisualContainer("Title", Rect(0, 0, 320, 200), parent) {
}

ViewTitle::~ViewTitle() {
}

void ViewTitle::draw() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::VisualSurface s = getSurface();

	// Draw horizontal title lines
	for (int idx = 0; idx < 3; ++idx) {
		s.hLine(112, idx + 58, 200, 1);
		s.hLine(112, idx + 74, 200, 1);
	}

	// Write text
	s.writeString(game->_res->TITLE_MESSAGES[0], TextPoint(16, 8), game->_whiteColor);
	s.writeString(game->_res->TITLE_MESSAGES[1], TextPoint(8, 11), game->_whiteColor);
	s.writeString(game->_res->TITLE_MESSAGES[2], TextPoint(0, 21), game->_whiteColor);
}

bool ViewTitle::KeypressMsg(CKeypressMsg &msg) {
	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
