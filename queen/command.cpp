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

#include "stdafx.h"
#include "queen/command.h"
#include "queen/display.h"
#include "queen/input.h"
#include "queen/graphics.h"
#include "queen/logic.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

namespace Queen {



void CmdText::clear() {

	memset(_command, 0, sizeof(_command));
}


void CmdText::display(uint8 color) {

	_graphics->textCurrentColor(color);
	_graphics->textSetCentered(COMMAND_Y_POS, _command, false);
}


void CmdText::displayTemp(uint8 color, bool locked, const Verb& v, const char *name) {

	char temp[MAX_COMMAND_LEN];
	if (locked) {
		sprintf(temp, "%s%s", _logic->lockedVerbPrefix(), v.name());
	}
	else {
		strcpy(temp, v.name());
	}
	if (name != NULL) {
		strcat(temp, " ");
		strcat(temp, name);
	}
	_graphics->textCurrentColor(color);
	_graphics->textSetCentered(COMMAND_Y_POS, temp, false);
}


void CmdText::displayTemp(uint8 color, const char *name) {
	
	char temp[MAX_COMMAND_LEN];
	sprintf(temp, "%s %s", _command, name);
	_graphics->textCurrentColor(color);
	_graphics->textSetCentered(COMMAND_Y_POS, temp, false);
}


void CmdText::setVerb(const Verb& v) {

	strcpy(_command, v.name());
}


void CmdText::addLinkWord(const Verb& v) {

	strcat(_command, " ");
	strcat(_command, v.name());
}


void CmdText::addObject(const char *objName) {

	strcat(_command, " ");
	strcat(_command, objName);
}


bool CmdText::isEmpty() const {
	return _command[0] == 0;
}


void CurrentCmdState::init() {

	commandLevel = 1;
	oldVerb = verb = action = Verb(VERB_NONE);
	oldNoun = noun = subject1 = subject2 = 0;
}


void CurrentCmdState::addObject(int16 objNum) {
	
	switch (commandLevel) {
	case 1:
		subject1 = objNum;
		break;
	case 2:
		subject2 = objNum;
		break;
	}
}


void SelectedCmdState::init() {
	
	action = defaultVerb = Verb(VERB_NONE);
	noun = 0;
}


Command::Command(Logic *l, Graphics *g, Input *i, Walk *w, Sound *s)
	: _logic(l), _graphics(g), _input(i), _walk(w), _sound(s) {
	_cmdText._graphics = _graphics;
	_cmdText._logic = _logic;
}


void Command::clear(bool clearTexts) {

	_cmdText.clear();
	if (clearTexts) {
		_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
	}
	_parse = false;
	_curCmd.init();
	_selCmd.init();
}


void Command::executeCurrentAction(bool walk) {

	_logic->entryObj(0);

	const char *obj1Name = NULL;
	const char *obj2Name = NULL;

	if (_curCmd.commandLevel == 2 && _mouseKey == Input::MOUSE_RBUTTON) {
		_mouseKey = Input::MOUSE_LBUTTON;
	}

	// XXX SUBJ1=SUBJECT[1]; SUBJ2=SUBJECT[2];

	uint16 objMax = _logic->currentRoomObjMax();
	uint16 roomData = _logic->currentRoomData();

	if (_mouseKey == Input::MOUSE_RBUTTON && _curCmd.subject1 != 0) {
		// check to see if selecting default command for object/item
		if (_curCmd.subject1 > 0) {
			// an object
			int16 i = _curCmd.subject1;
			if (_curCmd.noun > objMax) {
				int16 aObj = _logic->currentRoomArea(_curCmd.noun - objMax)->object;
				int16 aObjName = _logic->objectData(aObj)->name;
				if (aObjName > 0) {
					_curCmd.noun = objMax;
					i = aObj;
				}
			}
			ObjectData *od = _logic->objectData(ABS(i));
			obj1Name = _logic->objectName(od->name);

			if (_curCmd.noun == 0 || _curCmd.noun > objMax || _logic->objectData(i)->name <= 0) {
				cleanupCurrentAction();
				return;
			}

			uint16 obj = roomData + _curCmd.noun;
			_curCmd.verb = findDefault(obj, false);
			if (_curCmd.verb.isNone()) {
				// no match made, so command not yet completed, redefine as WALK_TO
				_cmdText.setVerb(Verb(VERB_WALK_TO));
				_selCmd.action = Verb(VERB_WALK_TO);
			}
			else {
				_cmdText.setVerb(_curCmd.verb);
				_selCmd.action = _curCmd.verb;
			}
			_cmdText.addObject(_logic->objectName(_logic->objectData(obj)->name));
		}
		else {
			// an item
			int16 name = _logic->itemData(ABS(_curCmd.subject1))->name;
			obj1Name = _logic->objectName(name);
		}
	}

	// make sure that command is always highlighted when actioned!
	_cmdText.display(INK_CMD_SELECT);

	_selCmd.noun = _curCmd.noun;
	_curCmd.commandLevel = 1;

	// XXX SUBJECT[2]=0;

	// get objects names
	obj1Name = _logic->objectOrItemName(_curCmd.subject1);
	obj2Name = _logic->objectOrItemName(_curCmd.subject2);

	if (handleBadCommand(walk)) {
		cleanupCurrentAction();
		return;
	}

	uint16 i;

	// get the commands associated with object/item
	uint16 comMax = 0;
	uint16 matchingCmds[MAX_MATCHING_CMDS];
	CmdListData *cmdList = &_cmdList[1];
	for (i = 1; i <= _numCmdList; ++i, ++cmdList) {
		if (cmdList->match(_selCmd.action, _curCmd.subject1, _curCmd.subject2)) {
			matchingCmds[comMax] = i;
			++comMax;
		}
	}

	if (comMax == 0) {
		// no command match was found, so exit
		// pass ACTION2 as parameter, as a new Command (and a new ACTION2) 
		// can be constructed while Joe speaks 
		executeStandardStuff(_selCmd.action, _curCmd.subject1, _curCmd.subject2);
		cleanupCurrentAction();
		return;
	}

	// process each associated command for the Object, until all done
	// or one of the Gamestate tests fails...
	int16 cond = 0;
	CmdListData *com = &_cmdList[0];
	uint16 comId = 0;
	for (i = 1; i <= comMax; ++i) {

		comId = matchingCmds[i - 1];
		com = &_cmdList[comId];

		// check the Gamestates and set them if necessary
		cond = 0;
		if (com->setConditions) {
			cond = setConditions(comId, (i == comMax));
		}

		if (cond == -1 && i == comMax) {
			// only exit on a condition fail if at last command
			// Joe hasnt spoken, so do normal LOOK command
			if (_selCmd.action.value() == VERB_LOOK_AT) {
				// Look At, do standard look at routine
				look();
				cleanupCurrentAction();
				return;
			}
		}
		else if (cond == -2 && i == comMax) {
			// only exit on a condition fail if at last command
			// Joe has spoken, so skip LOOK command
			cleanupCurrentAction();
			return;
		}
		else if (cond >= 0) {
			// we've had a successful Gamestate check, so we must now exit
			break;
		}
	}

	debug(0, "Command::executeCurrentAction() - cond = %X, com = %X", cond, comId);

	if (com->setAreas) {
		setAreas(comId);
	}

	// Don't grab if action is TALK or WALK
	if (_selCmd.action.value() != VERB_TALK_TO && _selCmd.action.value() != VERB_WALK_TO) {
		if (_curCmd.subject1 > 0) {
			_logic->joeGrab(_logic->objectData(_curCmd.subject1)->state, 0);
		}
		if (_curCmd.subject2 > 0) {
			_logic->joeGrab(_logic->objectData(_curCmd.subject2)->state, 0);
		}
	}

	bool cutDone = false;
	if (cond > 0) {
		// FIXME: this stuff must be seriously re-designed !

		// CR 2 - 7/3/95, Because we may be calling a cutaway triggered from walking
		// to the pinnacle (which connects to Map room 7), we'll be caught in the
		// R_MAP routine until we select a location and then call this very procedure
		// again - the result being that COM will be reset. So to avoid that, we'll
		// keep a copy of COM until we return from the recursive call...
		// Otherwise, all remaining commands will be wiped and not carried out!
		uint16 comTempId = comId;
		CmdListData *comTemp = com;

		const char *desc = _logic->objectTextualDescription(cond);
		if (executeIfCutaway(desc)) {
			cond = 0;
			cutDone = true;
		}

		comId = comTempId;
		com = comTemp;

		// check for dialogs before updating Objects
		if (executeIfDialog(desc)) {
			cond = 0;
		}
	}

	int16 oldImage = 0;
	if (_curCmd.subject1 > 0) {
		// an object (not an item)
		oldImage = _logic->objectData(_curCmd.subject1)->image;
	}

	if (com->setObjects) {
		setObjects(comId);
	}
	if (com->setItems) {
		setItems(comId);
	}

	if (com->imageOrder != 0) {
		ObjectData* od = _logic->objectData(_curCmd.subject1);
		// we must update the graphic image of the object
		if (com->imageOrder < 0) {
			// instead of setting to -1 or -2, flag as negative
			if (od->image > 0) {
				// make sure that object is not already updated
				od->image = -(od->image + 10);
			}
		}
		else {
			od->image = com->imageOrder;
		}
		_logic->roomRefreshObject(_curCmd.subject1);
	}
	else {
		// this object is not being updated by command list, see if
		// it has another image copied to it
		if (_curCmd.subject1 > 0) {
			// an object (not an item)
			if (_logic->objectData(_curCmd.subject1)->image != oldImage) {
				_logic->roomRefreshObject(_curCmd.subject1);
			}
		}
	}

	// don't play music on an OPEN/CLOSE command - in case the command fails
	if (_selCmd.action.value() != VERB_OPEN && _selCmd.action.value() != VERB_CLOSE) {
		// only play song if it's a PLAY BEFORE type
		if (com->song > 0) {
			_sound->playSong(com->song);
		}
	}

	// do a special hardcoded section
	// l.419-452 execute.c
	switch (com->specialSection) {
	case 1:
		_logic->useJournal();
		return;
	case 2:
		_logic->joeUseDress(true);
		break;
	case 3:
		_logic->joeUseClothes(true);
		break;
	case 4:
		_logic->joeUseUnderwear();
		break;
	}

	changeObjectState(_selCmd.action, _curCmd.subject1, com->song, cutDone);

	if (_selCmd.action.value() == VERB_TALK_TO && cond > 0) {
		if (executeIfDialog(_logic->objectTextualDescription(cond))) {
			cleanupCurrentAction();
			return;
		}
	}

// EXECUTE_EXIT1:

	if (cond > 0) {
		const char *desc = _logic->objectTextualDescription(cond);
		// Joe needs to say something as a result of a Gamestate
		// check first to see if it is a cutaway scene!
		if (executeIfCutaway(desc)) {
		}
		else if (executeIfDialog(desc)) {
			cleanupCurrentAction();
			return;
		}
		else {
			_logic->joeSpeak(cond, true);
		}
	}
	else {
		// we've failed commands with nothing to say
		if (_selCmd.action.value() == VERB_LOOK_AT) {
			// Look At, do standard look at routine
			look();
			cleanupCurrentAction();
			return;
		}
	}

	// only play song if it's a PLAY AFTER type
	if (com->song < 0) {
		_sound->playSong(com->song);
	}

	clear(true);
	cleanupCurrentAction();
}


void Command::updatePlayer() {

	if (_input->cutawayRunning()) return;

	lookCurrentRoom();
	lookCurrentIcon();

	if (!_input->keyVerb().isNone()) {

		if (_input->keyVerb().isJournal()) {
			_logic->useJournal();
		}
		else if (!_input->keyVerb().isSkipText()) {
			_curCmd.verb = _input->keyVerb();
			if (_curCmd.verb.isInventory()) {
				_curCmd.noun = _selCmd.noun = 0;
				// Clear old noun and old verb in case we're pointing at an
				// object (noun) or item (verb) and we want to use an item
				// on it. This was the command will be redisplayed with the
				// object/item that the cursor is currently on.
				_curCmd.oldNoun = 0;
				_curCmd.oldVerb = Verb(VERB_NONE);
				grabSelectedItem();
			}
			else {
				grabSelectedVerb();
			}
			_input->clearKeyVerb();
		}
	}

	_mouseKey = _input->mouseButton();
	_input->clearMouseButton();
	if (_mouseKey > 0) {
		grabCurrentSelection();
	}
}


void Command::readCommandsFrom(byte *&ptr) {

	uint16 i;

	// Command List Data
	_numCmdList = READ_BE_UINT16(ptr); ptr += 2;

	_cmdList = new CmdListData[_numCmdList + 1];
	memset(&_cmdList[0], 0, sizeof(CmdListData));
	for (i = 1; i <= _numCmdList; i++) {
		_cmdList[i].readFrom(ptr);
	}
	
	// Command AREA
	_numCmdArea = READ_BE_UINT16(ptr); ptr += 2;

	_cmdArea = new CmdArea[_numCmdArea + 1];
	memset(&_cmdArea[0], 0, sizeof(CmdArea));
	for (i = 1; i <= _numCmdArea; i++) {
		_cmdArea[i].readFrom(ptr);
	}

	// Command OBJECT
	_numCmdObject = READ_BE_UINT16(ptr); ptr += 2;

	_cmdObject = new CmdObject[_numCmdObject + 1];
	memset(&_cmdObject[0], 0, sizeof(CmdObject));
	for (i = 1; i <= _numCmdObject; i++) {
		_cmdObject[i].readFrom(ptr);
	}

	// Command INVENTORY
	_numCmdInventory = READ_BE_UINT16(ptr);	ptr += 2;

	_cmdInventory = new CmdInventory[_numCmdInventory + 1];
	memset(&_cmdInventory[0], 0, sizeof(CmdInventory));
	for (i = 1; i <= _numCmdInventory; i++) {
		_cmdInventory[i].readFrom(ptr);
	}

	// Command GAMESTATE
	_numCmdGameState = READ_BE_UINT16(ptr);	ptr += 2;
	_cmdGameState = new CmdGameState[_numCmdGameState + 1];
	memset(&_cmdGameState[0], 0, sizeof(CmdGameState));
	for (i = 1; i <= _numCmdGameState; i++) {
		_cmdGameState[i].readFrom(ptr);
	}
}


int16 Command::makeJoeWalkTo(int16 x, int16 y, int16 objNum, const Verb &v, bool mustWalk) {

	// Check to see if object is actually an exit to another
	// room. If so, then set up new room
	ObjectData *objData = _logic->objectData(objNum);
	if (objData->x != 0 || objData->y != 0) {
		x = objData->x;
		y = objData->y;
	}

	if (v.value() == VERB_WALK_TO) {
		_logic->entryObj(objData->entryObj);
		if (_logic->entryObj() != 0) {
			_logic->newRoom(_logic->objectData(_logic->entryObj())->room);
			// because this is an exit object, see if there is
			// a walk off point and set (x,y) accordingly
			WalkOffData *wod = _logic->walkOffPointForObject(objNum);
			if (wod != NULL) {
				x = wod->x;
				y = wod->y;
			}
		}
	}
	else {
		_logic->entryObj(0);
		_logic->newRoom(0);
	}

	debug(0, "Command::makeJoeWalkTo() - x=%d y=%d newRoom=%d", x, y, _logic->newRoom());

	int16 p = 0;
	if (mustWalk) {
		// determine which way for Joe to face Object
		uint16 facing = State::findDirection(objData->state);

		BobSlot *bobJoe  = _graphics->bob(0);
		if (x == bobJoe->x && y == bobJoe->y) {
			_logic->joeFacing(facing);
			_logic->joeFace();
		}
		else {
			p = _walk->joeMove(facing, x, y, false); // XXX inCutaway parameter
			// XXX if(P != 0) P = FIND_VERB
		}
	}
	return p;
}


void Command::grabCurrentSelection() {
	
	_selPosX = _input->mousePosX();
	_selPosY = _input->mousePosY();

	uint16 zone = _logic->findObjectUnderCursor(_selPosX, _selPosY);
	_curCmd.noun = _logic->findObjectRoomNumber(zone);
	_curCmd.verb = _logic->findVerbUnderCursor(_selPosX, _selPosY);

	_selPosX += _logic->display()->horizontalScroll();

	debug(0, "Command::grabCurrentSelection() - _curCmd.noun = %d, _curCmd.verb = %d, objMax=%d", _curCmd.noun, _curCmd.verb.value(), _logic->currentRoomObjMax());
	if (_curCmd.verb.isAction()) {
		grabSelectedVerb();
	}
	else if (_curCmd.verb.isInventory()) {
		grabSelectedItem();
	}
	else if (_curCmd.noun > 0 && _curCmd.noun <= _logic->currentRoomObjMax()) {
		grabSelectedNoun();
	}
	else if (_selPosY < ROOM_ZONE_HEIGHT && _curCmd.verb.isNone()) {
		// select without a command, do a WALK
		_logic->newRoom(0); // cancel makeJoeWalkTo, that should be equivalent to cr10 fix
		clear(true);
		_logic->joeWalk(JWM_EXECUTE);
	}
}


void Command::grabSelectedObject(int16 objNum, uint16 objState, uint16 objName) {

	if (!_curCmd.action.isNone()) {
		_cmdText.addObject(_logic->objectName(objName));
	}
	
	_curCmd.addObject(objNum);

	// if first noun and it's a 2 level command then set up action word
	if (_curCmd.action.value() == VERB_USE && _curCmd.commandLevel == 1) {
		if (State::findUse(objState) == STATE_USE_ON) {
			// object supports 2 levels
			_curCmd.commandLevel = 2;
			_cmdText.addLinkWord(Verb(VERB_PREP_WITH));
			 // command not fully constructed
			_cmdText.display(INK_CMD_NORMAL);
			_parse = false;
		}
		else {
			_cmdText.display(INK_CMD_SELECT);
			_parse = true;
		}
	}
	else if (_curCmd.action.value() == VERB_GIVE && _curCmd.commandLevel == 1) {
		_curCmd.commandLevel = 2;
		_cmdText.addLinkWord(Verb(VERB_PREP_TO));
		 // command not fully constructed
		_cmdText.display(INK_CMD_NORMAL);
		_parse = false;
	}
	else {
		_cmdText.display(INK_CMD_SELECT);
		_parse = true;
	}

	if (_parse) {
		_curCmd.verb = Verb(VERB_NONE);
		_logic->joeWalk(JWM_EXECUTE);
		_selCmd.action = _curCmd.action;
		_curCmd.action = Verb(VERB_NONE);
	}
}


void Command::grabSelectedItem() {

	// if the NOUN has been selected from screen then it is positive
	// Otherwise it has been selected from inventory and is negative
	// Set PARSE to TRUE, default FALSE if command half complete

	_parse = true;
	uint16 item = _logic->findInventoryItem(_curCmd.verb.inventoryItem());
	if (item == 0 || _logic->itemData(item)->name == 0) {
		return;
	}

	// If we've selected via keyboard, and there is no VERB then do
	// the ITEMs default, otherwise keep constructing!

	if (_mouseKey == Input::MOUSE_LBUTTON || 
		(!_input->keyVerb().isNone() && !_curCmd.verb.isNone())) {
		if (_curCmd.action.isNone()) {
			if (!_input->keyVerb().isNone()) {
				/* 2 - We've selected via the keyboard, no command is being */
				/*        constructed, so we shall find the item's default     */
				_curCmd.verb = findDefault(item, true);
				if (_curCmd.verb.isNone()) {
					// set to Look At
					_curCmd.verb = Verb(VERB_LOOK_AT);
					_cmdText.setVerb(Verb(VERB_LOOK_AT));
				}
				_curCmd.action = _curCmd.verb;
			}
			else {
				// Action>0 ONLY if command has been constructed 
				// Left Mouse Button pressed just do Look At     
				_curCmd.verb = Verb(VERB_LOOK_AT);
				_curCmd.action = Verb(VERB_LOOK_AT);
				_cmdText.setVerb(Verb(VERB_LOOK_AT));
			}
		}
		_curCmd.verb = Verb(VERB_NONE);
	}
	else {
		if (_logic->joeWalk() == JWM_MOVE) {
			_cmdText.clear();
			_curCmd.commandLevel = 1;
			_logic->joeWalk(JWM_NORMAL);
			_curCmd.action = Verb(VERB_NONE);
			lookCurrentIcon();
		}

		if (!_selCmd.defaultVerb.isNone()) {
			alterDefault(_selCmd.defaultVerb, true);
			_selCmd.defaultVerb = Verb(VERB_NONE);
			clear(true);
			return;
		}

		if (_cmdText.isEmpty()) {
			_curCmd.verb = Verb(VERB_LOOK_AT);
			_curCmd.action = Verb(VERB_LOOK_AT);
			_cmdText.setVerb(Verb(VERB_LOOK_AT));
		}
		else {
			if (_curCmd.commandLevel == 2 && _parse) {
				_curCmd.verb = _curCmd.action;
			}
			else {
				_curCmd.verb = findDefault(item, true);
			}
			if (_curCmd.verb.isNone()) {
				// No match made, so command not yet completed. Redefine as LOOK AT
				_curCmd.action = Verb(VERB_LOOK_AT);
				_cmdText.setVerb(Verb(VERB_LOOK_AT));
			}
			else {
				_curCmd.action = _curCmd.verb;
			}
			_curCmd.verb = Verb(VERB_NONE);
		}
	}

	grabSelectedObject(-item, _logic->itemData(item)->state, _logic->itemData(item)->name);
}


void Command::grabSelectedNoun() {

	// if the NOUN has been selected from screen then it is positive
	// otherwise it has been selected from inventory and is negative
	// set PARSE to TRUE, default FALSE if command half complete
	// click object without a command, if DEFAULT then
	// do that, otherwise do a WALK!

	uint16 objNum = _logic->currentRoomData() + _curCmd.noun;
	int16 objName = _logic->objectData(objNum)->name;
	if (objName < 0) {
		// selected a turned off object, so just walk
		_curCmd.noun = 0;
		clear(true);
		_logic->joeWalk(JWM_EXECUTE);
		return;
	}

	if (_curCmd.verb.isNone()) {
		if (_mouseKey == Input::MOUSE_LBUTTON) {
			if ((_curCmd.commandLevel != 2 && _curCmd.action.isNone()) || 
				(_curCmd.commandLevel == 2 && _parse)) {
					// action2 > 0 only if command has been constructed
					// lmb pressed, just walk
					_curCmd.verb = Verb(VERB_WALK_TO);
					_curCmd.action = Verb(VERB_WALK_TO);
					_cmdText.setVerb(Verb(VERB_WALK_TO));
			}
		}
		else if (_mouseKey == Input::MOUSE_RBUTTON) {

			// rmb pressed, do default if one exists
			if (!_selCmd.defaultVerb.isNone()) {
				// change default of command
				alterDefault(_selCmd.defaultVerb, false);
				_selCmd.defaultVerb = Verb(VERB_NONE);
				clear(true);
				return;
			}

			if (_cmdText.isEmpty()) {
				// Ensures that Right Mkey will select correct default
				_curCmd.verb = findDefault(objNum, false);
				if (!_curCmd.verb.isNone()) {
					// no match made, redefine as Walk To
					_selCmd.action = Verb(VERB_WALK_TO);
				}
				else {
					_selCmd.action = _curCmd.verb;
				}
				_cmdText.setVerb(_selCmd.action);
				_cmdText.addObject(_logic->objectName(_logic->objectData(objNum)->name));
			}
			else {
				_curCmd.verb = Verb(VERB_NONE);
				if ((_curCmd.commandLevel == 2 && !_parse) || !_curCmd.action.isNone()) {
					_curCmd.verb = _curCmd.action;
				}
				else {
					_curCmd.verb = findDefault(objNum, false);
				}
				if (_curCmd.verb.isNone()) {
					_curCmd.action = Verb(VERB_WALK_TO);
					_cmdText.setVerb(Verb(VERB_WALK_TO));
				}
				else {
					_curCmd.action = _curCmd.verb;
				}
				_curCmd.verb = Verb(VERB_NONE);
			}
		}
	}

	_selCmd.noun = 0;
	grabSelectedObject(objNum, _logic->objectData(objNum)->state, objName);
}


void Command::grabSelectedVerb() {

	if (_curCmd.verb.isScrollInventory()) {
		// move through inventory (by four if right mouse button)
		uint16 scroll = _mouseKey == Input::MOUSE_RBUTTON ? 4 : 1;
		_logic->inventoryScroll(scroll, _curCmd.verb.value() == VERB_SCROLL_UP);
	}
	else if (_curCmd.verb.isPanelCommand() || _curCmd.verb.value() == VERB_WALK_TO) {
		_curCmd.action = _curCmd.verb;
		_curCmd.subject1 = 0;
		_curCmd.subject2 = 0;

		// if right mouse key selected, then store command VERB
		if (_mouseKey == Input::MOUSE_RBUTTON) {
			_selCmd.defaultVerb = _curCmd.verb;
			_cmdText.displayTemp(11, true, _curCmd.verb);
		}
		else {
			_selCmd.defaultVerb = Verb(VERB_NONE);
			if (_logic->joeWalk() == JWM_MOVE && !_curCmd.verb.isNone()) {
				_logic->joeWalk(JWM_NORMAL);
			}
			_curCmd.commandLevel = 1;
			_curCmd.oldVerb = Verb(VERB_NONE);
			_curCmd.oldNoun = 0;
			_cmdText.setVerb(_curCmd.verb);
			_cmdText.display(INK_CMD_NORMAL);
		}
	}
}


bool Command::executeIfCutaway(const char *description) {

	if (strlen(description) > 4 && 
		scumm_stricmp(description + strlen(description) - 4, ".cut") == 0) {

		_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);

		char nextCutaway[20];
		memset(nextCutaway, 0, sizeof(nextCutaway));
		_logic->playCutaway(description, nextCutaway);
		while (nextCutaway[0] != '\0') {
			_logic->playCutaway(nextCutaway, nextCutaway);
		}
		return true;
	}
	return false;
}


