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
};

class NodeVisitor;

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;
	bool isLabel;
	bool isLoop;
	Node *parent;
	uint32 _startOffset;
	uint32 _endOffset;

	Node(NodeType t, uint32 offset) : type(t), isExpression(false), isStatement(false), isLabel(false), isLoop(false), parent(nullptr), _startOffset(offset), _endOffset(offset) {}
	virtual ~Node() {}
	virtual void accept(NodeVisitor& visitor) const = 0;
	virtual Common::SharedPtr<Datum> getValue();
	Node *ancestorStatement();
	LoopNode *ancestorLoop();
	virtual bool hasSpaces(bool dot);
};

/* ExprNode */

struct ExprNode : Node {
	ExprNode(NodeType t, uint32 offset) : Node(t, offset) {
		isExpression = true;
	}
};

/* StmtNode */

struct StmtNode : Node {
	StmtNode(NodeType t, uint32 offset) : Node(t, offset) {
		isStatement = true;
	}
};

/* LabelNode */

struct LabelNode : Node {
	LabelNode(NodeType t, uint32 offset) : Node(t, offset) {
		isLabel = true;
	}
};

/* LoopNode */

struct LoopNode : StmtNode {
	uint32 startIndex;

	LoopNode(NodeType t, uint32 startIndex_, uint32 offset) : StmtNode(t, offset), startIndex(startIndex_) {
		isLoop = true;
	}
};

/* ErrorNode */

struct ErrorNode : ExprNode {
	explicit ErrorNode(uint32 offset) : ExprNode(kErrorNode, offset) {}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* CommentNode */

struct CommentNode : Node {
	Common::String text;

	CommentNode(uint32 offset, Common::String t) : Node(kCommentNode, offset), text(t) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* LiteralNode */

struct LiteralNode : ExprNode {
	Common::SharedPtr<Datum> value;

	LiteralNode(uint32 offset, Common::SharedPtr<Datum> d) : ExprNode(kLiteralNode, offset) {
		value = Common::move(d);
	}
	virtual Common::SharedPtr<Datum> getValue() override;
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* BlockNode */

struct BlockNode : Node {
	Common::Array<Common::SharedPtr<Node>> children;

	// for use during translation:
	uint32 endPos;
	CaseLabelNode *currentCaseLabel;

	explicit BlockNode(uint32 offset) : Node(kBlockNode, offset), endPos(-1), currentCaseLabel(nullptr) {}
	void addChild(Common::SharedPtr<Node> child);
	virtual void accept(NodeVisitor &visitor) const override;
};

/* HandlerNode */

struct HandlerNode : Node {
	Handler *handler;
	Common::SharedPtr<BlockNode> block;

