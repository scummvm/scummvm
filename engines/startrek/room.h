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

#ifndef STARTREK_ROOM_H
#define STARTREK_ROOM_H

#include "common/rect.h"
#include "common/ptr.h"
#include "common/str.h"

#include "startrek/startrek.h"
#include "startrek/text.h"

using Common::SharedPtr;


namespace StarTrek {

class StarTrekEngine;
class Room;

// Per-room action functions
struct RoomAction {
	const Action action;
	void (Room::*funcPtr)();
};

// Offsets of data in RDF files

const int RDF_WARP_ROOM_INDICES = 0x22;
const int RDF_ROOM_ENTRY_POSITIONS = 0x2a;
const int RDF_BEAM_IN_POSITIONS = 0xaa;

class Room {

public:
	Room(StarTrekEngine *vm, const Common::String &name);
	~Room();

	// Helper stuff for RDF access
	uint16 readRdfWord(int offset);

	// Scale-related stuff (rename these later)
	uint16 getVar06() { return readRdfWord(0x06); }
	uint16 getVar08() { return readRdfWord(0x08); }
	uint16 getVar0a() { return readRdfWord(0x0a); }
	uint16 getVar0c() { return readRdfWord(0x0c); }

	// words 0x0e and 0x10 in RDF file are pointers to start and end of event code.
	// That code is instead rewritten on a per-room basis.
	bool actionHasCode(const Action &action);
	bool handleAction(const Action &action);

	// Same as above, but if any byte in the action is -1 (0xff), it matches any value.
	bool handleActionWithBitmask(const Action &action);

	uint16 getFirstHotspot() { return readRdfWord(0x12); }
	uint16 getHotspotEnd()   { return readRdfWord(0x14); }

	// Warp-related stuff
	uint16 getFirstWarpPolygonOffset() { return readRdfWord(0x16); }
	uint16 getWarpPolygonEndOffset()   { return readRdfWord(0x18); }
	uint16 getFirstDoorPolygonOffset() { return readRdfWord(0x1a); }
	uint16 getDoorPolygonEndOffset()   { return readRdfWord(0x1c); }

	Common::Point getBeamInPosition(int crewmanIndex);

public:
	byte *_rdfData;

private:
	StarTrekEngine *_vm;

	RoomAction *_roomActionList;
	int _numRoomActions;


	// Interface for room-specific code
	void loadActorAnim(int actorIndex, Common::String anim, int16 x, int16 y, uint16 field66); // Cmd 0x00
	void loadActorStandAnim(int actorIndex);                                                   // Cmd 0x01
	void loadActorAnim2(int actorIndex, Common::String anim, int16 x, int16 y, uint16 field66);// Cmd 0x02
	int showRoomSpecificText(const char **textAddr); // (Deprecated, use function below)       // Cmd 0x03
	int showText(const int *text);                                                             // Cmd 0x03
	int showText(int speaker, int text);                                                       // Cmd 0x03
	int showText(int text);                                                                    // Cmd 0x03
	void giveItem(int item);                                                                   // Cmd 0x04
	// Command 0x05: "demon4ShowSunPuzzle"
	void loadRoomIndex(int roomIndex, int spawnIndex);                                         // Cmd 0x06
	void loseItem(int item);                                                                   // Cmd 0x07
	void walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam);// Cmd 0x08
	void loadMapFile(const Common::String &name);                                              // Cmd 0x09
	// Command 0x0c: "demon6ShowCase"
	Common::Point getActorPos(int actorIndex);                                                 // Cmd 0x0d
	void playSoundEffectIndex(int soundEffect);                                                // Cmd 0x0f
	void playMidiMusicTracks(int startTrack, int loopTrack);                                   // Cmd 0x10
	void endMission(int16 score, int16 arg2, int16 arg3);                                      // Cmd 0x11
	void showGameOverMenu();                                                                   // Cmd 0x12
	void playVoc(Common::String filename);                                                     // Cmd 0x15

