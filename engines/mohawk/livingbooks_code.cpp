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

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"

namespace Mohawk {

LBCode::LBCode(MohawkEngine_LivingBooks *vm) : _vm(vm) {
	Common::SeekableSubReadStreamEndian *bcodStream = _vm->wrapStreamEndian(ID_BCOD, 1000);
	uint32 totalSize = bcodStream->readUint32();
	if (totalSize != (uint32)bcodStream->size())
		error("BCOD had size %d, but claimed to be of size %d", bcodStream->size(), totalSize);
	size = bcodStream->readUint32();
	if (size + 8 > totalSize)
		error("BCOD code was of size %d, beyond size %d", size, totalSize);
	data = new byte[size];
	bcodStream->read(data, size);
	uint16 pos = 0;
	while (bcodStream->pos() < bcodStream->size()) {
		uint16 unknown = bcodStream->readUint16();
		if (unknown != 0) {
			warning("unknown was %04x, not zero, while reading strings", unknown);
			if (bcodStream->pos() != bcodStream->size())
				error(".. and there was more data afterwards");
			break;
		}
		Common::String string = _vm->readString(bcodStream);
		strings[pos] = string;
		debug(2, "read '%s' from BCOD at 0x%04x", string.c_str(), pos);
		pos += 2 + string.size() + 1;
	}
}

LBCode::~LBCode() {
	delete[] data;
}

Common::Array<LBValue> LBCode::readParams(LBItem *src, uint32 &offset) {
	Common::Array<LBValue> params;

	if (offset + 1 >= size)
		error("went off the end of code");

	byte numParams = data[offset];
	offset++;

	if (!numParams) {
		debugN("()\n");
		return params;
	}

	byte nextToken = data[offset];
	offset++;
	if (nextToken != kLBCodeTokenOpenBracket)
		error("missing ( before code parameter list (got %02x)", nextToken);
	debugN("(");

	for (uint i = 0; i < numParams; i++) {
		if (i != 0) {
			nextToken = data[offset];
			offset++;
			if (nextToken != ',')
				error("missing , between code parameters (got %02x)", nextToken);
			debugN(", ");
		}

		nextToken = data[offset];
		offset++;

		LBValue nextValue;

		switch (nextToken) {
		case kLBCodeTokenLiteral:
			{
			byte literalType = data[offset];
			offset++;
			if (literalType == kLBCodeLiteralInteger) {
				uint16 intValue = READ_BE_UINT16(data + offset);
				offset += 2;
				nextValue.type = kLBValueInteger;
				nextValue.integer = intValue;
				debugN("%d", nextValue.integer);
			} else
				error("unknown literal type %02x in code", literalType);
			}
			break;

		case kLBCodeTokenString:
			{
			uint16 stringOffset = READ_BE_UINT16(data + offset);
			offset += 2;
			// TODO: check string exists
			nextValue.type = kLBValueString;
			nextValue.string = strings[stringOffset];
			debugN("\"%s\"", nextValue.string.c_str());
			}
			break;

		case kLBCodeTokenChar:
			{
			uint16 stringOffset = READ_BE_UINT16(data + offset);
			offset += 2;
			// TODO: check string exists
			nextValue.type = kLBValueString;
			nextValue.string = strings[stringOffset];
			debugN("'%s'", nextValue.string.c_str());
			}
			break;

		case kLBCodeTokenLong: // FIXME: wrong?
			{
			uint32 intValue = READ_BE_UINT32(data + offset);
			offset += 4;
			nextValue.type = kLBValueInteger;
			nextValue.integer = intValue;
			debugN("%d", nextValue.integer);
			}
			break;

		case 0x31:
			{
			// TODO
			uint16 intValue = READ_BE_UINT16(data + offset);
			offset += 2;
			nextValue.type = kLBValueInteger;
			nextValue.integer = intValue;
			debugN("%d", nextValue.integer);
			}
			break;

		case 0x4d:
			// TODO
			runCodeCommand(src, offset);
			break;

		case 0x5f:
			// keycode
			nextValue.type = kLBValueInteger;
			nextValue.integer = data[offset];
			debugN("%d", nextValue.integer);
			offset++;
			offset++; // TODO
			break;

		default:
			error("unknown token %02x in code parameter", nextToken);
		}

		params.push_back(nextValue);
	}

	nextToken = data[offset];
	offset++;
	if (nextToken != kLBCodeTokenCloseBracket)
		error("missing ) after code parameter list (got %02x)", nextToken);
	debugN(")");

	return params;
}

void LBCode::runCodeCommand(LBItem *src, uint32 &offset) {
	if (offset + 1 >= size)
		error("went off the end of code");

	byte commandType = data[offset];
	offset++;

	switch (commandType) {
	case 0x23:
		{
		debugN("setViewOrigin");
		Common::Array<LBValue> params = readParams(src, offset);
		// TODO
		}
		break;

	case 0x36:
		{
		debugN("setWorld");
		Common::Array<LBValue> params = readParams(src, offset);
		// TODO
		}
		break;

	case 0x42:
		{
		debugN("setPlayParams");
		Common::Array<LBValue> params = readParams(src, offset);
		if (params.size() > 8)
			error("too many parameters (%d) to setPlayParams", params.size());
		if (!params.size())
			error("no target for setPlayParams");
		LBItem *target;
		if (params[0].string.equalsIgnoreCase("self")) {
			target = src;
		} else {
			error("didn't understand target '%s'", params[0].string.c_str());
		}
		// TODO: type-checking
		switch (params.size()) {
		case 8:
			target->_soundMode = params[7].integer;
		case 7:
			target->_controlMode = params[6].integer;
		case 6:
			// TODO: _relocPoint?
		case 5:
			// TODO: _periodMin/Max
		case 4:
			target->_timingMode = params[3].integer;
		case 3:
			// TODO: _delayMin/Max
		case 2:
			target->_loopMode = params[1].integer;
		}
		}
		break;

	case 0x50:
		{
		debugN("setKeyEvent");
		Common::Array<LBValue> params = readParams(src, offset);
		if (params.size() != 2)
			error("incorrect number of parameters (%d) to setKeyEvent", params.size());
		// FIXME: params[0] is key, params[1] is opcode id
		}
		break;

	case 0x51:
		{
		debugN("setHitTest");
		Common::Array<LBValue> params = readParams(src, offset);
		if (params.size() > 2)
			error("incorrect number of parameters (%d) to setHitTest", params.size());
		// TODO
		}
		break;

	case 0x52:
		{
		debugN("key");
		Common::Array<LBValue> params = readParams(src, offset);
		// TODO
		}
		break;

	case 0x5E:
		{
		debugN("setPageFade");
		Common::Array<LBValue> params = readParams(src, offset);
		// TODO
		}
		break;

	default:
		error("unknown command %02x in code", commandType);
	}
}

void LBCode::runCodeItemCommand(LBItem *src, uint32 &offset) {
	if (offset + 1 >= size)
		error("went off the end of code");

	byte commandType = data[offset];
	offset++;

	switch (commandType) {
	case 0x1d:
		{
		debugN("setParent");
		Common::Array<LBValue> params = readParams(src, offset);
		if (params.size() > 2)
			error("incorrect number of parameters (%d) to setParent", params.size());
		// TODO
		}
		break;

	default:
		error("unknown item command %02x in code", commandType);
	}
}

void LBCode::runCodeNotifyCommand(LBItem *src, uint32 &offset) {
	if (offset + 1 >= size)
		error("went off the end of code");

	byte commandType = data[offset];
	offset++;

	switch (commandType) {
	case 0x3:
		{
		debugN("goto");
		Common::Array<LBValue> params = readParams(src, offset);
		// TODO: type-checking
		switch (params.size()) {
		case 1:
			_vm->addNotifyEvent(NotifyEvent(kLBNotifyChangePage, params[0].integer));
			break;

		case 2:
			// FIXME
		case 4:
			// FIXME

		default:
			error("incorrect number of parameters (%d) to goto", params.size());
		}
		}
		break;

	default:
		error("unknown notify command %02x in code", commandType);
	}
}

void LBCode::runCode(LBItem *src, uint32 offset) {
	while (true) {
		if (offset + 1 >= size) {
			warning("went off the end of code");
			return;
		}

		byte tokenType = data[offset];
		offset++;

		switch (tokenType) {
		case 0x01: // FIXME
		case kLBCodeTokenEndOfFile:
			return;

		case 0x4D:
			runCodeCommand(src, offset);
			break;

		case 0x4E:
			runCodeItemCommand(src, offset);
			break;

		case 0x4F:
			runCodeNotifyCommand(src, offset);
			break;

		default:
			debugN("at %04x: %02x ", offset - 1, tokenType);
			for (uint i = 0; i < size; i++)
				debugN("%02x ", data[offset++]);
			debugN("\n");
			error("unknown token %02x in code", tokenType);
		}

		byte nextToken = data[offset];
		offset++;
		if (nextToken != kLBCodeTokenEndOfStatement)
			warning("missing EndOfStatement after code statement (got %04x)", nextToken);
		if (nextToken == kLBCodeTokenEndOfFile)
			return;

		debugN("\n");
	}
}

} // End of namespace Mohawk
