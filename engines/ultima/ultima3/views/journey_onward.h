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

#ifndef ULTIMA3_VIEWS_JOURNEY_ONWARD_H
#define ULTIMA3_VIEWS_JOURNEY_ONWARD_H

#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * Explains why the party can't set out yet: there isn't one, or none of its
 * members are fit to travel
 */
class JourneyOnward : public WindowView {
public:
	JourneyOnward() : WindowView("JourneyOnward") {}
	~JourneyOnward() override {}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
