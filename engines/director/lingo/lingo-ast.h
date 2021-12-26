/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIRECTOR_LINGO_LINGO_AST_H
#define DIRECTOR_LINGO_LINGO_AST_H

namespace Director {

struct Node;
struct ScriptNode;
struct FactoryNode;
struct HandlerNode;
struct CmdNode;
struct PutIntoNode;
struct PutAfterNode;
struct PutBeforeNode;
struct SetNode;
struct GlobalNode;
struct PropertyNode;
struct InstanceNode;
struct IfStmtNode;
struct IfElseStmtNode;
struct RepeatWhileNode;
struct RepeatWithToNode;
struct RepeatWithInNode;
struct NextRepeatNode;
struct ExitRepeatNode;
struct ExitNode;
struct TellNode;
struct WhenNode;
struct DeleteNode;
struct HiliteNode;
struct AssertErrorNode;
struct IntNode;
struct FloatNode;
struct SymbolNode;
struct StringNode;
struct ListNode;
struct PropListNode;
struct PropPairNode;
struct FuncNode;
struct VarNode;
struct ParensNode;
struct UnaryOpNode;
struct BinaryOpNode;
struct FrameNode;
struct MovieNode;
struct IntersectsNode;
struct WithinNode;
struct TheNode;
struct TheOfNode;
struct TheNumberOfNode;
struct TheLastNode;
struct TheDateTimeNode;
struct MenuNode;
struct MenuItemNode;
struct SoundNode;
struct SpriteNode;
struct ChunkExprNode;

typedef Common::Array<Node *> NodeList;
typedef Common::Array<Common::String *> IDList;

typedef void (*inst)(void);

template <typename T>
void deleteList(Common::Array<T> *list) {
	for (uint i = 0; i < list->size(); i++) {
		delete (*list)[i];
	}
	delete list;
}

enum NodeType {
	kScriptNode,
	kFactoryNode,
	kHandlerNode,
	kCmdNode,
	kPutIntoNode,
	kPutAfterNode,
	kPutBeforeNode,
	kSetNode,
	kGlobalNode,
	kPropertyNode,
	kInstanceNode,
	kIfStmtNode,
	kIfElseStmtNode,
	kRepeatWhileNode,
	kRepeatWithToNode,
	kRepeatWithInNode,
	kNextRepeatNode,
	kExitRepeatNode,
	kExitNode,
	kTellNode,
	kWhenNode,
	kDeleteNode,
	kHiliteNode,
	kAssertErrorNode,
	kIntNode,
	kFloatNode,
	kSymbolNode,
	kStringNode,
	kListNode,
	kPropListNode,
	kPropPairNode,
	kFuncNode,
	kVarNode,
	kParensNode,
	kUnaryOpNode,
	kBinaryOpNode,
	kFrameNode,
	kMovieNode,
	kIntersectsNode,
	kWithinNode,
	kTheNode,
	kTheOfNode,
	kTheNumberOfNode,
	kTheLastNode,
	kTheDateTimeNode,
	kMenuNode,
	kMenuItemNode,
	kSoundNode,
	kSpriteNode,
	kChunkExprNode
};

enum NumberOfType {
	kNumberOfChars,
	kNumberOfWords,
	kNumberOfItems,
	kNumberOfLines,
	kNumberOfMenuItems
};

/* NodeVisitor */

class NodeVisitor {
public:
	NodeVisitor() {}
	virtual ~NodeVisitor() {}

