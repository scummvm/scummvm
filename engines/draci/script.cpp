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

#include "common/debug.h"
#include "common/stream.h"
#include "common/stack.h"
#include "common/queue.h"

#include "draci/draci.h"
#include "draci/script.h"
#include "draci/game.h"

namespace Draci {

// FIXME: Change parameter types to names once I figure out what they are exactly

void Script::setupCommandList() {
	/** A table of all the commands the game player uses */
	static const GPL2Command gplCommands[] = {
		{ 0,  0, "gplend",				0, { 0 }, NULL },
		{ 0,  1, "exit",				0, { 0 }, NULL },
		{ 1,  1, "goto", 				1, { 3 }, &Script::c_Goto },
		{ 2,  1, "Let", 				2, { 3, 4 }, &Script::c_Let },
		{ 3,  1, "if", 					2, { 4, 3 }, &Script::c_If },
		{ 4,  1, "Start", 				2, { 3, 2 }, &Script::start },
		{ 5,  1, "Load", 				2, { 3, 2 }, &Script::load },
		{ 5,  2, "StartPlay", 			2, { 3, 2 }, &Script::startPlay },
		{ 5,  3, "JustTalk", 			0, { 0 }, NULL },
		{ 5,  4, "JustStay", 			0, { 0 }, NULL },
		{ 6,  1, "Talk", 				2, { 3, 2 }, &Script::talk },
		{ 7,  1, "ObjStat", 			2, { 3, 3 }, &Script::objStat },
		{ 7,  2, "ObjStat_On", 			2, { 3, 3 }, &Script::objStatOn },
		{ 8,  1, "IcoStat", 			2, { 3, 3 }, NULL },
		{ 9,  1, "Dialogue", 			1, { 2 }, &Script::dialogue },
		{ 9,  2, "ExitDialogue", 		0, { 0 }, &Script::exitDialogue },
		{ 9,  3, "ResetDialogue", 		0, { 0 }, &Script::resetDialogue },
		{ 9,  4, "ResetDialogueFrom", 	0, { 0 }, &Script::resetDialogueFrom },
		{ 9,  5, "ResetBlock", 			1, { 3 }, &Script::resetBlock },
		{ 10, 1, "WalkOn", 				3, { 1, 1, 3 }, &Script::walkOn },
		{ 10, 2, "StayOn", 				3, { 1, 1, 3 }, &Script::walkOn }, // HACK: not a proper implementation
		{ 10, 3, "WalkOnPlay", 			3, { 1, 1, 3 }, &Script::walkOnPlay },
		{ 11, 1, "LoadPalette", 		1, { 2 }, NULL },
		{ 12, 1, "SetPalette", 			0, { 0 }, NULL },
		{ 12, 2, "BlackPalette", 		0, { 0 }, NULL },
		{ 13, 1, "FadePalette", 		3, { 1, 1, 1 }, NULL },
		{ 13, 2, "FadePalettePlay", 	3, { 1, 1, 1 }, NULL },
		{ 14, 1, "NewRoom", 			2, { 3, 1 }, &Script::newRoom },
		{ 15, 1, "ExecInit", 			1, { 3 }, &Script::execInit },
		{ 15, 2, "ExecLook", 			1, { 3 }, &Script::execLook },
		{ 15, 3, "ExecUse", 			1, { 3 }, &Script::execUse },
		{ 16, 1, "RepaintInventory", 	0, { 0 }, NULL },
		{ 16, 2, "ExitInventory", 		0, { 0 }, NULL },
		{ 17, 1, "ExitMap", 			0, { 0 }, NULL },
		{ 18, 1, "LoadMusic", 			1, { 2 }, NULL },
		{ 18, 2, "StartMusic", 			0, { 0 }, NULL },
		{ 18, 3, "StopMusic", 			0, { 0 }, NULL },
		{ 18, 4, "FadeOutMusic",		1, { 1 }, NULL },
		{ 18, 5, "FadeInMusic", 		1, { 1 }, NULL },
		{ 19, 1, "Mark", 				0, { 0 }, &Script::mark },
		{ 19, 2, "Release", 			0, { 0 }, &Script::release },
		{ 20, 1, "Play", 				0, { 0 }, &Script::play },
		{ 21, 1, "LoadMap", 			1, { 2 }, &Script::loadMap },
		{ 21, 2, "RoomMap", 			0, { 0 }, &Script::roomMap },
		{ 22, 1, "DisableQuickHero", 	0, { 0 }, NULL },
		{ 22, 2, "EnableQuickHero", 	0, { 0 }, NULL },
		{ 23, 1, "DisableSpeedText", 	0, { 0 }, NULL },
		{ 23, 2, "EnableSpeedText", 	0, { 0 }, NULL },
		{ 24, 1, "QuitGame", 			0, { 0 }, NULL },
		{ 25, 1, "PushNewRoom", 		0, { 0 }, NULL },
		{ 25, 2, "PopNewRoom", 			0, { 0 }, NULL },
		{ 26, 1, "ShowCheat", 			0, { 0 }, NULL },
		{ 26, 2, "HideCheat", 			0, { 0 }, NULL },
		{ 26, 3, "ClearCheat", 			1, { 1 }, NULL },
		{ 27, 1, "FeedPassword", 		3, { 1, 1, 1 }, NULL }
	};

	/** Operators used by the mathematical evaluator */
	static const GPL2Operator gplOperators[] = {
		{"&", 	&Script::operAnd 			},
		{"|",	&Script::operOr 			},
		{"^", 	&Script::operXor 			},
		{"==",	&Script::operEqual 			},
		{"!=", 	&Script::operNotEqual		},
		{"<", 	&Script::operLess			},
		{">", 	&Script::operGreater		},
		{"<=",	&Script::operLessOrEqual	},
		{">=",	&Script::operGreaterOrEqual	},
		{"*",	&Script::operMul			},
		{"/", 	&Script::operDiv			},
		{"%",	&Script::operMod			},
		{"+", 	&Script::operAdd			},
		{"-",	&Script::operSub			}
	};

	/** Functions used by the mathematical evaluator */
	static const GPL2Function gplFunctions[] = {
		{ "Not", 		&Script::funcNot },
		{ "Random", 	&Script::funcRandom },
		{ "IsIcoOn", 	&Script::funcIsIcoOn },
		{ "IsIcoAct", 	&Script::funcIsIcoAct },
		{ "IcoStat", 	&Script::funcIcoStat },
		{ "ActIco", 	&Script::funcActIco },
		{ "IsObjOn", 	&Script::funcIsObjOn },
		{ "IsObjOff", 	&Script::funcIsObjOff },
		{ "IsObjAway", 	&Script::funcIsObjAway },
		{ "ObjStat", 	&Script::funcObjStat },
		{ "LastBlock", 	&Script::funcLastBlock },
		{ "AtBegin", 	&Script::funcAtBegin },
		{ "BlockVar", 	&Script::funcBlockVar },
		{ "HasBeen", 	&Script::funcHasBeen },
		{ "MaxLine", 	&Script::funcMaxLine },
		{ "ActPhase", 	&Script::funcActPhase },
		{ "Cheat",  	NULL },
	};

	_commandList = gplCommands;
	_operatorList = gplOperators;
	_functionList = gplFunctions;
}

/** Type of mathematical object */
enum mathExpressionObject {
	kMathEnd,
	kMathNumber,
	kMathOperator,
	kMathFunctionCall,
	kMathVariable
};

/* GPL operators */

int Script::operAnd(int op1, int op2) {
	return op1 & op2;
}

int Script::operOr(int op1, int op2) {
	return op1 | op2;
}

int Script::operXor(int op1, int op2) {
	return op1 ^ op2;
}

int Script::operEqual(int op1, int op2) {
	return op1 == op2;
}

int Script::operNotEqual(int op1, int op2) {
	return op1 != op2;
}

int Script::operLess(int op1, int op2) {
	return op1 < op2;
}

int Script::operGreater(int op1, int op2) {
	return op1 > op2;
}

int Script::operGreaterOrEqual(int op1, int op2) {
	return op1 >= op2;
}

int Script::operLessOrEqual(int op1, int op2) {
	return op1 <= op2;
}

int Script::operMul(int op1, int op2) {
	return op1 * op2;
}

int Script::operAdd(int op1, int op2) {
	return op1 + op2;
}

int Script::operSub(int op1, int op2) {
	return op1 - op2;
}

int Script::operDiv(int op1, int op2) {
	return op1 / op2;
}

int Script::operMod(int op1, int op2) {
	return op1 % op2;
}

/* GPL functions */

int Script::funcRandom(int n) {

// The function needs to return numbers in the [0..n-1] range so we need to deduce 1
// (RandomSource::getRandomNumber returns a number in the range [0..n])

	n -= 1;
	return _vm->_rnd.getRandomNumber(n);
}

int Script::funcAtBegin(int yesno) {
	return _vm->_game->_dialogueBegin == yesno;
}	

int Script::funcLastBlock(int blockID) {

	return _vm->_game->_lastBlock == blockID;
}

int Script::funcBlockVar(int blockID) {
	return _vm->_game->_dialogueVars[_vm->_game->_dialogueOffsets[_vm->_game->_currentDialogue] + blockID];
}

int Script::funcHasBeen(int blockID) {
	return _vm->_game->_dialogueVars[_vm->_game->_dialogueOffsets[_vm->_game->_currentDialogue] + blockID] > 0;
}

int Script::funcMaxLine(int lines) {
	return _vm->_game->_dialogueLines < lines;
}

int Script::funcNot(int n) {
	return !n;
}

int Script::funcIsIcoOn(int iconID) {
	iconID -= 1;

	return _vm->_game->getIconStatus(iconID) == 1;
} 

int Script::funcIcoStat(int iconID) {
	iconID -= 1;

	int status = _vm->_game->getIconStatus(iconID);
	return (status == 1) ? 1 : 2;
}

int Script::funcIsIcoAct(int iconID) {
	iconID -= 1;

	return _vm->_game->getCurrentIcon() == iconID;
}

int Script::funcActIco(int iconID) {
	
	// The parameter seems to be an omission in the original player since it's not
	// used in the implementation of the function. It's possible that the functions were
	// implemented in such a way that they had to have a single parameter so this is only
	// passed as a dummy.

	return _vm->_game->getCurrentIcon();
}

int Script::funcIsObjOn(int objID) {
	objID -= 1;

	GameObject *obj = _vm->_game->getObject(objID);

	return obj->_visible;
}

int Script::funcIsObjOff(int objID) {
	objID -= 1;

	GameObject *obj = _vm->_game->getObject(objID);

	// We index locations from 0 (as opposed to the original player where it was from 1)
	// That's why the "away" location 0 from the data files is converted to -1
	return !obj->_visible && obj->_location != -1;
}

int Script::funcObjStat(int objID) {
	objID -= 1;

	GameObject *obj = _vm->_game->getObject(objID);

	if (obj->_location == _vm->_game->getRoomNum()) {
		if (obj->_visible) {
			return 1; 	// object is ON (in the room and visible)
		} else {
			return 2; 	// object is OFF (in the room, not visible)
		}
	} else {
		return 3; 		// object is AWAY (not in the room)
	}
}

int Script::funcIsObjAway(int objID) {
	objID -= 1;

	GameObject *obj = _vm->_game->getObject(objID);

	// see Script::funcIsObjOff
	return !obj->_visible && obj->_location == -1;
}

int Script::funcActPhase(int objID) {

	objID -= 1;	

	// Default return value
	int ret = 0;

	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return ret;
	}

