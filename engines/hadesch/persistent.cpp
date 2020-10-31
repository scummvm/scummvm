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
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/macresman.h"
#include "common/util.h"

#include "hadesch/persistent.h"

namespace Hadesch {

Persistent::Persistent() {
	_currentRoomId = kInvalidRoom;
	_previousRoomId = kInvalidRoom;
	_quest = kNoQuest;
	for (unsigned i = 0; i < ARRAYSIZE(_powerLevel); i++)
		_powerLevel[i] = 0;
	_hintsAreEnabled = true;

	for (unsigned i = 0; i < ARRAYSIZE(_roomVisited); i++)
		_roomVisited[i] = false;
	for (unsigned i = 0; i < ARRAYSIZE(_argoSailedInQuest); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_argoSailedInQuest[0]); j++)
			_argoSailedInQuest[i][j] = false;
	for (unsigned i = 0; i < ARRAYSIZE(_statuesTouched); i++)
		_statuesTouched[i] = false;
	for (unsigned i = 0; i < ARRAYSIZE(_statuePhase); i++)
		_statuePhase[i] = 0;

	_argoSaidTroyFinally = false;
	_argoSaidCretePort = false;

	_creteShowMerchant = false;
	_creteShowAtlantisBoat = false;
	_creteShowHorned = false;
	_creteShowHornless1 = false;
	_creteShowHornless2 = false;
	_creteShowHornless3 = false;
	_creteShowHornless4 = false;
	_creteDaedalusRoomAvailable = false;
	_creteMinosInstructed = false;
	_creteIntroMerchant = false;
	_cretePlayedEyeGhostTown = false;
	_creteIntroAtlantisBoat = false;
	_creteIntroAtlantisWood = false;
	_creteSandalsState = SANDALS_NOT_SOLVED;
	_creteStrongBoxState = BOX_CLOSED;
	_creteAlchemistExploded = false;
	_cretePlayedPhilAlchemist = false;
	_cretePlayedZeusCheckOutThatBox = false;
	_creteHadesPusnishesPainAndPanic = false;
	_creteVisitedAfterAlchemistIntro = false;
	_creteSaidHelenPermanentResident = false;

	_daedalusShowedNote = false;

	_seriphosStrawCartTaken = false;
	_seriphosPlayedMedusa = false;
	_seriphosPhilWarnedAthena = false;
	_seriphosPhilCurtainsItems = false;

	_athenaPuzzleSolved = false;
	_athenaSwordTaken = false;
	_athenaShieldTaken = false;
	_athenaPlayedPainAndPanic = false;
	_athenaIntroPlayed = false;

	_medisleStoneTaken = false;
	_medislePlayedPerseusIntro = false;
	_medisleShowFates = false;
	_medisleShowFatesIntro = false;
	for (unsigned i = 0; i < ARRAYSIZE(_medislePlacedItems); i++)
		_medislePlacedItems[i] = false;
	_medisleEyeballIsActive = false;
	_medisleEyePosition = kLachesis;
	_medisleBagPuzzleState = BAG_NOT_STARTED;
	_medislePlayedPhilFatesDesc = false;

	_troyPlayAttack = false;
	_troyWallDamaged = false;
	_troyShowBricks = false;
	_troyShowBricks = false;
	_troyIsDefeated = false;
	_troyPlayedOdysseus = false;
	_troyKeyAndDecreeState = KEY_AND_DECREE_NOT_GIVEN;
	_troyMessageIsDelivered = false;
	_troyCatacombCounter = 0;
	_troyCatacombsUnlocked = false;
	_troyPlayedOdysseusCongrats = false;
	_troyPlayFinish = false;
	_doQuestIntro = false;
	_gender = kUnknown;

