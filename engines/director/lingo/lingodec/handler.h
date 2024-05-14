/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_HANDLER_H
#define LINGODEC_HANDLER_H

#include "common/array.h"
#include "common/stablemap.h"
#include "common/str.h"
#include "./enums.h"

namespace Common {
class SeekableReadStream;
}

namespace LingoDec {

struct AST;
struct Bytecode;
class CodeWriter;
struct Node;
struct Script;

/* Handler */

struct Handler {
	int16_t nameID = 0;
	uint16_t vectorPos = 0;
	uint32_t compiledLen = 0;
	uint32_t compiledOffset = 0;
	uint16_t argumentCount = 0;
	uint32_t argumentOffset = 0;
	uint16_t localsCount = 0;
	uint32_t localsOffset = 0;
	uint16_t globalsCount = 0;
	uint32_t globalsOffset = 0;
	uint32_t unknown1 = 0;
	uint16_t unknown2 = 0;
	uint16_t lineCount = 0;
	uint32_t lineOffset = 0;
	uint32_t stackHeight = 0;

	Common::Array<int16_t> argumentNameIDs;
	Common::Array<int16_t> localNameIDs;
	Common::Array<int16_t> globalNameIDs;

	Script *script = nullptr;
	Common::Array<Bytecode> bytecodeArray;
	Common::StableMap<uint32_t, size_t> bytecodePosMap;
	Common::Array<Common::String> argumentNames;
	Common::Array<Common::String> localNames;
	Common::Array<Common::String> globalNames;
	Common::String name;

	Common::Array<Common::SharedPtr<Node>> stack;
	AST ast;

	bool isGenericEvent = false;

	Handler(): ast(this) {}

	void setScript(Script *s) {
		script = s;
	}

	void readRecord(Common::SeekableReadStream &stream);
	void readData(Common::SeekableReadStream &stream);
	Common::Array<int16_t> readVarnamesTable(Common::SeekableReadStream &stream, uint16_t count, uint32_t offset);
	void readNames();
	bool validName(int id) const;
	Common::String getName(int id) const;
	Common::String getArgumentName(int id) const;
	Common::String getLocalName(int id) const;
	Common::SharedPtr<Node> pop();
	int variableMultiplier();
	Common::SharedPtr<Node> readVar(int varType);
	Common::String getVarNameFromSet(const Bytecode &bytecode);
	Common::SharedPtr<Node> readV4Property(int propertyType, int propertyID);
	Common::SharedPtr<Node> readChunkRef(Common::SharedPtr<Node> string);
	void tagLoops();
	bool isRepeatWithIn(uint32_t startIndex, uint32_t endIndex);
	BytecodeTag identifyLoop(uint32_t startIndex, uint32_t endIndex);
	void parse();
	uint32_t translateBytecode(Bytecode &bytecode, uint32_t index);
	void writeBytecodeText(CodeWriter &code, bool dotSyntax) const;
};

/* Bytecode */

struct Bytecode {
	uint8_t opID;
	OpCode opcode;
	int32_t obj;
	uint32_t pos;
	BytecodeTag tag;
	uint32_t ownerLoop;
	Common::SharedPtr<Node> translation;

	Bytecode(uint8_t op, int32_t o, uint32_t p)
		: opID(op), obj(o), pos(p), tag(kTagNone), ownerLoop(UINT32_MAX) {
		opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
	}
};

}

#endif // LINGODEC_HANDLER_H
