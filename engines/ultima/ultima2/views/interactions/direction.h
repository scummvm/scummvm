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

#ifndef ULTIMA2_VIEWS_INTERACTIONS_DIRECTION_H
#define ULTIMA2_VIEWS_INTERACTIONS_DIRECTION_H

#include "ultima/ultima2/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

/**
 * Prompts for an arrow-key direction, then feeds it back into the active
 * Logic's attack(). Suspends the gameplay keymap while active so the
 * arrow keypress arrives raw instead of as a movement action
 */
class Direction : public Interaction {
private:
	uint32 _startTime = 0;

public:
	Direction();
	~Direction() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool tick() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
