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

#include "ultima/ultima2/views/interactions/offer_gold.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

OfferGold::OfferGold() : Interaction("OfferGold") {
}

bool OfferGold::msgKeypress(const KeypressMessage &msg) {
	// Any other key withdraws the offer
	if (msg.ascii < '0' || msg.ascii > '9') {
		close();
		writeString("\n");
		_G(logic)->resumeTurn();
		return true;
	}

	int digit = msg.ascii - '0';
	writeString("%d\n", digit);

	close();
	_G(logic)->completeOffer(digit);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
