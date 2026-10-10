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

#include "ultima/ultima3/views/interactions/peer_gem.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

PeerGem::~PeerGem() {
	_G(overview).hide();
}

bool PeerGem::keypress(const KeypressMessage &msg) {
	if (_viewing) {
		if (isModifierKey(msg.keycode))
			return false;

		_G(overview).hide();
		return true;
	}

	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	Data::RosterEntry &e = _G(savegame).partyMember(_chooser.slot());
	if (e._gems == 0) {
		_G(messages).print("None Left!\n");
		g_engine->playSoundEffect(0xFE);
		return true;
	}

	e._gems = Data::toBcd(Data::fromBcd(e._gems) - 1);

	if (_G(savegame)._location == Data::LOCATION_DUNGEON)
		_G(overview).showLevel();
	else
		_G(overview).showMap();

	_viewing = true;
	return false;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
