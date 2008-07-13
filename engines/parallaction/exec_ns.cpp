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

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {

#define INST_ON							1
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
#define INST_ENDSCRIPT					19

#define SetOpcodeTable(x) table = &x;

typedef Common::Functor0Mem<void, CommandExec_ns> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_ns::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_ns::cmdOp_##op()

typedef Common::Functor0Mem<void, ProgramExec_ns> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_ns::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_ns::instOp_##op()




DECLARE_INSTRUCTION_OPCODE(on) {
	InstructionPtr inst = *_ctxt.inst;

	inst->_a->_flags |= kFlagsActive;
	inst->_a->_flags &= ~kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(off) {
	(*_ctxt.inst)->_a->_flags |= kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(loop) {
	InstructionPtr inst = *_ctxt.inst;

	_ctxt.program->_loopCounter = inst->_opB.getRValue();
	_ctxt.program->_loopStart = _ctxt.inst;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--_ctxt.program->_loopCounter > 0) {
		_ctxt.inst = _ctxt.program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = *_ctxt.inst;
	int16 _si = inst->_opB.getRValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (_modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16* lvalue = inst->_opA.getLValue();

	if (inst->_index == INST_INC) {
		*lvalue += _si;
	} else {
		*lvalue -= _si;
	}

	if (inst->_opA._flags & kParaLocal) {
		inst->_opA._local->wrap();
	}

}


DECLARE_INSTRUCTION_OPCODE(set) {
	InstructionPtr inst = *_ctxt.inst;

	int16 _si = inst->_opB.getRValue();
	int16 *lvalue = inst->_opA.getLValue();

	*lvalue = _si;

}


DECLARE_INSTRUCTION_OPCODE(put) {
	InstructionPtr inst = *_ctxt.inst;
	Graphics::Surface v18;
	v18.w = inst->_a->width();
	v18.h = inst->_a->height();
	v18.pixels = inst->_a->getFrameData(inst->_a->_frame);

	int16 x = inst->_opA.getRValue();
	int16 y = inst->_opB.getRValue();
	bool mask = (inst->_flags & kInstMaskedPut) == kInstMaskedPut;

	_vm->_gfx->patchBackground(v18, x, y, mask);
}

DECLARE_INSTRUCTION_OPCODE(null) {

}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", (*_ctxt.inst)->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction((*_ctxt.inst)->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (_engineFlags & kEngineWalking)
		_ctxt.suspend = true;
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_ctxt.inst)->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_vm->_activeZone = (*_ctxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	InstructionPtr inst = (*_ctxt.inst);

	int16 x = inst->_opA.getRValue();
	int16 y = inst->_opB.getRValue();

	_vm->_char.scheduleWalk(x, y);
}

DECLARE_INSTRUCTION_OPCODE(endscript) {
	if ((_ctxt.anim->_flags & kFlagsLooping) == 0) {
		_ctxt.anim->_flags &= ~kFlagsActing;
		_vm->_cmdExec->run(_ctxt.anim->_commands, _ctxt.anim);
		_ctxt.program->_status = kProgramDone;
	}
	_ctxt.program->_ip = _ctxt.program->_instructions.begin();

	_ctxt.suspend = true;
}




DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", _ctxt.cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags |= _ctxt.cmd->u._flags;
	} else {
		_vm->setLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags &= ~_ctxt.cmd->u._flags;
	} else {
		_vm->clearLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(start) {
	_ctxt.cmd->u._zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	_vm->_activeZone = _ctxt.cmd->u._zone;
}


DECLARE_COMMAND_OPCODE(get) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsFixed;
	_vm->runZone(_ctxt.cmd->u._zone);
}


DECLARE_COMMAND_OPCODE(location) {
	_vm->scheduleLocationSwitch(_ctxt.cmd->u._string);
}


DECLARE_COMMAND_OPCODE(open) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsClosed;
	if (_ctxt.cmd->u._zone->u.door->gfxobj) {
		_vm->updateDoor(_ctxt.cmd->u._zone);
	}
}


DECLARE_COMMAND_OPCODE(close) {
	_ctxt.cmd->u._zone->_flags |= kFlagsClosed;
	if (_ctxt.cmd->u._zone->u.door->gfxobj) {
		_vm->updateDoor(_ctxt.cmd->u._zone);
	}
}

void CommandExec_ns::updateGetZone(ZonePtr z, bool visible) {
	if (!z) {
		return;
	}

	if ((z->_type & 0xFFFF) == kZoneGet) {
		_vm->_gfx->showGfxObj(z->u.get->gfxobj, visible);
	}
}

DECLARE_COMMAND_OPCODE(on) {
	ZonePtr z = _ctxt.cmd->u._zone;

	if (z) {
		z->_flags &= ~kFlagsRemove;
		z->_flags |= kFlagsActive;
		updateGetZone(z, true);
	}
}


DECLARE_COMMAND_OPCODE(off) {
	ZonePtr z = _ctxt.cmd->u._zone;

	if (z) {
		_ctxt.cmd->u._zone->_flags |= kFlagsRemove;
		updateGetZone(z, false);
	}
}


DECLARE_COMMAND_OPCODE(call) {
	_vm->callFunction(_ctxt.cmd->u._callable, &_ctxt.z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags ^= _ctxt.cmd->u._flags;
	} else {
		_vm->toggleLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(drop){
	_vm->dropItem( _ctxt.cmd->u._object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_engineFlags |= kEngineQuit;
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->_char.scheduleWalk(_ctxt.cmd->u._move.x, _ctxt.cmd->u._move.y);
}


DECLARE_COMMAND_OPCODE(stop) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsActing;
}


void Parallaction_ns::drawAnimations() {

	uint16 layer = 0;

	for (AnimationList::iterator it = _location._animations.begin(); it != _location._animations.end(); it++) {

		AnimationPtr v18 = *it;
		GfxObj *obj = v18->gfxobj;

		if ((v18->_flags & kFlagsActive) && ((v18->_flags & kFlagsRemove) == 0))   {

			int16 frame = CLIP((int)v18->_frame, 0, v18->getFrameNum()-1);
			if (v18->_flags & kFlagsNoMasked)
				layer = 3;
			else
				layer = _gfx->_backgroundInfo.getLayer(v18->_top + v18->height());

			if (obj) {
				_gfx->showGfxObj(obj, true);
				obj->frame = frame;
				obj->x = v18->_left;
				obj->y = v18->_top;
				obj->z = v18->_z;
				obj->layer = layer;
			}
		}

		if (((v18->_flags & kFlagsActive) == 0) && (v18->_flags & kFlagsRemove))   {
			v18->_flags &= ~kFlagsRemove;
			v18->_oldPos.x = -1000;
		}

		if ((v18->_flags & kFlagsActive) && (v18->_flags & kFlagsRemove))	{
			v18->_flags &= ~kFlagsActive;
			v18->_flags |= kFlagsRemove;
			if (obj) {
				_gfx->showGfxObj(obj, false);
			}
		}
	}

	return;
}


void ProgramExec::runScripts(ProgramList::iterator first, ProgramList::iterator last) {
	if (_engineFlags & kEnginePauseJobs) {
		return;
	}

	debugC(9, kDebugExec, "runScripts");

	for (ProgramList::iterator it = first; it != last; it++) {

		AnimationPtr a = (*it)->_anim;

		if (a->_flags & kFlagsCharacter)
			a->_z = a->_top + a->height();

		if ((a->_flags & kFlagsActing) == 0)
			continue;

		InstructionList::iterator inst = (*it)->_ip;
		while (((*inst)->_index != INST_SHOW) && (a->_flags & kFlagsActing)) {

			(*it)->_status = kProgramRunning;

			debugC(9, kDebugExec, "Animation: %s, instruction: %i", a->_name, (*inst)->_index); //_instructionNamesRes[(*inst)->_index - 1]);

			_ctxt.inst = inst;
			_ctxt.anim = AnimationPtr(a);
			_ctxt.program = *it;
			_ctxt.suspend = false;

			(*_opcodes[(*inst)->_index])();

			inst = _ctxt.inst;		// handles endloop correctly

			if (_ctxt.suspend)
				goto label1;

			inst++;
		}

		(*it)->_ip = ++inst;

label1:
		if (a->_flags & kFlagsCharacter)
			a->_z = a->_top + a->height();
	}

	_modCounter++;

	return;
}


void CommandExec::run(CommandList& list, ZonePtr z) {
	if (list.size() == 0) {
		debugC(3, kDebugExec, "runCommands: nothing to do");
		return;
	}

	debugC(3, kDebugExec, "runCommands starting");

	uint32 useFlags = 0;
	bool useLocalFlags;

	CommandList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {
		if (_engineFlags & kEngineQuit)
			break;

		CommandPtr cmd = *it;

		if (cmd->_flagsOn & kFlagsGlobal) {
			useFlags = _commandFlags | kFlagsGlobal;
			useLocalFlags = false;
		} else {
			useFlags = _vm->getLocationFlags();
			useLocalFlags = true;
		}

		bool onMatch = (cmd->_flagsOn & useFlags) == cmd->_flagsOn;
		bool offMatch = (cmd->_flagsOff & ~useFlags) == cmd->_flagsOff;

		debugC(3, kDebugExec, "runCommands[%i] (on: %x, off: %x), (%s = %x)", cmd->_id,  cmd->_flagsOn, cmd->_flagsOff,
			useLocalFlags ? "LOCALFLAGS" : "GLOBALFLAGS", useFlags);

		if (!onMatch || !offMatch) continue;

		_ctxt.z = z;
		_ctxt.cmd = cmd;

		(*_opcodes[cmd->_id])();
	}

	debugC(3, kDebugExec, "runCommands completed");

	return;

}

CommandExec_ns::CommandExec_ns(Parallaction_ns* vm) : _vm(vm) {

}

CommandExec_ns::~CommandExec_ns() {

}

//
//	ZONE TYPE: EXAMINE
//

void Parallaction::displayComment(ExamineData *data) {
	if (!data->_description) {
		return;
	}

	int id;

	if (data->_filename) {
		if (data->_cnv == 0) {
			data->_cnv = _disk->loadStatic(data->_filename);
		}

		_gfx->setHalfbriteMode(true);
		_balloonMan->setSingleBalloon(data->_description, 0, 90, 0, 0);
		Common::Rect r;
		data->_cnv->getRect(0, r);
		id = _gfx->setItem(data->_cnv, 140, (_screenHeight - r.height())/2);
		_gfx->setItemFrame(id, 0);
		id = _gfx->setItem(_char._head, 100, 152);
		_gfx->setItemFrame(id, 0);
	} else {
		_balloonMan->setSingleBalloon(data->_description, 140, 10, 0, 0);
		id = _gfx->setItem(_char._talk, 190, 80);
		_gfx->setItemFrame(id, 0);
	}

	_input->_inputMode = Input::kInputModeComment;
}



uint16 Parallaction::runZone(ZonePtr z) {
	debugC(3, kDebugExec, "runZone (%s)", z->_name);

	uint16 subtype = z->_type & 0xFFFF;

	debugC(3, kDebugExec, "type = %x, object = %x", subtype, (z->_type & 0xFFFF0000) >> 16);
	switch(subtype) {

	case kZoneExamine:
		displayComment(z->u.examine);
		break;

	case kZoneGet:
		if (z->_flags & kFlagsFixed) break;
		if (pickupItem(z) != 0) {
			return 1;
		}
		z->_flags |= kFlagsRemove;
		break;

	case kZoneDoor:
		if (z->_flags & kFlagsLocked) break;
		z->_flags ^= kFlagsClosed;
		updateDoor(z);
		break;

	case kZoneHear:
		_soundMan->playSfx(z->u.hear->_name, z->u.hear->_channel, (z->_flags & kFlagsLooping) == kFlagsLooping, 60);
		break;

	case kZoneSpeak:
		runDialogue(z->u.speak);
		if (_engineFlags & kEngineQuit)
			return 0;
		break;

	}

	debugC(3, kDebugExec, "runZone completed");

	_cmdExec->run(z->_commands, z);

	return 0;
}

//
//	ZONE TYPE: DOOR
//
void Parallaction::updateDoor(ZonePtr z) {

	if (z->u.door->gfxobj) {
		uint frame = (z->_flags & kFlagsClosed ? 0 : 1);
//		z->u.door->gfxobj->setFrame(frame);
		z->u.door->gfxobj->frame = frame;
	}

	return;
}



//
//	ZONE TYPE: GET
//

int16 Parallaction::pickupItem(ZonePtr z) {
	int r = addInventoryItem(z->u.get->_icon);
	if (r != -1) {
		_gfx->showGfxObj(z->u.get->gfxobj, false);
	}

	return (r == -1);
}



ZonePtr Parallaction::hitZone(uint32 type, uint16 x, uint16 y) {
//	printf("hitZone(%i, %i, %i)", type, x, y);

	uint16 _di = y;
	uint16 _si = x;

	for (ZoneList::iterator it = _location._zones.begin(); it != _location._zones.end(); it++) {
//		printf("Zone name: %s", z->_name);

		ZonePtr z = *it;

		if (z->_flags & kFlagsRemove) continue;

		Common::Rect r;
		z->getRect(r);
		r.right++;		// adjust border because Common::Rect doesn't include bottom-right edge
		r.bottom++;

		r.grow(-1);		// allows some tolerance for mouse click

		if (!r.contains(_si, _di)) {

			// out of Zone, so look for special values
			if ((z->_left == -2) || (z->_left == -3)) {

				// WORKAROUND: this huge condition is needed because we made TypeData a collection of structs
				// instead of an union. So, merge->_obj1 and get->_icon were just aliases in the original engine,
				// but we need to check it separately here. The same workaround is applied in freeZones.
				if ((((z->_type & 0xFFFF) == kZoneMerge) && (((_si == z->u.merge->_obj1) && (_di == z->u.merge->_obj2)) || ((_si == z->u.merge->_obj2) && (_di == z->u.merge->_obj1)))) ||
					(((z->_type & 0xFFFF) == kZoneGet) && ((_si == z->u.get->_icon) || (_di == z->u.get->_icon)))) {

					// special Zone
					if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
						return z;
					if (z->_type == type)
						return z;
					if ((z->_type & 0xFFFF0000) == type)
						return z;

				}
			}

			if (z->_left != -1)
				continue;
			if (_si < _char._ani->_left)
				continue;
			if (_si > (_char._ani->_left + _char._ani->width()))
				continue;
			if (_di < _char._ani->_top)
				continue;
			if (_di > (_char._ani->_top + _char._ani->height()))
				continue;

		}

		// normal Zone
		if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
			return z;
		if (z->_type == type)
			return z;
		if ((z->_type & 0xFFFF0000) == type)
			return z;

	}


	int16 _a, _b, _c, _d, _e, _f;
	for (AnimationList::iterator ait = _location._animations.begin(); ait != _location._animations.end(); ait++) {

		AnimationPtr a = *ait;

		_a = (a->_flags & kFlagsActive) ? 1 : 0;															   // _a: active Animation
		_e = ((_si >= a->_left + a->width()) || (_si <= a->_left)) ? 0 : 1;		// _e: horizontal range
		_f = ((_di >= a->_top + a->height()) || (_di <= a->_top)) ? 0 : 1;		// _f: vertical range

		_b = ((type != 0) || (a->_type == kZoneYou)) ? 0 : 1;										 // _b: (no type specified) AND (Animation is not the character)
		_c = (a->_type & 0xFFFF0000) ? 0 : 1;															// _c: Animation is not an object
		_d = ((a->_type & 0xFFFF0000) != type) ? 0 : 1;													// _d: Animation is an object of the same type

		if ((_a != 0 && _e != 0 && _f != 0) && ((_b != 0 && _c != 0) || (a->_type == type) || (_d != 0))) {

			return a;

		}

	}

	return nullZonePtr;
}


void CommandExec_ns::init() {
	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	COMMAND_OPCODE(invalid);
	COMMAND_OPCODE(set);
	COMMAND_OPCODE(clear);
	COMMAND_OPCODE(start);
	COMMAND_OPCODE(speak);
	COMMAND_OPCODE(get);
	COMMAND_OPCODE(location);
	COMMAND_OPCODE(open);
	COMMAND_OPCODE(close);
	COMMAND_OPCODE(on);
	COMMAND_OPCODE(off);
	COMMAND_OPCODE(call);
	COMMAND_OPCODE(toggle);
	COMMAND_OPCODE(drop);
	COMMAND_OPCODE(quit);
	COMMAND_OPCODE(move);
	COMMAND_OPCODE(stop);
}

void ProgramExec_ns::init() {

	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	INSTRUCTION_OPCODE(invalid);
	INSTRUCTION_OPCODE(on);
	INSTRUCTION_OPCODE(off);
	INSTRUCTION_OPCODE(set);		// x
	INSTRUCTION_OPCODE(set);		// y
	INSTRUCTION_OPCODE(set);		// z
	INSTRUCTION_OPCODE(set);		// f
	INSTRUCTION_OPCODE(loop);
	INSTRUCTION_OPCODE(endloop);
	INSTRUCTION_OPCODE(null);
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(sound);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(endscript);

}

ProgramExec_ns::ProgramExec_ns(Parallaction_ns *vm) : _vm(vm) {
}

ProgramExec_ns::~ProgramExec_ns() {
}

}	// namespace Parallaction
