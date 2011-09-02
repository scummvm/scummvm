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

#include "pegasus/MMShell/Utilities/MMIDObject.h"
#include "pegasus/types.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

/*

	Item is an object which can be picked up and carried around.
	Items have
		a location
		an ID.
		weight
		an owner (kNoActorID if no one is carrying the Item)

*/

//	Item IDs.

const tItemID kAirMask = 7;
const tItemID kAntidote = 8;
const tItemID kArgonCanister = 9;
const tItemID kCardBomb = 10;
const tItemID kCrowbar = 11;
const tItemID kGasCanister = 12;
const tItemID kHistoricalLog = 13;
const tItemID kJourneymanKey = 14;
const tItemID kKeyCard = 15;
const tItemID kMachineGun = 16;
const tItemID kMarsCard = 17;
const tItemID kNitrogenCanister = 18;
const tItemID kOrangeJuiceGlassFull = 19;
const tItemID kOrangeJuiceGlassEmpty = 20;
const tItemID kPoisonDart = 21;
const tItemID kSinclairKey = 22;
const tItemID kStunGun = 23;
const tItemID kArgonPickup = 24;

//	Biochips.

const tItemID kAIBiochip = 0;
const tItemID kInterfaceBiochip = 1;
const tItemID kMapBiochip = 2;
const tItemID kOpticalBiochip = 3;
const tItemID kPegasusBiochip = 4;
const tItemID kRetinalScanBiochip = 5;
const tItemID kShieldBiochip = 6;

const tItemID kNumItems = 25;

//	Item States.

