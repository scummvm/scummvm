/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/stream.h"
#include "common/util.h"
#include "./ast.h"
#include "./codewritervisitor.h"
#include "./handler.h"
#include "./names.h"
#include "./script.h"

namespace LingoDec {

/* Handler */

void Handler::readRecord(Common::SeekableReadStream &stream) {
	nameID = stream.readSint16BE();
	vectorPos = stream.readUint16BE();
	compiledLen = stream.readUint32BE();
	compiledOffset = stream.readUint32BE();
	argumentCount = stream.readUint16BE();
	argumentOffset = stream.readUint32BE();
	localsCount = stream.readUint16BE();
	localsOffset = stream.readUint32BE();
	globalsCount = stream.readUint16BE();
	globalsOffset = stream.readUint32BE();
	unknown1 = stream.readUint32BE();
	unknown2 = stream.readUint16BE();
	lineCount = stream.readUint16BE();
	lineOffset = stream.readUint32BE();
	// yet to implement
	if (script->version >= 850)
		stackHeight = stream.readUint32BE();
}

void Handler::readData(Common::SeekableReadStream &stream) {
	stream.seek(compiledOffset);
	while (stream.pos() < compiledOffset + compiledLen) {
		uint32 pos = stream.pos() - compiledOffset;
		byte op = stream.readByte();
		OpCode opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
		// argument can be one, two or four bytes
		int32 obj = 0;
		if (op >= 0xc0) {
			// four bytes
			obj = stream.readSint32BE();
		} else if (op >= 0x80) {
			// two bytes
			if (opcode == kOpPushInt16 || opcode == kOpPushInt8) {
				// treat pushint's arg as signed
				// pushint8 may be used to push a 16-bit int in older Lingo
				obj = stream.readSint16BE();
			} else {
				obj = stream.readUint16BE();
			}
		} else if (op >= 0x40) {
			// one byte
			if (opcode == kOpPushInt8) {
				// treat pushint's arg as signed
				obj = stream.readSByte();
			} else {
				obj = stream.readByte();
			}
		}
		Bytecode bytecode(op, obj, pos);
		bytecodeArray.push_back(bytecode);
		bytecodePosMap[pos] = bytecodeArray.size() - 1;
	}

	argumentNameIDs = readVarnamesTable(stream, argumentCount, argumentOffset);
	localNameIDs = readVarnamesTable(stream, localsCount, localsOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);
}

Common::Array<int16> Handler::readVarnamesTable(Common::SeekableReadStream &stream, uint16 count, uint32 offset) {
	stream.seek(offset);
	Common::Array<int16> nameIDs;
	nameIDs.resize(count);
	for (size_t i = 0; i < count; i++) {
		nameIDs[i] = stream.readUint16BE();
	}
	return nameIDs;
}

void Handler::readNames() {
	if (!isGenericEvent) {
		name = getName(nameID);
	}
	for (size_t i = 0; i < argumentNameIDs.size(); i++) {
		if (i == 0 && script->isFactory())
			continue;
		argumentNames.push_back(getName(argumentNameIDs[i]));
	}
	for (auto nameID_ : localNameIDs) {
		if (validName(nameID_)) {
			localNames.push_back(getName(nameID_));
		}
	}
	for (auto nameID_ : globalNameIDs) {
		if (validName(nameID_)) {
			globalNames.push_back(getName(nameID_));
		}
	}
}

bool Handler::validName(int id) const {
	return script->validName(id);
}

Common::String Handler::getName(int id) const {
	return script->getName(id);
}

Common::String Handler::getArgumentName(int id) const {
	if (-1 < id && (unsigned)id < argumentNameIDs.size())
		return getName(argumentNameIDs[id]);
	return Common::String::format("UNKNOWN_ARG_%d", id);
}

Common::String Handler::getLocalName(int id) const {
	if (-1 < id && (unsigned)id < localNameIDs.size())
		return getName(localNameIDs[id]);
	return Common::String::format("UNKNOWN_LOCAL_%d", id);
}

Common::SharedPtr<Node> Handler::pop() {
	if (stack.empty())
		return Common::SharedPtr<Node>(new ErrorNode(0));

	auto res = stack.back();
	stack.pop_back();
	return res;
}

int Handler::variableMultiplier() {
	if (script->version >= 850)
		return 1;
	if (script->version >= 500)
		return 8;
	return 6;
}

Common::SharedPtr<Node> Handler::readVar(int varType) {
	Common::SharedPtr<Node> castID;
	if (varType == 0x6 && script->version >= 500) // field cast ID
		castID = pop();
	Common::SharedPtr<Node> id = pop();

	switch (varType) {
	case 0x1: // global
	case 0x2: // global
	case 0x3: // property/instance
		return id;
	case 0x4: // arg
		{
			Common::String name_ = getArgumentName(id->getValue()->i / variableMultiplier());
			auto ref = Common::SharedPtr<Datum>(new Datum(kDatumVarRef, name_));
			return Common::SharedPtr<Node>(new LiteralNode(id->_startOffset, Common::move(ref)));
		}
	case 0x5: // local
		{
			Common::String name_ = getLocalName(id->getValue()->i / variableMultiplier());
			auto ref = Common::SharedPtr<Datum>(new Datum(kDatumVarRef, name_));
			return Common::SharedPtr<Node>(new LiteralNode(id->_startOffset, Common::move(ref)));
		}
	case 0x6: // field
		return Common::SharedPtr<Node>(new MemberExprNode(id->_startOffset, "field", Common::move(id), Common::move(castID)));
	default:
		warning("findVar: unhandled var type %d", varType);
		break;
	}
	return Common::SharedPtr<Node>(new ErrorNode(id->_startOffset));
}

Common::String Handler::getVarNameFromSet(const Bytecode &bytecode) {
	Common::String varName;
	switch (bytecode.opcode) {
	case kOpSetGlobal:
	case kOpSetGlobal2:
		varName = getName(bytecode.obj);
		break;
	case kOpSetProp:
		varName = getName(bytecode.obj);
		break;
	case kOpSetParam:
		varName = getArgumentName(bytecode.obj / variableMultiplier());
		break;
	case kOpSetLocal:
		varName = getLocalName(bytecode.obj / variableMultiplier());
		break;
	default:
		varName = "ERROR";
		break;
	}
	return varName;
}

Common::SharedPtr<Node> Handler::readV4Property(uint32 offset, int propertyType, int propertyID) {
	switch (propertyType) {
	case 0x00:
		{
			if (propertyID <= 0x0b) { // movie property
				Common::String propName(StandardNames::moviePropertyNames[propertyID]);
				return Common::SharedPtr<Node>(new TheExprNode(offset, propName));
			} else { // last chunk
				auto string = pop();
				auto chunkType = static_cast<ChunkExprType>(propertyID - 0x0b);
				return Common::SharedPtr<Node>(new LastStringChunkExprNode(offset, chunkType, Common::move(string)));
			}
		}
		break;
	case 0x01: // number of chunks
		{
			auto string = pop();
			return Common::SharedPtr<Node>(new StringChunkCountExprNode(offset, static_cast<ChunkExprType>(propertyID), Common::move(string)));
		}
		break;
	case 0x02: // menu property
		{
			auto menuID = pop();
			return Common::SharedPtr<Node>(new MenuPropExprNode(offset, Common::move(menuID), propertyID));
		}
		break;
	case 0x03: // menu item property
		{
			auto menuID = pop();
			auto itemID = pop();
			return Common::SharedPtr<Node>(new MenuItemPropExprNode(offset, Common::move(menuID), Common::move(itemID), propertyID));
		}
		break;
	case 0x04: // sound property
		{
			auto soundID = pop();
			return Common::SharedPtr<Node>(new SoundPropExprNode(offset, Common::move(soundID), propertyID));
		}
		break;
	case 0x05: // resource property - unused?
		return Common::SharedPtr<Node>(new CommentNode(offset, "ERROR: Resource property"));
	case 0x06: // sprite property
		{
			auto spriteID = pop();
			return Common::SharedPtr<Node>(new SpritePropExprNode(offset, Common::move(spriteID), propertyID));
		}
		break;
	case 0x07: // animation property
		return Common::SharedPtr<Node>(new TheExprNode(offset, StandardNames::animationPropertyNames[propertyID]));
	case 0x08: // animation 2 property
		if (propertyID == 0x02 && script->version >= 500) { // the number of castMembers supports castLib selection from Director 5.0
			auto castLib = pop();
			if (!(castLib->type == kLiteralNode && castLib->getValue()->type == kDatumInt && castLib->getValue()->toInt() == 0)) {
				auto castLibNode = Common::SharedPtr<Node>(new MemberExprNode(offset, "castLib", castLib, nullptr));
				return Common::SharedPtr<Node>(new ThePropExprNode(offset, castLibNode, StandardNames::animation2PropertyNames[propertyID]));
			}
		}
		return Common::SharedPtr<Node>(new TheExprNode(offset, StandardNames::animation2PropertyNames[propertyID]));
	case 0x09: // generic cast member
	case 0x0a: // chunk of cast member
	case 0x0b: // field
	case 0x0c: // chunk of field
	case 0x0d: // digital video
	case 0x0e: // bitmap
	case 0x0f: // sound
	case 0x10: // button
	case 0x11: // shape
	case 0x12: // movie
	case 0x13: // script
	case 0x14: // scriptText
	case 0x15: // chunk of scriptText
		{
			auto propName = StandardNames::memberPropertyNames[propertyID];
			Common::SharedPtr<Node> castID;
			if (script->version >= 500) {
				castID = pop();
			}
			auto memberID = pop();
			Common::String prefix;
			if (propertyType == 0x0b || propertyType == 0x0c) {
				prefix = "field";
			} else if (propertyType == 0x14 || propertyType == 0x15) {
				prefix = "script";
			} else {
				prefix = (script->version >= 500) ? "member" : "cast";
			}
			auto member = Common::SharedPtr<Node>(new MemberExprNode(offset, prefix, Common::move(memberID), Common::move(castID)));
			Common::SharedPtr<Node> entity;
			if (propertyType == 0x0a || propertyType == 0x0c || propertyType == 0x15) {
				entity = readChunkRef(offset, Common::move(member));
			} else {
				entity = member;
			}
			return Common::SharedPtr<Node>(new ThePropExprNode(offset, Common::move(entity), propName));
		}
		break;
	default:
		break;
	}
	return Common::SharedPtr<Node>(new CommentNode(offset, Common::String::format("ERROR: Unknown property type %d", propertyType)));
}

Common::SharedPtr<Node> Handler::readChunkRef(uint32 offset, Common::SharedPtr<Node> string) {
	auto lastLine = pop();
	auto firstLine = pop();
	auto lastItem = pop();
	auto firstItem = pop();
	auto lastWord = pop();
	auto firstWord = pop();
	auto lastChar = pop();
	auto firstChar = pop();

	if (!(firstLine->type == kLiteralNode && firstLine->getValue()->type == kDatumInt && firstLine->getValue()->toInt() == 0))
		string = Common::SharedPtr<Node>(new ChunkExprNode(offset, kChunkLine, Common::move(firstLine), Common::move(lastLine), Common::move(string)));
	if (!(firstItem->type == kLiteralNode && firstItem->getValue()->type == kDatumInt && firstItem->getValue()->toInt() == 0))
		string = Common::SharedPtr<Node>(new ChunkExprNode(offset, kChunkItem, Common::move(firstItem), Common::move(lastItem), Common::move(string)));
	if (!(firstWord->type == kLiteralNode && firstWord->getValue()->type == kDatumInt && firstWord->getValue()->toInt() == 0))
		string = Common::SharedPtr<Node>(new ChunkExprNode(offset, kChunkWord, Common::move(firstWord), Common::move(lastWord), Common::move(string)));
	if (!(firstChar->type == kLiteralNode && firstChar->getValue()->type == kDatumInt && firstChar->getValue()->toInt() == 0))
		string = Common::SharedPtr<Node>(new ChunkExprNode(offset, kChunkChar, Common::move(firstChar), Common::move(lastChar), Common::move(string)));

	return string;
}

void Handler::tagLoops() {
	// Tag any jmpifz which is a loop with the loop type
	// (kTagRepeatWhile, kTagRepeatWithIn, kTagRepeatWithTo, kTagRepeatWithDownTo).
	// Tag the instruction which `next repeat` jumps to with kTagNextRepeatTarget.
	// Tag any instructions which are internal loop logic with kTagSkip, so that
	// they will be skipped during translation.

	for (uint32 startIndex = 0; startIndex < bytecodeArray.size(); startIndex++) {
		// All loops begin with jmpifz...
		auto &jmpifz = bytecodeArray[startIndex];
		if (jmpifz.opcode != kOpJmpIfZ)
			continue;

		// ...and end with endrepeat.
		uint32 jmpPos = jmpifz.pos + jmpifz.obj;
		uint32 endIndex = bytecodePosMap[jmpPos];
		auto &endRepeat = bytecodeArray[endIndex - 1];
		if (endRepeat.opcode != kOpEndRepeat || (endRepeat.pos - endRepeat.obj) > jmpifz.pos)
			continue;

		BytecodeTag loopType = identifyLoop(startIndex, endIndex);
		bytecodeArray[startIndex].tag = loopType;

		if (loopType == kTagRepeatWithIn) {
			for (uint32 i = startIndex - 7, end = startIndex - 1; i <= end; i++)
				bytecodeArray[i].tag = kTagSkip;
			for (uint32 i = startIndex + 1, end = startIndex + 5; i <= end; i++)
				bytecodeArray[i].tag = kTagSkip;
			bytecodeArray[endIndex - 3].tag = kTagNextRepeatTarget; // pushint8 1
			bytecodeArray[endIndex - 3].ownerLoop = startIndex;
			bytecodeArray[endIndex - 2].tag = kTagSkip; // add
			bytecodeArray[endIndex - 1].tag = kTagSkip; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
			bytecodeArray[endIndex].tag = kTagSkip; // pop 3
		} else if (loopType == kTagRepeatWithTo || loopType == kTagRepeatWithDownTo) {
			uint32 conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];
			bytecodeArray[conditionStartIndex - 1].tag = kTagSkip; // set
			bytecodeArray[conditionStartIndex].tag = kTagSkip; // get
			bytecodeArray[startIndex - 1].tag = kTagSkip; // lteq / gteq
			bytecodeArray[endIndex - 5].tag = kTagNextRepeatTarget; // pushint8 1 / pushint8 -1
			bytecodeArray[endIndex - 5].ownerLoop = startIndex;
			bytecodeArray[endIndex - 4].tag = kTagSkip; // get
			bytecodeArray[endIndex - 3].tag = kTagSkip; // add
			bytecodeArray[endIndex - 2].tag = kTagSkip; // set
			bytecodeArray[endIndex - 1].tag = kTagSkip; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
		} else if (loopType == kTagRepeatWhile) {
			bytecodeArray[endIndex - 1].tag = kTagNextRepeatTarget; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
		}
	}
}