	GameObject *obj = _vm->_game->getObject(objID);

	bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);

	if (objID == kDragonObject || visible) {
		int animID = obj->_anims[0];
		Animation *anim = _vm->_anims->getAnimation(animID);
		ret = anim->currentFrameNum();
	}

	return ret;
}

/* GPL commands */

void Script::play(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	_vm->_game->setLoopStatus(kStatusStrange);
	_vm->_game->setExitLoop(true);    
	_vm->_game->loop();
	_vm->_game->setExitLoop(false);    
	_vm->_game->setLoopStatus(kStatusOrdinary);
}

void Script::load(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop() - 1;
	int animID = params.pop() - 1;
	
	uint i;
	GameObject *obj = _vm->_game->getObject(objID);

	// If the animation is already loaded, return
	for(i = 0; i < obj->_anims.size(); ++i) {
		if (obj->_anims[i] == animID) {
			return; 
		}
	}

	// Load the animation into memory

	_vm->_game->loadAnimation(animID, obj->_z);
	
	// We insert the ID of the loaded animation into the object's internal array
	// of owned animation IDs.
	// Care must be taken to store them sorted (increasing order) as some things
	// depend on this.

	for(i = 0; i < obj->_anims.size(); ++i) {
		if (obj->_anims[i] > animID) {		
			break;
		}
	}

	obj->_anims.insert_at(i, animID);
}

