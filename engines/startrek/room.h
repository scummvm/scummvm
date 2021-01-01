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
#include "common/hashmap.h"

#include "startrek/action.h"
#include "startrek/awaymission.h"
#include "startrek/fixedint.h"
#include "startrek/object.h"
#include "startrek/soundeffects.h"
#include "startrek/text.h"

using Common::SharedPtr;

namespace StarTrek {

class StarTrekEngine;
class Room;

#include "common/pack-start.h"	// START STRUCT PACKING

// Per-room action functions
struct RoomAction {
	const Action action;
	void (Room::*funcPtr)();
};

#include "common/pack-end.h"	// END STRUCT PACKING

// Offsets of data in RDF files

const int RDF_BAN_DATA_START = 0x1e;
const int RDF_BAN_DATA_END   = 0x20;
const int RDF_WARP_ROOM_INDICES = 0x22;
const int RDF_ROOM_ENTRY_POSITIONS = 0x2a;
const int RDF_BEAM_IN_POSITIONS = 0xaa;
const int RDF_SPAWN_POSITIONS = 0xba;

#define COMMON_MESSAGE_OFFSET 1000
#define FOLLOWUP_MESSAGE_OFFSET 600
#define SCOTTY_MESSAGE_OFFSET 500

class Room {
public:
	Room(StarTrekEngine *vm, const Common::String &name);
	~Room();

	friend class Console;

	uint16 readRdfWord(int offset);

	/**
	 * Scale-related stuff; at the "min Y" position or below, the crewmembers have
	 * "minimum" scale; that value rises to the "max scale" value by the time they reach
	 * the "max Y" value.
	 */
	uint16 getMaxY() {
		return readRdfWord(0x06);
	}
	uint16 getMinY() {
		return readRdfWord(0x08);
	}
	Fixed8 getMinScale() {
		return Fixed8::fromRaw(readRdfWord(0x0a));
	}
	Fixed8 getMaxScale() {
		return Fixed8::fromRaw(readRdfWord(0x0c));
	}
	int16 getBanDataStart() {
		return readRdfWord(RDF_BAN_DATA_START);
	}
	int16 getBanDataEnd() {
		return readRdfWord(RDF_BAN_DATA_END);
	}

	/**
	 * Check if a particular action is defined for this room.
	 */
	bool actionHasCode(const Action &action);
	bool actionHasCode(int8 type, byte b1, byte b2, byte b3);

	/**
	 * Execute a particular action for this room, if defined.
	 */
	bool handleAction(const Action &action);
	bool handleAction(int8 type, byte b1, byte b2, byte b3);

	/**
	 * Same as above, but if any byte in the action is -1 (0xff), it matches any value.
	 */
	bool handleActionWithBitmask(const Action &action);
	bool handleActionWithBitmask(int8 type, byte b1, byte b2, byte b3);

	uint16 getFirstHotspot() {
		return readRdfWord(0x12);
	}
	uint16 getHotspotEnd()   {
		return readRdfWord(0x14);
	}

	// Warp-related stuff
	uint16 getFirstWarpPolygonOffset() {
		return readRdfWord(0x16);
	}
	uint16 getWarpPolygonEndOffset()   {
		return readRdfWord(0x18);
	}
	uint16 getFirstDoorPolygonOffset() {
		return readRdfWord(0x1a);
	}
	uint16 getDoorPolygonEndOffset()   {
		return readRdfWord(0x1c);
	}

	/**
	 * Get the point at which a crewman beams in to this room (not properly defined for
	 * all rooms).
	 */
	Common::Point getBeamInPosition(int crewmanIndex);

	/**
	 * This is analagous to above, but instead of beaming in, they just appear in a spot.
	 * Used sparingly, ie. in feather's serpent when appearing in cave after Quetzecoatl
	 * warps the crew.
	 */
	Common::Point getSpawnPosition(int crewmanIndex);

	/**
	 * Returns true if the given position is contained in a polygon.
	 *
	 * The data passed contains the following words in this order:
	 *   * Index of polygon (unused here)
	 *   * Number of vertices in polygon
	 *   * For each vertex: x and y coordinates.
	 */
	bool isPointInPolygon(int offset, int16 x, int16 y);

	byte *_rdfData;

private:
	uint16 _rdfSize;

	StarTrekEngine *_vm;
	AwayMission *_awayMission;

	const RoomAction *_roomActionList;

	Common::HashMap<int, Common::String> _lookMessages;
	Common::HashMap<int, Common::String> _lookWithTalkerMessages;
	Common::HashMap<int, Common::String> _talkMessages;

	void loadRoomMessages();
	void loadOtherRoomMessages();
	void loadRoomMessage(const char *text);
	Common::String patchRoomMessage(const char *text);

	Common::MemoryReadStreamEndian *loadBitmapFile(Common::String baseName);
	Common::MemoryReadStreamEndian *loadFileWithParams(Common::String filename, bool unk1, bool unk2, bool unk3);

	int findFunctionPointer(int action, void (Room::*funcPtr)());


	// Interface for room-specific code
	/**
	 * Cmd 0x00
	 */
	void loadActorAnim(int actorIndex, Common::String anim, int16 x = -1, int16 y = -1, uint16 field66 = 0);
	/**
	 * Cmd 0x00
	 */
	void loadActorAnimC(int actorIndex, Common::String anim, int16 x, int16 y, void (Room::*funcPtr)());
	/**
	 * Cmd 0x01
	 */
	void loadActorStandAnim(int actorIndex);
	/**
	 * Cmd 0x02
	 * This is exactly the same as "loadActorAnim", but the game calls it at different times?
	 */
	void loadActorAnim2(int actorIndex, Common::String anim, int16 x = -1, int16 y = -1, uint16 field66 = 0);
	/**
	 * Cmd 0x03
	 */
	int showRoomSpecificText(const char **textAddr);
	int showMultipleTexts(const TextRef *text, bool fromRDF = false, bool lookWithTalker = false);
	int showDescription(TextRef text, bool fromRDF = false, bool lookWithTalker = false);
	int showText(TextRef speaker, TextRef text, bool fromRDF = false, bool lookWithTalker = false);

	/**
	 * Cmd 0x04
	 */
	void giveItem(int item);

	// Command 0x05: "demon4ShowSunPuzzle"

	/**
	 * Cmd 0x06
	 */
	void loadRoomIndex(int roomIndex, int spawnIndex);
	/**
	 * Cmd 0x07
	 */
	void loseItem(int item);
	/**
	 * Cmd 0x08
	 */
	void walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam = 0);
	void walkCrewmanC(int actorIndex, int16 destX, int16 destY, void (Room::*funcPtr)());      // Cmd 0x08

public:
	/**
	 * Cmd 0x09: Loads a pair of .map and .iw files to change the room's collisions and pathfinding.
	 */
	void loadMapFile(const Common::String &name);

private:
	/**
	 * Cmd 0x0a
	 */
	void showBitmapFor5Ticks(const Common::String &bmpName, int priority);
	/**
	 * Cmd 0x0b
	 */
	bool haveItem(int item);

	// Command 0x0c: "demon6ShowCase"

	/**
	 * Cmd 0x0d
	 */
	Common::Point getActorPos(int actorIndex);
	/**
	 * Cmd 0x0e: Returns a word in range [start, end] (that's inclusive).
	 */
	int16 getRandomWordInRange(int start, int end);
	/**
	 * Cmd 0x0f
	 */
	void playSoundEffectIndex(int soundEffect);
	/**
	 * Cmd 0x10
	 */
	void playMidiMusicTracks(int startTrack, int loopTrack = -1);
	/**
	 * Cmd 0x11
	 */
	void endMission(int16 score, int16 arg2, int16 arg3);
	/**
	 * Cmd 0x12
	 */
	void showGameOverMenu();
	/**
	 * Cmd 0x13: Text input used in "Than Old Devil Moon" (SINS mission)
	 * Takes a list of codes (ending with nullptr) and returns the index of the matched
	 * code (plus one), or 0 if no code was matched.
	 */
	int showCodeInputBox(const char * const *codes);
	/**
	 * Cmd 0x14: Show a map of the republic for mission 7
	 */
	void showRepublicMap(int16 arg0, int16 arg2);
	/**
	 * Cmd 0x15
	 */
	void playVoc(Common::String filename);
	/**
	 * Cmd 0x17
	 */
	void stopAllVocSounds();

	// Helper functions for repetitive stuff.

	Common::String getCrewmanAnimFilename(int object, const Common::String &str);

