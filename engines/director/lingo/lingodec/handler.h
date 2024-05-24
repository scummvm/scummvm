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
class CodeWriterVisitor;
struct Node;
struct Script;

/* Handler */

struct Handler {
	int16 nameID = 0;
	uint16 vectorPos = 0;
	uint32 compiledLen = 0;
	uint32 compiledOffset = 0;
	uint16 argumentCount = 0;
	uint32 argumentOffset = 0;
	uint16 localsCount = 0;
	uint32 localsOffset = 0;
	uint16 globalsCount = 0;
	uint32 globalsOffset = 0;
	uint32 unknown1 = 0;
	uint16 unknown2 = 0;
	uint16 lineCount = 0;
	uint32 lineOffset = 0;
	uint32 stackHeight = 0;

	Common::Array<int16> argumentNameIDs;
	Common::Array<int16> localNameIDs;
	Common::Array<int16> globalNameIDs;

	Script *script = nullptr;
	Common::Array<Bytecode> bytecodeArray;
	Common::StableMap<uint32, size_t> bytecodePosMap;
	Common::Array<Common::String> argumentNames;
	Common::Array<Common::String> localNames;
	Common::Array<Common::String> globalNames;
	Common::String name;

	Common::Array<Common::SharedPtr<Node>> stack;
	AST ast;

	bool isGenericEvent = false;

	Handler(): ast(0, this) {}

	void setScript(Script *s) {
		script = s;
	}

	void readRecord(Common::SeekableReadStream &stream);
	void readData(Common::SeekableReadStream &stream);
	Common::Array<int16> readVarnamesTable(Common::SeekableReadStream &stream, uint16 count, uint32 offset);
	void readNames();
	bool validName(int id) const;
	Common::String getName(int id) const;
	Common::String getArgumentName(int id) const;
	Common::String getLocalName(int id) const;
	Common::SharedPtr<Node> pop();
	int variableMultiplier();
	Common::SharedPtr<Node> readVar(int varType);
	Common::String getVarNameFromSet(const Bytecode &bytecode);
	Common::SharedPtr<Node> readV4Property(uint32 offset, int propertyType, int propertyID);
	Common::SharedPtr<Node> readChunkRef(uint32 offset, Common::SharedPtr<Node> string);
	void tagLoops();
	bool isRepeatWithIn(uint32 startIndex, uint32 endIndex);
	BytecodeTag identifyLoop(uint32 startIndex, uint32 endIndex);
	void parse();
	uint32 translateBytecode(Bytecode &bytecode, uint32 index);
	void writeBytecodeText(CodeWriterVisitor &code) const;
};

/* Bytecode */

struct Bytecode {
	byte opID;
	OpCode opcode;
	int32 obj;
	uint32 pos;
	BytecodeTag tag;
	uint32 ownerLoop;
	Common::SharedPtr<Node> translation;

	Bytecode(byte op, int32 o, uint32 p)
		: opID(op), obj(o), pos(p), tag(kTagNone), ownerLoop(0xffffffff) {
		opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
	}
};

}

#endif // LINGODEC_HANDLER_H
