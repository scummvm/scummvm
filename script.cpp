/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Change Log:
 * $Log$
 * Revision 1.6  2001/11/05 19:21:49  strigeus
 * bug fixes,
 * speech in dott
 *
 * Revision 1.5  2001/10/26 17:34:50  strigeus
 * bug fixes, code cleanup
 *
 * Revision 1.4  2001/10/23 19:51:50  strigeus
 * recompile not needed when switching games
 * debugger skeleton implemented
 *
 * Revision 1.3  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 * Revision 1.2  2001/10/11 13:36:25  strigeus
 * fixed swapped parameters in o_walkActorToActor
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

#define NO_SOUND_HACK

void Scumm::runScript(int script, int a, int b, int16 *lvarptr) {
	byte *scriptPtr;
	uint32 scriptOffs;
	byte scriptType;
	int slot,i;
	ScriptSlot *s;

#ifdef NO_SOUND_HACK
	if (script==212 && _currentRoom==50)
		return;
#endif

	if (script==0)
		return;

	if (b==0)
		stopScriptNr(script);

	if (script < _numGlobalScripts) {
		scriptPtr = getResourceAddress(rtScript, script);
		scriptOffs = 8;
		scriptType = 2;
	} else {
		scriptOffs = _localScriptList[script - _numGlobalScripts];
		if (scriptOffs == 0)
			error("Local script %d is not in room %d", script, _roomResource);
		scriptOffs += 9;
		scriptType = 3;
	}

	slot = getScriptSlot();

	s = &vm.slot[slot];
	s->number = script;
	s->offs = scriptOffs;
	s->status = 2;
	s->type = scriptType;
	s->unk1 = a;
	s->unk2 = b;
	s->freezeCount = 0;
	
	if (lvarptr==NULL) {
		for(i=0; i<16; i++)
			vm.localvar[slot * 0x11 + i] = 0;
	} else {
		for(i=0; i<16; i++)
			vm.localvar[slot * 0x11 + i] = lvarptr[i];
	}

	runScriptNested(slot);
}

void Scumm::stopScriptNr(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i,num;

	if (script==0)
		return;

	ss = &vm.slot[1];
	
	for (i=1; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if (script!=ss->number || ss->type!=2 && ss->type!=3 || ss->status==0)
			continue;

		if (ss->cutsceneOverride)
			error("Script %d stopped with active cutscene/override", script);
		ss->number = 0;
		ss->status = 0;
		if (_currentScript == i)
			_currentScript = 0xFF;
	}

	if (_numNestedScripts==0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script && (nest->type==2 || nest->type==3)) {
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->type = 0xFF;
		}
	} while(nest++,--num);
}