bool Command::executeIfDialog(const char *description) {

	if (strlen(description) > 4 && 
			scumm_stricmp(description + strlen(description) - 4, ".dog") == 0) {
		char cutaway[20];

		_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);

		_logic->dialogue(description, _selCmd.noun, cutaway);

		while (cutaway[0] != '\0') {
			char currentCutaway[20];
			strcpy(currentCutaway, cutaway);
			_logic->playCutaway(currentCutaway, cutaway);
		}

		return true;
	}
	return false;
}


bool Command::handleBadCommand(bool walk) {

	// l.96-141 execute.c
	uint16 objMax = _logic->currentRoomObjMax();
	uint16 roomData = _logic->currentRoomData();

	// select without a command or WALK TO ; do a WALK
	if ((_selCmd.action.value() == VERB_WALK_TO || _selCmd.action.isNone()) && 
		(_selCmd.noun > objMax || _selCmd.noun == 0)) {
		if (_selCmd.action.isNone()) {
			_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
		}
		_walk->joeMove(0, _selPosX, _selPosY, false); // XXX inCutaway parameter
		return true;
	}
	// check to see if one of the objects is hidden
	if (_curCmd.subject1 > 0 && _logic->objectData(_curCmd.subject1)->name <= 0) {
		return true;
	}
	if (_selCmd.action.value() == VERB_GIVE && 
		_curCmd.subject2 > 0 && _logic->objectData(_curCmd.subject2)->name <= 0) {
		return true;
	}
	// check for USE command on exists
	if (_selCmd.action.value() == VERB_USE && 
		_curCmd.subject1 > 0 && _logic->objectData(_curCmd.subject1)->entryObj > 0) {
		_selCmd.action = Verb(VERB_WALK_TO);
	}
	if (_selCmd.noun > 0 && _selCmd.noun <= objMax) {
		uint16 objNum = _logic->currentRoomData() + _selCmd.noun;
		if (makeJoeWalkTo(_selPosX, _selPosY, objNum, _selCmd.action, walk) != 0) {
			return true;
		}
		if (_selCmd.action.value() == VERB_WALK_TO && _logic->objectData(roomData + _selCmd.noun)->entryObj < 0) {
			return true;
		}
	}
	return false;
}


