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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_OTHER_COMMAND_H
#define ULTIMA3_VIEWS_INTERACTIONS_OTHER_COMMAND_H

#include "ultima/ultima3/views/interactions/log_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Has a party member carry out a command typed in as a word, such as digging
 * or searching
 */
class OtherCommand : public Interaction {
private:
	enum Stage {
		CHOOSE_PLAYER,
		TYPE_COMMAND,
		DIRECTION
	};

	Stage _stage = CHOOSE_PLAYER;
	PlayerChooser _players;
	LogText _text;
	DirectionChooser _directions;

	void dig();
	void search();
	void pray();
	bool bribe(Direction dir);

public:
	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
