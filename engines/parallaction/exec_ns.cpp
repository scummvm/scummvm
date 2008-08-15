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

extern const char *_instructionNamesRes_ns[];


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

	_ctxt.program->_loopCounter = inst->_opB.getValue();
	_ctxt.program->_loopStart = _ctxt.ip;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--_ctxt.program->_loopCounter > 0) {
		_ctxt.ip = _ctxt.program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = *_ctxt.inst;
	int16 _si = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (_ctxt.modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	if (inst->_index == INST_INC) {
		lvalue += _si;
	} else {
		lvalue -= _si;
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(set) {
	InstructionPtr inst = *_ctxt.inst;
	inst->_opA.setValue(inst->_opB.getValue());
}


DECLARE_INSTRUCTION_OPCODE(put) {
	InstructionPtr inst = *_ctxt.inst;
	Graphics::Surface v18;
	v18.w = inst->_a->width();
	v18.h = inst->_a->height();
	v18.pixels = inst->_a->getFrameData(inst->_a->getF());

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();
	bool mask = (inst->_flags & kInstMaskedPut) == kInstMaskedPut;

	_vm->_gfx->patchBackground(v18, x, y, mask);
}

DECLARE_INSTRUCTION_OPCODE(show) {
	_ctxt.suspend = true;
}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", (*_ctxt.inst)->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction((*_ctxt.inst)->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (_engineFlags & kEngineWalking) {
		_ctxt.ip--;
		_ctxt.suspend = true;
	}
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_ctxt.inst)->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_vm->_activeZone = (*_ctxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	InstructionPtr inst = (*_ctxt.inst);

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();

	_vm->_char.scheduleWalk(x, y);
}

DECLARE_INSTRUCTION_OPCODE(endscript) {
	if ((_ctxt.anim->_flags & kFlagsLooping) == 0) {
		_ctxt.anim->_flags &= ~kFlagsActing;
		_vm->_cmdExec->run(_ctxt.anim->_commands, _ctxt.anim);
		_ctxt.program->_status = kProgramDone;
	}

	_ctxt.ip = _ctxt.program->_instructions.begin();
	_ctxt.suspend = true;
}




DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", _ctxt.cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags |= _ctxt.cmd->u._flags;
	} else {
		_vm->setLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags &= ~_ctxt.cmd->u._flags;
	} else {
		_vm->clearLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(start) {
	_ctxt.cmd->u._zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	if ((_ctxt.cmd->u._zone->_type & 0xFFFF) == kZoneSpeak) {
		_vm->enterDialogueMode(_ctxt.cmd->u._zone);
	} else {
		_vm->_activeZone = _ctxt.cmd->u._zone;
	}
}


DECLARE_COMMAND_OPCODE(get) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsFixed;
	_vm->runZone(_ctxt.cmd->u._zone);
}


DECLARE_COMMAND_OPCODE(location) {
	_vm->scheduleLocationSwitch(_ctxt.cmd->u._string);
}


DECLARE_COMMAND_OPCODE(open) {
	_vm->updateDoor(_ctxt.cmd->u._zone, false);
}


DECLARE_COMMAND_OPCODE(close) {
	_vm->updateDoor(_ctxt.cmd->u._zone, true);
}

void Parallaction::showZone(ZonePtr z, bool visible) {
	if (!z) {
		return;
	}

	if (visible) {
		z->_flags &= ~kFlagsRemove;
		z->_flags |= kFlagsActive;
	} else {
		z->_flags |= kFlagsRemove;
	}

	if ((z->_type & 0xFFFF) == kZoneGet) {
		_gfx->showGfxObj(z->u.get->gfxobj, visible);
	}

}

DECLARE_COMMAND_OPCODE(on) {
	_vm->showZone(_ctxt.cmd->u._zone, true);
}


DECLARE_COMMAND_OPCODE(off) {
	_vm->showZone(_ctxt.cmd->u._zone, false);
}