bool Handler::isRepeatWithIn(uint32 startIndex, uint32 endIndex) {
	if (startIndex < 7 || startIndex > bytecodeArray.size() - 6)
		return false;
	if (!(bytecodeArray[startIndex - 7].opcode == kOpPeek && bytecodeArray[startIndex - 7].obj == 0))
		return false;
	if (!(bytecodeArray[startIndex - 6].opcode == kOpPushArgList && bytecodeArray[startIndex - 6].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex - 5].opcode == kOpExtCall && getName(bytecodeArray[startIndex - 5].obj) == "count"))
		return false;
	if (!(bytecodeArray[startIndex - 4].opcode == kOpPushInt8 && bytecodeArray[startIndex - 4].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex - 3].opcode == kOpPeek && bytecodeArray[startIndex - 3].obj == 0))
		return false;
	if (!(bytecodeArray[startIndex - 2].opcode == kOpPeek && bytecodeArray[startIndex - 2].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex - 1].opcode == kOpLtEq))
		return false;
	// if (!(bytecodeArray[startIndex].opcode == kOpJmpIfZ))
	//     return false;
	if (!(bytecodeArray[startIndex + 1].opcode == kOpPeek && bytecodeArray[startIndex + 1].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex + 2].opcode == kOpPeek && bytecodeArray[startIndex + 2].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex + 3].opcode == kOpPushArgList && bytecodeArray[startIndex + 3].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex + 4].opcode == kOpExtCall && getName(bytecodeArray[startIndex + 4].obj) == "getAt"))
		return false;
	if (!(bytecodeArray[startIndex + 5].opcode == kOpSetGlobal || bytecodeArray[startIndex + 5].opcode == kOpSetProp
			|| bytecodeArray[startIndex + 5].opcode == kOpSetParam || bytecodeArray[startIndex + 5].opcode == kOpSetLocal))
		return false;

	if (endIndex < 3)
		return false;
	if (!(bytecodeArray[endIndex - 3].opcode == kOpPushInt8 && bytecodeArray[endIndex - 3].obj == 1))
		return false;
	if (!(bytecodeArray[endIndex - 2].opcode == kOpAdd))
		return false;
	// if (!(bytecodeArray[startIndex - 1].opcode == kOpEndRepeat))
	//     return false;
	if (!(bytecodeArray[endIndex].opcode == kOpPop && bytecodeArray[endIndex].obj == 3))
		return false;

	return true;
}

