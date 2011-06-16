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

#include "pegasus/Game_Shell/CItem.h"
#include "pegasus/Game_Shell/CItemList.h"
#include "pegasus/Game_Shell/Headers/Game_Shell_Constants.h"

namespace Pegasus {

CItem::CItem(const tItemID id, const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) : MMIDObject(id) {
	fItemNeighborhood = neighborhood;
	fItemRoom = room;
	fItemDirection = direction;
	fItemWeight = 1;
	fItemOwnerID = kNoActorID;
	fItemState = 0;

	gAllItems.push_back(this);
}

CItem::~CItem() {
}

Common::Error CItem::WriteToStream(Common::WriteStream *stream) {
	stream->writeUint16BE(fItemNeighborhood);
	stream->writeUint16BE(fItemRoom);
	stream->writeByte(fItemDirection);
	stream->writeUint16BE(fItemOwnerID);
	stream->writeUint16BE(fItemState);

	if (stream->err())
		return Common::kWritingFailed;
	
	return Common::kNoError;
}

Common::Error CItem::ReadFromStream(Common::ReadStream *stream) {
	fItemNeighborhood = stream->readUint16BE();
	fItemRoom = stream->readUint16BE();
	fItemDirection = stream->readByte();
	fItemOwnerID = stream->readUint16BE();
	fItemState = stream->readUint16BE();
	
	if (stream->err())
		return Common::kReadingFailed;
	
	return Common::kNoError;
}

tActorID CItem::GetItemOwner() const {
	return fItemOwnerID;
}

void CItem::SetItemOwner(const tActorID owner) {
	fItemOwnerID = owner;
}

tWeightType CItem::GetItemWeight() {
	return fItemWeight;
}

tItemState CItem::GetItemState() const {
	return fItemState;
}

void CItem::SetItemState(const tItemState state) {
	fItemState = state;
}

void CItem::GetItemRoom(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) const {
	neighborhood = fItemNeighborhood;
	room = fItemRoom;
	direction = fItemDirection;
}

void CItem::SetItemRoom(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	fItemNeighborhood = neighborhood;
	fItemRoom = room;
	fItemDirection = direction;
}

tNeighborhoodID CItem::GetItemNeighborhood() const {
	return fItemNeighborhood;
}

} // End of namespace Pegasus
