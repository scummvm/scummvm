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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

Neighborhood::Neighborhood(PegasusEngine *vm, const Common::String &resName, tNeighborhoodID id) : _vm(vm), _resName(resName) {
	GameState.setOpenDoorLocation(kNoRoomID, kNoDirection);
	_currentAlternate = 0;
}

Neighborhood::~Neighborhood() {
	for (HotspotIterator it = _neighborhoodHotspots.begin(); it != _neighborhoodHotspots.end(); it++)
		g_allHotspots.remove(*it);

	_neighborhoodHotspots.deleteHotspots();
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

	createNeighborhoodSpots();

	// TODO: AI, movies, notifications, buncha other stuff
}

void Neighborhood::start() {
	GameState.setCurrentRoom(GameState.getLastRoom());
	GameState.setCurrentDirection(GameState.getLastDirection());
	arriveAt(GameState.getNextRoom(), GameState.getNextDirection());
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
	if (GameState.getOpenDoorRoom() == room && GameState.getOpenDoorDirection() == direction) {
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
	
	getExitEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), entry);
	getDoorEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), door);

	//	Fixed this so that doors that don't lead anywhere can be opened, but not walked
	//	through.
	if (door.flags & kDoorPresentMask) {
		if (GameState.isCurrentDoorOpen()) {
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
	nextDir = getTurnEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), turn);

	if (nextDir == kNoDirection)
		return kCantTurnNoTurn;

	return kCanTurn;
}

tCanOpenDoorReason Neighborhood::canOpenDoor(DoorTable::Entry &entry) {
	getDoorEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), entry);

	if (entry.flags & kDoorPresentMask) {
		if (GameState.isCurrentDoorOpen())
			return kCantOpenAlreadyOpen;

		if (entry.flags & kDoorLockedMask)
			return kCantOpenLocked;

		return kCanOpenDoor;
	}

	return kCantOpenNoDoor;
}

void Neighborhood::createNeighborhoodSpots() {
	Common::SeekableReadStream *hotspotList = _vm->_resFork->getResource(MKTAG('H', 'S', 'L', 's'), _resName);
	if (!hotspotList)
		error("Could not load neighborhood hotspots");

	uint32 hotspotCount = hotspotList->readUint32BE();

	while (hotspotCount--) {
		uint16 id = hotspotList->readUint16BE();
		uint32 flags = hotspotList->readUint32BE();
		uint32 rgnSize = hotspotList->readUint32BE();

		// duplicate of rgnSize
		hotspotList->readUint16BE();

		Common::Rect boundingBox;
		boundingBox.top = hotspotList->readUint16BE();
		boundingBox.left = hotspotList->readUint16BE();
		boundingBox.bottom = hotspotList->readUint16BE();
		boundingBox.right = hotspotList->readUint16BE();

		debug(0, "Hotspot[%d]: Flags = %08x", id, flags); 
		boundingBox.debugPrint(0, "\tBounding Box:");

		// TODO: Handle non-rectangular hotspots
		if (rgnSize != 10)
			warning("Non-rectangular hotspot found - %d extra bytes", rgnSize - 10);
		hotspotList->skip(rgnSize - 10);

		Hotspot *hotspot = new Hotspot(id);
		hotspot->setHotspotFlags(flags);
		hotspot->setArea(boundingBox);

		g_allHotspots.push_back(hotspot);
		_neighborhoodHotspots.push_back(hotspot);
	}

	delete hotspotList;
}

} // End of namespace Pegasus