DECLARE_COMMAND_OPCODE(call) {
	_vm->callFunction(_ctxt.cmd->u._callable, &_ctxt.z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags ^= _ctxt.cmd->u._flags;
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
	debugC(9, kDebugExec, "Parallaction_ns::drawAnimations()\n");

	uint16 layer = 0;

	for (AnimationList::iterator it = _location._animations.begin(); it != _location._animations.end(); it++) {

		AnimationPtr anim = *it;
		GfxObj *obj = anim->gfxobj;

		// Validation is performed here, so that every animation is affected, instead that only the ones
		// who *own* a script. In fact, some scripts can change values in other animations.
		// The right way to do this would be to enforce validation when any variable is modified from
		// a script.
		anim->validateScriptVars();

		if ((anim->_flags & kFlagsActive) && ((anim->_flags & kFlagsRemove) == 0))   {

			if (anim->_flags & kFlagsNoMasked)
				layer = LAYER_FOREGROUND;
			else {
				if (getGameType() == GType_Nippon) {
					// Layer in NS depends on where the animation is on the screen, for each animation.
					layer = _gfx->_backgroundInfo->getLayer(anim->getFrameY() + anim->height());
				} else {
					// Layer in BRA is calculated from Z value. For characters it is the same as NS,
					// but other animations can have Z set from scripts independently from their
					// position on the screen.
					layer = _gfx->_backgroundInfo->getLayer(anim->getZ());
				}
			}

			if (obj) {
				_gfx->showGfxObj(obj, true);
				obj->frame = anim->getF();
				obj->x = anim->getX();
				obj->y = anim->getY();
				obj->z = anim->getZ();
				obj->layer = layer;
			}
		}

		if (((anim->_flags & kFlagsActive) == 0) && (anim->_flags & kFlagsRemove))   {
			anim->_flags &= ~kFlagsRemove;
		}

		if ((anim->_flags & kFlagsActive) && (anim->_flags & kFlagsRemove))	{
			anim->_flags &= ~kFlagsActive;
			anim->_flags |= kFlagsRemove;
			if (obj) {
				_gfx->showGfxObj(obj, false);
			}
		}
	}

	debugC(9, kDebugExec, "Parallaction_ns::drawAnimations done()\n");

	return;
}

void ProgramExec::runScript(ProgramPtr script, AnimationPtr a) {
	debugC(9, kDebugExec, "runScript(Animation = %s)", a->_name);

	_ctxt.ip = script->_ip;
	_ctxt.anim = a;
	_ctxt.program = script;
	_ctxt.suspend = false;
	_ctxt.modCounter = _modCounter;

	InstructionList::iterator inst;
	for ( ; (a->_flags & kFlagsActing) ; ) {

		inst = _ctxt.ip;
		_ctxt.inst = inst;
		_ctxt.ip++;

		debugC(9, kDebugExec, "inst [%02i] %s\n", (*inst)->_index, _instructionNames[(*inst)->_index - 1]);

		script->_status = kProgramRunning;

		(*_opcodes[(*inst)->_index])();

		if (_ctxt.suspend)
			break;

	}
	script->_ip = _ctxt.ip;

}

void ProgramExec::runScripts(ProgramList::iterator first, ProgramList::iterator last) {
	if (_engineFlags & kEnginePauseJobs) {
		return;
	}

	for (ProgramList::iterator it = first; it != last; it++) {

		AnimationPtr a = (*it)->_anim;

		if (a->_flags & kFlagsCharacter)
			a->setZ(a->getFrameY() + a->height());

		if ((a->_flags & kFlagsActing) == 0)
			continue;

		runScript(*it, a);

		if (a->_flags & kFlagsCharacter)
			a->setZ(a->getFrameY() + a->height());
	}

	_modCounter++;

	return;
}

void CommandExec::runList(CommandList::iterator first, CommandList::iterator last) {

	uint32 useFlags = 0;
	bool useLocalFlags;

	_ctxt.suspend = false;

	for ( ; first != last; first++) {
		if (_engineFlags & kEngineQuit)
			break;

		CommandPtr cmd = *first;

		if (cmd->_flagsOn & kFlagsGlobal) {
			useFlags = _globalFlags | kFlagsGlobal;
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

		_ctxt.z = _execZone;
		_ctxt.cmd = cmd;

		(*_opcodes[cmd->_id])();

		if (_ctxt.suspend) {
			createSuspendList(++first, last);
			return;
		}
	}

}

void CommandExec::run(CommandList& list, ZonePtr z) {
	if (list.size() == 0) {
		debugC(3, kDebugExec, "runCommands: nothing to do");
		return;
	}

	_execZone = z;

	debugC(3, kDebugExec, "runCommands starting");
	runList(list.begin(), list.end());
	debugC(3, kDebugExec, "runCommands completed");
}

void CommandExec::createSuspendList(CommandList::iterator first, CommandList::iterator last) {
	if (first == last) {
		return;
	}

	debugC(3, kDebugExec, "CommandExec::createSuspendList()");

	_suspendedCtxt.valid = true;
	_suspendedCtxt.first = first;
	_suspendedCtxt.last = last;
	_suspendedCtxt.zone = _execZone;
}

void CommandExec::cleanSuspendedList() {
	debugC(3, kDebugExec, "CommandExec::cleanSuspended()");

	_suspendedCtxt.valid = false;
	_suspendedCtxt.first = _suspendedCtxt.last;
	_suspendedCtxt.zone = nullZonePtr;
}

void CommandExec::runSuspended() {
	if (_engineFlags & kEngineWalking) {
		return;
	}

	if (_suspendedCtxt.valid) {
		debugC(3, kDebugExec, "CommandExec::runSuspended()");

		_execZone = _suspendedCtxt.zone;
		runList(_suspendedCtxt.first, _suspendedCtxt.last);
		cleanSuspendedList();
	}
}

CommandExec_ns::CommandExec_ns(Parallaction_ns* vm) : _vm(vm) {

}

CommandExec_ns::~CommandExec_ns() {

}

//
//	ZONE TYPE: EXAMINE
//

void Parallaction::enterCommentMode(ZonePtr z) {
	if (!z) {
		return;
	}

	_commentZone = z;

	ExamineData *data = _commentZone->u.examine;

	if (!data->_description) {
		return;
	}

	// TODO: move this balloons stuff into DialogueManager and BalloonManager
	if (getGameType() == GType_Nippon) {
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
	} else
	if (getGameType() == GType_BRA) {
		_balloonMan->setSingleBalloon(data->_description, 0, 0, 1, 0);
		int id = _gfx->setItem(_char._talk, 10, 80);
		_gfx->setItemFrame(id, 0);
	}

	_input->_inputMode = Input::kInputModeComment;
}

void Parallaction::exitCommentMode() {
	_input->_inputMode = Input::kInputModeGame;

	hideDialogueStuff();
	_gfx->setHalfbriteMode(false);

	_cmdExec->run(_commentZone->_commands, _commentZone);
	_commentZone = nullZonePtr;
}

void Parallaction::runCommentFrame() {
	if (_input->_inputMode != Input::kInputModeComment) {
		return;
	}

	if (_input->getLastButtonEvent() == kMouseLeftUp) {
		exitCommentMode();
	}
}


void Parallaction::runZone(ZonePtr z) {
	debugC(3, kDebugExec, "runZone (%s)", z->_name);

	uint16 subtype = z->_type & 0xFFFF;

	debugC(3, kDebugExec, "type = %x, object = %x", subtype, (z->_type & 0xFFFF0000) >> 16);
	switch(subtype) {

	case kZoneExamine:
		enterCommentMode(z);
		return;

	case kZoneGet:
		pickupItem(z);
		break;

	case kZoneDoor:
		if (z->_flags & kFlagsLocked) break;
		updateDoor(z, !(z->_flags & kFlagsClosed));
		break;

	case kZoneHear:
		_soundMan->playSfx(z->u.hear->_name, z->u.hear->_channel, (z->_flags & kFlagsLooping) == kFlagsLooping, 60);
		break;

	case kZoneSpeak:
		enterDialogueMode(z);
		return;
	}

	debugC(3, kDebugExec, "runZone completed");

	_cmdExec->run(z->_commands, z);

	return;
}

//
//	ZONE TYPE: DOOR
//
void Parallaction::updateDoor(ZonePtr z, bool close) {
	z->_flags = close ? (z->_flags |= kFlagsClosed) : (z->_flags &= ~kFlagsClosed);

	if (z->u.door->gfxobj) {
		uint frame = (close ? 0 : 1);
//		z->u.door->gfxobj->setFrame(frame);
		z->u.door->gfxobj->frame = frame;
	}

	return;
}



//
//	ZONE TYPE: GET
//

bool Parallaction::pickupItem(ZonePtr z) {
	if (z->_flags & kFlagsFixed) {
		return false;
	}

	int slot = addInventoryItem(z->u.get->_icon);
	if (slot != -1) {
		showZone(z, false);
	}

	return (slot != -1);
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
		z->getBox(r);
		r.right++;		// adjust border because Common::Rect doesn't include bottom-right edge
		r.bottom++;

		r.grow(-1);		// allows some tolerance for mouse click

		if (!r.contains(_si, _di)) {

			// out of Zone, so look for special values
			if ((z->getX() == -2) || (z->getX() == -3)) {

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

			if (z->getX() != -1)
				continue;
			if (_si < _char._ani->getFrameX())
				continue;
			if (_si > (_char._ani->getFrameX() + _char._ani->width()))
				continue;
			if (_di < _char._ani->getFrameY())
				continue;
			if (_di > (_char._ani->getFrameY() + _char._ani->height()))
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
		_e = ((_si >= a->getFrameX() + a->width()) || (_si <= a->getFrameX())) ? 0 : 1;		// _e: horizontal range
		_f = ((_di >= a->getFrameY() + a->height()) || (_di <= a->getFrameY())) ? 0 : 1;		// _f: vertical range

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
	INSTRUCTION_OPCODE(show);
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
	_instructionNames = _instructionNamesRes_ns;
}

ProgramExec_ns::~ProgramExec_ns() {
}

}	// namespace Parallaction
