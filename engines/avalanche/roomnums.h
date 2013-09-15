/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_ROOMNUMS_H
#define AVALANCHE_ROOMNUMS_H

#include "common/system.h"

namespace Avalanche {

enum Room {
	kRoomNowhere = 0,
	kRoomYours = 1,
	kRoomOutsideYours = 2,
	kRoomOutsideSpludwicks = 3,
	kRoomYourHall = 5,
	kRoomMusicRoom = 7,
	kRoomOutsideArgentPub = 9,
	kRoomArgentRoad = 10,
	kRoomWiseWomans = 11,
	kRoomSpludwicks = 12,
	kRoomInsideAbbey = 13,
	kRoomOutsideAbbey = 14,
	kRoomAvvysGarden = 15,
	kRoomAylesOffice = 16,
	kRoomArgentPub = 19,
	kRoomBrummieRoad = 20,
	kRoomBridge = 21, 
	kRoomLusties = 22,
	kRoomLustiesRoom = 23,
	kRoomWestHall = 25,
	kRoomEastHall = 26,
	kRoomOubliette = 27,
	kRoomGeidas = 28,
	kRoomCatacombs = 29,
	kRoomEntranceHall = 40,
	kRoomRobins = 42,
	kRoomOutsideNottsPub = 46,
	kRoomNottsPub = 47,
	kRoomOutsideDucks = 50,
	kRoomDucks = 51,
	kRoomOutsideCardiffCastle = 70,
	kRoomInsideCardiffCastle = 71,
	kRoomBossKey = 98, // assumed
	kRoomMap = 99
};

} // End of namespace Avalanche

#endif // AVALANCHE_ROOMNUMS_H
