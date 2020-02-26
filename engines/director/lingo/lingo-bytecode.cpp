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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/substream.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-bytecode.h"
#include "director/lingo/lingo-the.h"

namespace Director {

static LingoV4Bytecode lingoV4[] = {
	{ 0x01, LC::c_procret,		"" },
	{ 0x03, LC::cb_zeropush,	"" },
	{ 0x04, LC::c_mul,			"" },
	{ 0x05, LC::c_add,			"" },
	{ 0x06, LC::c_sub,			"" },
	{ 0x07, LC::c_div,			"" },
	{ 0x08, LC::c_mod,			"" },
	{ 0x09, LC::c_negate,		"" },
	{ 0x0a, LC::c_ampersand,	"" },
	{ 0x0b, LC::c_concat,		"" },
	{ 0x0c, LC::c_lt,			"" },
	{ 0x0d, LC::c_le,			"" },
	{ 0x0e, LC::c_neq,			"" },
	{ 0x0f, LC::c_eq,			"" },
	{ 0x10, LC::c_gt,			"" },
	{ 0x11, LC::c_ge,			"" },
	{ 0x12, LC::c_and,			"" },
	{ 0x13, LC::c_or,			"" },
	{ 0x14, LC::c_not,			"" },
	{ 0x15, LC::c_contains,		"" },
	{ 0x16, LC::c_starts,		"" },
	{ 0x17, LC::c_of,			"" },
	{ 0x18, LC::c_hilite,		"" },
	{ 0x19, LC::c_intersects,	"" },
	{ 0x1a, LC::c_within,		"" },
	{ 0x1b, LC::cb_field,		"" },
	{ 0x1c, LC::c_tell,			"" },
	{ 0x1d, LC::c_telldone,		"" },
	{ 0x1e, LC::cb_list,		"" },
	{ 0x41, LC::c_intpush,		"b" },
	{ 0x42, LC::c_argcnoretpush,"b" },
	{ 0x43, LC::c_argcpush,		"b" },
	// 0x44, push a constant
	{ 0x45, LC::c_namepush,		"b" },
	{ 0x46, LC::cb_objectpush,  "b" },
	{ 0x49, LC::cb_globalpush,	"b" },
	{ 0x4b, LC::cb_varpush,		"bpa" },
	{ 0x4c, LC::cb_varpush,		"bpv" },
	{ 0x4f, LC::cb_globalassign,"b" },
	{ 0x51, LC::cb_varassign,	"bpa" },
	{ 0x52, LC::cb_varassign,	"bpv" },
	{ 0x53, LC::c_jump,			"jb" },
	{ 0x54, LC::c_jump,			"jbn" },
	{ 0x55, LC::c_jumpifz,		"jb" },
	{ 0x56, LC::cb_localcall,	"b" },
	{ 0x57, LC::cb_call,		"b" },
	{ 0x58, LC::cb_methodcall,  "b" },
	{ 0x59, LC::cb_v4assign,	"b" },
	{ 0x5c, LC::cb_v4theentitypush, "b" },
	{ 0x5d, LC::cb_v4theentityassign, "b" },
	{ 0x66, LC::cb_v4theentitynamepush, "b" },
	{ 0x81, LC::c_intpush,		"w" },
	{ 0x82, LC::c_argcnoretpush,"w" },
	{ 0x83, LC::c_argcpush,		"w" },
	// 0x84, push a constant
	{ 0x85, LC::c_namepush,     "w" },
	{ 0x86, LC::cb_objectpush,  "w" },
	{ 0x89, LC::cb_globalpush,	"w" },
	{ 0x8b, LC::cb_varpush,		"wpa" },
	{ 0x8c, LC::cb_varpush,		"wpv" },
	{ 0x8f, LC::cb_globalassign,"w" },
	{ 0x91, LC::cb_varassign,	"wpa" },
	{ 0x92, LC::cb_varassign,	"wpv" },
	{ 0x93, LC::c_jump,			"jw" },
	{ 0x94, LC::c_jump,			"jwn" },
	{ 0x95, LC::c_jumpifz,		"jw" },
	{ 0x96, LC::cb_localcall,	"w" },
	{ 0x97, LC::cb_call,		"w" },
	{ 0x98, LC::cb_methodcall,  "w" },
	{ 0x99, LC::cb_v4assign,	"w" },
	{ 0x9c, LC::cb_v4theentitypush, "w" },
	{ 0x9d, LC::cb_v4theentityassign, "w" },
	{ 0xa6, LC::cb_v4theentitynamepush, "w" },
	{ 0, 0, 0 }
};

static LingoV4TheEntity lingoV4TheEntity[] = {
	{ 0x00, 0x00, kTheFloatPrecision,	kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x01, kTheMouseDownScript,	kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x02, kTheMouseUpScript,	kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x03, kTheKeyDownScript,	kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x04, kTheKeyUpScript,		kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x05, kTheTimeoutScript,	kTheNOField,		true, kTEANOArgs },
	{ 0x00, 0x06, kTheTime,				kTheShort,			false, kTEANOArgs },
	{ 0x00, 0x07, kTheTime,				kTheAbbr,			false, kTEANOArgs },
	{ 0x00, 0x08, kTheTime,				kTheLong,			false, kTEANOArgs },
	{ 0x00, 0x09, kTheDate,				kTheShort,			false, kTEANOArgs },
	{ 0x00, 0x0a, kTheDate,				kTheAbbr,			false, kTEANOArgs },
	{ 0x00, 0x0b, kTheDate,				kTheLong,			false, kTEANOArgs },
	{ 0x00, 0x0c, kTheChars,			kTheLast,			false, kTEAString },
	{ 0x00, 0x0d, kTheWords,			kTheLast,			false, kTEAString },
	{ 0x00, 0x0e, kTheItems,			kTheLast,			false, kTEAString },
	{ 0x00, 0x0f, kTheLines,			kTheLast,			false, kTEAString },
	{ 0x01, 0x01, kTheChars,			kTheNumber,			false, kTEAString },
	{ 0x01, 0x02, kTheWords,			kTheNumber,			false, kTEAString },
	{ 0x01, 0x03, kTheItems,			kTheNumber,			false, kTEAString },
	{ 0x01, 0x04, kTheLines,			kTheNumber,			false, kTEAString },
	{ 0x02, 0x01, kTheMenu,				kTheName,			false, kTEAItemId },
	{ 0x02, 0x02, kTheMenuItems,		kTheNumber,			false, kTEAItemId },
	{ 0x03, 0x01, kTheMenuItem,			kTheName,			true, kTEAMenuIdItemId },
	{ 0x03, 0x02, kTheMenuItem,			kTheCheckMark,		true, kTEAMenuIdItemId },
	{ 0x03, 0x03, kTheMenuItem,			kTheEnabled,		true, kTEAMenuIdItemId },
	{ 0x03, 0x04, kTheMenuItem,			kTheScript,			true, kTEAMenuIdItemId },
	{ 0x04, 0x01, kTheSoundEntity,		kTheVolume,			true, kTEAItemId },
	{ 0x06, 0x01, kTheSprite,			kTheCursor,			true, kTEAItemId },
	{ 0x06, 0x02, kTheSprite,			kTheBackColor,		true, kTEAItemId },
	{ 0x06, 0x03, kTheSprite,			kTheBottom,			true, kTEAItemId },
	{ 0x06, 0x04, kTheSprite,			kTheCastNum,		true, kTEAItemId },
	{ 0x06, 0x05, kTheSprite,			kTheConstraint,		true, kTEAItemId },
	{ 0x06, 0x06, kTheSprite,			kTheCursor,			true, kTEAItemId },
	{ 0x06, 0x07, kTheSprite,			kTheForeColor,		true, kTEAItemId },
	{ 0x06, 0x08, kTheSprite,			kTheHeight,			true, kTEAItemId },
	{ 0x06, 0x0a, kTheSprite,			kTheInk,			true, kTEAItemId },
	{ 0x06, 0x0b, kTheSprite,			kTheLeft,			true, kTEAItemId },
	{ 0x06, 0x0c, kTheSprite,			kTheLineSize,		true, kTEAItemId },
	{ 0x06, 0x0d, kTheSprite,			kTheLocH,			true, kTEAItemId },
	{ 0x06, 0x0e, kTheSprite,			kTheLocV,			true, kTEAItemId },
	{ 0x06, 0x0f, kTheSprite,			kTheMovieRate,		true, kTEAItemId },
	{ 0x06, 0x10, kTheSprite,			kTheMovieTime,		true, kTEAItemId },
	{ 0x06, 0x12, kTheSprite,			kThePuppet,			true, kTEAItemId },
	{ 0x06, 0x13, kTheSprite,			kTheRight,			true, kTEAItemId },
	{ 0x06, 0x14, kTheSprite,			kTheStartTime,		true, kTEAItemId },
	{ 0x06, 0x15, kTheSprite,			kTheStopTime,		true, kTEAItemId },
	{ 0x06, 0x16, kTheSprite,			kTheStretch,		true, kTEAItemId },
	{ 0x06, 0x17, kTheSprite,			kTheTop,			true, kTEAItemId },
	{ 0x06, 0x18, kTheSprite,			kTheTrails,			true, kTEAItemId },
	{ 0x06, 0x19, kTheSprite,			kTheVisible,		true, kTEAItemId },
	{ 0x06, 0x1a, kTheSprite,			kTheVolume,			true, kTEAItemId },
	{ 0x06, 0x1b, kTheSprite,			kTheWidth,			true, kTEAItemId },
	{ 0x06, 0x1d, kTheSprite,			kTheScriptNum,		true, kTEAItemId },
	{ 0x06, 0x1e, kTheSprite,			kTheMoveableSprite,	true, kTEAItemId },
	{ 0x06, 0x20, kTheSprite,			kTheScoreColor,		true, kTEAItemId },
	{ 0x07, 0x01, kTheBeepOn,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x02, kTheButtonStyle,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x03, kTheCenterStage,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x04, kTheCheckBoxAccess,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x05, kTheCheckBoxType,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x06, kTheColorDepth,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x08, kTheExitLock,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x09, kTheFixStageSize,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x13, kTheTimeoutLapsed,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x17, kTheSelEnd,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x18, kTheSelStart,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x19, kTheSoundEnabled,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1a, kTheSoundLevel,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1b, kTheStageColor,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1d, kTheStillDown,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1e, kTheTimeoutKeyDown,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1f, kTheTimeoutLength,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x20, kTheTimeoutMouse,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x21, kTheTimeoutPlay,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x22, kTheTimer,			kTheNOField,		true, kTEANOArgs },
	{ 0x08, 0x01, kThePerFrameHook,		kTheNOField,		false, kTEANOArgs },
	{ 0x08, 0x02, kTheCastMembers,		kTheNumber,			false, kTEANOArgs },
	{ 0x08, 0x03, kTheMenus,			kTheNumber,			false, kTEANOArgs },
	{ 0x09, 0x01, kTheCast,				kTheName,			true, kTEAItemId },
	{ 0x09, 0x02, kTheCast,				kTheText,			true, kTEAItemId },
	{ 0x09, 0x08, kTheCast,				kThePicture,		true, kTEAItemId },
	{ 0x09, 0x0a, kTheCast,				kTheNumber,			true, kTEAItemId },
	{ 0x09, 0x0b, kTheCast,				kTheSize,			true, kTEAItemId },
	{ 0x09, 0x11, kTheCast,				kTheForeColor,		true, kTEAItemId },
	{ 0x09, 0x12, kTheCast,				kTheBackColor,		true, kTEAItemId },
	{ 0x0c, 0x03, kTheField,			kTheTextStyle,		true, kTEAItemId },
	{ 0x0c, 0x04, kTheField,			kTheTextFont,		true, kTEAItemId },
	{ 0x0c, 0x05, kTheField,			kTheTextHeight,		true, kTEAItemId },
	{ 0x0c, 0x06, kTheField,			kTheTextAlign,		true, kTEAItemId },
	{ 0x0c, 0x07, kTheField,			kTheTextSize,		true, kTEAItemId },
	{ 0x0d, 0x0f, kTheCast,				kTheDirectToStage,	true, kTEAItemId },
	{ 0x0d, 0x10, kTheCast,				kTheSound,			true, kTEAItemId },
	{ 0xff, 0, 0, 0, false, kTEANOArgs }
};


void Lingo::initBytecode() {
	for (LingoV4Bytecode *op = lingoV4; op->opcode; op++) {
		_lingoV4[op->opcode] = op;
	}

	for (LingoV4TheEntity *ent = lingoV4TheEntity; ent->bank != 0xff; ent++) {
		_lingoV4TheEntity[(ent->bank << 8) + ent->firstArg] = ent;
	}
}

void LC::cb_unk() {
	uint opcode = g_lingo->readInt();
	warning("STUB: opcode 0x%02x", opcode);
}

void LC::cb_unk1() {
	uint opcode = g_lingo->readInt();
	uint arg1 = g_lingo->readInt();
	warning("STUB: opcode 0x%02x (%d)", opcode, arg1);
}

void LC::cb_unk2() {
	uint opcode = g_lingo->readInt();
	uint arg1 = g_lingo->readInt();
	uint arg2 = g_lingo->readInt();
	warning("STUB: opcode 0x%02x (%d, %d)", opcode, arg1, arg2);
}

void LC::cb_field() {
	LB::b_field(1);
	Datum field = g_lingo->pop();
	Datum result = g_lingo->varFetch(field);
	g_lingo->push(result);
}


void LC::cb_localcall() {
	int functionId = g_lingo->readInt();

	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		Symbol *sym = g_lingo->_currentScriptContext->functions[functionId];
		if (debugChannelSet(3, kDebugLingoExec))
			g_lingo->printSTUBWithArglist(sym->name.c_str(), nargs.u.i, "localcall:");

		LC::call(sym, nargs.u.i);

	} else {
		warning("cb_localcall: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

}


void LC::cb_methodcall() {
	g_lingo->readInt();
	Datum obj = g_lingo->pop();
	obj.toString();
	warning("STUB: cb_methodcall(%s)", obj.u.s->c_str());

	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		if (debugChannelSet(3, kDebugLingoExec))
			g_lingo->printSTUBWithArglist("", nargs.u.i, "methodcall:");

	} else {
		warning("cb_methodcall: second arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

}


void LC::cb_v4assign() {
	int op = g_lingo->readInt();

	switch (op) {
	case 0x16:
		// put value into field textVar
		{
			LB::b_field(1);
			LC::c_assign();
		}
		break;
	case 0x26:
		// put value after field textVar
		{
			LB::b_field(1);
			Datum field = g_lingo->pop();
			g_lingo->push(field);
			LC::c_after();
			g_lingo->push(field);
			LC::c_assign();
		}
		break;
	case 0x36:
		// put value before field textVar
		{
			LB::b_field(1);
			Datum field = g_lingo->pop();
			g_lingo->push(field);
			LC::c_before();
			g_lingo->push(field);
			LC::c_assign();
		}
		break;
	default:
		warning("cb_v4assign: unknown operator %d", op);
		break;
	}
}


void LC::cb_list() {
	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		LB::b_list(nargs.u.i);
	} else {
		warning("cb_list: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}
}


void LC::cb_call() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);

	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		LC::call(name, nargs.u.i);

	} else {
		warning("cb_call: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

}


void LC::cb_globalpush() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);
	Datum result;
	result.type = VOID;

	Symbol *s = g_lingo->lookupVar(name.c_str(), false);
	if (!s) {
		warning("Variable %s not found", name.c_str());
		g_lingo->push(result);
		return;
	} else if (s && !s->global) {
		warning("Variable %s is local, not global", name.c_str());
	}

	Datum target;
	target.type = VAR;
	target.u.sym = s;
	result = g_lingo->varFetch(target);
	g_lingo->push(result);
}


void LC::cb_globalassign() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);

	Symbol *s = g_lingo->lookupVar(name.c_str(), false);
	if (!s) {
		// Lingo lets you declare globals inside a method.
		// This doesn't define them in the script list, but you can still
		// read and write to them???
		s = g_lingo->lookupVar(name.c_str(), true, true);
	}
	if (s && !s->global) {
		warning("Variable %s is local, not global", name.c_str());
	}

	Datum target;
	target.type = VAR;
	target.u.sym = s;
	Datum source = g_lingo->pop();
	g_lingo->varAssign(target, source);
}


