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
#include "queen/structs.h"
#include "queen/verb.h"

namespace Queen {

class Logic;
class Graphics;
class Input;
class Walk;
class Sound;

struct CmdText {

	void clear();
	void display(uint8 color);
	void displayTemp(uint8 color, bool locked, const Verb& v, const char *name = NULL);
	void displayTemp(uint8 color, const char *name);
	void setVerb(const Verb& v);
	void addLinkWord(const Verb& v);
	void addObject(const char *objName);
	bool isEmpty() const;

	enum {
		MAX_COMMAND_LEN = 256,
		COMMAND_Y_POS   = 151
	};

	char _command[MAX_COMMAND_LEN];
	Graphics *_graphics;
	Logic *_logic;
};


struct CurrentCmdState {

	void init();
	void addObject(int16 objNum);

	Verb oldVerb;
	Verb verb;
	Verb action;
	int16 oldNoun;
	int16 noun;
	int commandLevel;
	int16 subject1;
	int16 subject2;
};


struct SelectedCmdState {

	void init();
	
	Verb defaultVerb;
	Verb action;
	int16 noun;
};


class Command {
public:
	
	Command(Logic *, Graphics *, Input *, Walk *, Sound *);
	
	//! initialise command construction
	void clear(bool clearTexts);

	//! execute last constructed command
	void executeCurrentAction(bool walk);

	//! get player input and construct command from it
	void updatePlayer();

	//! read all command arrays from stream
	void readCommandsFrom(byte *&ptr);

	//! return true if command is ready to be executed
	bool parse() const { return _parse; }

	enum {
		MAX_MATCHING_CMDS = 50
	};


private:

	int16 executeCommand(uint16 comId, int16 condResult);

	int16 makeJoeWalkTo(int16 x, int16 y, int16 objNum, const Verb &v, bool mustWalk);

	void grabCurrentSelection();
	void grabSelectedObject(int16 objNum, uint16 objState, uint16 objName);
	void grabSelectedItem();
	void grabSelectedNoun();
	void grabSelectedVerb();

	bool executeIfCutaway(const char *description);
	bool executeIfDialog(const char *description);
	
	bool handleDefaultCommand(bool walk);
	void executeStandardStuff(const Verb& action, int16 subj1, int16 subj2);
	void changeObjectState(const Verb& action, int16 obj, int16 song, bool cutDone);
	void cleanupCurrentAction();

	//! find default verb action for specified object
	Verb findDefault(uint16 obj, bool itemType);

	//! alter default verb action for specified object and update command display
	void alterDefault(const Verb& def, bool itemType);
	
	//! OPEN_CLOSE_OTHER(OBJECT_DATA[S][4])
	void openOrCloseAssociatedObject(const Verb& action, int16 obj);
	
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

	//! look at current object / item and speak its description
	void look();
	void lookCurrentItem();
	void lookCurrentRoom();
	void lookCurrentIcon();


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

	CurrentCmdState _curCmd;

	SelectedCmdState _selCmd;

	//! last user selection
	int _mouseKey, _selPosX, _selPosY;

	Logic *_logic;
	Graphics *_graphics;
	Input *_input;
	Sound *_sound;
	Walk *_walk;

};

} // End of namespace Queen

#endif