const tItemState kAI000 = 0;
const tItemState kAI005 = 1;
const tItemState kAI006 = 2;
const tItemState kAI010 = 3;
const tItemState kAI015 = 4;
const tItemState kAI016 = 5;
const tItemState kAI020 = 6;
const tItemState kAI024 = 7;
const tItemState kAI100 = 8;
const tItemState kAI101 = 9;
const tItemState kAI105 = 10;
const tItemState kAI106 = 11;
const tItemState kAI110 = 12;
const tItemState kAI111 = 13;
const tItemState kAI115 = 14;
const tItemState kAI116 = 15;
const tItemState kAI120 = 16;
const tItemState kAI121 = 17;
const tItemState kAI124 = 18;
const tItemState kAI125 = 19;
const tItemState kAI126 = 20;
const tItemState kAI200 = 21;
const tItemState kAI201 = 22;
const tItemState kAI202 = 23;
const tItemState kAI205 = 24;
const tItemState kAI206 = 25;
const tItemState kAI210 = 26;
const tItemState kAI211 = 27;
const tItemState kAI212 = 28;
const tItemState kAI215 = 29;
const tItemState kAI216 = 30;
const tItemState kAI220 = 31;
const tItemState kAI221 = 32;
const tItemState kAI222 = 33;
const tItemState kAI224 = 34;
const tItemState kAI225 = 35;
const tItemState kAI226 = 36;
const tItemState kAI300 = 37;
const tItemState kAI301 = 38;
const tItemState kAI302 = 39;
const tItemState kAI303 = 40;
const tItemState kAI305 = 41;
const tItemState kAI306 = 42;
const tItemState kAI310 = 43;
const tItemState kAI311 = 44;
const tItemState kAI312 = 45;
const tItemState kAI313 = 46;
const tItemState kAI315 = 47;
const tItemState kAI316 = 48;
const tItemState kAI320 = 49;
const tItemState kAI321 = 50;
const tItemState kAI322 = 51;
const tItemState kAI323 = 52;
const tItemState kAI324 = 53;
const tItemState kAI325 = 54;
const tItemState kAI326 = 55;
const tItemState kNormalItem = 56;
const tItemState kMapUnavailable = 57;
const tItemState kMapEngaged = 58;
const tItemState kOptical000 = 59;
const tItemState kOptical001 = 60;
const tItemState kOptical002 = 61;
const tItemState kOptical010 = 62;
const tItemState kOptical011 = 63;
const tItemState kOptical012 = 64;
const tItemState kOptical020 = 65;
const tItemState kOptical021 = 66;
const tItemState kOptical100 = 67;
const tItemState kOptical101 = 68;
const tItemState kOptical102 = 69;
const tItemState kOptical110 = 70;
const tItemState kOptical111 = 71;
const tItemState kOptical112 = 72;
const tItemState kOptical120 = 73;
const tItemState kOptical121 = 74;
const tItemState kOptical200 = 75;
const tItemState kOptical201 = 76;
const tItemState kOptical210 = 77;
const tItemState kOptical211 = 78;
const tItemState kPegasusTSA00 = 79;
const tItemState kPegasusTSA10 = 80;
const tItemState kPegasusPrehistoric00 = 81;
const tItemState kPegasusPrehistoric01 = 82;
const tItemState kPegasusPrehistoric10 = 83;
const tItemState kPegasusPrehistoric11 = 84;
const tItemState kPegasusMars00 = 85;
const tItemState kPegasusMars01 = 86;
const tItemState kPegasusMars10 = 87;
const tItemState kPegasusMars11 = 88;
const tItemState kPegasusNorad00 = 89;
const tItemState kPegasusNorad01 = 90;
const tItemState kPegasusNorad10 = 91;
const tItemState kPegasusNorad11 = 92;
const tItemState kPegasusWSC00 = 93;
const tItemState kPegasusWSC01 = 94;
const tItemState kPegasusWSC10 = 95;
const tItemState kPegasusWSC11 = 96;
const tItemState kPegasusCaldoria = 97;
const tItemState kRetinalSimulating = 98;
const tItemState kShieldNormal = 99;
const tItemState kShieldRadiation = 100;
const tItemState kShieldPlasma = 101;
const tItemState kShieldCardBomb = 102;
const tItemState kShieldDraining = 103;
const tItemState kAirMaskEmptyOff = 104;
const tItemState kAirMaskEmptyFilter = 105;
const tItemState kAirMaskLowOff = 106;
const tItemState kAirMaskLowFilter = 107;
const tItemState kAirMaskLowOn = 108;
const tItemState kAirMaskFullOff = 109;
const tItemState kAirMaskFullFilter = 110;
const tItemState kAirMaskFullOn = 111;
const tItemState kArgonEmpty = 112;
const tItemState kArgonFull = 113;
const tItemState kFlashlightOff = 114;
const tItemState kFlashlightOn = 115;
const tItemState kNitrogenEmpty = 116;
const tItemState kNitrogenFull = 117;
const tItemState kFullGlass = 118;

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

class Item : public MMIDObject {
public:
	Item(const tItemID id, const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	virtual ~Item();
	
	// WriteToStream writes everything EXCEPT the item's ID.
	// It is assumed that the calling function will write and read the ID.
	virtual Common::Error writeToStream(Common::WriteStream *stream);
	virtual Common::Error readFromStream(Common::ReadStream *stream);
	
	virtual tActorID getItemOwner() const;
	virtual void setItemOwner(const tActorID owner);
	
	void getItemRoom(tNeighborhoodID &neighborhood, tRoomID &room, tDirectionConstant &direction) const;
	void setItemRoom(const tNeighborhoodID neighborhood, const tRoomID room, const tDirectionConstant direction);
	tNeighborhoodID getItemNeighborhood() const;
	
	virtual tWeightType getItemWeight();
	
	virtual void setItemState(const tItemState state);
	virtual tItemState getItemState() const;

protected:
	tNeighborhoodID _itemNeighborhood;
	tRoomID	_itemRoom;
	tDirectionConstant _itemDirection;
	tActorID _itemOwnerID;
	tWeightType _itemWeight;
	tItemState _itemState;
};

} // End of namespace Pegasus

#endif
