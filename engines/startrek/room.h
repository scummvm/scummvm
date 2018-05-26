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

const char SPEAKER_KIRK[] = "Capt. Kirk";
const char SPEAKER_SPOCK[] = "Mr. Spock";
const char SPEAKER_MCCOY[] = "Dr. McCoy";
const char SPEAKER_EVERTS[] = "Ensign Everts";

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
	int showRoomSpecificText(const char **textAddr);                                           // Cmd 0x03
	void giveItem(int item);                                                                   // Cmd 0x04
	void loadRoomIndex(int roomIndex, int spawnIndex);                                         // Cmd 0x06
	void walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam);// Cmd 0x08
	void loadMapFile(const Common::String &name);                                              // Cmd 0x09
	Common::Point getActorPos(int actorIndex);                                                 // Cmd 0x0d
	void playSoundEffectIndex(int soundEffect);                                                // Cmd 0x0f
	void playMidiMusicTracks(int startTrack, int loopTrack);                                   // Cmd 0x10
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
		} demon3;
	} _roomVar;
};

}

#endif
