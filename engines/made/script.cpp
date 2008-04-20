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

#include "common/endian.h"
#include "common/util.h"

#include "made/made.h"
#include "made/script.h"
#include "made/database.h"
#include "made/scriptfuncs.h"

namespace Made {


const char *extendFuncNames[] = {
	"SYSTEM",
	"INITGRAF",
	"RESTOREGRAF",
	"DRAWPIC",
	"CLS",
	"SHOWPAGE",
	"EVENT",
	"EVENTX",
	"EVENTY",
	"EVENTKEY",
	"VISUALFX",
	"PLAYSND",
	"PLAYMUS",
	"STOPMUS",
	"ISMUS",
	"TEXTPOS",
	"FLASH",
	"PLAYNOTE",
	"STOPNOTE",
	"PLAYTELE",
	"STOPTELE",
	"HIDECURS",
	"SHOWCURS",
	"MUSICBEAT",
	"SCREENLOCK",
	"ADDSPRITE",
	"FREEANIM",
	"DRAWSPRITE",
	"ERASESPRITES",
	"UPDATESPRITES",
	"GETTIMER",
	"SETTIMER",
	"RESETTIMER",
	"ALLOCTIMER",
	"FREETIMER",
	"PALETTELOCK",
	"FONT",
	"DRAWTEXT",
	"HOMETEXT",
	"TEXTRECT",
	"TEXTXY",
	"DROPSHADOW",
	"TEXTCOLOR",
	"OUTLINE",
	"LOADCURSOR",
	"SETGROUND",
	"RESTEXT",
	"CLIPAREA",
	"SETCLIP",
	"ISSND",
	"STOPSND",
	"PLAYVOICE",
	"CDPLAY",
	"STOPCD",
	"CDSTATUS",
	"CDTIME",
	"CDPLAYSEG",
	"PRINTF",
	"MONOCLS",
	"SNDENERGY",
	"CLEARTEXT",
	"ANIMTEXT",
	"TEXTWIDTH",
	"PLAYMOVIE",
	"LOADSND",
	"LOADMUS",
	"LOADPIC",
	"MUSICVOL",
	"RESTARTEVENTS",
	"PLACESPRITE",
	"PLACETEXT",
	"DELETECHANNEL",
	"CHANNELTYPE",
	"SETSTATE",
	"SETLOCATION",
	"SETCONTENT",
	"EXCLUDEAREA",
	"SETEXCLUDE",
	"GETSTATE",
	"PLACEANIM",
	"SETFRAME",
	"GETFRAME",
	"GETFRAMECOUNT",
	"PICWIDTH",
	"PICHEIGHT",
	"SOUNDRATE",
	"DRAWANIMPIC",
	"LOADANIM",
	"READTEXT",
	"READMENU",
	"DRAWMENU",
	"MENUCOUNT",
	"SAVEGAME",
	"LOADGAME",
	"GAMENAME",
	"SHAKESCREEN",
	"PLACEMENU",
	"SETVOLUME",
	"WHATSYNTH",
	"SLOWSYSTEM"
};

/* ScriptStack */

ScriptStack::ScriptStack() {
	for (int16 i = 0; i < kScriptStackSize; i++)
		_stack[i] = 0;
	_stackPos = kScriptStackSize;
}

ScriptStack::~ScriptStack() {
}

int16 ScriptStack::top() {
	return _stack[_stackPos];
}

int16 ScriptStack::pop() {
	if (_stackPos == kScriptStackSize)
	    error("ScriptStack::pop() Stack underflow");
	return _stack[_stackPos++];
}

void ScriptStack::push(int16 value) {
	if (_stackPos == 0)
	    error("ScriptStack::push() Stack overflow");
	_stack[--_stackPos] = value;
}

void ScriptStack::setTop(int16 value) {
	_stack[_stackPos] = value;
}

int16 ScriptStack::peek(int16 index) {
	return _stack[index];
}

void ScriptStack::poke(int16 index, int16 value) {
    _stack[index] = value;
}

void ScriptStack::alloc(int16 count) {
	_stackPos -= count;
}

void ScriptStack::free(int16 count) {
	_stackPos += count;
}

void ScriptStack::setStackPos(int16 stackPtr) {
	_stackPos = stackPtr;
}

int16 *ScriptStack::getStackPtr() {
	return &_stack[_stackPos];
}

/* ScriptInterpreter */

ScriptInterpreter::ScriptInterpreter(MadeEngine *vm) : _vm(vm) {
#define COMMAND(x) { &ScriptInterpreter::x, #x }
	static CommandEntry commandProcs[] = {
	    /* 01 */
		COMMAND(cmd_branchTrue),
		COMMAND(cmd_branchFalse),
		COMMAND(cmd_branch),
		COMMAND(cmd_true),
	    /* 05 */
		COMMAND(cmd_false),
		COMMAND(cmd_push),
		COMMAND(cmd_not),
		COMMAND(cmd_add),
	    /* 09 */
		COMMAND(cmd_sub),
		COMMAND(cmd_mul),
		COMMAND(cmd_div),
		COMMAND(cmd_mod),
	    /* 13 */
		COMMAND(cmd_band),
		COMMAND(cmd_bor),
		COMMAND(cmd_bnot),
		COMMAND(cmd_lt),
	    /* 17 */
		COMMAND(cmd_eq),
		COMMAND(cmd_gt),
		COMMAND(cmd_loadConstant),
		COMMAND(cmd_loadVariable),
	    /* 21 */
		COMMAND(cmd_getObjectProperty),
		COMMAND(cmd_setObjectProperty),
		COMMAND(cmd_set),
		COMMAND(cmd_print),
	    /* 25 */
		COMMAND(cmd_terpri),
		COMMAND(cmd_printNumber),
		COMMAND(cmd_vref),
		COMMAND(cmd_vset),
	    /* 29 */
		COMMAND(cmd_vsize),
		COMMAND(cmd_exit),
		COMMAND(cmd_return),
		COMMAND(cmd_call),
	    /* 33 */
		COMMAND(cmd_svar),
		COMMAND(cmd_sset),
		COMMAND(cmd_split),
		COMMAND(cmd_snlit),
	    /* 37 */
		COMMAND(cmd_yorn),
		COMMAND(cmd_save),
		COMMAND(cmd_restore),
		COMMAND(cmd_arg),
	    /* 41 */
		COMMAND(cmd_aset),
		COMMAND(cmd_tmp),
		COMMAND(cmd_tset),
		COMMAND(cmd_tspace),
	    /* 45 */
		COMMAND(cmd_class),
		COMMAND(cmd_objectp),
		COMMAND(cmd_vectorp),
		COMMAND(cmd_restart),
	    /* 49 */
		COMMAND(cmd_rand),
		COMMAND(cmd_randomize),
		COMMAND(cmd_send),
		COMMAND(cmd_extend),
	    /* 53 */
		COMMAND(cmd_catch),
		COMMAND(cmd_cdone),
		COMMAND(cmd_throw),
		COMMAND(cmd_functionp),
	    /* 57 */
		COMMAND(cmd_le),
		COMMAND(cmd_ge),
		COMMAND(cmd_varx),
		COMMAND(cmd_setx)
	};
	_commands = commandProcs;
	_commandsMax = ARRAYSIZE(commandProcs) + 1;
	
	_functions = new ScriptFunctionsRtz(_vm);
	_functions->setupExternalsTable();
	
#undef COMMAND
}

ScriptInterpreter::~ScriptInterpreter() {
	delete _functions;
}

void ScriptInterpreter::runScript(int16 scriptObjectIndex) {

    _terminated = false;
    _runningScriptObjectIndex = scriptObjectIndex;

    _localStackPos = _stack.getStackPos();

    _codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase;
	
	while (!_terminated) {
	    byte opcode = readByte();
	    if (opcode >= 1 && opcode <= _commandsMax) {
	        debug(4, "opcode = %s\n", _commands[opcode - 1].desc);
	        (this->*_commands[opcode - 1].proc)();
		} else {
		    printf("ScriptInterpreter::runScript(%d) Unknown opcode %02X\n", _runningScriptObjectIndex, opcode);
		}
	}

}

byte ScriptInterpreter::readByte() {
	return *_codeIp++;
}

int16 ScriptInterpreter::readInt16() {
	int16 temp = (int16)READ_LE_UINT16(_codeIp);
	_codeIp += 2;
	debug(4, "readInt16() value = %04X\n", temp);
	return temp;
}

void ScriptInterpreter::cmd_branchTrue() {
	int16 ofs = readInt16();
	if (_stack.top() != 0)
		_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_branchFalse() {
	int16 ofs = readInt16();
	if (_stack.top() == 0)
		_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_branch() {
	int16 ofs = readInt16();
	_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_true() {
	_stack.setTop(-1);
}

void ScriptInterpreter::cmd_false() {
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_push() {
	_stack.push();
}

void ScriptInterpreter::cmd_not() {
	if (_stack.top() == 0)
	    _stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_add() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() + value);
}

void ScriptInterpreter::cmd_sub() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() - value);
}

void ScriptInterpreter::cmd_mul() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() * value);
}

