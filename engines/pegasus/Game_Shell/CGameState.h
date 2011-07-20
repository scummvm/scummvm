/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_GAMESHELL_CGAMESTATE_H
#define PEGASUS_GAMESHELL_CGAMESTATE_H

#include "pegasus/Game_Shell/Headers/Game_Shell_Types.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

class CGameState {
public:
	static Common::Error WriteGameState(Common::WriteStream *stream);
	static Common::Error ReadGameState(Common::ReadStream *stream);
	
	static void ResetGameState();
	
	static void GetCurrentLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction);
	static void SetCurrentLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	
	static tNeighborhoodID GetCurrentNeighborhood();
	static void SetCurrentNeighborhood(const tNeighborhoodID neighborhood);
	static tRoomID GetCurrentRoom();
	static void SetCurrentRoom(const tRoomID room);
	static tDirectionConstant GetCurrentDirection();
	static void SetCurrentDirection(const tDirectionConstant direction);
	
	static tRoomViewID GetCurrentRoomAndView();
	
	static void GetNextLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction);
	static void	SetNextLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	
	static tNeighborhoodID GetNextNeighborhood();
	static void SetNextNeighborhood(const tNeighborhoodID neighborhood);
	static tRoomID GetNextRoom();
	static void SetNextRoom(const tRoomID room);
	static tDirectionConstant GetNextDirection();
	static void SetNextDirection(const tDirectionConstant direction);
	
	static void GetLastLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction);
	static void SetLastLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	
	static tNeighborhoodID GetLastNeighborhood();
	static void SetLastNeighborhood(const tNeighborhoodID neighborhood);
	static tRoomID GetLastRoom();
	static void SetLastRoom(const tRoomID room);
	static tDirectionConstant GetLastDirection();
	static void SetLastDirection(const tDirectionConstant direction);
	
	static tRoomViewID GetLastRoomAndView();
	
	static void GetOpenDoorLocation(tRoomID &room, tDirectionConstant &direction);
	static void SetOpenDoorLocation(const tRoomID room, const tDirectionConstant direction);
	static tRoomID GetOpenDoorRoom();
	static void SetOpenDoorRoom(const tRoomID room);
	static tDirectionConstant GetOpenDoorDirection();
	static void SetOpenDoorDirection(const tDirectionConstant direction);
	
	static tRoomViewID GetDoorOpenRoomAndView();
	
	static bool IsCurrentDoorOpen();

protected:
	static tNeighborhoodID gCurrentNeighborhood;
	static tRoomID gCurrentRoom;
	static tDirectionConstant gCurrentDirection;
	static tNeighborhoodID gNextNeighborhoodID;
	static tRoomID gNextRoomID;
	static tDirectionConstant gNextDirection;
	static tNeighborhoodID gLastNeighborhood;
	static tRoomID gLastRoom;
	static tDirectionConstant gLastDirection;
	static tRoomID gOpenDoorRoom;
	static tDirectionConstant gOpenDoorDirection;
};

} // End of namespace Pegasus

#endif