	// Room-specific code
public:
	// DEMON0
	void demon0Tick1();
	void demon0Tick2();
	void demon0Tick60();
	void demon0Tick100();
	void demon0Tick140();
	void demon0TouchedWarp0();
	void demon0WalkToBottomDoor();
	void demon0TouchedHotspot1();
	void demon0ReachedBottomDoor();
	void demon0WalkToTopDoor();
	void demon0TouchedHotspot0();
	void demon0ReachedTopDoor();
	void demon0TalkToPrelate();
	void demon0LookAtPrelate();
	void demon0UsePhaserOnSnow();
	void demon0UsePhaserOnSign();
	void demon0UsePhaserOnShelter();
	void demon0UsePhaserOnPrelate();
	void demon0LookAtSign();
	void demon0LookAtTrees();
	void demon0LookAtSnow();
	void demon0LookAnywhere();
	void demon0LookAtBushes();
	void demon0LookAtKirk();
	void demon0LookAtMcCoy();
	void demon0LookAtRedShirt();
	void demon0LookAtSpock();
	void demon0LookAtShelter();
	void demon0TalkToKirk();
	void demon0TalkToRedshirt();
	void demon0TalkToMcCoy();
	void demon0TalkToSpock();
	void demon0AskPrelateAboutSightings();
	void demon0UseSTricorderAnywhere();
	void demon0UseMTricorderAnywhere();
	void demon0UseMTricorderOnPrelate();
	void demon0BadResponse();

	// DEMON1
	void demon1Tick1();
	void demon1WalkToCave();
	void demon1TouchedTopWarp();
	void demon1TouchedBottomWarp();
	void demon1Timer2Expired();
	void demon1Timer0Expired();
	void demon1Timer3Expired();
	void demon1Timer1Expired();
	void demon1KlingonFinishedAimingWeapon();
	void demon1KirkShot();
	void demon1UsePhaserOnAnything();
	void demon1UsePhaserOnKlingon1();
	void demon1ShootKlingon1();
	void demon1KlingonDropsHand();
	void demon1UsePhaserOnKlingon2();
	void demon1ShootKlingon2();
	void demon1UsePhaserOnKlingon3();
	void demon1ShootKlingon3();
	void demon1AllKlingonsDead();
	void demon1Timer5Expired();
	void demon1UseMTricorderOnKlingon();
	void demon1UseSTricorderOnTulips();
	void demon1UseSTricorderOnPods();
	void demon1UseSTricorderOnCattails();
	void demon1UseSTricorderOnFerns();
	void demon1UseSTricorderOnHand();
	void demon1UseSTricorderOnKlingon1();
	void demon1UseSTricorderOnKlingon2Or3();
	void demon1UseMTricorderOnKirk();
	void demon1UseMTricorderOnSpock();
	void demon1UseMTricorderOnRedshirt();
	void demon1UseMTricorderOnCrewman();
	void demon1GetHand();
	void demon1ReachedHand();
	void demon1PickedUpHand();
	void demon1FinishedGettingHand();
	void demon1LookAtKlingon();
	void demon1LookAtCattails();
	void demon1LookAtTulips();
	void demon1LookAtPods();
	void demon1LookAtFerns();
	void demon1LookAtStream();
	void demon1LookAtMine();
	void demon1LookAtMountain();
	void demon1LookAtHand();
	void demon1LookAnywhere();
	void demon1LookAtKirk();
	void demon1LookAtSpock();
	void demon1LookAtMcCoy();
	void demon1LookAtRedshirt();
	void demon1TalkToKirk();
	void demon1TalkToSpock();
	void demon1TalkToMcCoy();
	void demon1TalkToRedshirt();
	void demon1TalkToUnconsciousCrewman();

	// DEMON2
	void demon2Tick1();
	void demon2WalkToCave();
	void demon2ReachedCave();
	void demon2TouchedWarp1();
	void demon2LookAtCave();
	void demon2LookAtMountain();
	void demon2LookAtBerries();
	void demon2LookAtFern();
	void demon2LookAtMoss();
	void demon2LookAtLights();
	void demon2LookAtAnything();
	void demon2LookAtKirk();
	void demon2LookAtSpock();
	void demon2LookAtMcCoy();
	void demon2LookAtRedshirt();
	void demon2TalkToKirk();
	void demon2TalkToSpock();
	void demon2TalkToMcCoy();
	void demon2TalkToRedshirt();
	void demon2UsePhaserOnBerries();
	void demon2UseSTricorderOnBerries();
	void demon2UseSTricorderOnMoss();
	void demon2UseSTricorderOnFern();
	void demon2UseSTricorderOnCave();
	void demon2UseMTricorderOnBerries();
	void demon2GetBerries();
	void demon2ReachedBerries();
	void demon2PickedUpBerries();