void Command::executeStandardStuff(const Verb& action, int16 subj1, int16 subj2) {

	// l.158-272 execute.c
	uint16 k;

	switch (action.value()) {

	case VERB_LOOK_AT:
		look();
		break;

	case VERB_OPEN:
		// 'it doesn't seem to open'
		_logic->joeSpeak(1);
		break;

	case VERB_USE:
		if (subj1 < 0) {
			k = _logic->itemData(-subj1)->sfxDescription;
			if (k > 0) {
				_logic->joeSpeak(k, true);
			}
			else {
				_logic->joeSpeak(2);
			}
		}
		else {
			_logic->joeSpeak(1);
		}
		break;

	case VERB_TALK_TO:
		_logic->joeSpeak(24 + Logic::randomizer.getRandomNumber(2));
		break;

	case VERB_CLOSE:
		_logic->joeSpeak(2);
		break;

	case 4: // weird, isn't it ? l.193 execute.c
		warning("Command::executeStandardStuff() - Use of verb 4");
	case VERB_MOVE:
		// 'I can't move it'
		if (subj1 > 0) {
			int16 img = _logic->objectData(subj1)->image;
			if (img == -4 || img == -3) {
				_logic->joeSpeak(18);
			}
			else {
				_logic->joeSpeak(3);
			}
		}
		else {
			_logic->joeSpeak(3);
		}
		break;

	case VERB_GIVE:
		// 'I can't give the subj1 to subj2'
		if (subj1 < 0) {
			k = 11;
			if (subj2 > 0) {
				int16 img = _logic->objectData(subj2)->image;
				if (img == -4 || img == -3) {
					_logic->joeSpeak(27 + Logic::randomizer.getRandomNumber(2));
				}
			}
			else {
				_logic->joeSpeak(11);
			}
		}
		else {
			_logic->joeSpeak(12);
		}
		break;

	case VERB_PICK_UP:
		if (subj1 < 0) {
			_logic->joeSpeak(14);
		}
		else {
			int16 img = _logic->objectData(subj2)->image;
			if (img == -4 || img == -3) {
				// Trying to get a person
				_logic->joeSpeak(20);
			}
			else {
				switch(Logic::randomizer.getRandomNumber(3)) {
				case 0:
					// 'I can't pick that up'
					_logic->joeSpeak(5);
					break;
				case 1:
					// 'I don't think I need that'
					_logic->joeSpeak(6);
					break;
				case 2:
					// 'I'd rather leave it here'
					_logic->joeSpeak(7);
					break;
				case 3:
					// 'I don't think I'd have any use for that'
					_logic->joeSpeak(8);
					break;
				}
			}
		}
		break;

	default:
		break;
	}
}