void ScriptInterpreter::cmd_div() {
	int16 value = _stack.pop();
	if (value == 0)
	    _stack.setTop(0);
	else
		_stack.setTop(_stack.top() / value);
}

void ScriptInterpreter::cmd_mod() {
	int16 value = _stack.pop();
	if (value == 0)
	    _stack.setTop(0);
	else
		_stack.setTop(_stack.top() % value);
}

void ScriptInterpreter::cmd_band() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() & value);
}

void ScriptInterpreter::cmd_bor() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() | value);
}

void ScriptInterpreter::cmd_bnot() {
	_stack.setTop(~_stack.top());
}

void ScriptInterpreter::cmd_lt() {
	int16 value = _stack.pop();
	if (_stack.top() < value)
		_stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_eq() {
	int16 value = _stack.pop();
	if (_stack.top() == value)
		_stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_gt() {
	int16 value = _stack.pop();
	if (_stack.top() > value)
		_stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_loadConstant() {
	int16 value = readInt16();
	debug(4, "value = %04X (%d)\n", value, value);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_loadVariable() {
	int16 variable = readInt16();
	int16 value = _vm->_dat->getVar(variable);
	debug(4, "variable = %d; value = %d (%04X)\n", variable, value, value); fflush(stdout);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_getObjectProperty() {
	int16 propertyId = _stack.pop();
	int16 objectIndex = _stack.top();
	int16 value = _vm->_dat->getObjectProperty(objectIndex, propertyId);
	debug(4, "value = %04X(%d)\n", value, value);
	//fflush(stdout); g_system->delayMillis(5000);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_setObjectProperty() {
	int16 value = _stack.pop();
	int16 propertyId = _stack.pop();
	int16 objectIndex = _stack.top();
	value = _vm->_dat->setObjectProperty(objectIndex, propertyId, value);
	//fflush(stdout); g_system->delayMillis(5000);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_set() {
	int16 variable = readInt16();
	debug(4, "var(%d) = %04d (%d)\n", variable, _stack.top(), _stack.top());
	_vm->_dat->setVar(variable, _stack.top());
}

void ScriptInterpreter::cmd_print() {
	// TODO: This opcode was used for printing debug messages
    Object *obj = _vm->_dat->getObject(_stack.top());
	const char *text = obj->getString();
	debug(4, "%s", text); fflush(stdout);
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_terpri() {
	// TODO: This opcode was used for printing debug messages
	debug(4, "\n");
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_printNumber() {
	// TODO: This opcode was used for printing debug messages
	debug(4, "%d", _stack.top());
}

void ScriptInterpreter::cmd_vref() {
	int16 index = _stack.pop();
	int16 objectIndex = _stack.top();
	int16 value = 0;
	debug(4, "index = %d; objectIndex = %d\n", index, objectIndex); fflush(stdout);
	if (objectIndex > 0) {
	    Object *obj = _vm->_dat->getObject(objectIndex);
	    value = obj->getVectorItem(index);
	}
	_stack.setTop(value);
	debug(4, "--> value = %d\n", value); fflush(stdout);
}

void ScriptInterpreter::cmd_vset() {
	int16 value = _stack.pop();
	int16 index = _stack.pop();
	int16 objectIndex = _stack.top();
	debug(4, "index = %d; objectIndex = %d; value = %d\n", index, objectIndex, value); fflush(stdout);
	if (objectIndex > 0) {
	    Object *obj = _vm->_dat->getObject(objectIndex);
	    obj->setVectorItem(index, value);
	}
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_vsize() {
	int16 objectIndex = _stack.top();
	if (objectIndex < 1) objectIndex = 1;	// HACK
	Object *obj = _vm->_dat->getObject(objectIndex);
	int16 size = obj->getVectorSize();
	_stack.setTop(size);
}

void ScriptInterpreter::cmd_exit() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_return() {
	// TODO: Check if returning from main function
	int16 funcResult = _stack.top();
	_stack.setStackPos(_localStackPos);
	_localStackPos = kScriptStackLimit - _stack.pop();
	//_localStackPos = _stack.pop();
	_runningScriptObjectIndex = _stack.pop();
    _codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase + _stack.pop();
	byte argc = _stack.pop();
    _stack.free(argc);
	_stack.setTop(funcResult);
	debug(4, "LEAVE: stackPtr = %d; _localStackPos = %d\n\n\n", _stack.getStackPos(), _localStackPos);
}

void ScriptInterpreter::cmd_call() {
	debug(4, "\n\n\nENTER: stackPtr = %d; _localStackPos = %d\n", _stack.getStackPos(), _localStackPos);
	byte argc = readByte();
	_stack.push(argc);
	_stack.push(_codeIp - _codeBase);
	_stack.push(_runningScriptObjectIndex);
	_stack.push(kScriptStackLimit - _localStackPos);
	_localStackPos = _stack.getStackPos();
    _runningScriptObjectIndex = _stack.peek(_localStackPos + argc + 4);
    debug(4, "argc = %d; _runningScriptObjectIndex = %04X\n", argc, _runningScriptObjectIndex); fflush(stdout);
    _codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase;
    //_vm->_dat->dumpObject(_runningScriptObjectIndex);
}

void ScriptInterpreter::cmd_svar() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_sset() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_split() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_snlit() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_yorn() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_save() {
    //fflush(stdout); g_system->delayMillis(5000);
    // TODO
    _stack.setTop(0);
}

void ScriptInterpreter::cmd_restore() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_arg() {
	int16 argIndex = readByte();
	debug(4, "argIndex = %d; value = %04X (%d)\n", argIndex, _stack.peek(_localStackPos + 4 + argIndex), _stack.peek(_localStackPos + 4 + argIndex));
	_stack.setTop(_stack.peek(_localStackPos + 4 + argIndex));
}

void ScriptInterpreter::cmd_aset() {
	int16 argIndex = readByte();
	debug(4, "argIndex = %d; value = %d\n", argIndex, _stack.peek(_localStackPos + 4 + argIndex));
	_stack.poke(_localStackPos + 4 + argIndex, _stack.top());
}

void ScriptInterpreter::cmd_tmp() {
	int16 tempIndex = readByte();
	debug(4, "tempIndex = %d; value = %d\n", tempIndex, _stack.peek(_localStackPos - tempIndex - 1));
	_stack.setTop(_stack.peek(_localStackPos - tempIndex - 1));
}

void ScriptInterpreter::cmd_tset() {
	int16 tempIndex = readByte();
	debug(4, "tempIndex = %d; value = %d\n", tempIndex, _stack.top());
	_stack.poke(_localStackPos - tempIndex - 1, _stack.top());
}

void ScriptInterpreter::cmd_tspace() {
	int16 tempCount = readByte();
	debug(4, "tempCount = %d\n", tempCount);
	_stack.alloc(tempCount);
}

void ScriptInterpreter::cmd_class() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_objectp() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_vectorp() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_restart() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_rand() {
    //fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_randomize() {
    // TODO
    _stack.setTop(0);
}

void ScriptInterpreter::cmd_send() {

	debug(4, "\n\n\nENTER: stackPtr = %d; _localStackPos = %d\n", _stack.getStackPos(), _localStackPos);

	byte argc = readByte();
	
	debug(4, "argc = %d\n", argc);
	
	_stack.push(argc);
	_stack.push(_codeIp - _codeBase);
	_stack.push(_runningScriptObjectIndex);
	_stack.push(kScriptStackLimit - _localStackPos);
 	_localStackPos = _stack.getStackPos();
 	
 	int16 propertyId = _stack.peek(_localStackPos + argc + 2);
	int16 objectIndex = _stack.peek(_localStackPos + argc + 4);

	debug(4, "objectIndex = %d (%04X); propertyId = %d(%04X)\n", objectIndex, objectIndex, propertyId, propertyId); fflush(stdout);
		
	if (objectIndex != 0) {
	    objectIndex = _vm->_dat->getObject(objectIndex)->getClass();
	} else {
		objectIndex = _stack.peek(_localStackPos + argc + 3);
	}

    debug(4, "--> objectIndex = %d(%04X)\n", objectIndex, objectIndex); fflush(stdout);

	if (objectIndex != 0) {
    	_runningScriptObjectIndex = _vm->_dat->getObjectProperty(objectIndex, propertyId);
    	if (_runningScriptObjectIndex != 0) {
    		_codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
			_codeIp = _codeBase;
		} else {
		    _stack.push(0);
		    cmd_return();
		}
	} else {
	    _stack.push(0);
	    cmd_return();
	}

}

void ScriptInterpreter::cmd_extend() {

	byte func = readByte();

	byte argc = readByte();
	int16 *argv = _stack.getStackPtr();

	debug(4, "func = %d (%s); argc = %d\n", func, extendFuncNames[func], argc); fflush(stdout);
	for (int i = 0; i < argc; i++)
	    debug(4, "argv[%02d] = %04X (%d)\n", i, argv[i], argv[i]);

	int16 result = _functions->callFunction(func, argc, argv);
	debug(4, "result = %04X (%d)\n", result, result);
	
	_stack.free(argc);
	
	_stack.setTop(result);

}

void ScriptInterpreter::cmd_catch() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_cdone() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_throw() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_functionp() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_le() {
	int16 value = _stack.pop();
	if (_stack.top() <= value)
		_stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_ge() {
	int16 value = _stack.pop();
	if (_stack.top() >= value)
		_stack.setTop(-1);
	else
	    _stack.setTop(0);
}

void ScriptInterpreter::cmd_varx() {
    fflush(stdout); g_system->delayMillis(5000);
}

void ScriptInterpreter::cmd_setx() {
    fflush(stdout); g_system->delayMillis(5000);
}

} // End of namespace Made