	virtual bool visitScriptNode(ScriptNode *node) = 0;
	virtual bool visitFactoryNode(FactoryNode *node) = 0;
	virtual bool visitHandlerNode(HandlerNode *node) = 0;
	virtual bool visitCmdNode(CmdNode *node) = 0;
	virtual bool visitPutIntoNode(PutIntoNode *node) = 0;
	virtual bool visitPutAfterNode(PutAfterNode *node) = 0;
	virtual bool visitPutBeforeNode(PutBeforeNode *node) = 0;
	virtual bool visitSetNode(SetNode *node) = 0;
	virtual bool visitGlobalNode(GlobalNode *node) = 0;
	virtual bool visitPropertyNode(PropertyNode *node) = 0;
	virtual bool visitInstanceNode(InstanceNode *node) = 0;
	virtual bool visitIfStmtNode(IfStmtNode *node) = 0;
	virtual bool visitIfElseStmtNode(IfElseStmtNode *node) = 0;
	virtual bool visitRepeatWhileNode(RepeatWhileNode *node) = 0;
	virtual bool visitRepeatWithToNode(RepeatWithToNode *node) = 0;
	virtual bool visitRepeatWithInNode(RepeatWithInNode *node) = 0;
	virtual bool visitNextRepeatNode(NextRepeatNode *node) = 0;
	virtual bool visitExitRepeatNode(ExitRepeatNode *node) = 0;
	virtual bool visitExitNode(ExitNode *node) = 0;
	virtual bool visitTellNode(TellNode *node) = 0;
	virtual bool visitWhenNode(WhenNode *node) = 0;
	virtual bool visitDeleteNode(DeleteNode *node) = 0;
	virtual bool visitHiliteNode(HiliteNode *node) = 0;
	virtual bool visitAssertErrorNode(AssertErrorNode *node) = 0;
	virtual bool visitIntNode(IntNode *node) = 0;
	virtual bool visitFloatNode(FloatNode *node) = 0;
	virtual bool visitSymbolNode(SymbolNode *node) = 0;
	virtual bool visitStringNode(StringNode *node) = 0;
	virtual bool visitListNode(ListNode *node) = 0;
	virtual bool visitPropListNode(PropListNode *node) = 0;
	virtual bool visitPropPairNode(PropPairNode *node) = 0;
	virtual bool visitFuncNode(FuncNode *node) = 0;
	virtual bool visitVarNode(VarNode *node) = 0;
	virtual bool visitParensNode(ParensNode *node) = 0;
	virtual bool visitUnaryOpNode(UnaryOpNode *node) = 0;
	virtual bool visitBinaryOpNode(BinaryOpNode *node) = 0;
	virtual bool visitFrameNode(FrameNode *node) = 0;
	virtual bool visitMovieNode(MovieNode *node) = 0;
	virtual bool visitIntersectsNode(IntersectsNode *node) = 0;
	virtual bool visitWithinNode(WithinNode *node) = 0;
	virtual bool visitTheNode(TheNode *node) = 0;
	virtual bool visitTheOfNode(TheOfNode *node) = 0;
	virtual bool visitTheNumberOfNode(TheNumberOfNode *node) = 0;
	virtual bool visitTheLastNode(TheLastNode *node) = 0;
	virtual bool visitTheDateTimeNode(TheDateTimeNode *node) = 0;
	virtual bool visitMenuNode(MenuNode *node) = 0;
	virtual bool visitMenuItemNode(MenuItemNode *node) = 0;
	virtual bool visitSoundNode(SoundNode *node) = 0;
	virtual bool visitSpriteNode(SpriteNode *node) = 0;
	virtual bool visitChunkExprNode(ChunkExprNode *node) = 0;
};

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;
	bool isLoop;

	Node(NodeType t) : type(t), isExpression(false), isStatement(false), isLoop(false) {}
	virtual ~Node() {}
	virtual bool accept(NodeVisitor *visitor) = 0;
};

/* ExprNode */

struct ExprNode : Node {
	ExprNode(NodeType t) : Node(t) {
		isExpression = true;
	}
	virtual ~ExprNode() {}
};

/* StmtNode */

struct StmtNode : Node {
	StmtNode(NodeType t) : Node(t) {
		isStatement = true;
	}
	virtual ~StmtNode() {}
};

/* LoopNode */

struct LoopNode : StmtNode {
	Common::Array<uint> nextRepeats;
	Common::Array<uint> exitRepeats;

	LoopNode(NodeType t) : StmtNode(t) {
		isLoop = true;
	}
	virtual ~LoopNode() {}
};

/* ScriptNode */

struct ScriptNode : Node {
	NodeList *children;

	ScriptNode(NodeList *childrenIn): Node(kScriptNode), children(childrenIn) {}
	virtual ~ScriptNode() {
		deleteList(children);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitScriptNode(this);
	}
};