void Command::changeObjectState(const Verb& action, int16 obj, int16 song, bool cutDone) {

	// l.456-533 execute.c
	ObjectData *objData = _logic->objectData(obj);

	if (action.value() == VERB_OPEN && !cutDone) {
		if (State::findOn(objData->state) == STATE_ON_ON) {
			State::alterOn(&objData->state, STATE_ON_OFF);
			State::alterDefaultVerb(&objData->state, Verb(VERB_NONE));

			// play music if it exists... (or SFX for open/close door)
			if (song != 0) {
				_sound->playSong(ABS(song));
			}

			if (objData->entryObj != 0) {
				// if it's a door, then update door that it links to
				openOrCloseAssociatedObject(action, objData->entryObj);
				objData->entryObj = ABS(objData->entryObj);
			}
		}
		else {
			// 'it's already open !'
			_logic->joeSpeak(9);
		}
	}
	else if (action.value() == VERB_CLOSE && !cutDone) {

		if (State::findOn(objData->state) == STATE_ON_OFF) {
			State::alterOn(&objData->state, STATE_ON_ON);
			State::alterDefaultVerb(&objData->state, Verb(VERB_OPEN));

			// play music if it exists... (or SFX for open/close door)
			if (song != 0) {
				_sound->playSong(ABS(song));
			}

			if (objData->entryObj != 0) {
				// if it's a door, then update door that it links to
				openOrCloseAssociatedObject(action, objData->entryObj);
				objData->entryObj = -ABS(objData->entryObj);
			}
		}
		else {
			// 'it's already closed !'
			_logic->joeSpeak(10);
		}
	}
	else if (action.value() == VERB_MOVE) {
		State::alterOn(&objData->state, STATE_ON_OFF);
	}
}

