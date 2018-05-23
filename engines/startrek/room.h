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
	int showRoomSpecificText(const char **textAddr);                                                 // Cmd 0x03
	void giveItem(int item);                                                                   // Cmd 0x04
	void loadRoomIndex(int roomIndex, int spawnIndex);                                         // Cmd 0x06
	void walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 field66);                // Cmd 0x08
	void playSoundEffectIndex(int soundEffect);                                                // Cmd 0x0f
	void playMidiMusicTracks(int startTrack, int loopTrack);                                   // Cmd 0x10
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
	void demon0TouchedHotspot1();
	void demon0TouchedHotspot0();
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
	void useSTricorderAnywhere();
	void useMTricorderAnywhere();
	void useMTricorderOnPrelate();

	void demon0BadResponse();
};

}

#endif