void Script::start(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop();
	int animID = params.pop();	

	// Fixes bug in the data files which makes the game crash in the intro
	// TODO: This is possibly exclusive to the English version, so check for that
	if (animID == 657) {
		Common::Queue<int> tmp;
		tmp.push(objID);
		tmp.push(animID);
		this->load(tmp);
	}

	objID -= 1;
	animID -= 1;

	GameObject *obj = _vm->_game->getObject(objID);

	// Stop all animation that the object owns

	for (uint i = 0; i < obj->_anims.size(); ++i) {
		_vm->_anims->stop(obj->_anims[i]);
	}

	Animation *anim = _vm->_anims->getAnimation(animID);
	anim->registerCallback(&Animation::stopAnimation);	

	bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);

	if (objID == kDragonObject || visible) {
		_vm->_anims->play(animID);
	}
}

void Script::startPlay(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop() - 1;
	int animID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	// Stop all animation that the object owns

	for (uint i = 0; i < obj->_anims.size(); ++i) {
		_vm->_anims->stop(obj->_anims[i]);
	}

	Animation *anim = _vm->_anims->getAnimation(animID);
	anim->registerCallback(&Animation::exitGameLoop);	

	_vm->_game->setLoopStatus(kStatusStrange);

	bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);

	if (objID == kDragonObject || visible) {
		_vm->_anims->play(animID);
	}	

	_vm->_game->loop();
	_vm->_game->setExitLoop(false);    
	_vm->_anims->stop(animID);
	_vm->_game->setLoopStatus(kStatusOrdinary);

	anim->registerCallback(&Animation::doNothing);
}