	// DEMON3
	void demon3Tick1();
	void demon3Timer0Expired();
	void demon3Timer1Expired();
	void demon3Timer3Expired();
	void demon3FinishedAnimation1();
	void demon3FinishedAnimation2();
	void demon3FinishedWalking5();
	void demon3McCoyInFiringPosition();
	void demon3SpockInFiringPosition();
	void demon3RedShirtInFiringPosition();
	void demon3KirkInFiringPosition();
	void demon3CrewmanInFiringPosition();
	void demon3PullOutPhaserAndFireAtBoulder();
	void demon3FireAtBoulder();
	void demon3UsePhaserOnRedshirt();
	void demon3UseStunPhaserOnBoulder();
	void demon3UsePhaserOnBoulder1();
	void demon3UsePhaserOnBoulder2();
	void demon3UsePhaserOnBoulder3();
	void demon3UsePhaserOnBoulder4();
	void demon3BoulderCommon();
	void demon3UseSTricorderOnMiner();
	void demon3UseSTricorderOnPanel();
	void demon3UseSTricorderOnBoulder();
	void demon3UseMTricorderOnBoulder();
	void demon3UseCrewmanOnPanel();
	void demon3UseRedshirtOnPanel();
	void demon3RedshirtReachedPanel();
	void demon3RedshirtUsedPanel();
	void demon3RedshirtElectrocuted();
	void demon3UseSTricorderOnDoor();
	void demon3UseSTricorderOnAnything();
	void demon3UseMTricorderOnDoor();
	void demon3UsePhaserOnDoor();
	void demon3UseHandOnPanel();
	void demon3KirkReachedHandPanel();
	void demon3KirkUsedHandPanel();
	void demon3UseMTricorderOnMiner();
	void demon3UseMedkitOnMiner();
	void demon3McCoyReachedMiner();
	void demon3McCoyFinishedHealingMiner();
	void demon3GetMiner();
	void demon3TalkToMiner();
	void demon3TalkToKirk();
	void demon3TalkToSpock();
	void demon3TalkToMccoy();
	void demon3TalkToRedshirt();
	void demon3LookAtKirk();
	void demon3LookAtSpock();
	void demon3LookAtMccoy();
	void demon3LookAtRedshirt();
	void demon3LookAnywhere();
	void demon3LookAtMiner();
	void demon3LookAtBoulder1();
	void demon3LookAtBoulder2();
	void demon3LookAtBoulder3();
	void demon3LookAtBoulder4();
	void demon3LookAtStructure();
	void demon3LookAtDoor();
	void demon3LookAtPanel();
	void demon3LookAtLight();

