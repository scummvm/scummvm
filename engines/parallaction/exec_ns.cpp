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
#include "parallaction/sound.h"


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


typedef OpcodeImpl<Parallaction_ns> OpcodeV1;
#define COMMAND_OPCODE(op) OpcodeV1(this, &Parallaction_ns::cmdOp_##op)
#define DECLARE_COMMAND_OPCODE(op) void Parallaction_ns::cmdOp_##op()

#define INSTRUCTION_OPCODE(op) OpcodeV1(this, &Parallaction_ns::instOp_##op)
#define DECLARE_INSTRUCTION_OPCODE(op) void Parallaction_ns::instOp_##op()




DECLARE_INSTRUCTION_OPCODE(on) {
	Instruction *inst = *_instRunCtxt.inst;

	inst->_a->_flags |= kFlagsActive;
	inst->_a->_flags &= ~kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(off) {
	(*_instRunCtxt.inst)->_a->_flags |= kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(loop) {
	Instruction *inst = *_instRunCtxt.inst;

	_instRunCtxt.a->_program->_loopCounter = inst->_opB.getRValue();
	_instRunCtxt.a->_program->_loopStart = _instRunCtxt.inst;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--_instRunCtxt.a->_program->_loopCounter > 0) {
		_instRunCtxt.inst = _instRunCtxt.a->_program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	Instruction *inst = *_instRunCtxt.inst;
	int16 _si = inst->_opB.getRValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (_instRunCtxt.modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16* lvalue = inst->_opA.getLValue();

	if (inst->_index == INST_INC) {
		*lvalue += _si;
	} else {
		*lvalue -= _si;
	}

	if (inst->_opA._flags & kParaLocal) {
		wrapLocalVar(inst->_opA._local);
	}

}


DECLARE_INSTRUCTION_OPCODE(set) {
	Instruction *inst = *_instRunCtxt.inst;

	int16 _si = inst->_opB.getRValue();
	int16 *lvalue = inst->_opA.getLValue();

	*lvalue = _si;

}


DECLARE_INSTRUCTION_OPCODE(put) {
	Instruction *inst = *_instRunCtxt.inst;
	Graphics::Surface v18;
	v18.w = inst->_a->width();
	v18.h = inst->_a->height();
	v18.pixels = inst->_a->getFrameData(inst->_a->_frame);

	int16 x = inst->_opA.getRValue();
	int16 y = inst->_opB.getRValue();

	if (inst->_flags & kInstMaskedPut) {
		uint16 z = _gfx->queryMask(y);
		_gfx->blitCnv(&v18, x, y, z, Gfx::kBitBack);
		_gfx->blitCnv(&v18, x, y, z, Gfx::kBit2);
	} else {
		_gfx->flatBlitCnv(&v18, x, y, Gfx::kBitBack);
		_gfx->flatBlitCnv(&v18, x, y, Gfx::kBit2);
	}
}

DECLARE_INSTRUCTION_OPCODE(null) {

}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", (*_instRunCtxt.inst)->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	callFunction((*_instRunCtxt.inst)->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (_engineFlags & kEngineWalking)
		_instRunCtxt.suspend = true;
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_instRunCtxt.inst)->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_activeZone = (*_instRunCtxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	Instruction *inst = (*_instRunCtxt.inst);

	int16 x = inst->_opA.getRValue();
	int16 y = inst->_opB.getRValue();

	WalkNodeList *v4 = _char._builder.buildPath(x, y);
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



void Parallaction_ns::wrapLocalVar(LocalVariable *local) {

	if (local->_value >= local->_max)
		local->_value = local->_min;
	if (local->_value < local->_min)
		local->_value = local->_max - 1;

	return;
}


DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", _cmdRunCtxt.cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags |= _cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] |= _cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags &= ~_cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] &= ~_cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(start) {
	_cmdRunCtxt.cmd->u._animation->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	_activeZone = _cmdRunCtxt.cmd->u._zone;
}


DECLARE_COMMAND_OPCODE(get) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsFixed;
	if (!runZone(_cmdRunCtxt.cmd->u._zone)) {
		runCommands(_cmdRunCtxt.cmd->u._zone->_commands);
	}
}


DECLARE_COMMAND_OPCODE(location) {
	strcpy(_location._name, _cmdRunCtxt.cmd->u._string);
	_engineFlags |= kEngineChangeLocation;
}


DECLARE_COMMAND_OPCODE(open) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsClosed;
	if (_cmdRunCtxt.cmd->u._zone->u.door->_cnv) {
		addJob(&jobToggleDoor, (void*)_cmdRunCtxt.cmd->u._zone, kPriority18 );
	}
}


