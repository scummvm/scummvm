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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_AWAYMISSION_H
#define STARTREK_AWAYMISSION_H

// All variables here get cleared to 0 upon starting an away mission.
struct AwayMission {
	// These timers count down automatically when nonzero. When they reach 0,
	// ACTION_TIMER_EXPIRED is invoked with the corresponding index (0-7).
	int16 timers[8]; // 0x00-0x0f

	int16 mouseX; // 0x10
	int16 mouseY; // 0x12
	int16 crewGetupTimers[4]; // 0x14
	bool disableWalking; // 0x1c
	byte disableInput; // 0x1d; Set while beaming in or walking into a room. Disables control?
	bool redshirtDead; // 0x1e
	byte activeAction; // 0x1f
	byte activeObject;  // 0x20; The item that is going to be used on something
	byte passiveObject; // 0x21; The item that the active item is used on (or the item looked at, etc).

	// If this is true after calling room-specific RDF code, the game will continue to run
	// any "default" code for the event, if any.
	bool rdfStillDoDefaultAction; // 0x23

	// If a bit in "crewDownBitset" is set, the corresponding timer in "crewGetupTimers"
	// begins counting down. When it reaches 0, they get up.
	byte crewDownBitset; // 0x24

	int8 crewDirectionsAfterWalk[4]; // 0x25: Sets an object's direction after they finish walking somewhere?

	// Mission-specific variables
	union {
		// Demon World (TODO: label remaining generic variables)
		struct {
			bool wasRudeToPrelate; // 0x29
			bool insultedStephen; // 0x2b
			bool field2d; // 0x2d
			bool beatKlingons; // 0x2f
			bool tookKlingonHand; // 0x31

			bool talkedToPrelate; // 0x33
			bool stephenWelcomedToStudy; // 0x34
			bool prelateWelcomedCrew; // 0x35
			bool askedPrelateAboutSightings; // 0x36
			byte field37; // 0x37
			bool mccoyMentionedFlora; // 0x38
			byte numBouldersGone; // 0x39
			byte enteredFrom; // 0x3a
			bool repairedHand; // 0x3b
			bool healedMiner; // 0x3c
			bool curedChub; // 0x3d
			bool field3e; // 0x3e
			bool knowAboutHypoDytoxin; // 0x3f
			bool minerDead; // 0x40
			byte field41; // 0x41
			bool foundMiner; // 0x43
			bool field45; // 0x45
			bool gaveSkullToNauian; // 0x47
			bool warpsDisabled; // 0x48
			bool boulder1Gone; // 0x49
			bool boulder2Gone; // 0x4a
			bool boulder3Gone; // 0x4b
			bool boulder4Gone; // 0x4c
			bool doorOpened; // 0x4d
			bool solvedSunPuzzle; // 0x4e
			byte itemsTakenFromCase; // 0x4f
			bool gotBerries; // 0x50
			bool madeHypoDytoxin; // 0x51
			bool metNauian; // 0x53
			bool gavePointsForDytoxin; // 0x54
			bool lookedAtComputer; // 0x55
			byte field56; // 0x56
			bool foundAlienRoom; // 0x57
			int16 missionScore; // 0x58
		} demon;

		// Hijacked
		struct {
			int16 missionScore; // 0x29
			int16 field2b; // 0x2b
			int16 field2d; // 0x2d
			bool engineerConscious; // 0x30
			byte field35; // 0x35
			bool gotWires; // 0x37
			byte orbitalDecayCounter; // 0x3b
			bool bridgeElasiDrewPhasers; // 0x3d
			bool talkedToCereth; // 0x3e
			bool gotJunkPile; // 0x3f
			bool gotTransmogrifier; // 0x43
			bool transporterRepaired; // 0x44
			bool spockExaminedTransporter; // 0x45
			bool usedTransmogrifierOnTransporter; // 0x46
			bool bridgeForceFieldDown; // 0x47
			bool savedPrisoners; // 0x48
			bool haveBomb; // 0x49
			bool brigElasiPhasersOnKill; // 0x4a
			byte field4b; // 0x4b
			byte guard1Status; // 0x4c
			byte guard2Status; // 0x4d
			byte field4e; // 0x4e
			byte crewmanKilled[4]; // 0x4f
			byte bridgeElasi1Status; // 0x53
			byte bridgeElasi2Status; // 0x54
			byte bridgeElasi3Status; // 0x55
			byte bridgeElasi4Status; // 0x56
			bool brigForceFieldDown; // 0x58
			byte field59; // 0x59
			byte field5b; // 0x5b
			bool elasiSurrendered; // 0x5c
			byte kirkPhaserDrawn; // 0x5d

			// 1: Decided to shoot them
			// 2: Talked, and they surrendered immediately
			// 3: They deorbited the ship (and perhps surrendered after)
			byte bridgeWinMethod; // 0x5e

			bool talkedToBrigCrewman; // 0x5f
		} tug;

		// Love's Labor Jeopardized
		struct {
			bool alreadyStartedMission; // 0x29
			bool knowAboutVirus; // 0x2a
			bool field2b; // 0x2b
			bool field2c; // 0x2c
			bool field2d; // 0x2d
			bool chamberHasCure; // 0x2e
			bool freezerOpen; // 0x2f
			bool chamberHasDish; // 0x30
			byte bottleInNozzle; // 0x31
			bool cabinetOpen; // 0x32
			bool gasFeedOn; // 0x33
			byte synthesizerProduct; // 0x34
			byte field35; // 0x35
			byte canister1; // 0x36
			byte canister2; // 0x37
			bool field41; // 0x41
			bool spockAccessedConsole; // 0x49
			bool mccoyAccessedConsole; // 0x4a
			bool gotPolyberylcarbonate; // 0x4b
			bool gotTLDH; // 0x4c (Got romulan laughing gas)
			bool gotCure; // 0x4f
			int16 missionScore; // 0x52
		} love;
	};
};
// Size: 0x129 bytes

// Bottle types for Love's Labor Jeopardized
enum BottleTypes {
	BOTTLETYPE_NONE = 0,
	BOTTLETYPE_N2O = 1,
	BOTTLETYPE_NH3 = 2,
	BOTTLETYPE_H2O = 3,
	BOTTLETYPE_RLG = 4 // Romulan Laughing Gas
};

// Canister types for Love's Labor Jeopardized
enum CanisterTypes {
	CANTYPE_NONE = 0,
	CANTYPE_O2 = 1,
	CANTYPE_H2 = 2,
	CANTYPE_N2 = 3
};

#endif