BytecodeTag Handler::identifyLoop(uint32 startIndex, uint32 endIndex) {
	if (isRepeatWithIn(startIndex, endIndex))
		return kTagRepeatWithIn;

	if (startIndex < 1)
		return kTagRepeatWhile;

	bool up;
	switch (bytecodeArray[startIndex - 1].opcode) {
	case kOpLtEq:
		up = true;
		break;
	case kOpGtEq:
		up = false;
		break;
	default:
		return kTagRepeatWhile;
	}

	auto &endRepeat = bytecodeArray[endIndex - 1];
	uint32 conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];

	if (conditionStartIndex < 1)
		return kTagRepeatWhile;

	OpCode getOp;
	switch (bytecodeArray[conditionStartIndex - 1].opcode) {
	case kOpSetGlobal:
		getOp = kOpGetGlobal;
		break;
	case kOpSetGlobal2:
		getOp = kOpGetGlobal2;
		break;
	case kOpSetProp:
		getOp = kOpGetProp;
		break;
	case kOpSetParam:
		getOp = kOpGetParam;
		break;
	case kOpSetLocal:
		getOp = kOpGetLocal;
		break;
	default:
		return kTagRepeatWhile;
	}
	OpCode setOp = bytecodeArray[conditionStartIndex - 1].opcode;
	int32 varID = bytecodeArray[conditionStartIndex - 1].obj;

	if (!(bytecodeArray[conditionStartIndex].opcode == getOp && bytecodeArray[conditionStartIndex].obj == varID))
		return kTagRepeatWhile;

	if (endIndex < 5)
		return kTagRepeatWhile;
	if (up) {
		if (!(bytecodeArray[endIndex - 5].opcode == kOpPushInt8 && bytecodeArray[endIndex - 5].obj == 1))
			return kTagRepeatWhile;
	} else {
		if (!(bytecodeArray[endIndex - 5].opcode == kOpPushInt8 && bytecodeArray[endIndex - 5].obj == -1))
			return kTagRepeatWhile;
	}
	if (!(bytecodeArray[endIndex - 4].opcode == getOp && bytecodeArray[endIndex - 4].obj == varID))
		return kTagRepeatWhile;
	if (!(bytecodeArray[endIndex - 3].opcode == kOpAdd))
		return kTagRepeatWhile;
	if (!(bytecodeArray[endIndex - 2].opcode == setOp && bytecodeArray[endIndex - 2].obj == varID))
		return kTagRepeatWhile;

	return up ? kTagRepeatWithTo : kTagRepeatWithDownTo;
}

