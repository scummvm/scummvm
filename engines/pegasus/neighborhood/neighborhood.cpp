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

#include "common/debug.h"
#include "common/stream.h"

#include "pegasus/pegasus.h"
#include "pegasus/game_shell/CGameState.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

Neighborhood::Neighborhood(PegasusEngine *vm, const Common::String &resName, tNeighborhoodID id) : _vm(vm), _resName(resName) {
	CGameState::SetOpenDoorLocation(kNoRoomID, kNoDirection);
	_currentAlternate = 0;
}

Neighborhood::~Neighborhood() {
}

void Neighborhood::init() {
	debug(0, "Loading '%s' neighborhood resources", _resName.c_str());

	Common::SeekableReadStream *stream = _vm->_resFork->getResource(_doorTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load doors");
	_doorTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_exitTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load exits");
	_exitTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_extraTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load extras");
	_extraTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_hotspotInfoTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load hotspot info");
	_hotspotInfoTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_spotTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load spots");
	_spotTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_turnTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load turns");
	_turnTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_viewTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load views");
	_viewTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_zoomTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load zooms");
	_zoomTable.loadFromStream(stream);
	delete stream;

	// TODO: AI, movies, notifications, buncha other stuff
}

void Neighborhood::start() {
	CGameState::SetCurrentRoom(CGameState::GetLastRoom());
	CGameState::SetCurrentDirection(CGameState::GetLastDirection());
	arriveAt(CGameState::GetNextRoom(), CGameState::GetNextDirection());
}

void Neighborhood::arriveAt(tRoomID room, tDirectionConstant direction) {
	// TODO
}

//	These functions can be overridden to tweak the exact frames used.

void Neighborhood::getExitEntry(const tRoomID room, const tDirectionConstant direction, ExitTable::Entry &entry) {
	entry = _exitTable.findEntry(room, direction, _currentAlternate);

	if (entry.isEmpty())
		entry = _exitTable.findEntry(room, direction, kNoAlternateID);
}

TimeValue Neighborhood::getViewTime(const tRoomID room, const tDirectionConstant direction) {
	if (CGameState::GetOpenDoorRoom() == room && CGameState::GetOpenDoorDirection() == direction) {
		// If we get here, the door entry for this location must exist.
		DoorTable::Entry doorEntry = _doorTable.findEntry(room, direction, _currentAlternate);

		if (doorEntry.isEmpty())
			doorEntry = _doorTable.findEntry(room, direction, kNoAlternateID);

		return doorEntry.movieEnd - 1;
	}

	ViewTable::Entry viewEntry = _viewTable.findEntry(room, direction, _currentAlternate);

	if (viewEntry.isEmpty())
		viewEntry = _viewTable.findEntry(room, direction, kNoAlternateID);

	return viewEntry.time;
}

void Neighborhood::getDoorEntry(const tRoomID room, const tDirectionConstant direction, DoorTable::Entry &doorEntry) {
	doorEntry = _doorTable.findEntry(room, direction, _currentAlternate);

	if (doorEntry.isEmpty())
		doorEntry = _doorTable.findEntry(room, direction, kNoAlternateID);
}

tDirectionConstant Neighborhood::getTurnEntry(const tRoomID room, const tDirectionConstant direction, const tTurnDirection turn) {
	TurnTable::Entry turnEntry = _turnTable.findEntry(room, direction, turn, _currentAlternate);

	if (turnEntry.isEmpty())
		turnEntry = _turnTable.findEntry(room, direction, turn, kNoAlternateID);

	return turnEntry.turnDirection;
}

void Neighborhood::findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &spotEntry) {
	spotEntry = _spotTable.findEntry(room, direction, flags, _currentAlternate);

	if (spotEntry.isEmpty())
		spotEntry = _spotTable.findEntry(room, direction, flags, kNoAlternateID);
}

void Neighborhood::getZoomEntry(const tHotSpotID id, ZoomTable::Entry &zoomEntry) {
	zoomEntry = _zoomTable.findEntry(id);
}

void Neighborhood::getHotspotEntry(const tHotSpotID id, HotspotInfoTable::Entry &hotspotEntry) {
	hotspotEntry = _hotspotInfoTable.findEntry(id);
}

void Neighborhood::getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry) {
	extraEntry = _extraTable.findEntry(id);
}

/////////////////////////////////////////////
//
//	"Can" functions: Called to see whether or not a user is allowed to do something

tCanMoveForwardReason Neighborhood::canMoveForward(ExitTable::Entry &entry) {
	DoorTable::Entry door;
	
	getExitEntry(CGameState::GetCurrentRoom(), CGameState::GetCurrentDirection(), entry);
	getDoorEntry(CGameState::GetCurrentRoom(), CGameState::GetCurrentDirection(), door);

	//	Fixed this so that doors that don't lead anywhere can be opened, but not walked
	//	through.
	if (door.flags & kDoorPresentMask) {
		if (CGameState::IsCurrentDoorOpen()) {
			if (entry.exitRoom == kNoRoomID)
				return kCantMoveBlocked;
			else
				return kCanMoveForward;
		} else if (door.flags & kDoorLockedMask) {
			return kCantMoveDoorLocked;
		} else {
			return kCantMoveDoorClosed;
		}
	} else if (entry.exitRoom == kNoRoomID) {
		return kCantMoveBlocked;
	}

	return kCanMoveForward;
}

tCanTurnReason Neighborhood::canTurn(tTurnDirection turn, tDirectionConstant &nextDir) {
	nextDir = getTurnEntry(CGameState::GetCurrentRoom(), CGameState::GetCurrentDirection(), turn);

	if (nextDir == kNoDirection)
		return kCantTurnNoTurn;

	return kCanTurn;
}

tCanOpenDoorReason Neighborhood::canOpenDoor(DoorTable::Entry &entry) {
	getDoorEntry(CGameState::GetCurrentRoom(), CGameState::GetCurrentDirection(), entry);

	if (entry.flags & kDoorPresentMask) {
		if (CGameState::IsCurrentDoorOpen())
			return kCantOpenAlreadyOpen;

		if (entry.flags & kDoorLockedMask)
			return kCantOpenLocked;

		return kCanOpenDoor;
	}

	return kCantOpenNoDoor;
}


} // End of namespace Pegasus
