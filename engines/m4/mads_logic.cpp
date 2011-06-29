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

#include "common/textconsole.h"

#include "m4/m4.h"
#include "m4/dialogs.h"
#include "m4/mads_logic.h"
#include "m4/scene.h"

#define MAX_CALL_PARAMS 10

namespace M4 {

void MadsGameLogic::initializeGlobals() {
	// Clear the entire globals list
	Common::set_to(&_madsVm->globals()->_globals[0], &_madsVm->globals()->_globals[TOTAL_NUM_VARIABLES], 0);

	SET_GLOBAL(4, 8);
	SET_GLOBAL(33, 1);
	SET_GLOBAL(10, 0xFFFF);
	SET_GLOBAL(13, 0xFFFF);
	SET_GLOBAL(15, 0xFFFF);
	SET_GLOBAL(19, 0xFFFF);
	SET_GLOBAL(20, 0xFFFF);
	SET_GLOBAL(21, 0xFFFF);
	SET_GLOBAL(95, 0xFFFF);

	// TODO: unknown sub call

	// Put the values 0 through 3 in a random ordering in global slots 83 - 86
	for (int idx = 0; idx < 4; ) {
		int randVal = _madsVm->_random->getRandomNumber(4);
		SET_GLOBAL(83 + idx, randVal);

		// Check whether the given value has already been used
		bool flag = false;
		for (int idx2 = 0; idx2 < idx; ++idx2) {
			if (randVal == GET_GLOBAL(83 + idx2))
				flag = true;
		}

		if (!flag)
			++idx;
	}

	// Put the values 0 through 3 in a random ordering in global slots 87 - 90
	for (int idx = 0; idx < 4; ) {
		int randVal = _madsVm->_random->getRandomNumber(3);
		SET_GLOBAL(87 + idx, randVal);

		// Check whether the given value has already been used
		bool flag = false;
		for (int idx2 = 0; idx2 < idx; ++idx2) {
			if (randVal == GET_GLOBAL(87 + idx2))
				flag = true;
		}

		if (!flag)
			++idx;
	}

	// Miscellaneous global settings
	SET_GLOBAL(120, 501);
	SET_GLOBAL(121, 0xFFFF);
	SET_GLOBAL(110, 0xFFFF);
	SET_GLOBAL(119, 1);
	SET_GLOBAL(134, 4);
	SET_GLOBAL(190, 201);
	SET_GLOBAL(191, 301);
	SET_GLOBAL(192, 413);
	SET_GLOBAL(193, 706);
	SET_GLOBAL(194, 801);
	SET_GLOBAL(195, 551);
	SET_GLOBAL(196, 752);

	// Fill out the globals 200 - 209 with unique random number values less than 10000
	for (int idx = 0; idx < 10; ) {
		int randVal = _madsVm->_random->getRandomNumber(9999);
		SET_GLOBAL(200 + idx, randVal);

		// Check whether the given value has already been used
		bool flag = false;
		for (int idx2 = 0; idx2 < idx; ++idx2) {
			if (randVal == GET_GLOBAL(87 + idx2))
				flag = true;
		}

		if (!flag)
			++idx;
	}

	switch (_madsVm->globals()->_difficultyLevel) {
	case 1:
		// Very hard
		SET_GLOBAL(35, 0);
		// TODO: object set room
		SET_GLOBAL(137, 5);
		SET_GLOBAL(136, 0);
		break;

	case 2:
		// Hard
		SET_GLOBAL(35, 0);
		// TODO: object set room
		SET_GLOBAL(136, 0xFFFF);
		SET_GLOBAL(137, 6);
		break;

	case 3:
		// Easy
		SET_GLOBAL(35, 2);
		// TODO: object set room
		break;
	}

	_madsVm->_player._direction = 8;
	_madsVm->_player._newDirection = 8;

	// TODO: unknown processing routine getting called for 'RXM' and 'ROX'
}

/*--------------------------------------------------------------------------*/

const char *MadsSceneLogic::subFormatList[] = {"scene%d_enter", "scene%d_step", "scene%d_preaction", "scene%d_actions"};

#define	OPSIZE8		0x40	///< when this bit is set - the operand size is 8 bits
#define	OPSIZE16	0x80	///< when this bit is set - the operand size is 16 bits
#define	OPMASK		0x3F	///< mask to isolate the opcode

enum Opcodes {
	OP_HALT	= 0, OP_IMM	= 1, OP_ZERO =  2, OP_ONE = 3, OP_MINUSONE = 4, OP_STR = 5, OP_DLOAD = 6,
	OP_DSTORE = 7, OP_PAL = 8, OP_LOAD = 9, OP_GLOAD = 10, OP_STORE = 11, OP_GSTORE = 12,
	OP_CALL = 13, OP_LIBCALL = 14, OP_RET = 15, OP_ALLOC = 16, OP_JUMP = 17, OP_JMPFALSE = 18,
	OP_JMPTRUE = 19, OP_EQUAL = 20, OP_LESS = 21, OP_LEQUAL = 22, OP_NEQUAL = 23, OP_GEQUAL = 24,
	OP_GREAT = 25, OP_PLUS = 26, OP_MINUS = 27, OP_LOR = 28, OP_MULT = 29, OP_DIV = 30,
	OP_MOD = 31, OP_AND = 32, OP_OR = 33, OP_EOR = 34, OP_LAND = 35, OP_NOT = 36, OP_COMP = 37,
	OP_NEG = 38, OP_DUP = 39,
	TOTAL_OPCODES = 40
};

const char *MadsSceneLogic::_opcodeStrings[] = {
	"HALT", "IMM", "ZERO", "ONE", "MINUSONE", "STR", "DLOAD", "DSTORE", NULL, "LOAD", "GLOAD",
	"STORE", "GSTORE", "CALL", "LIBCALL", "RET", "ALLOC", "JUMP", "JMPFALSE", "JMPTRUE", "EQUAL",
	"LESS", "LEQUAL", "NEQUAL", "GEQUAL", "GREAT", "PLUS", "MINUS", "LOR", "MULT", "DIV",
	"MOD", "AND", "OR", "EOR", "LAND", "NOT", "COMP", "NEG", "DUP"
};

/**
 * This method sets up the data map with pointers to all the common game objects. This allows the script engine to
 * convert game specific offsets for various fields in the original game's data segment into a generic data index
 * that will be common across all the MADS games

void MadsSceneLogic::initializeDataMap() {
	// The unique order of these items must be maintained
}
*/

uint32 MadsSceneLogic::getDataValue(int dataId) {
	switch (dataId) {
	case 1:
		return _madsVm->scene()->_abortTimersMode2;
	case 2:
		return _madsVm->scene()->_abortTimers;
	case 3:
		return _madsVm->_player._stepEnabled ? 0xffff : 0;
	case 4:
		return _madsVm->scene()->_nextScene;
	case 5:
		return _madsVm->scene()->_previousScene;
	case 6:
		return _madsVm->_player._playerPos.x;
	case 7:
		return _madsVm->_player._playerPos.y;
	case 8:
		return _madsVm->_player._direction;
	case 9:
		return _madsVm->_player._visible ? 0xffff : 0;
	case 10:
		return getActiveAnimationBool();
	case 11:
		return getAnimationCurrentFrame();
	default:
		// All other data variables get stored in the hash table
		return _madsVm->globals()->_dataMap[dataId];
		break;
	}
}

void MadsSceneLogic::setDataValue(int dataId, uint16 dataValue) {
	switch (dataId) {
	case 1:
		_madsVm->scene()->_abortTimersMode2 = (AbortTimerMode)dataValue;
		break;
	case 2:
		_madsVm->scene()->_abortTimers = dataValue;
		break;
	case 3:
		_madsVm->_player._stepEnabled = dataValue != 0;
		break;
	case 4:
		_madsVm->scene()->_nextScene = dataValue;
		break;
	case 5:
		_madsVm->scene()->_previousScene = dataValue;
		break;
	case 6:
		_madsVm->_player._playerPos.x = dataValue;
		break;
	case 7:
		_madsVm->_player._playerPos.y = dataValue;
		break;
	case 8:
		_madsVm->_player._direction = dataValue;
		break;
	case 9:
		_madsVm->_player._visible = dataValue != 0;
		break;
	case 10:
	case 11:
		error("Tried to set read only data field %d", dataId);
		break;
	default:
		// All other data variables get stored in the hash table
		_madsVm->globals()->_dataMap[dataId] = dataValue;
		break;
	}
}

const char *MadsSceneLogic::formAnimName(char sepChar, int16 suffixNum) {
	return MADSResourceManager::getResourceName(sepChar, _sceneNumber, EXTTYPE_NONE, NULL, suffixNum);
}

void MadsSceneLogic::getSceneSpriteSet() {
	char prefix[100];

	// Room change sound
	_madsVm->_sound->playSound(5);

	// Set up sprite set prefix to use
	if ((_sceneNumber <= 103) || (_sceneNumber == 111)) {
		if (_madsVm->globals()->_globals[0] == SEX_FEMALE)
			strcpy(prefix, "ROX");
		else
			strcpy(prefix, "RXM");
	} else if (_sceneNumber <= 110) {
		strcpy(prefix, "RXSW");
		_madsVm->globals()->_globals[0] = SEX_UNKNOWN;
	} else if (_sceneNumber == 112)
		strcpy(prefix, "");

	_madsVm->globals()->playerSpriteChanged = true;
	_madsVm->_player.loadSprites(prefix);

//	if ((_sceneNumber == 105) ((_sceneNumber == 109) && (word_84800 != 0)))
//		_madsVm->globals()->playerSpriteChanged = true;

	_vm->_palette->setEntry(16, 0x38, 0xFF, 0xFF);
	_vm->_palette->setEntry(17, 0x38, 0xb4, 0xb4);
}

void MadsSceneLogic::getAnimName() {
	const char *newName = MADSResourceManager::getAAName(
		((_sceneNumber <= 103) || (_sceneNumber > 111)) ? 0 : 1);
	strcpy(_madsVm->scene()->_aaName, newName);
}

/*--------------------------------------------------------------------------*/

uint16 MadsSceneLogic::loadSpriteSet(uint16 suffixNum, uint16 sepChar) {
	assert(sepChar < 256);
	const char *resName = formAnimName((char)sepChar, (int16)suffixNum);
	return _madsVm->scene()->loadSceneSpriteSet(resName);
}

uint16 MadsSceneLogic::startReversibleSpriteSequence(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, flipped, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0,
		true, 100, depth - 1, 1, ANIMTYPE_REVERSIBLE, 0, 0);
}

