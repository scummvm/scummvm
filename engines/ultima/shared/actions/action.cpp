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

#include "ultima/shared/actions/action.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/gfx/visual_item.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {
namespace Actions {

Action::Action(TreeItem *parent) : TreeItem() {
	assert(parent);
	addUnder(parent);
}

Game *Action::getGame() {
	return static_cast<Game *>(TreeItem::getGame());
}

Maps::Map *Action::getMap() {
	return static_cast<Maps::Map *>(getGame()->getMap());
}

void Action::addInfoMsg(const Common::String &text, bool newLine, bool replaceLine) {
	CInfoMsg msg(text, newLine, replaceLine);
	msg.execute(getView());
}

void Action::playFX(uint effectId) {
	getGame()->playFX(effectId);
}

void Action::endOfTurn() {
	getGame()->endOfTurn();
}

} // End of namespace Actions
} // End of namespace Shared
} // End of namespace Ultima

