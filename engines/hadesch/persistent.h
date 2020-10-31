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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2020 Google
 *
 */

#include "common/serializer.h"

#include "hadesch/enums.h"

#ifndef HADESCH_PERSISTENT_H
#define HADESCH_PERSISTENT_H

namespace Hadesch {
  
struct HadeschSaveDescriptor {
	HadeschSaveDescriptor(Common::Serializer &s, int slot);

	int _slot;
	Common::U32String _heroName;
	Common::U32String _slotName;
	RoomId _room;
};

struct HadeschSaveDescriptorSlotComparator {
	bool operator()(const HadeschSaveDescriptor &x, const HadeschSaveDescriptor &y) const {
		return x._slot < y._slot;
	}
};

static const int inventorySize = 6;

struct Persistent {
	// Generic
	Gender _gender;
	Common::U32String _heroName;
	Common::U32String _slotDescription; // valid only in saves
	Quest _quest;
	int _powerLevel[3];
	RoomId _currentRoomId;
	RoomId _previousRoomId;
	bool _roomVisited[kNumRooms];
	bool _statuesTouched[kNumStatues];
	int _statuePhase[kNumStatues];
	bool _doQuestIntro;
	InventoryItem _inventory[inventorySize];
	bool _hintsAreEnabled;

	// Argo
	bool _argoSailedInQuest[kNumRooms][kNumQuests];
	bool _argoSaidTroyFinally;
	bool _argoSaidCretePort;

	// Crete and Minos
	bool _creteShowMerchant;
	bool _creteShowAtlantisBoat;
	bool _creteShowHorned;
	bool _creteShowHornless1;
	bool _creteShowHornless2;
	bool _creteShowHornless3;
	bool _creteShowHornless4;
	bool _creteDaedalusRoomAvailable;
	bool _creteMinosInstructed;
	bool _creteIntroMerchant;
	bool _cretePlayedEyeGhostTown;
	bool _creteTriedHornless[4];
	bool _creteIntroAtlantisBoat;
	bool _creteIntroAtlantisWood;
	bool _creteAlchemistExploded;
	enum CreteSandalsState {
		SANDALS_NOT_SOLVED,
		SANDALS_SOLVED,
		SANDALS_TAKEN
	} _creteSandalsState;
	enum CreteStrongBoxState {
	    BOX_CLOSED, BOX_OPEN,
	    BOX_OPEN_POTION, BOX_OPEN_NO_POTION } _creteStrongBoxState;
	bool _cretePlayedPhilAlchemist;
	bool _cretePlayedZeusCheckOutThatBox;
	bool _creteHadesPusnishesPainAndPanic;
	bool _creteVisitedAfterAlchemistIntro;
	bool _creteSaidHelenPermanentResident;

	// Daedalus
	bool _daedalusShowedNote;
	bool _daedalusLabItem[4];

	// Seriphos
	bool _seriphosStrawCartTaken;
	bool _seriphosPlayedMedusa;
	bool _seriphosPhilWarnedAthena;
	bool _seriphosPhilCurtainsItems;

	// Athena
	bool _athenaPuzzleSolved;
	bool _athenaSwordTaken;
	bool _athenaShieldTaken;
	bool _athenaPlayedPainAndPanic;
	bool _athenaIntroPlayed;

	// Medusa Island
	bool _medisleStoneTaken;
	bool _medislePlacedItems[5];
	bool _medislePlayedPerseusIntro;
	bool _medisleShowFates;
	bool _medisleShowFatesIntro;
	bool _medisleEyeballIsActive;
	FateId _medisleEyePosition;
	enum MedisleBagPuzzleState {
		BAG_NOT_STARTED,
		BAG_STARTED,
		BAG_SOLVED,
		BAG_TAKEN
	} _medisleBagPuzzleState;
	bool _medislePlayedPhilFatesDesc;

	// Troy
	bool _troyPlayAttack;
	bool _troyWallDamaged;
	bool _troyShowBricks;
	bool _troyIsDefeated;
	bool _troyPlayedOdysseus;
	bool _troyMessageIsDelivered;
	enum TroyKeyAndDecreeState {
		KEY_AND_DECREE_NOT_GIVEN,
		KEY_AND_DECREE_THROWN,
		KEY_AND_DECREE_TAKEN
	} _troyKeyAndDecreeState;
	int _troyCatacombCounter;
	bool _troyCatacombsUnlocked;
	bool _troyPlayedOdysseusCongrats;
	bool _troyPlayFinish;

	// Catacombs
	int _catacombVariants[3][3];
	CatacombsPath _catacombPaths[3][3];
	CatacombsLevel _catacombLevel;
	CatacombsPosition _catacombDecoderSkullPosition;
	CatacombsLevel _catacombLastLevel;
	bool _catacombPainAndPanic;

	// Volcano
	bool _volcanoPainAndPanicIntroDone;
	bool _volcanoHeyKid;
	enum VolcanoPuzzleState {
		VOLCANO_NO_BOULDERS_THROWN,
		VOLCANO_SQUASHED_PANIC,
		VOLCANO_BOULDER_ON_VOLCANO,
		VOLCANO_HELMET_SHOWN
	} _volcanoPuzzleState;
	int _volcanoToStyxCounter;

	// River Styx
	bool _styxCharonUsedPotion;
	bool _styxCharonUsedCoin;
	bool _styxAlchemistSaidIntro;

	Persistent();

	bool isInInventory(InventoryItem item);
	
	bool isRoomVisited(RoomId id) const {
		return _roomVisited[id];
	}

	void clearInventory();

	bool syncGameStream(Common::Serializer &s);
};
}
#endif