	/**
	 * If "changeDirection" is true, they remain facing that direction even after their
	 * animation is finished. The game is inconsistent about doing this.
	 */
	void spockScan(int direction, TextRef text, bool changeDirection = false, bool fromRDF = false);
	void mccoyScan(int direction, TextRef text, bool changeDirection = false, bool fromRDF = false);

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
	void demon6StephenIsInsulted();
	void demon6StephenDescribesItemsInCase();
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
	int demon6ShowCaseProcessInput(Sprite *sprites, Sprite *buttonSprite, int visible);
	int demon6ShowCaseProcessSelection(Sprite *sprites, Sprite *clickedSprite, int visible);

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
	void mudd4UseCommunicator();
	void mudd4Timer2Expired();
	void mudd4Timer3Expired();
	void mudd4UseSpockOnLeftConsole();
	void mudd4UseSpockOnRightConsole();
	void mudd4UseSpockOnConsole();
	void mudd4SpockReachedChair();
	void mudd4SpockSatInChair();
	void mudd4ShowLeftConsoleMenu();
	void mudd4SpockUsedSensors();
	void mudd4SpockUsedEngineering();
	void mudd4SpockUsedNavigation();
	void mudd4FinishedWalking3();
	void mudd4UseKirkOnRightConsole();
	void mudd4UseKirkOnLeftConsole();
	void mudd4UseKirkOnConsole();
	void mudd4KirkReachedRightConsole();
	void mudd4KirkSatInChair();
	void mudd4ShowRightConsoleMenu();
	void mudd4KirkUsedViewScreen();
	void mudd4Timer1Expired();
	void mudd4KirkUsedCommunications();
	void mudd4TalkWithMuddAtMissionEnd();
	void mudd4KirkReachedPositionToTalkToMudd();
	void mudd4UseMccoyOnConsole();
	void mudd4UseRedshirtOnConsole();
	void mudd4UseSTricorderOnRepairTool();
	void mudd4UseSTricorderOnConsole();
	void mudd4UseSTricorderOnViewscreen();
	void mudd4UseMedkit();
	void mudd4GetRepairTool();
	void mudd4ReachedRepairTool();
	void mudd4PickedUpRepairTool();
	void mudd4LookAtConsole();
	void mudd4LookAtViewscreen();
	void mudd4LookAtKirk();
	void mudd4LookAtSpock();
	void mudd4LookAtMccoy();
	void mudd4LookAtRedshirt();
	void mudd4LookAtRepairTool();
	void mudd4TalkToKirk();
	void mudd4TalkToSpock();
	void mudd4TalkToMccoy();
	void mudd4TalkToRedshirt();
	void mudd4WalkToEastDoor();
	void mudd4WalkToWestDoor();
	void mudd4TouchedHotspot0();

	// MUDD5
	void mudd5Tick1();
	void mudd5Timer1Expired();
	void mudd5Timer2Expired();
	void mudd5UseCommunicator();
	void mudd5UseSTricorderAnywhere();
	void mudd5UseSTricorderOnEngine();
	void mudd5UseSTricorderOnCrane();
	void mudd5UseSTricorderOnHatch();
	void mudd5UseSTricorderOnLifeSupportGenerator();
	void mudd5UseStunPhaserOnHatch();
	void mudd5UseKillPhaserOnHatch();
	void mudd5UseAnythingOnLifeSupportGenerator();
	void mudd5UseDooverOnLifeSupportGenerator();
	void mudd5KirkReachedLifeSupportGenerator();
	void mudd5KirkTimer4Expired();
	void mudd5KirkRepairedLifeSupportGenerator();
	void mudd5KirkTimer3Expired();
	void mudd5LookAtHatch();
	void mudd5LookAtLifeSupportGenerator();
	void mudd5TouchedHotspot0();
	void mudd5WalkToDoor();
	void mudd5LookAtKirk();
	void mudd5LookAtSpock();
	void mudd5LookAtMccoy();
	void mudd5LookAtRedshirt();
	void mudd5LookAtDoor();
	void mudd5LookAtCrane();
	void mudd5LookAtEngine();
	void mudd5TalkToKirk();
	void mudd5TalkToSpock();
	void mudd5TalkToMccoy();
	void mudd5TalkToRedshirt();
	void mudd5UseMedkit();

	// MUDDA
	void muddaUseLenseOnDegrimer();
	void muddaUseAlienDevice();
	void muddaFiredAlienDevice();
	void muddaUseDegrimer();
	void muddaTick();

	// FEATHER0
	void feather0Tick1();
	void feather0Tick60();
	void feather0TalkToQuetzecoatl();
	void feather0Timer0Expired();
	void feather0Timer1Expired();
	void feather0UsePhaserOnQuetzecoatl();
	void feather0UsePhaserOnMccoy();
	void feather0UseMedkit();
	void feather0LookAtQuetzecoatl();
	void feather0LookAtMoon();
	void feather0LookAtLog();
	void feather0LookAtHut();
	void feather0LookAnywhere();
	void feather0LookAtEyes();
	void feather0LookAtTree();
	void feather0LookAtMccoy();
	void feather0LookAtSpock();
	void feather0LookAtRedshirt();
	void feather0TalkToMccoy();
	void feather0TalkToSpock();
	void feather0TalkToRedshirt();
	void feather0UseSTricorderOnQuetzecoatl();
	void feather0UseSTricorderAnywhere();
	void feather0UseMTricorderOnQuetzecoatl();

	// FEATHER1
	void feather1Tick1();
	void feather1Tick45();
	void feather1Tick85();
	void feather1Tick95();
	void feather1QuetzecoatlDisappeared();
	void feather1GetRightVine();
	void feather1GetLeftVine();
	void feather1GetRocks();
	void feather1ReachedRocks();
	void feather1PickedUpRocks();
	void feather1GetSnake();
	void feather1ReachedSnake();
	void feather1Timer1Expired();
	void feather1Timer2Expired();
	void feather1PickedUpSnake();
	void feather1Timer0Expired();
	void feather1UseCommunicator();
	void feather1UseRockOnHole();
	void feather1ReachedHole();
	void feather1Timer3Expired();
	void feather1PutRockInHole();
	void feather1UseSnakeOnLeftVine();
	void feather1UseSnakeOnSpock();
	void feather1UseSnakeOnKirk();
	void feather1UseSnakeOnMccoy();
	void feather1UseSnakeOnRedshirt();
	void feather1UseRockOnSnake();
	void feather1UseSpockOnSnake();
	void feather1UseMccoyOnSnake();
	void feather1UseRedshirtOnSnake();
	void feather1UseSpockOnHole();
	void feather1UseMccoyOnHole();
	void feather1UseRedshirtOnHole();
	void feather1UseRockOnMoss();
	void feather1UseRockOnSpock();
	void feather1UseRockOnMccoy();
	void feather1UseRockOnRedshirt();
	void feather1UseSpockOnMoss();
	void feather1UseMccoyOnMoss();
	void feather1UseRedshirtOnMoss();
	void feather1UseRockOnLeftVine();
	void feather1ReadyToThrowRock1();
	void feather1ThrewRock1();
	void feather1ReadyToThrowRock2();
	void feather1ThrewRock2();
	void feather1UseSpockOnVine();
	void feather1UseMccoyOnVine();
	void feather1UseRedshirtOnVine();
	void feather1UseKirkOnVine();
	void feather1CrewmanClimbVine();
	void feather1ReachedVineToClimbUp();
	void feather1ClimbedUpVine();
	void feather1ReachedVineToClimbDown();
	void feather1ClimbedDownVine();
	void feather1UsePhaser();
	void feather1UseSTricorderOnRightVine();
	void feather1UseSTricorderOnSnake();
	void feather1UseSTricorderOnMoss();
	void feather1UseSTricorderOnHole();
	void feather1UseSTricorderAnywhere();
	void feather1UseSTricorderOnRocks();
	void feather1UseMTricorderOnVine();
	void feather1UseMTricorderOnMoss();
	void feather1UseMTricorderOnHole();
	void feather1UseMTricorderOnSnake();
	void feather1UseMedkit();
	void feather1TalkToMccoy();
	void feather1TalkToSpock();
	void feather1TalkToRedshirt();
	void feather1WalkToExit();
	void feather1LookAnywhere();
	void feather1LookAtSnake();
	void feather1LookAtRightVine();
	void feather1LookAtHole();
	void feather1LookAtMoss();
	void feather1LookAtRocks();
	void feather1LookAtLight();
	void feather1LookAtEyes();
	void feather1LookAtKirk();
	void feather1LookAtSpock();
	void feather1LookAtMccoy();
	void feather1LookAtRedshirt();
	void feather1LookAtLeftVine();

	// FEATHER2
	void feather2Tick1();
	void feather2UseCommunicator();
	void feather2UsePhaser();
	void feather2UseSTricorderAnywhere();
	void feather2UseMTricorderAnywhere();
	void feather2TalkToMccoy();
	void feather2TalkToSpock();
	void feather2TalkToRedshirt();
	void feather2LookAtVines();
	void feather2UseMedkit();
	void feather2WalkToLeftExit();
	void feather2LookAtEyes();
	void feather2LookAtBigTree();
	void feather2LookAtTrees();
	void feather2LookAnywhere();
	void feather2LookAtKirk();
	void feather2LookAtSpock();
	void feather2LookAtMccoy();
	void feather2LookAtRedshirt();

