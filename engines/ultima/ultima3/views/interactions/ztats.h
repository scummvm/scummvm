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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_ZTATS_H
#define ULTIMA3_VIEWS_INTERACTIONS_ZTATS_H

#include "common/array.h"
#include "ultima/ultima3/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Shows a party member's attributes and belongings in the message window a
 * few lines at a time, moving on at each key press
 */
class Ztats : public Interaction {
private:
	PlayerChooser _chooser;
	bool _showing = false;
	Common::Array<Common::String> _pages;
	Common::String _tail;
	uint _page = 0;

	/**
	 * Works out the text to be shown for a party member
	 */
	void buildPages(const Data::RosterEntry &e);

	void finish();

public:
	~Ztats() override;

	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
