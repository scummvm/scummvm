/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_SCRIPT_H
#define LINGODEC_SCRIPT_H

#include "common/ptr.h"
#include "common/str.h"
#include "./enums.h"

namespace Common {
class ReadStream;
}

namespace LingoDec {

class CodeWriterVisitor;
struct Datum;
struct Handler;
struct ScriptContext;

/* LiteralStore */

struct LiteralStore {
	LiteralType type;
	uint32 offset;
	Common::SharedPtr<Datum> value;

	void readRecord(Common::SeekableReadStream &stream, int version);
	void readData(Common::SeekableReadStream &stream, uint32 startOffset);
};

/* Script */

struct Script {
	/*  8 */ uint32 totalLength;
	/* 12 */ uint32 totalLength2;
	/* 16 */ uint16 headerLength;
	/* 18 */ uint16 scriptNumber;
	/* 20 */ int16 unk20;
	/* 22 */ int16 parentNumber;

	/* 38 */ uint32 scriptFlags;
	/* 42 */ int16 unk42;
	/* 44 */ int32 castID;
	/* 48 */ int16 factoryNameID;
	/* 50 */ uint16 handlerVectorsCount;
	/* 52 */ uint32 handlerVectorsOffset;
	/* 56 */ uint32 handlerVectorsSize;
	/* 60 */ uint16 propertiesCount;
	/* 62 */ uint32 propertiesOffset;
	/* 66 */ uint16 globalsCount;
	/* 68 */ uint32 globalsOffset;
	/* 72 */ uint16 handlersCount;
	/* 74 */ uint32 handlersOffset;
	/* 78 */ uint16 literalsCount;
	/* 80 */ uint32 literalsOffset;
	/* 84 */ uint32 literalsDataCount;
	/* 88 */ uint32 literalsDataOffset;

	Common::Array<int16> propertyNameIDs;
	Common::Array<int16> globalNameIDs;

	Common::String factoryName;
	Common::Array<Common::String> propertyNames;
	Common::Array<Common::String> globalNames;
	Common::Array<Handler> handlers;
	Common::Array<LiteralStore> literals;
	Common::Array<Script *> factories;

	unsigned int version;
	ScriptContext *context;

	Script(unsigned int version_);
	~Script();
	void read(Common::SeekableReadStream &stream);
	Common::Array<int16> readVarnamesTable(Common::SeekableReadStream &stream, uint16 count, uint32 offset);
	bool validName(int id) const;
	Common::String getName(int id) const;
	void setContext(ScriptContext *ctx);
	void parse();
	void writeVarDeclarations(CodeWriterVisitor &code) const;
	void writeScriptText(CodeWriterVisitor &code) const;
	Common::String scriptText(const char *lineEnding, bool dotSyntax) const;
	void writeBytecodeText(CodeWriterVisitor &code) const;
	Common::String bytecodeText(const char *lineEnding, bool dotSyntax) const;

	bool isFactory() const;
};

} // namespace LingoDec

#endif // LINGODEC_SCRIPT_H
