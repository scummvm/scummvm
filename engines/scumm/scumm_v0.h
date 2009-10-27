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
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_SCRIPT_V0_H
#define SCUMM_SCRIPT_V0_H

#include "scumm/scumm_v2.h"

namespace Scumm {

/**
 * Engine for Apple II and Commodore 64 versions of Maniac Mansion
 */
class ScummEngine_v0 : public ScummEngine_v2 {
protected:
	byte _currentMode;
	bool _verbExecuting;			// is a verb executing
	bool _verbPickup;				// are we picking up an object during a verb execute

	int _activeActor;				// Actor Number
	int _activeObject2;				// 2nd Object Number

	bool _activeInvExecute;			// is activeInventory first to be executed
	bool _activeObject2Inv;			// is activeobject2 in the inventory
	bool _activeObjectObtained;		// collected _activeobject?
	bool _activeObject2Obtained;	// collected _activeObject2?

	int _activeObjectIndex;
	int _activeObject2Index;

public:
	ScummEngine_v0(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	virtual void setupOpcodes();

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void decodeParseString();

	virtual void processInput();

	virtual void runObject(int obj, int entry);
	virtual void saveOrLoad(Serializer *s);

	// V0 MM Verb commands
	int  verbPrep(int object);
	bool verbMove(int object, int objectIndex, bool invObject);
	bool verbMoveToActor(int actor);
	bool verbObtain(int object, int objIndex);
	bool verbExecutes(int object, bool inventory = false);
	bool verbExec();

	int findObjectIndex(int x, int y);

	virtual void checkExecVerbs();
	virtual void handleMouseOver(bool updateInventory);
	void resetVerbs();
	void setNewKidVerbs();

	void drawSentenceWord(int object, bool usePrep, bool objInInventory);
	void drawSentence();

	void switchActor(int slot);

	virtual int getVarOrDirectWord(byte mask);
	virtual uint fetchScriptWord();

	virtual int getActiveObject();

	virtual void resetSentence(bool walking = false);

	virtual bool areBoxesNeighbours(int box1nr, int box2nr);

	/* Version C64 script opcodes */
	void o_stopCurrentScript();
	void o_loadSound();
	void o_getActorMoving();
	void o_animateActor();
	void o_putActorAtObject();
	void o_pickupObject();
	void o_setObjectName();
	void o_lockSound();
	void o_lockCostume();
	void o_loadCostume();
	void o_loadRoom();
	void o_loadRoomWithEgo();
	void o_lockScript();
	void o_loadScript();
	void o_lockRoom();
	void o_cursorCommand();
	void o_lights();
	void o_unlockCostume();
	void o_unlockScript();
	void o_decrement();
	void o_nop();
	void o_getActorBitVar();
	void o_setActorBitVar();
	void o_getBitVar();
	void o_setBitVar();
	void o_doSentence();
	void o_unknown2();
	void o_ifActiveObject();
	void o_getClosestObjActor();
	void o_printEgo_c64();
	void o_print_c64();
	void o_unlockRoom();
	void o_unlockSound();
	void o_cutscene();
	void o_endCutscene();
	void o_beginOverride();
	void o_setOwnerOf();
};


} // End of namespace Scumm

#endif