/* FactoryNode */

struct FactoryNode : Node {
	Common::String *name;
	NodeList *methods;

	FactoryNode(Common::String *nameIn, NodeList *methodsIn)
		: Node(kFactoryNode), name(nameIn), methods(methodsIn) {}
	virtual ~FactoryNode() {
		delete name;
		deleteList(methods);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitFactoryNode(this);
	}
};

/* HandlerNode */

struct HandlerNode : Node {
	Common::String *name;
	IDList *args;
	NodeList *stmts;

	HandlerNode(Common::String *nameIn, IDList *argsIn, NodeList *stmtsIn)
		: Node(kHandlerNode), name(nameIn), args(argsIn), stmts(stmtsIn) {}
	virtual ~HandlerNode() {
		delete name;
		deleteList(args);
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitHandlerNode(this);
	}
};

/* CmdNode */

struct CmdNode : StmtNode {
	Common::String *name;
	NodeList *args;
	uint lineNumber;

	CmdNode(Common::String *nameIn, NodeList *argsIn, uint lineNumberIn)
		: StmtNode(kCmdNode), name(nameIn), args(argsIn), lineNumber(lineNumberIn) {}
	virtual ~CmdNode() {
		delete name;
		deleteList(args);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitCmdNode(this);
	}
};

/* PutIntoNode */

struct PutIntoNode : StmtNode {
	Node *val;
	Node *var;

	PutIntoNode(Node *valIn, Node *varIn)
		: StmtNode(kPutIntoNode), val(valIn), var(varIn) {}
	virtual ~PutIntoNode() {
		delete val;
		delete var;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPutIntoNode(this);
	}
};

/* PutAfterNode */

struct PutAfterNode : StmtNode {
	Node *val;
	Node *var;

	PutAfterNode(Node *valIn, Node *varIn)
		: StmtNode(kPutAfterNode), val(valIn), var(varIn) {}
	virtual ~PutAfterNode() {
		delete val;
		delete var;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPutAfterNode(this);
	}
};

/* PutBeforeNode */

struct PutBeforeNode : StmtNode {
	Node *val;
	Node *var;

	PutBeforeNode(Node *valIn, Node *varIn)
		: StmtNode(kPutBeforeNode), val(valIn), var(varIn) {}
	virtual ~PutBeforeNode() {
		delete val;
		delete var;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPutBeforeNode(this);
	}
};

/* SetNode */

struct SetNode : StmtNode {
	Node *var;
	Node *val;

	SetNode(Node *varIn, Node *valIn)
		: StmtNode(kSetNode), var(varIn), val(valIn) {}
	virtual ~SetNode() {
		delete var;
		delete val;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitSetNode(this);
	}
};

/* GlobalNode */

struct GlobalNode : StmtNode {
	IDList *names;

	GlobalNode(IDList *namesIn) : StmtNode(kGlobalNode), names(namesIn) {}
	virtual ~GlobalNode() {
		deleteList(names);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitGlobalNode(this);
	}
};

/* PropertyNode */

struct PropertyNode : StmtNode {
	IDList *names;

	PropertyNode(IDList *namesIn) : StmtNode(kPropertyNode), names(namesIn) {}
	virtual ~PropertyNode() {
		deleteList(names);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPropertyNode(this);
	}
};

/* InstanceNode */

struct InstanceNode : StmtNode {
	IDList *names;

	InstanceNode(IDList *namesIn) : StmtNode(kInstanceNode), names(namesIn) {}
	virtual ~InstanceNode() {
		deleteList(names);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitInstanceNode(this);
	}
};

/* IfStmtNode */

struct IfStmtNode : StmtNode {
	Node *cond;
	NodeList *stmts;

	IfStmtNode(Node *condIn, NodeList *stmtsIn)
		: StmtNode(kIfStmtNode), cond(condIn), stmts(stmtsIn) {}
	virtual ~IfStmtNode() {
		delete cond;
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitIfStmtNode(this);
	}
};

/* IfElseStmtNode */

struct IfElseStmtNode : StmtNode {
	Node *cond;
	NodeList *stmts1;
	NodeList *stmts2;

