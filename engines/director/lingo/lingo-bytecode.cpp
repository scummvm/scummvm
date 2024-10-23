/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/script.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-bytecode.h"
#include "director/lingo/lingo-the.h"

namespace Director {

static LingoV4Bytecode lingoV4[] = {
	{ 0x01, LC::c_procret,		"" },
	{ 0x02, LC::c_procret,		"" },
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
	{ 0x18, LC::cb_hilite,		"" },
	{ 0x19, LC::c_intersects,	"" },
	{ 0x1a, LC::c_within,		"" },
	{ 0x1b, LC::c_field,		"" },
	{ 0x1c, LC::c_tell,			"" },
	{ 0x1d, LC::c_telldone,		"" },
	{ 0x1e, LC::cb_list,		"" },
	{ 0x1f, LC::cb_proplist,	"" },

	{ 0x41, LC::c_intpush,		"B" },
	{ 0x42, LC::c_argcnoretpush,"b" },
	{ 0x43, LC::c_argcpush,		"b" },
	// 0x44, push a constant
	{ 0x45, LC::c_namepush,		"bN" },
	{ 0x46, LC::cb_varrefpush,  "bN" },
	{ 0x48, LC::cb_globalpush,	"bN" }, // used in event scripts
	{ 0x49, LC::cb_globalpush,	"bN" },
	{ 0x4a, LC::cb_thepush,		"bN" },
	{ 0x4b, LC::cb_varpush,		"bpaN" },
	{ 0x4c, LC::cb_varpush,		"bpvN" },
	{ 0x4e, LC::cb_globalassign,"bN" }, // used in event scripts
	{ 0x4f, LC::cb_globalassign,"bN" },
	{ 0x50, LC::cb_theassign,	"bN" },
	{ 0x51, LC::cb_varassign,	"bpaN" },
	{ 0x52, LC::cb_varassign,	"bpvN" },
	{ 0x53, LC::c_jump,			"jb" },
	{ 0x54, LC::c_jump,			"jbn" },
	{ 0x55, LC::c_jumpifz,		"jb" },
	{ 0x56, LC::cb_localcall,	"b" },
	{ 0x57, LC::cb_call,		"bN" },
	{ 0x58, LC::cb_objectcall,  "b" },
	{ 0x59, LC::cb_v4assign,	"b" },
	{ 0x5a, LC::cb_v4assign2,	"b" },
	{ 0x5b, LC::cb_delete, 		"b" },
	{ 0x5c, LC::cb_v4theentitypush, "b" },
	{ 0x5d, LC::cb_v4theentityassign, "b" },
	{ 0x5f, LC::cb_thepush2,	"bN" },
	{ 0x60, LC::cb_theassign2,	"bN" },
	{ 0x61, LC::cb_objectfieldpush, "bN" },
	{ 0x62, LC::cb_objectfieldassign, "bN" },
	{ 0x63, LC::cb_call,		"bN" }, // tellcall
	{ 0x64, LC::c_stackpeek, 	"b" },
	{ 0x65, LC::c_stackdrop, 	"b" },
	{ 0x66, LC::cb_v4theentitynamepush, "bN" },

	{ 0x81, LC::c_intpush,		"W" },
	{ 0x82, LC::c_argcnoretpush,"w" },
	{ 0x83, LC::c_argcpush,		"w" },
	// 0x84, push a constant
	{ 0x85, LC::c_namepush,		"wN" },
	{ 0x86, LC::cb_varrefpush,  "wN" },
	{ 0x88, LC::cb_globalpush,	"wN" }, // used in event scripts
	{ 0x89, LC::cb_globalpush,	"wN" },
	{ 0x8a, LC::cb_thepush,		"wN" },
	{ 0x8b, LC::cb_varpush,		"wpaN" },
	{ 0x8c, LC::cb_varpush,		"wpvN" },
	{ 0x8e, LC::cb_globalassign,"wN" }, // used in event scripts
	{ 0x8f, LC::cb_globalassign,"wN" },
	{ 0x90, LC::cb_theassign, 	"wN" },
	{ 0x91, LC::cb_varassign,	"wpaN" },
	{ 0x92, LC::cb_varassign,	"wpvN" },
	{ 0x93, LC::c_jump,			"jw" },
	{ 0x94, LC::c_jump,			"jwn" },
	{ 0x95, LC::c_jumpifz,		"jw" },
	{ 0x96, LC::cb_localcall,	"w" },
	{ 0x97, LC::cb_call,		"wN" },
	{ 0x98, LC::cb_objectcall,  "w" },
	{ 0x99, LC::cb_v4assign,	"w" },
	{ 0x9a, LC::cb_v4assign2,	"w" },
	{ 0x9c, LC::cb_v4theentitypush, "w" },
	{ 0x9d, LC::cb_v4theentityassign, "w" },
	{ 0x9f, LC::cb_thepush2, 	"wN" },
	{ 0xa0, LC::cb_theassign2, "wN" },
	{ 0xa1, LC::cb_objectfieldpush, "wN" },
	{ 0xa2, LC::cb_objectfieldassign, "wN" },
	{ 0xa3, LC::cb_call,		"wN" }, // tellcall
	{ 0xa4, LC::c_stackpeek, 	"w" },
	{ 0xa5, LC::c_stackdrop, 	"w" },
	{ 0xa6, LC::cb_v4theentitynamepush, "wN" },
	{ 0, nullptr, nullptr }
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

	{ 0x02, 0x01, kTheMenu,				kTheName,			false, kTEAMenuId },
	{ 0x02, 0x02, kTheMenuItems,		kTheNumber,			false, kTEAMenuId },

	{ 0x03, 0x01, kTheMenuItem,			kTheName,			true, kTEAMenuIdItemId },
	{ 0x03, 0x02, kTheMenuItem,			kTheCheckMark,		true, kTEAMenuIdItemId },
	{ 0x03, 0x03, kTheMenuItem,			kTheEnabled,		true, kTEAMenuIdItemId },
	{ 0x03, 0x04, kTheMenuItem,			kTheScript,			true, kTEAMenuIdItemId },

	{ 0x04, 0x01, kTheSoundEntity,		kTheVolume,			true, kTEAItemId },

	{ 0x06, 0x01, kTheSprite,			kTheType,			true, kTEAItemId },
	{ 0x06, 0x02, kTheSprite,			kTheBackColor,		true, kTEAItemId },
	{ 0x06, 0x03, kTheSprite,			kTheBottom,			true, kTEAItemId },
	{ 0x06, 0x04, kTheSprite,			kTheCastNum,		true, kTEAItemId },
	{ 0x06, 0x05, kTheSprite,			kTheConstraint,		true, kTEAItemId },
	{ 0x06, 0x06, kTheSprite,			kTheCursor,			true, kTEAItemId },
	{ 0x06, 0x07, kTheSprite,			kTheForeColor,		true, kTEAItemId },
	{ 0x06, 0x08, kTheSprite,			kTheHeight,			true, kTEAItemId },
	{ 0x06, 0x09, kTheSprite,			kTheImmediate,		true, kTEAItemId },
	{ 0x06, 0x0a, kTheSprite,			kTheInk,			true, kTEAItemId },
	{ 0x06, 0x0b, kTheSprite,			kTheLeft,			true, kTEAItemId },
	{ 0x06, 0x0c, kTheSprite,			kTheLineSize,		true, kTEAItemId },
	{ 0x06, 0x0d, kTheSprite,			kTheLocH,			true, kTEAItemId },
	{ 0x06, 0x0e, kTheSprite,			kTheLocV,			true, kTEAItemId },
	{ 0x06, 0x0f, kTheSprite,			kTheMovieRate,		true, kTEAItemId },
	{ 0x06, 0x10, kTheSprite,			kTheMovieTime,		true, kTEAItemId },
	{ 0x06, 0x11, kTheSprite,			kThePattern,		true, kTEAItemId },
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
	{ 0x06, 0x1c, kTheSprite,			kTheBlend,			true, kTEAItemId },
	{ 0x06, 0x1d, kTheSprite,			kTheScriptNum,		true, kTEAItemId },
	{ 0x06, 0x1e, kTheSprite,			kTheMoveableSprite,	true, kTEAItemId },
	{ 0x06, 0x1f, kTheSprite,			kTheEditableText,	true, kTEAItemId },
	{ 0x06, 0x20, kTheSprite,			kTheScoreColor,		true, kTEAItemId },
	{ 0x06, 0x21, kTheSprite,			kTheLoc,			true, kTEAItemId },
	{ 0x06, 0x22, kTheSprite,			kTheRect,			true, kTEAItemId },
	{ 0x06, 0x23, kTheSprite,			kTheMemberNum,		true, kTEAItemId }, // D5
	{ 0x06, 0x24, kTheSprite, 			kTheCastLibNum, 	true, kTEAItemId },
	{ 0x06, 0x25, kTheSprite,			kTheMember,			true, kTEAItemId },

	{ 0x07, 0x01, kTheBeepOn,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x02, kTheButtonStyle,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x03, kTheCenterStage,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x04, kTheCheckBoxAccess,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x05, kTheCheckBoxType,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x06, kTheColorDepth,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x07, kTheColorQD,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x08, kTheExitLock,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x09, kTheFixStageSize,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x0a, kTheFullColorPermit,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x0b, kTheImageDirect,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x0c, kTheDoubleClick,		kTheNOField,		true, kTEANOArgs },
//	{ 0x07, 0x0d, ???,					kTheNOField,		true, kTEANOArgs }, // key
	{ 0x07, 0x0e, kTheLastClick,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x0f, kTheLastEvent,		kTheNOField,		true, kTEANOArgs },
//	{ 0x07, 0x10, ???,					kTheNOField,		true, kTEANOArgs }, // keyCode
	{ 0x07, 0x11, kTheLastKey,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x12, kTheLastRoll,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x13, kTheTimeoutLapsed,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x14, kTheMultiSound,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x15, kThePauseState,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x16, kTheQuickTimePresent,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x17, kTheSelEnd,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x18, kTheSelStart,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x19, kTheSoundEnabled,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1a, kTheSoundLevel,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1b, kTheStageColor,		kTheNOField,		true, kTEANOArgs },
//	{ 0x07, 0x1c, ????,					kTheNOField,		true, kTEANOArgs }, // indicates dontPassEvent was called
	{ 0x07, 0x1d, kTheSwitchColorDepth,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1e, kTheTimeoutKeyDown,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x1f, kTheTimeoutLength,	kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x20, kTheTimeoutMouse,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x21, kTheTimeoutPlay,		kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x22, kTheTimer,			kTheNOField,		true, kTEANOArgs },
	{ 0x07, 0x23, kThePreLoadRAM,		kTheNOField,		true, kTEANOArgs },

	{ 0x08, 0x01, kThePerFrameHook,		kTheNOField,		true, kTEANOArgs },
	{ 0x08, 0x02, kTheCastMembers,		kTheNumber,			false, kTEANOArgs },
	{ 0x08, 0x03, kTheMenus,			kTheNumber,			false, kTEANOArgs },
	{ 0x08, 0x04, kTheCastLibs,			kTheNumber,			false, kTEANOArgs }, // D5
	{ 0x08, 0x05, kTheXtras,			kTheNumber,			false, kTEANOArgs }, // D5

	{ 0x09, 0x01, kTheCast,				kTheName,			true, kTEAItemId },
	{ 0x09, 0x02, kTheCast,				kTheText,			true, kTEAItemId },
	{ 0x09, 0x03, kTheCast,	 			kTheTextStyle,		true, kTEAItemId },
	{ 0x09, 0x04, kTheCast,	 			kTheTextFont,		true, kTEAItemId },
	{ 0x09, 0x05, kTheCast,	 			kTheTextHeight,		true, kTEAItemId },
	{ 0x09, 0x06, kTheCast,	 			kTheTextAlign,		true, kTEAItemId },
	{ 0x09, 0x07, kTheCast,	 			kTheTextSize,		true, kTEAItemId },
	{ 0x09, 0x08, kTheCast,				kThePicture,		true, kTEAItemId },
	{ 0x09, 0x09, kTheCast,				kTheHilite,			true, kTEAItemId },
	{ 0x09, 0x0a, kTheCast,				kTheNumber,			true, kTEAItemId },
	{ 0x09, 0x0b, kTheCast,				kTheSize,			true, kTEAItemId },
	{ 0x09, 0x11, kTheCast,				kTheForeColor,		true, kTEAItemId },
	{ 0x09, 0x12, kTheCast,				kTheBackColor,		true, kTEAItemId },

	// the chunk of cast
	{ 0x0a, 0x03, kTheChunk,			kTheTextStyle,		true, kTEAChunk },
	{ 0x0a, 0x04, kTheChunk,			kTheTextFont,		true, kTEAChunk },
	{ 0x0a, 0x05, kTheChunk,			kTheTextHeight,		true, kTEAChunk },
	{ 0x0a, 0x07, kTheChunk,			kTheTextSize,		true, kTEAChunk },
	{ 0x0a, 0x11, kTheChunk,			kTheForeColor,		true, kTEAChunk },

	{ 0x0b, 0x01, kTheField,			kTheName,			true, kTEAItemId },
	{ 0x0b, 0x02, kTheField,			kTheText,			true, kTEAItemId },
	{ 0x0b, 0x03, kTheField,			kTheTextStyle,		true, kTEAItemId },
	{ 0x0b, 0x04, kTheField,			kTheTextFont,		true, kTEAItemId },
	{ 0x0b, 0x05, kTheField,			kTheTextHeight,		true, kTEAItemId },
	{ 0x0b, 0x06, kTheField,			kTheTextAlign,		true, kTEAItemId },
	{ 0x0b, 0x07, kTheField,			kTheTextSize,		true, kTEAItemId },
	{ 0x0b, 0x09, kTheField,			kTheHilite,			true, kTEAItemId },
	{ 0x0b, 0x11, kTheField,			kTheForeColor,		true, kTEAItemId },

	// the chunk of field
	{ 0x0c, 0x03, kTheChunk,			kTheTextStyle,		true, kTEAChunk },
	{ 0x0c, 0x04, kTheChunk,			kTheTextFont,		true, kTEAChunk },
	{ 0x0c, 0x05, kTheChunk,			kTheTextHeight,		true, kTEAChunk },
	{ 0x0c, 0x07, kTheChunk,			kTheTextSize,		true, kTEAChunk },
	{ 0x0c, 0x11, kTheChunk,			kTheForeColor,		true, kTEAChunk },

	{ 0x0d, 0x0c, kTheCast,				kTheLoop,			true, kTEAItemId },
	{ 0x0d, 0x0d, kTheCast,				kTheDuration,		true, kTEAItemId },
	{ 0x0d, 0x0e, kTheCast,				kTheController,		true, kTEAItemId },
	{ 0x0d, 0x0f, kTheCast,				kTheDirectToStage,	true, kTEAItemId },
	{ 0x0d, 0x10, kTheCast,				kTheSound,			true, kTEAItemId },

	{ 0xff, 0, 0, 0, false, kTEANOArgs }
};


void Lingo::initBytecode() {
	// All new bytecodes must have respective entry in funcDescr[]
	// array in lingo-code.cpp, which is used for decompilation
	//
	// Check that all opcodes have entries
	Common::HashMap<inst, bool> list;
	bool bailout = false;

	// Build reverse hashmap
	for (auto &it : _functions)
		list[(inst)it._key] = true;

	for (LingoV4Bytecode *op = lingoV4; op->opcode; op++) {
		_lingoV4[op->opcode] = op;

		if (!list.contains(op->func)) {
			warning("Lingo::initBytecode(): Missing prototype for opcode 0x%02x", op->opcode);
			bailout = true;
		}
	}

	if (bailout)
		error("Lingo::initBytecode(): Add entries to funcDescr[] in lingo-code.cpp");

	for (LingoV4TheEntity *ent = lingoV4TheEntity; ent->bank != 0xff; ent++) {
		_lingoV4TheEntity[(ent->bank << 8) + ent->firstArg] = ent;
	}
}

Datum Lingo::findVarV4(int varType, const Datum &id) {
	Datum res;
	switch (varType) {
	case 1: // global
	case 2: // global
	case 3: // property/instance
		if (id.type == SYMBOL) {
			res = id;
			res.type = (varType == 3) ? PROPREF : GLOBALREF;
		} else {
			warning("BUILDBOT: findVarV4: expected ID for var type %d to be SYMBOL, got %s", varType, id.type2str());
		}
		break;
	case 4: // arg
	case 5: // local
		{
			Common::Array<CFrame *> &callstack = _state->callstack;
			if (callstack.empty()) {
				warning("BUILDBOT: findVarV4: no call frame");
				return res;
			}
			int stride = 6;
			if (g_director->getVersion() >= 500) {
				stride = 8;
			}
			if (id.asInt() % stride != 0) {
				warning("BUILDBOT: findVarV4: invalid var ID %d for var type %d (not divisible by %d)", id.asInt(), varType, stride);
				return res;
			}
			int varIndex = id.asInt() / stride;
			Common::Array<Common::String> *varNames = (varType == 4)
				? callstack.back()->sp.argNames
				: callstack.back()->sp.varNames;

			if (varIndex < (int)varNames->size()) {
				res = (*varNames)[varIndex];
				res.type = LOCALREF;
			} else {
				warning("BUILDBOT: findVarV4: invalid var ID %d for var type %d (too high)", id.asInt(), varType);
			}
		}
		break;
	case 6: // field
		if (g_director->getVersion() < 500) {
			res = id.asMemberID();
		} else {
			Datum castName = g_lingo->pop();
			res = castName.asMemberID(kCastTypeAny, id.asInt());
		}
		res.type = FIELDREF;
		break;
	default:
		warning("BUILDBOT: findVarV4: unhandled var type %d", varType);
		break;
	}
	return res;
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

void LC::cb_delete() {
	int varType = g_lingo->readInt();
	Datum varID = g_lingo->pop();
	Datum var = g_lingo->findVarV4(varType, varID);
	Datum chunkRef = readChunkRef(var);
	g_lingo->push(chunkRef);
	LC::c_delete();
}

void LC::cb_hilite() {
	Datum fieldID = g_lingo->pop().asMemberID();
	fieldID.type = FIELDREF;
	Datum chunkRef = readChunkRef(fieldID);
	g_lingo->push(chunkRef);
	LC::c_hilite();
}

void LC::cb_localcall() {
	int functionId = g_lingo->readInt();

	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		Common::String name = g_lingo->_state->context->_functionNames[functionId];
		if (debugChannelSet(3, kDebugLingoExec))
			g_lingo->printArgs(name.c_str(), nargs.u.i, "localcall:");

		LC::call(name, nargs.u.i, nargs.type == ARGC);

	} else {
		warning("cb_localcall: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

}


void LC::cb_objectcall() {
	int varType = g_lingo->readInt();
	Datum varId = g_lingo->pop();
	Datum nargs = g_lingo->pop();

	Datum var = g_lingo->findVarV4(varType, varId);
	if (!var.isVarRef()) {
		warning("cb_objectcall: first arg did not resolve to variable");
		return;
	}

	if ((nargs.type != ARGC) && (nargs.type != ARGCNORET)) {
		warning("cb_objectcall: second arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
		return;
	}

	if (nargs.u.i > 0) {
		Datum &firstArg = g_lingo->_stack[g_lingo->_stack.size() - nargs.u.i];
		// The first arg could be either a method name or a variable name
		if (firstArg.type == SYMBOL) {
			firstArg.type = VARREF;
		}
	}

	LC::call(*var.u.s, nargs.u.i, nargs.type == ARGC);
}


void LC::cb_v4assign() {
	int arg = g_lingo->readInt();
	int op = (arg >> 4) & 0xF;
	int varType = arg & 0xF;
	Datum varId = g_lingo->pop();

	Datum var = g_lingo->findVarV4(varType, varId);
	g_lingo->push(var);

	switch (op) {
	case 1:
		// put value into var
		LC::c_assign();
		break;
	case 2:
		// put value after var
		LC::c_putafter();
		break;
	case 3:
		// put value before var
		LC::c_putbefore();
		break;
	default:
		warning("cb_v4assign: unknown operator %d", op);
		g_lingo->pop();
		break;
	}
}


void LC::cb_list() {
	Datum nargs = g_lingo->pop();
	if ((nargs.type != ARGC) && (nargs.type != ARGCNORET)) {
		error("cb_list: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}
	LB::b_list(nargs.u.i);
}


void LC::cb_proplist() {
	Datum nargs = g_lingo->pop();
	if ((nargs.type != ARGC) && (nargs.type != ARGCNORET)) {
		error("cb_proplist: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}
	int arraySize = nargs.u.i;
	if (arraySize % 2) {
		warning("cb_proplist: list should have an even number of entries, ignoring the last one");
	}

	Datum result;
	result.type = PARRAY;
	result.u.parr = new PArray;
	arraySize /= 2;

	for (int i = 0; i < arraySize; i++) {
		Datum v = g_lingo->pop();
		Datum p = g_lingo->pop();

		PCell cell = PCell(p, v);
		result.u.parr->arr.insert_at(0, cell);
	};

	if (nargs.u.i % 2)
		g_lingo->pop();

	g_lingo->push(result);
}


void LC::cb_call() {
	Common::String name = g_lingo->readString();

	Datum nargs = g_lingo->pop();
	if ((nargs.type == ARGC) || (nargs.type == ARGCNORET)) {
		LC::call(name, nargs.u.i, nargs.type == ARGC);

	} else {
		warning("cb_call: first arg should be of type ARGC or ARGCNORET, not %s", nargs.type2str());
	}

}


void LC::cb_globalpush() {
	Common::String name = g_lingo->readString();
	Datum target(name);
	target.type = GLOBALREF;
	debugC(3, kDebugLingoExec, "cb_globalpush: pushing %s to stack", name.c_str());
	Datum result = g_lingo->varFetch(target);
	g_lingo->push(result);
}


void LC::cb_globalassign() {
	Common::String name = g_lingo->readString();
	Datum target(name);
	target.type = GLOBALREF;
	debugC(3, kDebugLingoExec, "cb_globalassign: assigning to %s", name.c_str());
	Datum source = g_lingo->pop();
	g_lingo->varAssign(target, source);
}

void LC::cb_objectfieldassign() {
	Common::String fieldName = g_lingo->readString();
	Datum value = g_lingo->pop();
	Datum object = g_lingo->pop();
	g_lingo->setObjectProp(object, fieldName, value);
}

void LC::cb_objectfieldpush() {
	Common::String fieldName = g_lingo->readString();
	Datum object = g_lingo->pop();
	g_lingo->getObjectProp(object, fieldName);
}

void LC::cb_varrefpush() {
	Common::String name = g_lingo->readString();
	Datum result(name);
	result.type = SYMBOL;
	g_lingo->push(result);
}

void LC::cb_theassign() {
	// cb_theassign is for setting script/factory-level properties
	Common::String name = g_lingo->readString();
	Datum value = g_lingo->pop();
	if (g_lingo->_state->me.type == OBJECT) {
		// Don't bother checking if the property is defined, leave that to the object.
		// For D3-style anonymous objects/factories, you are allowed to define whatever properties you like.
		g_debugger->propWriteHook(name);
		g_lingo->_state->me.u.obj->setProp(name, value);
	} else {
		warning("cb_theassign: no me object");
	}
}

void LC::cb_theassign2() {
	// cb_theassign2 is for setting movie-level properties
	Common::String name = g_lingo->readString();
	Datum value = g_lingo->pop();

	if (g_lingo->_theEntities.contains(name)) {
		TheEntity *entity = g_lingo->_theEntities[name];
		Datum id;
		id.u.i = 0;
		id.type = VOID;
		g_lingo->setTheEntity(entity->entity, id, kTEANOArgs, value);
	} else {
		warning("LC::cb_theassign2 Can't assign theEntity: (%s)", name.c_str());
	}
}

void LC::cb_thepush() {
	Common::String name = g_lingo->readString();
	if (g_lingo->_state->me.type == OBJECT) {
		if (g_lingo->_state->me.u.obj->hasProp(name)) {
			g_lingo->push(g_lingo->_state->me.u.obj->getProp(name));
			g_debugger->propReadHook(name);
			return;
		}

		warning("cb_thepush: me object has no property '%s', type: %d", name.c_str(), g_lingo->_state->me.type);
	} else {
		debugC(1, kDebugLingoExec, "cb_thepush: attempted to access property '%s' with no me object, returning VOID", name.c_str());
	}
	g_lingo->pushVoid();
}

void LC::cb_thepush2() {
	Datum result;
	Common::String name = g_lingo->readString();
	if (g_lingo->_theEntities.contains(name)) {
		TheEntity *entity = g_lingo->_theEntities[name];
		Datum id;
		id.u.i = 0;
		id.type = VOID;
		debugC(3, kDebugLingoExec, "cb_thepush: pushing value of entity %s to stack", name.c_str());
		result = g_lingo->getTheEntity(entity->entity, id, kTEANOArgs);
	} else {
		warning("LC::cb_thepush2 Can't find theEntity: (%s)", name.c_str());
		result.type = VOID;
	}
	g_lingo->push(result);
}

void LC::cb_varpush() {
	Common::String name = g_lingo->readString();
	Datum target(name);
	target.type = LOCALREF;
	debugC(3, kDebugLingoExec, "cb_varpush: pushing %s to stack", name.c_str());
	Datum result = g_lingo->varFetch(target);
	g_lingo->push(result);
}


void LC::cb_varassign() {
	Common::String name = g_lingo->readString();
	Datum target(name);
	target.type = LOCALREF;
	debugC(3, kDebugLingoExec, "cb_varassign: assigning to %s", name.c_str());
	Datum source = g_lingo->pop();
	// Local variables should be initialised by the script, no varCreate here
	g_lingo->varAssign(target, source);
}


void LC::cb_v4assign2() {
int arg = g_lingo->readInt();
	int op = (arg >> 4) & 0xF;
	int varType = arg & 0xF;
	Datum varId = g_lingo->pop();

	Datum var = g_lingo->findVarV4(varType, varId);
	Datum ref = readChunkRef(var);
	g_lingo->push(ref);

	switch (op) {
	case 1:
		// put value into chunk
		LC::c_assign();
		break;
	case 2:
		// put value after chunk
		LC::c_putafter();
		break;
	case 3:
		// put value before chunk
		LC::c_putbefore();
		break;
	default:
		warning("cb_v4assign2: unknown operator %d", op);
		g_lingo->pop();
		break;
	}
}


void LC::cb_v4theentitypush() {
	int bank = g_lingo->readInt();

	int firstArg = g_lingo->pop().asInt();
	Datum result;
	result.u.s = nullptr;
	result.type = VOID;

	int key = (bank << 8) + firstArg;
	if (g_lingo->_lingoV4TheEntity.contains(key)) {
		debugC(3, kDebugLingoExec, "cb_v4theentitypush: mapping 0x%02x, 0x%02x", bank, firstArg);
		int entity = g_lingo->_lingoV4TheEntity[key]->entity;
		int field = g_lingo->_lingoV4TheEntity[key]->field;
		switch (g_lingo->_lingoV4TheEntity[key]->type) {
		case kTEANOArgs:
			{
				Datum id;
				id.u.i = 0;
				id.type = VOID;
				debugC(3, kDebugLingoExec, "cb_v4theentitypush: calling getTheEntity(%s, VOID, %s)", g_lingo->entity2str(entity), g_lingo->field2str(field));
				result = g_lingo->getTheEntity(entity, id, field);
			}
			break;
		case kTEAItemId:
			{
				Datum id = g_lingo->pop();
				if (entity == kTheCast && g_director->getVersion() >= 500) {
					// For "the member", D5 and above have a lib ID followed by a member ID
					// Pre-resolve them here.
					CastMemberID resolved = g_lingo->resolveCastMember(g_lingo->pop(), id, kCastTypeAny);
					id = Datum(resolved);
				}
				debugC(3, kDebugLingoExec, "cb_v4theentitypush: calling getTheEntity(%s, %s, %s)", g_lingo->entity2str(entity), id.asString(true).c_str(), g_lingo->field2str(field));
				result = g_lingo->getTheEntity(entity, id, field);
			}
			break;
		case kTEAMenuId:
			{
				Datum id = g_lingo->pop();
				debugC(3, kDebugLingoExec, "cb_v4theentitypush: calling getTheEntity(%s, %s, %s)", g_lingo->entity2str(entity), id.asString(true).c_str(), g_lingo->field2str(field));
				if (id.type == INT) {
					int menuId = id.u.i;
					id.u.menu = new MenuReference();
					id.u.menu->menuIdNum = menuId;
				} else if (id.type == STRING) {
					Common::String *menuId = id.u.s;
					id.u.menu = new MenuReference();
					id.u.menu->menuIdStr = menuId;
				} else {
					warning("LC::cb_v4theentitypush : Unknown type of menu Reference %d of entity type %d", id.type, g_lingo->_lingoV4TheEntity[key]->type);
					break;
				}
				id.type = MENUREF;

				result = g_lingo->getTheEntity(entity, id, field);
			}
			break;
		case kTEAString:
			{
				Datum stringArg = g_lingo->pop();
				ChunkType chunkType = kChunkChar;
				switch (entity) {
				case kTheChars:
					chunkType = kChunkChar;
					break;
				case kTheWords:
					chunkType = kChunkWord;
					break;
				case kTheLines:
					chunkType = kChunkLine;
					break;
				case kTheItems:
					chunkType = kChunkItem;
					break;
				}
				Datum chunkRef = LC::lastChunk(chunkType, stringArg);
				switch (field) {
				case kTheLast:
					result = chunkRef.eval();
					break;
				case kTheNumber:
					result = chunkRef.u.cref->startChunk;
					break;
				}
			}
			break;
		case kTEAMenuIdItemId:
			{
				/*Datum menuId = */g_lingo->pop();
				/*Datum itemId = */g_lingo->pop();
				warning("cb_v4theentitypush: STUB: kTEAMenuIdItemId");
			}
			break;
		case kTEAChunk:
			{
				Datum fieldRef = g_lingo->pop().asMemberID();
				fieldRef.type = FIELDREF;
				Datum chunkRef = readChunkRef(fieldRef);
				result = g_lingo->getTheEntity(entity, chunkRef, field);
			}
			break;
		default:
			warning("cb_v4theentitypush: unknown call type %d", g_lingo->_lingoV4TheEntity[key]->type);
			break;
		}
	} else {
		warning("cb_v4theentitypush: BUILDBOT: unhandled mapping 0x%02x 0x%02x", bank, firstArg);
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

	Common::String name = g_lingo->readString();

	Datum id;
	id.u.s = nullptr;
	id.type = VOID;

	if (!g_lingo->_theEntities.contains(name)) {
		warning("BUILDBOT: cb_v4theentitynamepush: missing the entity %s", name.c_str());
		g_lingo->push(Datum());
		return;
	}
	TheEntity *entity = g_lingo->_theEntities[name];

	debugC(3, kDebugLingoExec, "cb_v4theentitynamepush: %s", name.c_str());
	debugC(3, kDebugLingoExec, "cb_v4theentitynamepush: calling getTheEntity(%s, VOID, kTheNOField)", g_lingo->entity2str(entity->entity));
	Datum result = g_lingo->getTheEntity(entity->entity, id, kTheNOField);

	g_lingo->push(result);
}


void LC::cb_v4theentityassign() {
	int bank = g_lingo->readInt();

	int firstArg = g_lingo->pop().asInt();
	Datum value = g_lingo->pop();
	Datum result;
	result.u.s = nullptr;
	result.type = VOID;

	int key = (bank << 8) + firstArg;
	if (!g_lingo->_lingoV4TheEntity.contains(key)) {
		warning("cb_v4theentityassign: unhandled mapping 0x%02x 0x%02x", bank, firstArg);

		return;
	}

	debugC(3, kDebugLingoExec, "cb_v4theentityassign: mapping 0x%02x, 0x%02x", bank, firstArg);

	if (!g_lingo->_lingoV4TheEntity[key]->writable) {
		warning("cb_v4theentityassign: non-writable mapping 0x%02x 0x%02x", bank, firstArg);

		return;
	}

	int entity = g_lingo->_lingoV4TheEntity[key]->entity;
	int field = g_lingo->_lingoV4TheEntity[key]->field;
	switch (g_lingo->_lingoV4TheEntity[key]->type) {
	case kTEANOArgs:
		{
			Datum id;
			id.u.s = nullptr;
			id.type = VOID;
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: calling setTheEntity(%s, VOID, %s, %s)", g_lingo->entity2str(entity), g_lingo->field2str(field), value.asString(true).c_str());
			g_lingo->setTheEntity(entity, id, field, value);
		}
		break;
	case kTEAItemId:
		{
			Datum id = g_lingo->pop();
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: calling setTheEntity(%s, %s, %s, %s)", g_lingo->entity2str(entity), id.asString(true).c_str(), g_lingo->field2str(field), value.asString(true).c_str());
			g_lingo->setTheEntity(entity, id, field, value);
		}
		break;
	case kTEAMenuId:
		{
			Datum id = g_lingo->pop();
			if (id.type == INT) {
				int menuId = id.u.i;
				id.u.menu = new MenuReference();
				id.u.menu->menuIdNum = menuId;
			} else if (id.type == STRING) {
				Common::String *menuId = id.u.s;
				id.u.menu = new MenuReference();
				id.u.menu->menuIdStr = menuId;
			} else {
				warning("LC::cb_v4theentityassign : Unknown type of menu Reference %d of entity type %d", id.type, g_lingo->_lingoV4TheEntity[key]->type);
				break;
			}
			id.type = MENUREF;
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: calling setTheEntity(%s, %s, %s, %s)", g_lingo->entity2str(entity), id.asString(true).c_str(), g_lingo->field2str(field), value.asString(true).c_str());
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
			Datum menuId = g_lingo->pop();
			Datum itemId = g_lingo->pop();
			Datum menuDatum;
			menuDatum.type = MENUREF;
			menuDatum.u.menu = new MenuReference();
			if (menuId.type == INT) {
				menuDatum.u.menu->menuIdNum = menuId.u.i;
			} else if (menuId.type == STRING) {
				menuDatum.u.menu->menuIdStr = menuId.u.s;
			} else {
				warning("LC::cb_v4theentityassign : Unknown type of menu Reference %d of entity type %d", menuId.type, g_lingo->_lingoV4TheEntity[key]->type);
				break;
			}
			if (itemId.type == INT) {
				menuDatum.u.menu->menuItemIdNum = itemId.u.i;
			} else if (itemId.type == STRING) {
				menuDatum.u.menu->menuItemIdStr = itemId.u.s;
			} else {
				warning("LC::cb_v4theentityassign : Unknown type of menuItem Reference %d of entity type %d", itemId.type, g_lingo->_lingoV4TheEntity[key]->type);
				break;
			}
			g_lingo->setTheEntity(entity, menuDatum, field, value);
		}
		break;
	case kTEAChunk:
		{
			Datum fieldRef = g_lingo->pop().asMemberID();
			fieldRef.type = FIELDREF;
			Datum chunkRef = readChunkRef(fieldRef);
			g_lingo->setTheEntity(entity, chunkRef, field, value);
		}
		break;
	default:
		warning("cb_v4theentityassign: unknown call type %d", g_lingo->_lingoV4TheEntity[key]->type);
		break;
	}
}

void LC::cb_zeropush() {
	Datum d(0);
	g_lingo->push(d);
}

ScriptContext *LingoCompiler::compileLingoV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, LingoArchive *archive, const Common::String &archName, uint16 version) {
	if (stream.size() < 0x5c) {
		warning("Lscr header too small");
		return nullptr;
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr header:");
		stream.hexdump(0x5c);
	}

	// read the Lscr header!
	// documentation:
	// https://docs.google.com/document/d/1jDBXE4Wv1AEga-o1Wi8xtlNZY4K2fHxW2Xs8RgARrqk/edit
	// https://github.com/Earthquake-Project/Format-Documentation/blob/master/structure/scripting/FormatNotes_Scripts.txt
	// https://github.com/Earthquake-Project/ProjectorRays/blob/master/src/chunk/Script.ts
	// (none of the above are totally complete)

	// unk1
	for (uint32 i = 0; i < 0x8; i++) {
		stream.readByte();
	}

	// offset 8
	/* uint32 length = */ stream.readUint32();
	/* uint32 length2 = */ stream.readUint32();
	uint16 codeStoreOffset = stream.readUint16();

	/* uint16 scriptId = */ stream.readUint16() /* + 1 */;
	// This field *should* match the script's index in Lctx, but this
	// is unreliable. (e.g. script 261 in DATA/LEVEL1.DIR in betterd-win
	// has this field incorrectly set to 263 instead of 261.)
	// Thus, ignore it and simply use the script's index in Lctx.
	uint16 scriptId = lctxIndex;

	// unk2
	for (uint32 i = 0; i < 0x10; i++) {
		stream.readByte();
	}

	// offset 36
	/* uint16 unk3 = */ stream.readUint16();
	uint32 scriptFlags = stream.readUint32();
	debugC(1, kDebugCompile, "Script flags (%d 0x%x):", scriptFlags, scriptFlags);
	debugC(1, kDebugCompile, "unused: %d funcsGlobal: %d varsGlobal: %d unk3: %d", (scriptFlags & kScriptFlagUnused) != 0, (scriptFlags & kScriptFlagFuncsGlobal) != 0, (scriptFlags & kScriptFlagVarsGlobal) != 0, (scriptFlags & kScriptFlagUnk3) != 0);
	debugC(1, kDebugCompile, "factoryDef: %d unk5: %d unk6: %d unk7: %d", (scriptFlags & kScriptFlagFactoryDef) != 0, (scriptFlags & kScriptFlagUnk5) != 0, (scriptFlags & kScriptFlagUnk6) != 0, (scriptFlags & kScriptFlagUnk7) != 0);
	debugC(1, kDebugCompile, "hasFactory: %d eventScript: %d eventScript2: %d unkB: %d", (scriptFlags & kScriptFlagHasFactory) != 0, (scriptFlags & kScriptFlagEventScript) != 0, (scriptFlags & kScriptFlagEventScript2) != 0, (scriptFlags & kScriptFlagUnkB) != 0);
	debugC(1, kDebugCompile, "unkC: %d unkD: %d unkE: %d unkF: %d", (scriptFlags & kScriptFlagUnkC) != 0, (scriptFlags & kScriptFlagUnkD) != 0, (scriptFlags & kScriptFlagUnkE) != 0, (scriptFlags & kScriptFlagUnkF) != 0);

	if (scriptFlags & kScriptFlagUnused) {
		warning("Script %d is unused", scriptId);
		return nullptr;
	}

	// unk4
	for (uint32 i = 0; i < 0x4; i++) {
		stream.readByte();
	}
	/* uint16 _assemblyId = */ stream.readUint16();
	// The script is coupled with to a Cast via the script ID.
	// This _assemblyId isn't always correct.

	int16 factoryNameId = stream.readSint16();

	// offset 50 - contents map

	/* uint16 eventMapCount = */ stream.readUint16();
	/* uint32 eventMapOffset = */ stream.readUint32();
	/* uint32 eventMapFlags = */ stream.readUint32();
	// The event map is an int16 array used to quickly access events.
	// Its first item is the index of the mouseDown handler or -1,
	// its second item is the index of the mouseUp handler or -1, etc.
	// eventMapFlags & (1 << 0) indicates there is a mouseDown handler,
	// eventMapFlags & (1 << 1) indicates there is a mouseUp handler, etc.
	// We probably don't need to read this since we already did something
	// similar with _eventHandlers.

	uint16 propertiesCount = stream.readUint16();
	uint32 propertiesOffset = stream.readUint32();
	uint16 globalsCount = stream.readUint16();
	uint32 globalsOffset = stream.readUint32();
	uint16 functionsCount = stream.readUint16();
	uint32 functionsOffset = stream.readUint32();
	uint16 constsCount = stream.readUint16();
	uint32 constsOffset = stream.readUint32();
	/* uint16 constsStoreCount = */ stream.readUint32();
	uint32 constsStoreOffset = stream.readUint32();

	// initialise the script
	ScriptType scriptType = kCastScript;
	Common::String castName;
	CastMember *member = archive->cast->getCastMemberByScriptId(scriptId);
	if (member) {
		if (member->_type == kCastLingoScript)
			scriptType = ((ScriptCastMember *)member)->_scriptType;

		_assemblyId = member->getID();
		CastMemberInfo *info = member->getInfo();
		if (info)
			castName = info->name;
	} else {
		warning("Script %d has no associated cast member", scriptId);
		scriptType = kNoneScript;
	}

	_assemblyArchive = archive;

	ScriptContext *sc = nullptr;
	Common::String factoryName;
	if (scriptFlags & kScriptFlagFactoryDef) {
		if (0 <= factoryNameId && factoryNameId < (int16)archive->names.size()) {
			factoryName = archive->names[factoryNameId];
		} else {
			warning("Factory %d has unknown name id %d, skipping define", scriptId, factoryNameId);
			return nullptr;
		}
		debugC(1, kDebugCompile, "Add V4 script %d: factory '%s'", scriptId, factoryName.c_str());

		sc = _assemblyContext = new ScriptContext(factoryName, scriptType, _assemblyId);
		registerFactory(factoryName);
	} else {
		debugC(1, kDebugCompile, "Add V4 script %d: %s %d", scriptId, scriptType2str(scriptType), _assemblyId);

		sc = _assemblyContext = new ScriptContext(!castName.empty() ? castName : Common::String::format("%d", _assemblyId), scriptType, _assemblyId);
	}

	// initialise each property
	if ((uint32)stream.size() < propertiesOffset + propertiesCount * 2) {
		warning("Lscr properties store missing");
		return nullptr;
	}
	stream.seek(propertiesOffset);
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr property list:");
		stream.hexdump(propertiesCount * 2);
	}
	for (uint16 i = 0; i < propertiesCount; i++) {
		int16 index = stream.readSint16();
		if (index == -1) {
			debugC(5, kDebugLoading, "[end of list]");
			break;
		} else if (0 <= index && index < (int16)archive->names.size()) {
			const char *name = archive->names[index].c_str();
			debugC(5, kDebugLoading, "%d: %s", i, name);
			_assemblyContext->setProp(name, Datum(), true);
		} else {
			warning("Property %d has unknown name id %d, skipping define", i, index);
		}
	}

	// initialise each global variable
	if ((uint32)stream.size() < globalsOffset + globalsCount * 2) {
		warning("Lscr globals store missing");
		return nullptr;
	}

	stream.seek(globalsOffset);
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr globals list:");
		stream.hexdump(globalsCount * 2);
	}
	for (uint16 i = 0; i < globalsCount; i++) {
		int16 index = stream.readSint16();
		if (0 <= index && index < (int16)archive->names.size()) {
			const char *name = archive->names[index].c_str();
			if (!g_lingo->_globalvars.contains(name)) {
				g_lingo->_globalvars[name] = Datum();
				debugC(5, kDebugLoading, "%d: %s", i, name);
			} else {
				debugC(5, kDebugLoading, "%d: %s (already defined)", i, name);
			}
		} else {
			warning("Global %d has unknown name id %d, skipping define", i, index);
		}
	}

	// preload all the constants!
	// these are stored as a reference table of 6 byte entries, followed by a storage area.

	// copy the storage area first.
	uint32 constsStoreSize = stream.size() - constsStoreOffset;

	if ((uint32)stream.size() < constsStoreOffset) {
		warning("Lscr consts store missing");
		return nullptr;
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
	int constsIndexSize = MAX((int)constsStoreOffset - (int)constsOffset, 0);
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "Lscr consts index:");
		stream.hexdump(constsIndexSize);
	}
	for (uint16 i = 0; i < constsCount; i++) {
		Datum constant;
		uint32 constType = 0;
		if (version >= kFileVer500) {
			constType = stream.readUint32();
		} else {
			constType = (uint32)stream.readUint16();
		}

		uint32 value = stream.readUint32();
		switch (constType) {
		case 1: // String type
			{
				Common::String str;
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
					if (constsStore[pointer] == '\0') {
						break;
					} else {
						str += constsStore[pointer];
					}
					pointer += 1;
				}
				if (pointer > constsStoreSize) {
					warning("Constant string has no null terminator");
					break;
				}
				constant.type = STRING;
				constant.u.s = new Common::String(archive->cast->decodeString(str), Common::kUtf8);
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

				if (length == 10) {
					// Floats are stored as an "80 bit IEEE Standard 754 floating
					// point number (Standard Apple Numeric Environment [SANE] data type
					// Extended).
					constant.u.f = readAppleFloat80(&constsStore[pointer]);
				} else if (length == 8) {
					constant.u.f = READ_BE_FLOAT64(&constsStore[pointer]);
				} else {
					error("Constant float expected to be 8 or 10 bytes but got %d", length);
				}

			}
			break;
		default:
			warning("Unknown constant type %d", constType);
			break;
		}
		_assemblyContext->_constants.push_back(constant);
	}
	free(constsStore);

	// parse each function!
	// these are stored as a code storage area, followed by a reference table of 42 byte entries.

	// copy the storage area first.
	if ((uint32)stream.size() < functionsOffset) {
		warning("Lscr functions store missing");
		return nullptr;
	}

	uint32 codeStoreSize = functionsOffset - codeStoreOffset;
	stream.seek(codeStoreOffset);
	byte *codeStore = (byte *)malloc(codeStoreSize);
	stream.read(codeStore, codeStoreSize);

	Common::DumpFile out;
	bool skipdump = false;

	if (ConfMan.getBool("dump_scripts")) {
		Common::Path buf;
		if (scriptFlags & kScriptFlagFactoryDef) {
			buf = dumpFactoryName(encodePathForDump(archName).c_str(), factoryName.c_str(), "lscr");
		} else {
			buf = dumpScriptName(encodePathForDump(archName).c_str(), scriptType, _assemblyId, "lscr");
		}

		if (!out.open(buf, true)) {
			warning("Lingo::addCodeV4(): Can not open dump file %s", buf.toString(Common::Path::kNativeSeparator).c_str());
			skipdump = true;
		} else {
			warning("Lingo::addCodeV4(): Dumping Lscr to %s", buf.toString(Common::Path::kNativeSeparator).c_str());
		}
	}

	// read each entry in the function table.
	stream.seek(functionsOffset);
	for (uint16 i = 0; i < functionsCount; i++) {
		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Function %d header:", i);
			stream.hexdump(0x2a);
		}

		int16 nameIndex = stream.readUint16();
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
				int16 index = READ_BE_INT16(&codeStore[namePointer]);
				namePointer += 2;
				Common::String name;
				if (0 <= index && index < (int16)archive->names.size()) {
					name = archive->names[index];
					argMap[j] = index;
				} else if (j == 0 && (scriptFlags & kScriptFlagFactoryDef)) {
					name = "me";

					// find or add "me" in the names list
					for (index = 0; index < (int16)archive->names.size(); index++) {
						if (archive->names[index].equalsIgnoreCase(name))
							break;
					}
					if (index == (int16)archive->names.size())
						archive->names.push_back(name);

					argMap[j] = index;
				} else {
					name = Common::String::format("arg_%d", j);
					warning("Argument has unknown name id %d, using name %s", index, name.c_str());
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
				int16 index = READ_BE_INT16(&codeStore[namePointer]);
				namePointer += 2;
				Common::String name;
				if (0 <= index && index < (int16)archive->names.size()) {
					name = archive->names[index];
					varMap[j] = index;
				} else {
					name = Common::String::format("var_%d", j);
					warning("Variable has unknown name id %d, using name %s", j, name.c_str());
				}
				debugC(5, kDebugLoading, "%d: %s", j, name.c_str());
				varNames->push_back(name);
			}
		}

		_currentAssembly = new ScriptData;

		if (debugChannelSet(5, kDebugLoading)) {
			debugC(5, kDebugLoading, "Function %d code:", i);
			Common::hexdump(codeStore, length, 16, startOffset);
		}

		uint32 pointer = startOffset - codeStoreOffset;
		Common::Array<uint32> offsetList;
		Common::Array<uint32> jumpList;
		Common::Array<uint32> byteOffsets;

		// Size of an entry in the consts index.
		int constEntrySize = 0;
		if (version >= kFileVer500) {
			// For D5 this is uint32 type + uint32 offset
			constEntrySize = 8;
		} else {
			// For D4 this is uint16 type + uint32 offset
			constEntrySize = 6;
		}

		while (pointer < startOffset + length - codeStoreOffset) {
			uint8 opcode = codeStore[pointer];
			pointer += 1;

			if (opcode == 0x44 || opcode == 0x84) {
				// Opcode for pushing a value from the constants table.
				// Rewrite these to inline the constant into our bytecode.
				offsetList.push_back(_currentAssembly->size());
				byteOffsets.push_back(_currentAssembly->size());
				int arg = 0;
				if (opcode == 0x84) {
					arg = (uint16)READ_BE_UINT16(&codeStore[pointer]);
					pointer += 2;
				} else {
					arg = (uint8)codeStore[pointer];
					pointer += 1;
				}
				// The argument is a byte offset to an entry in the consts index.
				// As such, it should be an exact multiple of the entry size.
				if (arg % constEntrySize) {
					warning("Opcode 0x%02x arg %d not a multiple of %d", opcode, arg, constEntrySize);
				}
				arg /= constEntrySize;
				Datum constant = _assemblyContext->_constants[arg];
				switch (constant.type) {
				case INT:
					code1(LC::c_intpush);
					break;
				case FLOAT:
					code1(LC::c_floatpush);
					break;
				case STRING:
					code1(LC::c_stringpush);
					break;
				default:
					error("Unknown constant type %d", constant.type);
					break;
				}
				if (opcode == 0x84) {
					offsetList.push_back(_currentAssembly->size());
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
				} else {
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
				}
				switch (constant.type) {
				case INT:
					codeInt(constant.u.i);
					break;
				case FLOAT:
					codeFloat(constant.u.f);
					break;
				case STRING:
					codeString(constant.u.s->c_str());
					break;
				default:
					error("Unknown constant type %d", constant.type);
					break;
				}
			} else if (g_lingo->_lingoV4.contains(opcode)) {
				offsetList.push_back(_currentAssembly->size());
				byteOffsets.push_back(_currentAssembly->size());
				code1(g_lingo->_lingoV4[opcode]->func);

				size_t argc = strlen(g_lingo->_lingoV4[opcode]->proto);
				if (argc) {
					bool codeName = false;
					int arg = 0;
					for (uint c = 0; c < argc; c++) {
						switch (g_lingo->_lingoV4[opcode]->proto[c]) {
						case 'b':
							// read one uint8 as an argument
							offsetList.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							arg = (uint8)codeStore[pointer];
							pointer += 1;
							break;
						case 'B':
							// read one int8 as an argument
							offsetList.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							arg = (int8)codeStore[pointer];
							pointer += 1;
							break;
						case 'w':
							// read one uint16 as an argument
							offsetList.push_back(_currentAssembly->size());
							offsetList.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							arg = (uint16)READ_BE_UINT16(&codeStore[pointer]);
							pointer += 2;
							break;
						case 'W':
							// read one int16 as an argument
							offsetList.push_back(_currentAssembly->size());
							offsetList.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							byteOffsets.push_back(_currentAssembly->size());
							arg = (int16)READ_BE_INT16(&codeStore[pointer]);
							pointer += 2;
							break;
						case 'n':
							// argument is negative
							arg *= -1;
							break;
						case 'p':
							// argument is some kind of denormalised offset
							if (arg % constEntrySize) {
								warning("Argument %d was expected to be a multiple of %d", arg, constEntrySize);
							}
							arg /= constEntrySize;
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
						case 'N':
							// argument is a name in the name table
							codeName = true;
							break;
						default:
							break;
						}
					}
					if (codeName) {
						codeString(_assemblyArchive->getName(arg).c_str());
					} else {
						codeInt(arg);
					}
				}
			} else {
				// unimplemented instruction
				if (opcode < 0x40) { // 1 byte instruction
					debugC(5, kDebugCompile, "Unimplemented opcode: 0x%02x", opcode);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					code1(LC::cb_unk);
					codeInt(opcode);
				} else if (opcode < 0x80) { // 2 byte instruction
					debugC(5, kDebugCompile, "Unimplemented opcode: 0x%02x (%d)", opcode, (uint)codeStore[pointer]);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					code1(LC::cb_unk1);
					codeInt(opcode);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					codeInt((uint)codeStore[pointer]);
					pointer += 1;
				} else { // 3 byte instruction
					debugC(5, kDebugCompile, "Unimplemented opcode: 0x%02x (%d, %d)", opcode, (uint)codeStore[pointer], (uint)codeStore[pointer+1]);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					code1(LC::cb_unk2);
					codeInt(opcode);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					codeInt((uint)codeStore[pointer]);
					offsetList.push_back(_currentAssembly->size());
					byteOffsets.push_back(_currentAssembly->size());
					codeInt((uint)codeStore[pointer+1]);
					pointer += 2;
				}
			}
		}

		// Add backstop
		code1(STOP);

		// Rewrite every offset flagged as a jump based on the new code alignment.
		for (uint j = 0; j < jumpList.size(); j++) {
			int originalJumpAddressLoc = jumpList[j];
			int originalJumpInstructionLoc = originalJumpAddressLoc-1;
			int jumpAddressPc = offsetList[originalJumpAddressLoc];
			int jump = (int)READ_UINT32(&((*_currentAssembly)[jumpAddressPc]));
			int oldTarget = originalJumpInstructionLoc + jump;
			if ((oldTarget >= 0) && (oldTarget < (int)offsetList.size())) {
				int newJump = offsetList[oldTarget];
				WRITE_UINT32(&((*_currentAssembly)[jumpAddressPc]), newJump - jumpAddressPc + 1);
			} else {
				warning("Jump of %d from position %d is outside the function!", jump, originalJumpAddressLoc);
			}
		}

		// Attach to handlers

		Common::String functionName;
		if (0 <= nameIndex && nameIndex < (int16)archive->names.size()) {
			functionName = archive->names[nameIndex];
		} else if (i == 0 && (scriptFlags & kScriptFlagEventScript)) {
			// event script (lingo not contained within a handler)
			functionName = g_lingo->_eventHandlerTypes[kEventGeneric];
		}

		Symbol sym;
		if (!functionName.empty()) {
			debugC(5, kDebugLoading, "Function %d binding: %s()", i, functionName.c_str());
			sym = _assemblyContext->define(functionName, _currentAssembly, argNames, varNames);
		} else {
			warning("Function has unknown name id %d, skipping define", nameIndex);
			sym.name = new Common::String();
			sym.type = HANDLER;
			sym.u.defn = _currentAssembly;
			sym.nargs = argCount;
			sym.maxArgs = argCount;
			sym.argNames = argNames;
			sym.varNames = varNames;
		}

		_assemblyContext->_functionByteOffsets[functionName] = byteOffsets;

		if (!skipdump && ConfMan.getBool("dump_scripts")) {
			out.writeString(g_lingo->formatFunctionBody(sym));
		}

		_assemblyContext->_functionNames.push_back(*sym.name);
		_currentAssembly = nullptr;
	}

	if (!_assemblyContext->isFactory()) {
		// Register this context's functions with the containing archive.
		for (auto &it : _assemblyContext->_functionHandlers) {
			if (!_assemblyArchive->functionHandlers.contains(it._key)) {
				_assemblyArchive->functionHandlers[it._key] = it._value;
			}
		}
	}

	if (!skipdump && ConfMan.getBool("dump_scripts")) {
		out.flush();
		out.close();
	}

	free(codeStore);
	_assemblyArchive = nullptr;
	_assemblyContext = nullptr;
	_assemblyId = -1;

	return sc;
}

void LingoArchive::addCodeV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, const Common::String &archName, uint16 version) {
	ScriptContext *ctx = g_lingo->_compiler->compileLingoV4(stream, lctxIndex, this, archName, version);
	if (ctx) {
		lctxContexts[lctxIndex] = ctx;
		ctx->incRefCount();
	}
}

void LingoArchive::addNamesV4(Common::SeekableReadStreamEndian &stream) {
	debugC(1, kDebugCompile, "Add V4 script name index");

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

	uint32 size = stream.readUint32(); // size of Lnam
	stream.readUint32(); // size of Lnam again
	uint16 offset = stream.readUint16();
	uint16 count = stream.readUint16();

	if ((uint32)stream.size() != size) {
		warning("Lnam content missing");
		return;
	}

	stream.seek(offset);

	names.clear();

	for (uint16 i = 0; i < count; i++) {
		Common::String name = stream.readPascalString();

		names.push_back(name);
		debugC(5, kDebugLoading, "%d: \"%s\"", i, name.c_str());
	}

}

} // end of namespace Director