	// FEATHER3
	void feather3Tick1();
	void feather3Tick40();
	void feather3TouchedHotspot0();
	void feather3UseSpockOnTlaoxac();
	void feather3UseMccoyOnTlaoxac();
	void feather3UseRedshirtOnTlaoxac();
	void feather3UseCrewmanOnLeftExit();
	void feather3UseCommunicator();
	void feather3UseSnakeOnKirk();
	void feather3UseSnakeOnSpock();
	void feather3UseSnakeOnRedshirt();
	void feather3KirkReachedRedshirtWithSnake();
	void feather3UseSnakeOnMccoy();
	void feather3UseSnakeOnTlaoxac();
	void feather3UseKnifeOnSpock();
	void feather3UseKnifeOnMccoy();
	void feather3UseKnifeOnRedshirt();
	void feather3UseRockOnTlaoxac();
	void feather3KirkReachedPositionToThrowRock();
	void feather3Timer4Expired();
	void feather3Timer2Expired();
	void feather3Timer3Expired();
	void feather3TlaoxacKnockedOutFromRockThrow();
	void feather3UsePhaser();
	void feather3TalkToMccoy();
	void feather3TalkToSpock();
	void feather3TalkToRedshirt();
	void feather3TalkToTlaoxac();
	void feather3TlaoxacKilledRedshirt();
	void feather3UseMTricorderOnTlaoxac();
	void feather3UseMedkitOnTlaoxac();
	void feather3UseMedkitOnRedshirt();
	void feather3UseMedkitAnywhere();
	void feather3UseMTricorderOnLight();
	void feather3UseSTricorderOnLight();
	void feather3UseSTricorderAnywhere();
	void feather3GetKnife();
	void feather3ReachedKnife();
	void feather3Timer1Expired();
	void feather3PickedUpKnife();
	void feather3LookAtSpock();
	void feather3LookAtRedshirt();
	void feather3LookAtTlaoxac();
	void feather3LookAtLight();
	void feather3LookAtMccoy();
	void feather3LookAtKnife();

	// FEATHER4
	void feather4Tick1();
	void feather4UseCommunicator();
	void feather4UsePhaser();
	void feather4UseSTricorderOnLight();
	void feather4UseSTricorderOnMushroom();
	void feather4UseSTricorderOnFern();
	void feather4UseSTricorderAnywhere();
	void feather4UseMTricorderOnLight();
	void feather4UseMTricorderOnMushroom();
	void feather4UseMTricorderAnywhere();
	void feather4UseMedkitAnywhere();
	void feather4TalkToMccoy();
	void feather4TalkToSpock();
	void feather4TalkToRedshirt();
	void feather4LookAtEyes();
	void feather4LookAnywhere();
	void feather4LookAtMushroom();
	void feather4LookAtFern();
	void feather4LookAtLight();
	void feather4LookAtKirk();
	void feather4LookAtSpock();
	void feather4LookAtMccoy();
	void feather4LookAtRedshirt();

	// FEATHER5
	void feather5Tick1();
	void feather5TouchedHotspot0();
	void feather5UseSpockOnFern();
	void feather5UseMccoyOnFern();
	void feather5UseRedshirtOnFern();
	void feather5UseRedshirtOnLeftExit();
	void feather5RedshirtReachedLog();
	void feather5RedshirtDeathFinished();
	void feather5UseSpockOnLeftExit();
	void feather5UseMccoyOnLeftExit();
	void feather5UseSnakeOnKirk();
	void feather5UseSnakeOnSpock();
	void feather5UseSnakeOnMccoy();
	void feather5UseSnakeOnRedshirt();
	void feather5UseSnakeOnWater();
	void feather5UseKnifeOnKirk();
	void feather5UseKnifeOnSpock();
	void feather5UseKnifeOnMccoy();
	void feather5UseKnifeOnRedshirt();
	void feather5UseKnifeOnWater();
	void feather5UseKnifeOnMonster();
	void feather5GetFern();
	void feather5UseKnifeOnFern();
	void feather5ReachedFern();
	void feather5PickedUpFern();
	void feather5UseFernOnMonster();
	void feather5ReachedPositionToUseFern();
	void feather5DoneThrowingFern();
	void feather5FernFellIntoWater();
	void feather5Timer1Expired();
	void feather5UseRockOnSpock();
	void feather5UseRockOnMccoy();
	void feather5UseRockOnRedshirt();
	void feather5UseCrewmanOnThrownFern();
	void feather5UseSpockOnMonster();
	void feather5UseMccoyOnMonster();
	void feather5UseRedshirtOnMonster();
	void feather5UsePhaser();
	void feather5LookAnywhere();
	void feather5LookAtLog();
	void feather5LookAtWater();
	void feather5LookAtFern();
	void feather5LookAtInsects();
	void feather5LookAtCave();
	void feather5LookAtMonster();
	void feather5LookAtSpock();
	void feather5LookAtKirk();
	void feather5LookAtMccoy();
	void feather5LookAtRedshirt();
	void feather5TalkToMccoy();
	void feather5TalkToSpock();
	void feather5TalkToRedshirt();
	void feather5UseMTricorderOnWater();
	void feather5UseMTricorderOnMonster();
	void feather5UseSTricorderOnMonster();
	void feather5UseSTricorderAnywhere();
	void feather5UseSTricorderOnWater();
	void feather5UseSTricorderOnFern();
	void feather5UseMedkitAnywhere();

	// FEATHER6
	void feather6Tick1();
	void feather6TouchedHotspot0();
	void feather6UseSpockOnCrystals();
	void feather6UseMccoyOnCrystals();
	void feather6UseRedshirtOnCrystals();
	void feather6UseRockOnStalactites();
	void feather6ReachedPositionToThrowRock();
	void feather6DoneThrowingRock();
	void feather6KirkDiedFromStalactites();
	void feather6UseRockOnCrystals();
	void feather6ReachedCrystalsWithRock();
	void feather6Tick();
	void feather6HitCrystalsWithRockFirstTime();
	void feather6HitCrystalsWithRockSecondTime();
	void feather6UseRockOnStalagmites();
	void feather6UseRockAnywhere();
	void feather6UseKnifeOnStalagmites();
	void feather6UseKnifeAnywhere();
	void feather6UseKnifeOnMccoy();
	void feather6UseKnifeOnSpock();
	void feather6UseKnifeOnRedshirt();
	void feather6UseCrystalAnywhere();
	void feather6UseSnakeAnywhere();
	void feather6UseMedkitAnywhere();
	void feather6UsePhaser();
	void feather6LookAtEastExit();
	void feather6LookAnywhere();
	void feather6LookAtRocks();
	void feather6LookAtStalagmites();
	void feather6LookAtCrystals();
	void feather6LookAtKirk();
	void feather6LookAtSpock();
	void feather6LookAtMccoy();
	void feather6LookAtRedshirt();
	void feather6LookAtStalactites();
	void feather6TalkToMccoy();
	void feather6TalkToRedshirt();
	void feather6TalkToSpock();
	void feather6UseMTricorderOnEastExit();
	void feather6UseMTricorderOnRocks();
	void feather6UseSTricorderOnEastExit();
	void feather6UseSTricorderAnywhere();
	void feather6UseSTricorderOnRocks();
	void feather6UseSTricorderOnCrystals();
	void feather6UseSTricorderOnStalactites();
	void feather6UseSTricorderOnStalagmites();
	void feather6GetCrystals();
	void feather6UseKnifeOnCrystals();
	void feather6ReachedCrystalsWithKnife();
	void feather6DoneCuttingCrystals();
	void feather6Timer4Expired();

	// FEATHER7
	void feather7Tick1();
	void feather7Tick40();
	void feather7KirkReachedSeat();
	void feather7SpockReachedSeat();
	void feather7MccoyReachedSeat();
	void feather7QuetzecoatlReachedSeat();
	void feather7KirkSatDown();
	void feather7ReadyToBeamOut();


	// TRIAL0
	void trial0Tick1();
	void trial0Tick40();
	void trial0Timer0Expired();
	void trial0WalkToRoomCenter();
	void trial0ReachedRoomCenter();
	void trial0LookAtWindow();
	void trial0LookAtVlict();
	void trial0LookAtGuard();
	void trial0LookAtBench();
	void trial0LookAtSeal();
	void trial0LookAtKirk();
	void trial0LookAtSpock();
	void trial0LookAtMccoy();
	void trial0LookAtRedshirt();
	void trial0LookAtQuetzecoatl();
	void trial0TalkToVlict();
	void trial0TalkToGuard();
	void trial0TalkToQuetzecoatl();
	void trial0TalkToMccoy();
	void trial0TalkToSpock();
	void trial0TalkToRedshirt();
	void trial0UsePhaserOrRock();
	void trial0UseSnakeOnVlict();
	void trial0UseSnakeOnGuard();
	void trial0UseMTricorderOnQuetzecoatl();
	void trial0UseMTricorderOnVlict();
	void trial0UseMTricorderAnywhere();
	void trial0UseSTricorderOnWindow();
	void trial0UseSTricorderAnywhere();
	void trial0UseCommunicator();
	void trial0UseCrewmanOnKlingon();
	void trial0UseMccoyOnVlict();
	void trial0UseMccoyOnGuard();
	void trial0UseMedkitAnywhere();