void LC::cb_objectpush() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);
	warning("STUB: cb_objectpush(%s)", name.c_str());
	Datum result;
	result.type = VOID;
	g_lingo->push(result);
}


void LC::cb_varpush() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);
	Datum result;
	result.type = VOID;

	Symbol *s = g_lingo->lookupVar(name.c_str(), false);
	if (!s) {
		warning("Variable %s not found", name.c_str());
		g_lingo->push(result);
		return;
	} else if (s && s->global) {
		warning("Variable %s is global, not local", name.c_str());
	}

	Datum target;
	target.type = VAR;
	target.u.sym = s;
	result = g_lingo->varFetch(target);
	g_lingo->push(result);
}


void LC::cb_varassign() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);

	Symbol *s = g_lingo->lookupVar(name.c_str(), false);
	if (!s) {
		warning("Variable %s not found", name.c_str());
		g_lingo->pop();
		return;
	} else if (s && s->global) {
		warning("Variable %s is global, not local", name.c_str());
	}

	Datum target;
	target.type = VAR;
	target.u.sym = s;
	Datum source = g_lingo->pop();
	g_lingo->varAssign(target, source);
}


void LC::cb_v4theentitypush() {
	int bank = g_lingo->readInt();

	Datum firstArg = g_lingo->pop();
	firstArg.toInt();
	Datum result;
	result.u.s = NULL;
	result.type = VOID;

	int key = (bank << 8) + firstArg.u.i;
	if (g_lingo->_lingoV4TheEntity.contains(key)) {
		debugC(3, kDebugLingoExec, "cb_v4theentitypush: mapping 0x%02x, 0x%02x", bank, firstArg.u.i);
		int entity = g_lingo->_lingoV4TheEntity[key]->entity;
		int field = g_lingo->_lingoV4TheEntity[key]->field;
		switch (g_lingo->_lingoV4TheEntity[key]->type) {
		case kTEANOArgs:
			{
				Datum id;
				id.u.s = NULL;
				id.type = VOID;
				debugC(3, kDebugLingoExec, "cb_v4theentitypush: calling getTheEntity(0x%02x, NULL, 0x%02x)", entity, field);
				result = g_lingo->getTheEntity(entity, id, field);
			}
			break;
		case kTEAItemId:
			{
				Datum id = g_lingo->pop();
				debugC(3, kDebugLingoExec, "cb_v4theentitypush: calling getTheEntity(0x%02x, id, 0x%02x)", entity, field);
				result = g_lingo->getTheEntity(entity, id, field);
			}
			break;
		case kTEAString:
			{
				/*Datum stringArg = */g_lingo->pop();
				warning("cb_v4theentitypush: STUB: kTEAString");
			}
			break;
		case kTEAMenuIdItemId:
			{
				/*Datum menuId = */g_lingo->pop();
				/*Datum itemId = */g_lingo->pop();
				warning("cb_v4theentitypush: STUB: kTEAMenuIdItemId");
			}
			break;
		default:
			warning("cb_v4theentitypush: unknown call type %d", g_lingo->_lingoV4TheEntity[key]->type);
			break;
		}
	} else {
		warning("cb_v4theentitypush: unhandled mapping 0x%02x 0x%02x", bank, firstArg.u.i);
	}

	g_lingo->push(result);
}


