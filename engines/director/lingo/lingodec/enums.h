/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_ENUMS_H
#define LINGODEC_ENUMS_H

namespace LingoDec {

enum OpCode {
	// single-byte
	kOpRet				= 0x01,
	kOpRetFactory		= 0x02,
	kOpPushZero			= 0x03,
	kOpMul				= 0x04,
	kOpAdd				= 0x05,
	kOpSub				= 0x06,
	kOpDiv				= 0x07,
	kOpMod				= 0x08,
	kOpInv				= 0x09,
	kOpJoinStr			= 0x0a,
	kOpJoinPadStr		= 0x0b,
	kOpLt				= 0x0c,
	kOpLtEq				= 0x0d,
	kOpNtEq				= 0x0e,
	kOpEq				= 0x0f,
	kOpGt				= 0x10,
	kOpGtEq				= 0x11,
	kOpAnd				= 0x12,
	kOpOr				= 0x13,
	kOpNot				= 0x14,
	kOpContainsStr		= 0x15,
	kOpContains0Str		= 0x16,
	kOpGetChunk			= 0x17,
	kOpHiliteChunk		= 0x18,
	kOpOntoSpr			= 0x19,
	kOpIntoSpr			= 0x1a,
	kOpGetField			= 0x1b,
	kOpStartTell		= 0x1c,
	kOpEndTell			= 0x1d,
	kOpPushList			= 0x1e,
	kOpPushPropList		= 0x1f,
	kOpSwap				= 0x21,

	// multi-byte
	kOpPushInt8			= 0x41,
	kOpPushArgListNoRet	= 0x42,
	kOpPushArgList		= 0x43,
	kOpPushCons			= 0x44,
	kOpPushSymb			= 0x45,
	kOpPushVarRef		= 0x46,
	kOpGetGlobal2		= 0x48,
	kOpGetGlobal		= 0x49,
	kOpGetProp			= 0x4a,
	kOpGetParam			= 0x4b,
	kOpGetLocal			= 0x4c,
	kOpSetGlobal2		= 0x4e,
	kOpSetGlobal		= 0x4f,
	kOpSetProp			= 0x50,
	kOpSetParam			= 0x51,
	kOpSetLocal			= 0x52,
	kOpJmp				= 0x53,
	kOpEndRepeat		= 0x54,
	kOpJmpIfZ			= 0x55,
	kOpLocalCall		= 0x56,
	kOpExtCall			= 0x57,
	kOpObjCallV4		= 0x58,
	kOpPut				= 0x59,
	kOpPutChunk			= 0x5a,
	kOpDeleteChunk		= 0x5b,
	kOpGet				= 0x5c,
	kOpSet				= 0x5d,
	kOpGetMovieProp		= 0x5f,
	kOpSetMovieProp		= 0x60,
	kOpGetObjProp		= 0x61,
	kOpSetObjProp		= 0x62,
	kOpTellCall			= 0x63,
	kOpPeek				= 0x64,
	kOpPop				= 0x65,
	kOpTheBuiltin		= 0x66,
	kOpObjCall			= 0x67,
	kOpPushChunkVarRef	= 0x6d,
	kOpPushInt16		= 0x6e,
	kOpPushInt32		= 0x6f,
	kOpGetChainedProp	= 0x70,
	kOpPushFloat32		= 0x71,
	kOpGetTopLevelProp	= 0x72,
	kOpNewObj			= 0x73
};

enum DatumType {
	kDatumVoid,
	kDatumSymbol,
	kDatumVarRef,
	kDatumString,
	kDatumInt,
	kDatumFloat,
	kDatumList,
	kDatumArgList,
	kDatumArgListNoRet,
	kDatumPropList
};

enum ChunkExprType {
	kChunkChar	= 0x01,
	kChunkWord	= 0x02,
	kChunkItem	= 0x03,
	kChunkLine	= 0x04
};

enum PutType {
	kPutInto	= 0x01,
	kPutAfter	= 0x02,
	kPutBefore	= 0x03
};

enum NodeType {
	kNoneNode,
	kErrorNode,
	kTempNode,
	kCommentNode,
	kLiteralNode,
	kBlockNode,
	kHandlerNode,
	kExitStmtNode,
	kInverseOpNode,
	kNotOpNode,
	kBinaryOpNode,
	kChunkExprNode,
	kChunkHiliteStmtNode,
	kChunkDeleteStmtNode,
	kSpriteIntersectsExprNode,
	kSpriteWithinExprNode,
	kMemberExprNode,
	kVarNode,
	kAssignmentStmtNode,
	kIfStmtNode,
	kRepeatWhileStmtNode,
	kRepeatWithInStmtNode,
	kRepeatWithToStmtNode,
	kCaseStmtNode,
	kCaseLabelNode,
	kOtherwiseNode,
	kEndCaseNode,
	kTellStmtNode,
	kSoundCmdStmtNode,
	kPlayCmdStmtNode,
	kCallNode,
	kObjCallNode,
	kObjCallV4Node,
	kTheExprNode,
	kLastStringChunkExprNode,
	kStringChunkCountExprNode,
	kMenuPropExprNode,
	kMenuItemPropExprNode,
	kSoundPropExprNode,
	kSpritePropExprNode,
	kThePropExprNode,
	kObjPropExprNode,
	kObjBracketExprNode,
	kObjPropIndexExprNode,
	kExitRepeatStmtNode,
	kNextRepeatStmtNode,
	kPutStmtNode,
	kWhenStmtNode,
	kNewObjNode
};

enum BytecodeTag {
	kTagNone,
	kTagSkip,
	kTagRepeatWhile,
	kTagRepeatWithIn,
	kTagRepeatWithTo,
	kTagRepeatWithDownTo,
	kTagNextRepeatTarget,
	kTagEndCase
};

enum CaseExpect {
	kCaseExpectEnd,
	kCaseExpectOr,
	kCaseExpectNext,
	kCaseExpectOtherwise
};

enum ScriptFlag {
	kScriptFlagUnused		= (1 << 0x0),
	kScriptFlagFuncsGlobal	= (1 << 0x1),
	kScriptFlagVarsGlobal	= (1 << 0x2),	// Occurs in event scripts (which have no local vars). Correlated with use of alternate global var opcodes.
	kScriptFlagUnk3			= (1 << 0x3),
	kScriptFlagFactoryDef	= (1 << 0x4),
	kScriptFlagUnk5			= (1 << 0x5),
	kScriptFlagUnk6			= (1 << 0x6),
	kScriptFlagUnk7			= (1 << 0x7),
	kScriptFlagHasFactory	= (1 << 0x8),
	kScriptFlagEventScript	= (1 << 0x9),
	kScriptFlagEventScript2	= (1 << 0xa),
	kScriptFlagUnkB			= (1 << 0xb),
	kScriptFlagUnkC			= (1 << 0xc),
	kScriptFlagUnkD			= (1 << 0xd),
	kScriptFlagUnkE			= (1 << 0xe),
	kScriptFlagUnkF			= (1 << 0xf)
};

enum LiteralType {
	kLiteralString	= 1,
	kLiteralInt		= 4,
	kLiteralFloat	= 9
};

}

#endif // LINGODEC_ENUMS_H
