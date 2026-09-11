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

#ifndef ULTIMA1_VIEWS_SPACE_STATS_H
#define ULTIMA1_VIEWS_SPACE_STATS_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

/**
 * The outer-space corner readout - the current ship's Shld and Fuel, plus
 * the character's Exp. and Coin. The space counterpart of Stats (which
 * shows Hits/Food instead)
 */
class SpaceStats : public Shared::UIElement {
public:
	SpaceStats(UIElement *parent) : Shared::UIElement("Stats", parent) {}
	~SpaceStats() override {}

	void draw() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
