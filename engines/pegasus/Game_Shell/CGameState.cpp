/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
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

#include "common/error.h"
#include "common/stream.h"

#include "pegasus/Game_Shell/CGameState.h"
#include "pegasus/Game_Shell/Headers/Game_Shell_Constants.h"

namespace Pegasus {

tNeighborhoodID CGameState::gCurrentNeighborhood = kNoNeighborhoodID;
tRoomID CGameState::gCurrentRoom = kNoRoomID;
tDirectionConstant CGameState::gCurrentDirection = kNoDirection;
tNeighborhoodID CGameState::gNextNeighborhoodID = kNoNeighborhoodID;
tRoomID CGameState::gNextRoomID = kNoRoomID;
tDirectionConstant CGameState::gNextDirection = kNoDirection;
tNeighborhoodID CGameState::gLastNeighborhood = kNoNeighborhoodID;
tRoomID CGameState::gLastRoom = kNoRoomID;
tDirectionConstant CGameState::gLastDirection = kNoDirection;
tRoomID CGameState::gOpenDoorRoom = kNoRoomID;
tDirectionConstant CGameState::gOpenDoorDirection = kNoDirection;

Common::Error CGameState::WriteGameState(Common::WriteStream *stream) {
	stream->writeUint16BE(gCurrentNeighborhood);
	stream->writeUint16BE(gCurrentRoom);
	stream->writeByte(gCurrentDirection);
	stream->writeUint16BE(gNextNeighborhoodID);
	stream->writeUint16BE(gNextRoomID);
	stream->writeByte(gNextDirection);
	stream->writeUint16BE(gLastNeighborhood);
	stream->writeUint16BE(gLastRoom);
	stream->writeUint16BE(gOpenDoorRoom);
	stream->writeByte(gOpenDoorDirection);

	if (stream->err())
		return Common::kWritingFailed;
	
	return Common::kNoError;
}

Common::Error CGameState::ReadGameState(Common::ReadStream *stream) {
	gCurrentNeighborhood = stream->readUint16BE();
	gCurrentRoom = stream->readUint16BE();
	gCurrentDirection = stream->readByte();
	gNextNeighborhoodID = stream->readUint16BE();
	gNextRoomID = stream->readUint16BE();
	gNextDirection = stream->readByte();
	gLastNeighborhood = stream->readUint16BE();
	gLastRoom = stream->readUint16BE();
	gOpenDoorRoom = stream->readUint16BE();
	gOpenDoorDirection = stream->readByte();

	if (stream->err())
		return Common::kReadingFailed;
	
	return Common::kNoError;
}

void CGameState::ResetGameState() {
	gCurrentNeighborhood = kNoNeighborhoodID;
	gCurrentRoom = kNoRoomID;
	gCurrentDirection = kNoDirection;
	gNextNeighborhoodID = kNoNeighborhoodID;
	gNextRoomID = kNoRoomID;
	gNextDirection = kNoDirection;
	gLastNeighborhood = kNoNeighborhoodID;
	gLastRoom = kNoRoomID;
	gLastDirection = kNoDirection;
	gOpenDoorRoom = kNoRoomID;
	gOpenDoorDirection = kNoDirection;
}

void CGameState::GetCurrentLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) {
	neighborhood = gCurrentNeighborhood;
	room = gCurrentRoom;
	direction = gCurrentDirection;
}

void CGameState::SetCurrentLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	gLastNeighborhood = gCurrentNeighborhood;
	gLastRoom = gCurrentRoom;
	gLastDirection = gCurrentDirection;
	gCurrentNeighborhood = neighborhood;
	gCurrentRoom = room;
	gCurrentDirection = direction;
}

tNeighborhoodID CGameState::GetCurrentNeighborhood() {
	return gCurrentNeighborhood;
}

void CGameState::SetCurrentNeighborhood(const tNeighborhoodID neighborhood) {
	gLastNeighborhood = gCurrentNeighborhood;
	gCurrentNeighborhood = neighborhood;
}

