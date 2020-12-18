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
// NOTE: Any changes here must be reflected in the corresponding serializer functions.
struct AwayMission {
	// These timers count down automatically when nonzero. When they reach 0,
	// ACTION_TIMER_EXPIRED is invoked with the corresponding index (0-7).
	int16 timers[8]; // 0x00-0x0f

	int16 mouseX; // 0x10
	int16 mouseY; // 0x12
	int16 crewGetupTimers[4]; // 0x14
	bool disableWalking; // 0x1c

	// 0 / false: input enabled
	// 1 / true:  input disabled, turns back on after walking or beaming into a room
	// 2:         input disabled, doesn't turn back on after walking or beaming into room
	byte disableInput; // 0x1d

	bool redshirtDead; // 0x1e
	char activeAction; // 0x1f
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

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(wasRudeToPrelate);
				ser.syncAsByte(insultedStephen);
				ser.syncAsByte(field2d);
				ser.syncAsByte(beatKlingons);
				ser.syncAsByte(tookKlingonHand);
				ser.syncAsByte(talkedToPrelate);
				ser.syncAsByte(stephenWelcomedToStudy);
				ser.syncAsByte(prelateWelcomedCrew);
				ser.syncAsByte(askedPrelateAboutSightings);
				ser.syncAsByte(field37);
				ser.syncAsByte(mccoyMentionedFlora);
				ser.syncAsByte(numBouldersGone);
				ser.syncAsByte(enteredFrom);
				ser.syncAsByte(repairedHand);
				ser.syncAsByte(healedMiner);
				ser.syncAsByte(curedChub);
				ser.syncAsByte(field3e);
				ser.syncAsByte(knowAboutHypoDytoxin);
				ser.syncAsByte(minerDead);
				ser.syncAsByte(field41);
				ser.syncAsByte(foundMiner);
				ser.syncAsByte(field45);
				ser.syncAsByte(gaveSkullToNauian);
				ser.syncAsByte(warpsDisabled);
				ser.syncAsByte(boulder1Gone);
				ser.syncAsByte(boulder2Gone);
				ser.syncAsByte(boulder3Gone);
				ser.syncAsByte(boulder4Gone);
				ser.syncAsByte(doorOpened);
				ser.syncAsByte(solvedSunPuzzle);
				ser.syncAsByte(itemsTakenFromCase);
				ser.syncAsByte(gotBerries);
				ser.syncAsByte(madeHypoDytoxin);
				ser.syncAsByte(metNauian);
				ser.syncAsByte(gavePointsForDytoxin);
				ser.syncAsByte(lookedAtComputer);
				ser.syncAsByte(field56);
				ser.syncAsByte(foundAlienRoom);
				ser.syncAsSint16LE(missionScore);
			}
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
			byte elasiTargetIndex; // 0x4b
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
			// 3: They deorbited the ship (and perhaps surrendered after)
			byte bridgeWinMethod; // 0x5e

			bool talkedToBrigCrewman; // 0x5f

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsSint16LE(missionScore);
				ser.syncAsSint16LE(field2b);
				ser.syncAsSint16LE(field2d);
				ser.syncAsByte(engineerConscious);
				ser.syncAsByte(field35);
				ser.syncAsByte(gotWires);
				ser.syncAsByte(orbitalDecayCounter);
				ser.syncAsByte(bridgeElasiDrewPhasers);
				ser.syncAsByte(talkedToCereth);
				ser.syncAsByte(gotJunkPile);
				ser.syncAsByte(gotTransmogrifier);
				ser.syncAsByte(transporterRepaired);
				ser.syncAsByte(spockExaminedTransporter);
				ser.syncAsByte(usedTransmogrifierOnTransporter);
				ser.syncAsByte(bridgeForceFieldDown);
				ser.syncAsByte(savedPrisoners);
				ser.syncAsByte(haveBomb);
				ser.syncAsByte(brigElasiPhasersOnKill);
				ser.syncAsByte(elasiTargetIndex);
				ser.syncAsByte(guard1Status);
				ser.syncAsByte(guard2Status);
				ser.syncAsByte(field4e);
				ser.syncBytes(crewmanKilled, 4);
				ser.syncAsByte(bridgeElasi1Status);
				ser.syncAsByte(bridgeElasi2Status);
				ser.syncAsByte(bridgeElasi3Status);
				ser.syncAsByte(bridgeElasi4Status);
				ser.syncAsByte(brigForceFieldDown);
				ser.syncAsByte(field59);
				ser.syncAsByte(field5b);
				ser.syncAsByte(elasiSurrendered);
				ser.syncAsByte(kirkPhaserDrawn);
				ser.syncAsByte(bridgeWinMethod);
				ser.syncAsByte(talkedToBrigCrewman);
			}
		} tug;

		// Love's Labor Jeopardized
		struct {
			bool alreadyStartedMission; // 0x29
			bool knowAboutVirus; // 0x2a
			bool romulansUnconsciousFromLaughingGas; // 0x2b
			bool releasedHumanLaughingGas; // 0x2c
			bool releasedRomulanLaughingGas; // 0x2d
			bool chamberHasCure; // 0x2e
			bool freezerOpen; // 0x2f
			bool chamberHasDish; // 0x30
			byte bottleInNozzle; // 0x31
			bool cabinetOpen; // 0x32
			bool gasFeedOn; // 0x33
			byte synthesizerBottleIndex; // 0x34
			byte synthesizerContents; // 0x35
			byte canister1; // 0x36
			byte canister2; // 0x37
			bool servicePanelOpen; // 0x38
			bool gasTankUnscrewed; // 0x39
			bool wrenchTaken; // 0x3a
			bool tookN2TankFromServicePanel; // 0x3b
			bool field3c; // 0x3c
			bool grateRemoved; // 0x3d
			bool insulationOnGround; // 0x3e
			bool visitedRoomWithRomulans; // 0x3f
			bool romulansCured; // 0x40
			bool romulansUnconsciousFromVirus; // 0x41
			bool freedMarcusAndCheever; // 0x42
			bool preaxCured; // 0x43
			byte spockInfectionCounter; // 0x45: When this reached 100, Spock dies.
			bool spockCured; // 0x46
			bool contactedEnterpriseBeforeCure; // 0x47
			bool contactedEnterpriseAfterCure; // 0x48
			bool spockAccessedConsole; // 0x49
			bool mccoyAccessedConsole; // 0x4a
			bool gotPolyberylcarbonate; // 0x4b
			bool gotTLDH; // 0x4c (Got romulan laughing gas)
			bool gotPointsForOpeningGrate; // 0x4d
			bool gotPointsForGassingRomulans; // 0x4e
			bool gotCure; // 0x4f
			bool gotPointsForHydratingPreax; // 0x50
			bool gotPointsForHydratingRomulans; // 0x51
			int16 missionScore; // 0x52

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(alreadyStartedMission);
				ser.syncAsByte(knowAboutVirus);
				ser.syncAsByte(romulansUnconsciousFromLaughingGas);
				ser.syncAsByte(releasedHumanLaughingGas);
				ser.syncAsByte(releasedRomulanLaughingGas);
				ser.syncAsByte(chamberHasCure);
				ser.syncAsByte(freezerOpen);
				ser.syncAsByte(chamberHasDish);
				ser.syncAsByte(bottleInNozzle);
				ser.syncAsByte(cabinetOpen);
				ser.syncAsByte(gasFeedOn);
				ser.syncAsByte(synthesizerBottleIndex);
				ser.syncAsByte(synthesizerContents);
				ser.syncAsByte(canister1);
				ser.syncAsByte(canister2);
				ser.syncAsByte(servicePanelOpen);
				ser.syncAsByte(gasTankUnscrewed);
				ser.syncAsByte(wrenchTaken);
				ser.syncAsByte(tookN2TankFromServicePanel);
				ser.syncAsByte(field3c);
				ser.syncAsByte(grateRemoved);
				ser.syncAsByte(insulationOnGround);
				ser.syncAsByte(visitedRoomWithRomulans);
				ser.syncAsByte(romulansCured);
				ser.syncAsByte(romulansUnconsciousFromVirus);
				ser.syncAsByte(freedMarcusAndCheever);
				ser.syncAsByte(preaxCured);
				ser.syncAsByte(spockInfectionCounter);
				ser.syncAsByte(spockCured);
				ser.syncAsByte(contactedEnterpriseBeforeCure);
				ser.syncAsByte(contactedEnterpriseAfterCure);
				ser.syncAsByte(spockAccessedConsole);
				ser.syncAsByte(mccoyAccessedConsole);
				ser.syncAsByte(gotPolyberylcarbonate);
				ser.syncAsByte(gotTLDH);
				ser.syncAsByte(gotPointsForOpeningGrate);
				ser.syncAsByte(gotPointsForGassingRomulans);
				ser.syncAsByte(gotCure);
				ser.syncAsByte(gotPointsForHydratingPreax);
				ser.syncAsByte(gotPointsForHydratingRomulans);
				ser.syncAsSint16LE(missionScore);
			}
		} love;

		struct {
			// 0: Haven't entered first room yet
			// 1: Have entered first room once
			// 2: Mudd is gone from first room
			byte muddFirstRoomState; // 0x29

			bool torpedoLoaded; // 0x33
			bool knowAboutTorpedo; // 0x34
			bool discoveredBase3System; // 0x35
			bool translatedAlienLanguage; // 0x36
			bool databaseDestroyed; // 0x37
			bool muddInDatabaseRoom; // 0x38
			bool muddCurrentlyInsane; // 0x39
			bool computerDataErasedOrDestroyed; // 0x3a
			bool muddErasedDatabase; // 0x3b

			// True if you've combined the lense + degrimer and fired it off, discovering
			// it's a weapon
			bool discoveredLenseAndDegrimerFunction; // 0x3c

			int16 torpedoStatus; // 0x3d
			bool muddUnavailable; // 0x3f
			bool muddVisitedDatabaseRoom; // 0x40
			bool accessedAlienDatabase; // 0x41
			bool tookRepairTool; // 0x42
			bool gotPointsForDownloadingData; // 0x43
			bool contactedEnterpriseFirstTime; // 0x44
			bool viewScreenEnabled; // 0x45
			bool lifeSupportMalfunctioning; // 0x46
			byte numTimesEnteredRoom5; // 0x47
			bool gotMemoryDisk; // 0x48
			bool gotLense; // 0x49
			bool gotDegrimer; // 0x4a
			bool putCapsuleInMedicalMachine; // 0x4c
			bool muddUnconscious; // 0x4d

			// 0: haven't entered room yet
			// 1: will go insane next time room is entered (if he's available)
			// 2: currently insane (or unconscious)
			// 3: cured
			byte muddInsanityState; // 0x4e

			bool muddInhaledGas; // 0x4f (mostly the same as "muddCurrentlyInsane"?)
			int16 lifeSupportTimer; // 0x50
			bool startedLifeSupportTimer; // 0x52
			bool enteredRoom0ForFirstTime; // 0x54
			bool gotPointsForLoadingTorpedo; // 0x55
			bool gotPointsForPressingRedButton; // 0x56
			bool gotPointsForEnablingViewscreen; // 0x57
			bool enteredRoom1ForFirstTime; // 0x58
			bool repairedLifeSupportGenerator; // 0x59
			int16 missionScore; // 0x5a

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(muddFirstRoomState);
				ser.syncAsByte(torpedoLoaded);
				ser.syncAsByte(knowAboutTorpedo);
				ser.syncAsByte(discoveredBase3System);
				ser.syncAsByte(translatedAlienLanguage);
				ser.syncAsByte(databaseDestroyed);
				ser.syncAsByte(muddInDatabaseRoom);
				ser.syncAsByte(muddCurrentlyInsane);
				ser.syncAsByte(computerDataErasedOrDestroyed);
				ser.syncAsByte(muddErasedDatabase);
				ser.syncAsByte(discoveredLenseAndDegrimerFunction);
				ser.syncAsSint16LE(torpedoStatus);
				ser.syncAsByte(muddUnavailable);
				ser.syncAsByte(muddVisitedDatabaseRoom);
				ser.syncAsByte(accessedAlienDatabase);
				ser.syncAsByte(tookRepairTool);
				ser.syncAsByte(gotPointsForDownloadingData);
				ser.syncAsByte(contactedEnterpriseFirstTime);
				ser.syncAsByte(viewScreenEnabled);
				ser.syncAsByte(lifeSupportMalfunctioning);
				ser.syncAsByte(numTimesEnteredRoom5);
				ser.syncAsByte(gotMemoryDisk);
				ser.syncAsByte(gotLense);
				ser.syncAsByte(gotDegrimer);
				ser.syncAsByte(putCapsuleInMedicalMachine);
				ser.syncAsByte(muddUnconscious);
				ser.syncAsByte(muddInsanityState);
				ser.syncAsByte(muddInhaledGas);
				ser.syncAsSint16LE(lifeSupportTimer);
				ser.syncAsByte(startedLifeSupportTimer);
				ser.syncAsByte(enteredRoom0ForFirstTime);
				ser.syncAsByte(gotPointsForLoadingTorpedo);
				ser.syncAsByte(gotPointsForPressingRedButton);
				ser.syncAsByte(gotPointsForEnablingViewscreen);
				ser.syncAsByte(enteredRoom1ForFirstTime);
				ser.syncAsByte(repairedLifeSupportGenerator);
				ser.syncAsSint16LE(missionScore);
			}
		} mudd;

		struct {
			bool diedFromStalactites; // 0x29
			// 0: initial state
			// 1: one rock thrown at it
			// 2: two rocks thrown at it (low enough to climb up)
			byte vineState; // 0x2a

			bool gotRock; // 0x2b
			bool gotSnake; // 0x2c
			bool tookKnife; // 0x2d
			bool field2e; // 0x2e
			byte numRocksThrownAtTlaoxac; // 0x2f
			bool gotFern; // 0x30
			bool holeBlocked; // 0x31
			bool tlaoxacTestPassed; // 0x32
			bool knockedOutTlaoxac; // 0x33
			bool waterMonsterRetreated; // 0x34
			bool showedSnakeToTlaoxac; // 0x35
			int16 missionScore; // 0x36

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(diedFromStalactites);
				ser.syncAsByte(vineState);
				ser.syncAsByte(gotRock);
				ser.syncAsByte(gotSnake);
				ser.syncAsByte(tookKnife);
				ser.syncAsByte(field2e);
				ser.syncAsByte(numRocksThrownAtTlaoxac);
				ser.syncAsByte(gotFern);
				ser.syncAsByte(holeBlocked);
				ser.syncAsByte(tlaoxacTestPassed);
				ser.syncAsByte(knockedOutTlaoxac);
				ser.syncAsByte(waterMonsterRetreated);
				ser.syncAsByte(showedSnakeToTlaoxac);
				ser.syncAsSint16LE(missionScore);
			}
		} feather;

		struct {
			int16 missionScore; // 0x29
			int16 field2b; // 0x2b
			bool entityDefeated; // 0x31: Used iron rod on the energy being
			bool doorOpen; // 0x32
			bool scannedLock; // 0x33

			// 0: Don't know the door code yet
			// 2: Will just open the door when the keypad is used
			// 5: Will activate the unknown program when the keypad is used
			byte doorCodeBehaviour; // 0x34

			bool globSplitInTwo; // 0x35
			bool globDefeated; // 0x36
			byte globEnergyLevels[3]; // 0x37
			bool enteredTrial3FirstTime; // 0x3a
			byte klingonShootIndex; // 0x3b
			byte shotKlingons; // 0x3c

			// 0: Hasn't appeared yet
			// 21: Is conscious
			// 22: Is stunned
			// 23: Is dead
			int16 shotKlingonState; // 0x3d

			bool neuralInterfaceActive; // 0x43
			int16 holeContents[3]; // 0x44 (Holes to put gems in for TRIAL5)

			bool enteredGlobRoom; // 0x5c
			bool forceFieldDown; // 0x5d
			bool uhuraAnalyzedCode; // 0x5e

			// 0: Gave up in court, letting Quetzecoatl die
			// 1: Beamed to enterprise after calling Uhura
			// 2: Lost the court battle after beaming back to the courtroom
			// 3: Quetzecoatl goes free and Vlict doesn't die
			// 4: Quetzecoatl goes free and Vlict dies
			int16 missionEndMethod; // 0x5f

			bool gotPointsForGettingRod; // 0x61
			bool gotPointsForCoatingRodWithIron; // 0x62
			bool gotPointsForActivatingInterface; // 0x63
			bool gotPointsForScanningGlob; // 0x64

			bool gotPointsForBeamingOut;

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsSint16LE(missionScore);
				ser.syncAsSint16LE(field2b);
				ser.syncAsByte(entityDefeated);
				ser.syncAsByte(doorOpen);
				ser.syncAsByte(scannedLock);
				ser.syncAsByte(doorCodeBehaviour);
				ser.syncAsByte(globSplitInTwo);
				ser.syncAsByte(globDefeated);
				ser.syncBytes(globEnergyLevels, 3);
				ser.syncAsByte(enteredTrial3FirstTime);
				ser.syncAsByte(klingonShootIndex);
				ser.syncAsByte(shotKlingons);
				ser.syncAsSint16LE(shotKlingonState);
				ser.syncAsByte(neuralInterfaceActive);
				for (int i = 0; i < 3; i++)
					ser.syncAsSint16LE(holeContents[i]);
				ser.syncAsByte(enteredGlobRoom);
				ser.syncAsByte(forceFieldDown);
				ser.syncAsByte(uhuraAnalyzedCode);
				ser.syncAsSint16LE(missionEndMethod);
				ser.syncAsByte(gotPointsForGettingRod);
				ser.syncAsByte(gotPointsForCoatingRodWithIron);
				ser.syncAsByte(gotPointsForActivatingInterface);
				ser.syncAsByte(gotPointsForScanningGlob);
				ser.syncAsByte(gotPointsForBeamingOut);
			}
		} trial;

		struct {
			byte field31; // 0x31
			byte field32; // 0x32
			bool field33; // 0x33
			bool doorLaserFiredOnce; // 0x34
			bool gotPointsForAccessingTerminal; // 0x35
			bool scannedKeycardLock; // 0x36
			byte laserSetting; // 0x37

			// 0 if the laser hasn't been programmed with the keycard template;
			// 1 if it has been programmed with the template;
			// 2 if the rock has been placed on the wall.
			byte laserPattern; // 0x38

			// bit 0: got a rock, or at least scanned the ground outside
			// bit 1: entered mineshaft room
			// bit 2: scanned the ID card panel
			// bit 3: set after all 3 clues are obtained and Spock explains how to make
			//        the keycard
			byte gatheredClues; // 0x39

			// bit 0: scanned left computer
			// bit 1: scanned right computer
			// bit 2: accessed left computer
			// bit 3: accessed right computer
			byte scannedAndUsedComputers; // 0x3a

			bool wireConnected1; // 0x3b
			bool wireConnected2; // 0x3c
			bool openedOuterDoor; // 0x3d
			bool openedInnerDoor; // 0x3e
			bool unlockedIDCardDoor; // 0x3f

			// 0: mold hasn't been created
			// 2: mold for the keycard has been etched into the rock
			// 3: rock placed top of the mold
			// 4: a keycard is there
			int8 moldState; // 0x40

			// 0: box closed
			// 1: box open
			// 2: box empty
			byte boxState; // 0x41

			bool enteredRoom0FirstTime; // 0x42
			bool scottyInformedKirkAboutVirus; // 0x43
			bool enteredRoom2FirstTime; // 0x44
			bool enteredRoom3FirstTime; // 0x45
			bool enteredRoom4FirstTime; // 0x46
			bool enteredRoom5FirstTime; // 0x47
			bool gotPointsForScanningStatue; // 0x48
			bool gotPointsForScanningRoom4; // 0x49
			bool gotPointsForScanningRoom5; // 0x4a
			bool gotPointsForScanningRightComputer; // 0x4b
			bool gotPointsForScanningLeftComputer; // 0x4c
			bool gotPointsForUsingRightComputer; // 0x4d
			bool gotPointsForUsingLeftComputer; // 0x4e
			bool discoveredComputersOutOfSync; // 0x4f
			bool enteredRoom1FirstTime; // 0x50
			bool playedMusicUponEnteringRoom5FirstTime; // 0x51
			int16 missionScore; // 0x52

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(field31);
				ser.syncAsByte(field32);
				ser.syncAsByte(field33);
				ser.syncAsByte(doorLaserFiredOnce);
				ser.syncAsByte(gotPointsForAccessingTerminal);
				ser.syncAsByte(scannedKeycardLock);
				ser.syncAsByte(laserSetting);
				ser.syncAsByte(laserPattern);
				ser.syncAsByte(gatheredClues);
				ser.syncAsByte(scannedAndUsedComputers);
				ser.syncAsByte(wireConnected1);
				ser.syncAsByte(wireConnected2);
				ser.syncAsByte(openedOuterDoor);
				ser.syncAsByte(openedInnerDoor);
				ser.syncAsByte(unlockedIDCardDoor);
				ser.syncAsByte(moldState);
				ser.syncAsByte(boxState);
				ser.syncAsByte(enteredRoom0FirstTime);
				ser.syncAsByte(scottyInformedKirkAboutVirus);
				ser.syncAsByte(enteredRoom2FirstTime);
				ser.syncAsByte(enteredRoom3FirstTime);
				ser.syncAsByte(enteredRoom4FirstTime);
				ser.syncAsByte(enteredRoom5FirstTime);
				ser.syncAsByte(gotPointsForScanningStatue);
				ser.syncAsByte(gotPointsForScanningRoom4);
				ser.syncAsByte(gotPointsForScanningRoom5);
				ser.syncAsByte(gotPointsForScanningRightComputer);
				ser.syncAsByte(gotPointsForScanningLeftComputer);
				ser.syncAsByte(gotPointsForUsingRightComputer);
				ser.syncAsByte(gotPointsForUsingLeftComputer);
				ser.syncAsByte(discoveredComputersOutOfSync);
				ser.syncAsByte(enteredRoom1FirstTime);
				ser.syncAsByte(playedMusicUponEnteringRoom5FirstTime);
				ser.syncAsSint16LE(missionScore);
			}
		} sins;

		struct {
			bool usedMedkitOnBrittany; // 0x29
			bool talkedToBrittany; // 0x2a
			bool brittanyDead; // 0x2b
			bool lookedAtBrittany; // 0x2c
			bool enteredRoom0FirstTime; // 0x2d
			int16 missionScore; // 0x2e
			bool bridgeCrewmanDead; // 0x30
			bool scannedCaptainsChair; // 0x31
			bool scannedComputerBank; // 0x32
			bool showedRepublicMapFirstTime; // 0x33
			bool impulseEnginesOn; // 0x34

			// 0: beam still in place
			// 1: used saw on beam
			// 2: beam taken
			byte beamState; // 0x35

			bool enterpriseLeftForDistressCall; // 0x36
			bool openedPanel; // 0x37
			bool clearedPanelDebris; // 0x38
			bool removedPanelDebris; // 0x39
			bool oilInHypo; // 0x3a
			bool loosenedDebrisWithOil; // 0x3b

			// 0: no power allocated
			// 1: power to weapons
			// 2: power to shields
			// 3: power to transporter
			byte poweredSystem; // 0x3c

			// 0: no tricorders plugged in
			// 1: med tricorder plugged in
			// 2: sci tricorder plugged in
			// 3: both plugged in
			byte tricordersPluggedIntoComputer; // 0x3d

			byte captainsLogIndex; // 0x3e
			bool elasiShieldsDown; // 0x40
			bool torpedoLoaded; // 0x41
			bool firedTorpedo; // 0x42
			bool elasiShipDecloaked; // 0x43
			bool countdownStarted; // 0x44
			bool toldElasiToBeamOver; // 0x45
			bool scannedMainComputer; // 0x46
			bool elasiHailedRepublic; // 0x47
			bool tookRecordDeckFromAuxilaryControl; // 0x48

			// Counter used when shields are down and Elasi are present
			int16 counterUntilElasiBoardWithShieldsDown; // 0x49

			// Counter used after talking with Elasi and they give you time to recover the
			// data
			int16 counterUntilElasiAttack; // 0x4b

			// Counter used after telling Elasi you'll turn off shields (initial warning)
			int16 counterUntilElasiNagToDisableShields; // 0x4d

			// Counter used after telling Elasi you'll turn off shields (they destroy the
			// ship this time)
			int16 counterUntilElasiDestroyShip; // 0x4f

			// Counter used after telling Elasi to beam over
			int16 counterUntilElasiBoardWithInvitation; // 0x51

			bool putSupportBeamInSickbayHallway; // 0x53
			bool clearedDebris; // 0x54
			bool lookedAtTurbolift2Door; // 0x55
			bool kirkShouldSuggestReestablishingPower; // 0x56
			bool tookHypoFromSickbay; // 0x57
			bool tookDrillFromSickbay; // 0x58
			bool clearedDebrisInRoom5; // 0x59
			bool havePowerPack; // 0x5a
			bool enteredRoom5FirstTime; // 0x5b
			bool askedSpockAboutFire; // 0x5c

			// bits 0-1 are set for each of the 2 cables that can be connected to the
			// junction box in engineering.
			byte junctionCablesConnected; // 0x5d

			bool tookCableFromTransporterRoomHallway; // 0x5e
			bool tookCableFromSickbayHallway; // 0x5f
			bool tookMolecularSaw; // 0x60
			bool readEngineeringJournal; // 0x61
			bool tookEngineeringJournal; // 0x62
			bool engineeringCabinetOpen; // 0x63
			bool setTransporterCoordinates; // 0x65
			bool examinedTorpedoControl; // 0x66
			bool powerPackPluggedIntoTransporter; // 0x67
			bool field68; // 0x68
			bool getPointsForHealingBrittany; // 0x69
			bool readAllLogs; // 0x6a
			bool field6b; // 0x6b
			bool field6c; // 0x6c

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(usedMedkitOnBrittany);
				ser.syncAsByte(talkedToBrittany);
				ser.syncAsByte(brittanyDead);
				ser.syncAsByte(lookedAtBrittany);
				ser.syncAsByte(enteredRoom0FirstTime);
				ser.syncAsSint16LE(missionScore);
				ser.syncAsByte(bridgeCrewmanDead);
				ser.syncAsByte(scannedCaptainsChair);
				ser.syncAsByte(scannedComputerBank);
				ser.syncAsByte(impulseEnginesOn);
				ser.syncAsByte(beamState);
				ser.syncAsByte(enterpriseLeftForDistressCall);
				ser.syncAsByte(openedPanel);
				ser.syncAsByte(clearedPanelDebris);
				ser.syncAsByte(removedPanelDebris);
				ser.syncAsByte(oilInHypo);
				ser.syncAsByte(loosenedDebrisWithOil);
				ser.syncAsByte(poweredSystem);
				ser.syncAsByte(tricordersPluggedIntoComputer);
				ser.syncAsByte(captainsLogIndex);
				ser.syncAsByte(elasiShieldsDown);
				ser.syncAsByte(torpedoLoaded);
				ser.syncAsByte(firedTorpedo);
				ser.syncAsByte(elasiShipDecloaked);
				ser.syncAsByte(countdownStarted);
				ser.syncAsByte(toldElasiToBeamOver);
				ser.syncAsByte(scannedMainComputer);
				ser.syncAsByte(elasiHailedRepublic);
				ser.syncAsByte(tookRecordDeckFromAuxilaryControl);
				ser.syncAsSint16LE(counterUntilElasiBoardWithShieldsDown);
				ser.syncAsSint16LE(counterUntilElasiAttack);
				ser.syncAsSint16LE(counterUntilElasiNagToDisableShields);
				ser.syncAsSint16LE(counterUntilElasiDestroyShip);
				ser.syncAsSint16LE(counterUntilElasiBoardWithInvitation);
				ser.syncAsByte(clearedDebris);
				ser.syncAsByte(lookedAtTurbolift2Door);
				ser.syncAsByte(kirkShouldSuggestReestablishingPower);
				ser.syncAsByte(tookHypoFromSickbay);
				ser.syncAsByte(tookDrillFromSickbay);
				ser.syncAsByte(clearedDebrisInRoom5);
				ser.syncAsByte(havePowerPack);
				ser.syncAsByte(enteredRoom5FirstTime);
				ser.syncAsByte(askedSpockAboutFire);
				ser.syncAsByte(junctionCablesConnected);
				ser.syncAsByte(tookCableFromTransporterRoomHallway);
				ser.syncAsByte(tookCableFromSickbayHallway);
				ser.syncAsByte(tookMolecularSaw);
				ser.syncAsByte(readEngineeringJournal);
				ser.syncAsByte(tookEngineeringJournal);
				ser.syncAsByte(engineeringCabinetOpen);
				ser.syncAsByte(setTransporterCoordinates);
				ser.syncAsByte(examinedTorpedoControl);
				ser.syncAsByte(powerPackPluggedIntoTransporter);
				ser.syncAsByte(field68);
				ser.syncAsByte(getPointsForHealingBrittany);
				ser.syncAsByte(readAllLogs);
				ser.syncAsByte(field6b);
				ser.syncAsByte(field6c);
			}
		} veng;
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

// Synthesizer contents for Love's Labor Jeopardized (values for synthesizerContents)
enum SynthesizerContent {
	SYNTHITEM_NONE = 0,
	SYNTHITEM_PBC = 1,
	SYNTHITEM_VIRUS_SAMPLE = 2,
	SYNTHITEM_CURE_SAMPLE = 3,
	SYNTHITEM_BOTTLE = 9 // Contents of bottle determined by "synthesizerBottleIndex" variable
};

#endif
