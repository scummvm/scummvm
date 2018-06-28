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
	bool handleAction(byte type, byte b1, byte b2, byte b3) { return handleAction(Action(type, b1, b2, b3)); };

	// Same as above, but if any byte in the action is -1 (0xff), it matches any value.
	bool handleActionWithBitmask(const Action &action);
	bool handleActionWithBitmask(byte type, byte b1, byte b2, byte b3) { return handleActionWithBitmask(Action(type, b1, b2, b3)); };

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

	int _roomIndex; // ie. for DEMON2, this is 2


	int findFunctionPointer(int action, void (Room::*funcPtr)());

	// Interface for room-specific code
	void loadActorAnim(int actorIndex, Common::String anim, int16 x = -1, int16 y = -1, uint16 field66 = 0); // Cmd 0x00
	void loadActorAnimC(int actorIndex, Common::String anim, int16 x, int16 y, void (Room::*funcPtr)());// Cmd 0x00
	void loadActorStandAnim(int actorIndex);                                                   // Cmd 0x01
	void loadActorAnim2(int actorIndex, Common::String anim, int16 x = -1, int16 y = -1, uint16 field66 = 0);// Cmd 0x02
	int showRoomSpecificText(const char **textAddr); // (Deprecated, use function below)       // Cmd 0x03
	int showText(const TextRef *text);                                                         // Cmd 0x03
	int showText(TextRef speaker, TextRef text);                                               // Cmd 0x03
	int showText(TextRef text);                                                                // Cmd 0x03
	void giveItem(int item);                                                                   // Cmd 0x04
	// Command 0x05: "demon4ShowSunPuzzle"
	void loadRoomIndex(int roomIndex, int spawnIndex);                                         // Cmd 0x06
	void loseItem(int item);                                                                   // Cmd 0x07
	void walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam = 0);// Cmd 0x08
	void walkCrewmanC(int actorIndex, int16 destX, int16 destY, void (Room::*funcPtr)());      // Cmd 0x08
	void loadMapFile(const Common::String &name);                                              // Cmd 0x09
	void showBitmapFor5Ticks(const Common::String &bmpName, int priority);                     // Cmd 0x0a
	// Command 0x0c: "demon6ShowCase"
	bool haveItem(int item);                                                                   // Cmd 0x0b
	Common::Point getActorPos(int actorIndex);                                                 // Cmd 0x0d
	int16 getRandomWordInRange(int start, int end);                                            // Cmd 0x0e
	void playSoundEffectIndex(int soundEffect);                                                // Cmd 0x0f
	void playMidiMusicTracks(int startTrack, int loopTrack = -1);                              // Cmd 0x10
	void endMission(int16 score, int16 arg2, int16 arg3);                                      // Cmd 0x11
	void showGameOverMenu();                                                                   // Cmd 0x12
	void playVoc(Common::String filename);                                                     // Cmd 0x15

	// Helper functions for repetitive stuff.

	// If "changeDirection" is true, they remain facing that direction even after their
	// animation is finished. The game is inconsistent about doing this.
	void spockScan(int direction, TextRef text, bool changeDirection);
	void mccoyScan(int direction, TextRef text, bool changeDirection);

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

	// TUG2
	void tug2Tick1();
	void tug2Tick60();
	void tug2LookAtButton();
	void tug2LookAtMccoy();
	void tug2LookAtSpock();
	void tug2LookAtRedshirt();
	void tug2GetBomb();
	void tug2KirkReachedBomb();
	void tug2KirkGotBomb();
	void tug2LookAtBomb();
	void tug2LookAtGuard1();
	void tug2LookAtGuard2();
	void tug2LookAtWires();
	void tug2UseSTricorderOnButton();
	void tug2UseMccoyOnWires();
	void tug2UseMccoyOnBomb();
	void tug2UseRedshirtOnWires();
	void tug2RedshirtReachedWires();
	void tug2RedshirtDefusedBomb();
	void tug2RedshirtReturnedToPosition();
	void tug2UseKirkOnWires();
	void tug2KirkReachedWires();
	void tug2UseSpockOnWires();
	void tug2SpockReachedWires();
	void tug2SpockReturnedToPosition();
	void tug2GetWires();
	void tug2KirkReachedWiresToGet();
	void tug2KirkGotWires();
	void tug2UseKirkOnButton();
	void tug2KirkReachedButton();
	void tug2UseSpockOnButton();
	void tug2SpockReachedButton();
	void tug2UseMccoyOnButton();
	void tug2MccoyReachedButton();
	void tug2UseRedshirtOnButton();
	void tug2RedshirtReachedButton();
	void tug2TurnedOffForceField();
	void tug2PrisonersDead();
	void tug2PrisonersReleased();
	void tug2UsePhaserOnBrig();
	void tug2ElasiReadyPhaser();
	void tug2CheckEndFirefight();
	void tug2UseStunPhaserOnGuard1();
	void tug2KirkShootGuard1();
	void tug2UseStunPhaserOnGuard2();
	void tug2KirkShootGuard2();
	void tug2UseKillPhaserOnGuard1();
	void tug2KirkKillGuard1();
	void tug2UseKillPhaserOnGuard2();
	void tug2KirkKillGuard2();
	void tug2UsePhaserOnWelder();
	void tug2UseWelderOnWireScraps();
	void tug2UseWelderOnMetalScraps();
	void tug2UseCombBitOnTransmogrifier();
	void tug2ShotByElasi();
	void tug2WalkToDoor();
	void tug2LookAtDoor();
	void tug2LookAtKirk();
	void tug2TalkToKirk();
	void tug2TalkToMccoy();
	void tug2TalkToRedshirt();
	void tug2TalkToSpock();
	void tug2UseCommunicator();
	void tug2DetermineElasiShooter();
	void tug2Timer0Expired();
	void tug2GuardShootsCrewman();
	void tug2Timer2Expired();
	void tug2UseSTricorderOnBomb();
	void tug2UseMTricorderOnGuard1();
	void tug2UseMTricorderOnGuard2();
	void tug2TalkToGuard1();
	void tug2TalkToGuard2();
	void tug2UseMedkitOnBomb();
	void tug2UseMedkitOnGuard1();
	void tug2UseMedkitOnGuard2();
	void tug2LookAnywhere();
	void tug2TalkToBrig();
	void tug2UseMTricorderOnBrig();
	void tug2UseMTricorderOnOpenBrig();
	void tug2UsePhaserAnywhere();

	// TUG3
	void tug3Tick1();
	void tug3Tick40();
	void tug3LookAnywhere();
	void tug3ElasiSurrendered();
	void tug3UsePhaserAnywhere();
	void tug3ElasiDrawPhasers();
	void tug3UseStunPhaserOnElasi1();
	void tug3UseStunPhaserOnElasi2();
	void tug3UseStunPhaserOnElasi3();
	void tug3UseStunPhaserOnElasi4();
	void tug3UseKillPhaserOnElasi1();
	void tug3UseKillPhaserOnElasi2();
	void tug3UseKillPhaserOnElasi3();
	void tug3UseKillPhaserOnElasi4();
	void tug3ElasiStunnedOrKilled();
	void tug3TalkToElasi1();
	void tug3Elasi1DrewPhaser();
	void tug3Elasi1ShotConsole();
	void tug3Elasi1DrewPhaser2();
	void tug3Elasi1ShotConsoleAndSurrenders();
	void tug3LookAtMccoy();
	void tug3LookAtSpock();
	void tug3LookAtRedshirt();
	void tug3LookAtElasi1();
	void tug3LookAtElasi2();
	void tug3LookAtElasi3();
	void tug3LookAtElasi4();
	void tug3TalkToMccoy();
	void tug3TalkToSpock();
	void tug3TalkToRedshirt();
	void tug3UseCommunicator();
	void tug3Timer0Expired();
	void tug3AllCrewmenDead();
	void tug3Timer1Expired();
	void tug3EndMission();
	void tug3SecurityTeamBeamedIn();

	// LOVE0
	void love0Tick1();
	void love0Tick10();
	void love0WalkToDoor2();
	void love0OpenDoor2();
	void love0ReachedDoor2();
	void love0WalkToDoor1();
	void love0OpenDoor1();
	void love0ReachedDoor1();
	void love0LookAtConsole();
	void love0LookAtViewscreen();
	void love0LookAnywhere();
	void love0LookAtDoor1();
	void love0LookAtDoor2();
	void love0LookAtKirk();
	void love0LookAtMccoy();
	void love0LookAtSpock();
	void love0LookAtRedshirt();
	void love0TalkToKirk();
	void love0TalkToMccoy();
	void love0TalkToSpock();
	void love0TalkToRedshirt();
	void love0UseMTricorderAnywhere();
	void love0UseSTricorderOnConsole();
	void love0UseSTricorderAnywhere();
	void love0UseKirkOnConsole();
	void love0UseRedshirtOnConsole();
	void love0UseSpockOnConsole();
	void love0SpockReachedConsole();
	void love0SpockAccessedConsole();
	void love0UseMccoyOnConsole();
	void love0MccoyReachedConsole();
	void love0MccoyAccessedConsole();
	void love0InteractWithConsole();
	void love0GetDoorOrConsole();

	// LOVE1
	void love1Tick1();
	void love1WalkToDoor3();
	void love1OpenDoor3();
	void love1ReachedDoor3();
	void love1WalkToDoor1();
	void love1OpenDoor1();
	void love1ReachedDoor1();
	void love1WalkToDoor2();
	void love1OpenDoor2();
	void love1ReachedDoor2();
	void love1LookAtLaser();
	void love1LookAtKirk();
	void love1LookAtSpock();
	void love1LookAtMccoy();
	void love1LookAtRedshirt();
	void love1LookAnywhere();
	void love1LookAtNozzle();
	void love1LookAtLadder();
	void love1LookAtDoor1Or2();
	void love1LookAtDoor3();
	void love1LookAtDistillator();
	void love1LookAtChamber();
	void love1LookAtReplicator();
	void love1LookAtFreezer();
	void love1LookAtDishes();
	void love1TalkToKirk();
	void love1TalkToSpock();
	void love1TalkToMccoy();
	void love1TalkToRedshirt();
	void love1UseMTricorderOnDishes();
	void love1UseMTricorderAnywhere();
	void love1UseSTricorderOnReplicator();
	void love1UseSTricorderOnLaser();
	void love1UseSTricorderOnFreezer();
	void love1UseSTricorderAnywhere();
	void love1UseSTricorderOnDishes();
	void love1UseSTricorderOnDistillator();
	void love1GetFreezer();
	void love1KirkReachedFreezer();
	void love1KirkGotVirusCulture();
	void love1GetFromChamber();
	void love1KirkReachedChamber();
	void love1KirkGotCureSample();
	void love1GetFromNozzle();
	void love1KirkReachedNozzleToGet();
	void love1KirkGotBottleFromNozzle();
	void love1UseN2OOnNozzle();
	void love1UseH2OOnNozzle();
	void love1UseNH3OnNozzle();
	void love1UseRLGOnNozzle();
	void love1KirkReachedNozzleToPut();
	void love1KirkPutBottleInNozzle();
	void love1UseAnthingOnNozzle();
	void love1UseSpockOnReplicator();
	void love1UseRedshirtOnReplicator();
	void love1UseMccoyOnReplicator();
	void love1MccoyReachedReplicator();
	void love1MccoyUsedReplicator();
	void love1ChamberClosed();
	void love1ChamberOpened();
	void love1UseAnythingOnChamber();
	void love1UseDishesOnChamber();
	void love1KirkReachedChamberToPut();
	void love1ChamberOpenedForDish();
	void love1KirkPutDishInChamber();
	void love1UseInsulationOnDistillator();
	void love1KirkReachedDistillator();
	void love1KirkGotPolyberylcarbonate();
	void love1UseKirkOnFreezer();
	void love1UseRedshirtOnFreezer();
	void love1UseSpockOnFreezer();
	void love1UseMccoyOnFreezer();
	void love1CrewmanReachedFreezer();
	void love1CrewmanOpenedOrClosedFreezer();
	void love1UseAnythingOnFreezer();
	void love1ReachedFreezerWithArbitraryItem();
	void love1FinishedUsingArbitraryItemOnFreezer();
	void love1UseAnythingOnReplicator();
	void love1ReachedReplicatorWithArbitraryItem();
	void love1FinishedUsingArbitraryItemOnReplicator();
	void love1UseAnythingOnDistillator();
	void love1ReachedDistillatorWithArbitraryItem();
	void love1FinishedUsingArbitraryItemOnDistillator();
	void love1UseKirkOnLadder();
	void love1UseSpockOnLadder();
	void love1UseMccoyOnLadder();
	void love1UseRedshirtOnLadder();
	void love1CrewmanReachedLadder();
	void love1CrewmanDiedFromPhaser();
	void love1TouchedHotspot0();

	// LOVE2
	void love2Tick1();
	void love2TouchedWarp1();
	void love2Timer3Expired();
	void love2WalkToDoor();
	void love2TouchedHotspot0();
	void love2DoorReachedOrOpened();
	void love2LookAtCabinet();
	void love2LookAtDoor();
	void love2LookAtSynthesizer();
	void love2LookAtAnywhere();
	void love2LookAtAntigrav();
	void love2LookAtMccoy();
	void love2LookAtSpock();
	void love2LookAtRedshirt();
	void love2LookAtKirk();
	void love2LookAtArdak();
	void love2LookAtChamber();
	void love2LookAtCan1();
	void love2LookAtCan2();
	void love2TalkToKirk();
	void love2TalkToSpock();
	void love2TalkToMccoy();
	void love2TalkToRedshirt();
	void love2UseMTricorderAnywhere();
	void love2UseSTricorderAnywhere();
	void love2UseKirkOnCabinet();
	void love2UseSpockOnCabinet();
	void love2UseMccoyOnCabinet();
	void love2UseRedshirtOnCabinet();
	void love2CrewmanReachedCabinet();
	void love2CrewmanAccessedCabinet();
	void love2CrewmanOpenedOrClosedCabinet();
	void love2UseWrenchOnGasFeed();
	void love2ReachedGasFeed();
	void love2ChangedGasFeed();
	void love2UseO2GasOnCanisterSlot();
	void love2UseH2GasOnCanisterSlot();
	void love2UseN2GasOnCanisterSlot();
	void love2ReachedCanisterSlot();
	void love2PutCanisterInSlot1();
	void love2PutCanisterInSlot2();
	void love2UseAntigravOnCanister1();
	void love2ReachedCanisterSlot1ToGet();
	void love2TookCanister1();
	void love2UseAntigravOnCanister2();
	void love2ReachedCanisterSlot2ToGet();
	void love2TookCanister2();
	void love2UseKirkOnSynthesizer();
	void love2UseSpockOnSynthesizer();
	void love2UseMccoyOnSynthesizer();
	void love2UseRedshirtOnSynthesizer();
	void love2CrewmanReachedSynthesizer();
	void love2CrewmanUsedSynthesizer();
	void love2SpockReachedGasFeeds();
	void love2SpockEnabledGasFeeds();
	void love2UseSynthesizer();
	void love2SynthesizerDoorClosed();
	void love2SynthesizerFinished();
	void love2ClosedSynthesizerDoorMakingRLG();
	void love2ClosedSynthesizerDoorMakingCure();
	void love2CureStartedSynthesizing();
	void love2CureSynthesized();
	void love2UsePolyberylcarbonateOnSynthesizerDoor();
	void love2KirkReachedSynthesizerWithPolyberylcarbonate();
	void love2SynthesizerDoorOpenedWithPolyberylcarbonate();
	void love2PutPolyberylcarbonateInSynthesizer();
	void love2UseVirusSampleOnSynthesizerDoor();
	void love2KirkReachedSynthesizerWithVirusSample();
	void love2SynthesizerDoorOpenedWithVirusSample();
	void love2PutVirusSampleInSynthesizer();
	void love2UseCureSampleOnSynthesizerDoor();
	void love2KirkReachedSynthesizerWithCureSample();
	void love2SynthesizerDoorOpenedWithCureSample();
	void love2PutCureSampleInSynthesizer();
	void love2UseAnythingOnSynthesizerDoor();
	void love2UseAnythingOnSynthesizer();
	void love2GetCanister();
	void love2GetAntigrav();
	void love2ReachedAntigrav();
	void love2GotAntigrav();
	void love2GetPolyberylcarbonate();
	void love2ReachedPolyberylcarbonate();
	void love2GotPolyberylcarbonate();
	void love2GetDishes();
	void love2ReachedDishes();
	void love2GotDishes();
	void love2GetSample();
	void love2ReachedSample();
	void love2GotSample();
	void love2GetSynthesizerOutput();
	void love2ReachedSynthesizerOutput();
	void love2GotSynthesizerOutput();
	void love2GetCure();
	void love2ReachedCure();
	void love2GotCure();

	// LOVE3
	void love3Tick1();
	void love3Tick80();
	void love3TouchedWarp1();
	void love3Timer3Expired();
	void love3LookAtServicePanel();
	void love3LookAtMonitor();
	void love3LookAtWrench();
	void love3LookAtKirk();
	void love3LookAtMccoy();
	void love3LookAtSpock();
	void love3LookAtRedshirt();
	void love3LookAnywhere();
	void love3LookAtShaft();
	void love3LookAtGrate();
	void love3LookAtPanel();
	void love3LookAtGasTank();
	void love3LookAtTurbines();
	void love3TalkToKirk();
	void love3TalkToSpock();
	void love3TalkToMccoy();
	void love3TalkToRedshirt();
	void love3UseMTricorderAnywhere();
	void love3UseSTricorderOnTurbines();
	void love3UseSTricorderOnInsulation();
	void love3UseSTricorderAnywhere();
	void love3UseKirkOnServicePanel();
	void love3UseSpockOnServicePanel();
	void love3UseMccoyOnServicePanel();
	void love3UseRedshirtOnServicePanel();
	void love3CrewmanReachedServicePanel();
	void love3OpenedOrClosedServicePanel();
	void love3UseWrenchOnGasTank();
	void love3ReachedGasTankToUnscrew();
	void love3ScrewedOrUnscrewedGasTank();
	void love3UseWrenchOnO2OrH2Gas();
	void love3UseWrenchOnN2Gas();
	void love3ReachedGasTankToPutDown();
	void love3PutN2TankBack();
	void love3UseWrenchOnEngineeringPanel();
	void love3ReachedEngineeringPanel();
	void love3OpenedEngineeringPanel();
	void love3UseWrenchOnMonitor();
	void love3UseWaterOnMonitor();
	void love3ReachedMonitorToUseWater();
	void love3PouredWaterOnMonitor();
	void love3UseCrewmanOnEngineeringPanelOrGrate();
	void love3UseWrenchOnGrate();
	void love3ReachedGateWithWrench();
	void love3OpenedOrClosedGrate();
	void love3UseCrewmanOnShaft();
	void love3UseWaterOnShaft();
	void love3ReachedShaftUsingWater();
	void love3PouredWaterDownShaft();
	void love3UseNitrousOxideOnShaft();
	void love3ReachedShaftUsingNitrousOxide();
	void love3PouredNitrousOxideDownShaft();
	void love3UseAmmoniaOnShaft();
	void love3ReachedShaftUsingAmmonia();
	void love3PouredAmmoniaDownShaft();
	void love3UseRomulanLaughingGasOnShaft();
	void love3ReachedShaftUsingRomulanLaughingGas();
	void love3PouredRomulanLaughingGasDownShaft();
	void love3GetWrench();
	void love3ReachedWrenchToGet();
	void love3PickedUpWrench();
	void love3GetGasTank();
	void love3ReachedGasTankToGet();
	void love3UseAntigravOnGasTank();
	void love3ReachedGasTankUsingAntigrav();
	void love3PickedUpGasTank();
	void love3GetInsulation();
	void love3ReachedInsulationToGet();
	void love3PickedUpInsulation();

	// LOVE4
	void love4Tick1();
	void love4Tick10();
	void love4WalkToDoor();
	void love4TouchedHotspot0();
	void love4DoorOpenedOrReached();
	void love4UseKirkOnLadder();
	void love4ReachedLadder();
	void love4UseStunPhaserOnRomulan();
	void love4LookAnywhere();
	void love4LookAtLadder();
	void love4LookAtDoor();
	void love4LookAtKirk();
	void love4LookAtMccoy();
	void love4LookAtSpock();
	void love4LookAtRedshirt();
	void love4LookAtRomulan();
	void love4TalkToKirk();
	void love4TalkToMccoy();
	void love4TalkToRedshirt();
	void love4TalkToSpock();
	void love4TalkToRomulan();
	void love4UseMTricorderOnRomulan();
	void love4UseMTricorderAnywhere();
	void love4UseSTricorderAnywhere();
	void love4UseCureOnRomulan();
	void love4MccoyReachedRomulan4();
	void love4MccoyCuredRomulan4();
	void love4MccoyReachedRomulan3();
	void love4MccoyCuredRomulan3();
	void love4MccoyReachedRomulan2();
	void love4MccoyCuredRomulan2();
	void love4MccoyReachedRomulan1();
	void love4MccoyCuredRomulan1();
	void love4UseWaterOnRomulan();
	void love4UseMedkitOnRomulan();
	void love4UseCureSampleOnRomulan();
	void love4UseAnythingOnRomulan();

	// LOVE5
	void love5Tick1();
	void love5TouchedWarp1();
	void love5Timer4Expired();
	void love5UseStunPhaserOnPreax();
	void love5LookAtKirk();
	void love5LookAtSpock();
	void love5LookAtMccoy();
	void love5LookAtRedshirt();
	void love5LookAnywhere();
	void love5LookAtDevice();
	void love5LookAtConsole();
	void love5LookAtDrMarcus();
	void love5LookAtDrCheever();
	void love5LookAtPreax();
	void love5TalkToPreax();
	void love5MccoyReachedSpockToCure();
	void love5MccoyCuredSpock();
	void love5EndMission();
	void love5CrewmanReachedBeamoutPosition();
	void love5TalkToDrMarcus();
	void love5TalkToDrCheever();
	void love5TalkToKirk();
	void love5TalkToSpock();
	void love5TalkToMccoy();
	void love5TalkToRedshirt();
	void love5UseSTricorderAnywhere();
	void love5UseSTricorderOnDevice();
	void love5UseMTricorderOnMarcusOrCheever();
	void love5UseMTricorderOnPreax();
	void love5UseMedkitOnPreax();
	void love5UseWaterOnPreax();
	void love5UseCureSampleOnPreax();
	void love5UseCureOnPreax();
	void love5ReachedPreaxUsingCure();
	void love5CuredPreax();
	void love5UseAnythingOnPreax();
	void love5UseKirkOnMarcusOrCheever();
	void love5KirkReachedCheever();
	void love5KirkUntiedCheever();
	void love5KirkReachedMarcus();
	void love5KirkUntiedMarcus();
	void love5MarcusStoodUp();

	// LOVEA (common code)
	void loveaTimer0Expired();
	void loveaTimer1Expired();
	void loveaUseMedkitOnSpock();
	void loveaUseCureSampleOnSpock();
	void loveaUseCureOnSpock();
	void loveaSpockOrMccoyInPositionToUseCure();
	void loveaFinishedCuringSpock();
	void loveaTimer2Expired();
	void loveaUseMTricorderOnSpock();
	void loveaUseMTricorderOnHuman();
	void loveaUseRomulanLaughingGas();
	void loveaUseHumanLaughingGas();
	void loveaUseAmmonia();
	void loveaUseCommunicator();


	// MUDD0
	void mudd0Tick1();
	void mudd0Tick50();
	void mudd0Tick60();
	void mudd0UsePhaserOnMudd();
	void mudd0UseCommunicator();
	void mudd0LookAtFoodBox();
	void mudd0LookAtComponentBox();
	void mudd0LookAnywhere();
	void mudd0LookAtMemoryDiskBox();
	void mudd0LookAtDegrimerBox();
	void mudd0LookAtLense();
	void mudd0UseSTricorderAnywhere();
	void mudd0UseMTricorderAnywhere();
	void mudd0UseSTricorderOnMemoryDiskBox();
	void mudd0UseSTricorderOnDegrimerBox();
	void mudd0UseMTricorderOnLense();
	void mudd0UseLenseOnDegrimer();
	void mudd0UseAlienDevice();
	void mudd0FiredAlienDevice();
	void mudd0UseDegrimer();
	void mudd0GetLense();
	void mudd0GetMemoryDisk();
	void mudd0GetDegrimer();
	void mudd0PickedUpLense();
	void mudd0PickedUpItem();
	void mudd0WalkToSouthDoor();
	void mudd0TouchedHotspot1();
	void mudd0WalkToNorthDoor();
	void mudd0TouchedHotspot0();
	void mudd0UseMedkit();
	void mudd0LookAtKirk();
	void mudd0LookAtSpock();
	void mudd0LookAtMccoy();
	void mudd0LookAtRedshirt();
	void mudd0LookAtMudd();
	void mudd0TalkToKirk();
	void mudd0TalkToSpock();
	void mudd0TalkToMccoy();
	void mudd0TalkToRedshirt();
	void mudd0TalkToMudd();

	// MUDD1
	void mudd1Tick1();
	void mudd1Timer1Expired();
	void mudd1UseCommunicator();
	void mudd1UseSpockOnBlueButton();
	void mudd1SpockReachedBlueButton();
	void mudd1SpockPressedBlueButton();
	void mudd1CraneFinishedMoving();
	void mudd1UseSpockOnYellowButton();
	void mudd1SpockReachedYellowButton();
	void mudd1SpockPressedYellowButton();
	void mudd1UseSpockOnRedButton();
	void mudd1SpockReachedRedButton();
	void mudd1SpockPressedRedButton();
	void mudd1GetTorpedo();
	void mudd1UseSTricorderOnTorpedo();
	void mudd1UseSTricorderOnTorpedoLauncher();
	void mudd1UseSTricorderOnButton();
	void mudd1UseSTricorderOnCrane();
	void mudd1UseMedkitAnywhere();
	void mudd1LookAnywhere();
	void mudd1LookAtTorpedo();
	void mudd1LookAtFallenTorpedo();
	void mudd1LookAtTorpedoLauncher();
	void mudd1LookAtKirk();
	void mudd1LookAtSpock();
	void mudd1LookAtMccoy();
	void mudd1LookAtRedshirt();
	void mudd1LookAtCrane();
	void mudd1LookAtRedButton();
	void mudd1LookAtBlueButton();
	void mudd1LookAtYellowButton();
	void mudd1TalkToKirk();
	void mudd1TalkToSpock();
	void mudd1TalkToMccoy();
	void mudd1TalkToRedshirt();
	void mudd1WalkToSouthDoor();
	void mudd1TouchedHotspot2();
	void mudd1WalkToNorthDoor();
	void mudd1TouchedHotspot1();
	void mudd1WalkToWestDoor();
	void mudd1TouchedHotspot0();

	// MUDD2
	void mudd2WalkToNorthDoor();
	void mudd2TouchedHotspot0();
	void mudd2WalkToSouthDoor();
	void mudd2TouchedHotspot1();
	void mudd2Tick1();
	void mudd2Timer1Expired();
	void mudd2UseSTricorderAnywhere();
	void mudd2UseSpockOnCapsules();
	void mudd2GetCapsules();
	void mudd2MccoyReachedCapsules();
	void mudd2MccoyPickedUpCapsules();
	void mudd2UseCommunicator();
	void mudd2LookAtCapsules();
	void mudd2UseMTricorderOnCapsules();
	void mudd2UseCapsuleOnControlPanel();
	void mudd2MccoyReachedControlPanel();
	void mudd2MccoyPutCapsuleInControlPanel();
	void mudd2UseKirkOnBed();
	void mudd2KirkReachedBed();
	void mudd2MuddNoticedKirk();
	void mudd2MuddDroppedCapsule();
	void mudd2UsePhaserOnMudd();
	void mudd2UseSpockOnMudd();
	void mudd2SpockReachedMudd();
	void mudd2SpockPinchedMudd();
	void mudd2UseKirkOnMudd();
	void mudd2UseRedshirtOnMudd();
	void mudd2RedshirtReachedMudd();
	void mudd2Timer2Expired();
	void mudd2MuddFinishedPushingRedshirt();
	void mudd2RedshirtPushedAway();
	void mudd2UseMTricorderOnMudd();
	void mudd2UseMedkitOnMudd();
	void mudd2MccoyReachedMudd();
	void mudd2MccoyCuredMudd();
	void mudd2LookAtKirk();
	void mudd2LookAtSpock();
	void mudd2LookAtMccoy();
	void mudd2LookAtRedshirt();
	void mudd2LookAtMudd();
	void mudd2LookAtControlPanel();
	void mudd2LookAtBed();
	void mudd2TalkToKirk();
	void mudd2TalkToSpock();
	void mudd2TalkToMccoy();
	void mudd2TalkToRedshirt();
	void mudd2TalkToMudd();

	// MUDD3
	void mudd3Tick1();
	void mudd3UseCommunicator();
	void mudd3LookAtScreen();
	void mudd3UseSTricorderOnScreen();
	void mudd3UseKirkOnScreen();
	void mudd3UseSpockOnSphere();
	void mudd3Timer3Expired();
	void mudd3UseMccoyOnSphere();
	void mudd3UseRedshirtOnSphere();
	void mudd3UseMTricorderOnSphere();
	void mudd3UseSTricorderOnSphere();
	void mudd3ReadyToHookUpTricorders();
	void mudd3Timer1Expired();
	void mudd3Timer2Expired();
	void mudd3Timer4Expired();
	void mudd3UseMemoryDiskOnSphere();
	void mudd3GetRepairTool();
	void mudd3ReachedRepairTool();
	void mudd3PickedUpRepairTool();
	void mudd3LookAtSphere();
	void mudd3WalkToNorthDoor();
	void mudd3TouchedHotspot0();
	void mudd3WalkToEastDoor();
	void mudd3TouchedHotspot1();
	void mudd3LookAtKirk();
	void mudd3LookAtSpock();
	void mudd3LookAtMccoy();
	void mudd3LookAtRedshirt();
	void mudd3LookAtMudd();
	void mudd3TalkToKirk();
	void mudd3TalkToSpock();
	void mudd3TalkToMccoy();
	void mudd3TalkToRedshirt();
	void mudd3TalkToMudd();
	void mudd3UseMedkit();

	// MUDD4
	void mudd4Tick1();

	// MUDD5
	void mudd5Tick1();

	// MUDDA

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

		struct {
			byte shootingObject; // 0x1ec1
			byte shootingTarget;
			bool elasiPhaserOnKill;
			byte shootKirkOverride; // 0x1ec4
		} tug2;


		struct {
			// love0
			bool heardSummaryOfVirus; // 0xda
			int16 consoleCrewman; // 0xe3
			char consoleAnimation[10]; // 0xe5
			TextRef consoleSpeaker; // 0xe7
			TextRef consoleText; // 0xe9

			// love1
			TextRef dyingSpeaker; // 0xcf
			int16 crewmanUsingFreezerRetX; // 0xd1
			int16 crewmanUsingFreezerRetY; // 0xd3
			int16 crewmanUsingDevice; // 0xd9
			int16 itemInNozzle; // 0xdd
			char bottleAnimation[10]; // 0xdf

			// love2
			byte canisterType; // 0xca
			byte cb; // 0xcb
			int16 canisterItem; // 0xce
			char canisterAnim[10]; // 0xd0
			int16 chamberObject; // 0xd2
			char chamberInputAnim[10]; // 0xd6
			char chamberOutputAnim[10]; // 0xd8

			// love3
			byte activeCrewman;

			// love4
			bool gaveWaterToRomulans; // 0xca

			// love5
			byte numCrewmenReadyToBeamOut; // 0xcb

			// common
			byte walkingToDoor;
			byte doorOpenCounter;
			byte spockAndMccoyReadyToUseCure;
			byte cmnXPosToCureSpock;
			byte cmnYPosToCureSpock;
		} love;

		struct {
			// mudd3
			bool suggestedUsingTricorders; // 0xca
			bool tricordersUnavailable; // 0xcb

			// common
			byte walkingToDoor;
		} mudd;


	} _roomVar;
};

}

#endif