	IfElseStmtNode(Node *condIn, NodeList *stmts1In, NodeList *stmts2In)
		: StmtNode(kIfElseStmtNode), cond(condIn), stmts1(stmts1In), stmts2(stmts2In) {}
	virtual ~IfElseStmtNode() {
		delete cond;
		deleteList(stmts1);
		deleteList(stmts2);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitIfElseStmtNode(this);
	}
};

/* RepeatWhileNode */

struct RepeatWhileNode : LoopNode {
	Node *cond;
	NodeList *stmts;

	RepeatWhileNode(Node *condIn, NodeList *stmtsIn)
		: LoopNode(kRepeatWhileNode), cond(condIn), stmts(stmtsIn) {}
	virtual ~RepeatWhileNode() {
		delete cond;
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitRepeatWhileNode(this);
	}
};

/* RepeatWithToNode */

struct RepeatWithToNode : LoopNode {
	Common::String *var;
	Node *start;
	bool down;
	Node *end;
	NodeList *stmts;

	RepeatWithToNode(Common::String *varIn, Node *startIn, bool downIn, Node *endIn, NodeList *stmtsIn)
		: LoopNode(kRepeatWithToNode), var(varIn), start(startIn), down(downIn), end(endIn), stmts(stmtsIn) {}
	virtual ~RepeatWithToNode() {
		delete var;
		delete start;
		delete end;
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitRepeatWithToNode(this);
	}
};

/* RepeatWithInNode */

struct RepeatWithInNode : LoopNode {
	Common::String *var;
	Node *list;
	NodeList *stmts;

	RepeatWithInNode(Common::String *varIn, Node *listIn, NodeList *stmtsIn)
		: LoopNode(kRepeatWithInNode), var(varIn), list(listIn), stmts(stmtsIn) {}
	virtual ~RepeatWithInNode() {
		delete var;
		delete list;
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitRepeatWithInNode(this);
	}
};

/* NextRepeatNode */

struct NextRepeatNode : StmtNode {
	NextRepeatNode() : StmtNode(kNextRepeatNode) {}
	virtual ~NextRepeatNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitNextRepeatNode(this);
	}
};

/* ExitRepeatNode */

struct ExitRepeatNode : StmtNode {
	ExitRepeatNode() : StmtNode(kExitRepeatNode) {}
	virtual ~ExitRepeatNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitExitRepeatNode(this);
	}
};

/* ExitNode */

struct ExitNode : StmtNode {
	ExitNode() : StmtNode(kExitNode) {}
	virtual ~ExitNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitExitNode(this);
	}
};

/* TellNode */

struct TellNode : StmtNode {
	Node *target;
	NodeList *stmts;

	TellNode(Node *targetIn, NodeList *stmtsIn)
		: StmtNode(kTellNode), target(targetIn), stmts(stmtsIn) {}
	virtual ~TellNode() {
		delete target;
		deleteList(stmts);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTellNode(this);
	}
};

/* WhenNode */

struct WhenNode : StmtNode {
	Common::String *event;
	Common::String *code;

	WhenNode(Common::String *eventIn, Common::String *codeIn)
		: StmtNode(kWhenNode), event(eventIn), code(codeIn) {}
	virtual ~WhenNode() {
		delete event;
		delete code;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitWhenNode(this);
	}
};

/* DeleteNode */

struct DeleteNode : StmtNode {
	Node *chunk;

	DeleteNode(Node *chunkIn) : StmtNode(kDeleteNode), chunk(chunkIn) {}
	virtual ~DeleteNode() {
		delete chunk;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitDeleteNode(this);
	}
};

/* HiliteNode */

struct HiliteNode : StmtNode {
	Node *chunk;

	HiliteNode(Node *chunkIn) : StmtNode(kHiliteNode), chunk(chunkIn) {}
	virtual ~HiliteNode() {
		delete chunk;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitHiliteNode(this);
	}
};

/* AssertErrorNode */

struct AssertErrorNode : StmtNode {
	Node *stmt;

	AssertErrorNode(Node *stmtIn) : StmtNode(kAssertErrorNode), stmt(stmtIn) {}
	virtual ~AssertErrorNode() {
		delete stmt;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitAssertErrorNode(this);
	}
};