void Command::cleanupCurrentAction() {

	// l.595-597 execute.c
	_logic->joeFace();
	_curCmd.oldNoun = 0;
	_curCmd.oldVerb = Verb(VERB_NONE);
}


Verb Command::findDefault(uint16 obj, bool itemType) {

	uint16 s = itemType ? _logic->itemData(obj)->state : _logic->objectData(obj)->state;
	return State::findDefaultVerb(s);
}


void Command::alterDefault(const Verb& def, bool itemType) {

	uint16 *newDefaultState = 0;
	const char *name = NULL;

	_curCmd.noun = _logic->findObjectUnderCursor(_selPosX, _selPosY);
	if (!itemType) {
		if (_curCmd.noun == 0) {
			return;
		}
		uint16 i = _logic->findObjectGlobalNumber(_curCmd.noun);
		ObjectData *od = _logic->objectData(i);
		if (od->name < 0) {
			return;
		}
		newDefaultState = &od->state;
		name = _logic->objectTextualDescription(od->name);
	}
	else {
		uint16 item = _logic->findInventoryItem(_curCmd.verb.inventoryItem());
		if (item == 0 || _logic->itemData(item)->name == 0) {
			return;
		}
		ItemData *id = _logic->itemData(item);
		newDefaultState = &id->state;
		name = _logic->objectTextualDescription(id->name);
	}

	State::alterDefaultVerb(newDefaultState, def);
	if (_curCmd.noun == 0) {
		_cmdText.clear();
	}
	else {
		_cmdText.setVerb(def.isNone() ? Verb(VERB_WALK_TO) : def);
	}
	_cmdText.displayTemp(INK_CMD_NORMAL, name);
	_curCmd.oldNoun = _curCmd.noun;
}


