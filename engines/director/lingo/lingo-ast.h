/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
struct NextRepeatNode;
struct ExitRepeatNode;
struct ExitNode;
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
	kNextRepeatNode,
	kExitRepeatNode,
	kExitNode,
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
	kBinaryOpNode
};

/* NodeVisitor */

class NodeVisitor {
public:
	NodeVisitor() {}
	virtual ~NodeVisitor() {}

	virtual void visitScriptNode(ScriptNode *node) = 0;
	virtual void visitFactoryNode(FactoryNode *node) = 0;
	virtual void visitHandlerNode(HandlerNode *node) = 0;
	virtual void visitCmdNode(CmdNode *node) = 0;
	virtual void visitPutIntoNode(PutIntoNode *node) = 0;
	virtual void visitPutAfterNode(PutAfterNode *node) = 0;
	virtual void visitPutBeforeNode(PutBeforeNode *node) = 0;
	virtual void visitSetNode(SetNode *node) = 0;
	virtual void visitGlobalNode(GlobalNode *node) = 0;
	virtual void visitPropertyNode(PropertyNode *node) = 0;
	virtual void visitInstanceNode(InstanceNode *node) = 0;
	virtual void visitIfStmtNode(IfStmtNode *node) = 0;
	virtual void visitIfElseStmtNode(IfElseStmtNode *node) = 0;
	virtual void visitRepeatWhileNode(RepeatWhileNode *node) = 0;
	virtual void visitRepeatWithToNode(RepeatWithToNode *node) = 0;
	virtual void visitNextRepeatNode(NextRepeatNode *node) = 0;
	virtual void visitExitRepeatNode(ExitRepeatNode *node) = 0;
	virtual void visitExitNode(ExitNode *node) = 0;
	virtual void visitIntNode(IntNode *node) = 0;
	virtual void visitFloatNode(FloatNode *node) = 0;
	virtual void visitSymbolNode(SymbolNode *node) = 0;
	virtual void visitStringNode(StringNode *node) = 0;
	virtual void visitListNode(ListNode *node) = 0;
	virtual void visitPropListNode(PropListNode *node) = 0;
	virtual void visitPropPairNode(PropPairNode *node) = 0;
	virtual void visitFuncNode(FuncNode *node) = 0;
	virtual void visitVarNode(VarNode *node) = 0;
	virtual void visitParensNode(ParensNode *node) = 0;
	virtual void visitUnaryOpNode(UnaryOpNode *node) = 0;
	virtual void visitBinaryOpNode(BinaryOpNode *node) = 0;
};

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;
	bool isLoop;

	Node(NodeType t) : type(t), isExpression(false), isStatement(false), isLoop(false) {}
	virtual ~Node() {}
	virtual void accept(NodeVisitor *visitor) = 0;
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitScriptNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitFactoryNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitHandlerNode(this);
	}
};

/* CmdNode */

struct CmdNode : StmtNode {
	Common::String *name;
	NodeList *args;

	CmdNode(Common::String *nameIn, NodeList *argsIn)
		: StmtNode(kCmdNode), name(nameIn), args(argsIn) {}
	virtual ~CmdNode() {
		delete name;
		deleteList(args);
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitCmdNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPutIntoNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPutAfterNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPutBeforeNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitSetNode(this);
	}
};

/* GlobalNode */

struct GlobalNode : StmtNode {
	IDList *names;

	GlobalNode(IDList *namesIn) : StmtNode(kGlobalNode), names(namesIn) {}
	virtual ~GlobalNode() {
		delete names;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitGlobalNode(this);
	}
};

/* PropertyNode */

struct PropertyNode : StmtNode {
	IDList *names;

	PropertyNode(IDList *namesIn) : StmtNode(kPropertyNode), names(namesIn) {}
	virtual ~PropertyNode() {
		delete names;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPropertyNode(this);
	}
};

/* InstanceNode */

struct InstanceNode : StmtNode {
	IDList *names;

	InstanceNode(IDList *namesIn) : StmtNode(kInstanceNode), names(namesIn) {}
	virtual ~InstanceNode() {
		delete names;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitInstanceNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitIfStmtNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitIfElseStmtNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitRepeatWhileNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitRepeatWithToNode(this);
	}
};

/* NextRepeatNode */

struct NextRepeatNode : StmtNode {
	NextRepeatNode() : StmtNode(kNextRepeatNode) {}
	virtual ~NextRepeatNode() {}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitNextRepeatNode(this);
	}
};

/* ExitRepeatNode */

struct ExitRepeatNode : StmtNode {
	ExitRepeatNode() : StmtNode(kExitRepeatNode) {}
	virtual ~ExitRepeatNode() {}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitExitRepeatNode(this);
	}
};

/* ExitNode */

struct ExitNode : StmtNode {
	ExitNode() : StmtNode(kExitNode) {}
	virtual ~ExitNode() {}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitExitNode(this);
	}
};

/* IntNode */

struct IntNode : ExprNode {
	int val;

	IntNode(int valIn) : ExprNode(kIntNode), val(valIn) {}
	virtual ~IntNode() = default;
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitIntNode(this);
	}
};

/* FloatNode */

struct FloatNode : ExprNode {
	double val;

	FloatNode(double valIn) : ExprNode(kFloatNode), val(valIn) {}
	virtual ~FloatNode() = default;
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitFloatNode(this);
	}
};

/* SymbolNode */

struct SymbolNode : ExprNode {
	Common::String *val;

	SymbolNode(Common::String *valIn) : ExprNode(kSymbolNode), val(valIn) {}
	virtual ~SymbolNode() {
		delete val;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitSymbolNode(this);
	}
};

/* StringNode */

struct StringNode : ExprNode {
	Common::String *val;

	StringNode(Common::String *valIn) : ExprNode(kStringNode), val(valIn) {}
	virtual ~StringNode() {
		delete val;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitStringNode(this);
	}
};

/* ListNode */

struct ListNode : ExprNode {
	NodeList *items;

	ListNode(NodeList *itemsIn) : ExprNode(kListNode), items(itemsIn) {}
	virtual ~ListNode() {
		deleteList(items);
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitListNode(this);
	}
};

/* PropListNode */

struct PropListNode : ExprNode {
	NodeList *items;

	PropListNode(NodeList *itemsIn) : ExprNode(kListNode), items(itemsIn) {}
	virtual ~PropListNode() {
		deleteList(items);
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPropListNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitPropPairNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitFuncNode(this);
	}
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String *name;

	VarNode(Common::String *nameIn) : ExprNode(kVarNode), name(nameIn) {}
	virtual ~VarNode() {
		delete name;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitVarNode(this);
	}
};

/* ParensNode */

struct ParensNode : ExprNode {
	Node *expr;

	ParensNode(Node *exprIn) : ExprNode(kParensNode), expr(exprIn) {}
	virtual ~ParensNode() {
		delete expr;
	}
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitParensNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitUnaryOpNode(this);
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
	virtual void accept(NodeVisitor *visitor) {
		visitor->visitBinaryOpNode(this);
	}
};

} // End of namespace Director

#endif