	// DEMON4
	void demon4Tick1();
	void demon4FinishedAnimation1();
	void demon4FinishedAnimation2();
	void demon4FinishedAnimation3();
	void demon4Timer0Expired();
	void demon4Timer1Expired();
	void demon4CrewmanReachedBeamoutPosition();
	void demon4Timer2Expired();
	void demon4Timer3Expired();
	void demon4Timer4Expired();
	void demon4Timer5Expired();
	void demon4Timer6Expired();
	void demon4UsePhaserOnNauian();
	void demon4UseMetalOnSecurityEquipment();
	void demon4KirkReachedSecurityEquipment();
	void demon4KirkFinishedUsingSecurityEquipment();
	void demon4UseMetalOnNauian();
	void demon4KirkReachedNauian();
	void demon4UseSkullOnNauian();
	void demon4KirkReachedNauianWithSkull();
	void demon4UsePhaserOnPanel();
	void demon4UsePhaserOnPattern();
	void demon4UsePhaserOnMccoy();
	void demon4TalkToNauian();
	void demon4LookAtPattern();
	void demon4LookAtAlien();
	void demon4LookAnywhere();
	void demon4LookAtSecurityEquipment();
	void demon4LookAtFloor();
	void demon4LookAtKirk();
	void demon4LookAtMccoy();
	void demon4LookAtSpock();
	void demon4LookAtRedshirt();
	void demon4LookAtChamber();
	void demon4LookAtPanel();
	void demon4UseKirkOnPanel();
	void demon4UseSpockOnPanel();
	void demon4UseMccoyOnPanel();
	void demon4UseRedshirtOnPanel();
	void demon4UseCrewmanOnPanel();
	void demon4CrewmanReachedPanel();
	bool demon4ShowSunPuzzle();
	void demon4TalkToKirk();
	void demon4TalkToMccoy();
	void demon4TalkToSpock();
	void demon4TalkToRedshirt();
	void demon4UseSTricorderOnChamber();
	void demon4UseSTricorderOnPattern();
	void demon4UseMTricorderOnPattern();
	void demon4UseSTricorderOnPanel();
	void demon4UseMTricorderOnPanel();
	void demon4UseSTricorderOnAnything();
	void demon4UseMTricorderOnAnything();
	void demon4UseSTricorderOnNauian();
	void demon4UseMTricorderOnNauian();
	void demon4UseSTricorderOnSecurityEquipment();
	void demon4UseMTricorderOnSecurityEquipment();

	// DEMON5
	void demon5Tick1();
	void demon5WalkToDoor();
	void demon5TouchedDoorOpenTrigger();
	void demon5DoorOpenedOrReachedDoor();
	void demon5UseSTricorderOnCrate();
	void demon5UsePhaserOnAnything();
	void demon5UseHandOnStephen();
	void demon5UseBerryOnStephen();
	void demon5UseHypoDytoxinOnChub();
	void demon5MccoyReachedChub();
	void demon5MccoyHealedChub();
	void demon5UseHypoDytoxinOnAnything();
	void demon5UseBerryOnChub();
	void demon5LookAtRoberts();
	void demon5LookAtGrisnash();
	void demon5LookAtStephen();
	void demon5LookAtKirk();
	void demon5LookAtSpock();
	void demon5LookAtMccoy();
	void demon5LookAtRedshirt();
	void demon5LookAtMountain();
	void demon5LookAtCrate();
	void demon5LookAnywhere();
	void demon5LookAtChub();
	void demon5TalkToRoberts();
	void demon5TalkToChub();
	void demon5TalkToGrisnash();
	void demon5TalkToStephen();
	void demon5TalkToKirk();
	void demon5TalkToSpock();
	void demon5TalkToRedshirt();
	void demon5TalkToMccoy();
	void demon5UseMTricorderOnRoberts();
	void demon5UseMTricorderOnChub();
	void demon5UseMTricorderOnGrisnash();
	void demon5UseMTricorderOnStephen();
	void demon5CheckCompletedStudy();
	void demon5GetCrate();