	HandlerNode(uint32 offset, Handler *h)
		: Node(kHandlerNode, offset), handler(h) {
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ExitStmtNode */

struct ExitStmtNode : StmtNode {
	explicit ExitStmtNode(uint32 offset) : StmtNode(kExitStmtNode, offset) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* InverseOpNode */

struct InverseOpNode : ExprNode {
	Common::SharedPtr<Node> operand;

	InverseOpNode(uint32 offset, Common::SharedPtr<Node> o) : ExprNode(kInverseOpNode, offset) {
		operand = Common::move(o);
		operand->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* NotOpNode */

struct NotOpNode : ExprNode {
	Common::SharedPtr<Node> operand;

	NotOpNode(uint32 offset, Common::SharedPtr<Node> o) : ExprNode(kNotOpNode, offset) {
		operand = Common::move(o);
		operand->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* BinaryOpNode */

struct BinaryOpNode : ExprNode {
	OpCode opcode;
	Common::SharedPtr<Node> left;
	Common::SharedPtr<Node> right;

	BinaryOpNode(uint32 offset, OpCode op, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kBinaryOpNode, offset), opcode(op) {
		left = Common::move(a);
		left->parent = this;
		right = Common::move(b);
		right->parent = this;
	}
	virtual unsigned int getPrecedence() const;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ChunkExprNode */

struct ChunkExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> first;
	Common::SharedPtr<Node> last;
	Common::SharedPtr<Node> string;

	ChunkExprNode(uint32 offset, ChunkExprType t, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b, Common::SharedPtr<Node> s)
		: ExprNode(kChunkExprNode, offset), type(t) {
		first = Common::move(a);
		first->parent = this;
		last = Common::move(b);
		last->parent = this;
		string = Common::move(s);
		string->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ChunkHiliteStmtNode */

struct ChunkHiliteStmtNode : StmtNode {
	Common::SharedPtr<Node> chunk;

	ChunkHiliteStmtNode(uint32 offset, Common::SharedPtr<Node> c) : StmtNode(kChunkHiliteStmtNode, offset) {
		chunk = Common::move(c);
		chunk->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ChunkDeleteStmtNode */

struct ChunkDeleteStmtNode : StmtNode {
	Common::SharedPtr<Node> chunk;

	ChunkDeleteStmtNode(uint32 offset, Common::SharedPtr<Node> c) : StmtNode(kChunkDeleteStmtNode, offset) {
		chunk = Common::move(c);
		chunk->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* SpriteIntersectsExprNode */

struct SpriteIntersectsExprNode : ExprNode {
	Common::SharedPtr<Node> firstSprite;
	Common::SharedPtr<Node> secondSprite;

	SpriteIntersectsExprNode(uint32 offset, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kSpriteIntersectsExprNode, offset) {
		firstSprite = Common::move(a);
		firstSprite->parent = this;
		secondSprite = Common::move(b);
		secondSprite->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* SpriteWithinExprNode */

struct SpriteWithinExprNode : ExprNode {
	Common::SharedPtr<Node> firstSprite;
	Common::SharedPtr<Node> secondSprite;

	SpriteWithinExprNode(uint32 offset, Common::SharedPtr<Node> a, Common::SharedPtr<Node> b)
		: ExprNode(kSpriteWithinExprNode, offset) {
		firstSprite = Common::move(a);
		firstSprite->parent = this;
		secondSprite = Common::move(b);
		secondSprite->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* MemberExprNode */

struct MemberExprNode : ExprNode {
	Common::String type;
	Common::SharedPtr<Node> memberID;
	Common::SharedPtr<Node> castID;

	MemberExprNode(uint32 offset, Common::String type_, Common::SharedPtr<Node> memberID_, Common::SharedPtr<Node> castID_)
		: ExprNode(kMemberExprNode, offset), type(type_) {
		this->memberID = Common::move(memberID_);
		this->memberID->parent = this;
		if (castID_) {
			this->castID = Common::move(castID_);
			this->castID->parent = this;
		}
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String varName;

	VarNode(uint32 offset, Common::String v) : ExprNode(kVarNode, offset), varName(v) {}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* AssignmentStmtNode */

struct AssignmentStmtNode : StmtNode {
	Common::SharedPtr<Node> variable;
	Common::SharedPtr<Node> value;
	bool forceVerbose;

	AssignmentStmtNode(uint32 offset, Common::SharedPtr<Node> var, Common::SharedPtr<Node> val, bool forceVerbose_ = false)
		: StmtNode(kAssignmentStmtNode, offset), forceVerbose(forceVerbose_) {
		variable = Common::move(var);
		variable->parent = this;
		value = Common::move(val);
		value->parent = this;
	}

	virtual void accept(NodeVisitor &visitor) const override;
};

/* IfStmtNode */

struct IfStmtNode : StmtNode {
	bool hasElse;
	Common::SharedPtr<Node> condition;
	Common::SharedPtr<BlockNode> block1;
	Common::SharedPtr<BlockNode> block2;

	IfStmtNode(uint32 offset, Common::SharedPtr<Node> c) : StmtNode(kIfStmtNode, offset), hasElse(false) {
		condition = Common::move(c);
		condition->parent = this;
		block1 = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block1->parent = this;
		block2 = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block2->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* RepeatWhileStmtNode */

struct RepeatWhileStmtNode : LoopNode {
	Common::SharedPtr<Node> condition;
	Common::SharedPtr<BlockNode> block;

	RepeatWhileStmtNode(uint32 startIndex_, Common::SharedPtr<Node> c, uint32 offset)
		: LoopNode(kRepeatWhileStmtNode, startIndex_, offset) {
		condition = Common::move(c);
		condition->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* RepeatWithInStmtNode */

struct RepeatWithInStmtNode : LoopNode {
	Common::String varName;
	Common::SharedPtr<Node> list;
	Common::SharedPtr<BlockNode> block;

	RepeatWithInStmtNode(uint32 startIndex_, Common::String v, Common::SharedPtr<Node> l, uint32 offset)
		: LoopNode(kRepeatWithInStmtNode, startIndex_, offset) {
		varName = v;
		list = Common::move(l);
		list->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* RepeatWithToStmtNode */

struct RepeatWithToStmtNode : LoopNode {
	Common::String varName;
	Common::SharedPtr<Node> start;
	bool up;
	Common::SharedPtr<Node> end;
	Common::SharedPtr<BlockNode> block;

	RepeatWithToStmtNode(uint32 startIndex_, Common::String v, Common::SharedPtr<Node> s, bool _up, Common::SharedPtr<Node> e, uint32 offset)
		: LoopNode(kRepeatWithToStmtNode, startIndex_, offset), up(_up) {
		varName = v;
		start = Common::move(s);
		start->parent = this;
		end = Common::move(e);
		end->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* CaseLabelNode */

struct CaseLabelNode : LabelNode {
	Common::SharedPtr<Node> value;
	CaseExpect expect;

	Common::SharedPtr<CaseLabelNode> nextOr;

	Common::SharedPtr<CaseLabelNode> nextLabel;
	Common::SharedPtr<BlockNode> block;

	CaseLabelNode(uint32 offset, Common::SharedPtr<Node> v, CaseExpect e) : LabelNode(kCaseLabelNode, offset), expect(e) {
		value = Common::move(v);
		value->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* OtherwiseNode */

struct OtherwiseNode : LabelNode {
	Common::SharedPtr<BlockNode> block;

	explicit OtherwiseNode(uint32 offset) : LabelNode(kOtherwiseNode, offset) {
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* EndCaseNode */

struct EndCaseNode : LabelNode {
	explicit EndCaseNode(uint32 offset) : LabelNode(kEndCaseNode, offset) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* CaseStmtNode */

struct CaseStmtNode : StmtNode {
	Common::SharedPtr<Node> value;
	Common::SharedPtr<CaseLabelNode> firstLabel;
	Common::SharedPtr<OtherwiseNode> otherwise;

	// for use during translation:
	int32 endPos = -1;
	int32 potentialOtherwisePos = -1;

	CaseStmtNode(uint32 offset, Common::SharedPtr<Node> v) : StmtNode(kCaseStmtNode, offset) {
		value = Common::move(v);
		value->parent = this;
	}
	void addOtherwise(uint32 offset);
	virtual void accept(NodeVisitor &visitor) const override;
};

/* TellStmtNode */

struct TellStmtNode : StmtNode {
	Common::SharedPtr<Node> window;
	Common::SharedPtr<BlockNode> block;

	TellStmtNode(uint32 offset, Common::SharedPtr<Node> w) : StmtNode(kTellStmtNode, offset) {
		window = Common::move(w);
		window->parent = this;
		block = Common::SharedPtr<BlockNode>(new BlockNode(offset));
		block->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* SoundCmdStmtNode */

struct SoundCmdStmtNode : StmtNode {
	Common::String cmd;
	Common::SharedPtr<Node> argList;

	SoundCmdStmtNode(uint32 offset, Common::String c, Common::SharedPtr<Node> a) : StmtNode(kSoundCmdStmtNode, offset) {
		cmd = c;
		argList = Common::move(a);
		argList->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* PlayCmdStmtNode */

struct PlayCmdStmtNode : StmtNode {
	Common::SharedPtr<Node> argList;

	PlayCmdStmtNode(uint32 offset, Common::SharedPtr<Node> a) : StmtNode(kPlayCmdStmtNode, offset) {
		argList = Common::move(a);
		argList->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* CallNode */

struct CallNode : Node {
	Common::String name;
	Common::SharedPtr<Node> argList;

	CallNode(uint32 offset, Common::String n, Common::SharedPtr<Node> a) : Node(kCallNode, offset) {
		name = n;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	bool noParens() const;
	bool isMemberExpr() const;
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ObjCallNode */

struct ObjCallNode : Node {
	Common::String name;
	Common::SharedPtr<Node> argList;

	ObjCallNode(uint32 offset, Common::String n, Common::SharedPtr<Node> a) : Node(kObjCallNode, offset) {
		name = n;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ObjCallV4Node */

struct ObjCallV4Node : Node {
	Common::SharedPtr<Node> obj;
	Common::SharedPtr<Node> argList;

	ObjCallV4Node(uint32 offset, Common::SharedPtr<Node> o, Common::SharedPtr<Node> a) : Node(kObjCallV4Node, offset) {
		obj = o;
		argList = Common::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* TheExprNode */

struct TheExprNode : ExprNode {
	Common::String prop;

	TheExprNode(uint32 offset, Common::String p) : ExprNode(kTheExprNode, offset), prop(p) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* LastStringChunkExprNode */

struct LastStringChunkExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> obj;

	LastStringChunkExprNode(uint32 offset, ChunkExprType t, Common::SharedPtr<Node> o)
		: ExprNode(kLastStringChunkExprNode, offset), type(t) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* StringChunkCountExprNode */

struct StringChunkCountExprNode : ExprNode {
	ChunkExprType type;
	Common::SharedPtr<Node> obj;

	StringChunkCountExprNode(uint32 offset, ChunkExprType t, Common::SharedPtr<Node> o)
		: ExprNode(kStringChunkCountExprNode, offset), type(t) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* MenuPropExprNode */

struct MenuPropExprNode : ExprNode {
	Common::SharedPtr<Node> menuID;
	unsigned int prop;

	MenuPropExprNode(uint32 offset, Common::SharedPtr<Node> m, unsigned int p)
		: ExprNode(kMenuPropExprNode, offset), prop(p) {
		menuID = Common::move(m);
		menuID->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* MenuItemPropExprNode */

struct MenuItemPropExprNode : ExprNode {
	Common::SharedPtr<Node> menuID;
	Common::SharedPtr<Node> itemID;
	unsigned int prop;

	MenuItemPropExprNode(uint32 offset, Common::SharedPtr<Node> m, Common::SharedPtr<Node> i, unsigned int p)
		: ExprNode(kMenuItemPropExprNode, offset), prop(p) {
		menuID = Common::move(m);
		menuID->parent = this;
		itemID = Common::move(i);
		itemID->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* SoundPropExprNode */

struct SoundPropExprNode : ExprNode {
	Common::SharedPtr<Node> soundID;
	unsigned int prop;

	SoundPropExprNode(uint32 offset, Common::SharedPtr<Node> s, unsigned int p)
		: ExprNode(kSoundPropExprNode, offset), prop(p) {
		soundID = Common::move(s);
		soundID->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* SpritePropExprNode */

struct SpritePropExprNode : ExprNode {
	Common::SharedPtr<Node> spriteID;
	unsigned int prop;

	SpritePropExprNode(uint32 offset, Common::SharedPtr<Node> s, unsigned int p)
		: ExprNode(kSpritePropExprNode, offset), prop(p) {
		spriteID = Common::move(s);
		spriteID->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ThePropExprNode */

struct ThePropExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;

	ThePropExprNode(uint32 offset, Common::SharedPtr<Node> o, Common::String p)
		: ExprNode(kThePropExprNode, offset), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ObjPropExprNode */

struct ObjPropExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;

	ObjPropExprNode(uint32 offset, Common::SharedPtr<Node> o, Common::String p)
		: ExprNode(kObjPropExprNode, offset), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ObjBracketExprNode */

struct ObjBracketExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::SharedPtr<Node> prop;

	ObjBracketExprNode(uint32 offset, Common::SharedPtr<Node> o, Common::SharedPtr<Node> p)
		: ExprNode(kObjBracketExprNode, offset) {
		obj = Common::move(o);
		obj->parent = this;
		prop = Common::move(p);
		prop->parent = this;
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ObjPropIndexExprNode */

struct ObjPropIndexExprNode : ExprNode {
	Common::SharedPtr<Node> obj;
	Common::String prop;
	Common::SharedPtr<Node> index;
	Common::SharedPtr<Node> index2;

	ObjPropIndexExprNode(uint32 offset, Common::SharedPtr<Node> o, Common::String p, Common::SharedPtr<Node> i, Common::SharedPtr<Node> i2)
		: ExprNode(kObjPropIndexExprNode, offset), prop(p) {
		obj = Common::move(o);
		obj->parent = this;
		index = Common::move(i);
		index->parent = this;
		if (i2) {
			index2 = Common::move(i2);
			index2->parent = this;
		}
	}
	virtual bool hasSpaces(bool dot) override;
	virtual void accept(NodeVisitor &visitor) const override;
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : StmtNode {
	explicit ExitRepeatStmtNode(uint32 offset) : StmtNode(kExitRepeatStmtNode, offset) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : StmtNode {
	explicit NextRepeatStmtNode(uint32 offset) : StmtNode(kNextRepeatStmtNode, offset) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* PutStmtNode */

struct PutStmtNode : StmtNode {
	PutType type;
	Common::SharedPtr<Node> variable;
	Common::SharedPtr<Node> value;

	PutStmtNode(uint32 offset, PutType t, Common::SharedPtr<Node> var, Common::SharedPtr<Node> val)
		: StmtNode(kPutStmtNode, offset), type(t) {
		variable = Common::move(var);
		variable->parent = this;
		value = Common::move(val);
		value->parent = this;
	}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* WhenStmtNode */

struct WhenStmtNode : StmtNode {
	int event;
	Common::String script;

	WhenStmtNode(uint32 offset, int e, Common::String s)
		: StmtNode(kWhenStmtNode, offset), event(e), script(s) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

/* NewObjNode */

struct NewObjNode : ExprNode {
	Common::String objType;
	Common::SharedPtr<Node> objArgs;

	NewObjNode(uint32 offset, Common::String o, Common::SharedPtr<Node> args) : ExprNode(kNewObjNode, offset), objType(o), objArgs(args) {}
	virtual void accept(NodeVisitor &visitor) const override;
};

class NodeVisitor {
public:
	virtual ~NodeVisitor() {}
	virtual void visit(const HandlerNode &node) { defaultVisit(node); }
	virtual void visit(const ErrorNode &node) { defaultVisit(node); }
	virtual void visit(const CommentNode &node) { defaultVisit(node); }
	virtual void visit(const NewObjNode &node) { defaultVisit(node); }
	virtual void visit(const LiteralNode &node) { defaultVisit(node); }
	virtual void visit(const IfStmtNode &node) { defaultVisit(node); }
	virtual void visit(const EndCaseNode &node) { defaultVisit(node); }
	virtual void visit(const ObjCallNode &node) { defaultVisit(node); }
	virtual void visit(const PutStmtNode &node) { defaultVisit(node); }
	virtual void visit(const TheExprNode &node) { defaultVisit(node); }
	virtual void visit(const BinaryOpNode &node) { defaultVisit(node); }
	virtual void visit(const CaseStmtNode &node) { defaultVisit(node); }
	virtual void visit(const ExitStmtNode &node) { defaultVisit(node); }
	virtual void visit(const TellStmtNode &node) { defaultVisit(node); }
	virtual void visit(const WhenStmtNode &node) { defaultVisit(node); }
	virtual void visit(const CaseLabelNode &node) { defaultVisit(node); }
	virtual void visit(const ChunkExprNode &node) { defaultVisit(node); }
	virtual void visit(const InverseOpNode &node) { defaultVisit(node); }
	virtual void visit(const ObjCallV4Node &node) { defaultVisit(node); }
	virtual void visit(const OtherwiseNode &node) { defaultVisit(node); }
	virtual void visit(const MemberExprNode &node) { defaultVisit(node); }
	virtual void visit(const ObjPropExprNode &node) { defaultVisit(node); }
	virtual void visit(const PlayCmdStmtNode &node) { defaultVisit(node); }
	virtual void visit(const ThePropExprNode &node) { defaultVisit(node); }
	virtual void visit(const MenuPropExprNode &node) { defaultVisit(node); }
	virtual void visit(const SoundCmdStmtNode &node) { defaultVisit(node); }
	virtual void visit(const SoundPropExprNode &node) { defaultVisit(node); }
	virtual void visit(const AssignmentStmtNode &node) { defaultVisit(node); }
	virtual void visit(const ExitRepeatStmtNode &node) { defaultVisit(node); }
	virtual void visit(const NextRepeatStmtNode &node) { defaultVisit(node); }
	virtual void visit(const ObjBracketExprNode &node) { defaultVisit(node); }
	virtual void visit(const SpritePropExprNode &node) { defaultVisit(node); }
	virtual void visit(const ChunkDeleteStmtNode &node) { defaultVisit(node); }
	virtual void visit(const ChunkHiliteStmtNode &node) { defaultVisit(node); }
	virtual void visit(const RepeatWhileStmtNode &node) { defaultVisit(node); }
	virtual void visit(const MenuItemPropExprNode &node) { defaultVisit(node); }
	virtual void visit(const ObjPropIndexExprNode &node) { defaultVisit(node); }
	virtual void visit(const RepeatWithInStmtNode &node) { defaultVisit(node); }
	virtual void visit(const RepeatWithToStmtNode &node) { defaultVisit(node); }
	virtual void visit(const SpriteWithinExprNode &node) { defaultVisit(node); }
	virtual void visit(const LastStringChunkExprNode &node) { defaultVisit(node); }
	virtual void visit(const SpriteIntersectsExprNode &node) { defaultVisit(node); }
	virtual void visit(const StringChunkCountExprNode &node) { defaultVisit(node); }
	virtual void visit(const VarNode &node) { defaultVisit(node); }
	virtual void visit(const CallNode &node) { defaultVisit(node); }
	virtual void visit(const BlockNode &node) { defaultVisit(node); }
	virtual void visit(const NotOpNode &node) { defaultVisit(node); }

	virtual void defaultVisit(const Node &) {}
};

/* AST */

struct AST {
	Common::SharedPtr<HandlerNode> root;
	BlockNode *currentBlock;

	AST(uint32 offset, Handler *handler){
		root = Common::SharedPtr<HandlerNode>(new HandlerNode(offset, handler));
		currentBlock = root->block.get();
	}

	void addStatement(Common::SharedPtr<Node> statement);
	void enterBlock(BlockNode *block);
	void exitBlock();
};

} // namespace LingoDec

#endif // LINGODEC_AST_H
