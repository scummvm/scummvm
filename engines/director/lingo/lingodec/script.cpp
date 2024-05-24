/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/stream.h"
#include "./ast.h"
#include "./codewritervisitor.h"
#include "./context.h"
#include "./handler.h"
#include "./script.h"

double readAppleFloat80(void *ptr);

namespace LingoDec {

/* Script */

Script::Script(unsigned int version_) :
	version(version_),
	context(nullptr) {}

Script::~Script() = default;

void Script::read(Common::SeekableReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.seek(8);
	/*  8 */ totalLength = stream.readUint32BE();
	/* 12 */ totalLength2 = stream.readUint32BE();
	/* 16 */ headerLength = stream.readUint16BE();
	/* 18 */ scriptNumber = stream.readUint16BE();
	/* 20 */ unk20 = stream.readSint16BE();
	/* 22 */ parentNumber = stream.readSint16BE();

	stream.seek(38);
	/* 38 */ scriptFlags = stream.readUint32BE();
	/* 42 */ unk42 = stream.readSint16BE();
	/* 44 */ castID = stream.readSint32BE();
	/* 48 */ factoryNameID = stream.readSint16BE();
	/* 50 */ handlerVectorsCount = stream.readUint16BE();
	/* 52 */ handlerVectorsOffset = stream.readUint32BE();
	/* 56 */ handlerVectorsSize = stream.readUint32BE();
	/* 60 */ propertiesCount = stream.readUint16BE();
	/* 62 */ propertiesOffset = stream.readUint32BE();
	/* 66 */ globalsCount = stream.readUint16BE();
	/* 68 */ globalsOffset = stream.readUint32BE();
	/* 72 */ handlersCount = stream.readUint16BE();
	/* 74 */ handlersOffset = stream.readUint32BE();
	/* 78 */ literalsCount = stream.readUint16BE();
	/* 80 */ literalsOffset = stream.readUint32BE();
	/* 84 */ literalsDataCount = stream.readUint32BE();
	/* 88 */ literalsDataOffset = stream.readUint32BE();

	propertyNameIDs = readVarnamesTable(stream, propertiesCount, propertiesOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);

	handlers.resize(handlersCount);
	for (auto &handler : handlers) {
		handler.setScript(this);
	}
	if ((scriptFlags & LingoDec::kScriptFlagEventScript) && handlersCount > 0) {
		handlers[0].isGenericEvent = true;
	}

	stream.seek(handlersOffset);
	for (auto &handler : handlers) {
		handler.readRecord(stream);
	}
	for (auto &handler : handlers) {
		handler.readData(stream);
	}

	stream.seek(literalsOffset);
	literals.resize(literalsCount);
	for (auto &literal : literals) {
		literal.readRecord(stream, version);
	}
	for (auto &literal : literals) {
		literal.readData(stream, literalsDataOffset);
	}
}

Common::Array<int16> Script::readVarnamesTable(Common::SeekableReadStream &stream, uint16 count, uint32 offset) {
	stream.seek(offset);
	Common::Array<int16> nameIDs(count);
	for (uint16 i = 0; i < count; i++) {
		nameIDs[i] = stream.readSint16BE();
	}
	return nameIDs;
}

bool Script::validName(int id) const {
	return context->validName(id);
}

Common::String Script::getName(int id) const {
	return context->getName(id);
}

void Script::setContext(ScriptContext *ctx) {
	this->context = ctx;
	if (factoryNameID != -1) {
		factoryName = getName(factoryNameID);
	}
	for (auto nameID : propertyNameIDs) {
		if (validName(nameID)) {
			Common::String name = getName(nameID);
			if (isFactory() && name == "me")
				continue;
			propertyNames.push_back(name);
		}
	}
	for (auto nameID : globalNameIDs) {
		if (validName(nameID)) {
			globalNames.push_back(getName(nameID));
		}
	}
	for (auto &handler : handlers) {
		handler.readNames();
	}
}

void Script::parse() {
	for (auto &handler : handlers) {
		handler.parse();
	}
}

void Script::writeVarDeclarations(CodeWriterVisitor &code) const {
	if (!isFactory()) {
		if (propertyNames.size() > 0) {
			code.write("property ");
			for (size_t i = 0; i < propertyNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(propertyNames[i]);
			}
			code.writeLine();
		}
	}
	if (globalNames.size() > 0) {
		code.write("global ");
		for (size_t i = 0; i < globalNames.size(); i++) {
			if (i > 0)
				code.write(", ");
			code.write(globalNames[i]);
		}
		code.writeLine();
	}
}

void Script::writeScriptText(CodeWriterVisitor &code) const {
	size_t origSize = code._str.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code._str.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code._str.size() != origSize) {
			code.writeLine();
		}
		handlers[i].ast.root->accept(code);
	}
	for (auto factory : factories) {
		if (code._str.size() != origSize) {
			code.writeLine();
		}
		factory->writeScriptText(code);
	}
}

Common::String Script::scriptText(const char *lineEnding, bool dotSyntax) const {
	CodeWriterVisitor code(dotSyntax, false, lineEnding);
	writeScriptText(code);
	return code._str;
}

void Script::writeBytecodeText(CodeWriterVisitor &code) const {
	size_t origSize = code._str.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code._str.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code._str.size() != origSize) {
			code.writeLine();
		}
		handlers[i].writeBytecodeText(code);
	}
	for (auto factory : factories) {
		if (code._str.size() != origSize) {
			code.writeLine();
		}
		factory->writeBytecodeText(code);
	}
}

Common::String Script::bytecodeText(const char *lineEnding, bool dotSyntax) const {
	CodeWriterVisitor code(dotSyntax, true, lineEnding);
	writeBytecodeText(code);
	return code._str;
}

bool Script::isFactory() const {
	return (scriptFlags & LingoDec::kScriptFlagFactoryDef);
}

/* LiteralStore */

void LiteralStore::readRecord(Common::SeekableReadStream &stream, int version) {
	if (version >= 500)
		type = static_cast<LiteralType>(stream.readUint32BE());
	else
		type = static_cast<LiteralType>(stream.readUint16BE());
	offset = stream.readUint32BE();
}

void LiteralStore::readData(Common::SeekableReadStream &stream, uint32 startOffset) {
	if (type == kLiteralInt) {
		value = Common::SharedPtr<LingoDec::Datum>(new LingoDec::Datum((int)offset));
	} else {
		stream.seek(startOffset + offset);
		auto length = stream.readUint32BE();
		if (type == kLiteralString) {
			char *buf = new char[length];
			stream.read(buf, length - 1);
			buf[length - 1] = '\0';
			value = Common::SharedPtr<LingoDec::Datum>(new LingoDec::Datum(LingoDec::kDatumString, Common::String(buf)));
			delete[] buf;
		} else if (type == kLiteralFloat) {
			double floatVal = 0.0;
			if (length == 8) {
				floatVal = stream.readDoubleBE();
			} else if (length == 10) {
				byte buf[10];
				stream.read(buf, 10);
				floatVal = readAppleFloat80(buf);
			}
			value = Common::SharedPtr<LingoDec::Datum>(new LingoDec::Datum(floatVal));
		} else {
			value = Common::SharedPtr<LingoDec::Datum>(new LingoDec::Datum());
		}
	}
}

} // namespace LingoDec