void LC::cb_v4theentitynamepush() {
	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		if (nargs.u.i > 0) {
			warning("cb_v4theentitynamepush: expecting argc to be 0, not %d", nargs.u.i);
			for (int i = 0; i < nargs.u.i; i++) {
				g_lingo->pop();
			}
		}
	} else {
		warning("cb_v4theentitynamepush: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->getName(nameId);

	Datum id;
	id.u.s = NULL;
	id.type = VOID;

	TheEntity *entity = g_lingo->_theEntities[name];

	debugC(3, kDebugLingoExec, "cb_v4theentitynamepush: %s", name.c_str());
	debugC(3, kDebugLingoExec, "cb_v4theentitynamepush: calling getTheEntity(0x%02x, id, kTheNOField)", entity->entity);
	Datum result = g_lingo->getTheEntity(entity->entity, id, kTheNOField);

	g_lingo->push(result);
}


void LC::cb_v4theentityassign() {
	int bank = g_lingo->readInt();

	Datum firstArg = g_lingo->pop();
	firstArg.toInt();
	Datum value = g_lingo->pop();
	Datum result;
	result.u.s = NULL;
	result.type = VOID;

	int key = (bank << 8) + firstArg.u.i;
	if (!g_lingo->_lingoV4TheEntity.contains(key)) {
		warning("cb_v4theentityassign: unhandled mapping 0x%02x 0x%02x", bank, firstArg.u.i);

		return;
	}

	debugC(3, kDebugLingoExec, "cb_v4theentityassign: mapping 0x%02x, 0x%02x", bank, firstArg.u.i);

	if (!g_lingo->_lingoV4TheEntity[key]->writable) {
		warning("cb_v4theentityassign: non-writable mapping 0x%02x 0x%02x", bank, firstArg.u.i);

		return;
	}

	int entity = g_lingo->_lingoV4TheEntity[key]->entity;
	int field = g_lingo->_lingoV4TheEntity[key]->field;
	switch (g_lingo->_lingoV4TheEntity[key]->type) {
	case kTEANOArgs:
		{
			Datum id;
			id.u.s = NULL;
			id.type = VOID;
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: calling setTheEntity(0x%02x, NULL, 0x%02x, value)", entity, field);
			g_lingo->setTheEntity(entity, id, field, value);
		}
		break;
	case kTEAItemId:
		{
			Datum id = g_lingo->pop();
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: calling setTheEntity(0x%02x, id, 0x%02x, value)", entity, field);
			g_lingo->setTheEntity(entity, id, field, value);
		}
		break;
	case kTEAString:
		{
			/*Datum stringArg = */g_lingo->pop();
			warning("cb_v4theentityassign: STUB: kTEAString");
		}
		break;
	case kTEAMenuIdItemId:
		{
			/*Datum menuId = */g_lingo->pop();
			/*Datum itemId = */g_lingo->pop();
			warning("cb_v4theentityassign: STUB: kTEAMenuIdItemId");
		}
		break;
	default:
		warning("cb_v4theentityassign: unknown call type %d", g_lingo->_lingoV4TheEntity[key]->type);
		break;
	}
}

void LC::cb_zeropush() {
	Datum d;
	d.u.i = 0;
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id) {
	debugC(1, kDebugLingoCompile, "Add V4 bytecode for type %s with id %d", scriptType2str(type), id);

	if (getScriptContext(type, id)) {
		// We can't undefine context data because it could be used in e.g. symbols.
		// Abort on double definitions.
		error("Script already defined for type %d, id %d", id, type);
		return;
	}

	_currentScriptContext = new ScriptContext;
	_currentScriptType = type;
	_currentEntityId = id;
	_archives[_archiveIndex].scriptContexts[type][id] = _currentScriptContext;

	if (stream.size() < 0x5c) {
		warning("Lscr header too small");
		return;
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr header:");
		stream.hexdump(0x5c);
	}

	// read the Lscr header!
	// unk1
	for (uint32 i = 0; i < 0x10; i++) {
		stream.readByte();
	}
	uint16 codeStoreOffset = stream.readUint16();
	// unk2
	for (uint32 i = 0; i < 0x2e; i++) {
		stream.readByte();
	}
	uint16 globalsOffset = stream.readUint16();
	uint16 globalsCount = stream.readUint16();
	// unk3
	for (uint32 i = 0; i < 0x4; i++) {
		stream.readByte();
	}
	uint16 functionsCount = stream.readUint16();
	stream.readUint16();
	uint16 functionsOffset = stream.readUint16();
	uint16 constsCount = stream.readUint16();
	stream.readUint16();
	uint16 constsOffset = stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	/*uint16 constsBase = */stream.readUint16();

	// initialise each global variable
	if (stream.size() < globalsOffset + globalsCount * 2) {
		warning("Lscr globals store missing");
		return;
	}

	debugC(5, kDebugLoading, "Lscr globals list:");
	stream.seek(globalsOffset);
	for (uint16 i = 0; i < globalsCount; i++) {
		uint16 index = stream.readUint16();
		if (index < _archives[_archiveIndex].names.size()) {
			const char *name = _archives[_archiveIndex].names[index].c_str();
			debugC(5, kDebugLoading, "%d: %s", i, name);
			g_lingo->lookupVar(name, true, true);
		} else {
			warning("Global %d has unknown name id %d, skipping define", i, index);
		}
	}

	// preload all the constants!
	// these are stored as a reference table of 6 byte entries, followed by a storage area.

	// copy the storage area first.
	uint32 constsStoreOffset = constsOffset + 6 * constsCount;
	uint32 constsStoreSize = stream.size() - constsStoreOffset;

	if ((uint32)stream.size() < constsStoreOffset) {
		warning("Lscr consts store missing");
		return;
	}

	stream.seek(constsStoreOffset);

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr consts store:");
		stream.hexdump(constsStoreSize);
	}

	byte *constsStore = (byte *)malloc(constsStoreSize);
	stream.read(constsStore, constsStoreSize);

	// read each entry in the reference table.
	stream.seek(constsOffset);
	for (uint16 i = 0; i < constsCount; i++) {
		Datum constant;
		uint32 constType = 0;
		if (_vm->getVersion() >= 5) {
			constType = stream.readUint32();
		} else {
			constType = (uint32)stream.readUint16();
		}
		uint32 value = stream.readUint32();
		switch (constType) {
		case 1: // String type
			{
				constant.type = STRING;
				constant.u.s = new Common::String();
				uint32 pointer = value;
				if (pointer + 4 > constsStoreSize) {
					error("Constant string is too small");
					break;
				}
				uint32 length = READ_BE_UINT32(&constsStore[pointer]);
				pointer += 4;
				uint32 end = pointer + length;
				if (end > constsStoreSize) {
					error("Constant string is too large");
					break;
				}
				while (pointer < end) {
					if (constsStore[pointer] == '\r') {
						*constant.u.s += '\n';
					} else if (constsStore[pointer] == '\0') {
						break;
					} else {
						*constant.u.s += constsStore[pointer];
					}
					pointer += 1;
				}
				if (pointer > constsStoreSize) {
					warning("Constant string has no null terminator");
					break;
				}
			}
			break;
		case 4: // Integer type
			constant.type = INT;
			constant.u.i = (int)value;
			break;
		case 9:  // Float type
			{
				constant.type = FLOAT;
				if (value + 4 > constsStoreSize) {
					warning("Constant float end offset is out of bounds");
					break;
				}

				uint32 pointer = value;
				uint32 length = READ_BE_UINT32(&constsStore[pointer]);
				pointer += 4;
				uint32 end = pointer + length;
				if (end > constsStoreSize) {
					error("Constant float is too large");
					break;
				}

				// Floats are stored as an "80 bit IEEE Standard 754 floating
				// point number (Standard Apple Numeric Environment [SANE] data type
				// Extended).
				if (length != 10) {
					error("Constant float expected to be 10 bytes");
					break;
				}
				uint16 exponent = READ_BE_UINT16(&constsStore[pointer]);
				uint64 f64sign = (uint64)(exponent & 0x8000) << 48;
				exponent &= 0x7fff;
				uint64 fraction = READ_BE_UINT64(&constsStore[pointer+2]);
				fraction &= 0x7fffffffffffffffULL;
				uint64 f64exp = 0;
				if (exponent == 0) {
					f64exp = 0;
				} else if (exponent == 0x7fff) {
					f64exp = 0x7ff;
				} else {
					int32 normexp = (int32)exponent - 0x3fff;
					if ((-0x3fe > normexp) || (normexp >= 0x3ff)) {
						error("Constant float exponent too big for a double");
						break;
					}
					f64exp = (uint64)(normexp + 0x3ff);
				}
				f64exp <<= 52;
				uint64 f64fract = fraction >> 11;
				uint64 f64bin = f64sign | f64exp | f64fract;

				constant.u.f = *(double *)(&f64bin);
			}
			break;
		default:
			warning("Unknown constant type %d", type);
			break;
		}

		_currentScriptContext->constants.push_back(constant);
	}
	free(constsStore);

	// parse each function!
	// these are stored as a code storage area, followed by a reference table of 42 byte entries.

	// copy the storage area first.
	if (stream.size() < functionsOffset) {
		warning("Lscr functions store missing");
		return;
	}

	uint32 codeStoreSize = functionsOffset - codeStoreOffset;
	stream.seek(codeStoreOffset);
	byte *codeStore = (byte *)malloc(codeStoreSize);
	stream.read(codeStore, codeStoreSize);

	// read each entry in the function table.
	stream.seek(functionsOffset);
	for (uint16 i = 0; i < functionsCount; i++) {
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Function %d header:", i);
			stream.hexdump(0x2a);
		}

		uint16 nameIndex = stream.readUint16();
		stream.readUint16();
		uint32 length = stream.readUint32();
		uint32 startOffset = stream.readUint32();
		uint16 argCount = stream.readUint16();
		uint32 argOffset = stream.readUint32();
		uint16 varCount = stream.readUint16();
		uint32 varOffset = stream.readUint32();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();

		if (startOffset < codeStoreOffset) {
			warning("Function %d start offset is out of bounds!", i);
			continue;
		} else if (startOffset + length >= codeStoreOffset + codeStoreSize) {
			warning("Function %d end offset is out of bounds", i);
			continue;
		}

		// Fetch argument name list
		Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
		Common::HashMap<uint16, uint16> argMap;
		if (argOffset < codeStoreOffset) {
			warning("Function %d argument names start offset is out of bounds!", i);
		} else if (argOffset + argCount*2 >= codeStoreOffset + codeStoreSize) {
			warning("Function %d argument names end offset is out of bounds!", i);
		} else {
			debugC(5, kDebugLoading, "Function %d argument list:", i);
			uint16 namePointer = argOffset - codeStoreOffset;
			for (int j = 0; j < argCount; j++) {
				uint16 index = (uint16)READ_BE_UINT16(&codeStore[namePointer]);
				namePointer += 2;
				Common::String name;
				if (index < _archives[_archiveIndex].names.size()) {
					name = _archives[_archiveIndex].names[index];
					argMap[j] = index;
				} else {
					name = Common::String::format("arg_%d", j);
					warning("Argument has unknown name id %d, using name %s", j, name.c_str());
				}
				debugC(5, kDebugLoading, "%d: %s", j, name.c_str());
				argNames->push_back(name);
			}
		}

		// Fetch variable name list
		Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
		Common::HashMap<uint16, uint16> varMap;
		if (varOffset < codeStoreOffset) {
			warning("Function %d variable names start offset is out of bounds!", i);
		} else if (varOffset + varCount*2 >= codeStoreOffset + codeStoreSize) {
			warning("Function %d variable names end offset is out of bounds!", i);
		} else {
			debugC(5, kDebugLoading, "Function %d variable list:", i);
			uint16 namePointer = varOffset - codeStoreOffset;
			for (int j = 0; j < varCount; j++) {
				uint16 index = (uint16)READ_BE_UINT16(&codeStore[namePointer]);
				namePointer += 2;
				Common::String name;
				if (index < _archives[_archiveIndex].names.size()) {
					name = _archives[_archiveIndex].names[index];
					varMap[j] = index;
				} else {
					name = Common::String::format("var_%d", j);
					warning("Variable has unknown name id %d, using name %s", j, name.c_str());
				}
				debugC(5, kDebugLoading, "%d: %s", j, name.c_str());
				varNames->push_back(name);
			}
		}

		_currentScriptFunction = i;
		_currentScript = new ScriptData;

		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Function %d code:", i);
			Common::hexdump(codeStore, length, 16, startOffset);
		}

		uint16 pointer = startOffset - codeStoreOffset;
		Common::Array<uint32> offsetList;
		Common::Array<uint32> jumpList;
		while (pointer < startOffset + length - codeStoreOffset) {
			uint8 opcode = codeStore[pointer];
			pointer += 1;

			if (opcode == 0x44 || opcode == 0x84) {
				// push a constant
				offsetList.push_back(_currentScript->size());
				int arg = 0;
				if (opcode == 0x84) {
					arg = (uint16)READ_BE_UINT16(&codeStore[pointer]);
					pointer += 2;
				} else {
					arg = (uint8)codeStore[pointer];
					pointer += 1;
				}
				// remove struct size alignment
				if (arg % 6) {
					warning("Opcode 0x%02x arg %d not a multiple of 6!", opcode, arg);
				}
				arg /= 6;
				Datum constant = _currentScriptContext->constants[arg];
				switch (constant.type) {
				case INT:
					g_lingo->code1(LC::c_intpush);
					break;
				case FLOAT:
					g_lingo->code1(LC::c_floatpush);
					break;
				case STRING:
					g_lingo->code1(LC::c_stringpush);
					break;
				default:
					error("Unknown constant type %d", constant.type);
					break;
				}
				if (opcode == 0x84) {
					offsetList.push_back(_currentScript->size());
					offsetList.push_back(_currentScript->size());
				} else {
					offsetList.push_back(_currentScript->size());
				}
				switch (constant.type) {
				case INT:
					g_lingo->codeInt(constant.u.i);
					break;
				case FLOAT:
					g_lingo->codeFloat(constant.u.f);
					break;
				case STRING:
					g_lingo->codeString(constant.u.s->c_str());
					break;
				default:
					error("Unknown constant type %d", constant.type);
					break;
				}
			} else if (_lingoV4.contains(opcode)) {
				offsetList.push_back(_currentScript->size());
				g_lingo->code1(_lingoV4[opcode]->func);

				size_t argc = strlen(_lingoV4[opcode]->proto);
				if (argc) {
					int arg = 0;
					for (uint c = 0; c < argc; c++) {
						switch (_lingoV4[opcode]->proto[c]) {
						case 'b':
							// read one uint8 as an argument
							offsetList.push_back(_currentScript->size());
							arg = (uint8)codeStore[pointer];
							pointer += 1;
							break;
						case 'w':
							// read one uint16 as an argument
							offsetList.push_back(_currentScript->size());
							offsetList.push_back(_currentScript->size());
							arg = (uint16)READ_BE_UINT16(&codeStore[pointer]);
							pointer += 2;
							break;
						case 'n':
							// argument is negative
							arg *= -1;
							break;
						case 'p':
							// argument is some kind of denormalised offset
							if (arg % 6) {
								warning("Argument %d was expected to be a multiple of 6", arg);
							}
							arg /= 6;
							break;
						case 'a':
							// argument is a function argument ID
							if (argMap.contains(arg)) {
								arg = argMap[arg];
							} else {
								warning("No argument name found for ID %d", arg);
								arg = -1;
							}
							break;
						case 'v':
							// argument is a local variable ID
							if (varMap.contains(arg)) {
								arg = varMap[arg];
							} else {
								warning("No variable name found for ID %d", arg);
								arg = -1;
							}
							break;
						case 'j':
							// argument refers to a code offset; fix alignment in post
							jumpList.push_back(offsetList.size());
							break;
						default:
							break;
						}
					}
					g_lingo->codeInt(arg);
				}
			} else {
				// unimplemented instruction
				if (opcode < 0x40) { // 1 byte instruction
					debugC(5, kDebugLingoCompile, "Unimplemented opcode: 0x%02x", opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(LC::cb_unk);
					g_lingo->codeInt(opcode);
				} else if (opcode < 0x80) { // 2 byte instruction
					debugC(5, kDebugLingoCompile, "Unimplemented opcode: 0x%02x (%d)", opcode, (uint)codeStore[pointer]);
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(LC::cb_unk1);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					pointer += 1;
				} else { // 3 byte instruction
					debugC(5, kDebugLingoCompile, "Unimplemented opcode: 0x%02x (%d, %d)", opcode, (uint)codeStore[pointer], (uint)codeStore[pointer+1]);
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(LC::cb_unk2);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer+1]);
					pointer += 2;
				}
			}
		}

		// Add backstop
		g_lingo->code1(STOP);

		// Rewrite every offset flagged as a jump based on the new code alignment.
		// This converts the relative offset from the bytecode to an absolute one.
		for (uint j = 0; j < jumpList.size(); j++) {
			int originalJumpAddressLoc = jumpList[j];
			int originalJumpInstructionLoc = originalJumpAddressLoc-1;
			int jumpAddressPc = offsetList[originalJumpAddressLoc];
			int jump = getInt(jumpAddressPc);
			int oldTarget = originalJumpInstructionLoc + jump;
			if ((oldTarget >= 0) && (oldTarget < (int)offsetList.size())) {
				int newJump = offsetList[oldTarget];
				WRITE_UINT32(&((*_currentScript)[jumpAddressPc]), newJump);
			} else {
				warning("Jump of %d from position %d is outside the function!", jump, originalJumpAddressLoc);
			}
		}

		// Attach to handlers
		Symbol *sym = NULL;
		if (nameIndex < _archives[_archiveIndex].names.size()) {
			debugC(5, kDebugLoading, "Function %d binding: %s()", i, _archives[_archiveIndex].names[nameIndex].c_str());
			sym = g_lingo->define(_archives[_archiveIndex].names[nameIndex], argCount, _currentScript);
		} else {
			warning("Function has unknown name id %d, skipping define", nameIndex);
			sym = new Symbol;
			sym->type = HANDLER;
			sym->u.defn = _currentScript;
			sym->nargs = argCount;
			sym->maxArgs = argCount;
		}
		sym->argNames = argNames;
		sym->varNames = varNames;
		sym->ctx = _currentScriptContext;
		sym->archiveIndex = _archiveIndex;
		_currentScriptContext->functions.push_back(sym);

	}

	free(codeStore);
}


void Lingo::addNamesV4(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLingoCompile, "Add V4 script name index");

	if (stream.size() < 0x14) {
		warning("Lnam header too small");
		return;
	}

	// read the Lnam header!
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lnam header:");
		stream.hexdump(0x14);
	}

	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	uint16 offset = stream.readUint16();
	uint16 count = stream.readUint16();

	if (stream.size() < offset) {
		warning("Lnam content missing");
		return;
	}

	stream.seek(offset);

	_archives[_archiveIndex].names.clear();

	Common::Array<Common::String> names;
	for (uint32 i = 0; i < count; i++) {
		uint8 size = stream.readByte();
		Common::String name;
		for (uint8 j = 0; j < size; j++) {
			name += stream.readByte();
		}
		_archives[_archiveIndex].names.push_back(name);
		debugC(5, kDebugLoading, "%d: \"%s\"", i, name.c_str());
	}

}

}