void Command::openOrCloseAssociatedObject(const Verb& action, int16 otherObj) {

	CmdListData *cmdList = &_cmdList[1];
	uint16 com = 0;
	uint16 i;
	for (i = 1; i <= _numCmdList && com != 0; ++i, ++cmdList) {
		if (cmdList->match(action, otherObj, 0)) {
			if (cmdList->setConditions) {
				CmdGameState *cmdGs = _cmdGameState;
				uint16 j;
				for (j = 1; j <= _numCmdGameState; ++j) {
					if (cmdGs[j].id == i && cmdGs[i].gameStateSlot > 0) {
						// FIXME: weird, why using 'i' instead of 'j' ?
						if (_logic->gameState(cmdGs[i].gameStateSlot) == cmdGs[i].gameStateValue) {
							com = i;
							break;
						}
					}
				}
			}
			else {
				com = i;
				break;
			}
		}
	}

	if (com != 0) {

		cmdList = &_cmdList[com];
		ObjectData *objData = _logic->objectData(otherObj);

		if (cmdList->imageOrder != 0) {
			// update the graphic image of object
			objData->image = cmdList->imageOrder;
		}

		if (action.value() == VERB_OPEN) {
			if (State::findOn(objData->state) == STATE_ON_ON) {
				State::alterOn(&objData->state, STATE_ON_OFF);
				State::alterDefaultVerb(&objData->state, Verb(VERB_NONE));
				objData->entryObj = ABS(objData->entryObj);
			}
		}
		else if (action.value() == VERB_CLOSE) {
			if (State::findOn(objData->state) == STATE_ON_OFF) {
				State::alterOn(&objData->state, STATE_ON_ON);
				State::alterDefaultVerb(&objData->state, Verb(VERB_OPEN));
				objData->entryObj = -ABS(objData->entryObj);
			}
		}
	}
}


int16 Command::setConditions(uint16 command, bool lastCmd) {

	debug(9, "Command::setConditions(%d, %d)", command, lastCmd);
	// Test conditions, if FAIL write &&  exit, Return -1
	// if(Joe speaks before he returns, -2 is returned
	// This way a -1 return will allow Joe to speak normal description

	uint16 temp[21];
	memset(temp, 0, sizeof(temp));
	uint16 tempInd = 0;

	int16 ret = 0;
	uint16 i;
	CmdGameState *cmdGs = &_cmdGameState[1];
	for (i = 1; i <= _numCmdGameState; ++i, ++cmdGs) {
		if (cmdGs->id == command) {
			if (cmdGs->gameStateSlot > 0) {
				if (_logic->gameState(cmdGs->gameStateSlot) != cmdGs->gameStateValue) {
					debug(0, "Command::setConditions() - Failed test %X", ret);
					debug(0, "Command::setConditions() - GS[%d] == %d (should be %d)", cmdGs->gameStateSlot, _logic->gameState(cmdGs->gameStateSlot), cmdGs->gameStateValue);
					// failed test
					ret = i;
					break;
				}
			}
			else {
				temp[tempInd] = i;
				++tempInd;
			}
		}
	}

	if (ret > 0) {
		// we've failed, so see if we need to make Joe speak
		cmdGs = &_cmdGameState[ret];
		if (cmdGs->speakValue > 0 && lastCmd) {
			// check to see if fail state is in fact a cutaway
			const char *objDesc = _logic->objectTextualDescription(cmdGs->speakValue);
			if (!executeIfCutaway(objDesc) && !executeIfDialog(objDesc)) {
				_logic->joeSpeak(cmdGs->speakValue, true);
			}
			ret = -2;
		}
		else {
			ret = -1;
		}
	}
	else {
		ret = 0;
		// all tests were okay, now set gamestates
		for (i = 0; i < tempInd; ++i) {
			cmdGs = &_cmdGameState[temp[i]];
			_logic->gameState(ABS(cmdGs->gameStateSlot), cmdGs->gameStateValue);
			// set return value for Joe to say something
			ret = cmdGs->speakValue;
		}
	}
	return ret;
}