tRoomID CGameState::GetCurrentRoom() {
	return gCurrentRoom;
}

void CGameState::SetCurrentRoom(const tRoomID room) {
	gLastRoom = gCurrentRoom;
	gCurrentRoom = room;
}

tDirectionConstant CGameState::GetCurrentDirection() {
	return gCurrentDirection;
}

void CGameState::SetCurrentDirection(const tDirectionConstant direction) {
	gLastDirection = gCurrentDirection;
	gCurrentDirection = direction;
}

tRoomViewID CGameState::GetCurrentRoomAndView() {
	return MakeRoomView(gCurrentRoom, gCurrentDirection);
}

void CGameState::GetNextLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) {
	neighborhood = gNextNeighborhoodID;
	room = gNextRoomID;
	direction = gNextDirection;
}

void CGameState::SetNextLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	gNextNeighborhoodID = neighborhood;
	gNextRoomID = room;
	gNextDirection = direction;
}

tNeighborhoodID CGameState::GetNextNeighborhood() {
	return gNextNeighborhoodID;
}

void CGameState::SetNextNeighborhood(const tNeighborhoodID neighborhood) {
	gNextNeighborhoodID = neighborhood;
}

tRoomID CGameState::GetNextRoom() {
	return gNextRoomID;
}

void CGameState::SetNextRoom(const tRoomID room) {
	gNextRoomID = room;
}

tDirectionConstant CGameState::GetNextDirection() {
	return gNextDirection;
}

void CGameState::SetNextDirection(const tDirectionConstant direction) {
	gNextDirection = direction;
}

void CGameState::GetLastLocation(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) {
	neighborhood = gCurrentNeighborhood;
	room = gCurrentRoom;
	direction = gCurrentDirection;
}

void CGameState::SetLastLocation(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	gCurrentNeighborhood = neighborhood;
	gCurrentRoom = room;
	gCurrentDirection = direction;
}

tNeighborhoodID CGameState::GetLastNeighborhood() {
	return gLastNeighborhood;
}

void CGameState::SetLastNeighborhood(const tNeighborhoodID neighborhood) {
	gLastNeighborhood = neighborhood;
}

tRoomID CGameState::GetLastRoom() {
	return gLastRoom;
}

void CGameState::SetLastRoom(const tRoomID room) {
	gLastRoom = room;
}

tDirectionConstant CGameState::GetLastDirection() {
	return gLastDirection;
}

void CGameState::SetLastDirection(const tDirectionConstant direction) {
	gLastDirection = direction;
}

tRoomViewID CGameState::GetLastRoomAndView() {
	return MakeRoomView(gLastRoom, gLastDirection);
}

void CGameState::GetOpenDoorLocation(tRoomID &room, tDirectionConstant &direction) {
	room = gOpenDoorRoom;
	direction = gOpenDoorDirection;
}

void CGameState::SetOpenDoorLocation(const tRoomID room, const tDirectionConstant direction) {
	gOpenDoorRoom = room;
	gOpenDoorDirection = direction;
}

tRoomID CGameState::GetOpenDoorRoom() {
	return gOpenDoorRoom;
}

void CGameState::SetOpenDoorRoom(const tRoomID room) {
	gOpenDoorRoom = room;
}

tDirectionConstant CGameState::GetOpenDoorDirection() {
	return gOpenDoorDirection;
}

void CGameState::SetOpenDoorDirection(const tDirectionConstant direction) {
	gOpenDoorDirection = direction;
}

tRoomViewID CGameState::GetDoorOpenRoomAndView() {
	return MakeRoomView(gOpenDoorRoom, gOpenDoorDirection);
}

bool CGameState::IsCurrentDoorOpen() {
	return gOpenDoorRoom == gCurrentRoom && gOpenDoorDirection == gCurrentDirection;
}

} // End of namespace Pegasus