	// TRIAL1
	void trial1Tick1();
	void trial1Tick50();
	void trial1DoorOpened();
	void trial1TouchedHotspot0();
	void trial1KirkDied();
	void trial1TouchedHotspot1();
	void trial1GetRod();
	void trial1ReachedRod();
	void trial1PickedUpRod();
	void trial1LookAtKirk();
	void trial1LookAtSpock();
	void trial1LookAtMccoy();
	void trial1LookAtRedshirt();
	void trial1LookAtWall();
	void trial1LookAtFloor();
	void trial1LookAtRods();
	void trial1LookAtDoor();
	void trial1LookAtLock();
	void trial1LookAtEntity();
	void trial1LookAtMoltenRock();
	void trial1TalkToKirk();
	void trial1TalkToSpock();
	void trial1TalkToMccoy();
	void trial1TalkToRedshirt();
	void trial1TalkToEntity();
	void trial1UsePhaserOnCrewman();
	void trial1UsePhaserOnEntity();
	void trial1UsePhaserOnWall();
	void trial1UseStunPhaserOnFloor();
	void trial1ReachedFloorToUseStunPhaser();
	void trial1DoneShootingFloorWithStunPhaser();
	void trial1UseKillPhaserOnFloor();
	void trial1ReachedFloorToUseKillPhaser();
	void trial1DoneShootingFloorWithKillPhaser();
	void trial1UsePhaserOnRod();
	void trial1UsePhaserOnDoor();
	void trial1UsePhaserOnLock();
	void trial1UseMTricorderOnKirk();
	void trial1UseMTricorderOnSpock();
	void trial1UseMTricorderOnMccoy();
	void trial1UseMTricorderOnRedshirt();
	void trial1UseMTricorderOnEntity();
	void trial1UseSTricorderOnWall();
	void trial1UseSTricorderOnEntity();
	void trial1UseSTricorderOnRods();
	void trial1UseSTricorderOnFloor();
	void trial1UseSTricorderOnDoor();
	void trial1UseSTricorderOnLock();
	void trial1UseCommunicator();
	void trial1UseMccoyOnEntity();
	void trial1UseMccoyOnLock();
	void trial1UseMccoyOnFloor();
	void trial1UseSpockOnEntity();
	void trial1UseSpockOnLock();
	void trial1SpockReachedKeypad();
	void trial1SpockUsedKeypad();
	void trial1SpockReachedKeypadWithExtraProgram();
	void trial1SpockUsedKeypadWithExtraProgram();
	void trial1UseSpockOnFloor();
	void trial1UseRedshirtOnEntity();
	void trial1UseRedshirtOnLock();
	void trial1RedshirtReachedKeypad();
	void trial1RedshirtUsedKeypad();
	void trial1RedshirtReachedKeypadWithExtraProgram();
	void trial1RedshirtUsedKeypadWithExtraProgram();
	void trial1UseRedshirtOnFloor();
	void trial1UseRodOnMccoy();
	void trial1UseRodOnSpock();
	void trial1UseRodOnRedshirt();
	void trial1UseRodOnFloorOrWall();
	void trial1UseWoodRodOnEntity();
	void trial1ReachedPositionToThrowWoodRod();
	void trial1DoneThrowingWoodRod();
	void trial1UseWoodRodOnLock();
	void trial1UseWoodRodOnMoltenRock();
	void trial1ReachedMoltenRock();
	void trial1DoneCoatingWoodRod();
	void trial1UseIronRodOnEntity();
	void trial1ReachedPositionToThrowIronRod();
	void trial1DoneThrowingIronRod();
	void trial1UseIronRodOnLock();
	void trial1GetThrownIronRod();
	void trial1WalkToDoor();
	void trial1UseMedkit();

	// TRIAL2
	void trial2Tick1();
	void trial2Tick60();
	void trial2TouchedHotspot0();
	void trial2LookAtKirk();
	void trial2LookAtSpock();
	void trial2LookAtMccoy();
	void trial2LookAtRedshirt();
	void trial2LookAtInsignia();
	void trial2LookAtGlob();
	void trial2LookAtWall();
	void trial2LookAtDoor();
	void trial2TalkToKirk();
	void trial2TalkToSpock();
	void trial2TalkToMccoy();
	void trial2TalkToRedshirt();
	void trial2TalkToGlob();
	void trial2UsePhaserOnWall();

	void trial2UsePhaserOnGlob(int object, bool phaserOnKill);
	void trial2ReachedPositionToShootGlob();
	void trial2DrewPhaserToShootGlob();
	void trial2GlobDoneExploding();
	void trial2GlobDoneSplitting();
	void trial2KirkDied();

	void trial2UseStunPhaserOnGlob();
	void trial2UseKillPhaserOnGlob();
	void trial2UseStunPhaserOnSplitGlob1();
	void trial2UseKillPhaserOnSplitGlob1();
	void trial2UseStunPhaserOnSplitGlob2();
	void trial2UseKillPhaserOnSplitGlob2();

	void trial2UseMTricorderOnKirk();
	void trial2UseMTricorderOnSpock();
	void trial2UseMTricorderOnMccoy();
	void trial2UseMTricorderOnRedshirt();
	void trial2UseMTricorderOnGlob();
	void trial2UseSTricorderOnWall();
	void trial2UseSTricorderAnywhere();
	void trial2UseSTricorderOnGlob();
	void trial2UseCommunicator();
	void trial2UseMccoyOnGlob();
	void trial2UseMccoyOnWall();
	void trial2UseSpockOnGlob();
	void trial2UseSpockOnWall();
	void trial2UseRedshirtOnGlob();
	void trial2UseRedshirtOnWall();
	void trial2WalkToDoor();
	void trial2UseMedkitAnywhere();

	// TRIAL3
	void trial3Tick1();
	void trial3Tick30();
	void trial3Klingon1BeamedIn();
	void trial3Klingon2BeamedIn();
	void trial3Klingon3BeamedIn();
	void trial3KlingonShootsSomeone1();
	void trial3Klingon1DoneShooting();
	void trial3Klingon2DoneShooting();
	void trial3Klingon3DoneShooting();
	void trial3KlingonShootsSomeone2();
	void trial3RedshirtDoneDying();
	void trial3KirkDoneDying();
	void trial3Klingon1Shot();
	void trial3Klingon2Shot();
	void trial3Klingon3Shot();
	void trial3CheckShowUhuraText();
	void trial3CrewmanBeamedOut();
	void trial3Tick90();
	void trial3TouchedHotspot3();
	void trial3KirkExploded();
	void trial3LookAtKirk();
	void trial3LookAtSpock();
	void trial3LookAtMccoy();
	void trial3LookAtRedshirt();
	void trial3LookAtExit();
	void trial3LookAtWall();
	void trial3TalkToKirk();
	void trial3TalkToSpock();
	void trial3TalkToMccoy();
	void trial3TalkToRedshirt();
	void trial3UsePhaserOnWall();
	void trial3UseStunPhaserOnKlingon1();
	void trial3ReadyToShootKlingon1OnStun();
	void trial3UseKillPhaserOnKlingon1();
	void trial3ReadyToShootKlingon1OnKill();
	void trial3UsePhaserAnywhere();
	void trial3UseMTricorderOnKirk();
	void trial3UseMTricorderOnSpock();
	void trial3UseMTricorderOnMccoy();
	void trial3UseMTricorderOnRedshirt();
	void trial3UseMTricorderOnExit();
	void trial3UseSTricorderOnWall();
	void trial3UseSTricorderOnExit();
	void trial3UseMTricorderOnKlingon();
	void trial3UseCommunicator();
	void trial3BeamToVlict();
	void trial3UseMccoyOnWall();
	void trial3UseMccoyOnExit();
	void trial3UseSpockOnWall();
	void trial3UseSpockOnExit();
	void trial3UseRedshirtOnExit();
	void trial3UseRedshirtOnWall();
	void trial3WalkToExit();
	void trial3UseMedkitAnywhere();

	// TRIAL4
	void trial4Tick1();
	void trial4Tick60();

	// TRIAL5
	void trial5Tick1();
	void trial5Tick60();
	void trial5VlictAppeared();
	void trial5WalkToInterface();
	void trial5ReachedInterface();
	void trial5GetGem(int16 item, int16 object);
	void trial5ReachedGem();
	void trial5PickedUpGem();
	void trial5GetRedGem1();
	void trial5GetRedGem2();
	void trial5GetRedGem3();
	void trial5GetGreenGem1();
	void trial5GetGreenGem2();
	void trial5GetGreenGem3();
	void trial5GetBlueGem1();
	void trial5GetBlueGem2();
	void trial5GetBlueGem3();
	void trial5LookAtKirk();
	void trial5LookAtSpock();
	void trial5LookAtMccoy();
	void trial5LookAtRedshirt();
	void trial5LookAtBeam();
	void trial5LookAtInterface();
	void trial5UseSTricorderOnInterface();
	void trial5UseMTricorderOnInterface();
	void trial5LookAtRedGem1();
	void trial5LookAtRedGem2();
	void trial5LookAtRedGem3();
	void trial5LookAtGreenGem1();
	void trial5LookAtGreenGem2();
	void trial5LookAtGreenGem3();
	void trial5LookAtBlueGem1();
	void trial5LookAtBlueGem2();
	void trial5LookAtBlueGem3();
	void trial5LookAtHole();
	void trial5TalkToKirk();
	void trial5TalkToSpock();
	void trial5TalkToMccoy();
	void trial5TalkToRedshirt();
	void trial5UsePhaser();
	void trial5UseMTricorderAnywhere();
	void trial5UseSTricorderAnywhere();
	void trial5UseCommunicator();
	void trial5UseGemOnHole(int16 item, int16 object, int16 hole);
	void trial5ReachedHoleToPutGem();
	void trial5ActivateLightOfWar();
	void trial5ActivateLightOfKnowledge();
	void trial5ActivateLightOfTravel();
	void trial5RestrictedCombination1();
	void trial5RestrictedCombination2();
	void trial5DestinationNoLongerExists();
	void trial5CrewmanBeamedOut();
	void trial5UseRedGem1OnHole1();
	void trial5UseRedGem2OnHole1();
	void trial5UseRedGem3OnHole1();
	void trial5UseGreenGem1OnHole1();
	void trial5UseGreenGem2OnHole1();
	void trial5UseGreenGem3OnHole1();
	void trial5UseBlueGem1OnHole1();
	void trial5UseBlueGem2OnHole1();
	void trial5UseBlueGem3OnHole1();
	void trial5UseRedGem1OnHole2();
	void trial5UseRedGem2OnHole2();
	void trial5UseRedGem3OnHole2();
	void trial5UseGreenGem1OnHole2();
	void trial5UseGreenGem2OnHole2();
	void trial5UseGreenGem3OnHole2();
	void trial5UseBlueGem1OnHole2();
	void trial5UseBlueGem2OnHole2();
	void trial5UseBlueGem3OnHole2();
	void trial5UseRedGem1OnHole3();
	void trial5UseRedGem2OnHole3();
	void trial5UseRedGem3OnHole3();
	void trial5UseGreenGem1OnHole3();
	void trial5UseGreenGem2OnHole3();
	void trial5UseGreenGem3OnHole3();
	void trial5UseBlueGem1OnHole3();
	void trial5UseBlueGem2OnHole3();
	void trial5UseBlueGem3OnHole3();
	void trial5UseMedkitAnywhere();


