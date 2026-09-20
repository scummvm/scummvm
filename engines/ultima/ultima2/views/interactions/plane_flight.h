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


#ifndef ULTIMA2_VIEWS_INTERACTIONS_PLANE_FLIGHT_H
#define ULTIMA2_VIEWS_INTERACTIONS_PLANE_FLIGHT_H

#include "ultima/ultima2/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

/**
 * A launched airplane flying across the overworld: it keeps moving in its
 * current heading until an arrow key turns it, and 'L' lands it on grass
 */
class PlaneFlight : public Interaction {
private:
	int _dx = 0, _dy = -1;
	uint32 _lastStep = 0;

	void setHeading(int dx, int dy, const char *name);
	void land();

public:
	PlaneFlight();
	~PlaneFlight() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool tick() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
