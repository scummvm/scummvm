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

#include "common/error.h"
#include "common/stream.h"

#include "pegasus/constants.h"
#include "pegasus/items/item.h"
#include "pegasus/items/itemlist.h"

namespace Pegasus {

Item::Item(const tItemID id, const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) : MMIDObject(id) {
	_itemNeighborhood = neighborhood;
	_itemRoom = room;
	_itemDirection = direction;
	_itemWeight = 1;
	_itemOwnerID = kNoActorID;
	_itemState = 0;

	g_allItems.push_back(this);
}

Item::~Item() {
}

Common::Error Item::writeToStream(Common::WriteStream *stream) {
	stream->writeUint16BE(_itemNeighborhood);
	stream->writeUint16BE(_itemRoom);
	stream->writeByte(_itemDirection);
	stream->writeUint16BE(_itemOwnerID);
	stream->writeUint16BE(_itemState);

	if (stream->err())
		return Common::kWritingFailed;
	
	return Common::kNoError;
}

Common::Error Item::readFromStream(Common::ReadStream *stream) {
	_itemNeighborhood = stream->readUint16BE();
	_itemRoom = stream->readUint16BE();
	_itemDirection = stream->readByte();
	_itemOwnerID = stream->readUint16BE();
	_itemState = stream->readUint16BE();
	
	if (stream->err())
		return Common::kReadingFailed;
	
	return Common::kNoError;
}

tActorID Item::getItemOwner() const {
	return _itemOwnerID;
}

void Item::setItemOwner(const tActorID owner) {
	_itemOwnerID = owner;
}

tWeightType Item::getItemWeight() {
	return _itemWeight;
}

tItemState Item::getItemState() const {
	return _itemState;
}

void Item::setItemState(const tItemState state) {
	_itemState = state;
}

void Item::getItemRoom(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) const {
	neighborhood = _itemNeighborhood;
	room = _itemRoom;
	direction = _itemDirection;
}

void Item::setItemRoom(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction) {
	_itemNeighborhood = neighborhood;
	_itemRoom = room;
	_itemDirection = direction;
}

tNeighborhoodID Item::getItemNeighborhood() const {
	return _itemNeighborhood;
}

} // End of namespace Pegasus