	// SINS0
	void sins0Tick1();
	void sins0LookAnywhere();
	void sins0LookAtOpenDoor();
	void sins0LookAtClosedDoor();
	void sins0TalkToSpock();
	void sins0TalkToMccoy();
	void sins0TalkToRedshirt();
	void sins0LookAtGround();
	void sins0GetRock();
	void sins0PickedUpRock();
	void sins0UseSTricorderAnywhere();
	void sins0UseSTricorderOnGround();
	void sins0UseSTricorderOnPlanet();
	void sins0Tick40();
	void sins0LookAtStatue();
	void sins0LookAtPlanet();
	void sins0LookAtSky();
	void sins0LookAtKirk();
	void sins0LookAtSpock();
	void sins0LookAtMccoy();
	void sins0LookAtRedshirt();
	void sins0UseSTricorderOnClosedDoor();
	void sins0UseSTricorderOnStatue();
	void sins0UseMedkitOnCrewman();
	void sins0UseMTricorderOnCrewman();
	void sins0UseCommunicator();
	void sins0WalkToDoor();

	// SINS1
	void sins1Tick1();
	void sins1LookAnywhere();
	void sins1LookAtLock();
	void sins1LookAtDoor();
	void sins1LookAtPanel();
	void sins1LookAtKeypad();
	void sins1UseSTricorderOnPanel();
	void sins1UseSTricorderOnKeypad();
	void sins1UseSTricorderOnLock();
	void sins1UseSTricorderOnDoor();
	void sins1UseSTricorderAnywhere();
	void sins1UseSTricorderOnLens();
	void sins1UseRockOnDoor();
	void sins1UseRedshirtOnKeypad();
	void sins1UseMccoyOnKeypad();
	void sins1UseKirkOnKeypad();
	void sins1UseSpockOnKeypad();
	void sins1SpockReachedKeypad();
	void sins1EnteredCorrectCode();
	void sins1DoorDoneOpening();
	void sins1EnteredIncorrectCode();
	void sins1EnteredSacredSofNumber();
	void sins1UsePhaserOnDoor();
	void sins1DoorUsedLaser();
	void sins1Tick40();
	void sins1TalkToKirk();
	void sins1TalkToSpock();
	void sins1TalkToMccoy();
	void sins1TalkToRedshirt();
	void sins1LookAtKirk();
	void sins1LookAtSpock();
	void sins1LookAtMccoy();
	void sins1LookAtRedshirt();
	void sins1LookAtLens();
	void sins1UseCommunicator();
	void sins1UseMedkitOnCrewman();
	void sins1UseMTricorderOnCrewman();
	void sins1WalkToDoor();

	// SINS2
	void sins2Tick1();
	void sins2UsePhaserOnDoor();
	void sins2UseSTricorderAnywhere();
	void sins2UseKirkOnTerminal();
	void sins2UseSpockOnTerminal();
	void sins2SpockReachedTerminal();
	void sins2SpockUsedTerminal();
	void sins2UseMccoyOnTerminal();
	void sins2UseRedshirtOnTerminal();
	void sins2UseSTricorderOnTerminal();
	void sins2UseSTricorderOnKeypad();
	void sins2UseSTricorderOnDoor();
	void sins2UseKirkOnKeypad();
	void sins2UseSpockOnKeypad();
	void sins2SpockReachedKeypad();
	void sins2EnteredCorrectCode();
	void sins2EnteredIncorrectCode();
	void sins2DoorFinishedOpening();
	void sins2Tick40();
	void sins2LookAnywhere();
	void sins2LookAtOpenDoor();
	void sins2LookAtTerminal();
	void sins2LookAtKeypad();
	void sins2LookAtKirk();
	void sins2LookAtSpock();
	void sins2LookAtMccoy();
	void sins2LookAtRedshirt();
	void sins2TalkToKirk();
	void sins2TalkToSpock();
	void sins2TalkToMccoy();
	void sins2TalkToRedshirt();
	void sins2LookAtDoor();
	void sins2UseSpockOnDoor();
	void sins2UseCommunicator();
	void sins2UseMedkitOnCrewman();
	void sins2UseMTricorderOnCrewman();
	void sins2UseMccoyOnKeypad();
	void sins2UseRedshirtOnKeypad();
	void sins2WalkToDoor();

	// SINS3
	void sins3Tick1();
	void sins3UseSTricorderOnBox();
	void sins3UseSTricorderOnMonitor();
	void sins3UseSTricorderAnywhere();
	void sins3UseSTricorderOnDrill();
	void sins3UseSTricorderOnPanel();
	void sins3LookAtDrill();
	void sins3LookAtPanel();
	void sins3LookAtMonitor();
	void sins3LookAnywhere();
	void sins3UseSTricorderOnWall();
	void sins3LookAtBox();
	void sins3UseKirkOnBox();
	void sins3KirkReachedBox();
	void sins3Timer0Expired();
	void sins3UseSpockOnBox();
	void sins3SpockReachedBox();
	void sins3UseMccoyOnBox();
	void sins3MccoyReachedBox();
	void sins3UseRedshirtOnBox();
	void sins3RedshirtReachedBox();
	void sins3BoxOpened();
	void sins3GetBox();
	void sins3KirkReachedBoxToGet();
	void sins3KirkGotBoxContents();
	void sins3Tick30();
	void sins3Tick60();
	void sins3UseKirkOnPanel();
	void sins3UseSpockOnPanel();
	void sins3SpockReachedPanel();
	void sins3UseRockOnWall();
	void sins3KirkReachedWall();
	void sins3KirkPutRockOnWall();
	void sins3KirkBackedAwayFromWall();
	void sins3UsedLowPowerLaserOnRock();
	void sins3MadeHoleInRock();
	void sins3CreatedTemplateInRock();
	void sins3RockTurnedIntoIDCard();
	void sins3RockVaporized();
	void sins3UseDrillAnywhere();
	void sins3GetIDCard();
	void sins3ReachedIDCard();
	void sins3PickedUpIDCard();
	void sins3LookAtItemBeingDrilled();
	void sins3LookAtKirk();
	void sins3LookAtSpock();
	void sins3LookAtMccoy();
	void sins3LookAtRedshirt();
	void sins3TalkToKirk();
	void sins3TalkToSpock();
	void sins3TalkToMccoy();
	void sins3TalkToRedshirt();
	void sins3UseCommunicator();
	void sins3UseSpockOnDrill();
	void sins3UseMccoyOnDrill();
	void sins3UseRedshirtOnDrill();
	void sins3WalkToDoor();
	void sins3UseMedkitOnCrewman();
	void sins3UseMTricorderOnCrewman();

	// SINS4
	void sins4Tick1();
	void sins4UseSTricorderOnPanel();
	void sins4UseSpockOnPanel();
	void sins4UsePhaserOnNorthDoor();
	void sins4UsePhaserOnPanel();
	void sins4UseIDCardOnPanel();
	void sins4KirkReachedPanel();
	void sins4KirkPutCardInPanel();
	void sins4UseRockOnPanel();
	void sins4KirkReachedPanelWithRock();
	void sins4KirkUsedRockOnPanel();
	void sins4LookAtWestDoor();
	void sins4LookAtEastDoor();
	void sins4LookAnywhere();
	void sins4LookAtPanel();
	void sins4Tick40();
	void sins4LookAtKirk();
	void sins4LookAtSpock();
	void sins4LookAtMccoy();
	void sins4LookAtRedshirt();
	void sins4LookAtLight();
	void sins4LookAtBeam();
	void sins4LookAtOpenNorthDoor();
	void sins4LookAtClosedNorthDoor();
	void sins4TalkToKirk();
	void sins4TalkToSpock();
	void sins4TalkToMccoy();
	void sins4TalkToRedshirt();
	void sins4UseSTricorderOnNorthDoor();
	void sins4UseSTricorderAnywhere();
	void sins4UseCommunicator();
	void sins4UseMccoyOnNorthDoor();
	void sins4UseRedshirtOnNorthDoor();
	void sins4UseSpockOnNorthDoor();
	void sins4UseMccoyOnPanel();
	void sins4UseRedshirtOnPanel();
	void sins4WalkToNorthDoor();
	void sins4WalkToWestDoor();
	void sins4WalkToEastDoor();
	void sins4UseMedkitOnCrewman();
	void sins4UseMTricorderOnCrewman();

