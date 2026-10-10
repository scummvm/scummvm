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

#include "ultima/ultima3/views/interactions/ignite_torch.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte TORCH_LIGHT = 0xFF;

bool IgniteTorch::keypress(const KeypressMessage &msg) {
	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	Data::RosterEntry &e = _G(savegame).partyMember(_chooser.slot());
	if (e._torches == 0) {
		_G(messages).print("None Left!\n");
		g_engine->playSoundEffect(0xFE);
		return true;
	}

	e._torches = Data::toBcd(Data::fromBcd(e._torches) - 1);
	_G(savegame)._lightTurns = TORCH_LIGHT;
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