DECLARE_COMMAND_OPCODE(close) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsClosed;
	if (_cmdRunCtxt.cmd->u._zone->u.door->_cnv) {
		addJob(&jobToggleDoor, (void*)_cmdRunCtxt.cmd->u._zone, kPriority18 );
	}
}


DECLARE_COMMAND_OPCODE(on) {
	// WORKAROUND: the original DOS-based engine didn't check u->_zone before dereferencing
	// the pointer to get structure members, thus leading to crashes in systems with memory
	// protection.
	// As a side note, the overwritten address is the 5th entry in the DOS interrupt table
	// (print screen handler): this suggests that a system would hang when the print screen
	// key is pressed after playing Nippon Safes, provided that this code path is taken.
	if (_cmdRunCtxt.cmd->u._zone != NULL) {
		_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsRemove;
		_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsActive;
		if ((_cmdRunCtxt.cmd->u._zone->_type & 0xFFFF) == kZoneGet) {
			addJob(&jobDisplayDroppedItem, _cmdRunCtxt.cmd->u._zone, kPriority17 );
		}
	}
}


DECLARE_COMMAND_OPCODE(off) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsRemove;
}


DECLARE_COMMAND_OPCODE(call) {
	callFunction(_cmdRunCtxt.cmd->u._callable, _cmdRunCtxt.z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags ^= _cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] ^= _cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(drop){
	dropItem( _cmdRunCtxt.cmd->u._object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_engineFlags |= kEngineQuit;
}


DECLARE_COMMAND_OPCODE(move) {
	if ((_char._ani._flags & kFlagsRemove) || (_char._ani._flags & kFlagsActive) == 0) {
		return;
	}

	WalkNodeList *vC = _char._builder.buildPath(_cmdRunCtxt.cmd->u._move.x, _cmdRunCtxt.cmd->u._move.y);

	addJob(&jobWalk, vC, kPriority19 );
	_engineFlags |= kEngineWalking;
}


DECLARE_COMMAND_OPCODE(stop) {
	_cmdRunCtxt.cmd->u._animation->_flags &= ~kFlagsActing;
}


void jobDisplayAnimations(void *parm, Job *j) {

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

	return;
}


void jobRunScripts(void *parm, Job *j) {
	debugC(3, kDebugJobs, "jobRunScripts");

	static uint16 modCounter = 0;

	for (AnimationList::iterator it = _vm->_animations.begin(); it != _vm->_animations.end(); it++) {

		Animation *a = *it;

		if (a->_flags & kFlagsCharacter)
			a->_z = a->_top + a->height();

		if ((a->_flags & kFlagsActing) == 0)
			continue;

		InstructionList::iterator inst = a->_program->_ip;
		while (((*inst)->_index != INST_SHOW) && (a->_flags & kFlagsActing)) {

			debugC(9, kDebugJobs, "Animation: %s, instruction: %s", a->_label._text, _vm->_instructionNamesRes[(*inst)->_index - 1]);

			_vm->_instRunCtxt.inst = inst;
			_vm->_instRunCtxt.a = a;
			_vm->_instRunCtxt.modCounter = modCounter;
			_vm->_instRunCtxt.suspend = false;

			(*_vm->_instructionOpcodes[(*inst)->_index])();

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


void Parallaction::runCommands(CommandList& list, Zone *z) {
	debugC(1, kDebugLocation, "runCommands");

	CommandList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {

		Command *cmd = *it;
		uint32 v8 = _localFlags[_currentLocationIndex];

		if (_engineFlags & kEngineQuit)
			break;

		if (cmd->_flagsOn & kFlagsGlobal) {
			v8 = _commandFlags | kFlagsGlobal;
		}

		if ((cmd->_flagsOn & v8) != cmd->_flagsOn) continue;
		if ((cmd->_flagsOff & ~v8) != cmd->_flagsOff) continue;

		debugC(1, kDebugLocation, "runCommands[%i]: %s (on: %x, off: %x)", cmd->_id, _commandsNamesRes[cmd->_id-1], cmd->_flagsOn, cmd->_flagsOff);

		_cmdRunCtxt.z = z;
		_cmdRunCtxt.cmd = cmd;

		(*_commandOpcodes[cmd->_id])();
	}

	debugC(1, kDebugLocation, "runCommands completed");

	return;

}




//	displays character head commenting an examined object
//
//	works on the frontbuffer
//
void Parallaction::displayCharacterComment(ExamineData *data) {
	if (data->_description == NULL) return;

	// NOTE: saving visible screen before displaying comment allows
	// to restore the exact situation after the comment is deleted.
	// This means animations are restored in the exact position as
	// they were, thus avoiding clipping effect as signalled in
	// BUG item #1762614.
	_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);

	_gfx->setFont(_dialogueFont);
	_gfx->flatBlitCnv(_char._talk, 0, 190, 80, Gfx::kBitFront);

	int16 v26, v28;
	_gfx->getStringExtent(data->_description, 130, &v28, &v26);
	Common::Rect r(v28, v26);
	r.moveTo(140, 10);
	_gfx->drawBalloon(r, 0);
	_gfx->displayWrappedString(data->_description, 140, 10, 0, 130);

	waitUntilLeftClick();

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	_gfx->updateScreen();

	return;
}

//
//	ZONE TYPE: EXAMINE
//

//	display detail view of an item (and eventually comments)
//
//	works on the frontbuffer
//

void Parallaction::displayItemComment(ExamineData *data) {

	if (data->_description == NULL) return;

	_gfx->setHalfbriteMode(true);

	char v68[PATH_LEN];
	strcpy(v68, data->_filename);
	data->_cnv = _disk->loadStatic(v68);
	_gfx->flatBlitCnv(data->_cnv, 140, (_screenHeight - data->_cnv->h)/2, Gfx::kBitFront);
	delete data->_cnv;

	int16 v6A = 0, v6C = 0;

	_gfx->setFont(_dialogueFont);
	_gfx->getStringExtent(data->_description, 130, &v6C, &v6A);
	Common::Rect r(v6C, v6A);
	r.moveTo(0, 90);
	_gfx->drawBalloon(r, 0);
	_gfx->flatBlitCnv(_char._head, 100, 152, Gfx::kBitFront);
	_gfx->displayWrappedString(data->_description, 0, 90, 0, 130);

	jobEraseAnimations((void*)1, NULL);
	_gfx->updateScreen();

	waitUntilLeftClick();

	_gfx->setHalfbriteMode(false);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	_gfx->updateScreen();

	return;
}



uint16 Parallaction::runZone(Zone *z) {
	debugC(3, kDebugLocation, "runZone (%s)", z->_label._text);

	uint16 subtype = z->_type & 0xFFFF;

	debugC(3, kDebugLocation, "type = %x, object = %x", subtype, (z->_type & 0xFFFF0000) >> 16);
	switch(subtype) {

	case kZoneExamine:
		if (z->u.examine->_filename) {
			displayItemComment(z->u.examine);
		} else {
			displayCharacterComment(z->u.examine);
		}
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
		if (z->u.door->_cnv == NULL) break;
		addJob(&jobToggleDoor, z, kPriority18 );
		break;

	case kZoneHear:
		_soundMan->playSfx(z->u.hear->_name, z->u.hear->_channel, (z->_flags & kFlagsLooping) == kFlagsLooping, 60);
		break;

	case kZoneSpeak:
		runDialogue(z->u.speak);
		break;

	}

	debugC(3, kDebugLocation, "runZone completed");

	return 0;
}

//
//	ZONE TYPE: DOOR
//
void jobToggleDoor(void *parm, Job *j) {

	static byte count = 0;

	Zone *z = (Zone*)parm;

	if (z->u.door->_cnv) {
		Common::Rect r(z->_left, z->_top, z->_left+z->u.door->_cnv->_width, z->_top+z->u.door->_cnv->_height);

		uint16 _ax = (z->_flags & kFlagsClosed ? 1 : 0);
		_vm->_gfx->restoreDoorBackground(r, z->u.door->_cnv->getFramePtr(_ax), z->u.door->_background);

		_ax = (z->_flags & kFlagsClosed ? 0 : 1);
		_vm->_gfx->flatBlitCnv(z->u.door->_cnv, _ax, z->_left, z->_top, Gfx::kBitBack);
		_vm->_gfx->flatBlitCnv(z->u.door->_cnv, _ax, z->_left, z->_top, Gfx::kBit2);
	}

	count++;
	if (count == 2) {
		j->_finished = 1;
		count = 0;
	}

	return;
}



//
//	ZONE TYPE: GET
//

int16 Parallaction::pickupItem(Zone *z) {
	int r = addInventoryItem(z->u.get->_icon);
	if (r == 0)
		addJob(&jobRemovePickedItem, z, kPriority17 );

	return r;
}

void jobRemovePickedItem(void *parm, Job *j) {

	Zone *z = (Zone*)parm;

	static uint16 count = 0;

	if (z->u.get->_cnv) {
		Common::Rect r(z->_left, z->_top, z->_left + z->u.get->_cnv->w, z->_top + z->u.get->_cnv->h);

		_vm->_gfx->restoreGetBackground(r, z->u.get->_backup);
	}

	count++;
	if (count == 2) {
		count = 0;
		j->_finished = 1;
	}

	return;
}

void jobDisplayDroppedItem(void *parm, Job *j) {
//	printf("jobDisplayDroppedItem...");

	Zone *z = (Zone*)parm;

	if (z->u.get->_cnv) {
		if (j->_count == 0) {
			_vm->_gfx->backupGetBackground(z->u.get, z->_left, z->_top);
		}

		_vm->_gfx->flatBlitCnv(z->u.get->_cnv, z->_left, z->_top, Gfx::kBitBack);
		_vm->_gfx->flatBlitCnv(z->u.get->_cnv, z->_left, z->_top, Gfx::kBit2);
	}

	j->_count++;
	if (j->_count == 2) {
		j->_count = 0;
		j->_finished = 1;
	}

//	printf("done");

	return;
}




Zone *Parallaction::hitZone(uint32 type, uint16 x, uint16 y) {
//	printf("hitZone(%i, %i, %i)", type, x, y);

	uint16 _di = y;
	uint16 _si = x;

	for (ZoneList::iterator it = _zones.begin(); it != _zones.end(); it++) {
//		printf("Zone name: %s", z->_name);

		Zone *z = *it;

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
			if (_si < _char._ani._left)
				continue;
			if (_si > (_char._ani._left + _char._ani.width()))
				continue;
			if (_di < _char._ani._top)
				continue;
			if (_di > (_char._ani._top + _char._ani.height()))
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
	for (AnimationList::iterator it = _animations.begin(); it != _animations.end(); it++) {

		Animation *a = *it;

		_a = (a->_flags & kFlagsActive) ? 1 : 0;															   // _a: active Animation
		_e = ((_si >= a->_left + a->width()) || (_si <= a->_left)) ? 0 : 1;		// _e: horizontal range
		_f = ((_di >= a->_top + a->height()) || (_di <= a->_top)) ? 0 : 1;		// _f: vertical range

		_b = ((type != 0) || (a->_type == kZoneYou)) ? 0 : 1; 										 // _b: (no type specified) AND (Animation is not the character)
		_c = (a->_type & 0xFFFF0000) ? 0 : 1; 															// _c: Animation is not an object
		_d = ((a->_type & 0xFFFF0000) != type) ? 0 : 1;													// _d: Animation is an object of the same type

		if ((_a != 0 && _e != 0 && _f != 0) && ((_b != 0 && _c != 0) || (a->_type == type) || (_d != 0))) {

			return a;

		}

	}

	return NULL;
}


void Parallaction_ns::initOpcodes() {

	static const OpcodeV1 op1[] = {
		INSTRUCTION_OPCODE(invalid),
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

	uint i;
	for (i = 0; i < ARRAYSIZE(op1); i++)
		_instructionOpcodes.push_back(&op1[i]);

	static const OpcodeV1 op3[] = {
		COMMAND_OPCODE(invalid),
		COMMAND_OPCODE(set),
		COMMAND_OPCODE(clear),
		COMMAND_OPCODE(start),
		COMMAND_OPCODE(speak),
		COMMAND_OPCODE(get),
		COMMAND_OPCODE(location),
		COMMAND_OPCODE(open),
		COMMAND_OPCODE(close),
		COMMAND_OPCODE(on),
		COMMAND_OPCODE(off),
		COMMAND_OPCODE(call),
		COMMAND_OPCODE(toggle),
		COMMAND_OPCODE(drop),
		COMMAND_OPCODE(quit),
		COMMAND_OPCODE(move),
		COMMAND_OPCODE(stop)
	};

	for (i = 0; i < ARRAYSIZE(op3); i++)
		_commandOpcodes.push_back(&op3[i]);

}



}	// namespace Parallaction