	// DEMON6
	void demon6Tick1();
	void demon6Tick30();
	void demon6SpockReachedComputer();
	void demon6WalkToDoor();
	void demon6TouchedDoorOpenTrigger();
	void demon6DoorOpenedOrReachedDoor();
	void demon6UsePhaserOnStephen();
	void demon6UsePhaserOnCase();
	void demon6LookAtWorkspace();
	void demon6LookAtCase();
	void demon6LookAtTable();
	void demon6LookAtMineral();
	void demon6LookAtShells();
	void demon6LookAtSkull();
	void demon6LookAtMetal();
	void demon6LookAtMeteor();
	void demon6LookAtMountain();
	void demon6LookAtSynthesizer();
	void demon6LookAtKirk();
	void demon6LookAtSpock();
	void demon6LookAtMccoy();
	void demon6LookAtRedshirt();
	void demon6LookAtComputer();
	void demon6LookAnywhere();
	void demon6LookAtStephen();
	void demon6TalkToMccoy();
	void demon6TalkToSpock();
	void demon6TalkToKirk();
	void demon6TalkToRedshirt();
	void demon6TalkToStephen();
	void demon6UseBerryOnStephen();
	void demon6UseBerryOnSynthesizer();
	void demon6MccoyReachedSynthesizer();
	void demon6FinishedMakingHypoDytoxin();
	void demon6UseHandOnWorkspace();
	void demon6SpockReachedWorkspace();
	void demon6SpockFinishedRepairingHand();
	void demon6UseAnythingOnWorkspace();
	void demon6UseCrewmanOnCase();
	void demon6UseKirkOnComputer();
	void demon6UseMccoyOnComputer();
	void demon6UseRedshirtOnComputer();
	void demon6UseSpockOnComputer();
	void demon6UseMineralOnStephen();
	void demon6UseShellsOnStephen();
	void demon6UseMeteorOnStephen();
	void demon6UseSkullOnStephen();
	void demon6UseMetalOnStephen();
	void demon6ReturnItemToStephen(int item);
	void demon6UseHandOnStephen();
	void demon6UseMTricoderOnStephen();
	void demon6UseSTricoderOnTable();
	void demon6UseSTricoderOnComputer();
	void demon6UseSTricoderOnMineral();
	void demon6UseSTricoderOnShells();
	void demon6UseSTricoderOnSkull();
	void demon6UseSTricoderOnMetal();
	void demon6UseSTricoderOnMeteor();
	void demon6UseSTricoderOnCase();
	void demon6UseSTricoderOnSynthesizer();
	void demon6GetCase();
	void demon6KirkReachedCase();
	int demon6ShowCase(int visible);

	// TUG0
	void tug0Tick1();
	void tug0LookAtEngineer();
	void tug0GetEngineer();
	void tug0LookAtControls();
	void tug0UseSpockOnControls();
	void tug0SpockReachedControlsToExamine();
	void tug0SpockExaminedControls();
	void tug0UseTransmogrifierWithoutBitOnControls();
	void tug0UseTransmogrifierWithBitOnControls();
	void tug0SpockReachedControlsWithTransmogrifier();
	void tug0SpockFinishedUsingTransmogrifier();
	void tug0TransporterScreenFullyLit();
	void tug0UseWireScrapsOnControls();
	void tug0UseWireOnControls();
	void tug0SpockReachedControlsWithWire();
	void tug0SpockFinishedUsingWire();
	void tug0UseMedkitOnEngineer();
	void tug0MccoyReachedEngineer();
	void tug0MccoyHealedEngineer();
	void tug0EngineerGotUp();
	void tug0GetTransmogrifier();
	void tug0KirkReachedToolbox();
	void tug0KirkGotTransmogrifier();
	void tug0LookAtToolbox();
	void tug0UsePhaserOnWelder();
	void tug0UseWelderOnWireScraps();
	void tug0UseWelderOnMetalScraps();
	void tug0UseCombBitOnTransmogrifier();
	void tug0UseTransporter();
	void tug0SpockReachedControlsToTransport();
	void tug0SpockPreparedTransporter();
	void tug0SpockReachedTransporter();
	void tug0FinishedTransporting();
	void tug0UseBombOnTransporter();
	void tug0KirkReachedTransporter();
	void tug0KirkPlacedBomb();
	void tug0SpockReachedControlsForBomb();
	void tug0SpockBeginsBeamingBomb();
	void tug0SpockFinishesBeamingBomb();
	void tug0BombExploded();
	void tug0UseMTricorderOnControls();
	void tug0UseSTricorderOnControls();
	void tug0UseMTricorderOnEngineer();
	void tug0MccoyReachedEngineerToScan();
	void tug0MccoyFinishedScanningEngineer();
	void tug0UseSTricorderOnEngineer();
	void tug0WalkToDoor();
	void tug0LookAtKirk();
	void tug0LookAtSpock();
	void tug0LookAtMccoy();
	void tug0LookAtRedshirt();
	void tug0LookAtTransporter();
	void tug0LookAtDoor();
	void tug0TalkToKirk();
	void tug0TalkToMccoy();
	void tug0TalkToSpock();
	void tug0TalkToRedshirt();
	void tug0TalkToEngineer();
	void tug0UseCommunicator();
	void tug0LookAnywhere();
	void tug0UseSTricorderAnywhere();
	void tug0UseMTricorderAnywhere();

