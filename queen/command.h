/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENCOMMAND_H
#define QUEENCOMMAND_H

#include "common/util.h"
#include "queen/defs.h"
#include "queen/structs.h"

namespace Queen {

class QueenEngine;

struct CmdText {

	void clear();
	void display(uint8 color);
	void displayTemp(uint8 color, bool locked, Verb v, const char *name = NULL);
	void displayTemp(uint8 color, const char *name);
	void setVerb(Verb v);
	void addLinkWord(Verb v);
	void addObject(const char *objName);
	bool isEmpty() const;

	enum {
		MAX_COMMAND_LEN = 256,
		COMMAND_Y_POS   = 151
	};

	char _command[MAX_COMMAND_LEN];
	QueenEngine *_vm;
};


struct CmdState {

	void init();

	Verb oldVerb, verb;
	Verb action;
	int16 oldNoun, noun;
	int commandLevel;
	int16 subject[2];

	Verb defaultVerb;
	Verb selAction;
	int16 selNoun;
};


class Command {
public:
	
	Command(QueenEngine *vm);
	
	//! initialise command construction
	void clear(bool clearTexts);

	//! execute last constructed command
	void executeCurrentAction();

	//! get player input and construct command from it
	void updatePlayer();

	//! read all command arrays from stream
	void readCommandsFrom(byte *&ptr);


	enum {
		MAX_MATCHING_CMDS = 50
	};


private:

	ObjectData *findObjectData(uint16 objRoomNum) const;
	ItemData *findItemData(Verb invNum) const;

	int16 executeCommand(uint16 comId, int16 condResult);

	int16 makeJoeWalkTo(int16 x, int16 y, int16 objNum, Verb v, bool mustWalk);

	void grabCurrentSelection();
	void grabSelectedObject(int16 objNum, uint16 objState, uint16 objName);
	void grabSelectedItem();
	void grabSelectedNoun();
	void grabSelectedVerb();

	bool executeIfCutaway(const char *description);
	bool executeIfDialog(const char *description);
	
	bool handleWrongAction();
	void sayInvalidAction(Verb action, int16 subj1, int16 subj2);
	void changeObjectState(Verb action, int16 obj, int16 song, bool cutDone);
	void cleanupCurrentAction();
	
	//! OPEN_CLOSE_OTHER(OBJECT_DATA[S][4])
	void openOrCloseAssociatedObject(Verb action, int16 obj);
	
	//! update gamestates - P1_SET_CONDITIONS
	int16 setConditions(uint16 command, bool lastCmd);

	//! turn on/off areas - P2_SET_AREAS
	void setAreas(uint16 command);

	//! hide/show objects, redisplay if in the same room as Joe - P3_SET_OBJECTS
	void setObjects(uint16 command);
	
	//! inserts/deletes items (inventory) - P4_SET_ITEMS
	void setItems(uint16 command);

	//! update description for object and returns description number to use
	uint16 nextObjectDescription(ObjectDescription *objDesc, uint16 firstDesc);

	//! speak description of selected object
	void lookAtSelectedObject();

	//! get the current objects under the cursor
	void lookForCurrentObject();

	//! get the current icon panel under the cursor (inventory item or verb)
	void lookForCurrentIcon();

	bool isVerbAction(Verb v) const { return (v >= VERB_PANEL_COMMAND_FIRST && v <= VERB_PANEL_COMMAND_LAST) || (v == VERB_WALK_TO); };
	bool isVerbInv(Verb v) const { return v >= VERB_INV_FIRST && v <= VERB_INV_LAST; }
 
	CmdListData *_cmdList;
	uint16 _numCmdList;

	CmdArea *_cmdArea;
	uint16 _numCmdArea;

	CmdObject *_cmdObject;
	uint16 _numCmdObject;

	CmdInventory *_cmdInventory;
	uint16 _numCmdInventory;

	CmdGameState *_cmdGameState;
	uint16 _numCmdGameState;

	//! textual form of the command (displayed between room and panel areas)
	CmdText _cmdText;
	
	//! flag indicating that the current command is fully constructed
	bool _parse;

	//! state of current constructed command
	CmdState _state;

	//! last user selection
	int _mouseKey, _selPosX, _selPosY;

	QueenEngine *_vm;
};

} // End of namespace Queen

#endif

