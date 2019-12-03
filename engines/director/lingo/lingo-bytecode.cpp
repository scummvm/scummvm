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

#include "director/lingo/lingo.h"

namespace Director {

static LingoV4Bytecode lingoV4[] = {
	{ 0x01, STOP, "" },
	{ 0x03, Lingo::c_voidpush, "" },
	{ 0x04, Lingo::c_mul, "" },
	{ 0x05, Lingo::c_add, "" },
	{ 0x06, Lingo::c_sub, "" },
	{ 0x07, Lingo::c_div, "" },
	{ 0x08, Lingo::c_mod, "" },
	{ 0x09, Lingo::c_negate, "" },
	{ 0x0a, Lingo::c_ampersand, "" },
	{ 0x0b, Lingo::c_concat, "" },
	{ 0x0c, Lingo::c_lt, "" },
	{ 0x0d, Lingo::c_le, "" },
	{ 0x0e, Lingo::c_neq, "" },
	{ 0x0f, Lingo::c_eq, "" },
	{ 0x10, Lingo::c_gt, "" },
	{ 0x11, Lingo::c_ge, "" },
	{ 0x12, Lingo::c_and, "" },
	{ 0x13, Lingo::c_or, "" },
	{ 0x14, Lingo::c_not, "" },
	{ 0x15, Lingo::c_contains, "" },
	{ 0x16, Lingo::c_starts, "" },
	{ 0x17, Lingo::c_of, "" },
	{ 0x18, Lingo::c_hilite, "" },
	{ 0x19, Lingo::c_intersects, "" },
	{ 0x1a, Lingo::c_within, "" },
	{ 0x1b, Lingo::c_field, "" },
	{ 0x1c, Lingo::c_tell, "" },
	{ 0x1d, Lingo::c_telldone, "" },
	{ 0x41, Lingo::c_intpush, "b" },
	{ 0x42, Lingo::c_argspush, "b" },
	{ 0x43, Lingo::c_arraypush, "b" },
	{ 0x44, Lingo::c_constpush, "bv" },
	{ 0x45, Lingo::c_symbolpush, "b" },
	{ 0x53, Lingo::c_jump, "jb" },
	{ 0x54, Lingo::c_jump, "jbn" },
	{ 0x55, Lingo::c_jumpifz, "jb" },
	{ 0x5c, Lingo::cb_v4theentitypush, "b" },
	{ 0x5d, Lingo::cb_v4theentityassign, "b" },
	{ 0x66, Lingo::cb_v4theentitynamepush, "b" },
	{ 0x81, Lingo::c_intpush, "w" },
	{ 0x82, Lingo::c_argspush, "w" },
	{ 0x83, Lingo::c_arraypush, "w" },
	{ 0x84, Lingo::c_constpush, "wv" },
	{ 0x93, Lingo::c_jump, "jw" },
	{ 0x94, Lingo::c_jump, "jwn" },
	{ 0x95, Lingo::c_jumpifz, "jw" },
	{ 0, 0, 0 }
};

static LingoV4TheEntity lingoV4TheEntity[] = {
	{ 0x00, 0x00, kTheFloatPrecision, kTheNOField, false, kTEANOArgs },
	{ 0x00, 0x01, kTheMouseDownScript, kTheNOField, true, kTEANOArgs },
	{ 0x00, 0x02, kTheMouseUpScript, kTheNOField, true, kTEANOArgs },
	{ 0x00, 0x03, kTheKeyDownScript, kTheNOField, true, kTEANOArgs },
	{ 0x00, 0x04, kTheKeyUpScript, kTheNOField, true, kTEANOArgs },
	{ 0x00, 0x05, kTheTimeoutScript, kTheNOField, true, kTEANOArgs },
	{ 0x00, 0x06, kTheTime, kTheShort, false, kTEANOArgs },
	{ 0x00, 0x07, kTheTime, kTheAbbr, false, kTEANOArgs },
	{ 0x00, 0x08, kTheTime, kTheLong, false, kTEANOArgs },
	{ 0x00, 0x09, kTheDate, kTheShort, false, kTEANOArgs },
	{ 0x00, 0x0a, kTheDate, kTheAbbr, false, kTEANOArgs },
	{ 0x00, 0x0b, kTheDate, kTheLong, false, kTEANOArgs },
	{ 0x00, 0x0c, kTheChars, kTheLast, false, kTEAString },
	{ 0x00, 0x0d, kTheWords, kTheLast, false, kTEAString },
	{ 0x00, 0x0e, kTheItems, kTheLast, false, kTEAString },
	{ 0x00, 0x0f, kTheLines, kTheLast, false, kTEAString },
	{ 0x01, 0x01, kTheChars, kTheNumber, false, kTEAString },
	{ 0x01, 0x02, kTheWords, kTheNumber, false, kTEAString },
	{ 0x01, 0x03, kTheItems, kTheNumber, false, kTEAString },
	{ 0x01, 0x04, kTheLines, kTheNumber, false, kTEAString },
	{ 0x02, 0x01, kTheMenu, kTheName, false, kTEAItemId },
	{ 0x02, 0x02, kTheMenuItems, kTheNumber, false, kTEAItemId },
	{ 0x03, 0x01, kTheMenuItem, kTheName, true, kTEAMenuIdItemId },
	{ 0x03, 0x02, kTheMenuItem, kTheCheckMark, true, kTEAMenuIdItemId },
	{ 0x03, 0x03, kTheMenuItem, kTheEnabled, true, kTEAMenuIdItemId },
	{ 0x03, 0x04, kTheMenuItem, kTheScript, true, kTEAMenuIdItemId },
	{ 0x04, 0x01, kTheSound, kTheVolume, true, kTEAItemId },
	{ 0x06, 0x01, kTheSprite, kTheCursor, true, kTEAItemId },
	{ 0x06, 0x02, kTheSprite, kTheBackColor, true, kTEAItemId },
	{ 0x06, 0x03, kTheSprite, kTheBottom, true, kTEAItemId },
	{ 0x06, 0x04, kTheSprite, kTheCastNum, true, kTEAItemId },
	{ 0x06, 0x05, kTheSprite, kTheConstraint, true, kTEAItemId },
	{ 0x06, 0x06, kTheSprite, kTheCursor, true, kTEAItemId },
	{ 0x06, 0x07, kTheSprite, kTheForeColor, true, kTEAItemId },
	{ 0x06, 0x08, kTheSprite, kTheHeight, true, kTEAItemId },
	{ 0x06, 0x0a, kTheSprite, kTheInk, true, kTEAItemId },
	{ 0x06, 0x0b, kTheSprite, kTheLeft, true, kTEAItemId },
	{ 0x06, 0x0c, kTheSprite, kTheLineSize, true, kTEAItemId },
	{ 0x06, 0x0d, kTheSprite, kTheLocH, true, kTEAItemId },
	{ 0x06, 0x0e, kTheSprite, kTheLocV, true, kTEAItemId },
	{ 0x06, 0x0f, kTheSprite, kTheMovieRate, true, kTEAItemId },
	{ 0x06, 0x10, kTheSprite, kTheMovieTime, true, kTEAItemId },
	{ 0x06, 0x12, kTheSprite, kThePuppet, true, kTEAItemId },
	{ 0x06, 0x13, kTheSprite, kTheRight, true, kTEAItemId },
	{ 0x06, 0x14, kTheSprite, kTheStartTime, true, kTEAItemId },
	{ 0x06, 0x15, kTheSprite, kTheStopTime, true, kTEAItemId },
	{ 0x06, 0x16, kTheSprite, kTheStretch, true, kTEAItemId },
	{ 0x06, 0x17, kTheSprite, kTheTop, true, kTEAItemId },
	{ 0x06, 0x18, kTheSprite, kTheTrails, true, kTEAItemId },
	{ 0x06, 0x19, kTheSprite, kTheVisible, true, kTEAItemId },
	{ 0x06, 0x1a, kTheSprite, kTheVolume, true, kTEAItemId },
	{ 0x06, 0x1b, kTheSprite, kTheWidth, true, kTEAItemId },
	{ 0x06, 0x1d, kTheSprite, kTheScriptNum, true, kTEAItemId },
	{ 0x06, 0x1e, kTheSprite, kTheMoveableSprite, true, kTEAItemId },
	{ 0x06, 0x20, kTheSprite, kTheScoreColor, true, kTEAItemId },
	{ 0x07, 0x01, kTheBeepOn, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x02, kTheButtonStyle, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x03, kTheCenterStage, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x04, kTheCheckBoxAccess, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x05, kTheCheckBoxType, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x06, kTheColorDepth, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x08, kTheExitLock, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x09, kTheFixStageSize, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x13, kTheTimeoutLapsed, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x17, kTheSelEnd, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x18, kTheSelStart, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x19, kTheSoundEnabled, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x1a, kTheSoundLevel, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x1b, kTheStageColor, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x1d, kTheStillDown, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x1e, kTheTimeoutKeyDown, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x1f, kTheTimeoutLength, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x20, kTheTimeoutMouse, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x21, kTheTimeoutPlay, kTheNOField, true, kTEANOArgs },
	{ 0x07, 0x22, kTheTimer, kTheNOField, true, kTEANOArgs },
	{ 0x08, 0x01, kThePerFrameHook, kTheNOField, false, kTEANOArgs },
	{ 0x08, 0x02, kTheCastMembers, kTheNumber, false, kTEANOArgs },
	{ 0x08, 0x03, kTheMenus, kTheNumber, false, kTEANOArgs },
	{ 0x09, 0x01, kTheCast, kTheName, true, kTEAItemId },
	{ 0x09, 0x02, kTheCast, kTheText, true, kTEAItemId },
	{ 0x09, 0x08, kTheCast, kThePicture, true, kTEAItemId },
	{ 0x09, 0x0a, kTheCast, kTheNumber, true, kTEAItemId },
	{ 0x09, 0x0b, kTheCast, kTheSize, true, kTEAItemId },
	{ 0x09, 0x11, kTheCast, kTheForeColor, true, kTEAItemId },
	{ 0x09, 0x12, kTheCast, kTheBackColor, true, kTEAItemId },
	{ 0x0c, 0x03, kTheField, kTheTextStyle, true, kTEAItemId },
	{ 0x0c, 0x04, kTheField, kTheTextFont, true, kTEAItemId },
	{ 0x0c, 0x05, kTheField, kTheTextHeight, true, kTEAItemId },
	{ 0x0c, 0x06, kTheField, kTheTextAlign, true, kTEAItemId },
	{ 0x0c, 0x07, kTheField, kTheTextSize, true, kTEAItemId },
	{ 0x0d, 0x0f, kTheCast, kTheDirectToStage, true, kTEAItemId },
	{ 0x0d, 0x10, kTheCast, kTheSound, true, kTEAItemId },
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


void Lingo::cb_v4theentitypush() {
	int bank = g_lingo->readInt();

	Datum firstArg = g_lingo->pop();
	Datum result;
	result.u.s = NULL;
	result.type = VOID;

	if (firstArg.type == INT) {
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

	} else {
		warning("cb_v4theentitypush: first arg should be of type INT, not %s", firstArg.type2str());
	}

	g_lingo->push(result);
}


void Lingo::cb_v4theentitynamepush() {
	int nameId = g_lingo->readInt();
	Common::String name = g_lingo->_namelist[nameId];

	Datum id;
	id.u.s = NULL;
	id.type = VOID;

	TheEntity *entity = g_lingo->_theEntities[name];

	debugC(3, kDebugLingoExec, "cb_v4theentitynamepush: calling getTheEntity(0x%02x, id, kTheNOField)", entity->entity, name.c_str());
	Datum result = g_lingo->getTheEntity(entity->entity, id, kTheNOField);

	g_lingo->push(result);
}


void Lingo::cb_v4theentityassign() {
	int bank = g_lingo->readInt();

	Datum firstArg = g_lingo->pop();
	Datum value = g_lingo->pop();
	Datum result;
	result.u.s = NULL;
	result.type = VOID;

	if (firstArg.type == INT) {
		int key = (bank << 8) + firstArg.u.i;
		if (g_lingo->_lingoV4TheEntity.contains(key)) {
			debugC(3, kDebugLingoExec, "cb_v4theentityassign: mapping 0x%02x, 0x%02x", bank, firstArg.u.i);
			if (g_lingo->_lingoV4TheEntity[key]->writable) {
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
			} else {
				warning("cb_v4theentityassign: non-writable mapping 0x%02x 0x%02x", bank, firstArg.u.i);
			}
		} else {
			warning("cb_v4theentityassign: unhandled mapping 0x%02x 0x%02x", bank, firstArg.u.i);
		}

	} else {
		warning("cb_v4theentityassign: first arg should be of type INT, not %s", firstArg.type2str());
	}
}


void Lingo::addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id) {
	debugC(1, kDebugLingoCompile, "Add V4 bytecode for type %s with id %d", scriptType2str(type), id);

	if (_scriptContexts[type].contains(id)) {
		for (size_t j = 0; j < _scriptContexts[type][id]->functions.size(); j++) {
			delete _scriptContexts[type][id]->functions[j];
		}
		delete _scriptContexts[type][id];
	}

	_currentScriptContext = new ScriptContext;
	_currentScriptType = type;
	_currentEntityId = id;
	_scriptContexts[type][id] = _currentScriptContext;

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
	/*uint16 globalsOffset = */stream.readUint16();
	/*uint16 globalsCount = */stream.readUint16();
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

	// preload all the constants!
	// these are stored as a reference table of 6 byte entries, followed by a storage area.

	// copy the storage area first.
	uint32 constsStoreOffset = constsOffset + 6 * constsCount;
	uint32 constsStoreSize = stream.size() - constsStoreOffset;
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
				while (pointer < constsStoreSize) {
					if (constsStore[pointer] == '\r') {
						constant.u.s += '\n';
					} else if (constsStore[pointer] == '\0') {
						break;
					} else {
						constant.u.s += constsStore[pointer];
					}
					pointer += 1;
				}
				if (pointer >= constsStoreSize) {
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
				if (value < constsStoreOffset) {
					warning("Constant float start offset is out of bounds");
					break;
				} else if (value + 4 > constsStoreSize) {
					warning("Constant float end offset is out of bounds");
					break;
				}
				constant.u.f = *(float *)(constsStore + value);
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

		_currentScriptFunction = i;
		_currentScriptContext->functions.push_back(new ScriptData);
		_currentScript = _currentScriptContext->functions[_currentScriptFunction];

		/*uint16 nameIndex = */stream.readUint16();
		stream.readUint16();
		uint32 length = stream.readUint32();
		uint32 startOffset = stream.readUint32();
		/*uint16 argCount = */stream.readUint16();
		/*uint32 argOffset = */stream.readUint32();
		/*uint16 varCount = */stream.readUint16();
		/*uint32 varNamesOffset = */stream.readUint32();
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
			if (_lingoV4.contains(opcode)) {
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
						case 'v':
							// argument refers to a variable; remove struct size alignment
							if (arg % 6) {
								warning("Opcode 0x%02x arg %d not a multiple of 6!", opcode, arg);
							}
							arg /= 6;
							break;
						case 'n':
							// argument is negative
							arg *= -1;
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
					g_lingo->code1(Lingo::c_unk);
					g_lingo->codeInt(opcode);
				} else if (opcode < 0x80) { // 2 byte instruction
					debugC(5, kDebugLingoCompile, "Unimplemented opcode: 0x%02x (%d)", opcode, (uint)codeStore[pointer]);
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk1);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					pointer += 1;
				} else { // 3 byte instruction
					debugC(5, kDebugLingoCompile, "Unimplemented opcode: 0x%02x (%d, %d)", opcode, (uint)codeStore[pointer], (uint)codeStore[pointer+1]);
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk2);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer+1]);
					pointer += 2;
				}
			}
		}

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
	}
	free(codeStore);
}


void Lingo::addNamesV4(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLingoCompile, "Add V4 script name index");

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

	stream.seek(offset);

	_namelist.clear();

	Common::Array<Common::String> names;
	for (uint32 i = 0; i < count; i++) {
		uint8 size = stream.readByte();
		Common::String name;
		for (uint8 j = 0; j < size; j++) {
			name += stream.readByte();
		}
		_namelist.push_back(name);
	}

}

}
