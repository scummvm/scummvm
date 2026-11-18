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

enum MuseumRoomId {
	kMuseumOffice = 4030,
	kMuseumLaboratory = 4050,
	kMuseumHall = 4060,
	kMuseumExhibition = 4090,
	kMuseumSecurity = 4100,
	kMuseumKeypad = 4140
};

struct MuseumHotspot {
	const char *name;
	int16 x, y;
	byte facing;
	uint16 action[2]; // Look, use; IDs from the original room's action table.
	byte approach[2];
};

struct MuseumItemAction {
	uint16 room;
	byte item, hotspot;
	uint16 action;
};

struct MuseumResponse {
	uint16 room, action, cue;
	byte variation;
};

struct MuseumExit {
	uint16 room, action, destination;
	int16 x, y;
	bool upperFloor;
	bool door;
};

struct MuseumRoom {
	uint16 number, width, height;
	const MuseumHotspot *hotspots;
	uint hotspotCount;
	byte light[10][3];
	Common::Point entry;
	const MuseumResponse *responses;
	uint responseCount;
	const MuseumItemAction *itemActions;
	uint itemActionCount;
	const MuseumExit *exits;
	uint exitCount;
};

extern const MuseumRoom kRunawayScene4030;
extern const MuseumRoom kRunawayScene4050;
extern const MuseumRoom kRunawayScene4060;
extern const MuseumRoom kRunawayScene4090;
extern const MuseumRoom kRunawayScene4100;
extern const MuseumRoom kRunawayScene4140;

const MuseumRoom *findRunawayMuseumRoom(uint16 number);

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_DATA_H
