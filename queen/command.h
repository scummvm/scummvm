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

#include "queen/queen.h"
#include "queen/defs.h"
#include "queen/verb.h"

namespace Queen {

class Logic;
class Graphics;
class Input;
class Walk;


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


class Command {
public:
	
	Command(Logic*, Graphics*, Input*, Walk*);
	
	void clear(bool clearTexts);

	void executeCurrentAction(bool walk);

	void updatePlayer();

	void readCommandsFrom(byte *&ptr);

	Verb selectedAction() const { return _selectedAction; }
	int16 selectedNoun() const { return _selectedNoun; }
	bool parse() const { return _parse; }

private:

	void grabCurrentSelection();
	void grabSelectedItem();
	void grabSelectedNoun();
	void grabSelectedVerb();

	bool executeIfCutaway(const char* description);
	bool executeIfDialog(const char* description);
	
	uint16 countAssociatedCommands(const Verb& verb, int16 subj1, int16 subj2);
	bool handleBadCommand(bool walk);
	void executeStandardStuff(const Verb& action, int16 subj1, int16 subj2);
	void changeObjectState(const Verb& action, int16 obj, int16 song, bool cutDone);
	void cleanupCurrentAction();

	Verb findDefault(uint16 obj, bool itemType);
	void alterDefault(const Verb& def, bool itemType);
	
	//! Opens/closes the object associated with object - OPEN_CLOSE_OTHER(OBJECT_DATA[S][4])
	void openOrCloseAssociatedObject(const Verb& action, int16 obj);
	
	//! Update gamestates - P1_SET_CONDITIONS
	int16 setConditions(uint16 command, bool lastCmd);

	//! Turn on/off areas - P2_SET_AREAS
	void setAreas(uint16 command);

	//! Hide/show objects, redisplay if in the same room as Joe - P3_SET_OBJECTS
	void setObjects(uint16 command);
	
	//! Inserts/deletes items (inventory) - P4_SET_ITEMS
	void setItems(uint16 command);

	uint16 nextObjectDescription(ObjectDescription* objDesc, uint16 firstDesc);

	//! Look at Objects/Items and speak their description
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

	//! Textual form of the command (displayed between room and panel areas)
	CmdText _cmdText;
	
	//! Locked verb (using 2nd mouse button)
	Verb _defaultVerb;

	//! OLDVERB, VERB
	Verb _oldVerb, _verb;
	
	//! OLDNOUN, NOUN
	int16 _oldNoun, _noun;

	//! If true, command string is executed
	bool _parse;

	//! Current level of the command (max=2 for GIVE and USE verbs)
	int _commandLevel;
	
	//! Object selected for action
	int16 _selectedNoun;
	
	//! Last selected action
	Verb _currentAction;

	//! Action to perform
	Verb _selectedAction;
		
	//! SUBJECT[3]
	int16 _subject1, _subject2;

	//! MKEY
	int _mouseKey;

	//! Position of last selection
	int _selPosX, _selPosY;

	Logic *_logic;
	Graphics *_graphics;
	Input *_input;
	Walk *_walk;

};

} // End of namespace Queen

#endif

