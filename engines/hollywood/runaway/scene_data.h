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
	constexpr RunawayHotspot() : x(0), y(0), facing(0), action{0, 0}, approach{0, 0}, actionCursor(0), preferLook(false) {}
	constexpr RunawayHotspot(int16 xPos, int16 yPos, byte direction, const uint16 (&actions)[2],
			const int8 (&approaches)[2], Common::Point lookAt, byte cursor = 0, bool lookFirst = false) :
		x(xPos), y(yPos), facing(direction), action{actions[0], actions[1]},
		approach{approaches[0], approaches[1]}, lookPosition(lookAt), actionCursor(cursor), preferLook(lookFirst) {}
	int16 x, y;
	byte facing;
	uint16 action[2]; // Look, use; IDs from the original room's action table.
	int8 approach[2]; // -1: keep facing; 0: face lookPosition; 1: walk and turn; 2/3: walk without turning.
	Common::Point lookPosition;
	byte actionCursor; // 0: ordinary hotspot; 3: talk; 4..7: northwest, northeast, southeast, southwest.
	bool preferLook; // Start with look even when the hotspot has a special action cursor.
};

struct RunawayItemAction {
	constexpr RunawayItemAction(uint16 roomId, byte itemId, byte hotspotId, uint16 actionId, int8 walk = 1) :
		room(roomId), item(itemId), hotspot(hotspotId), action(actionId), approach(walk) {}
	uint16 room;
	byte item, hotspot;
	uint16 action;
	int8 approach;
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
extern const RunawayRoom kRunawayScene6030;
extern const RunawayRoom kRunawayScene6080;
extern const RunawayRoom kRunawayScene6090;
extern const RunawayRoom kRunawayScene6130;
extern const RunawayRoom kRunawayScene6180;
extern const RunawayRoom kRunawayScene6190;
extern const RunawayRoom kRunawayScene6200;
extern const RunawayRoom kRunawayScene6210;
extern const RunawayRoom kRunawayScene6220;
extern const RunawayRoom kRunawayScene6226;
extern const RunawayRoom kRunawayScene6240;
extern const RunawayRoom kRunawayScene6250;
extern const RunawayRoom kRunawayScene6260;
extern const RunawayRoom kRunawayScene6263;
extern const RunawayRoom kRunawayScene6270;
extern const RunawayRoom kRunawayScene6280;
extern const RunawayRoom kRunawayScene6290;
extern const RunawayRoom kRunawayScene6300;
extern const RunawayRoom kRunawayScene6350;
extern const RunawayRoom kRunawayScene6370;
extern const RunawayRoom kRunawayScene6390;
extern const RunawayRoom kRunawayScene7010;
extern const RunawayRoom kRunawayScene7020;
extern const RunawayRoom kRunawayScene7030;
extern const RunawayRoom kRunawayScene7040;
extern const RunawayRoom kRunawayScene7050;
extern const RunawayRoom kRunawayScene8090;
extern const RunawayRoom kRunawayScene8100;
extern const RunawayRoom kRunawayScene8130;
extern const RunawayRoom kRunawayScene8180;
extern const RunawayRoom kRunawayScene8190;
extern const RunawayRoom kRunawayScene8200;
extern const RunawayRoom kRunawayScene8210;
extern const RunawayRoom kRunawayScene8220;
extern const RunawayRoom kRunawayScene8226;
extern const RunawayRoom kRunawayScene8230;
extern const RunawayRoom kRunawayScene8240;
extern const RunawayRoom kRunawayScene8260;
extern const RunawayRoom kRunawayScene8263;
extern const RunawayRoom kRunawayScene8290;
extern const RunawayRoom kRunawayScene8300;
extern const RunawayRoom kRunawayScene8350;
extern const RunawayRoom kRunawayScene8370;
extern const RunawayRoom kRunawayScene8380;
extern const RunawayRoom kRunawayScene8400;
extern const RunawayRoom kRunawayScene8410;
extern const RunawayRoom kRunawayScene8420;
extern const RunawayRoom kRunawayScene9010;
extern const RunawayRoom kRunawayScene9020;
extern const RunawayRoom kRunawayScene9030;

const RunawayRoom *findRunawayRoom(uint16 number);

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE_DATA_H
