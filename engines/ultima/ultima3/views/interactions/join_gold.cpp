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

#include "ultima/ultima3/views/interactions/join_gold.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int MAX_GOLD = 9999;

bool JoinGold::keypress(const KeypressMessage &msg) {
	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	Data::Savegame &save = _G(savegame);
	int total = 0;
	for (int i = 0; i < save._partySize; ++i)
		total += Data::fromBcdWord(save.partyMember(i)._gold);

	if (total > MAX_GOLD) {
		_G(messages).print("No more room!\n");
		g_engine->playSoundEffect(0xFF);
		return true;
	}

	for (int i = 0; i < save._partySize; ++i)
		save.partyMember(i)._gold = 0;
	save.partyMember(_chooser.slot())._gold = Data::toBcdWord(total);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
