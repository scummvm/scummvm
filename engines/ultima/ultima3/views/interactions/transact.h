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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_TRANSACT_H
#define ULTIMA3_VIEWS_INTERACTIONS_TRANSACT_H

#include "common/ptr.h"
#include "ultima/ultima3/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Has a party member speak to someone next to them, or deal with the shop
 * whose counter they're facing
 */
class Transact : public Interaction {
private:
	enum Stage {
		CHOOSE_PLAYER,
		CHOOSE_DIRECTION,
		IN_SHOP
	};

	Stage _stage = CHOOSE_PLAYER;
	PlayerChooser _players;
	DirectionChooser _directions;
	Common::ScopedPtr<Interaction> _shop;

	/**
	 * Says what someone in the town has to say
	 */
	void talkTo(int creature);

	/**
	 * Lord British lets those who have earned it grow stronger
	 */
	void lordBritish();

	/**
	 * Starts dealing with the shop whose counter is in a direction
	 * @returns		False if there isn't one
	 */
	bool startShop(int dx, int dy);

public:
	Transact() {}
	~Transact() override;

	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