void Handler::parse() {
	tagLoops();
	stack.clear();
	uint32 i = 0;
	while (i < bytecodeArray.size()) {
		auto &bytecode = bytecodeArray[i];
		uint32 pos = bytecode.pos;
		// exit last block if at end
		while (pos == ast.currentBlock->endPos) {
			auto exitedBlock = ast.currentBlock;
			auto ancestorStmt = ast.currentBlock->ancestorStatement();
			ast.exitBlock();
			if (ancestorStmt) {
				if (ancestorStmt->type == kIfStmtNode) {
					auto ifStatement = static_cast<IfStmtNode *>(ancestorStmt);
					if (ifStatement->hasElse && exitedBlock == ifStatement->block1.get()) {
						ast.enterBlock(ifStatement->block2.get());
					}
				} else if (ancestorStmt->type == kCaseStmtNode) {
					auto caseStmt = static_cast<CaseStmtNode *>(ancestorStmt);
					auto caseLabel = ast.currentBlock->currentCaseLabel;
					if (caseLabel) {
						if (caseLabel->expect == kCaseExpectOtherwise) {
							ast.currentBlock->currentCaseLabel = nullptr;
							caseStmt->addOtherwise(i);
							size_t otherwiseIndex = bytecodePosMap[caseStmt->potentialOtherwisePos];
							bytecodeArray[otherwiseIndex].translation = Common::SharedPtr<Node>(caseStmt->otherwise);
							ast.enterBlock(caseStmt->otherwise->block.get());
						} else if (caseLabel->expect == kCaseExpectEnd) {
							ast.currentBlock->currentCaseLabel = nullptr;
						}
					}
				}
			}
		}
		auto translateSize = translateBytecode(bytecode, i);
		i += translateSize;
	}
}

