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
 * Foundation, In, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/core/quests.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {

Quests::Quests(Ultima1Game *game) {
	for (int idx = 0; idx < FLAGS_COUNT; ++idx)
		push_back(QuestFlag(game));
}

void Quests::synchronize(Common::Serializer &s) {
	for (uint idx = 0; idx < size(); ++idx)
		(*this)[idx].synchronize(s);
}

/*-------------------------------------------------------------------*/

void QuestFlag::synchronize(Common::Serializer &s) {
	s.syncAsByte(_state);
}

void QuestFlag::start() {
	_state = IN_PROGRESS;
}

void QuestFlag::complete() {
	if (isInProgress()) {
		_state = COMPLETED;

		Shared::CInfoMsg msg(_game->_res->QUEST_COMPLETED, true);
		msg.execute(_game);
		_game->playFX(5);
	}
}

} // End of namespace Ultima1
} // End of namespace Ultima
