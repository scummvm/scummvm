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

#ifndef PEGASUS_ITEMS_ITEM_H
#define PEGASUS_ITEMS_ITEM_H

#include "common/endian.h"

#include "pegasus/types.h"
#include "pegasus/util.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
	class SeekableReadStream;
}

namespace Pegasus {

//	JMPItemInfo contains resource data used by all Items.

struct JMPItemInfo {
	TimeValue infoLeftTime;
	TimeValue infoRightStart;
	TimeValue infoRightStop;
	uint32 dragSpriteNormalID;
	uint32 dragSpriteUsedID;
};

//	ItemStateEntry contains a single state/TimeValue pair. The TimeValue is
//	the time value to set the shared area movie that corresponds with the given
//	state of an inventory item.

struct ItemStateEntry {
	ItemState itemState;
	TimeValue itemTime;
};

//	ItemStateInfoHandle is an array of ItemStateEntry.

struct ItemStateInfo {
	uint16 numEntries; //	For easy ResEdit access
	ItemStateEntry *entries;
};

//	ItemExtraEntry

const short kLeftAreaExtra = 0;
const short kMiddleAreaExtra = 1;
const short kRightAreaExtra = 2;

struct ItemExtraEntry {
	uint32 extraID;
	uint16 extraArea;
	TimeValue extraStart;
	TimeValue extraStop;
};

//	tItemExtraInfoHandle is an array of tItemExtraEntry.

struct ItemExtraInfo {
	uint16 numEntries;		//	For easy ResEdit access
	ItemExtraEntry *entries;
};

//	Inventory info resource type and ID:
//	Individual inventory items are stored in these resource types.
//	Resource ID is item ID + kItemBaseResID.

const uint32 kItemInfoResType = MKTAG('I', 't', 'e', 'm');       // JMPItemInfoHandle
const uint32 kLeftAreaInfoResType = MKTAG('L', 'e', 'f', 't');   // ItemStateInfoHandle
const uint32 kMiddleAreaInfoResType = MKTAG('M', 'i', 'd', 'l'); // ItemStateInfoHandle
const uint32 kRightAreaInfoResType = MKTAG('R', 'g', 'h', 't');  // ItemStateInfoHandle
const uint32 kItemExtraInfoResType = MKTAG('I', 'X', 't', 'r');	 // ItemExtraInfoHandle

const uint16 kItemBaseResID = 128;

//	Item IDs.

const ItemID kAirMask = 7;
const ItemID kAntidote = 8;
const ItemID kArgonCanister = 9;
const ItemID kCardBomb = 10;
const ItemID kCrowbar = 11;
const ItemID kGasCanister = 12;
const ItemID kHistoricalLog = 13;
const ItemID kJourneymanKey = 14;
const ItemID kKeyCard = 15;
const ItemID kMachineGun = 16;
const ItemID kMarsCard = 17;
const ItemID kNitrogenCanister = 18;
const ItemID kOrangeJuiceGlassFull = 19;
const ItemID kOrangeJuiceGlassEmpty = 20;
const ItemID kPoisonDart = 21;
const ItemID kSinclairKey = 22;
const ItemID kStunGun = 23;
const ItemID kArgonPickup = 24;

//	Biochips.

const ItemID kAIBiochip = 0;
const ItemID kInterfaceBiochip = 1;
const ItemID kMapBiochip = 2;
const ItemID kOpticalBiochip = 3;
const ItemID kPegasusBiochip = 4;
const ItemID kRetinalScanBiochip = 5;
const ItemID kShieldBiochip = 6;

const ItemID kNumItems = 25;

//	Item States.

const ItemState kAI000 = 0;
const ItemState kAI005 = 1;
const ItemState kAI006 = 2;
const ItemState kAI010 = 3;
const ItemState kAI015 = 4;
const ItemState kAI016 = 5;
const ItemState kAI020 = 6;
const ItemState kAI024 = 7;
const ItemState kAI100 = 8;
const ItemState kAI101 = 9;
const ItemState kAI105 = 10;
const ItemState kAI106 = 11;
const ItemState kAI110 = 12;
const ItemState kAI111 = 13;
const ItemState kAI115 = 14;
const ItemState kAI116 = 15;
const ItemState kAI120 = 16;
const ItemState kAI121 = 17;
const ItemState kAI124 = 18;
const ItemState kAI125 = 19;
const ItemState kAI126 = 20;
const ItemState kAI200 = 21;
const ItemState kAI201 = 22;
const ItemState kAI202 = 23;
const ItemState kAI205 = 24;
const ItemState kAI206 = 25;
const ItemState kAI210 = 26;
const ItemState kAI211 = 27;
const ItemState kAI212 = 28;
const ItemState kAI215 = 29;
const ItemState kAI216 = 30;
const ItemState kAI220 = 31;
const ItemState kAI221 = 32;
const ItemState kAI222 = 33;
const ItemState kAI224 = 34;
const ItemState kAI225 = 35;
const ItemState kAI226 = 36;
const ItemState kAI300 = 37;
const ItemState kAI301 = 38;
const ItemState kAI302 = 39;
const ItemState kAI303 = 40;
const ItemState kAI305 = 41;
const ItemState kAI306 = 42;
const ItemState kAI310 = 43;
const ItemState kAI311 = 44;
const ItemState kAI312 = 45;
const ItemState kAI313 = 46;
const ItemState kAI315 = 47;
const ItemState kAI316 = 48;
const ItemState kAI320 = 49;
const ItemState kAI321 = 50;
const ItemState kAI322 = 51;
const ItemState kAI323 = 52;
const ItemState kAI324 = 53;
const ItemState kAI325 = 54;
const ItemState kAI326 = 55;
const ItemState kNormalItem = 56;
const ItemState kMapUnavailable = 57;
const ItemState kMapEngaged = 58;
const ItemState kOptical000 = 59;
const ItemState kOptical001 = 60;
const ItemState kOptical002 = 61;
const ItemState kOptical010 = 62;
const ItemState kOptical011 = 63;
const ItemState kOptical012 = 64;
const ItemState kOptical020 = 65;
const ItemState kOptical021 = 66;
const ItemState kOptical100 = 67;
const ItemState kOptical101 = 68;
const ItemState kOptical102 = 69;
const ItemState kOptical110 = 70;
const ItemState kOptical111 = 71;
const ItemState kOptical112 = 72;
const ItemState kOptical120 = 73;
const ItemState kOptical121 = 74;
const ItemState kOptical200 = 75;
const ItemState kOptical201 = 76;
const ItemState kOptical210 = 77;
const ItemState kOptical211 = 78;
const ItemState kPegasusTSA00 = 79;
const ItemState kPegasusTSA10 = 80;
const ItemState kPegasusPrehistoric00 = 81;
const ItemState kPegasusPrehistoric01 = 82;
const ItemState kPegasusPrehistoric10 = 83;
const ItemState kPegasusPrehistoric11 = 84;
const ItemState kPegasusMars00 = 85;
const ItemState kPegasusMars01 = 86;
const ItemState kPegasusMars10 = 87;
const ItemState kPegasusMars11 = 88;
const ItemState kPegasusNorad00 = 89;
const ItemState kPegasusNorad01 = 90;
const ItemState kPegasusNorad10 = 91;
const ItemState kPegasusNorad11 = 92;
const ItemState kPegasusWSC00 = 93;
const ItemState kPegasusWSC01 = 94;
const ItemState kPegasusWSC10 = 95;
const ItemState kPegasusWSC11 = 96;
const ItemState kPegasusCaldoria = 97;
const ItemState kRetinalSimulating = 98;
const ItemState kShieldNormal = 99;
const ItemState kShieldRadiation = 100;
const ItemState kShieldPlasma = 101;
const ItemState kShieldCardBomb = 102;
const ItemState kShieldDraining = 103;
const ItemState kAirMaskEmptyOff = 104;
const ItemState kAirMaskEmptyFilter = 105;
const ItemState kAirMaskLowOff = 106;
const ItemState kAirMaskLowFilter = 107;
const ItemState kAirMaskLowOn = 108;
const ItemState kAirMaskFullOff = 109;
const ItemState kAirMaskFullFilter = 110;
const ItemState kAirMaskFullOn = 111;
const ItemState kArgonEmpty = 112;
const ItemState kArgonFull = 113;
const ItemState kFlashlightOff = 114;
const ItemState kFlashlightOn = 115;
const ItemState kNitrogenEmpty = 116;
const ItemState kNitrogenFull = 117;
const ItemState kFullGlass = 118;

//	Extra IDs.

const uint32 kRetinalScanSearching = 0;
const uint32 kRetinalScanActivated = 1;
const uint32 kShieldIntro = 2;
const uint32 kRemoveAirMask = 3;
const uint32 kRemoveArgon = 4;
const uint32 kRemoveCrowbar = 5;
const uint32 kGasCanLoop = 6;
const uint32 kRemoveJourneymanKey = 7;
const uint32 kRemoveMarsCard = 8;
const uint32 kRemoveNitrogen = 9;
const uint32 kRemoveGlass = 10;
const uint32 kRemoveDart = 11;
const uint32 kRemoveSinclairKey = 12;

enum ItemType {
	kInventoryItemType,
	kBiochipItemType
};

class Sprite;

/*

	Item is an object which can be picked up and carried around.
	Items have
		a location
		an ID.
		weight
		an owner (kNoActorID if no one is carrying the Item)

*/

class Item : public IDObject {
public:
	Item(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);
	virtual ~Item();
	