void Script::c_If(Common::Queue<int> &params) {
	int expression = params.pop();
	int jump = params.pop();

	if (expression)
		_jump = jump;
}

void Script::c_Goto(Common::Queue<int> &params) {
	int jump = params.pop();

	_jump = jump;
}

void Script::c_Let(Common::Queue<int> &params) {
	int var = params.pop() - 1;
	int value = params.pop();

	_vm->_game->setVariable(var, value);
}

void Script::mark(Common::Queue<int> &params) {
	_vm->_game->setMarkedAnimationIndex(_vm->_anims->getLastIndex());
}

void Script::release(Common::Queue<int> &params) {
	int markedIndex = _vm->_game->getMarkedAnimationIndex();

	// Also delete those animations from the game's objects
	for (uint i = 0; i < _vm->_game->getNumObjects(); ++i) {
		GameObject *obj = _vm->_game->getObject(i);

		for (uint j = 0; j < obj->_anims.size(); ++j) {
			Animation *anim;

			anim = _vm->_anims->getAnimation(obj->_anims[j]);
			if (anim != NULL && anim->getIndex() > markedIndex)
				obj->_anims.remove_at(j);
		}
	}

	// Delete animations which have an index greater than the marked index
	_vm->_anims->deleteAfterIndex(markedIndex);
}

void Script::objStatOn(Common::Queue<int> &params) {
	int objID = params.pop() - 1;
	int roomID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	obj->_location = roomID;
	obj->_visible = true;
}

void Script::objStat(Common::Queue<int> &params) {
	int status = params.pop();
	int objID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	if (status == 1) {
		return;
	} else if (status == 2) {
		obj->_visible = false;
	} else {
		obj->_visible = false;
		obj->_location = -1;
	}
		
	for (uint i = 0; i < obj->_anims.size(); ++i) {
		_vm->_anims->stop(obj->_anims[i]);
	}	
}

void Script::execInit(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	run(obj->_program, obj->_init);
}

void Script::execLook(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	run(obj->_program, obj->_look);
}

void Script::execUse(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	run(obj->_program, obj->_use);
}

void Script::walkOn(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int x = params.pop();
	int y = params.pop();
	params.pop(); // facing direction, not used yet

	_vm->_game->walkHero(x, y);
}