uint32 Handler::translateBytecode(Bytecode &bytecode, uint32 index) {
	if (bytecode.tag == kTagSkip || bytecode.tag == kTagNextRepeatTarget) {
		// This is internal loop logic. Skip it.
		return 1;
	}

	Common::SharedPtr<Node> translation = nullptr;
	BlockNode *nextBlock = nullptr;

	switch (bytecode.opcode) {
	case kOpRet:
	case kOpRetFactory:
		if (index == bytecodeArray.size() - 1) {
			ast.root->_endOffset = bytecode.pos;
			ast.currentBlock->_endOffset = bytecode.pos;
			return 1; // end of handler
		}
		translation = Common::SharedPtr<Node>(new ExitStmtNode(bytecode.pos));
		break;
	case kOpPushZero:
		translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::SharedPtr<Datum>(new Datum(0))));
		break;
	case kOpMul:
	case kOpAdd:
	case kOpSub:
	case kOpDiv:
	case kOpMod:
	case kOpJoinStr:
	case kOpJoinPadStr:
	case kOpLt:
	case kOpLtEq:
	case kOpNtEq:
	case kOpEq:
	case kOpGt:
	case kOpGtEq:
	case kOpAnd:
	case kOpOr:
	case kOpContainsStr:
	case kOpContains0Str:
		{
			auto b = pop();
			auto a = pop();
			translation = Common::SharedPtr<Node>(new BinaryOpNode(bytecode.pos, bytecode.opcode, Common::move(a), Common::move(b)));
		}
		break;
	case kOpInv:
		{
			auto x = pop();
			translation = Common::SharedPtr<Node>(new InverseOpNode(bytecode.pos, Common::move(x)));
		}
		break;
	case kOpNot:
		{
			auto x = pop();
			translation = Common::SharedPtr<Node>(new NotOpNode(bytecode.pos, Common::move(x)));
		}
		break;
	case kOpGetChunk:
		{
			auto string = pop();
			translation = readChunkRef(bytecode.pos, Common::move(string));
		}
		break;
	case kOpHiliteChunk:
		{
			Common::SharedPtr<Node> castID;
			if (script->version >= 500)
				castID = pop();
			auto fieldID = pop();
			auto field = Common::SharedPtr<Node>(new MemberExprNode(bytecode.pos, "field", Common::move(fieldID), Common::move(castID)));
			auto chunk = readChunkRef(bytecode.pos, Common::move(field));
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = Common::SharedPtr<Node>(new ChunkHiliteStmtNode(bytecode.pos, Common::move(chunk)));
			}
		}
		break;
	case kOpOntoSpr:
		{
			auto secondSprite = pop();
			auto firstSprite = pop();
			translation = Common::SharedPtr<Node>(new SpriteIntersectsExprNode(bytecode.pos, Common::move(firstSprite), Common::move(secondSprite)));
		}
		break;
	case kOpIntoSpr:
		{
			auto secondSprite = pop();
			auto firstSprite = pop();
			translation = Common::SharedPtr<Node>(new SpriteWithinExprNode(bytecode.pos, Common::move(firstSprite), Common::move(secondSprite)));
		}
		break;
	case kOpGetField:
		{
			Common::SharedPtr<Node> castID;
			if (script->version >= 500)
				castID = pop();
			auto fieldID = pop();
			translation = Common::SharedPtr<Node>(new MemberExprNode(bytecode.pos, "field", Common::move(fieldID), Common::move(castID)));
		}
		break;
	case kOpStartTell:
		{
			auto window = pop();
			auto tellStmt = Common::SharedPtr<TellStmtNode>(new TellStmtNode(bytecode.pos, Common::move(window)));
			translation = tellStmt;
			nextBlock = tellStmt->block.get();
		}
		break;
	case kOpEndTell:
		{
			ast.currentBlock->_endOffset = bytecode.pos;
			ast.exitBlock();
			return 1;
		}
		break;
	case kOpPushList:
		{
			auto list = pop();
			list->getValue()->type = kDatumList;
			translation = list;
		}
		break;
	case kOpPushPropList:
		{
			auto list = pop();
			list->getValue()->type = kDatumPropList;
			translation = list;
		}
		break;
	case kOpSwap:
		if (stack.size() >= 2) {
			SWAP(stack[stack.size() - 1], stack[stack.size() - 2]);
		} else {
			warning("kOpSwap: Stack too small!");
		}
		return 1;
	case kOpPushInt8:
	case kOpPushInt16:
	case kOpPushInt32:
		{
			auto i = Common::SharedPtr<Datum>(new Datum((int)bytecode.obj));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(i)));
		}
		break;
	case kOpPushFloat32:
		{
			auto f = Common::SharedPtr<Datum>(new Datum(*(float *)(&bytecode.obj)));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(f)));
		}
		break;
	case kOpPushArgListNoRet:
		{
			auto argCount = bytecode.obj;
			Common::Array<Common::SharedPtr<Node>> args;
			args.resize(argCount);
			while (argCount) {
				argCount--;
				args[argCount] = pop();
			}
			auto argList = Common::SharedPtr<Datum>(new Datum(kDatumArgListNoRet, args));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(argList)));
		}
		break;
	case kOpPushArgList:
		{
			auto argCount = bytecode.obj;
			Common::Array<Common::SharedPtr<Node>> args;
			args.resize(argCount);
			while (argCount) {
				argCount--;
				args[argCount] = pop();
			}
			auto argList = Common::SharedPtr<Datum>(new Datum(kDatumArgList, args));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(argList)));
		}
		break;
	case kOpPushCons:
		{
			int literalID = bytecode.obj / variableMultiplier();
			if (-1 < literalID && (unsigned)literalID < script->literals.size()) {
				translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, script->literals[literalID].value));
			} else {
				translation = Common::SharedPtr<Node>(new ErrorNode(bytecode.pos));
			}
			break;
		}
	case kOpPushSymb:
		{
			auto sym = Common::SharedPtr<Datum>(new Datum(kDatumSymbol, getName(bytecode.obj)));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(sym)));
		}
		break;
	case kOpPushVarRef:
		{
			auto ref = Common::SharedPtr<Datum>(new Datum(kDatumVarRef, getName(bytecode.obj)));
			translation = Common::SharedPtr<Node>(new LiteralNode(bytecode.pos, Common::move(ref)));
		}
		break;
	case kOpGetGlobal:
	case kOpGetGlobal2:
		{
			auto name_ = getName(bytecode.obj);
			translation = Common::SharedPtr<Node>(new VarNode(bytecode.pos, name_));
		}
		break;
	case kOpGetProp:
		translation = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getName(bytecode.obj)));
		break;
	case kOpGetParam:
		translation = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getArgumentName(bytecode.obj / variableMultiplier())));
		break;
	case kOpGetLocal:
		translation = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getLocalName(bytecode.obj / variableMultiplier())));
		break;
	case kOpSetGlobal:
	case kOpSetGlobal2:
		{
			auto varName = getName(bytecode.obj);
			auto var = Common::SharedPtr<Node>(new VarNode(bytecode.pos, varName));
			auto value = pop();
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(var), Common::move(value)));
		}
		break;
	case kOpSetProp:
		{
			auto var = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getName(bytecode.obj)));
			auto value = pop();
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(var), Common::move(value)));
		}
		break;
	case kOpSetParam:
		{
			auto var = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getArgumentName(bytecode.obj / variableMultiplier())));
			auto value = pop();
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(var), Common::move(value)));
		}
		break;
	case kOpSetLocal:
		{
			auto var = Common::SharedPtr<Node>(new VarNode(bytecode.pos, getLocalName(bytecode.obj / variableMultiplier())));
			auto value = pop();
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(var), Common::move(value)));
		}
		break;
	case kOpJmp:
		{
			uint32 targetPos = bytecode.pos + bytecode.obj;
			size_t targetIndex = bytecodePosMap[targetPos];
			auto &targetBytecode = bytecodeArray[targetIndex];
			auto ancestorLoop = ast.currentBlock->ancestorLoop();
			if (ancestorLoop) {
				if (bytecodeArray[targetIndex - 1].opcode == kOpEndRepeat && bytecodeArray[targetIndex - 1].ownerLoop == ancestorLoop->startIndex) {
					translation = Common::SharedPtr<Node>(new ExitRepeatStmtNode(bytecode.pos));
					break;
				} else if (bytecodeArray[targetIndex].tag == kTagNextRepeatTarget && bytecodeArray[targetIndex].ownerLoop == ancestorLoop->startIndex) {
					translation = Common::SharedPtr<Node>(new NextRepeatStmtNode(bytecode.pos));
					break;
				}
			}
			auto &nextBytecode = bytecodeArray[index + 1];
			auto ancestorStatement = ast.currentBlock->ancestorStatement();
			if (ancestorStatement && nextBytecode.pos == ast.currentBlock->endPos) {
				if (ancestorStatement->type == kIfStmtNode) {
					auto ifStmt = static_cast<IfStmtNode *>(ancestorStatement);
					if (ast.currentBlock == ifStmt->block1.get()) {
						ifStmt->hasElse = true;
						ifStmt->block2->_startOffset = ifStmt->block1->_endOffset;
						ifStmt->block2->endPos = targetPos;
						ifStmt->block2->_endOffset = targetPos;
						ifStmt->_endOffset = targetPos;
						return 1; // if statement amended, nothing to push
					}
				} else if (ancestorStatement->type == kCaseStmtNode) {
					auto caseStmt = static_cast<CaseStmtNode *>(ancestorStatement);
					caseStmt->potentialOtherwisePos = bytecode.pos;
					caseStmt->endPos = targetPos;
					caseStmt->_endOffset = targetPos;
					targetBytecode.tag = kTagEndCase;
					return 1;
				}
			}
			if (targetBytecode.opcode == kOpPop && targetBytecode.obj == 1) {
				// This is a case statement starting with 'otherwise'
				auto value = pop();
				auto caseStmt = Common::SharedPtr<CaseStmtNode>(new CaseStmtNode(bytecode.pos, Common::move(value)));
				caseStmt->endPos = targetPos;
				caseStmt->_endOffset = targetPos;
				targetBytecode.tag = kTagEndCase;
				caseStmt->addOtherwise(bytecode.pos);
				translation = caseStmt;
				nextBlock = caseStmt->otherwise->block.get();
				break;
			}
			translation = Common::SharedPtr<Node>(new CommentNode(bytecode.pos, "ERROR: Could not identify jmp"));
		}
		break;
	case kOpEndRepeat:
		// This should normally be tagged kTagSkip or kTagNextRepeatTarget and skipped.
		translation = Common::SharedPtr<Node>(new CommentNode(bytecode.pos, "ERROR: Stray endrepeat"));
		break;
	case kOpJmpIfZ:
		{
			uint32 endPos = bytecode.pos + bytecode.obj;
			uint32 endIndex = bytecodePosMap[endPos];
			switch (bytecode.tag) {
			case kTagRepeatWhile:
				{
					auto condition = pop();
					auto loop = Common::SharedPtr<RepeatWhileStmtNode>(new RepeatWhileStmtNode(bytecode.pos, Common::move(condition), bytecode.pos));
					loop->block->endPos = endPos;
					loop->block->_endOffset = endPos;
					loop->_endOffset = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			case kTagRepeatWithIn:
				{
					auto list = pop();
					Common::String varName = getVarNameFromSet(bytecodeArray[index + 5]);
					auto loop = Common::SharedPtr<RepeatWithInStmtNode>(new RepeatWithInStmtNode(bytecode.pos, varName, Common::move(list), bytecode.pos));
					loop->block->endPos = endPos;
					loop->block->_endOffset = endPos;
					loop->_endOffset = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			case kTagRepeatWithTo:
			case kTagRepeatWithDownTo:
				{
					bool up = (bytecode.tag == kTagRepeatWithTo);
					auto end = pop();
					auto start = pop();
					auto endRepeat = bytecodeArray[endIndex - 1];
					uint32 conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];
					Common::String varName = getVarNameFromSet(bytecodeArray[conditionStartIndex - 1]);
					auto loop = Common::SharedPtr<RepeatWithToStmtNode>(new RepeatWithToStmtNode(bytecode.pos, varName, Common::move(start), up, Common::move(end), bytecode.pos));
					loop->block->endPos = endPos;
					loop->block->_endOffset = endPos;
					loop->_endOffset = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			default:
				{
					auto condition = pop();
					auto ifStmt = Common::SharedPtr<IfStmtNode>(new IfStmtNode(bytecode.pos, Common::move(condition)));
					ifStmt->block1->endPos = endPos;
					ifStmt->block1->_endOffset = endPos;
					ifStmt->_endOffset = endPos;
					translation = ifStmt;
					nextBlock = ifStmt->block1.get();
				}
				break;
			}
		}
		break;
	case kOpLocalCall:
		{
			auto argList = pop();
			translation = Common::SharedPtr<Node>(new CallNode(bytecode.pos, script->handlers[bytecode.obj].name, Common::move(argList)));
		}
		break;
	case kOpExtCall:
	case kOpTellCall:
		{
			Common::String name_ = getName(bytecode.obj);
			auto argList = pop();
			bool isStatement = (argList->getValue()->type == kDatumArgListNoRet);
			auto &rawArgList = argList->getValue()->l;
			size_t nargs = rawArgList.size();
			if (isStatement && name_ == "sound" && nargs > 0 && rawArgList[0]->type == kLiteralNode && rawArgList[0]->getValue()->type == kDatumSymbol) {
				Common::String cmd = rawArgList[0]->getValue()->s;
				rawArgList.erase(rawArgList.begin());
				translation = Common::SharedPtr<Node>(new SoundCmdStmtNode(bytecode.pos, cmd, Common::move(argList)));
			} else if (isStatement && name_ == "play" && nargs <= 2) {
				translation = Common::SharedPtr<Node>(new PlayCmdStmtNode(bytecode.pos, Common::move(argList)));
			} else {
				translation = Common::SharedPtr<Node>(new CallNode(bytecode.pos, name_, Common::move(argList)));
			}
		}
		break;
	case kOpObjCallV4:
		{
			auto object = readVar(bytecode.obj);
			auto argList = pop();
			auto &rawArgList = argList->getValue()->l;
			if (rawArgList.size() > 0) {
				// first arg is a symbol
				// replace it with a variable
				rawArgList[0] = Common::SharedPtr<Node>(new VarNode(bytecode.pos, rawArgList[0]->getValue()->s));
			}
			translation = Common::SharedPtr<Node>(new ObjCallV4Node(bytecode.pos, Common::move(object), Common::move(argList)));
		}
		break;
	case kOpPut:
		{
			PutType putType = static_cast<PutType>((bytecode.obj >> 4) & 0xF);
			uint32 varType = bytecode.obj & 0xF;
			auto var = readVar(varType);
			auto val = pop();
			translation = Common::SharedPtr<Node>(new PutStmtNode(bytecode.pos, putType, Common::move(var), Common::move(val)));
		}
		break;
	case kOpPutChunk:
		{
			PutType putType = static_cast<PutType>((bytecode.obj >> 4) & 0xF);
			uint32 varType = bytecode.obj & 0xF;
			auto var = readVar(varType);
			auto chunk = readChunkRef(bytecode.pos, Common::move(var));
			auto val = pop();
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = Common::SharedPtr<Node>(new PutStmtNode(bytecode.pos, putType, Common::move(chunk), Common::move(val)));
			}
		}
		break;
	case kOpDeleteChunk:
		{
			auto var = readVar(bytecode.obj);
			auto chunk = readChunkRef(bytecode.pos, Common::move(var));
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = Common::SharedPtr<Node>(new ChunkDeleteStmtNode(bytecode.pos, Common::move(chunk)));
			}
		}
		break;
	case kOpGet:
		{
			int propertyID = pop()->getValue()->toInt();
			translation = readV4Property(bytecode.pos, bytecode.obj, propertyID);
		}
		break;
	case kOpSet:
		{
			int propertyID = pop()->getValue()->toInt();
			auto value = pop();
			if (bytecode.obj == 0x00 && 0x01 <= propertyID && propertyID <= 0x05 && value->getValue()->type == kDatumString) {
				// This is either a `set eventScript to "script"` or `when event then script` statement.
				// If the script starts with a space, it's probably a when statement.
				// If the script contains a line break, it's definitely a when statement.
				Common::String script_ = value->getValue()->s;
				if (script_.size() > 0 && (script_[0] == ' ' || script_.find('\r') != Common::String::npos)) {
					translation = Common::SharedPtr<Node>(new WhenStmtNode(bytecode.pos, propertyID, script_));
				}
			}
			if (!translation) {
				auto prop = readV4Property(bytecode.pos, bytecode.obj, propertyID);
				if (prop->type == kCommentNode) { // error comment
					translation = prop;
				} else {
					translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(prop), Common::move(value), true));
				}
			}
		}
		break;
	case kOpGetMovieProp:
		translation = Common::SharedPtr<Node>(new TheExprNode(bytecode.pos, getName(bytecode.obj)));
		break;
	case kOpSetMovieProp:
		{
			auto value = pop();
			auto prop = Common::SharedPtr<TheExprNode>(new TheExprNode(bytecode.pos, getName(bytecode.obj)));
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(prop), Common::move(value)));
		}
		break;
	case kOpGetObjProp:
	case kOpGetChainedProp:
		{
			auto object = pop();
			translation = Common::SharedPtr<Node>(new ObjPropExprNode(bytecode.pos, Common::move(object), getName(bytecode.obj)));
		}
		break;
	case kOpSetObjProp:
		{
			auto value = pop();
			auto object = pop();
			auto prop = Common::SharedPtr<ObjPropExprNode>(new ObjPropExprNode(bytecode.pos, Common::move(object), getName(bytecode.obj)));
			translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(prop), Common::move(value)));
		}
		break;
	case kOpPeek:
		{
			// This op denotes the beginning of a 'repeat with ... in list' statement or a case in a cases statement.

			// In a 'repeat with ... in list' statement, this peeked value is the list.
			// In a cases statement, this is the switch expression.

			auto prevLabel = ast.currentBlock->currentCaseLabel;

			// This must be a case. Find the comparison against the switch expression.
			auto originalStackSize = stack.size();
			uint32 currIndex = index + 1;
			Bytecode *currBytecode = &bytecodeArray[currIndex];
			do {
				translateBytecode(*currBytecode, currIndex);
				currIndex += 1;
				currBytecode = &bytecodeArray[currIndex];
			} while (
				currIndex < bytecodeArray.size()
				&& !(stack.size() == originalStackSize + 1 && (currBytecode->opcode == kOpEq || currBytecode->opcode == kOpNtEq))
			);
			if (currIndex >= bytecodeArray.size()) {
				bytecode.translation = Common::SharedPtr<Node>(new CommentNode(bytecode.pos, "ERROR: Expected eq or nteq!"));
				ast.addStatement(bytecode.translation);
				return currIndex - index + 1;
			}

			// If the comparison is <>, this is followed by another, equivalent case.
			// (e.g. this could be case1 in `case1, case2: statement`)
			bool notEq = (currBytecode->opcode == kOpNtEq);
			Common::SharedPtr<Node> caseValue = pop(); // This is the value the switch expression is compared against.

			currIndex += 1;
			currBytecode = &bytecodeArray[currIndex];
			if (currIndex >= bytecodeArray.size() || currBytecode->opcode != kOpJmpIfZ) {
				bytecode.translation = Common::SharedPtr<Node>(new CommentNode(bytecode.pos, "ERROR: Expected jmpifz!"));
				ast.addStatement(bytecode.translation);
				return currIndex - index + 1;
			}

			auto &jmpifz = *currBytecode;
			auto jmpPos = jmpifz.pos + jmpifz.obj;
			size_t targetIndex = bytecodePosMap[jmpPos];
			auto &targetBytecode = bytecodeArray[targetIndex];
			auto &prevFromTarget = bytecodeArray[targetIndex - 1];
			CaseExpect expect;
			if (notEq) {
				expect = kCaseExpectOr; // Expect an equivalent case after this one.
			} else if (targetBytecode.opcode == kOpPeek) {
				expect = kCaseExpectNext; // Expect a different case after this one.
			} else if (targetBytecode.opcode == kOpPop
					&& targetBytecode.obj == 1
					&& (prevFromTarget.opcode != kOpJmp || prevFromTarget.pos + prevFromTarget.obj == targetBytecode.pos)) {
				expect = kCaseExpectEnd; // Expect the end of the switch statement.
			} else {
				expect = kCaseExpectOtherwise; // Expect an 'otherwise' block.
			}

			auto currLabel = Common::SharedPtr<CaseLabelNode>(new CaseLabelNode(bytecode.pos, Common::move(caseValue), expect));
			jmpifz.translation = currLabel;
			ast.currentBlock->currentCaseLabel = currLabel.get();

			if (!prevLabel) {
				auto peekedValue = pop();
				auto caseStmt = Common::SharedPtr<CaseStmtNode>(new CaseStmtNode(bytecode.pos, Common::move(peekedValue)));
				caseStmt->firstLabel = currLabel;
				currLabel->parent = caseStmt.get();
				bytecode.translation = caseStmt;
				ast.addStatement(caseStmt);
			} else if (prevLabel->expect == kCaseExpectOr) {
				prevLabel->nextOr = currLabel;
				currLabel->parent = prevLabel;
			} else if (prevLabel->expect == kCaseExpectNext) {
				prevLabel->nextLabel = currLabel;
				currLabel->parent = prevLabel;
			}

			// The block doesn't start until the after last equivalent case,
			// so don't create a block yet if we're expecting an equivalent case.
			if (currLabel->expect != kCaseExpectOr) {
				currLabel->block = Common::SharedPtr<BlockNode>(new BlockNode(bytecode.pos));
				currLabel->block->parent = currLabel.get();
				currLabel->block->endPos = jmpPos;
				currLabel->block->_endOffset = jmpPos;
				currLabel->_endOffset = jmpPos;
				ast.enterBlock(currLabel->block.get());
			}

			return currIndex - index + 1;
		}
		break;
	case kOpPop:
		{
			// Pop instructions in 'repeat with in' loops are tagged kTagSkip and skipped.
			if (bytecode.tag == kTagEndCase) {
				// We've already recognized this as the end of a case statement.
				// Attach an 'end case' node for the summary only.
				bytecode.translation = Common::SharedPtr<EndCaseNode>();
				return 1;
			}
			if (bytecode.obj == 1 && stack.size() == 1) {
				// We have an unused value on the stack, so this must be the end
				// of a case statement with no labels.
				auto value = pop();
				translation = Common::SharedPtr<Node>(new CaseStmtNode(bytecode.pos, Common::move(value)));
				break;
			}
			// Otherwise, this pop instruction occurs before a 'return' within
			// a case statement. No translation needed.
			return 1;
		}
		break;
	case kOpTheBuiltin:
		{
			pop(); // empty arglist
			translation = Common::SharedPtr<Node>(new TheExprNode(bytecode.pos, getName(bytecode.obj)));
		}
		break;
	case kOpObjCall:
		{
			Common::String method = getName(bytecode.obj);
			auto argList = pop();
			auto &rawArgList = argList->getValue()->l;
			size_t nargs = rawArgList.size();
			if (method == "getAt" && nargs == 2)  {
				// obj.getAt(i) => obj[i]
				auto obj = rawArgList[0];
				auto prop = rawArgList[1];
				translation = Common::SharedPtr<Node>(new ObjBracketExprNode(bytecode.pos, Common::move(obj), Common::move(prop)));
			} else if (method == "setAt" && nargs == 3) {
				// obj.setAt(i) => obj[i] = val
				auto obj = rawArgList[0];
				auto prop = rawArgList[1];
				auto val = rawArgList[2];
				Common::SharedPtr<Node> propExpr = Common::SharedPtr<Node>(new ObjBracketExprNode(bytecode.pos, Common::move(obj), Common::move(prop)));
				translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(propExpr), Common::move(val)));
			} else if ((method == "getProp" || method == "getPropRef") && (nargs == 3 || nargs == 4) && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.getProp(#prop, i) => obj.prop[i]
				// obj.getProp(#prop, i, i2) => obj.prop[i..i2]
				auto obj = rawArgList[0];
				Common::String propName  = rawArgList[1]->getValue()->s;
				auto i = rawArgList[2];
				auto i2 = (nargs == 4) ? rawArgList[3] : nullptr;
				translation = Common::SharedPtr<Node>(new ObjPropIndexExprNode(bytecode.pos, Common::move(obj), propName, Common::move(i), Common::move(i2)));
			} else if (method == "setProp" && (nargs == 4 || nargs == 5) && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.setProp(#prop, i, val) => obj.prop[i] = val
				// obj.setProp(#prop, i, i2, val) => obj.prop[i..i2] = val
				auto obj = rawArgList[0];
				Common::String propName  = rawArgList[1]->getValue()->s;
				auto i = rawArgList[2];
				auto i2 = (nargs == 5) ? rawArgList[3] : nullptr;
				auto propExpr = Common::SharedPtr<ObjPropIndexExprNode>(new ObjPropIndexExprNode(bytecode.pos, Common::move(obj), propName, Common::move(i), Common::move(i2)));
				auto val = rawArgList[nargs - 1];
				translation = Common::SharedPtr<Node>(new AssignmentStmtNode(bytecode.pos, Common::move(propExpr), Common::move(val)));
			} else if (method == "count" && nargs == 2 && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.count(#prop) => obj.prop.count
				auto obj = rawArgList[0];
				Common::String propName  = rawArgList[1]->getValue()->s;
				auto propExpr = Common::SharedPtr<ObjPropExprNode>(new ObjPropExprNode(bytecode.pos, Common::move(obj), propName));
				translation = Common::SharedPtr<Node>(new ObjPropExprNode(bytecode.pos, Common::move(propExpr), "count"));
			} else if ((method == "setContents" || method == "setContentsAfter" || method == "setContentsBefore") && nargs == 2) {
				// var.setContents(val) => put val into var
				// var.setContentsAfter(val) => put val after var
				// var.setContentsBefore(val) => put val before var
				PutType putType;
				if (method == "setContents") {
					putType = kPutInto;
				} else if (method == "setContentsAfter") {
					putType = kPutAfter;
				} else {
					putType = kPutBefore;
				}
				auto var = rawArgList[0];
				auto val = rawArgList[1];
				translation = Common::SharedPtr<Node>(new PutStmtNode(bytecode.pos, putType, Common::move(var), Common::move(val)));
			} else if (method == "hilite" && nargs == 1) {
				// chunk.hilite() => hilite chunk
				auto chunk = rawArgList[0];
				translation = Common::SharedPtr<Node>(new ChunkHiliteStmtNode(bytecode.pos, chunk));
			} else if (method == "delete" && nargs == 1) {
				// chunk.delete() => delete chunk
				auto chunk = rawArgList[0];
				translation = Common::SharedPtr<Node>(new ChunkDeleteStmtNode(bytecode.pos, chunk));
			} else {
				translation = Common::SharedPtr<Node>(new ObjCallNode(bytecode.pos, method, Common::move(argList)));
			}
		}
		break;
	case kOpPushChunkVarRef:
		translation = readVar(bytecode.obj);
		break;
	case kOpGetTopLevelProp:
		{
			auto name_ = getName(bytecode.obj);
			translation = Common::SharedPtr<VarNode>(new VarNode(bytecode.pos, name_));
		}
		break;
	case kOpNewObj:
		{
			auto objType = getName(bytecode.obj);
			auto objArgs = pop();
			translation = Common::SharedPtr<NewObjNode>(new NewObjNode(bytecode.pos, objType, Common::move(objArgs)));
		}
		break;
	default:
		{
			auto commentText = StandardNames::getOpcodeName(bytecode.opID);
			if (bytecode.opcode >= 0x40)
				commentText += Common::String::format(" %d", bytecode.obj);
			translation = Common::SharedPtr<CommentNode>(new CommentNode(bytecode.pos, commentText));
			stack.clear(); // Clear stack so later bytecode won't be too screwed up
		}
	}

	if (!translation)
		translation = Common::SharedPtr<ErrorNode>(new ErrorNode(bytecode.pos));

	bytecode.translation = translation;
	if (translation->isExpression) {
		stack.push_back(Common::move(translation));
	} else {
		ast.addStatement(Common::move(translation));
	}

	if (nextBlock)
		ast.enterBlock(nextBlock);

	return 1;
}