/* IntNode */

struct IntNode : ExprNode {
	int val;

	IntNode(int valIn) : ExprNode(kIntNode), val(valIn) {}
	virtual ~IntNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitIntNode(this);
	}
};

/* FloatNode */

struct FloatNode : ExprNode {
	double val;

	FloatNode(double valIn) : ExprNode(kFloatNode), val(valIn) {}
	virtual ~FloatNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitFloatNode(this);
	}
};

/* SymbolNode */

struct SymbolNode : ExprNode {
	Common::String *val;

	SymbolNode(Common::String *valIn) : ExprNode(kSymbolNode), val(valIn) {}
	virtual ~SymbolNode() {
		delete val;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitSymbolNode(this);
	}
};

/* StringNode */

struct StringNode : ExprNode {
	Common::String *val;

	StringNode(Common::String *valIn) : ExprNode(kStringNode), val(valIn) {}
	virtual ~StringNode() {
		delete val;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitStringNode(this);
	}
};

/* ListNode */

struct ListNode : ExprNode {
	NodeList *items;

	ListNode(NodeList *itemsIn) : ExprNode(kListNode), items(itemsIn) {}
	virtual ~ListNode() {
		deleteList(items);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitListNode(this);
	}
};

/* PropListNode */

struct PropListNode : ExprNode {
	NodeList *items;

	PropListNode(NodeList *itemsIn) : ExprNode(kListNode), items(itemsIn) {}
	virtual ~PropListNode() {
		deleteList(items);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPropListNode(this);
	}
};

/* PropPairNode */

struct PropPairNode : ExprNode {
	Node *key;
	Node *val;

	PropPairNode(Node *keyIn, Node *valIn)
		: ExprNode(kPropPairNode), key(keyIn), val(valIn) {}
	virtual ~PropPairNode() {
		delete key;
		delete val;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitPropPairNode(this);
	}
};

/* FuncNode */

struct FuncNode : ExprNode {
	Common::String *name;
	NodeList *args;

	FuncNode(Common::String *nameIn, NodeList *argsIn)
		: ExprNode(kFuncNode), name(nameIn), args(argsIn) {}
	virtual ~FuncNode() {
		delete name;
		deleteList(args);
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitFuncNode(this);
	}
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String *name;

	VarNode(Common::String *nameIn) : ExprNode(kVarNode), name(nameIn) {}
	virtual ~VarNode() {
		delete name;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitVarNode(this);
	}
};

/* ParensNode */

struct ParensNode : ExprNode {
	Node *expr;

	ParensNode(Node *exprIn) : ExprNode(kParensNode), expr(exprIn) {}
	virtual ~ParensNode() {
		delete expr;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitParensNode(this);
	}
};

/* UnaryOpNode */

struct UnaryOpNode : ExprNode {
	inst op;
	Node *arg;

	UnaryOpNode(inst opIn, Node *argIn) : ExprNode(kUnaryOpNode), op(opIn), arg(argIn) {}
	virtual ~UnaryOpNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitUnaryOpNode(this);
	}
};

/* BinaryOpNode */

struct BinaryOpNode : ExprNode {
	inst op;
	Node *a;
	Node *b;

	BinaryOpNode(inst opIn, Node *aIn, Node *bIn) : ExprNode(kBinaryOpNode), op(opIn), a(aIn), b(bIn) {}
	virtual ~BinaryOpNode() {
		delete a;
		delete b;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitBinaryOpNode(this);
	}
};

/* FrameNode */

struct FrameNode : ExprNode {
	Node *arg;

	FrameNode(Node *argIn) : ExprNode(kFrameNode), arg(argIn) {}
	virtual ~FrameNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitFrameNode(this);
	}
};

/* MovieNode */

struct MovieNode : ExprNode {
	Node *arg;

	MovieNode(Node *argIn) : ExprNode(kMovieNode), arg(argIn) {}
	virtual ~MovieNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitMovieNode(this);
	}
};

/* IntersectsNode */

struct IntersectsNode : ExprNode {
	Node *sprite1;
	Node *sprite2;