	for (unsigned i = 0; i < ARRAYSIZE(_catacombVariants); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_catacombVariants[0]); j++)
			_catacombVariants[i][j] = 0;
	for (unsigned i = 0; i < ARRAYSIZE(_catacombPaths); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_catacombPaths[0]); j++)
			_catacombPaths[i][j] = kCatacombsHelen;
	_catacombLevel = kCatacombLevelSign;
	_catacombLastLevel = kCatacombLevelSign;
	_catacombDecoderSkullPosition = kCatacombsLeft;
	_catacombPainAndPanic = false;

	for (unsigned i = 0; i < ARRAYSIZE(_creteTriedHornless); i++)
		_creteTriedHornless[i] = false;
	for (unsigned i = 0; i < ARRAYSIZE(_daedalusLabItem); i++)
		_daedalusLabItem[i] = false;

	_volcanoPainAndPanicIntroDone = false;
	_volcanoPuzzleState = Persistent::VOLCANO_NO_BOULDERS_THROWN;
	_volcanoHeyKid = false;
	_volcanoToStyxCounter = 0;

	_styxCharonUsedPotion = false;
	_styxCharonUsedCoin = false;
	_styxAlchemistSaidIntro = false;

	for (unsigned i = 0; i < ARRAYSIZE(_inventory); i++)
		_inventory[i] = kNone;
}

void Persistent::clearInventory() {
	memset(_inventory, 0, sizeof (_inventory));
}

bool Persistent::isInInventory(InventoryItem item) {
	for (unsigned i = 0; i < inventorySize; i++) {
		if (_inventory[i] == item) {
			return true;
		}
	}

	return false;
}

HadeschSaveDescriptor::HadeschSaveDescriptor(Common::Serializer &s, int slot) {
	s.matchBytes("hadesch", 7);
	s.syncVersion(2);
	if (s.getVersion() < 2) {
		Common::String str;
		s.syncString(str);
		_heroName = str;
		s.syncString(str);
		_slotName = str;
	} else {
		s.syncString32(_heroName);
		s.syncString32(_slotName);
	}
	s.syncAsByte(_room);
	_slot = slot;
}