	// SINS5
	void sins5Tick1();
	void sins5UseSTricorderAnywhere();
	void sins5UseSTricorderOnRightComputer();
	void sins5UseSTricorderOnLeftComputer();
	void sins5UseSTricorderOnMiddleComputer();
	void sins5UseKirkOnRightComputer();
	void sins5UseKirkOnLeftComputer();
	void sins5UseKirkOnMiddleComputer();
	void sins5UseMccoyOnComputer();
	void sins5UseRedshirtOnComputer();
	void sins5UseSpockOnMiddleComputer();
	void sins5UseSpockOnRightComputer();
	void sins5SpockReachedRightComputer();
	void sins5SpockUsedRightComputer();
	void sins5UseSpockOnLeftComputer();
	void sins5SpockReachedLeftComputer();
	void sins5SpockUsedLeftComputer();
	void sins5CrewmanReadyToBeamOut();
	void sins5Tick20();
	void sins5CheckGatheredAllClues();
	void sins5UseWireOnComputer();
	void sins5KirkOrSpockInPositionToUseWire();
	void sins5WireConnected();
	void sins5UseStunPhaserOnComputer();
	void sins5UseKillPhaserOnLeftComputer();
	void sins5ReachedPositionToShootLeftComputer();
	void sins5DrewPhaserToShootLeftComputer();
	void sins5Timer0Expired();
	void sins5UseKillPhaserOnMiddleComputer();
	void sins5ReachedPositionToShootMiddleComputer();
	void sins5DrewPhaserToShootMiddleComputer();
	void sins5Timer1Expired();
	void sins5UseKillPhaserOnRightComputer();
	void sins5ReachedPositionToShootRightComputer();
	void sins5DrewPhaserToShootRightComputer();
	void sins5Timer2Expired();
	void sins5ComputerLaunchesMissiles();
	void sins5Timer3Expired();
	void sins5LookAnywhere();
	void sins5LookAtKirk();
	void sins5LookAtSpock();
	void sins5LookAtMccoy();
	void sins5LookAtRedshirt();
	void sins5LookAtLight();
	void sins5LookAtLeftComputer();
	void sins5LookAtMiddleComputer();
	void sins5LookAtRightComputer();
	void sins5LookAtMissile();
	void sins5LookAtNorthDoor();
	void sins5UseMedkitOnCrewman();
	void sins5UseMTricorderAnywhere();
	void sins5TalkToKirk();
	void sins5TalkToSpock();
	void sins5TalkToMccoy();
	void sins5TalkToRedshirt();
	void sins5UseSTricorderOnMissile();
	void sins5UseSTricorderOnNorthDoor();
	void sins5UseMTricorderOnCrewman();

	// VENG0
	void veng0Tick1();
	void veng0Tick10();
	void veng0Tick50();
	void veng0TouchedDoor();
	void veng0UseSTricorderOnComputerBank();
	void veng0UseSTricorderOnChair();
	void veng0UseSTricorderAnywhere();
	void veng0UseMccoyOnBeam();
	void veng0UseSTricorderOnBeam();
	void veng0UseRedshirtOnBeam();
	void veng0UseSpockOnConsole();
	void veng0UseRedshirtOnConsole();
	void veng0UsePhaserOnBeam();
	void veng0UseMccoyOnDeadCrewman();
	void veng0UseSawOnBeam();
	void veng0KirkReachedSaw();
	void veng0Timer0Expired();
	void veng0DoneCuttingBeam();
	void veng0GetBeam();
	void veng0TalkToKirk();
	void veng0TalkToMccoy();
	void veng0TalkToSpock();
	void veng0TalkToRedshirt();
	void veng0LookAtPatterson();
	void veng0LookAtDeadCrewman2();
	void veng0LookAtComputerBank();
	void veng0LookAtChair();
	void veng0LookAtKirk();
	void veng0LookAtSpock();
	void veng0LookAtMccoy();
	void veng0LookAtRedshirt();
	void veng0LookAtViewscreen();
	void veng0LookAtHelmConsole();
	void veng0LookAtNavConsole();
	void veng0LookAtBeam();
	void veng0LookAnywhere();
	void veng0LookAtDeadCrewman1();
	void veng0UseMccoyOnLivingCrewman();
	void veng0MccoyReachedCrewman();
	void veng0MccoyScannedCrewman();

	// VENG1
	void veng1Tick1();
	void veng1WalkToDoor();
	void veng1TouchedHotspot0();
	void veng1ReachedDoor();
	void veng1TouchedTurbolift();
	void veng1LookAtDeadGuy();
	void veng1UseSTricorderOnVent();
	void veng1LookAtPanel();
	void veng1LookAtDoor();
	void veng1LookAtSaw();
	void veng1LookAtDebris();
	void veng1LookAtKirk();
	void veng1LookAtMccoy();
	void veng1LookAtSpock();
	void veng1LookAtRedshirt();
	void veng1LookAnywhere();
	void veng1TalkToKirk();
	void veng1TalkToSpock();
	void veng1TalkToMccoy();
	void veng1TalkToRedshirt();
	void veng1UsePhaserOnDoor();
	void veng1UsePhaserOnPanel();
	void veng1UseSTricorderOnSaw();
	void veng1UseSTricorderOnDoor();
	void veng1UseSTricorderOnPanel();
	void veng1GetPanel();
	void veng1KirkReachedPanel();
	void veng1KirkAccessedPanel();
	void veng1UseSawOnPanel();
	void veng1UseSawOnDoor();
	void veng1UseDrillOnDebris();
	void veng1UseSawOnDebris();
	void veng1UseDrillOnPanel();
	void veng1KirkReachedPanelToDrill();
	void veng1KirkDoneDrillingPanel();
	void veng1UseHypoOnDebris();
	void veng1KirkReachedDebris();
	void veng1KirkUsedHypoOnDebris();
	void veng1GetSaw();
	void veng1ReachedSaw();
	void veng1PickedUpSaw();
	void veng1GetDebris();
	void veng1ReachedDebrisToGet();
	void veng1TriedToGetDebris();

	// VENG2
	void veng2Tick1();
	void veng2Timer0Expired();
	void veng2ElasiShipDecloaked();
	void veng2Timer2Expired();
	void veng2TouchedDoor();
	void veng2LookAtViewscreen();
	void veng2LookAtMiddleConsole();
	void veng2LookAtDamageDisplay();
	void veng2LookAtSTricorder();
	void veng2LookAtMTricorder();
	void veng2LookAtTorpedoButton();
	void veng2LookAtTorpedoControl();
	void veng2LookAtImpulseConsole();
	void veng2LookAtMainComputer();
	void veng2LookAtRecordDeck();
	void veng2LookAtDeckIOConsole();
	void veng2LookAtKirk();
	void veng2LookAtSpock();
	void veng2LookAtMccoy();
	void veng2LookAtRedshirt();
	void veng2LookAnywhere();
	void veng2TalkToKirk();
	void veng2TalkToSpock();
	void veng2TalkToMccoy();
	void veng2TalkToRedshirt();
	void veng2UseTechJournalOnDeckIOConsole();
	void veng2ReachedDeckIOWithTechJournal();
	void veng2DoneReadingTechJournal();
	void veng2UseLogOnDeckIOConsole();
	void veng2ReachedDeckIOWithLog();
	void veng2LoadedLog();
	void veng2UseCommunicator();
	void veng2UseKirkOnTorpedoButton();
	void veng2UseKirkReachedTorpedoButton();
	void veng2KirkFiredTorpedo();
	void veng2TorpedoStruckElasiShip();
	void veng2ElasiRetreated();
	void veng2CrewmanInPositionToBeamOut();
	void veng2UseSTricorderOnTorpedoButton();
	void veng2UseSTricorderOnMiddleConsole();
	void veng2UseSTricorderOnTorpedoControl();
	void veng2UseImpulseConsole();
	void veng2SpockReachedImpulseConsole();
	void veng2SpockUsedImpulseConsole();
	void veng2PowerWeapons();
	void veng2UseMainComputer();
	void veng2UseSTricorderOnMainComputer();
	void veng2SpockReachedMainComputerToPutTricorder();
	void veng2AttachedSTricorderToComputer();
	void veng2UseMTricorderOnMainComputer();
	void veng2MccoyReachedMainComputerToPutTricorder();
	void veng2AttachedMTricorderToComputer();
	void veng2BothTricordersPluggedIn();
	void veng2UseSTricorderOnDeckIOConsole();
	void veng2UseSTricorderOnImpulseConsole();
	void veng2UseFusionPackOnMainComputer();
	void veng2GetLogDeck();
	void veng2ReachedLogDeck();
	void veng2PickedUpLogDeck();
	void veng2GetSTricorder();
	void veng2ReachedSTricorderToGet();
	void veng2PickedUpSTricorder();
	void veng2GetMTricorder();
	void veng2ReachedMTricorderToGet();
	void veng2PickedUpMTricorder();