void Script::walkOnPlay(Common::Queue<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int x = params.pop();
	int y = params.pop();
	params.pop(); // facing direction, not used yet

	// HACK: This should be an onDest action when hero walking is properly implemented
	_vm->_game->setExitLoop(true);
	
	_vm->_game->walkHero(x, y);

	_vm->_game->setLoopSubstatus(kStatusStrange);
	_vm->_game->loop();
	_vm->_game->setLoopSubstatus(kStatusOrdinary);

	_vm->_game->setExitLoop(false);
}

void Script::newRoom(Common::Queue<int> &params) {

	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int room = params.pop() - 1;
	int gate = params.pop() - 1;

	_vm->_game->setRoomNum(room);
	_vm->_game->setGateNum(gate);

	// HACK: Won't be needed once I've implemented the loop properly
	_vm->_game->_roomChange = true;
}

void Script::talk(Common::Queue<int> &params) {

	int personID = params.pop() - 1;
	int sentenceID = params.pop() - 1;

	Surface *surface = _vm->_screen->getSurface();

	// Fetch string
	BAFile *f = _vm->_stringsArchive->getFile(sentenceID);

	// Fetch frame for the speech text
	Animation *speechAnim = _vm->_anims->getAnimation(kSpeechText);
	Text *speechFrame = reinterpret_cast<Text *>(speechAnim->getFrame());

	// Fetch person info
	Person *person = _vm->_game->getPerson(personID);

	// Set the string and text colour
	surface->markDirtyRect(speechFrame->getRect(true));
	speechFrame->setText(Common::String((const char *)f->_data+1, f->_length-1));
	speechFrame->setColour(person->_fontColour);

	// HACK: Some strings in the English data files are too long to fit the screen
	// This is a temporary resolution.
	if (speechFrame->getWidth() >= kScreenWidth) {
		speechFrame->setFont(_vm->_smallFont);
	} else {
		speechFrame->setFont(_vm->_bigFont);		
	}

	// Set the loop substatus to an appropriate value
	_vm->_game->setLoopSubstatus(kStatusTalk);

	// Record time
	_vm->_game->setSpeechTick(_vm->_system->getMillis());

	// TODO: Implement inventory part

	// Set speech text coordinates

	int x = surface->centerOnX(person->_x, speechFrame->getWidth());
	int y = surface->centerOnY(person->_y, speechFrame->getHeight() * 2);

	speechFrame->setX(x);
	speechFrame->setY(y);

	// Call the game loop to enable interactivity until the text expires
	_vm->_game->loop();

	// Delete the text
	_vm->_screen->getSurface()->markDirtyRect(speechFrame->getRect(true));
	speechFrame->setText("");

	// Revert to "normal" loop status
	_vm->_game->setLoopSubstatus(kStatusOrdinary);
	_vm->_game->setExitLoop(false);
}

void Script::dialogue(Common::Queue<int> &params) {
	int dialogueID = params.pop() - 1;

	_vm->_game->dialogueMenu(dialogueID);
}

void Script::loadMap(Common::Queue<int> &params) {
	int mapID = params.pop() - 1;

	_vm->_game->loadWalkingMap(mapID);
}

void Script::resetDialogue(Common::Queue<int> &params) {
	
	for (int i = 0; i < _vm->_game->_blockNum; ++i) {
		_vm->_game->_dialogueVars[_vm->_game->_dialogueOffsets[_vm->_game->_currentDialogue]+i] = 0;
	}
}

void Script::resetDialogueFrom(Common::Queue<int> &params) {

	for (int i = _vm->_game->_currentBlock; i < _vm->_game->_blockNum; ++i) {
		_vm->_game->_dialogueVars[_vm->_game->_dialogueOffsets[_vm->_game->_currentDialogue]+i] = 0;
	}
}

void Script::resetBlock(Common::Queue<int> &params) {
	int blockID = params.pop();

	_vm->_game->_dialogueVars[_vm->_game->_dialogueOffsets[_vm->_game->_currentDialogue]+blockID] = 0;
}

void Script::exitDialogue(Common::Queue<int> &params) {
	_vm->_game->_dialogueExit = true;
}

void Script::roomMap(Common::Queue<int> &params) {

	// Load the default walking map for the room
	_vm->_game->loadWalkingMap();
}

void Script::endCurrentProgram() {
	_endProgram = true;
}