void Command::setAreas(uint16 command) {

	debug(9, "Command::setAreas(%d)", command);

	CmdArea *cmdArea = &_cmdArea[1];
	uint16 i;
	for (i = 1; i <= _numCmdArea; ++i, ++cmdArea) {
		if (cmdArea->id == command) {
			uint16 areaNum = ABS(cmdArea->area);
			Area *area = _logic->area(cmdArea->room, areaNum);
			if (cmdArea->area > 0) {
				// turn on area
				area->mapNeighbours = ABS(area->mapNeighbours);
			}
			else {
				// turn off area
				area->mapNeighbours = -ABS(area->mapNeighbours);
			}
		}
	}
}


void Command::setObjects(uint16 command) {

	debug(9, "Command::setObjects(%d)", command);

	CmdObject *cmdObj = &_cmdObject[1];
	uint16 i;
	for (i = 1; i <= _numCmdObject; ++i, ++cmdObj) {
		if (cmdObj->id == command) {

			// found an object
			uint16 dstObj = ABS(cmdObj->dstObj);
			ObjectData *objData = _logic->objectData(dstObj);

			if (cmdObj->dstObj > 0) {
				// show the object
				objData->name = ABS(objData->name);
				// test that the object has not already been deleted
				// by checking if it is not equal to zero
				if (cmdObj->srcObj == -1 && objData->name != 0) {
					// delete object by setting its name to 0 and
					// turning off graphic image
					objData->name = 0;
					if (objData->room == _logic->currentRoom()) {
						if (dstObj != _curCmd.subject1) {
							// if the new object we have updated is on screen and is not the 
							// current object, then we can update. This is because we turn
							// current object off ourselves by COM_LIST(com, 8)
							if (objData->image != -3 && objData->image != -4) {
								// it is a normal object (not a person)
								// turn the graphic image off for the object
								objData->image = -(objData->image + 10);
							}
						}
						// invalidate object area
						uint16 objZone = dstObj - _logic->currentRoomData();
						_logic->zoneSet(ZONE_ROOM, objZone, 0, 0, 1, 1);
					}
				}

				if (cmdObj->srcObj > 0) {
					// copy data from dummy object to object
					int16 image1 = objData->image;
					int16 image2 = _logic->objectData(cmdObj->srcObj)->image;
					_logic->objectCopy(cmdObj->srcObj, dstObj);
					if (image1 != 0 && image2 == 0 && objData->room == _logic->currentRoom()) {
						uint16 bobNum = _logic->findBob(dstObj);
						if (bobNum != 0) {
							_graphics->bobClear(bobNum);
						}
					}
				}

				if (dstObj != _curCmd.subject1) {
					// if the new object we have updated is on screen and
					// is not current object then update it
					_logic->roomRefreshObject(dstObj);
				}
			}
			else {
				// hide the object
				if (objData->name > 0) {
					objData->name = -objData->name;
					// may need to turn BOBs off for objects to be hidden on current 
					// screen ! if the new object we have updated is on screen and
					// is not current object then update it
					_logic->roomRefreshObject(dstObj);
				}
			}
		}
	}
}


void Command::setItems(uint16 command) {

	debug(9, "Command::setItems(%d)", command);

	CmdInventory *cmdInv = &_cmdInventory[1];
	ItemData *items = _logic->itemData(0);
	uint16 i;
	for (i = 1; i <= _numCmdInventory; ++i, ++cmdInv) {
		if (cmdInv->id == command) {
			uint16 dstItem = ABS(cmdInv->dstItem);
			// found an item
			if (cmdInv->dstItem > 0) {
				// add item to inventory
				if (cmdInv->srcItem > 0) {
					// copy data from source item to item
					items[dstItem] = items[cmdInv->srcItem];
					// enable it
					items[dstItem].name = ABS(items[dstItem].name);
				}
				_logic->inventoryInsertItem(cmdInv->dstItem);
			}
			else {
				// delete item
				if (items[dstItem].name > 0) {
					_logic->inventoryDeleteItem(dstItem);
				}
				if (cmdInv->srcItem > 0) {
					// copy data from source item to item
					items[dstItem] = items[cmdInv->srcItem];
					// disable it
					items[dstItem].name = -ABS(items[dstItem].name);
				}
			}
		}
	}
}


uint16 Command::nextObjectDescription(ObjectDescription* objDesc, uint16 firstDesc) {

	// l.69-103 select.c
	uint16 i;
	uint16 diff = objDesc->lastDescription - firstDesc;
	debug(0, "Command::updateNextDescription() - diff = %d, type = %d", diff, objDesc->type);
	switch (objDesc->type) {
	case 0:
		// random type, start with first description
		if (objDesc->lastSeenNumber == 0) {
			// first time look at called
			objDesc->lastSeenNumber = firstDesc;
		}
		else {
			// already displayed first, do a random
			i = objDesc->lastSeenNumber;
			while (i == objDesc->lastSeenNumber) {
				i = firstDesc + Logic::randomizer.getRandomNumber(diff);
			}
			objDesc->lastSeenNumber = i;
		}
		break;
	case 1:
		i = objDesc->lastSeenNumber;
		while (i == objDesc->lastSeenNumber) {
			i = firstDesc + Logic::randomizer.getRandomNumber(diff);
		}
		objDesc->lastSeenNumber = i;
		break;
	case 2:
		// sequential, but loop
		++objDesc->lastSeenNumber;
		if (objDesc->lastSeenNumber > objDesc->lastDescription) {
			objDesc->lastSeenNumber = firstDesc;
		}
		break;
	case 3:
		// sequential without looping
		if (objDesc->lastSeenNumber != objDesc->lastDescription) {
			++objDesc->lastSeenNumber;
		}
		break;
	}
	return objDesc->lastSeenNumber;
}


