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

#include "ultima/ultima3/views/interactions/exchange.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

bool Exchange::keypress(const KeypressMessage &msg) {
	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	if (_first < 0) {
		_first = _chooser.slot();
		_G(messages).print("Player: ");
		return false;
	}

	Data::Savegame &save = _G(savegame);
	SWAP(save._partyEntries[_first], save._partyEntries[_chooser.slot()]);
	_G(messages).print("Exchanged!\n");
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
