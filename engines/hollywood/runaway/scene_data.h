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

#ifndef HOLLYWOOD_RUNAWAY_SCENE_DATA_H
#define HOLLYWOOD_RUNAWAY_SCENE_DATA_H

#include "common/rect.h"

namespace Hollywood {

enum RunawayRoomId {
	kHospitalBedroom = 2020,
	kHospitalStoreroom = 2060,
	kHospitalBathroom = 2080,
	kMuseumOffice = 4030,
	kMuseumLaboratory = 4050,
	kMuseumHall = 4060,
	kMuseumWilly = 4070,
	kMuseumExhibition = 4090,
	kMuseumSecurity = 4100,
	kMuseumKeypad = 4140
};

struct RunawayHotspot {
	const char *name;
	int16 x, y;
	byte facing;
	uint16 action[2]; // Look, use; IDs from the original room's action table.
	byte approach[2];
};

struct RunawayItemAction {
	constexpr RunawayItemAction(uint16 roomId, byte itemId, byte hotspotId, uint16 actionId, bool walk = true) :
		room(roomId), item(itemId), hotspot(hotspotId), action(actionId), approach(walk) {}
	uint16 room;
	byte item, hotspot;
	uint16 action;
	bool approach;
};

struct RunawayResponse {
	uint16 room, action, cue;
	byte variation;
};

struct RunawayExit {
	uint16 room, action, destination;
	int16 x, y;
	bool upperFloor;
	bool door;
};

struct RunawayRoom {
	uint16 number, width, height;
	const RunawayHotspot *hotspots;
	uint hotspotCount;
	byte light[10][3];
	Common::Point entry;
	const RunawayResponse *responses;
	uint responseCount;
	const RunawayItemAction *itemActions;
	uint itemActionCount;
	const RunawayExit *exits;
	uint exitCount;
};

extern const RunawayRoom kRunawayScene2020;
extern const RunawayRoom kRunawayScene2060;
extern const RunawayRoom kRunawayScene2080;
extern const RunawayRoom kRunawayScene4030;
extern const RunawayRoom kRunawayScene4050;
extern const RunawayRoom kRunawayScene4060;
extern const RunawayRoom kRunawayScene4070;
extern const RunawayRoom kRunawayScene4090;
extern const RunawayRoom kRunawayScene4100;
extern const RunawayRoom kRunawayScene4140;
extern const RunawayRoom kRunawayScene5010;
extern const RunawayRoom kRunawayScene5020;
extern const RunawayRoom kRunawayScene5050;
extern const RunawayRoom kRunawayScene5060;
extern const RunawayRoom kRunawayScene5070;
extern const RunawayRoom kRunawayScene5080;
extern const RunawayRoom kRunawayScene5100;
extern const RunawayRoom kRunawayScene5110;
extern const RunawayRoom kRunawayScene5130;
extern const RunawayRoom kRunawayScene5140;
extern const RunawayRoom kRunawayScene5160;
extern const RunawayRoom kRunawayScene5170;

const RunawayRoom *findRunawayRoom(uint16 number);

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_DATA_H