uint16 MadsSceneLogic::startCycledSpriteSequence(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, flipped, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0,
		true, 100, depth - 1, 1, ANIMTYPE_CYCLED, 0, 0);
}

uint16 MadsSceneLogic::startSpriteSequence3(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, flipped, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0,
		true, 100, depth - 1, -1, ANIMTYPE_CYCLED, 0, 0);
}

void MadsSceneLogic::activateHotspot(int idx, bool active) {
	// TODO:
}

void MadsSceneLogic::getPlayerSpritesPrefix() {
	_madsVm->_sound->playSound(5);

	char oldName[80];
	strcpy(oldName, _madsVm->_player._spritesPrefix);

	if ((_madsVm->globals()->_nextSceneId <= 103) || (_madsVm->globals()->_nextSceneId == 111))
		strcpy(_madsVm->_player._spritesPrefix, (_madsVm->globals()->_globals[0] == SEX_FEMALE) ? "ROX" : "RXM");
	else if (_madsVm->globals()->_nextSceneId <= 110)
		strcpy(_madsVm->_player._spritesPrefix, "RXSM");
	else if (_madsVm->globals()->_nextSceneId == 112)
		strcpy(_madsVm->_player._spritesPrefix, "");

	if (strcmp(oldName, _madsVm->_player._spritesPrefix) != 0)
		_madsVm->_player._spritesChanged = true;

	if ((_madsVm->globals()->_nextSceneId == 105) ||
		((_madsVm->globals()->_nextSceneId == 109) && (_madsVm->globals()->_globals[15] != 0))) {
		// TODO: unknown flag setting
		_madsVm->_player._spritesChanged = true;
	}

	_madsVm->_palette->setEntry(16, 40, 255, 255);
	_madsVm->_palette->setEntry(17, 40, 180, 180);

}