Common::String posToString(int32 pos) {
	return Common::String::format("[%3d]", pos);
}

void Handler::writeBytecodeText(CodeWriterVisitor &code) const {
	bool isMethod = script->isFactory();

	if (!isGenericEvent) {
		if (isMethod) {
			code.write("method ");
		} else {
			code.write("on ");
		}
		code.write(name);
		if (argumentNames.size() > 0) {
			code.write(" ");
			for (size_t i = 0; i < argumentNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(argumentNames[i]);
			}
		}
		code.writeLine();
		code.indent();
	}
	for (auto &bytecode : bytecodeArray) {
		code.write(posToString(bytecode.pos));
		code.write(" ");
		code.write(StandardNames::getOpcodeName(bytecode.opID));
		switch (bytecode.opcode) {
		case kOpJmp:
		case kOpJmpIfZ:
			code.write(" ");
			code.write(posToString(bytecode.pos + bytecode.obj));
			break;
		case kOpEndRepeat:
			code.write(" ");
			code.write(posToString(bytecode.pos - bytecode.obj));
			break;
		case kOpPushFloat32:
			code.write(" ");
			code.write(Common::String::format("%g", (*(const float *)(&bytecode.obj))));
			break;
		default:
			if (bytecode.opID > 0x40) {
				code.write(" ");
				code.write(Common::String::format("%d", bytecode.obj));
			}
			break;
		}
		if (bytecode.translation) {
			code.write(" ...");
			while (code.lineWidth() < 49) {
				code.write(".");
			}
			code.write(" ");
			if (bytecode.translation->isExpression) {
				code.write("<");
			}
			bytecode.translation->accept(code);
			if (bytecode.translation->isExpression) {
				code.write(">");
			}
		}
		code.writeLine();
	}
	if (!isGenericEvent) {
		code.unindent();
		if (!isMethod) {
			code.writeLine("end");
		}
	}
}

} // namespace LingoDec
