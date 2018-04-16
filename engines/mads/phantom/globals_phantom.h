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
 */

#ifndef MADS_GLOBALS_PHANTOM_H
#define MADS_GLOBALS_PHANTOM_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/game.h"
#include "mads/resources.h"

namespace MADS {

namespace Phantom {

enum GlobalId {
	// Global variables

	kWalkerTiming           = 0,
	kWalkerTiming2			= 1,
	kStopWalkerDisabled		= 2,	// disable walker idle animations
	kTempInterface          = 3,
	kWalkerConverse         = 4,	// conversation started with an NPC
	kWalkerConverseState    = 5,
	kWalkerConverseNow      = 6,

	kCurrentYear            = 10,	// current year (1881 or 1993)
	kMusicSelected          = 11,
	kPlayerScore            = 12,
	kPlayerScoreFlags       = 13,
	kDoneBrieConv203        = 14,
	kLanternStatus          = 15,

	// Section #1 variables
	kLeaveAngelMusicOn      = 19,
	kTrapDoorStatus         = 20,
	kChristineDoorStatus    = 21,
	kSandbagStatus          = 22,
	kChrisFStatus           = 23,
	kBrieTalkStatus         = 24,
	kJuliesDoor             = 25,
	kPrompterStandStatus    = 26,
	kChrisDStatus           = 27,
	kJulieNameIsKnown       = 28,
	kChrisKickedRaoulOut    = 29,
	kJacquesNameIsKnown     = 30,
	kJacquesStatus          = 31,
	kFlorentNameIsKnown     = 32,
	kCharlesNameIsKnown     = 33,
	kRoom103104Transition   = 34,
	kObservedPhan104        = 35,
	kDeathLocation          = 36,
	kMakeBrieLeave203       = 37,
	kHintThatDaaeIsHome1    = 38,
	kHintThatDaaeIsHome2    = 39,

	// Section #2 variables
	kChristineToldEnvelope  = 40,
	kReadBook               = 41,
	kScannedBookcase        = 42,
	kRanConvIn205           = 43,
	kDoorsIn205             = 44,
	kPanelIn206             = 45,
	kMadameNameIsKnown      = 46,
	kMadameGiryLocation     = 47,
	kLookedAtCase           = 48,
	kMadameGiryShowsUp      = 49,
	kDoneRichConv203        = 50,
	kCameFromFade           = 51,
	kTicketPeoplePresent    = 52,
	kDegasNameIsKnown       = 53,
	kTempVar                = 54,
	kFlickedLever1          = 55,
	kFlickedLever2          = 56,
	kFlickedLever3          = 57,
	kFlickedLever4          = 58,

	// Section #3 Variables
	kTopFloorLocked         = 60,

	// Section #4 Variables
	kCatacombsRoom          = 80,
	kCatacombsMisc          = 81,
	kCatacombsFlag          = 82,
	kCatacombsFrom          = 83,
	kCatacombs309           = 84,
	kCatacombs409a          = 85,
	kCatacombs409b          = 86,
	kCatacombs501           = 87,
	kCatacombs309From       = 88,
	kCatacombs409aFrom      = 89,
	kCatacombs409bFrom      = 90,
	kCatacombs501From       = 91,
	kCatacombsNextRoom      = 92,
	kDoorIn409IsOpen        = 93,
	kPriestPistonPuke       = 94,
	kCobwebIsCut            = 95,

	// Section #5 Variables
	kChristineIsInBoat      = 100,
	kChrisWillTakeSeat      = 101,
	kRightDoorIsOpen504     = 102,
	kCoffinStatus           = 103,
	kChrisLeft505           = 104,
	kKnockedOverHead        = 105,
	kFightStatus            = 106,
	kHeListened             = 107,
	kCanFindBookInLibrary   = 108,
	kRingIsOnFinger         = 109,
	kLookedAtSkullFace      = 110,
	kCableHookWasSeparate   = 111,
	kMakeRichLeave203       = 112
};

class PhantomGlobals : public Globals {
public:
	SynchronizedList _spriteIndexes;
	SynchronizedList _sequenceIndexes;
	SynchronizedList _animationIndexes;
public:
	/**
	 * Constructor
	 */
	PhantomGlobals();

	/**
	* Synchronize the globals data
	*/
	virtual void synchronize(Common::Serializer &s);
};

} // End of namespace Phantom

} // End of namespace MADS

#endif /* MADS_GLOBALS_PHANTOM_H */