void Command::look() {

	if (_selCmd.noun > 0 && _selCmd.noun <= _logic->currentRoomObjMax()) {
		uint16 objNum = _logic->currentRoomData() + _selCmd.noun;
		if (_logic->objectData(objNum)->entryObj == 0) {
			if (makeJoeWalkTo(_selPosX, _selPosY, objNum, _selCmd.action, false) == -2) { // XXX inCutaway parameter
				// 'I can't get close enough to have a look.'
				_logic->joeSpeak(13);
			}
		}
	}

	// if object type and disabled, don't look
	if (_curCmd.subject1 > 0 && _logic->objectData(_curCmd.subject1)->name <= 0) {
		return;
	}

	uint16 desc;
	if (_curCmd.subject1 < 0) {
		desc = _logic->itemData(-_curCmd.subject1)->description;
	}
	else {
		desc = _logic->objectData(_curCmd.subject1)->description;
	}

	debug(0, "Command::look() - desc = %X, _curCmd.subject1 = %X", desc, _curCmd.subject1);

	// check to see if the object/item has a series of description
	ObjectDescription *objDesc = _logic->objectDescription(1);
	uint16 i;
	for (i = 1; i <= _logic->objectDescriptionCount(); ++i, ++objDesc) {
		if (objDesc->object == _curCmd.subject1) {
			desc = nextObjectDescription(objDesc, desc);
			break;
		}
	}

	_logic->joeSpeak(desc, true);
	_logic->joeFace();
}


void Command::lookCurrentItem() {

	if (_curCmd.verb.isInventory()) {
		uint16 item = _logic->findInventoryItem(_curCmd.verb.inventoryItem());
		if (item != 0) {
			ItemData *itemData = _logic->itemData(item);
			const char *name = _logic->objectName(itemData->name);		
			if (_curCmd.action.isNone()) {
				Verb v = State::findDefaultVerb(itemData->state);
				_cmdText.setVerb(v.isNone() ? Verb(VERB_LOOK_AT) : v);
			}
	
			if (!_selCmd.defaultVerb.isNone()) {
				_cmdText.displayTemp(INK_CMD_LOCK, true, _selCmd.defaultVerb, name);
			}
			else {
				_cmdText.displayTemp(INK_CMD_NORMAL, name);
			}
			_curCmd.oldVerb = _curCmd.verb;
		}
	}
}


void Command::lookCurrentRoom() {

	_curCmd.noun = _logic->findObjectUnderCursor(_input->mousePosX(), _input->mousePosY());

	if (_logic->joeWalk() == JWM_MOVE) {
		return;
	}

	int16 aObjName = 0;
	uint16 k = _logic->currentRoomData();
	int16 i = 0;
	if (_curCmd.noun > _logic->currentRoomObjMax()) {
		uint16 obj = _logic->currentRoomArea(_curCmd.noun - _logic->currentRoomObjMax())->object;
		if (obj) {
			aObjName = _logic->objectData(obj)->name;
			if (aObjName > 0) {
				i = aObjName;
				_curCmd.noun = obj - k;
			}
		}
	}
	else {
		i = _logic->objectData(k + _curCmd.noun)->name;
	}

	if (_curCmd.oldNoun == _curCmd.noun) {
		return;
	}

	// if pointing at an Area then exit
	// if the AREA is linked to an object, then dont exit. Find
	// the object its linked to &&  store in AOBJ

	if (_curCmd.noun > _logic->currentRoomObjMax() && aObjName <= 0) {
		if (_curCmd.oldNoun != 0) {
			if (!_selCmd.defaultVerb.isNone()) {
				_cmdText.displayTemp(INK_CMD_LOCK, true, _selCmd.defaultVerb);
			}
			else if (!_curCmd.action.isNone()) {
				_cmdText.display(INK_CMD_NORMAL);
			}
			_curCmd.oldNoun = 0;
			return;
		}
	}

	if (i <= 0) {
		_curCmd.oldNoun = _curCmd.noun;
		_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
		if (!_selCmd.defaultVerb.isNone()) {
			_cmdText.displayTemp(INK_CMD_LOCK, true, _selCmd.defaultVerb);
		}
		else if (!_curCmd.action.isNone()) {
			_cmdText.display(INK_CMD_NORMAL);
		}
		return;
	}

	// if no command yet selected, then use DEFAULT command, if any
	if (_curCmd.action.isNone()) {
		Verb v = State::findDefaultVerb(_logic->objectData(k + _curCmd.noun)->state);
		_cmdText.setVerb(v.isNone() ? Verb(VERB_WALK_TO) : v);
		if (_curCmd.noun == 0) {
			_cmdText.clear();
		}
	}
	const char *objName = "";
	if (_curCmd.noun > 0) {
		objName = _logic->objectName(i);
	}
	if (!_selCmd.defaultVerb.isNone()) {
		_cmdText.displayTemp(INK_CMD_LOCK, true, _selCmd.defaultVerb, objName);
	}
	else {
		_cmdText.displayTemp(INK_CMD_NORMAL, objName);
	}
	_curCmd.oldNoun = _curCmd.noun;
}


void Command::lookCurrentIcon() {

	_curCmd.verb = _logic->findVerbUnderCursor(_input->mousePosX(), _input->mousePosY());
	if (_curCmd.verb != _curCmd.oldVerb && _logic->joeWalk() != JWM_MOVE) {

		if (_curCmd.action.isNone()) {
			_cmdText.clear();
		}
		_graphics->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
		lookCurrentItem();

		// ensure that registers when move to top screen
		if (_curCmd.noun > 0) {
			_curCmd.oldNoun = -1;
		}

		_curCmd.oldVerb = _curCmd.verb;
		if (_curCmd.verb.isPanelCommand() || _curCmd.verb.value() == VERB_WALK_TO) {
			if (_curCmd.verb.isNone()) {
				_cmdText.display(INK_CMD_NORMAL);
			}
			else {
				_cmdText.displayTemp(INK_CMD_NORMAL, false, _curCmd.verb);
			}
		}
	}
}

}