	// TUG1
	void tug1Tick1();
	void tug1UseSTricorderOnAnything();
	void tug1LookAtBridgeDoor();
	void tug1UseSTricorderOnBridgeDoor();
	void tug1UsePhaserOnBridgeDoor();
	void tug1KirkReachedFiringPosition();
	void tug1KirkPulledOutPhaser();
	void tug1KirkFinishedFiringPhaser();
	void tug1TalkToSpock();
	void tug1UseSTricorderOnJunkPile();
	void tug1LookAtJunkPile();
	void tug1GetJunkPile();
	void tug1KirkReachedJunkPile();
	void tug1KirkFinishedTakingJunkPile();
	void tug1UsePhaserOnWelder();
	void tug1UseWelderOnWireScraps();
	void tug1UseWelderOnMetalScraps();
	void tug1UseCombBitOnTransmogrifier();
	void tug1UsePhaserWelderOnBridgeDoor();
	void tug1UsePhaserWelderOnBridgeDoorInLeftSpot();
	void tug1KirkReachedBridgeDoorWithWelder();
	void tug1KirkFinishedUsingWelder();
	void tug1KirkReachedBridgeDoorWithWelderInLeftSpot();
	void tug1KirkFinishedUsingWelderInLeftSpot();
	void tug1LookAnywhere();
	void tug1LookAtMccoy();
	void tug1LookAtSpock();
	void tug1LookAtRedshirt();
	void tug1TalkToMccoy();
	void tug1TalkToRedshirt();
	void tug1LookAtTerminal();
	void tug1LookAtDebris();
	void tug1LookAtBrigDoor();
	void tug1UseSTricorderOnBrigDoor();
	void tug1TalkToKirk();
	void tug1UseCommunicator();
	void tug1WalkToBridgeDoor();
	void tug1KirkReachedBridgeDoor();
	void tug1BridgeDoorOpened();
	void tug1WalkToBrigDoor();
	void tug1UseMTricorderAnywhere();
	void tug1UseMTricorderOnBridgeDoor();
	void tug1UseMTricorderOnBrigDoor();
	void tug1UseSpockOnBridgeDoor();
	void tug1UseRedshirtOnBridgeDoor();
	void tug1UseMedkitOnBridgeDoor();

private:
	// Room-specific variables. This is memset'ed to 0 when the room is initialized.
	union {
		struct {
			int numKlingonsKilled;
			byte attackIndex;
			bool kirkShooting;
			char d6[10];
		} demon1;

		struct {
			bool shootingBoulder; // 0xca
			bool boulder1Shot; // 0xcb
			byte boulderBeingShot; // 0xcc
			bool kirkInPosition; // 0xcd
			bool redshirtInPosition; // 0xce
			bool spockInPosition; // 0xcf
			bool mccoyInPosition; // 0xd0
			bool inFiringPosition; // 0xd1
			bool kirkPhaserOut; // 0xd3
			char boulderAnim[10]; // 0xd4
			int16 usedPhaserOnDoor; // 0xd6
		} demon3;

		struct {
			bool triedToShootNauian; // 0xca
			bool nauianEmerged; // 0xcb
			bool disabledSecurity; // 0xcc
			bool cd; // 0xcd
			byte crewReadyToBeamOut; // 0xce
			int16 crewmanUsingPanel; // 0xcf
		} demon4;

		struct {
			bool scannedRoberts; // 0xca
			bool scannedChub; // 0xcb
			bool scannedGrisnash; // 0xcc
			bool scannedStephen; // 0xcd
			byte numScanned; // 0xce
			byte numTalkedTo; // 0xcf
			bool talkedToRoberts; // 0xd0
			bool talkedToChub; // 0xd1
			bool talkedToGrisnash; // 0xd2
			bool talkedToStephen; // 0xd3
			byte doorCounter; // 0xd4
			bool movingToDoor; // 0xd5
		} demon5;

		struct {
			bool insultedStephenRecently; // 0xca
			bool stephenInRoom; // 0xcb
			bool caseOpened; // 0xcd
			byte doorCounter; // 0xce
			bool movingToDoor; // 0xcf
		} demon6;

	} _roomVar;
};

}

#endif
