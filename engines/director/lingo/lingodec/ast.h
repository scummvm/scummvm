/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_AST_H
#define LINGODEC_AST_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/util.h"
#include "./codewriter.h"
#include "./enums.h"

namespace LingoDec {

struct CaseLabelNode;
struct Handler;
struct LoopNode;
struct Node;
struct RepeatWithInStmtNode;

/* Datum */

struct Datum {
	DatumType type;
	int i;
	double f;
	Common::String s;
	Common::Array<Common::SharedPtr<Node>> l;

	Datum() {
		type = kDatumVoid;
	}
	Datum(int val) {
		type = kDatumInt;
		i = val;
	}
	Datum(double val) {
		type = kDatumFloat;
		f = val;
	}
	Datum(DatumType t, Common::String val) {
		type = t;
		s = val;
	}
	Datum(DatumType t, Common::Array<Common::SharedPtr<Node>> val) {
		type = t;
		l = val;
	}

	int toInt();
	void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;
	bool isLabel;
	bool isLoop;
	Node *parent;

	Node(NodeType t) : type(t), isExpression(false), isStatement(false), isLabel(false), isLoop(false), parent(nullptr) {}
	virtual ~Node() = default;
	virtual void writeScriptText(CodeWriter&, bool, bool) const {}
	virtual Common::SharedPtr<Datum> getValue();
	Node *ancestorStatement();
	LoopNode *ancestorLoop();
	virtual bool hasSpaces(bool dot);
};

/* ExprNode */

struct ExprNode : Node {
	ExprNode(NodeType t) : Node(t) {
		isExpression = true;
	}
	virtual ~ExprNode() = default;
};

/* StmtNode */

struct StmtNode : Node {
	StmtNode(NodeType t) : Node(t) {
		isStatement = true;
	}
	virtual ~StmtNode() = default;
};

/* LabelNode */

struct LabelNode : Node {
	LabelNode(NodeType t) : Node(t) {
		isLabel = true;
	}
	virtual ~LabelNode() = default;
};

/* LoopNode */

struct LoopNode : StmtNode {
	uint32_t startIndex;

	LoopNode(NodeType t, uint32_t startIndex) : StmtNode(t), startIndex(startIndex) {
		isLoop = true;
	}
	virtual ~LoopNode() = default;
};

/* ErrorNode */

struct ErrorNode : ExprNode {
	ErrorNode() : ExprNode(kErrorNode) {}
	virtual ~ErrorNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* CommentNode */

struct CommentNode : Node {
	Common::String text;

	CommentNode(Common::String t) : Node(kCommentNode), text(t) {}
	virtual ~CommentNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* LiteralNode */

struct LiteralNode : ExprNode {
	Common::SharedPtr<Datum> value;

	LiteralNode(Common::SharedPtr<Datum> d) : ExprNode(kLiteralNode) {
		value = Common::move(d);
	}
	virtual ~LiteralNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual Common::SharedPtr<Datum> getValue();
	virtual bool hasSpaces(bool dot);
};

/* BlockNode */

struct BlockNode : Node {
	Common::Array<Common::SharedPtr<Node>> children;

	// for use during translation:
	uint32_t endPos;
	CaseLabelNode *currentCaseLabel;

	BlockNode() : Node(kBlockNode), endPos(-1), currentCaseLabel(nullptr) {}
	virtual ~BlockNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	void addChild(Common::SharedPtr<Node> child);
};

/* HandlerNode */

struct HandlerNode : Node {
	Handler *handler;
	Common::SharedPtr<BlockNode> block;

