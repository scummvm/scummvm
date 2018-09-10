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

#include "ultima/ultima1/u1dialogs/drop.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Drop, Dialog)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(TextInputMsg)
END_MESSAGE_MAP()

Drop::Drop(Ultima1Game *game) : Dialog(game), _mode(SELECT) {
	// The dialog itself doesn't initially display, instead we add a prompt to the info area for
	// what kind of thing to drop
}

void Drop::draw() {
	return;
}

bool Drop::ShowMsg(CShowMsg &msg) {
	// Add a prompt in the info area for what kind of thing to drop
	addInfoMsg(getGame()->_res->DROP_PENCE_WEAPON_ARMOR);
	Shared::CInfoGetKeypress keyMsg(this);
	keyMsg.execute(_game);

	return true;
}

bool Drop::TextInputMsg(CTextInputMsg &msg) {
	if (msg._escaped) {
		addInfoMsg(Common::String::format("%s %s", _game->_res->ACTION_NAMES[3], _game->_res->NOTHING), true, true);
		hide();
		delete this;
	}

	return true;
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