bool Persistent::syncGameStream(Common::Serializer &s) {
	if(!s.matchBytes("hadesch", 7))
		return false;
	if (!s.syncVersion(2))
		return false;

	if (s.getVersion() < 2) {
		Common::String str;
		s.syncString(str);
		_heroName = str;
		s.syncString(str);
		_slotDescription = str;
	} else {
		s.syncString32(_heroName);
		s.syncString32(_slotDescription);
	}

	s.syncAsByte(_currentRoomId);
	s.syncAsByte(_previousRoomId);

	s.syncAsByte(_quest);
	for (unsigned i = 0; i < ARRAYSIZE(_powerLevel); i++)
		s.syncAsByte(_powerLevel[i]);
	s.syncAsByte(_hintsAreEnabled);

	for (unsigned i = 0; i < ARRAYSIZE(_roomVisited); i++)
		s.syncAsByte(_roomVisited[i]);
	for (unsigned i = 0; i < ARRAYSIZE(_argoSailedInQuest); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_argoSailedInQuest[0]); j++)
			s.syncAsByte(_argoSailedInQuest[i][j]);
	for (unsigned i = 0; i < ARRAYSIZE(_statuesTouched); i++)
		s.syncAsByte(_statuesTouched[i]);
	for (unsigned i = 0; i < ARRAYSIZE(_statuePhase); i++)
		s.syncAsByte(_statuePhase[i]);
	
	s.syncAsByte(_argoSaidTroyFinally);
	s.syncAsByte(_argoSaidCretePort);

	s.syncAsByte(_creteShowMerchant);
	s.syncAsByte(_creteShowAtlantisBoat);
	s.syncAsByte(_creteShowHorned);
	s.syncAsByte(_creteShowHornless1);
	s.syncAsByte(_creteShowHornless2);
	s.syncAsByte(_creteShowHornless3);
	s.syncAsByte(_creteShowHornless4);
	s.syncAsByte(_creteDaedalusRoomAvailable);
	s.syncAsByte(_creteMinosInstructed);
	s.syncAsByte(_creteIntroMerchant);
	s.syncAsByte(_cretePlayedEyeGhostTown);
	s.syncAsByte(_creteIntroAtlantisBoat);
	s.syncAsByte(_creteIntroAtlantisWood);
	s.syncAsByte(_creteSandalsState);
	s.syncAsByte(_creteStrongBoxState);
	s.syncAsByte(_creteAlchemistExploded);
	s.syncAsByte(_cretePlayedPhilAlchemist);
	s.syncAsByte(_cretePlayedZeusCheckOutThatBox);
	s.syncAsByte(_creteHadesPusnishesPainAndPanic);
	s.syncAsByte(_creteVisitedAfterAlchemistIntro);
	s.syncAsByte(_creteSaidHelenPermanentResident);

	s.syncAsByte(_daedalusShowedNote);

	s.syncAsByte(_seriphosStrawCartTaken);
	s.syncAsByte(_seriphosPlayedMedusa);
	s.syncAsByte(_seriphosPhilWarnedAthena);
	s.syncAsByte(_seriphosPhilCurtainsItems);

	s.syncAsByte(_athenaPuzzleSolved);
	s.syncAsByte(_athenaSwordTaken);
	s.syncAsByte(_athenaShieldTaken);
	s.syncAsByte(_athenaPlayedPainAndPanic);
	s.syncAsByte(_athenaIntroPlayed);

	s.syncAsByte(_medisleStoneTaken);
	s.syncAsByte(_medislePlayedPerseusIntro);
	s.syncAsByte(_medisleShowFates);
	s.syncAsByte(_medisleShowFatesIntro);
	for (unsigned i = 0; i < ARRAYSIZE(_statuePhase); i++)
		s.syncAsByte(_medislePlacedItems[i]);
	s.syncAsByte(_medisleEyeballIsActive);
	s.syncAsByte(_medisleEyePosition);
	s.syncAsByte(_medisleBagPuzzleState);
	s.syncAsByte(_medislePlayedPhilFatesDesc);

	s.syncAsByte(_troyPlayAttack);
	s.syncAsByte(_troyWallDamaged);
	s.syncAsByte(_troyShowBricks);
	s.syncAsByte(_troyShowBricks);
	s.syncAsByte(_troyIsDefeated);
	s.syncAsByte(_troyPlayedOdysseus);
	s.syncAsByte(_troyKeyAndDecreeState);
	s.syncAsByte(_troyMessageIsDelivered);
	s.syncAsByte(_troyCatacombCounter);
	s.syncAsByte(_troyCatacombsUnlocked);
	s.syncAsByte(_troyPlayedOdysseusCongrats);
	s.syncAsByte(_troyPlayFinish);

	for (unsigned i = 0; i < ARRAYSIZE(_catacombVariants); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_catacombVariants[0]); j++)
			s.syncAsByte(_catacombVariants[i][j]);
	for (unsigned i = 0; i < ARRAYSIZE(_catacombPaths); i++)
		for (unsigned j = 0; j < ARRAYSIZE(_catacombPaths[0]); j++)
			s.syncAsByte(_catacombPaths[i][j]);
	s.syncAsByte(_catacombLevel);
	s.syncAsByte(_catacombLastLevel);
	s.syncAsByte(_catacombDecoderSkullPosition);
	s.syncAsByte(_catacombPainAndPanic);

	for (unsigned i = 0; i < ARRAYSIZE(_creteTriedHornless); i++)
		s.syncAsByte(_creteTriedHornless[i]);
	for (unsigned i = 0; i < ARRAYSIZE(_daedalusLabItem); i++)
		s.syncAsByte(_daedalusLabItem[i]);

	s.syncAsByte(_volcanoPainAndPanicIntroDone);
	s.syncAsByte(_volcanoPuzzleState);
	s.syncAsByte(_volcanoHeyKid);
	s.syncAsByte(_volcanoToStyxCounter);

	s.syncAsByte(_styxCharonUsedPotion);
	s.syncAsByte(_styxCharonUsedCoin);
	s.syncAsByte(_styxAlchemistSaidIntro);

	for (unsigned i = 0; i < ARRAYSIZE(_inventory); i++)
		s.syncAsByte(_inventory[i]);

	s.syncAsByte(_gender, 1);

	debug("serialized");

	return true;
}
	
}