	// VENG3
	void veng3Tick1();
	void veng3Tick15();
	void veng3TouchedTurbolift2();
	void veng3TouchedTurbolift1();
	void veng3WalkToDoor();
	void veng3ReachedTurbolift2();
	void veng3Turbolift2Opened();
	void veng3WalkToSickbayDoor();
	void veng3ReachedSickbayDoor();
	void veng3SickbayDoorOpened();
	void veng3LookAtSupportBeam();
	void veng3LookAtDebris();
	void veng3LookAtTurbolift2Door();
	void veng3LookAtSickbayDoor();
	void veng3LookAtCable();
	void veng3LookAtDeadGuy();
	void veng3LookAtKirk();
	void veng3LookAtMccoy();
	void veng3LookAtSpock();
	void veng3LookAtRedshirt();
	void veng3LookAnywhere();
	void veng3UseStunPhaserOnDebris();
	void veng3UseKillPhaserOnDebris();
	void veng3ReachedPositionToShootDebris();
	void veng3DrewPhaserToShootDebris();
	void veng3DebrisVaporized();
	void veng3DebrisFellAgain();
	void veng3UseSpockOnDebris();
	void veng3UseRedshirtOnDebris();
	void veng3UseMccoyOnDebris();
	void veng3UseBeamOnDebris();
	void veng3ReachedPositionToPlaceBeam();
	void veng3PlacedBeam();
	void veng3UseSTricorderOnDebris();
	void veng3UseMolecularSawOnDebris();
	void veng3TalkToKirk();
	void veng3TalkToSpock();
	void veng3TalkToMccoy();
	void veng3TalkToRedshirt();
	void veng3GetDebris();
	void veng3GetCable();
	void veng3ReachedCable();
	void veng3PickedUpCable();

	// VENG4
	void veng4Tick1();
	void veng4TalkToKirk();
	void veng4TalkToMccoy();
	void veng4TalkToSpock();
	void veng4TalkToRedshirt();
	void veng4UseMccoyOnDeadGuy();
	void veng4UseHypoOnBrittany();
	void veng4UseMedkitOnBrittany();
	void veng4UseMTricorderOnBrittany();
	void veng4UseMccoyOnBrittany();
	void veng4MccoyReachedBrittany();
	void veng4UsedMedkitOnBrittany();
	void veng4MccoyScannedBrittany();
	void veng4TalkToBrittany();
	void veng4LookAtDeadGuy();
	void veng4LookAtRightBedReadings();
	void veng4LookAtDoorHotspot();
	void veng4LookAtKirk();
	void veng4LookAtSpock();
	void veng4LookAtMccoy();
	void veng4LookAtRedshirt();
	void veng4LookAtLeftBed();
	void veng4LookAtLeftBedReadings();
	void veng4LookAtBrittany();
	void veng4LookAtDrill();
	void veng4LookAtHypoOnTable();
	void veng4LookAtDoorObject();
	void veng4LookAnywhere();
	void veng4GetHypo();
	void veng4ReachedHypo();
	void veng4PickedUpHypo();
	void veng4GetDrill();
	void veng4ReachedDrill();
	void veng4PickedUpDrill();
	void veng4TouchedHotspot0();
	void veng4WalkToDoor();

	// VENG5
	void veng5Tick1();
	void veng5Tick45();
	void veng5WalkToDoor();
	void veng5ReachedDoor();
	void veng5DoorOpened();
	void veng5TouchedTurboliftDoor();
	void veng5LookAtDoor();
	void veng5LookAtLadder();
	void veng5LookAtDeadGuy();
	void veng5LookAtPowerPack();
	void veng5LookAtDebris();
	void veng5LookAtKirk();
	void veng5LookAtSpock();
	void veng5LookAtMccoy();
	void veng5LookAtRedshirt();
	void veng5LookAnywhere();
	void veng5UsePhaserOnPowerPack();
	void veng5UseStunPhaserOnDebris();
	void veng5UseKillPhaserOnDebris();
	void veng5ReachedPositionToShootDebris();
	void veng5DrewPhaser();
	void veng5VaporizedDebris();
	void veng5UseMTricorderOnDebris();
	void veng5UseSTricorderOnDebris();
	void veng5UseMTricorderOnPowerPack();
	void veng5UseSTricorderOnPowerPack();
	void veng5TalkToKirk();
	void veng5TalkToMccoy();
	void veng5TalkToSpock();
	void veng5TalkToRedshirt();
	void veng5TouchedHotspot0();
	void veng5GetPowerPack();
	void veng5ReachedPowerPack();
	void veng5PickedUpPowerPack();

	// VENG6
	void veng6Tick1();
	void veng6WalkToDoor();
	void veng6ReachedDoor();
	void veng6DoorOpened();
	void veng6LookAtLeftPort();
	void veng6LookAtRightPort();
	void veng6LookAtDoor();
	void veng6LookAtOilCanister();
	void veng6LookAtCabinet();
	void veng6LookAtCable();
	void veng6LookAtRightConsole();
	void veng6LookAtImpulseEngines();
	void veng6LookAtLeftConsole();
	void veng6LookAtJunctionBox();
	void veng6LookAtJournal();
	void veng6LookAtKirk();
	void veng6LookAtSpock();
	void veng6LookAtMccoy();
	void veng6LookAtRedshirt();
	void veng6LookAtDeadGuy();
	void veng6LookAnywhere();
	void veng6TalkToKirk();
	void veng6TalkToSpock();
	void veng6TalkToMccoy();
	void veng6TalkToRedshirt();
	void veng6UseCable1OnLeftPort();
	void veng6UseCable2OnLeftPort();
	void veng6UseCableOnLeftPort(byte cable);
	void veng6KirkOrSpockReachedLeftPort();
	void veng6AttachedLeftCable();
	void veng6UseCable1OnRightPort();
	void veng6UseCable2OnRightPort();
	void veng6UseCableOnRightPort(byte cable);
	void veng6KirkOrSpockReachedRightPort();
	void veng6AttachedRightCable();
	void veng6UseCrewmanOnJunctionBox();
	void veng6SpockReachedJunctionBox();
	void veng6SpockAdjustedJunctionBox();
	void veng6UseHypoOnOilCanister();
	void veng6ReachedOilCanister();
	void veng6FilledHypoWithOil();
	void veng6UseCabinet();
	void veng6ReachedCabinet();
	void veng6OpenedOrClosedCabinet();
	void veng6UseSTricorderOnImpulseEngines();
	void veng6UseSTricorderOnRightConsole();
	void veng6UseSTricorderOnJunctionBox();
	void veng6UsePowerPackOnLeftConsole();
	void veng6GetJunctionBox();
	void veng6GetOilCanister();
	void veng6GetEngineeringJournal();
	void veng6ReachedEngineeringJournal();
	void veng6TookEngineeringJournal();

	// VENG7
	void veng7Tick1();
	void veng7WalkToDoor();
	void veng7ReachedDoor();
	void veng7DoorOpened();
	void veng7TouchedTurboliftDoor();
	void veng7LookAtCollapsedSection();
	void veng7LookAtDeadGuy();
	void veng7LookAtDoor();
	void veng7LookAtCable();
	void veng7LookAtKirk();
	void veng7LookAtSpock();
	void veng7LookAtMccoy();
	void veng7LookAtRedshirt();
	void veng7LookAnywhere();
	void veng7TalkToKirk();
	void veng7TalkToSpock();
	void veng7TalkToMccoy();
	void veng7TalkToRedshirt();
	void veng7UseSTricorderOnCollapsedSection();
	void veng7TouchedHotspot0();
	void veng7GetCable();
	void veng7ReachedCable();
	void veng7PickedUpCable();

	// VENG8
	void veng8Tick1();
	void veng8WalkToDoor();
	void veng8ReachedDoor();
	void veng8DoorOpened();
	void veng8Timer0Expired();
	void veng8SparkAnim1Done();
	void veng8Timer1Expired();
	void veng8SparkAnim2Done();
	void veng8LookAtPowerPack();
	void veng8LookAtDoor();
	void veng8LookAtPowerCircuits();
	void veng8LookAtControls();
	void veng8LookAtDeadGuy();
	void veng8LookAtSlider();
	void veng8LookAtKirk();
	void veng8LookAtSpock();
	void veng8LookAtMccoy();
	void veng8LookAtRedshirt();
	void veng8LookAtTransporter();
	void veng8LookAnywhere();
	void veng8TalkToKirk();
	void veng8TalkToSpock();
	void veng8TalkToMccoy();
	void veng8TalkToRedshirt();
	void veng8UseKirkOnControls();
	void veng8UseSpockOnControls();
	void veng8SpockReachedControls();
	void veng8SpockUsedControls();
	void veng8UsePowerPackOnPowerCircuit();
	void veng8SpockReachedPowerCircuit();
	void veng8SpockConnectedPowerPack();
	void veng8TransporterPoweredUp();
	void veng8UseKirkOnSlider();
	void veng8UseSpockOnSlider();
	void veng8SpockReachedSlider();
	void veng8RedshirtReachedTransporter();
	void veng8SpockPulledSliderToBeamOut();
	void veng8RedshirtBeamedOut();
	void veng8SpockPulledSliderToBeamIn();
	void veng8RedshirtBeamedIn();
	void veng8UseSTricorderOnConsole();