void MadsSceneLogic::getPlayerSpritesPrefix2() {
	_madsVm->_sound->playSound(5);

	char oldName[80];
	strcpy(oldName, _madsVm->_player._spritesPrefix);

	if ((_madsVm->globals()->_nextSceneId == 213) || (_madsVm->globals()->_nextSceneId == 216))
		strcpy(_madsVm->_player._spritesPrefix, "");
	else if (_madsVm->globals()->_globals[0] == SEX_MALE)
		strcpy(_madsVm->_player._spritesPrefix, "RXM");
	else
		strcpy(_madsVm->_player._spritesPrefix, "ROX");

	// TODO: unknown flag setting for next scene Id > 212

	if (strcmp(oldName, _madsVm->_player._spritesPrefix) != 0)
		_madsVm->_player._spritesChanged = true;

/*	if ((_madsVm->globals()->_nextSceneId == 203) && (_madsVm->globals()->_nextSceneId == 204) &&
		(_madsVm->globals()->_globals[0x22] == 0))
		// TODO: unknown flag set
*/
	_madsVm->_palette->setEntry(16, 40, 255, 255);
	_madsVm->_palette->setEntry(17, 40, 180, 180);
}


/*--------------------------------------------------------------------------*/

/**
 * Loads the MADS.DAT file and loads the script data for the correct game/language
 */
