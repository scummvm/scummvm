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

#ifndef PEGASUS_GAMESHELL_CITEM_H
#define PEGASUS_GAMESHELL_CITEM_H

#include "pegasus/MMShell/Utilities/MMIDObject.h"
#include "pegasus/Game_Shell/Headers/Game_Shell_Types.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

/*

	CItem is an object which can be picked up and carried around.
	CItems have
		a location
		an ID.
		weight
		an owner (kNoActorID if no one is carrying the CItem)

*/

class CItem : public MMIDObject {
public:
	CItem(const tItemID id, const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	virtual ~CItem();
	
	// WriteToStream writes everything EXCEPT the item's ID.
	// It is assumed that the calling function will write and read the ID.
	virtual Common::Error WriteToStream(Common::WriteStream *stream);
	virtual Common::Error ReadFromStream(Common::ReadStream *stream);
	
	virtual tActorID GetItemOwner() const;
	virtual void SetItemOwner(const tActorID owner);
	
	void GetItemRoom(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) const;
	void SetItemRoom(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	tNeighborhoodID GetItemNeighborhood() const;
	
	virtual tWeightType GetItemWeight();
	
	virtual void SetItemState(const tItemState state);
	virtual tItemState GetItemState() const;

protected:
	tNeighborhoodID fItemNeighborhood;
	tRoomID	fItemRoom;
	tDirectionConstant fItemDirection;
	tActorID fItemOwnerID;
	tWeightType fItemWeight;
	tItemState fItemState;
};

} // End of namespace Pegasus

#endif