/**
 * @brief Evaluates mathematical expressions
 * @param reader Stream reader set to the beginning of the expression
 */

int Script::handleMathExpression(Common::MemoryReadStream &reader) {
	Common::Stack<int> stk;
	mathExpressionObject obj;
	GPL2Operator oper;
	GPL2Function func;

	debugC(3, kDraciBytecodeDebugLevel, "\t<MATHEXPR>");

	// Read in initial math object
	obj = (mathExpressionObject)reader.readSint16LE();

	int value;
	int arg1, arg2, res;

	while (1) {
		if (obj == kMathEnd) {
			// Check whether the expression was evaluated correctly
			// The stack should contain only one value after the evaluation
			// i.e. the result of the expression
			assert(stk.size() == 1 && "Mathematical expression error");	
			break;
		}

		switch (obj) {

		// If the object type is not known, assume that it's a number
		default:
		case kMathNumber:
			value = reader.readSint16LE();
			stk.push(value);
			debugC(3, kDraciBytecodeDebugLevel, "\t\tnumber: %d", value);
			break;

		case kMathOperator:
			value = reader.readSint16LE();
			arg2 = stk.pop();
			arg1 = stk.pop();

			// Fetch operator
			oper = _operatorList[value-1];

			// Calculate result
			res = (this->*(oper._handler))(arg1, arg2);
			
			// Push result
			stk.push(res);

			debugC(3, kDraciBytecodeDebugLevel, "\t\t%d %s %d (res: %d)",
				arg1, oper._name.c_str(), arg2, res);
			break;

		case kMathVariable:
			value = reader.readSint16LE();

			stk.push(_vm->_game->getVariable(value-1));

			debugC(3, kDraciBytecodeDebugLevel, "\t\tvariable: %d (%d)", value,
				_vm->_game->getVariable(value-1));
			break;

		case kMathFunctionCall:
			value = reader.readSint16LE();

			// Fetch function
			func = _functionList[value-1];
					
			// If not yet implemented	
			if (func._handler == NULL) {
				stk.pop();

				// FIXME: Pushing dummy value for now, but should push return value
				stk.push(0);

				debugC(3, kDraciBytecodeDebugLevel, "\t\tcall: %s (not implemented)",
					func._name.c_str());
			} else {
				arg1 = stk.pop();

				// Calculate result
				res = (this->*(func._handler))(arg1);

				// Push the result on the evaluation stack
				stk.push(res);
			
				debugC(3, kDraciBytecodeDebugLevel, "\t\tcall: %s(%d) (res: %d)",
					func._name.c_str(), arg1, res);
			}

			break;
		}

		obj = (mathExpressionObject) reader.readSint16LE();
	}

	return stk.pop();
}

/**
 * @brief Evaluates a GPL mathematical expression on a given offset and returns 
 * the result (which is normally a boolean-like value)
 * 
 * @param program	A GPL2Program instance of the program containing the expression
 * @param offset	Offset of the expression inside the program (in multiples of 2 bytes)
 * 
 * @return The result of the expression converted to a bool.
 *
 * Reference: the function equivalent to this one is called "Can()" in the original engine.
 */
bool Script::testExpression(GPL2Program program, uint16 offset) {
	
	// Initialize program reader
	Common::MemoryReadStream reader(program._bytecode, program._length);

	// Offset is given as number of 16-bit integers so we need to convert
	// it to a number of bytes  
	offset -= 1;
	offset *= 2;

	// Seek to the expression
	reader.seek(offset);

	debugC(2, kDraciBytecodeDebugLevel, 
		"Evaluating (standalone) GPL expression at offset %d:", offset);

	return (bool)handleMathExpression(reader);
}

/**
 * @brief Find the current command in the internal table
 *
 * @param num 		Command number
 * @param subnum 	Command subnumer
 *
 * @return NULL if command is not found. Otherwise, a pointer to a GPL2Command
 *         struct representing the command.
 */
const GPL2Command *Script::findCommand(byte num, byte subnum) {
	unsigned int i = 0;
	while (1) {

		// Command not found
		if (i >= kNumCommands) {
			break;
		}

		// Return found command
		if (_commandList[i]._number == num &&
			_commandList[i]._subNumber == subnum) {
			return &_commandList[i];
		}

		++i;
	}

	return NULL;
}