void MadsSceneLogic::initializeScripts() {
	Common::File f;
	if (!f.open("mads.dat")) {
		warning("Could not locate mads.dat file");
		return;
	}

	// Validate that the file being read is a valid mads.dat file
	char header[4];
	f.read(&header[0], 4);
	if (strncmp(header, "MADS", 4) != 0) {
		warning("Invalid mads.dat file");
		return;
	}

	// Get a list of the offsets of game blocks
	uint32 v;
	Common::Array<uint32> offsets;
	while ((v = f.readUint32LE()) != 0)
		offsets.push_back(v);

	// Check the header of each block in turn
	_scriptsData = NULL;
	_scriptsSize = 0;

	for (uint i = 0; i < offsets.size(); ++i) {
		// Get the block header
		f.seek(offsets[i]);
		byte gameId = f.readByte();
		byte language = f.readByte();
		f.readByte();			// Language currently unused

		// If this block isn't for the current game, skip it
		if (_madsVm->getGameType() != (gameId + 2))
			continue;
		if ((language != 1) || (_madsVm->getLanguage() != Common::EN_ANY))
			continue;

		// Found script block for the given game and language.
		_scriptsSize = (i < (offsets.size() - 1)) ? offsets[i + 1] - offsets[i] : f.size() - offsets[i];
		break;
	}

	if (!_scriptsSize) {
		warning("Could not find appropriate scripts block for game in mads.dat file");
		f.close();
		return;
	}

	// Load up the list of subroutines into a hash map
	uint32 blockOffset = f.pos() - 3;
	uint32 subsStart = 0;
	for (;;) {
		// Get next entry
		Common::String subName;
		char c;
		while ((c = (char)f.readByte()) != '\0')
			subName += c;
		if (subName.empty())
			// Reached end of subroutine list
			break;

		// Read in the offset of the routine
		uint32 offset = f.readUint32LE();
		if (_subroutines.empty()) {
			// The first subroutine offset is used to reduce the amount of data to later load in. In essence,
			// the subroutine index will not be separately loaded, since it's contents will be in the hash map
			subsStart = offset;
			_scriptsSize -= offset;
		}

		_subroutines[subName] = offset - subsStart;
		_subroutineOffsets.push_back(offset - subsStart);
	}

	// Read in the remaining data
	f.seek(blockOffset + subsStart, SEEK_SET);
	_scriptsData = (byte *)malloc(_scriptsSize);
	f.read(_scriptsData, _scriptsSize);

	f.close();
}

void MadsSceneLogic::selectScene(int sceneNum) {
	assert(sceneNum == 101);
	_sceneNumber = sceneNum;

	Common::set_to(&_spriteIndexes[0], &_spriteIndexes[50], 0);

	// If debugging is turned on, show a debug warning if any of the scene methods aren't present
	if (gDebugLevel > 0) {
		for (int i = 0; i < 4; ++i) {
			char buffer[20];
			sprintf(buffer, subFormatList[i], sceneNum);
			Common::HashMap<Common::String, uint32>::iterator it = _subroutines.find(Common::String(buffer));
			if (it == _subroutines.end())
				debugC(1, kDebugScript, "Scene method %s not found", buffer);
		}
	}
}

