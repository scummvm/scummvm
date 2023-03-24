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

#include "mm/mm1/maps/map_town.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void MapTown::blacksmith() {
	visitedBusiness();

	send(SoundMessage(
		STRING[Common::String::format("maps.map%.2u.blacksmith_inside", _mapIndex)],
		[]() {
			g_events->send("Blacksmith", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::inn() {
	visitedBusiness();

	send(SoundMessage(
		STRING[Common::String::format("maps.map%.2u.inn_inside", _mapIndex)],
		[]() {
			g_events->replaceView("Inn", true);
		}
	));
}

void MapTown::market() {
	visitedBusiness();

	send(SoundMessage(
		STRING[Common::String::format("maps.map%.2u.market_inside", _mapIndex)],
		[]() {
			g_events->send("Market", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::tavern() {
	visitedBusiness();

	send(SoundMessage(
		STRING["maps.tavern_inside"],
		[]() {
			g_events->send("Tavern", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::temple() {
	visitedBusiness();

	send(SoundMessage(
		STRING[Common::String::format("maps.map%.2u.temple_inside", _mapIndex)],
		[]() {
			g_events->send("Temple", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::training() {
	visitedBusiness();

	send(SoundMessage(
		STRING[Common::String::format("maps.map%.2u.training_inside", _mapIndex)],
		[]() {
			g_events->send("Training", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::showSign(const Common::String &msg) {
	send(SoundMessage(
		2, 1, STRING["maps.sign"],
		6, 2, msg
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