	IntersectsNode(Node *sprite1In, Node *sprite2In)
		: ExprNode(kIntersectsNode), sprite1(sprite1In), sprite2(sprite2In) {}
	virtual ~IntersectsNode() {
		delete sprite1;
		delete sprite2;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitIntersectsNode(this);
	}
};

/* WithinNode */

struct WithinNode : ExprNode {
	Node *sprite1;
	Node *sprite2;

	WithinNode(Node *sprite1In, Node *sprite2In)
		: ExprNode(kWithinNode), sprite1(sprite1In), sprite2(sprite2In) {}
	virtual ~WithinNode() {
		delete sprite1;
		delete sprite2;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitWithinNode(this);
	}
};

/* TheNode */

struct TheNode : ExprNode {
	Common::String *prop;

	TheNode(Common::String *propIn) : ExprNode(kTheNode), prop(propIn) {}
	virtual ~TheNode() {
		delete prop;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTheNode(this);
	}
};

/* TheOfNode */

struct TheOfNode : ExprNode {
	Common::String *prop;
	Node *obj;

	TheOfNode(Common::String *propIn, Node *objIn)
		: ExprNode(kTheOfNode), prop(propIn), obj(objIn) {}
	virtual ~TheOfNode() {
		delete prop;
		delete obj;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTheOfNode(this);
	}
};

/* TheNumberOfNode */

struct TheNumberOfNode : ExprNode {
	NumberOfType type;
	Node *arg;

	TheNumberOfNode(NumberOfType typeIn, Node *argIn)
		: ExprNode(kTheNumberOfNode), type(typeIn), arg(argIn) {}
	virtual ~TheNumberOfNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTheNumberOfNode(this);
	}
};

/* TheLastNode */

struct TheLastNode : ExprNode {
	ChunkType type;
	Node *arg;

	TheLastNode(ChunkType typeIn, Node *argIn)
		: ExprNode(kTheLastNode), type(typeIn), arg(argIn) {}
	virtual ~TheLastNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTheLastNode(this);
	}
};

/* TheDateTimeNode */

struct TheDateTimeNode : ExprNode {
	int field;
	int entity;

	TheDateTimeNode(int fieldIn, int entityIn)
		: ExprNode(kTheDateTimeNode), field(fieldIn), entity(entityIn) {}
	virtual ~TheDateTimeNode() {}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitTheDateTimeNode(this);
	}
};

/* MenuNode */

struct MenuNode : ExprNode {
	Node *arg;

	MenuNode(Node *argIn) : ExprNode(kMenuNode), arg(argIn) {}
	virtual ~MenuNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitMenuNode(this);
	}
};

/* MenuItemNode */

struct MenuItemNode : ExprNode {
	Node *arg1;
	Node *arg2;

	MenuItemNode(Node *arg1In, Node *arg2In)
		: ExprNode(kMenuItemNode), arg1(arg1In), arg2(arg2In) {}
	virtual ~MenuItemNode() {
		delete arg1;
		delete arg2;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitMenuItemNode(this);
	}
};

/* SoundNode */

struct SoundNode : ExprNode {
	Node *arg;

	SoundNode(Node *argIn) : ExprNode(kSoundNode), arg(argIn) {}
	virtual ~SoundNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitSoundNode(this);
	}
};

/* SpriteNode */

struct SpriteNode : ExprNode {
	Node *arg;

	SpriteNode(Node *argIn) : ExprNode(kSpriteNode), arg(argIn) {}
	virtual ~SpriteNode() {
		delete arg;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitSpriteNode(this);
	}
};

/* ChunkExprNode */

struct ChunkExprNode : ExprNode {
	ChunkType type;
	Node *start;
	Node *end;
	Node *src;

	ChunkExprNode(ChunkType typeIn, Node *startIn, Node *endIn, Node *srcIn)
		: ExprNode(kChunkExprNode), type(typeIn), start(startIn), end(endIn), src(srcIn) {}
	virtual ~ChunkExprNode() {
		delete start;
		delete end;
		delete src;
	}
	virtual bool accept(NodeVisitor *visitor) {
		return visitor->visitChunkExprNode(this);
	}
};

} // End of namespace Director

#endif