void MadsSceneLogic::setupScene() {
	// FIXME: This is the hardcoded logic for Rex scene 101 only
	const char *animName = formAnimName('A', -1);
	warning("anim - %s", animName);

//	sub_1e754(animName, 3);

	if ((_sceneNumber >= 101) && (_sceneNumber <= 112))
		getPlayerSpritesPrefix();
	else
		getPlayerSpritesPrefix2();

	getAnimName();
}

/**
 * Handles the logic when a scene is entered
 */
void MadsSceneLogic::doEnterScene() {
	char buffer[20];
	sprintf(buffer, subFormatList[SUBFORMAT_ENTER], _sceneNumber);
	execute(Common::String(buffer));
}

/**
 * Handles the script execution which is called regularly every frame
 */
void MadsSceneLogic::doSceneStep() {
	char buffer[20];
	sprintf(buffer, subFormatList[SUBFORMAT_STEP], _sceneNumber);
	execute(Common::String(buffer));
}

/**
 * Handles and preactions before an action is started
 */
void MadsSceneLogic::doPreactions() {
	char buffer[20];
	sprintf(buffer, subFormatList[SUBFORMAT_PREACTIONS], _sceneNumber);
	execute(Common::String(buffer));
}

/**
 * Handles any action that has been selected
 */
void MadsSceneLogic::doAction() {
	char buffer[20];
	sprintf(buffer, subFormatList[SUBFORMAT_ACTIONS], _sceneNumber);
	execute(Common::String(buffer));
}

/**
 * Executes the script with the specified name
 */
void MadsSceneLogic::execute(const Common::String &scriptName) {
	Common::HashMap<Common::String, uint32>::iterator it = _subroutines.find(scriptName);
	if (it != _subroutines.end())
		execute(it->_value);
}

#define UNUSED_VAL 0xEAEAEAEA
/**
 * Executes the script at the specified offset
 */
