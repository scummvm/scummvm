/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_scripts_h__
#define __lure_scripts_h__

#include "lure/luredefs.h"
#include "lure/memory.h"
#include "lure/hotspots.h"

namespace Lure {

// Opcode list
#define S_OPCODE_ABORT 0
#define S_OPCODE_ADD 1
#define S_OPCODE_SUBTRACT 2
#define S_OPCODE_MULTIPLY 3
#define S_OPCODE_DIVIDE 4
#define S_OPCODE_NOT_EQUALS 5
#define S_OPCODE_EQUALS 6
#define S_OPCODE_GT 7
#define S_OPCODE_LT 8
#define S_OPCODE_LT2 9
#define S_OPCODE_GT2 10
#define S_OPCODE_AND 11
#define S_OPCODE_OR 12
#define S_OPCODE_LOGICAL_AND 13
#define S_OPCODE_LOGICAL_OR 14
#define S_OPCODE_GET_FIELD 15
#define S_OPCODE_SET_FIELD 16
#define S_OPCODE_PUSH 17
#define S_OPCODE_SUBROUTINE 18
#define S_OPCODE_EXEC 19
#define S_OPCODE_END 20
#define S_OPCODE_COND_JUMP 21
#define S_OPCODE_JUMP 22
#define S_OPCODE_ABORT2 23
#define S_OPCODE_ABORT3 24
#define S_OPCODE_RANDOM 25

#define S2_OPCODE_TIMEOUT -1
#define S2_OPCODE_POSITION -2
#define S2_OPCODE_CHANGE_POS -3
#define S2_OPCODE_UNLOAD -4
#define S2_OPCODE_DIMENSIONS -5
#define S2_OPCODE_JUMP -6
#define S2_OPCODE_ANIMATION -7
#define S2_OPCODE_UNKNOWN_247 -8
#define S2_OPCODE_UNKNOWN_258 -9
#define S2_OPCODE_ACTIONS -10



class Script {
public:
	static uint16 execute(uint16 startOffset);

	static void activateHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void setHotspotScript(uint16 hotspotId, uint16 scriptIndex, uint16 v3);
	static void clearSequenceDelayList(uint16 v1, uint16 scriptIndex, uint16 v3);
	static void deactivateHotspotSet(uint16 listIndex, uint16 v2, uint16 v3);
	static void method2(uint16 v1, uint16 v2, uint16 v3);
	static void deactivateHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void setActionsOffset(uint16 hotspotId, uint16 offset, uint16 v3);
	static void addDelayedSequence(uint16 seqOffset, uint16 delay, uint16 v3);
	static void characterInRoom(uint16 characterId, uint16 roomNumber, uint16 v3);
	static void setHotspotName(uint16 hotspotId, uint16 nameId, uint16 v3);
	static void playSound(uint16 v1, uint16 v2, uint16 v3);
	static void displayDialog(uint16 stringId, uint16 v2, uint16 v3);
	static void remoteRoomViewSetup(uint16 v1, uint16 v2, uint16 v3);
	static void getDoorBlocked(uint16 hotspotId, uint16 v2, uint16 v3);
	static void isSkorlInCell(uint16 v1, uint16 v2, uint16 v3);
	static void setBlockingHotspotScript(uint16 charId, uint16 scriptIndex, uint16 v3);
	static void decrInventoryItems(uint16 v1, uint16 v2, uint16 v3);
	static void setFrameNumber(uint16 hotspotId, uint16 offset, uint16 v3);
	static void disableHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void cutSack(uint16 hotspotId, uint16 v2, uint16 v3);
	static void increaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3);
	static void enableHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void transformPlayer(uint16 v1, uint16 v2, uint16 v3);
	static void room14DoorClose(uint16 v1, uint16 v2, uint16 v3);
	static void checkDroppedDesc(uint16 hotspotId, uint16 v2, uint16 v3);
	static void doorClose(uint16 hotspotId, uint16 v2, uint16 v3);
	static void displayMessage(uint16 messageId, uint16 characterId, uint16 unknownVal);
	static void doorOpen(uint16 hotspotId, uint16 v2, uint16 v3);
	static void givePlayerItem(uint16 hotspotId, uint16 v2, uint16 v3);
	static void decreaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3);
	static void setVillageSkorlTickProc(uint16 v1, uint16 v2, uint16 v3);
	static void getNumGroats(uint16 v1, uint16 v2, uint16 v3);
	static void animationLoad(uint16 hotspotId, uint16 v2, uint16 v3);
	static void addActions(uint16 hotspotId, uint16 actions, uint16 v3);
	static void checkCellDoor(uint16 v1, uint16 v2, uint16 v3);
};

class HotspotScript {
private:
	static int16 nextVal(MemoryBlock *data, uint16 &offset);
public:
	static bool execute(Hotspot *h);
};

} // End of namespace Lure

#endif