	// WriteToStream writes everything EXCEPT the item's ID.
	// It is assumed that the calling function will write and read the ID.
	virtual void writeToStream(Common::WriteStream *stream);
	virtual void readFromStream(Common::ReadStream *stream);
	
	virtual ActorID getItemOwner() const;
	virtual void setItemOwner(const ActorID owner);
	
	void getItemRoom(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) const;
	void setItemRoom(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);
	NeighborhoodID getItemNeighborhood() const;
	
	virtual WeightType getItemWeight();
	
	virtual void setItemState(const ItemState state);
	virtual ItemState getItemState() const;

	virtual ItemType getItemType() = 0;
	
	TimeValue getInfoLeftTime() const;
	void getInfoRightTimes(TimeValue &, TimeValue &) const;
	TimeValue getSharedAreaTime() const;

	Sprite *getDragSprite(const DisplayElementID) const;
	
	/*
		select		--	called when this item becomes current. Also called when the inventory
						panel holding this item is raised and this is the current item.
		deselect	--	called when this item is no longer current.
		activate	--	called on the current item when the panel is closed.
	*/
	//	In an override of these three member functions, you must call the inherited
	//	member functions.
	virtual void select();
	virtual void deselect();
	virtual bool isSelected() { return _isSelected; }
	
	virtual void activate() { _isActive = true; }
	virtual bool isActive() { return _isActive; }
	virtual void pickedUp() {}
	virtual void addedToInventory() {}
	virtual void removedFromInventory() {}
	virtual void dropped() {}

	//	Called when the shared area is taken by another item, but this item is still
	//	selected.
	virtual void giveUpSharedArea() {}
	virtual void takeSharedArea() {}
	
	void findItemExtra(const uint32 extraID, ItemExtraEntry &entry);

protected:
	NeighborhoodID _itemNeighborhood;
	RoomID	_itemRoom;
	DirectionConstant _itemDirection;
	ActorID _itemOwnerID;
	WeightType _itemWeight;
	ItemState _itemState;

	JMPItemInfo _itemInfo;
	ItemStateInfo _sharedAreaInfo;
	ItemExtraInfo _itemExtras;
	bool _isActive;
	bool _isSelected;

	static void getItemStateEntry(ItemStateInfo, uint32, ItemState &, TimeValue &);
	static void findItemStateEntryByState(ItemStateInfo, ItemState, TimeValue &);
	static ItemStateInfo readItemState(Common::SeekableReadStream *stream);
};

} // End of namespace Pegasus

#endif
