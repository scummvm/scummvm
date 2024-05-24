/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/util.h"
#include "./ast.h"
#include "./handler.h"
#include "./names.h"
#include "./script.h"

namespace LingoDec {

/* Datum */

int Datum::toInt() {
	switch (type) {
	case kDatumInt:
		return i;
	case kDatumFloat:
		return f;
	default:
		break;
	}
	return 0;
}

/* AST */

void AST::addStatement(Common::SharedPtr<Node> statement) {
	currentBlock->addChild(Common::move(statement));
}

void AST::enterBlock(BlockNode *block) {
	currentBlock = block;
}

void AST::exitBlock() {
	auto ancestorStatement = currentBlock->ancestorStatement();
	if (!ancestorStatement) {
		currentBlock = nullptr;
		return;
	}

	ancestorStatement->_endOffset = currentBlock->_endOffset;

	auto block = ancestorStatement->parent;
	if (!block || block->type != kBlockNode) {
		currentBlock = nullptr;
		return;
	}

	currentBlock = static_cast<BlockNode *>(block);
}

/* Node */

Common::SharedPtr<Datum> Node::getValue() {
	return Common::SharedPtr<Datum>(new Datum());
}

Node *Node::ancestorStatement() {
	Node *ancestor = parent;
	while (ancestor && !ancestor->isStatement) {
		ancestor = ancestor->parent;
	}
	return ancestor;
}

LoopNode *Node::ancestorLoop() {
	Node *ancestor = parent;
	while (ancestor && !ancestor->isLoop) {
		ancestor = ancestor->parent;
	}
	return static_cast<LoopNode *>(ancestor);
}

bool Node::hasSpaces(bool) {
	return true;
}

/* ErrorNode */

bool ErrorNode::hasSpaces(bool) {
	return false;
}

/* LiteralNode */

Common::SharedPtr<Datum> LiteralNode::getValue() {
	return value;
}

bool LiteralNode::hasSpaces(bool) {
	return false;
}

/* BlockNode */

void BlockNode::addChild(Common::SharedPtr<Node> child) {
	child->parent = this;
	children.push_back(Common::move(child));
}

/* BinaryOpNode */

unsigned int BinaryOpNode::getPrecedence() const {
	switch (opcode) {
	case kOpMul:
	case kOpDiv:
	case kOpMod:
		return 1;
	case kOpAdd:
	case kOpSub:
		return 2;
	case kOpLt:
	case kOpLtEq:
	case kOpNtEq:
	case kOpEq:
	case kOpGt:
	case kOpGtEq:
		return 3;
	case kOpAnd:
		return 4;
	case kOpOr:
		return 5;
	default:
		break;
	}
	return 0;
}

/* MemberExprNode */

bool MemberExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* VarNode */

bool VarNode::hasSpaces(bool) {
	return false;
}

/* CaseStmtNode */

void CaseStmtNode::addOtherwise(uint32 offset) {
	otherwise = Common::SharedPtr<OtherwiseNode>(new OtherwiseNode(offset));
	otherwise->parent = this;
	otherwise->block->endPos = endPos;
}

/* CallNode */

bool CallNode::noParens() const {
	if (isStatement) {
		// TODO: Make a complete list of commonly paren-less commands
		if (name == "put")
			return true;
		if (name == "return")
			return true;
	}

	return false;
}

bool CallNode::isMemberExpr() const {
	if (isExpression) {
		size_t nargs = argList->getValue()->l.size();
		if (name == "cast" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "member" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "script" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "castLib" && nargs == 1)
			return true;
		if (name == "window" && nargs == 1)
			return true;
	}

	return false;
}

bool CallNode::hasSpaces(bool dot) {
	if (!dot && isMemberExpr())
		return true;

	if (noParens())
		return true;

	return false;
}

/* ObjCallNode */

bool ObjCallNode::hasSpaces(bool) {
	return false;
}

/* ObjCallV4Node */

bool ObjCallV4Node::hasSpaces(bool) {
	return false;
}

/* ObjPropExprNode */

bool ObjPropExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* ObjBracketExprNode */

bool ObjBracketExprNode::hasSpaces(bool) {
	return false;
}

/* ObjPropIndexExprNode */

bool ObjPropIndexExprNode::hasSpaces(bool) {
	return false;
}

void ErrorNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void CommentNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void LiteralNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void IfStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void NewObjNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void EndCaseNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void HandlerNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ObjCallNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void PutStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void TheExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void BinaryOpNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void CaseStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ExitStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void TellStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void WhenStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void CaseLabelNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ChunkExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void InverseOpNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ObjCallV4Node::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void OtherwiseNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void MemberExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ObjPropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void PlayCmdStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ThePropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void MenuPropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void SoundCmdStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void SoundPropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void AssignmentStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ExitRepeatStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void NextRepeatStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ObjBracketExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void SpritePropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ChunkDeleteStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ChunkHiliteStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void RepeatWhileStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void MenuItemPropExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void ObjPropIndexExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void RepeatWithInStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void RepeatWithToStmtNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void SpriteWithinExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void LastStringChunkExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void SpriteIntersectsExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void StringChunkCountExprNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void VarNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void CallNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void BlockNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }
void NotOpNode::accept(NodeVisitor &visitor) const { visitor.visit(*this); }

} // namespace LingoDec
