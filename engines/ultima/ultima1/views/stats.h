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

#ifndef ULTIMA1_VIEWS_STATS_H
#define ULTIMA1_VIEWS_STATS_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

/**
 * The always-visible Hits/Food/Exp./Coin display in the corner of the
 * game screen. Shared between the overworld/town and dungeon views.
 */
class Stats : public Shared::UIElement {
public:
	Stats(UIElement *parent) : Shared::UIElement("Stats", parent) {}
	~Stats() override {}

	void draw() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
