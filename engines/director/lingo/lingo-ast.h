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
	kGlobalNode,
	kPropertyNode,
	kInstanceNode,
	kIntNode,
	kFloatNode,
	kSymbolNode,
	kStringNode,
	kFuncNode,
	kVarNode,
	kParensNode,
	kUnaryOpNode,
	kBinaryOpNode
};

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;

	Node(NodeType t) : type(t), isExpression(false), isStatement(false) {}
	virtual ~Node() = default;
	virtual void compile() {}
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

/* ScriptNode */

struct ScriptNode : Node {
	NodeList *nodes;

	ScriptNode(NodeList *nodesIn): Node(kScriptNode), nodes(nodesIn) {}
	virtual ~ScriptNode() {
		deleteList(nodes);
	}
	virtual void compile();
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
	virtual void compile();
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
	virtual void compile();
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
	virtual void compile();
};

/* GlobalNode */

struct GlobalNode : StmtNode {
	IDList *names;

	GlobalNode(IDList *namesIn) : StmtNode(kGlobalNode), names(namesIn) {}
	virtual ~GlobalNode() {
		delete names;
	}
	virtual void compile();
};

/* PropertyNode */

struct PropertyNode : StmtNode {
	IDList *names;

	PropertyNode(IDList *namesIn) : StmtNode(kPropertyNode), names(namesIn) {}
	virtual ~PropertyNode() {
		delete names;
	}
	virtual void compile();
};

/* InstanceNode */

struct InstanceNode : StmtNode {
	IDList *names;

	InstanceNode(IDList *namesIn) : StmtNode(kInstanceNode), names(namesIn) {}
	virtual ~InstanceNode() {
		delete names;
	}
	virtual void compile();
};

/* IntNode */

struct IntNode : ExprNode {
	int val;

	IntNode(int valIn) : ExprNode(kIntNode), val(valIn) {}
	virtual ~IntNode() = default;
	virtual void compile();
};

/* FloatNode */

struct FloatNode : ExprNode {
	double val;

	FloatNode(double valIn) : ExprNode(kFloatNode), val(valIn) {}
	virtual ~FloatNode() = default;
	virtual void compile();
};

/* SymbolNode */

struct SymbolNode : ExprNode {
	Common::String *val;

	SymbolNode(Common::String *valIn) : ExprNode(kSymbolNode), val(valIn) {}
	virtual ~SymbolNode() {
		delete val;
	}
	virtual void compile();
};

/* StringNode */

struct StringNode : ExprNode {
	Common::String *val;

	StringNode(Common::String *valIn) : ExprNode(kStringNode), val(valIn) {}
	virtual ~StringNode() {
		delete val;
	}
	virtual void compile();
};

/* FuncNode */

struct FuncNode : StmtNode {
	Common::String *name;
	NodeList *args;

	FuncNode(Common::String *nameIn, NodeList *argsIn)
		: StmtNode(kFuncNode), name(nameIn), args(argsIn) {}
	virtual ~FuncNode() {
		delete name;
		deleteList(args);
	}
	virtual void compile();
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String *name;

	VarNode(Common::String *nameIn) : ExprNode(kVarNode), name(nameIn) {}
	virtual ~VarNode() {
		delete name;
	}
	virtual void compile();
};

/* ParensNode */

struct ParensNode : ExprNode {
	Node *expr;

	ParensNode(Node *exprIn) : ExprNode(kParensNode), expr(exprIn) {}
	virtual ~ParensNode() {
		delete expr;
	}
	virtual void compile();
};

/* UnaryOpNode */

struct UnaryOpNode : ExprNode {
	inst op;
	Node *arg;

	UnaryOpNode(inst opIn, Node *argIn) : ExprNode(kUnaryOpNode), op(opIn), arg(argIn) {}
	virtual ~UnaryOpNode() {
		delete arg;
	}
	virtual void compile();
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
	virtual void compile();
};

} // End of namespace Director

#endif
