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

class CodeWriter;
struct Datum;
struct Handler;
struct ScriptContext;

/* LiteralStore */

struct LiteralStore {
	LiteralType type;
	uint32_t offset;
	Common::SharedPtr<Datum> value;

	void readRecord(Common::SeekableReadStream &stream, int version);
	void readData(Common::SeekableReadStream &stream, uint32_t startOffset);
};

/* Script */

struct Script {
	/*  8 */ uint32_t totalLength;
	/* 12 */ uint32_t totalLength2;
	/* 16 */ uint16_t headerLength;
	/* 18 */ uint16_t scriptNumber;
	/* 20 */ int16_t unk20;
	/* 22 */ int16_t parentNumber;

	/* 38 */ uint32_t scriptFlags;
	/* 42 */ int16_t unk42;
	/* 44 */ int32_t castID;
	/* 48 */ int16_t factoryNameID;
	/* 50 */ uint16_t handlerVectorsCount;
	/* 52 */ uint32_t handlerVectorsOffset;
	/* 56 */ uint32_t handlerVectorsSize;
	/* 60 */ uint16_t propertiesCount;
	/* 62 */ uint32_t propertiesOffset;
	/* 66 */ uint16_t globalsCount;
	/* 68 */ uint32_t globalsOffset;
	/* 72 */ uint16_t handlersCount;
	/* 74 */ uint32_t handlersOffset;
	/* 78 */ uint16_t literalsCount;
	/* 80 */ uint32_t literalsOffset;
	/* 84 */ uint32_t literalsDataCount;
	/* 88 */ uint32_t literalsDataOffset;

	Common::Array<int16_t> propertyNameIDs;
	Common::Array<int16_t> globalNameIDs;

	Common::String factoryName;
	Common::Array<Common::String> propertyNames;
	Common::Array<Common::String> globalNames;
	Common::Array<Handler> handlers;
	Common::Array<LiteralStore> literals;
	Common::Array<Script *> factories;

	unsigned int version;
	ScriptContext *context;

	Script(unsigned int version);
	~Script();
	void read(Common::SeekableReadStream &stream);
	Common::Array<int16_t> readVarnamesTable(Common::SeekableReadStream &stream, uint16_t count, uint32_t offset);
	bool validName(int id) const;
	Common::String getName(int id) const;
	void setContext(ScriptContext *ctx);
	void parse();
	void writeVarDeclarations(CodeWriter &code) const;
	void writeScriptText(CodeWriter &code, bool dotSyntax) const;
	Common::String scriptText(const char *lineEnding, bool dotSyntax) const;
	void writeBytecodeText(CodeWriter &code, bool dotSyntax) const;
	Common::String bytecodeText(const char *lineEnding, bool dotSyntax) const;

	bool isFactory() const;
};

} // namespace LingoDec

#endif // LINGODEC_SCRIPT_H
