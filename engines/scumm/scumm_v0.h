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

#ifndef SCUMM_SCRIPT_V0_H
#define SCUMM_SCRIPT_V0_H

#include "scumm/scumm_v2.h"

namespace Scumm {

/**
 * Engine for Apple II and Commodore 64 versions of Maniac Mansion
 */
class ScummEngine_v0 : public ScummEngine_v2 {
protected:
	enum CurrentMode {
		kModeCutscene = 0,   // cutscene active
		kModeKeypad = 1,     // kid selection / dial pad / save-load dialog
		kModeNoNewKid = 2,   // verb "new kid" disabled (e.g. when entering lab)
		kModeNormal = 3,     // normal playing mode
	};

	enum ObjectType {
		kObjectTypeFG = 0,    // foreground object
		                      //   - with owner/state, might (but has not to) be pickupable
		                      //     -> with entry in _objectOwner/StateTable
		                      //     -> all objects in _inventory have this type
		                      //   - image can be exchanged (background overlay)
		kObjectTypeBG = 1,    // background object
		                      //   - without owner/state, not pickupable  (room only)
		                      //     -> without entry in _objectOwner/StateTable
		                      //   - image cannot be exchanged (part of background image)
		kObjectTypeActor = 2  // object is an actor
	};

protected:
	byte _currentMode;

	int _activeVerb;
	int _activeObjectNr;			// 1st Object Number
	int _activeObjectType;			// 1st Object Type (0: inventory (or room), 1: room)
	int _activeObject2Nr;			// 2nd Object Number
	int _activeObject2Type;			// 2nd Object Type (0: inventory (or room), 1: room, 2: actor)

	int _cmdVerb;
	int _cmdObjectNr;
	int _cmdObjectType;
	int _cmdObject2Nr;
	int _cmdObject2Type;

	int _walkToObject;
	int _walkToObjectIdx;

public:
	ScummEngine_v0(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	virtual void setupOpcodes();

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void scummLoop(int delta);
	virtual void decodeParseString();

	virtual void processInput();

	virtual void saveOrLoad(Serializer *s);

	// V0 MM Verb commands
	int getVerbPrepId();
	int activeVerbPrep();
	void walkToActorOrObject(int object);
	void verbExec();

	virtual void runSentenceScript();
	virtual void checkAndRunSentenceScript();
	virtual void checkExecVerbs();
	virtual void handleMouseOver(bool updateInventory);
	int verbPrepIdType(int verbid);
	void resetVerbs();
	void setNewKidVerbs();

	void drawSentenceObject(int object);
	void drawSentence();

	void switchActor(int slot);

	virtual int getVarOrDirectWord(byte mask);
	virtual uint fetchScriptWord();

	virtual int getActiveObject();

	virtual void resetSentence(bool walking);

	virtual bool areBoxesNeighbors(int box1nr, int box2nr);

	bool ifEqualActiveObject2Common(bool ignoreType);

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
	void o_getObjectOwner();
	void o_getActorBitVar();
	void o_setActorBitVar();
	void o_getBitVar();
	void o_setBitVar();
	void o_doSentence();
	void o_ifEqualActiveObject2();
	void o_ifNotEqualActiveObject2();
	void o_getClosestObjActor();
	void o_printEgo_c64();
	void o_print_c64();
	void o_unlockRoom();
	void o_unlockSound();
	void o_cutscene();
	void o_endCutscene();
	void o_beginOverride();
	void o_setOwnerOf();

	byte VAR_ACTIVE_OBJECT2;
	byte VAR_IS_SOUND_RUNNING;
	byte VAR_ACTIVE_VERB;
};


} // End of namespace Scumm

#endif