	HandlerNode(Handler *h)
		: Node(kHandlerNode), handler(h) {
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~HandlerNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* ExitStmtNode */

struct ExitStmtNode : StmtNode {
	ExitStmtNode() : StmtNode(kExitStmtNode) {}
	virtual ~ExitStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* InverseOpNode */

struct InverseOpNode : ExprNode {
	Common::SharedPtr<Node> operand;

	InverseOpNode(Common::SharedPtr<Node> o) : ExprNode(kInverseOpNode) {
		operand = Common::move(o);
		operand->parent = this;
	}
	virtual ~InverseOpNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* NotOpNode */

struct NotOpNode : ExprNode {
	Common::SharedPtr<Node> operand;

	NotOpNode(Common::SharedPtr<Node> o) : ExprNode(kNotOpNode) {
		operand = Common::move(o);
		operand->parent = this;
	}
	virtual ~NotOpNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* BinaryOpNode */

struct BinaryOpNode : ExprNode {
	OpCode opcode;
	Common::SharedPtr<Node> left;
	Common::SharedPtr<Node> right;

	BinaryOpNode(OpCode op, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kBinaryOpNode), opcode(op) {
		left = Common::move(a);
		left->parent = this;
		right = Common::move(b);
		right->parent = this;
	}
	virtual ~BinaryOpNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual unsigned int getPrecedence() const;
};

/* ChunkExprNode */

struct ChunkExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> first;
	Common::SharedPtr<Node> last;
	Common::SharedPtr<Node> string;

	ChunkExprNode(ChunkExprType t, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b, Common::SharedPtr<Node> s)
		: ExprNode(kChunkExprNode), type(t) {
		first = Common::move(a);
		first->parent = this;
		last = Common::move(b);
		last->parent = this;
		string = Common::move(s);
		string->parent = this;
	}
	virtual ~ChunkExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* ChunkHiliteStmtNode */

struct ChunkHiliteStmtNode : StmtNode {
	Common::SharedPtr<Node> chunk;

	ChunkHiliteStmtNode(Common::SharedPtr<Node> c) : StmtNode(kChunkHiliteStmtNode) {
		chunk = Common::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkHiliteStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* ChunkDeleteStmtNode */

struct ChunkDeleteStmtNode : StmtNode {
	Common::SharedPtr<Node> chunk;

	ChunkDeleteStmtNode(Common::SharedPtr<Node> c) : StmtNode(kChunkDeleteStmtNode) {
		chunk = Common::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkDeleteStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* SpriteIntersectsExprNode */

struct SpriteIntersectsExprNode : ExprNode {
	Common::SharedPtr<Node> firstSprite;
	Common::SharedPtr<Node> secondSprite;

	SpriteIntersectsExprNode(Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kSpriteIntersectsExprNode) {
		firstSprite = Common::move(a);
		firstSprite->parent = this;
		secondSprite = Common::move(b);
		secondSprite->parent = this;
	}
	virtual ~SpriteIntersectsExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* SpriteWithinExprNode */

struct SpriteWithinExprNode : ExprNode {
	Common::SharedPtr<Node> firstSprite;
	Common::SharedPtr<Node> secondSprite;

	SpriteWithinExprNode(Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kSpriteWithinExprNode) {
		firstSprite = Common::move(a);
		firstSprite->parent = this;
		secondSprite = Common::move(b);
		secondSprite->parent = this;
	}
	virtual ~SpriteWithinExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* MemberExprNode */

struct MemberExprNode : ExprNode {
	Common::String type;
	Common::SharedPtr<Node> memberID;
	Common::SharedPtr<Node> castID;

	MemberExprNode(Common::String type, Common::SharedPtr<Node> memberID, Common::SharedPtr<Node> castID)
		: ExprNode(kMemberExprNode), type(type) {
		this->memberID = Common::move(memberID);
		this->memberID->parent = this;
		if (castID) {
			this->castID = Common::move(castID);
			this->castID->parent = this;
		}
	}
	virtual ~MemberExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String varName;

	VarNode(Common::String v) : ExprNode(kVarNode), varName(v) {}
	virtual ~VarNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* AssignmentStmtNode */

struct AssignmentStmtNode : StmtNode {
	Common::SharedPtr<Node> variable;
	Common::SharedPtr<Node> value;
	bool forceVerbose;

	AssignmentStmtNode(Common::SharedPtr<Node> var, Common::SharedPtr<Node> val, bool forceVerbose = false)
		: StmtNode(kAssignmentStmtNode), forceVerbose(forceVerbose) {
		variable = Common::move(var);
		variable->parent = this;
		value = Common::move(val);
		value->parent = this;
	}

	virtual ~AssignmentStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* IfStmtNode */

struct IfStmtNode : StmtNode {
	bool hasElse;
	Common::SharedPtr<Node> condition;
	Common::SharedPtr<BlockNode> block1;
	Common::SharedPtr<BlockNode> block2;

	IfStmtNode(Common::SharedPtr<Node> c) : StmtNode(kIfStmtNode), hasElse(false) {
		condition = Common::move(c);
		condition->parent = this;
		block1 = Common::SharedPtr<BlockNode>(new BlockNode());
		block1->parent = this;
		block2 = Common::SharedPtr<BlockNode>(new BlockNode());
		block2->parent = this;
	}
	virtual ~IfStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWhileStmtNode */

struct RepeatWhileStmtNode : LoopNode {
	Common::SharedPtr<Node> condition;
	Common::SharedPtr<BlockNode> block;

	RepeatWhileStmtNode(uint32_t startIndex, Common::SharedPtr<Node> c)
		: LoopNode(kRepeatWhileStmtNode, startIndex) {
		condition = Common::move(c);
		condition->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~RepeatWhileStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWithInStmtNode */

struct RepeatWithInStmtNode : LoopNode {
	Common::String varName;
	Common::SharedPtr<Node> list;
	Common::SharedPtr<BlockNode> block;

	RepeatWithInStmtNode(uint32_t startIndex, Common::String v, Common::SharedPtr<Node> l)
		: LoopNode(kRepeatWithInStmtNode, startIndex) {
		varName = v;
		list = Common::move(l);
		list->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~RepeatWithInStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWithToStmtNode */

struct RepeatWithToStmtNode : LoopNode {
	Common::String varName;
	Common::SharedPtr<Node> start;
	bool up;
	Common::SharedPtr<Node> end;
	Common::SharedPtr<BlockNode> block;

	RepeatWithToStmtNode(uint32_t startIndex, Common::String v, Common::SharedPtr<Node> s, bool up, Common::SharedPtr<Node> e)
		: LoopNode(kRepeatWithToStmtNode, startIndex), up(up) {
		varName = v;
		start = Common::move(s);
		start->parent = this;
		end = Common::move(e);
		end->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~RepeatWithToStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* CaseLabelNode */

struct CaseLabelNode : LabelNode {
	Common::SharedPtr<Node> value;
	CaseExpect expect;

	Common::SharedPtr<CaseLabelNode> nextOr;

	Common::SharedPtr<CaseLabelNode> nextLabel;
	Common::SharedPtr<BlockNode> block;

	CaseLabelNode(Common::SharedPtr<Node> v, CaseExpect e) : LabelNode(kCaseLabelNode), expect(e) {
		value = Common::move(v);
		value->parent = this;
	}
	virtual ~CaseLabelNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* OtherwiseNode */

struct OtherwiseNode : LabelNode {
	Common::SharedPtr<BlockNode> block;

	OtherwiseNode() : LabelNode(kOtherwiseNode) {
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~OtherwiseNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* EndCaseNode */

struct EndCaseNode : LabelNode {
	EndCaseNode() : LabelNode(kEndCaseNode) {}
	virtual ~EndCaseNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* CaseStmtNode */

struct CaseStmtNode : StmtNode {
	Common::SharedPtr<Node> value;
	Common::SharedPtr<CaseLabelNode> firstLabel;
	Common::SharedPtr<OtherwiseNode> otherwise;

	// for use during translation:
	int32_t endPos = -1;
	int32_t potentialOtherwisePos = -1;

	CaseStmtNode(Common::SharedPtr<Node> v) : StmtNode(kCaseStmtNode) {
		value = Common::move(v);
		value->parent = this;
	}
	virtual ~CaseStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	void addOtherwise();
};

/* TellStmtNode */

struct TellStmtNode : StmtNode {
	Common::SharedPtr<Node> window;
	Common::SharedPtr<BlockNode> block;

	TellStmtNode(Common::SharedPtr<Node> w) : StmtNode(kTellStmtNode) {
		window = Common::move(w);
		window->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode());
		block->parent = this;
	}
	virtual ~TellStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* SoundCmdStmtNode */

struct SoundCmdStmtNode : StmtNode {
	Common::String cmd;
	Common::SharedPtr<Node> argList;

	SoundCmdStmtNode(Common::String c, Common::SharedPtr<Node> a) : StmtNode(kSoundCmdStmtNode) {
		cmd = c;
		argList = Common::move(a);
		argList->parent = this;
	}
	virtual ~SoundCmdStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* PlayCmdStmtNode */

struct PlayCmdStmtNode : StmtNode {
	Common::SharedPtr<Node> argList;

	PlayCmdStmtNode(Common::SharedPtr<Node> a) : StmtNode(kPlayCmdStmtNode) {
		argList = Common::move(a);
		argList->parent = this;
	}
	virtual ~PlayCmdStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* CallNode */

struct CallNode : Node {
	Common::String name;
	Common::SharedPtr<Node> argList;

	CallNode(Common::String n, Common::SharedPtr<Node> a) : Node(kCallNode) {
		name = n;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~CallNode() = default;
	bool noParens() const;
	bool isMemberExpr() const;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjCallNode */

struct ObjCallNode : Node {
	Common::String name;
	Common::SharedPtr<Node> argList;

	ObjCallNode(Common::String n, Common::SharedPtr<Node> a) : Node(kObjCallNode) {
		name = n;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~ObjCallNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjCallV4Node */

struct ObjCallV4Node : Node {
	Common::SharedPtr<Node> obj;
	Common::SharedPtr<Node> argList;

	ObjCallV4Node(Common::SharedPtr<Node> o, Common::SharedPtr<Node> a) : Node(kObjCallV4Node) {
		obj = o;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~ObjCallV4Node() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* TheExprNode */

struct TheExprNode : ExprNode {
	Common::String prop;

	TheExprNode(Common::String p) : ExprNode(kTheExprNode), prop(p) {}
	virtual ~TheExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* LastStringChunkExprNode */

struct LastStringChunkExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> obj;

	LastStringChunkExprNode(ChunkExprType t, Common::SharedPtr<Node> o)
		: ExprNode(kLastStringChunkExprNode), type(t) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual ~LastStringChunkExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* StringChunkCountExprNode */

struct StringChunkCountExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> obj;

	StringChunkCountExprNode(ChunkExprType t, Common::SharedPtr<Node> o)
		: ExprNode(kStringChunkCountExprNode), type(t) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual ~StringChunkCountExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* MenuPropExprNode */

struct MenuPropExprNode : ExprNode {
	Common::SharedPtr<Node> menuID;
	unsigned int prop;

	MenuPropExprNode(Common::SharedPtr<Node> m, unsigned int p)
		: ExprNode(kMenuPropExprNode), prop(p) {
		menuID = Common::move(m);
		menuID->parent = this;
	}
	virtual ~MenuPropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* MenuItemPropExprNode */

struct MenuItemPropExprNode : ExprNode {
	Common::SharedPtr<Node> menuID;
	Common::SharedPtr<Node> itemID;
	unsigned int prop;

	MenuItemPropExprNode(Common::SharedPtr<Node> m, Common::SharedPtr<Node> i, unsigned int p)
		: ExprNode(kMenuItemPropExprNode), prop(p) {
		menuID = Common::move(m);
		menuID->parent = this;
		itemID = Common::move(i);
		itemID->parent = this;
	}
	virtual ~MenuItemPropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* SoundPropExprNode */

struct SoundPropExprNode : ExprNode {
	Common::SharedPtr<Node> soundID;
	unsigned int prop;

	SoundPropExprNode(Common::SharedPtr<Node> s, unsigned int p)
		: ExprNode(kSoundPropExprNode), prop(p) {
		soundID = Common::move(s);
		soundID->parent = this;
	}
	virtual ~SoundPropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* SpritePropExprNode */

struct SpritePropExprNode : ExprNode {
	Common::SharedPtr<Node> spriteID;
	unsigned int prop;

	SpritePropExprNode(Common::SharedPtr<Node> s, unsigned int p)
		: ExprNode(kSpritePropExprNode), prop(p) {
		spriteID = Common::move(s);
		spriteID->parent = this;
	}
	virtual ~SpritePropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* ThePropExprNode */

struct ThePropExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;

	ThePropExprNode(Common::SharedPtr<Node> o, Common::String p)
		: ExprNode(kThePropExprNode), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual ~ThePropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* ObjPropExprNode */

struct ObjPropExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;

	ObjPropExprNode(Common::SharedPtr<Node> o, Common::String p)
		: ExprNode(kObjPropExprNode), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual ~ObjPropExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjBracketExprNode */

struct ObjBracketExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::SharedPtr<Node> prop;

	ObjBracketExprNode(Common::SharedPtr<Node> o, Common::SharedPtr<Node> p)
		: ExprNode(kObjBracketExprNode) {
		obj = Common::move(o);
		obj->parent = this;
		prop = Common::move(p);
		prop->parent = this;
	}
	virtual ~ObjBracketExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjPropIndexExprNode */

struct ObjPropIndexExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;
	Common::SharedPtr<Node> index;
	Common::SharedPtr<Node> index2;

	ObjPropIndexExprNode(Common::SharedPtr<Node> o, Common::String p, Common::SharedPtr<Node> i, Common::SharedPtr<Node> i2)
		: ExprNode(kObjPropIndexExprNode), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
		index = Common::move(i);
		index->parent = this;
		if (i2) {
			index2 = Common::move(i2);
			index2->parent = this;
		}
	}
	virtual ~ObjPropIndexExprNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : StmtNode {
	ExitRepeatStmtNode() : StmtNode(kExitRepeatStmtNode) {}
	virtual ~ExitRepeatStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : StmtNode {
	NextRepeatStmtNode() : StmtNode(kNextRepeatStmtNode) {}
	virtual ~NextRepeatStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* PutStmtNode */

struct PutStmtNode : StmtNode {
	PutType type;
	Common::SharedPtr<Node> variable;
	Common::SharedPtr<Node> value;

	PutStmtNode(PutType t, Common::SharedPtr<Node> var, Common::SharedPtr<Node> val)
		: StmtNode(kPutStmtNode), type(t) {
		variable = Common::move(var);
		variable->parent = this;
		value = Common::move(val);
		value->parent = this;
	}
	virtual ~PutStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* WhenStmtNode */

struct WhenStmtNode : StmtNode {
	int event;
	Common::String script;

	WhenStmtNode(int e, Common::String s)
		: StmtNode(kWhenStmtNode), event(e), script(s) {}
	virtual ~WhenStmtNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* NewObjNode */

struct NewObjNode : ExprNode {
	Common::String objType;
	Common::SharedPtr<Node> objArgs;

	NewObjNode(Common::String o, Common::SharedPtr<Node> args) : ExprNode(kNewObjNode), objType(o), objArgs(args) {}
	virtual ~NewObjNode() = default;
	virtual void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
};

/* AST */

struct AST {
	Common::SharedPtr<HandlerNode> root;
	BlockNode *currentBlock;

	AST(Handler *handler){
		root = Common::SharedPtr<HandlerNode>(new HandlerNode(handler));
		currentBlock = root->block.get();
	}

	void writeScriptText(CodeWriter &code, bool dot, bool sum) const;
	void addStatement(Common::SharedPtr<Node> statement);
	void enterBlock(BlockNode *block);
	void exitBlock();
};

} // namespace LingoDec

#endif // LINGODEC_AST_H