void MadsSceneLogic::execute(uint32 subOffset) {
	Common::Array<ScriptVar> locals;
	Common::Stack<ScriptVar> stack;
	char opcodeBuffer[100];
	uint32 scriptOffset = subOffset;
	uint32 param;

	debugC(1, kDebugScript, "executing script at %xh", subOffset);

	bool done = false;
	while (!done) {
		param = UNUSED_VAL;
		byte opcode = _scriptsData[scriptOffset++];
		sprintf(opcodeBuffer, "%.4x[%.2d] - %s", scriptOffset - 1, stack.size(), _opcodeStrings[opcode & OPMASK]);

		switch (opcode & OPMASK) {
		case OP_HALT:			// end of program
		case OP_RET:
			done = true;
			break;

		case OP_IMM:			// Loads immediate value onto stack
			param = getParam(scriptOffset, opcode);
			stack.push(ScriptVar(param));
			break;

		case OP_ZERO:			// loads zero onto stack
			stack.push(ScriptVar((uint32)0));
			break;

		case OP_ONE:			// loads one onto stack
			stack.push(ScriptVar(1));
			break;

		case OP_MINUSONE:		// loads minus one (0xffff) onto stack
			stack.push(ScriptVar(0xffff));
			break;

		case OP_DLOAD: {		// Gets data variable
			param = getParam(scriptOffset, opcode);
			uint16 v = getDataValue(param);
			stack.push(ScriptVar(v));
			break;
		}

		case OP_DSTORE:	{		// Stores data variable
			param = getParam(scriptOffset, opcode);
			ScriptVar v = stack.pop();
			setDataValue(param, v.isInt() ? v.get() : 0);
			break;
		}

		case OP_LOAD:			// loads local variable onto stack
			param = getParam(scriptOffset, opcode);
			stack.push(locals[param]);
			break;

		case OP_STORE:			// Pops a value and stores it in a local
			// Get the local index and expand the locals store if necessary
			param = getParam(scriptOffset, opcode);
			while (param >= locals.size())
				locals.push_back(ScriptVar());

			locals[param] = stack.pop();
			break;

		case OP_GLOAD:				// loads global variable onto stack
			param = getParam(scriptOffset, opcode);
			assert(param < TOTAL_NUM_VARIABLES);
			stack.push(_madsVm->globals()->_globals[param]);
			break;

		case OP_GSTORE:				// pops stack and stores in global variable
			param = getParam(scriptOffset, opcode);
			assert(param < TOTAL_NUM_VARIABLES);
			_madsVm->globals()->_globals[param] = stack.pop().get();
			break;

		case OP_CALL:				// procedure call
			param = getParam(scriptOffset, opcode);
			assert(param < _subroutineOffsets.size());
			execute(_subroutineOffsets[param]);
			break;

		case OP_LIBCALL:		// library procedure or function call
			param = getParam(scriptOffset, opcode);
			callSubroutine(param, stack);
			break;

		case OP_JUMP:	// unconditional jump
			param = subOffset + getParam(scriptOffset, opcode);
			scriptOffset = param;
			break;

		case OP_JMPFALSE:	// conditional jump
			param = subOffset + getParam(scriptOffset, opcode);
			if (stack.pop().get() == 0)
				// Condition satisfied - do the jump
				scriptOffset = param;
			break;

		case OP_JMPTRUE:	// conditional jump
			param = subOffset + getParam(scriptOffset, opcode);
			if (stack.pop().get() != 0)
				// Condition satisfied - do the jump
				scriptOffset = param;
			break;

		case OP_EQUAL:			// tests top two items on stack for equality
		case OP_LESS:			// tests top two items on stack
		case OP_LEQUAL:			// tests top two items on stack
		case OP_NEQUAL:			// tests top two items on stack
		case OP_GEQUAL:			// tests top two items on stack
		case OP_GREAT:			// tests top two items on stack
		case OP_LOR:			// logical or of top two items on stack and replaces with result
		case OP_LAND:			// logical ands top two items on stack and replaces with result
			{
				uint32 param2 = stack.pop().get();
				uint32 param1 = stack.pop().get();

				// Do the comparison
				uint32 tmp = 0;
				switch (opcode) {
				case OP_EQUAL:  tmp = (param1 == param2); break;
				case OP_LESS:   tmp = (param1 <  param2); break;
				case OP_LEQUAL: tmp = (param1 <= param2); break;
				case OP_NEQUAL: tmp = (param1 != param2); break;
				case OP_GEQUAL: tmp = (param1 >= param2); break;
				case OP_GREAT:  tmp = (param1 >  param2); break;

				case OP_LOR:    tmp = (param1 || param2); break;
				case OP_LAND:   tmp = (param1 && param2); break;
				}

				stack.push(ScriptVar(tmp));
			}
			break;

		case OP_PLUS:			// adds top two items on stack and replaces with result
		case OP_MINUS:			// subs top two items on stack and replaces with result
		case OP_MULT:			// multiplies top two items on stack and replaces with result
		case OP_DIV:			// divides top two items on stack and replaces with result
		case OP_MOD:			// divides top two items on stack and replaces with modulus
		case OP_AND:			// bitwise ands top two items on stack and replaces with result
		case OP_OR:				// bitwise ors top two items on stack and replaces with result
		case OP_EOR:			// bitwise exclusive ors top two items on stack and replaces with result
			{
				uint32 param2 = stack.pop().get();
				uint32 param1 = stack.pop().get();

				// replace other operand with result of operation
				switch (opcode) {
				case OP_PLUS:   param1 += param2; break;
				case OP_MINUS:  param1 -= param2; break;
				case OP_MULT:   param1 *= param2; break;
				case OP_DIV:    param1 /= param2; break;
				case OP_MOD:    param1 %= param2; break;
				case OP_AND:    param1 &= param2; break;
				case OP_OR:     param1 |= param2; break;
				case OP_EOR:    param1 ^= param2; break;
				}

				stack.push(ScriptVar(param1));
			}
			break;

		case OP_NOT:			// logical nots top item on stack
			param = stack.pop().get();
			stack.push(ScriptVar((uint32)(!param) & 0xffff));
			break;

		case OP_COMP:			// complements top item on stack
			param = stack.pop().get();
			stack.push(ScriptVar((~param) & 0xffff));
			break;

		case OP_NEG:			// negates top item on stack
			param = stack.pop().get();
			stack.push(ScriptVar(((uint32)-(int32)param) & 0xffff));
			break;

		case OP_DUP:			// duplicates top item on stack
			stack.push(stack.top());
			break;

		default:
			error("execute() - Unknown opcode");
		}

		// check for stack size
		assert(stack.size() < 100);

		if (gDebugLevel > 0) {
			if (param != UNUSED_VAL)
				sprintf(opcodeBuffer + strlen(opcodeBuffer), "\t%d", param);
			debugC(2, kDebugScript, "%s", opcodeBuffer);
		}
	}

	debugC(1, kDebugScript, "finished executing script");

	// make sure stack is unwound
	assert(stack.size() == 0);
}