/**
 * @brief GPL2 bytecode interpreter
 * @param program GPL program in the form of a GPL2Program struct
 *        offset Offset into the program where execution should begin
 *
 * GPL2 is short for Game Programming Language 2 which is the script language
 * used by Draci Historie. This is the interpreter for the language.
 *
 * A compiled GPL2 program consists of a stream of bytes representing commands
 * and their parameters. The syntax is as follows:
 *
 * Syntax of a command:
 *	<name of the command> <number> <sub-number> <list of parameters...>
 *
 *	Syntax of a parameter:
 *	- 1: integer number literally passed to the program
 *	- 2-1: string stored in the reservouir of game strings (i.e. something to be
 *	  displayed) and stored as an index in this list
 *	- 2-2: string resolved by the compiler (i.e., a path to another file) and
 *	  replaced by an integer index of this entity in the appropriate namespace
 *	  (e.g., the index of the palette, location, ...)
 *	- 3-0: relative jump to a label defined in this code.  Each label must be
 *	  first declared in the beginning of the program.
 *	- 3-1 .. 3-9: index of an entity in several namespaces, defined in file ident
 *	- 4: mathematical expression compiled into a postfix format
 *
 * 	In the compiled program, parameters of type 1..3 are represented by a single
 *	16-bit integer.  The called command knows by its definition what namespace the
 *	value comes from.
 */

int Script::run(GPL2Program program, uint16 offset) {

	int oldJump = _jump;

	// Mark the last animation index before we do anything so a Release command
	// doesn't unload too many animations if we forget to use a Mark command first
	_vm->_game->setMarkedAnimationIndex(_vm->_anims->getLastIndex());

	// Stream reader for the whole program
	Common::MemoryReadStream reader(program._bytecode, program._length);
	
	// Parameter queue that is passed to each command
	Common::Queue<int> params;

	// Offset is given as number of 16-bit integers so we need to convert
	// it to a number of bytes  
	offset -= 1;
	offset *= 2;

	// Seek to the requested part of the program
	reader.seek(offset);
	
	debugC(3, kDraciBytecodeDebugLevel, 
		"Starting GPL program at offset %d (program length: %d)", offset, program._length);

	const GPL2Command *cmd;
	do {

		// Account for GPL jump that some commands set
		if (_jump != 0)	{
			debugC(6, kDraciBytecodeDebugLevel, 
				"Jumping from offset %d to %d (%d bytes)", 
				reader.pos(), reader.pos() + _jump, _jump);	
			reader.seek(_jump, SEEK_CUR);
		}

		// Reset jump
		_jump = 0;

		// Clear any parameters left on the stack from the previous command
		// This likely won't be needed once all commands are implemented
		params.clear();

		// read in command pair
		uint16 cmdpair = reader.readUint16BE();

		// extract high byte, i.e. the command number
		byte num = (cmdpair >> 8) & 0xFF;

		// extract low byte, i.e. the command subnumber
		byte subnum = cmdpair & 0xFF;

		// This might get set by some GPL commands via Script::endCurrentProgram()
		// if they need a program to stop midway
		_endProgram = false;

		if ((cmd = findCommand(num, subnum))) {
			int tmp;

			// Print command name
			debugC(1, kDraciBytecodeDebugLevel, "%s", cmd->_name.c_str());

			for (int i = 0; i < cmd->_numParams; ++i) {
				if (cmd->_paramTypes[i] == 4) {
					debugC(2, kDraciBytecodeDebugLevel, 
						"Evaluating (in-script) GPL expression at offset %d: ", offset);
					params.push(handleMathExpression(reader));
				}
				else {
					tmp = reader.readSint16LE();
					params.push(tmp);
					debugC(2, kDraciBytecodeDebugLevel, "\t%d", tmp);
				}
			}
		}
		else {
			debugC(1, kDraciBytecodeDebugLevel, "Unknown opcode %d, %d",
				num, subnum);
			abort();
		}

		GPLHandler handler = cmd->_handler;

		if (handler != NULL) {
			// Call the handler for the current command
			(this->*(cmd->_handler))(params);
		}

	} while (cmd->_name != "gplend" && cmd->_name != "exit" && !_endProgram);

	_endProgram = false;
	_jump = oldJump;

	return 0;
}

}