void Scumm::stopObjectScript(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i,num;

	if (script==0)
		return;

	ss = &vm.slot[1];
	
	for (i=1; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if (script==ss->number && (ss->type==1 || ss->type==0 || ss->type==4) && ss->status!=0) {
			if (ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = 0;
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	if (_numNestedScripts==0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script && (nest->type==1 || nest->type==4 || nest->type==0)) {
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->type = 0xFF;
		}
	} while(nest++,--num);
}

int Scumm::getScriptSlot() {
	ScriptSlot *ss;
	int i;
	ss = &vm.slot[1];
	
	for (i=1; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if(ss->status==0)
			return i;
	}
	error("Too many scripts running, %d max", NUM_SCRIPT_SLOT);
}

void Scumm::runScriptNested(int script) {
	NestedScript *nest;
	ScriptSlot *slot;

	updateScriptPtr();
	
	nest = &vm.nest[_numNestedScripts];

	if (_currentScript==0xFF) {
		nest->number = 0xFF;
		nest->type = 0xFF;
	} else {
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->type = slot->type;
		nest->slot = _currentScript;
	}

	if (++_numNestedScripts>=0x10)
		error("Too many nested scripts");

	_currentScript = script;

	getScriptBaseAddress();
	getScriptEntryPoint();
	executeScript();

	_numNestedScripts--;

	nest = &vm.nest[_numNestedScripts];
	
	if (nest->number != 0xFF) {
		slot = &vm.slot[nest->slot];
		if (slot->number == nest->number && slot->type==nest->type &&
			slot->status != 0 && slot->freezeCount==0) {
			_currentScript = nest->slot;
			getScriptBaseAddress();
			getScriptEntryPoint();
			return;
		}
	}
	_currentScript = 0xFF;
}

void Scumm::updateScriptPtr() {
	if (_currentScript == 0xFF)
		return;

	vm.slot[_currentScript].offs = _scriptPointer - _scriptOrgPointer;
}

void Scumm::getScriptBaseAddress() {
	ScriptSlot *ss;
	int index;

	if (_currentScript == 0xFF)
		return;

	ss = &vm.slot[_currentScript];
	switch(ss->type) {
	case 0: /* inventory script **/
		index = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(rtInventory, index);
		_lastCodePtr = &_baseInventoryItems[index];
		break;

	case 3:
	case 1: /* room script */
		_scriptOrgPointer = getResourceAddress(rtRoom, _roomResource);
		_lastCodePtr = &_baseRooms[_roomResource];
		break;

	case 2: /* global script */
		_scriptOrgPointer = getResourceAddress(rtScript, ss->number);
		_lastCodePtr = &_baseScripts[ss->number];
		break;

	case 4: /* flobject script */
		index = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(rtFlObject,_objs[index].fl_object_index);
		_lastCodePtr = &_baseFLObject[ss->number];
		break;
	default:
		error("Bad type while getting base address");
	}
}


void Scumm::getScriptEntryPoint() {
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

void Scumm::executeScript() {
	OpcodeProc op;
	while (_currentScript != 0xFF) {
		_opcode = fetchScriptByte();
		_scriptPointerStart = _scriptPointer;
		vm.slot[_currentScript].didexec = 1;
//		debug(1, "%X", _opcode);
		op = getOpcode(_opcode);
		(this->*op)();
	}
	CHECK_HEAP
}

byte Scumm::fetchScriptByte() {
	if (*_lastCodePtr != _scriptOrgPointer + 6) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	return *_scriptPointer++;
}

int Scumm::fetchScriptWord() {
	int a;

	if (*_lastCodePtr != _scriptOrgPointer + 6) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	
	a = READ_LE_UINT16(_scriptPointer);
	_scriptPointer += 2;

	debug(9, "fetchword=%d", a);
	return a;
}

void Scumm::ignoreScriptWord() {
	fetchScriptWord();
}

void Scumm::ignoreScriptByte() {
	fetchScriptByte();
}


int Scumm::readVar(uint var) {
	int a;
#ifdef BYPASS_COPY_PROT
	static byte copyprotbypassed;
#endif
	debug(9, "readvar=%d", var);
	if (!(var&0xF000)) {
		checkRange(_numVariables-1, 0, var, "Variable %d out of range(r)");
		return _vars[var];
	}

	if (var&0x2000 && _majorScummVersion==5) {
		a = fetchScriptWord();
		if (a&0x2000)
			var = (var+readVar(a&~0x2000))&~0x2000;
		else
			var = (var+(a&0xFFF))&~0x2000;
	}

	if (!(var&0xF000))
		return _vars[var];

	if (var&0x8000) {
		var &= 0x7FFF;
		checkRange(_numBitVariables-1, 0, var, "Bit variable %d out of range(r)");
		return (_bitVars[var>>3] & (1<<(var&7))) ? 1 : 0;
	}

	if (var&0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(r)");

#ifdef BYPASS_COPY_PROT
		if (!copyprotbypassed && _currentScript==1 && _gameId==GID_MONKEY2) {
			copyprotbypassed=1;
			return 1;
		}
#endif
		return vm.localvar[_currentScript * 17 + var];
	}

	error("Illegal varbits (r)");
}

void Scumm::writeVar(uint var, int value) {
	int a;

	if (!(var&0xF000)) {
		checkRange(_numVariables-1, 0, var, "Variable %d out of range(w)");
		_vars[var] = value;
		return;
	}

	if (var&0x8000) {
		var &= 0x7FFF;
		checkRange(_numBitVariables-1, 0, var, "Bit variable %d out of range(w)");
		if (value)
			_bitVars[var>>3] |= (1<<(var&7));
		else
			_bitVars[var>>3] &= ~(1<<(var&7));
		return;
	}

	if (var&0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(w)");
		vm.localvar[_currentScript * 17 + var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void Scumm::getResultPos() {
	int a;

	_resultVarNumber = fetchScriptWord();
	if (_resultVarNumber&0x2000) {
		a = fetchScriptWord();
		if (a&0x2000) {
			_resultVarNumber += readVar(a&~0x2000);
		} else {
			_resultVarNumber += a&0xFFF;
		}
		_resultVarNumber&=~0x2000;
	}

	debug(9, "getResultPos=%d", _resultVarNumber);
}

void Scumm::setResult(int value) {
	writeVar(_resultVarNumber, value);
}

void Scumm::drawBox(int x, int y, int x2, int y2, int color) {
	int top,bottom,count;
	VirtScreen *vs;
	byte *backbuff;

	if ((vs=findVirtScreen(y)) == NULL)
		return;

	top = vs->topline;
	bottom = top + vs->height;

	if (x > x2)
		SWAP(x,x2);

	if (y > y2)
		SWAP(y,y2);

	x2++;
	y2++;

	if (x>319) return;
	if (x<0) x=0;
	if (x2<0) return;
	if (x2>320) x2=320;
	if (y2 > bottom) y2=bottom;

	updateDirtyRect(vs->number, x, x2, y-top, y2-top, 0);

	backbuff = getResourceAddress(rtBuffer, vs->number+1) + vs->xstart + (y-top)*320 + x;

	count = y2 - y;
	while (count) {
		memset(backbuff, color, x2 - x);
		backbuff += 320;
		count--;
	}
}


void Scumm::stopObjectCode() {
	ScriptSlot *ss;

	ss = &vm.slot[_currentScript];

	if (ss->type!=2 && ss->type!=3) {
		if (ss->cutsceneOverride)
			error("Object %d ending with active cutscene/override", ss->number);
		
		/* I wonder if the removal of this breaks anything.
		 * put ss->number and ss->status at another place if using this
 	   * stopObjectScript(ss->number); */
	} else {
		if (ss->cutsceneOverride)
			error("Script %d ending with active cutscene/override", ss->number);
	}
	ss->number = 0;
	ss->status = 0;
	_currentScript = 0xFF;
}

bool Scumm::isScriptInUse(int script) {
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i=0; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if (ss->number == script)
			return true;
	}
	return false;
}

void Scumm::runHook(int i) {
	int16 tmp[16];
	tmp[0] = i;
	if (_vars[VAR_HOOK_SCRIPT]) {
		runScript(_vars[VAR_HOOK_SCRIPT], 0, 0, tmp);
	}
}

void Scumm::freezeScripts(int flag) {
	int i;

	for(i=1; i<NUM_SCRIPT_SLOT; i++) {
		if (_currentScript!=i && vm.slot[i].status!=0 && (vm.slot[i].unk1==0 || flag>=0x80)) {
			vm.slot[i].status |= 0x80;
			vm.slot[i].freezeCount++;
		}
	}

	for (i=0; i<6; i++)
		sentence[i].unk++;

	if(vm.cutSceneScriptIndex != 0xFF) {
		vm.slot[vm.cutSceneScriptIndex].status&=0x7F;
		vm.slot[vm.cutSceneScriptIndex].freezeCount=0;
	}
}

void Scumm::unfreezeScripts() {
	int i;
	for (i=1; i<NUM_SCRIPT_SLOT; i++) {
		if (vm.slot[i].status&0x80) {
			if (!--vm.slot[i].freezeCount) {
				vm.slot[i].status&=0x7F;
			}
		}
	}

	for (i=0; i<6; i++) {
		if (((int8)--sentence[i].unk)<0)
			sentence[i].unk = 0;
	}
}

void Scumm::runAllScripts() {
	int i;

	for (i=0; i<NUM_SCRIPT_SLOT; i++)
		vm.slot[i].didexec = 0;
	
	_currentScript = 0xFF;
	for(_curExecScript = 0; _curExecScript<NUM_SCRIPT_SLOT; _curExecScript++) {
		if (vm.slot[_curExecScript].status == 2 &&
			vm.slot[_curExecScript].didexec == 0) {
			_currentScript = _curExecScript;
			getScriptBaseAddress();
			getScriptEntryPoint();
			executeScript();
		}
	}
}

void Scumm::runExitScript() {
	if (_vars[VAR_EXIT_SCRIPT])
		runScript(_vars[VAR_EXIT_SCRIPT], 0, 0, 0);
	if (_EXCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = 2;
		vm.slot[slot].number = 10001;
		vm.slot[slot].type = 1;
		vm.slot[slot].offs = _EXCD_offs + 8;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (_vars[VAR_EXIT_SCRIPT2])
		runScript(_vars[VAR_EXIT_SCRIPT2], 0, 0, 0);
}

void Scumm::runEntryScript() {
	if (_vars[VAR_ENTRY_SCRIPT])
		runScript(_vars[VAR_ENTRY_SCRIPT], 0, 0, 0);
	if (_ENCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = 2;
		vm.slot[slot].number = 10002;
		vm.slot[slot].type = 1;
		vm.slot[slot].offs = _ENCD_offs + 8;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (_vars[VAR_ENTRY_SCRIPT2])
		runScript(_vars[VAR_ENTRY_SCRIPT2], 0, 0, 0);
}

void Scumm::killScriptsAndResources() {
	ScriptSlot *ss;
	int i;

	ss = &vm.slot[1];
	
	for (i=1; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if (ss->type==1 || ss->type==4) {
			if(ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override in exit", ss->number);
			ss->status = 0;
		} else if (ss->type==3) {
			if(ss->cutsceneOverride)
				error("Script %d stopped with active cutscene/override in exit", ss->number);
			ss->status = 0;
		}
	}
	
	/* Nuke FL objects */
	i = 0;
	do {
		if (_objs[i].fl_object_index)
			nukeResource(0xD, _objs[i].fl_object_index);
	} while (++i <= _numObjectsInRoom);

	/* Nuke local object names */
	if (_newNames) {
		for (i=0; i<50; i++) {
			int j = _newNames[i];
			if (j && (getOwner(j)&0xF) == 0) {
				_newNames[i] = 0;
				nukeResource(rtObjectName, i);
			}
		}
	}
}

void Scumm::checkAndRunVar33() {
	int i;
	ScriptSlot *ss;

	memset(_localParamList, 0, sizeof(_localParamList));
	if (isScriptInUse(_vars[VAR_SENTENCE_SCRIPT])) {
		ss = vm.slot;
		for (i=0; i<NUM_SCRIPT_SLOT; i++,ss++)
			if (ss->number==_vars[VAR_SENTENCE_SCRIPT] && ss->status!=0 && ss->freezeCount==0)
				return;
	}

	if (_sentenceIndex > 0x7F || sentence[_sentenceIndex].unk)
		return;
	
	if (sentence[_sentenceIndex].unk2 && 
		sentence[_sentenceIndex].unk3==sentence[_sentenceIndex].unk4) {
		_sentenceIndex--;
		return;
	}

	_localParamList[0] = sentence[_sentenceIndex].unk5;
	_localParamList[1] = sentence[_sentenceIndex].unk4;
	_localParamList[2] = sentence[_sentenceIndex].unk3;
	_sentenceIndex--;
	_currentScript = 0xFF;
	if (_vars[VAR_SENTENCE_SCRIPT])
		runScript(_vars[VAR_SENTENCE_SCRIPT], 0, 0, _localParamList);
}

void Scumm::runInputScript(int a, int cmd, int mode) {
	memset(_localParamList, 0, sizeof(_localParamList));
	_localParamList[0] = a;
	_localParamList[1] = cmd;
	_localParamList[2] = mode;
	if (_vars[VAR_VERB_SCRIPT])
		runScript(_vars[VAR_VERB_SCRIPT], 0, 0, _localParamList);
}

void Scumm::decreaseScriptDelay(int amount) {
	ScriptSlot *ss = &vm.slot[0];
	int i;
	for (i=0; i<NUM_SCRIPT_SLOT; i++,ss++) {
		if(ss->status==1) {
			ss->delay -= amount;
			if (ss->delay < 0){
				ss->status = 2;
				ss->delay = 0;
			}
		}
	}
}



void Scumm::runVerbCode(int object, int entry, int a, int b, int16 *vars) {
	uint32 obcd;
	int slot, where, offs,i;

	if (!object)
		return;
	if (!b)	
		stopObjectScript(object);

	where = whereIsObject(object);

	if (where == -1) {
		error("Code for object %d not in room %d", object, _roomResource);
	}

	obcd = getOBCDOffs(object);
	slot = getScriptSlot();

	offs = getVerbEntrypoint(object, entry);
	if (offs==0)
		return;

	vm.slot[slot].number = object;
	vm.slot[slot].offs = obcd + offs;
	vm.slot[slot].status = 2;
	vm.slot[slot].type = where;
	vm.slot[slot].unk1 = a;
	vm.slot[slot].unk2 = b;
	vm.slot[slot].freezeCount = 0;
	vm.slot[slot].newfield = 0;

	if (!vars) {
		for(i=0; i<16; i++)
			vm.localvar[slot * 17 + i] = 0;
	} else {
		for (i=0; i<16; i++)
			vm.localvar[slot * 17 + i] = vars[i];
	}

	runScriptNested(slot);
}

int Scumm::getVerbEntrypoint(int obj, int entry) {
	byte *objptr, *verbptr;
	int verboffs;

	if (whereIsObject(obj)==-1)
		return 0;

	objptr = getObjectAddress(obj);

	verbptr = findResource(MKID('VERB'), objptr, 0);
	if (verbptr==NULL)
		error("No verb block in object %d", obj);

	verboffs = verbptr - objptr;

	verbptr += 8;
	do {
		if (!*verbptr)
			return 0;
		if (*verbptr==entry || *verbptr==0xFF)
			break;
		verbptr += 3;
	} while (1);

	return verboffs + READ_LE_UINT16(verbptr+1);
}


void Scumm::push(int a) {
	assert(_scummStackPos >=0 && _scummStackPos < sizeof(_scummStack)-1);
	_scummStack[_scummStackPos++] = a;	
}

int Scumm::pop() {
	assert(_scummStackPos >0 && _scummStackPos < sizeof(_scummStack));
	return _scummStack[--_scummStackPos];
}


void Scumm::endCutscene() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	uint32 *csptr;
	int16 args[16];

	memset(args, 0, sizeof(args));
	
	ss->cutsceneOverride--;

	args[0] = vm.cutSceneData[vm.cutSceneStackPointer];
	_vars[VAR_OVERRIDE] = 0;

	csptr = &vm.cutScenePtr[vm.cutSceneStackPointer];
	if (*csptr)
		ss->cutsceneOverride--;

	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	*csptr = 0;
	vm.cutSceneStackPointer--;

	if (_vars[VAR_CUTSCENE_END_SCRIPT])
		runScript(_vars[VAR_CUTSCENE_END_SCRIPT], 0, 0, args);
}

void Scumm::cutscene(int16 *args) {
	int scr = _currentScript;
	vm.slot[scr].cutsceneOverride++;
	
	if (++vm.cutSceneStackPointer > 5)
		error("Cutscene stack overflow");

	vm.cutSceneData[vm.cutSceneStackPointer] = args[0];
	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;

	vm.cutSceneScriptIndex = scr;
	if (_vars[VAR_CUTSCENE_START_SCRIPT])
		runScript(_vars[VAR_CUTSCENE_START_SCRIPT], 0, 0, args);
	vm.cutSceneScriptIndex = 0xFF;
}

void Scumm::faceActorToObj(int act, int obj) {
	int x,dir;

	if (getObjectOrActorXY(act)==-1)
		return;

	x = _xPos;

	if (getObjectOrActorXY(obj)==-1)
		return;

	dir = (_xPos > x) ? 1 : 0;
	turnToDirection(derefActorSafe(act, "faceActorToObj"), dir);
}

void Scumm::animateActor(int act, int anim) {
	int shr,dir;
	bool inRoom;
	Actor *a;

	a = derefActorSafe(act, "animateActor");

	shr = anim>>2;
	dir = anim&3;

	inRoom = (a->room == _currentRoom);

	if (shr == 0x3F) {
		if (inRoom) {
			startAnimActor(a, a->standFrame, a->facing);
			a->moving = 0;
		}
		return;
	}

	if (shr == 0x3E) {
		if (inRoom) {
			startAnimActor(a, 0x3E, dir);
			a->moving &= ~4;
		}
		a->facing = dir;
		return;
	}

	if (shr == 0x3D) {
		if (inRoom) {
			turnToDirection(a, dir);
		} else {
			a->facing = dir;
		}
		return;
	}

	startAnimActor(a, anim, a->facing);
}

int Scumm::getScriptRunning(int script) {
	int i;
	ScriptSlot *ss = vm.slot;
	for (i=0; i<NUM_SCRIPT_SLOT; i++,ss++)
		if (ss->number==script && (ss->type==2 || ss->type==3) && ss->status)
			return 1;
	return 0;
}

void Scumm::beginOverride() {
	int index;
	uint32 *ptr;

	index = vm.cutSceneStackPointer;
	ptr = &vm.cutScenePtr[index];
	if (!*ptr) {
		vm.slot[_currentScript].cutsceneOverride++;
	}
	*ptr = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[index] = _currentScript;

	fetchScriptByte();
	fetchScriptWord();
	_vars[VAR_OVERRIDE] = 0;
}

void Scumm::endOverride() {
	int index;
	uint32 *ptr;

	index = vm.cutSceneStackPointer;
	ptr = &vm.cutScenePtr[index];
	if (*ptr) {
		vm.slot[_currentScript].cutsceneOverride--;
	}
	*ptr = 0;
	vm.cutSceneScript[index] = 0;
	_vars[VAR_OVERRIDE] = 0;
}


int Scumm::defineArray(int array, int type, int dim2, int dim1) {
	int id;
	int size;
	ArrayHeader *ah;

	if (type!=5 && type!=4)
		type=5;

	nukeArray(array);

	id = getArrayId();

	if (array&0x4000) {
		_arrays[id] = vm.slot[_currentScript].number;
	}

	if (array&0x8000) {
		error("Can't define bit variable as array pointer");
	}

	writeVar(array, id);

	size = (type==5) ? 16 : 8;
	size *= dim2+1;
	size *= dim1+1;
	size >>= 3;

	ah = (ArrayHeader*)createResource(rtString, id, size+sizeof(ArrayHeader));

	ah->type = type;
	ah->dim1_size = dim1+1;
	ah->dim2_size = dim2+1;

	return id;
}

void Scumm::nukeArray(int a) {
	int data;

	data = readVar(a);

	if (data)
		nukeResource(rtString, data);
	_arrays[data] = 0;

	writeVar(a, 0);
}

int Scumm::getArrayId() {
	byte **addr = _baseArrays;
	int i;

	for (i=1; i<_numArray; i++) {
		if (!addr[i])
			return i;
	}
	error("Out of array pointers, %d max", _numArray);
}

void Scumm::arrayop_1(int a, byte *ptr) {
	ArrayHeader *ah;
	int r;
	int len = getStringLen(ptr);
			
	r = defineArray(a, 4, 0, len);
	ah = (ArrayHeader*)getResourceAddress(rtString,r);
	copyString(ah->data,ptr,len);
}

void Scumm::copyString(byte *dst, byte *src, int len) {
	if (!src) {
		while (--len>=0)
			*dst++ = fetchScriptByte();
	} else {
		while (--len>=0)
			*dst++ = *src++;
	}
}

int Scumm::getStringLen(byte *ptr) {
	int len;
	byte c;
	if (!ptr)
		ptr = _scriptPointer;
	len = 0;
	do {
		c = *ptr++;
		if (!c) break;
		len++;
		if (c==0xFF)
			ptr += 3, len += 3;
	} while (1);
	return len+1;
}