uint32 MadsSceneLogic::getParam(uint32 &scriptOffset, int opcode) {
	switch (opcode & (~OPMASK)) {
	case OPSIZE8:
		return _scriptsData[scriptOffset++];
	case OPSIZE16: {
		uint16 v = READ_LE_UINT16(&_scriptsData[scriptOffset]);
		scriptOffset += sizeof(uint16);
		return v;
	}
	default: {
		uint32 v = READ_LE_UINT32(&_scriptsData[scriptOffset]);
		scriptOffset += sizeof(uint32);
		return v;
	}
	}
}

/**
 * Support method for extracting the required number of parameters needed for a library routine call
 */
void MadsSceneLogic::getCallParameters(int numParams, Common::Stack<ScriptVar> &stack, ScriptVar *callParams) {
	assert(numParams <= MAX_CALL_PARAMS);
	for (int i = 0; i < numParams; ++i, ++callParams)
		*callParams = stack.pop();
}

#define EXTRACT_PARAMS(n) getCallParameters(n, stack, p)

void MadsSceneLogic::callSubroutine(int subIndex, Common::Stack<ScriptVar> &stack) {
	ScriptVar p[MAX_CALL_PARAMS];

	switch (subIndex) {
	case 1: {
		// dialog_show
		EXTRACT_PARAMS(1);
		Dialog *dlg = new Dialog(_vm, p[0].getStr(), "TODO: Proper Title");
		_vm->_viewManager->addView(dlg);
		_vm->_viewManager->moveToFront(dlg);
		break;
	}

	case 2:
		// SequenceList_remove
		EXTRACT_PARAMS(1);
		_madsVm->scene()->_sequenceList.remove(p[0]);
		break;

	case 3:
	case 6:
	case 20: {
		// 3: start_reversible_sprite_sequence
		// 6: start_cycled_sprite_sequence
		// 20: start_sprite_sequence3
		EXTRACT_PARAMS(6);
		int idx;
		if (subIndex == 3)
			idx = startReversibleSpriteSequence(p[0], p[1] != 0, p[2], p[3], p[4], p[5]);
		else if (subIndex == 6)
			idx = startCycledSpriteSequence(p[0], p[1], p[2], p[3], p[4], p[5]);
		else
			idx = startSpriteSequence3(p[0], p[1] != 0, p[2], p[3], p[4], p[5]);
		stack.push(ScriptVar(idx));
		break;
	}

	case 4:
		// SequenceList_setAnimRange
		EXTRACT_PARAMS(3);
		_madsVm->scene()->_sequenceList.setAnimRange(p[0], p[1], p[2]);
		break;

	case 5:
		// SequenceList_addSubEntry
		EXTRACT_PARAMS(4);
		stack.push(ScriptVar(_madsVm->scene()->_sequenceList.addSubEntry(p[0], (SequenceSubEntryMode)p[1].get(), p[2], p[3])));
		break;

	case 7: {
		// quotes_get_pointer
		EXTRACT_PARAMS(1);
		const char *quoteStr = _madsVm->globals()->getQuote(p[0]);
		stack.push(ScriptVar(quoteStr));
		break;
	}

	case 8: {
		// KernelMessageList_add
		EXTRACT_PARAMS(8);
		int msgIndex = _madsVm->scene()->_kernelMessages.add(Common::Point(p[0], p[1]), p[2],
			p[3], p[4], p[5] | (p[6] << 16), p[7].getStr());
		stack.push(ScriptVar(msgIndex));
		break;
	}

	case 9:
		// SequenceList_unk3
		EXTRACT_PARAMS(1);
		// TODO: Implement unk3 method
//		stack.push(ScriptVar(_madsVm->scene()->_sequenceList.unk3(p[0])));
		break;

	case 10:
		// start_sound
		EXTRACT_PARAMS(1);
		_madsVm->_sound->playSound(p[0]);
		break;

	case 11:
		// SceneLogic_formAnimName
		EXTRACT_PARAMS(2);
		stack.push(ScriptVar(formAnimName((char)p[0], p[1])));
		break;

	case 12:
		// SpriteList_addSprites
		EXTRACT_PARAMS(2);
		stack.push(ScriptVar(_madsVm->scene()->_spriteSlots.addSprites(p[0].getStr(), false, p[1])));
		break;

	case 13:
		// hotspot_activate
		EXTRACT_PARAMS(2);
		// TODO: Implement setActive version that takes in a hotspot Id
//		_madsVm->scene()->getSceneResources().hotspots->setActive(p[0], p[1] != 0);
		break;

	case 14: {
		// DynamicHotspots_add
		EXTRACT_PARAMS(7);
		int idx = _madsVm->scene()->_dynamicHotspots.add(p[0], p[1], p[2],
			Common::Rect(p[6], p[5], p[6] + p[4], p[5] + p[3]));
		stack.push(ScriptVar(idx));
		break;
	}

	case 15:
		// SequenceList_setDepth
		EXTRACT_PARAMS(2);
		_madsVm->scene()->_sequenceList.setDepth(p[0], p[1]);
		break;

	case 16: {
		// quotes_load
		// Quotes loading can take an arbitrary number of quote Ids, terminated by a 0
		int firstId = -1;
		int quoteId;
		while ((quoteId = stack.pop()) != 0) {
			if (firstId == -1)
				firstId = quoteId;
			_madsVm->globals()->loadQuote(quoteId);
		}

		if (firstId != -1)
			stack.push(ScriptVar(_madsVm->globals()->getQuote(firstId)));
		break;
	}

	case 17: {
		// form_resource_name
		EXTRACT_PARAMS(4);
		const char *suffix = NULL;
		if (p[4].isInt()) {
			// If integer provided for suffix, it must be a value of 0 (NULL)
			uint32 vTemp = p[4] | stack.pop();
			assert(!vTemp);
		} else
			suffix = p[4].getStr();

		stack.push(ScriptVar(MADSResourceManager::getResourceName((char)p[1], p[0], (ExtensionType)p[3].get(),
			suffix, (int16)p[2])));
		break;
	}

	case 18:
		// MadsScene_loadAnimation
		EXTRACT_PARAMS(3);
		_madsVm->scene()->loadAnimation(p[1].getStr(), p[0]);
		break;

	case 19: {
		// Action_isAction
		int verbId = stack.pop();
		int objectNameId = (verbId == 0) ? 0 : stack.pop().get();
		int indirectObjectId = (objectNameId == 0) ? 0 : stack.pop().get();

		stack.push(ScriptVar(_madsVm->scene()->_action.isAction(verbId, objectNameId, indirectObjectId)));
		break;
	}

	case 21:
		// DynamicHotspots_remove
		EXTRACT_PARAMS(1);
		_madsVm->scene()->_dynamicHotspots.remove(p[0]);
		break;

	case 22: {
		// object_is_present
		EXTRACT_PARAMS(1);
		const MadsObject *obj = _madsVm->globals()->getObject(p[0]);
		stack.push(ScriptVar((obj->_roomNumber == _madsVm->scene()->_currentScene)));
		break;
	}

	case 23:
		// inventory_add
		EXTRACT_PARAMS(1);
		_madsVm->scene()->getInterface()->addObjectToInventory(p[0]);
		break;

	case 24: {
		// dialog_picture_show
		EXTRACT_PARAMS(3);
		int messageId = p[0] | (p[1] << 16);
		int objectNum = p[2];
		warning("TODO: Implement dialog with picture. MessageId=%d, objectNum=%d", messageId, objectNum);
		break;
	}

	case 25: {
		// object_is_in_inventory
		EXTRACT_PARAMS(1);
		const MadsObject *obj = _madsVm->globals()->getObject(p[0]);
		stack.push(ScriptVar(obj->isInInventory()));
		break;
	}

	case 26: {
		// object_set_room
		EXTRACT_PARAMS(2);
		MadsObject *obj = _madsVm->globals()->getObject(p[0]);
		obj->setRoom(p[1]);
		break;
	}

	default:
		error("Unknown subroutine %d called", subIndex);
		break;
	}
}

#undef EXTRACT_PARAMS

}