	// VENGA (common code)
	void vengaTick();
	void vengaElasiBeamOver();
	void vengaUsePhaserAnywhere();
	void vengaLookAtHypo();
	void vengaUseCommunicator();
	void vengaUseMccoyOnDeadGuy();

public:
	// Room-specific variables. This is memset'ed to 0 when the room is initialized.
	// NOTE: Any changes here must be reflected in the corresponding serializer functions.
	union {
		struct {
			// demon0
			byte bottomDoorCounter; // 0xca
			byte topDoorCounter; // 0xcb
			bool movingToTopDoor; // 0xcc
			bool movingToBottomDoor; // 0xcd

			// demon1
			byte klingonShot[3]; // 0xca
			int16 numKlingonsKilled;
			byte attackIndex;
			bool kirkShooting;
			char d6[10];

			// demon3
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

			// demon4
			bool triedToShootNauian; // 0xca
			bool nauianEmerged; // 0xcb
			bool disabledSecurity; // 0xcc
			bool cd; // 0xcd
			byte crewReadyToBeamOut; // 0xce
			int16 crewmanUsingPanel; // 0xcf

			// demon5
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

			// demon6
			bool insultedStephenRecently; // 0xca
			bool stephenInRoom; // 0xcb
			bool caseOpened; // 0xcd

			// common
			bool movingToDoor;
			byte doorCounter;

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// demon0
				ser.syncAsByte(bottomDoorCounter);
				ser.syncAsByte(topDoorCounter);
				ser.syncAsByte(movingToTopDoor);
				ser.syncAsByte(movingToBottomDoor);

				// demon1
				ser.syncBytes((byte *)klingonShot, 3);
				ser.syncAsSint16LE(numKlingonsKilled);
				ser.syncAsByte(attackIndex);
				ser.syncAsByte(kirkShooting);
				ser.syncBytes((byte *)d6, 10);

				// demon3
				ser.syncAsByte(shootingBoulder);
				ser.syncAsByte(boulder1Shot);
				ser.syncAsByte(boulderBeingShot);
				ser.syncAsByte(kirkInPosition);
				ser.syncAsByte(redshirtInPosition);
				ser.syncAsByte(spockInPosition);
				ser.syncAsByte(mccoyInPosition);
				ser.syncAsByte(inFiringPosition);
				ser.syncAsByte(kirkPhaserOut);
				ser.syncBytes((byte *)boulderAnim, 10);
				ser.syncAsSint16LE(usedPhaserOnDoor);

				// demon4
				ser.syncAsByte(triedToShootNauian);
				ser.syncAsByte(nauianEmerged);
				ser.syncAsByte(disabledSecurity);
				ser.syncAsByte(cd);
				ser.syncAsByte(crewReadyToBeamOut);
				ser.syncAsSint16LE(crewmanUsingPanel);

				// demon5
				ser.syncAsByte(scannedRoberts);
				ser.syncAsByte(scannedChub);
				ser.syncAsByte(scannedGrisnash);
				ser.syncAsByte(scannedStephen);
				ser.syncAsByte(numScanned);
				ser.syncAsByte(numTalkedTo);
				ser.syncAsByte(talkedToRoberts);
				ser.syncAsByte(talkedToChub);
				ser.syncAsByte(talkedToGrisnash);
				ser.syncAsByte(talkedToStephen);

				// demon6
				ser.syncAsByte(insultedStephenRecently);
				ser.syncAsByte(stephenInRoom);
				ser.syncAsByte(caseOpened);

				// common
				ser.syncAsByte(movingToDoor);
				ser.syncAsByte(doorCounter);
			}
		} demon;

		struct {
			// tug2
			byte shootingObject; // 0x1ec1
			byte shootingTarget;
			bool elasiPhaserOnKill;
			bool shootKirkOverride; // 0x1ec4

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// tug2
				ser.syncAsByte(shootingObject);
				ser.syncAsByte(shootingTarget);
				ser.syncAsByte(elasiPhaserOnKill);
				ser.syncAsByte(shootKirkOverride);
			}
		} tug;

		struct {
			// love0
			bool heardSummaryOfVirus; // 0xda

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

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// love0
				ser.syncAsByte(heardSummaryOfVirus);
				uint16 tmp = 0;
				uint32 tmp2 = 0;
				byte tmp3[10];
				ser.syncAsSint16LE(tmp);	// consoleCrewman
				ser.syncBytes((byte *)tmp3, 10);	// consoleAnimation
				ser.syncAsSint32LE(tmp2);	// consoleSpeaker
				ser.syncAsSint32LE(tmp2);	// consoleText

				// love1
				ser.syncAsSint32LE(dyingSpeaker);
				ser.syncAsSint16LE(crewmanUsingFreezerRetX);
				ser.syncAsSint16LE(crewmanUsingFreezerRetY);
				ser.syncAsSint16LE(crewmanUsingDevice);
				ser.syncAsSint16LE(itemInNozzle);
				ser.syncBytes((byte *)bottleAnimation, 10);

				// love2
				ser.syncAsByte(canisterType);
				ser.syncAsByte(cb);
				ser.syncAsSint16LE(canisterItem);
				ser.syncBytes((byte *)canisterAnim, 10);
				ser.syncAsSint16LE(chamberObject);
				ser.syncBytes((byte *)chamberInputAnim, 10);
				ser.syncBytes((byte *)chamberOutputAnim, 10);

				// love3
				ser.syncAsByte(activeCrewman);

				// love4
				ser.syncAsByte(gaveWaterToRomulans);

				// love5
				ser.syncAsByte(numCrewmenReadyToBeamOut);

				// common
				ser.syncAsByte(walkingToDoor);
				ser.syncAsByte(doorOpenCounter);
				ser.syncAsByte(spockAndMccoyReadyToUseCure);
				ser.syncAsByte(cmnXPosToCureSpock);
				ser.syncAsByte(cmnYPosToCureSpock);
			}
		} love;

		struct {
			// mudd3
			bool suggestedUsingTricorders; // 0xca
			bool tricordersUnavailable; // 0xcb

			// mudd4
			bool usingLeftConsole; // 0xca
			bool kirkUsingRightConsole; // 0xcb

			// common
			byte walkingToDoor;

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// mudd3
				ser.syncAsByte(suggestedUsingTricorders);
				ser.syncAsByte(tricordersUnavailable);

				// mudd4
				ser.syncAsByte(usingLeftConsole);
				ser.syncAsByte(kirkUsingRightConsole);

				// common
				ser.syncAsByte(walkingToDoor);
			}
		} mudd;

		struct {
			// feather1
			byte snakeInHole; // 0xca
			bool scannedSnake; // 0xcb
			byte crewEscaped[4]; // 0xcc
			byte crewmanClimbingVine;

			// feather2
			bool showedSnakeToTlaoxac; // 0xca
			bool tlaoxacUnconscious; // 0xcb

			// feather6
			bool usedRockOnCrystalsOnce;

			// feather7
			bool insultedQuetzecoatl;

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// feather1
				ser.syncAsByte(snakeInHole);
				ser.syncAsByte(scannedSnake);
				ser.syncBytes(crewEscaped, 4);
				ser.syncAsByte(crewmanClimbingVine);

				// feather2
				ser.syncAsByte(showedSnakeToTlaoxac);
				ser.syncAsByte(tlaoxacUnconscious);

				// feather6
				ser.syncAsByte(usedRockOnCrystalsOnce);

				// feather7
				ser.syncAsByte(insultedQuetzecoatl);
			}
		} feather;

		struct {
			// trial2
			byte globBeingShot;
			bool phaserOnKill;

			// trial5
			int16 itemToUse; // 0x1386
			int16 objectToUse; // 0x1388
			int16 hole;

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// trial2
				ser.syncAsByte(globBeingShot);
				ser.syncAsByte(phaserOnKill);

				// trial5
				ser.syncAsSint16LE(itemToUse);
				ser.syncAsSint16LE(objectToUse);
				ser.syncAsSint16LE(hole);
			}
		} trial;

		struct {
			// sins5
			byte numCrewmenInPositionForWire; // 0xca
			byte numCrewmenReadyToBeamOut; // 0xcb

			void saveLoadWithSerializer(Common::Serializer &ser) {
				ser.syncAsByte(numCrewmenInPositionForWire);
				ser.syncAsByte(numCrewmenReadyToBeamOut);
			}
		} sins;

		struct {
			// veng1
			byte doorOpenCounter; // 0xca

			// veng2
			byte numCrewmenReadyToBeamOut; // 0xca

			// veng4
			bool usingMedkitOnBrittany;

			// veng6
			byte kirkAndSpockReadyToAttachLeftCable; // 0xca
			byte kirkAndSpockReadyToAttachRightCable; // 0xcb
			byte cableInUse; // 0xcc

			// venga (common)
			bool walkingToDoor; // 0xcb (veng1), 0xca (veng4)

			void saveLoadWithSerializer(Common::Serializer &ser) {
				// veng1
				ser.syncAsByte(doorOpenCounter);

				// veng2
				ser.syncAsByte(numCrewmenReadyToBeamOut);

				// veng4
				ser.syncAsByte(usingMedkitOnBrittany);

				// veng6
				ser.syncAsByte(kirkAndSpockReadyToAttachLeftCable);
				ser.syncAsByte(kirkAndSpockReadyToAttachRightCable);
				ser.syncAsByte(cableInUse);

				// venga (common)
				ser.syncAsByte(walkingToDoor);
			}
		} veng;

	} _roomVar;
};

} // End of namespace StarTrek

#endif
