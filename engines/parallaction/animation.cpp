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

#include "common/stdafx.h"

#include "parallaction/parallaction.h"


namespace Parallaction {


#define INST_ON 						1
#define INST_OFF						2
#define INST_X							3
#define INST_Y							4
#define INST_Z							5
#define INST_F							6
#define INST_LOOP						7
#define INST_ENDLOOP					8
#define INST_SHOW						9
#define INST_INC						10
#define INST_DEC						11
#define INST_SET						12
#define INST_PUT						13
#define INST_CALL						14
#define INST_WAIT						15
#define INST_START						16
#define INST_SOUND						17
#define INST_MOVE						18
#define INST_END						19


void	wrapLocalVar(LocalVariable *local);


#define NUM_LOCALS	10

uint16	_numLocals = 0;
char	_localNames[NUM_LOCALS][10];

Animation *Parallaction::findAnimation(const char *name) {

	for (AnimationList::iterator it = _animations.begin(); it != _animations.end(); it++)
		if (!scumm_stricmp((*it)->_label._text, name)) return *it;

	return NULL;
}


Animation *Parallaction::parseAnimation(Script& script, AnimationList &list, char *name) {
//	printf("parseAnimation(%s)\n", name);

	Animation *vD0 = new Animation;

	vD0->_label._text = (char*)malloc(strlen(name)+1);
	strcpy(vD0->_label._text, name);

	list.push_front(vD0);

	fillBuffers(script, true);
	while (scumm_stricmp(_tokens[0], "endanimation")) {
//		printf("token[0] = %s\n", _tokens[0]);

		if (!scumm_stricmp(_tokens[0], "script")) {
			loadProgram(vD0, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "commands")) {
			 parseCommands(script, vD0->_commands);
		}
		if (!scumm_stricmp(_tokens[0], "type")) {
			if (_tokens[2][0] != '\0') {
				vD0->_type = ((4 + _objectsNames->lookup(_tokens[2])) << 16) & 0xFFFF0000;
			}
			int16 _si = _zoneTypeNames->lookup(_tokens[1]);
			if (_si != -1) {
				vD0->_type |= 1 << (_si-1);
				if (((vD0->_type & 0xFFFF) != kZoneNone) && ((vD0->_type & 0xFFFF) != kZoneCommand)) {
					parseZoneTypeBlock(script, vD0);
				}
			}
		}
		if (!scumm_stricmp(_tokens[0], "label")) {
			renderLabel(&vD0->_label._cnv, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "flags")) {
			uint16 _si = 1;

			do {
				byte _al = _zoneFlagNames->lookup(_tokens[_si]);
				_si++;
				vD0->_flags |= 1 << (_al - 1);
			} while (!scumm_stricmp(_tokens[_si++], "|"));
		}
		if (!scumm_stricmp(_tokens[0], "file")) {
			char vC8[200];
			strcpy(vC8, _tokens[1]);
			if (_engineFlags & kEngineTransformedDonna) {
				if (!scumm_stricmp(_tokens[1], "donnap") || !scumm_stricmp(_tokens[1], "donnapa")) {
					strcat(vC8, "tras");
				}
			}
			vD0->_cnv = _disk->loadFrames(vC8);
		}
		if (!scumm_stricmp(_tokens[0], "position")) {
			vD0->_left = atoi(_tokens[1]);
			vD0->_top = atoi(_tokens[2]);
			vD0->_z = atoi(_tokens[3]);
		}
		if (!scumm_stricmp(_tokens[0], "moveto")) {
			vD0->_moveTo.x = atoi(_tokens[1]);
			vD0->_moveTo.y = atoi(_tokens[2]);
		}

		fillBuffers(script, true);
	}

	vD0->_oldPos.x = -1000;
	vD0->_oldPos.y = -1000;

	vD0->_flags |= 0x1000000;

	return vD0;
}


void Parallaction::freeAnimations() {
	_animations.clear();
	return;
}



void jobDisplayAnimations(void *parm, Job *j) {
//	printf("jobDisplayAnimations()...\n");

	Graphics::Surface v14;

	uint16 _si = 0;

	for (AnimationList::iterator it = _vm->_animations.begin(); it != _vm->_animations.end(); it++) {

		Animation *v18 = *it;

		if ((v18->_flags & kFlagsActive) && ((v18->_flags & kFlagsRemove) == 0))   {
			v14.w = v18->width();
			v14.h = v18->height();

			int16 frame = CLIP((int)v18->_frame, 0, v18->getFrameNum()-1);

			v14.pixels = v18->getFrameData(frame);

			if (v18->_flags & kFlagsNoMasked)
				_si = 3;
			else
				_si = _vm->_gfx->queryMask(v18->_top + v18->height());

			debugC(9, kDebugLocation, "jobDisplayAnimations(%s, x:%i, y:%i, z:%i, w:%i, h:%i, f:%i/%i, %p)", v18->_label._text, v18->_left, v18->_top, _si, v14.w, v14.h,
				frame, v18->getFrameNum(), v14.pixels);
			_vm->_gfx->blitCnv(&v14, v18->_left, v18->_top, _si, Gfx::kBitBack);

		}

		if (((v18->_flags & kFlagsActive) == 0) && (v18->_flags & kFlagsRemove))   {
			v18->_flags &= ~kFlagsRemove;
			v18->_oldPos.x = -1000;
		}

		if ((v18->_flags & kFlagsActive) && (v18->_flags & kFlagsRemove))	{
			v18->_flags &= ~kFlagsActive;
			v18->_flags |= kFlagsRemove;
		}

	}

//	  printf("done\n");

	return;
}


void jobEraseAnimations(void *arg_0, Job *j) {
	debugC(3, kDebugJobs, "jobEraseAnimations");

	for (AnimationList::iterator it = _vm->_animations.begin(); it != _vm->_animations.end(); it++) {

		Animation *a = *it;

		if (((a->_flags & kFlagsActive) == 0) && ((a->_flags & kFlagsRemove) == 0)) continue;

		Common::Rect r(a->width(), a->height());
		r.moveTo(a->_oldPos);
		_vm->_gfx->restoreBackground(r);

		if (arg_0) {
			a->_oldPos.x = a->_left;
			a->_oldPos.y = a->_top;
		}

	}

//	printf("done\n");

	return;
}


void Parallaction::loadProgram(Animation *a, char *filename) {
//	printf("loadProgram(%s)\n", filename);

	Script *script = _disk->loadScript(filename);

	_numLocals = 0;

	fillBuffers(*script);

	a->_program = new Program;

	Instruction *vCC = new Instruction;

	while (scumm_stricmp(_tokens[0], "endscript")) {
		parseScriptLine(vCC, a, a->_program->_locals);
		a->_program->_instructions.push_back(vCC);
		vCC = new Instruction;
		fillBuffers(*script);
	}

	// TODO: use List<>::end() to detect the end of the program
	vCC->_index = INST_END;
	a->_program->_instructions.push_back(vCC);
	a->_program->_ip = a->_program->_instructions.begin();

	delete script;

	return;
}

int16 findLocal(const char* name, LocalVariable *locals) {
	for (uint16 _si = 0; _si < NUM_LOCALS; _si++) {
		if (!scumm_stricmp(name, _localNames[_si]))
			return _si;
	}

	return -1;
}

int16 addLocal(const char *name, LocalVariable *locals, int16 value = 0, int16 min = -10000, int16 max = 10000) {
	assert(_numLocals < NUM_LOCALS);

	strcpy(_localNames[_numLocals], name);
	locals[_numLocals]._value = value;

	locals[_numLocals]._min = min;
	locals[_numLocals]._max = max;

	return _numLocals++;
}



void Parallaction::parseScriptLine(Instruction *inst, Animation *a, LocalVariable *locals) {
//	printf("parseScriptLine()\n");

	if (_tokens[0][1] == '.') {
		_tokens[0][1] = '\0';
		a = findAnimation(&_tokens[0][2]);
	}

	if (_tokens[1][1] == '.') {
		_tokens[1][1] = '\0';
		a = findAnimation(&_tokens[1][2]);
	}

	int16 _si = _instructionNames->lookup(_tokens[0]);
	inst->_index = _si;

//	printf("token[0] = %s (%i)\n", _tokens[0], inst->_index);

	switch (inst->_index) {
	case INST_ON:	// on
	case INST_OFF:	// off
	case INST_START:	// start
		if (!scumm_stricmp(_tokens[1], a->_label._text)) {
			inst->_opBase._a = a;
		} else {
			inst->_opBase._a = findAnimation(_tokens[1]);
		}
		break;

	case INST_LOOP: // loop
		inst->_opBase._loopCounter = getLValue(inst, _tokens[1], locals, a);
		break;

	case INST_X:	// x
		inst->_opA._pvalue = &a->_left;
		inst->_opB = getLValue(inst, _tokens[1], locals, a);
		break;

	case INST_Y:	// y
		inst->_opA._pvalue = &a->_top;
		inst->_opB = getLValue(inst, _tokens[1], locals, a);
		break;

	case INST_Z:	// z
		inst->_opA._pvalue = &a->_z;
		inst->_opB = getLValue(inst, _tokens[1], locals, a);
		break;

	case INST_F:	// f
		inst->_opA._pvalue = &a->_frame;
		inst->_opB = getLValue(inst, _tokens[1], locals, a);
		break;

	case INST_INC:	// inc
	case INST_DEC:	// dec
		if (!scumm_stricmp(_tokens[1], "X")) {
			inst->_opA._pvalue = &a->_left;
		} else
		if (!scumm_stricmp(_tokens[1], "Y")) {
			inst->_opA._pvalue = &a->_top;
		} else
		if (!scumm_stricmp(_tokens[1], "Z")) {
			inst->_opA._pvalue = &a->_z;
		} else
		if (!scumm_stricmp(_tokens[1], "F")) {
			inst->_opA._pvalue = &a->_frame;
		} else {
			inst->_flags |= kInstUsesLocal;
			inst->_opA = getLValue(inst, _tokens[1], locals, a);
		}

		inst->_opB = getLValue(inst, _tokens[2], locals, a);

		if (!scumm_stricmp(_tokens[3], "mod")) {
			inst->_flags |= kInstMod;
		}
		break;

	case INST_SET:	// set
		// WORKAROUND: At least one script (balzo.script) in Amiga versions didn't declare
		//	local variables before using them, thus leading to crashes. The line launching the
		// script was commented out on Dos version. This workaround enables the engine
		// to dynamically add a local variable when it is encountered the first time in
		// the script, so should fix any other occurrence as well.
		if (findLocal(_tokens[1], locals) == -1) {
			addLocal(_tokens[1], locals);
		}

		inst->_opA = getLValue(inst, _tokens[1], locals, a);
		inst->_flags |= kInstUsesLocal;
		inst->_opB = getLValue(inst, _tokens[2], locals, a);
		break;

	case INST_MOVE: // move
		inst->_opA = getLValue(inst, _tokens[1], locals, a);
		inst->_opB = getLValue(inst, _tokens[2], locals, a);
		break;

	case INST_PUT:	// put
		if (!scumm_stricmp(_tokens[1], a->_label._text)) {
			inst->_opBase._a = a;
		} else {
			inst->_opBase._a = findAnimation(_tokens[1]);
		}

		inst->_opA = getLValue(inst, _tokens[2], locals, a);
		inst->_opB = getLValue(inst, _tokens[3], locals, a);
		if (!scumm_stricmp(_tokens[4], "masked")) {
			inst->_flags |= kInstMaskedPut;
		}
		break;

	case INST_CALL: {	// call
		int16 _ax = _callableNames->lookup(_tokens[1]);
		inst->_opBase._index = _ax - 1;
		if (_ax - 1 < 0) exit(0);
	}
	break;

	case INST_SOUND:	// sound
		inst->_opBase._z = findZone(_tokens[1]);
		break;

	case INST_ENDLOOP:	// endloop
	case INST_SHOW: // show
	case INST_WAIT: // wait
		break;

	default: {	// local definition
		int16 val = atoi(_tokens[2]);
		int16 index;

		if (_tokens[3][0] != '\0') {
			index = addLocal(_tokens[0], locals, val, atoi(_tokens[3]), atoi(_tokens[4]));
		} else {
			index = addLocal(_tokens[0], locals, val);
		}

		inst->_opA._local = &locals[index];
		inst->_opB._value = locals[index]._value;

		inst->_flags = kInstUsesLiteral | kInstUsesLocal;
		inst->_index = INST_SET;
	}
	break;

	}


	return;
}

LValue Parallaction::getLValue(Instruction *inst, char *str, LocalVariable *locals, Animation *a) {

	LValue v;

	v._pvalue = 0;	// should stop compiler from complaining

	if (isdigit(str[0]) || str[0] == '-') {
		inst->_flags |= kInstUsesLiteral;
		v._value = atoi(str);
		return v;
	}

	int index = findLocal(str, locals);
	if (index != -1) {
		v._local = &locals[index];
		return v;
	}

	if (str[1] == '.') {
		a = findAnimation(&str[2]);
	}

	if (str[0] == 'X') {
		v._pvalue = &a->_left;
	} else
	if (str[0] == 'Y') {
		v._pvalue = &a->_top;
	} else
	if (str[0] == 'Z') {
		v._pvalue = &a->_z;
	} else
	if (str[0] == 'F') {
		v._pvalue = &a->_frame;
	}

	return v;
}


DECLARE_INSTRUCTION_OPCODE(on) {
	(*_instRunCtxt.inst)->_opBase._a->_flags |= kFlagsActive;
	(*_instRunCtxt.inst)->_opBase._a->_flags &= ~kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(off) {
	(*_instRunCtxt.inst)->_opBase._a->_flags |= kFlagsRemove;
//				v1C = (*_instRunCtxt.inst)->_opBase;
}


DECLARE_INSTRUCTION_OPCODE(loop) {
	if ((*_instRunCtxt.inst)->_flags & kInstUsesLiteral) {
		_instRunCtxt.a->_program->_loopCounter = (*_instRunCtxt.inst)->_opBase._loopCounter._value;
	} else {
		_instRunCtxt.a->_program->_loopCounter = *(*_instRunCtxt.inst)->_opBase._loopCounter._pvalue;
	}
	_instRunCtxt.a->_program->_loopStart = _instRunCtxt.inst;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--_instRunCtxt.a->_program->_loopCounter > 0) {
		_instRunCtxt.inst = _instRunCtxt.a->_program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	int16 _si = 0;
	int16 _ax = 0, _bx = 0;
	if ((*_instRunCtxt.inst)->_flags & kInstUsesLiteral) {
		_si = (*_instRunCtxt.inst)->_opB._value;
	} else {
		_si = *(*_instRunCtxt.inst)->_opB._pvalue;
	}
	if ((*_instRunCtxt.inst)->_flags & kInstMod) {	// mod
		_bx = (_si > 0 ? _si : -_si);
		if (_instRunCtxt.modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}
	if ((*_instRunCtxt.inst)->_flags & kInstUsesLocal) {	// local
		if ((*_instRunCtxt.inst)->_index == INST_INC) _ax = _si;
		else _ax = -_si;

		(*_instRunCtxt.inst)->_opA._local->_value += _ax;
		wrapLocalVar((*_instRunCtxt.inst)->_opA._local);
		return;
	}

	// built-in variable (x, y, z, f)
	if ((*_instRunCtxt.inst)->_index == INST_INC) _ax = _si;
	else _ax = -_si;
	*(*_instRunCtxt.inst)->_opA._pvalue += _ax;
}


DECLARE_INSTRUCTION_OPCODE(set) {
	int16 _si;
	if ((*_instRunCtxt.inst)->_flags & kInstUsesLiteral) {
		_si = (*_instRunCtxt.inst)->_opB._value;
	} else {
		_si = *(*_instRunCtxt.inst)->_opB._pvalue;
	}

	if ((*_instRunCtxt.inst)->_flags & kInstUsesLocal) {
		(*_instRunCtxt.inst)->_opA._local->_value = _si;
	} else {
		*(*_instRunCtxt.inst)->_opA._pvalue = _si;
	}
}


DECLARE_INSTRUCTION_OPCODE(put) {
	Graphics::Surface v18;
	v18.w = (*_instRunCtxt.inst)->_opBase._a->width();
	v18.h = (*_instRunCtxt.inst)->_opBase._a->height();
	v18.pixels = (*_instRunCtxt.inst)->_opBase._a->getFrameData((*_instRunCtxt.inst)->_opBase._a->_frame);

	if ((*_instRunCtxt.inst)->_flags & kInstMaskedPut) {
		uint16 _si = _gfx->queryMask((*_instRunCtxt.inst)->_opB._value);
		_gfx->blitCnv(&v18, (*_instRunCtxt.inst)->_opA._value, (*_instRunCtxt.inst)->_opB._value, _si, Gfx::kBitBack);
		_gfx->blitCnv(&v18, (*_instRunCtxt.inst)->_opA._value, (*_instRunCtxt.inst)->_opB._value, _si, Gfx::kBit2);
	} else {
		_gfx->flatBlitCnv(&v18, (*_instRunCtxt.inst)->_opA._value, (*_instRunCtxt.inst)->_opB._value, Gfx::kBitBack);
		_gfx->flatBlitCnv(&v18, (*_instRunCtxt.inst)->_opA._value, (*_instRunCtxt.inst)->_opB._value, Gfx::kBit2);
	}
}

DECLARE_INSTRUCTION_OPCODE(null) {

}

DECLARE_INSTRUCTION_OPCODE(call) {
	callFunction((*_instRunCtxt.inst)->_opBase._index, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (_engineFlags & kEngineWalking)
		_instRunCtxt.suspend = true;
}


DECLARE_INSTRUCTION_OPCODE(start) {
//				v1C = (*_instRunCtxt.inst)->_opBase;
	(*_instRunCtxt.inst)->_opBase._a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_activeZone = (*_instRunCtxt.inst)->_opBase._z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	WalkNodeList *v4 = _char._builder.buildPath(*(*_instRunCtxt.inst)->_opA._pvalue, *(*_instRunCtxt.inst)->_opB._pvalue);
	addJob(&jobWalk, v4, kPriority19 );
	_engineFlags |= kEngineWalking;
}

DECLARE_INSTRUCTION_OPCODE(end) {
	if ((_instRunCtxt.a->_flags & kFlagsLooping) == 0) {
		_instRunCtxt.a->_flags &= ~kFlagsActing;
		runCommands(_instRunCtxt.a->_commands, _instRunCtxt.a);
	}
	_instRunCtxt.a->_program->_ip = _instRunCtxt.a->_program->_instructions.begin();

	_instRunCtxt.suspend = true;
}



void jobRunScripts(void *parm, Job *j) {
	debugC(3, kDebugJobs, "jobRunScripts");

	static uint16 modCounter = 0;

	static const Parallaction::Opcode opcodes[] = {
		INSTRUCTION_OPCODE(on),
		INSTRUCTION_OPCODE(off),
		INSTRUCTION_OPCODE(set),		// x
		INSTRUCTION_OPCODE(set),		// y
		INSTRUCTION_OPCODE(set),		// z
		INSTRUCTION_OPCODE(set),		// f
		INSTRUCTION_OPCODE(loop),
		INSTRUCTION_OPCODE(endloop),
		INSTRUCTION_OPCODE(null),
		INSTRUCTION_OPCODE(inc),
		INSTRUCTION_OPCODE(inc),		// dec
		INSTRUCTION_OPCODE(set),
		INSTRUCTION_OPCODE(put),
		INSTRUCTION_OPCODE(call),
		INSTRUCTION_OPCODE(wait),
		INSTRUCTION_OPCODE(start),
		INSTRUCTION_OPCODE(sound),
		INSTRUCTION_OPCODE(move),
		INSTRUCTION_OPCODE(end)
	};

	_vm->_instructionOpcodes = opcodes;

	for (AnimationList::iterator it = _vm->_animations.begin(); it != _vm->_animations.end(); it++) {

		Animation *a = *it;

		if (a->_flags & kFlagsCharacter) a->_z = a->_top + a->height();

		if ((a->_flags & kFlagsActing) == 0) continue;
		InstructionList::iterator inst = a->_program->_ip;

		while (((*inst)->_index != INST_SHOW) && (a->_flags & kFlagsActing)) {

			debugC(9, kDebugJobs, "Animation: %s, instruction: %s", a->_label._text, (*inst)->_index == INST_END ? "end" : _vm->_instructionNamesRes[(*inst)->_index - 1]);

			_vm->_instRunCtxt.inst = inst;
			_vm->_instRunCtxt.a = a;
			_vm->_instRunCtxt.modCounter = modCounter;
			_vm->_instRunCtxt.suspend = false;

			(_vm->*(_vm->_instructionOpcodes)[(*inst)->_index - 1])();

			inst = _vm->_instRunCtxt.inst;		// handles endloop correctly

			if (_vm->_instRunCtxt.suspend)
				goto label1;

			inst++;
		}

		a->_program->_ip = ++inst;

label1:
		if (a->_flags & kFlagsCharacter)
			a->_z = a->_top + a->height();
	}

	_vm->sortAnimations();
	modCounter++;

	return;
}

void wrapLocalVar(LocalVariable *local) {
//	  printf("wrapLocalVar(v: %i, min: %i, max: %i)\n", local->_value, local->_min, local->_max);

	if (local->_value >= local->_max)
		local->_value = local->_min;
	if (local->_value < local->_min)
		local->_value = local->_max - 1;

	return;
}

int compareAnimationZ(const AnimationPointer &a1, const AnimationPointer &a2) {
	if (a1->_z == a2->_z) return 0;
	return (a1->_z < a2->_z ? -1 : 1);
}


void Parallaction::sortAnimations() {
	_char._ani._z = _char._ani.height() + _char._ani._top;
	_animations.sort(compareAnimationZ);
	return;
}

Animation::Animation() {
	_cnv = NULL;
	_program = NULL;
	_frame = 0;
	_z = 0;
}

Animation::~Animation() {
	if (_program)
		delete _program;

	if (_cnv)
		delete _cnv;
}

uint16 Animation::width() const {
	if (!_cnv) return 0;
	return _cnv->_width;
}

uint16 Animation::height() const {
	if (!_cnv) return 0;
	return _cnv->_height;
}

uint16 Animation::getFrameNum() const {
	if (!_cnv) return 0;
	return _cnv->_count;
}

byte* Animation::getFrameData(uint32 index) const {
	if (!_cnv) return NULL;
	return _cnv->getFramePtr(index);
}


Program::Program() {
	_loopCounter = 0;
	_locals = new LocalVariable[10];
}

Program::~Program() {
	delete[] _locals;
}


} // namespace Parallaction
