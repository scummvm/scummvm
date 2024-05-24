/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/ptr.h"
#include "./ast.h"
#include "./handler.h"
#include "./names.h"
#include "./script.h"
#include "./codewritervisitor.h"

namespace LingoDec {

void CodeWriterVisitor::visit(const HandlerNode &node) {
	if (node.handler->isGenericEvent) {
		node.block->accept(*this);
	} else {
		Script *script = node.handler->script;
		bool isMethod = script->isFactory();
		if (isMethod) {
			write("method ");
		} else {
			write("on ");
		}
		write(node.handler->name);
		if (node.handler->argumentNames.size() > 0) {
			write(" ");
			for (size_t i = 0; i < node.handler->argumentNames.size(); i++) {
				if (i > 0)
					write(", ");
				write(node.handler->argumentNames[i]);
			}
		}
		writeLine();
		indent();
		if (isMethod && script->propertyNames.size() > 0 && node.handler == &script->handlers[0]) {
			write("instance ");
			for (size_t i = 0; i < script->propertyNames.size(); i++) {
				if (i > 0)
					write(", ");
				write(script->propertyNames[i]);
			}
			writeLine();
		}
		if (node.handler->globalNames.size() > 0) {
			write("global ");
			for (size_t i = 0; i < node.handler->globalNames.size(); i++) {
				if (i > 0)
					write(", ");
				write(node.handler->globalNames[i]);
			}
			writeLine();
		}
		unindent();
		node.block->accept(*this);
		if (!isMethod) {
			writeLine("end");
		}
	}
}
void CodeWriterVisitor::visit(const ErrorNode &) {
	write("ERROR");
}
void CodeWriterVisitor::visit(const CommentNode &node) {
	write("-- ");
	write(node.text);
}
void CodeWriterVisitor::visit(const NewObjNode &node) {
	write("new ");
	write(node.objType);
	write("(");
	node.objArgs->accept(*this);
	write(")");
}
void CodeWriterVisitor::visit(const LiteralNode &node) {
	write(*node.value);
}
void CodeWriterVisitor::visit(const IfStmtNode &node) {
	write("if ");
	node.condition->accept(*this);
	write(" then");
	if (_sum) {
		if (node.hasElse) {
			write(" / else");
		}
	} else {
		writeLine();
		node.block1->accept(*this);
		if (node.hasElse) {
			writeLine("else");
			node.block2->accept(*this);
		}
		write("end if");
	}
}
void CodeWriterVisitor::visit(const EndCaseNode &) {
	write("end case");
}

void CodeWriterVisitor::visit(const ObjCallNode &node) {
	auto &rawArgs = node.argList->getValue()->l;

	auto &obj = rawArgs[0];
	bool parenObj = obj->hasSpaces(_dot);
	if (parenObj) {
		write("(");
	}
	obj->accept(*this);
	if (parenObj) {
		write(")");
	}

	write(".");
	write(node.name);
	write("(");
	for (size_t i = 1; i < rawArgs.size(); i++) {
		if (i > 1)
			write(", ");
		rawArgs[i]->accept(*this);
	}
	write(")");
}
void CodeWriterVisitor::visit(const PutStmtNode &node) {
	write("put ");
	node.value->accept(*this);
	write(" ");
	write(StandardNames::putTypeNames[node.type]);
	write(" ");
	node.variable->accept(*this); // TODO: we want the variable to always be verbose
}
void CodeWriterVisitor::visit(const TheExprNode &node) {
	write("the ");
	write(node.prop);
}
void CodeWriterVisitor::visit(const BinaryOpNode &node) {
	unsigned int precedence = node.getPrecedence();
	bool parenLeft = false;
	bool parenRight = false;
	if (precedence) {
		if (node.left->type == kBinaryOpNode) {
			auto leftBinaryOpNode = static_cast<BinaryOpNode *>(node.left.get());
			parenLeft = (leftBinaryOpNode->getPrecedence() != precedence);
		}
		parenRight = (node.right->type == kBinaryOpNode);
	}

	if (parenLeft) {
		write("(");
	}
	node.left->accept(*this);
	if (parenLeft) {
		write(")");
	}

	write(" ");
	write(StandardNames::binaryOpNames[node.opcode]);
	write(" ");

	if (parenRight) {
		write("(");
	}
	node.right->accept(*this);
	if (parenRight) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const CaseStmtNode &node) {
	write("case ");
	node.value->accept(*this);
	write(" of");
	if (_sum) {
		if (!node.firstLabel) {
			if (node.otherwise) {
				write(" / otherwise:");
			} else {
				write(" / end case");
			}
		}
	} else {
		writeLine();
		indent();
		if (node.firstLabel) {
			node.firstLabel->accept(*this);
		}
		if (node.otherwise) {
			node.otherwise->accept(*this);
		}
		unindent();
		write("end case");
	}
}
void CodeWriterVisitor::visit(const ExitStmtNode &) {
	write("exit");
}
void CodeWriterVisitor::visit(const TellStmtNode &node) {
	write("tell ");
	node.window->accept(*this);
	if (!_sum) {
		writeLine();
		node.block->accept(*this);
		write("end tell");
	}
}
void CodeWriterVisitor::visit(const WhenStmtNode &node) {
	write("when ");
	write(StandardNames::whenEventNames[node.event]);
	write(" then");

	for (size_t i = 0; i < node.script.size(); i++) {
		char ch = node.script[i];
		if (ch == '\r') {
			if (i != node.script.size() - 1) {
				writeLine();
			}
		} else {
			write(ch);
		}
	}
}

void CodeWriterVisitor::visit(const CaseLabelNode &node) {
	if (_sum) {
		write("(case) ");
		if (node.parent->type == kCaseLabelNode) {
			auto parentLabel = static_cast<CaseLabelNode *>(node.parent);
			if (parentLabel->nextOr.get() == &node) {
				write("..., ");
			}
		}

		bool parenValue = node.value->hasSpaces(_dot);
		if (parenValue) {
			write("(");
		}
		node.value->accept(*this);
		if (parenValue) {
			write(")");
		}

		if (node.nextOr) {
			write(", ...");
		} else {
			write(":");
		}
	} else {
		bool parenValue = node.value->hasSpaces(_dot);
		if (parenValue) {
			write("(");
		}
		node.value->accept(*this);
		if (parenValue) {
			write(")");
		}

		if (node.nextOr) {
			write(", ");
			node.nextOr->accept(*this);
		} else {
			writeLine(":");
			node.block->accept(*this);
		}
		if (node.nextLabel) {
			node.nextLabel->accept(*this);
		}
	}
}
void CodeWriterVisitor::visit(const ChunkExprNode &node) {
	write(StandardNames::chunkTypeNames[node.type]);
	write(" ");
	node.first->accept(*this);
	if (!(node.last->type == kLiteralNode && node.last->getValue()->type == kDatumInt && node.last->getValue()->i == 0)) {
		write(" to ");
		node.last->accept(*this);
	}
	write(" of ");
	node.string->accept(*this); // TODO: we want the string to always be verbose
}
void CodeWriterVisitor::visit(const InverseOpNode &node) {
	write("-");

	bool parenOperand = node.operand->hasSpaces(_dot);
	if (parenOperand) {
		write("(");
	}
	node.operand->accept(*this);
	if (parenOperand) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const ObjCallV4Node &node) {
	node.obj->accept(*this);
	write("(");
	node.argList->accept(*this);
	write(")");
}
void CodeWriterVisitor::visit(const OtherwiseNode &node) {
	if (_sum) {
		write("(case) otherwise:");
	} else {
		writeLine("otherwise:");
		node.block->accept(*this);
	}
}
void CodeWriterVisitor::visit(const MemberExprNode &node) {
	bool hasCastID = node.castID && !(node.castID->type == kLiteralNode && node.castID->getValue()->type == kDatumInt && node.castID->getValue()->i == 0);
	write(node.type);
	if (_dot) {
		write("(");
		node.memberID->accept(*this);
		if (hasCastID) {
			write(", ");
			node.castID->accept(*this);
		}
		write(")");
	} else {
		write(" ");

		bool parenMemberID = (node.memberID->type == kBinaryOpNode);
		if (parenMemberID) {
			write("(");
		}
		node.memberID->accept(*this);
		if (parenMemberID) {
			write(")");
		}

		if (hasCastID) {
			write(" of castLib ");

			bool parenCastID = (node.castID->type == kBinaryOpNode);
			if (parenCastID) {
				write("(");
			}
			node.castID->accept(*this);
			if (parenCastID) {
				write(")");
			}
		}
	}
}
void CodeWriterVisitor::visit(const ObjPropExprNode &node) {
	if (_dot) {
		bool parenObj = node.obj->hasSpaces(_dot);
		if (parenObj) {
			write("(");
		}
		node.obj->accept(*this);
		if (parenObj) {
			write(")");
		}

		write(".");
		write(node.prop);
	} else {
		write("the ");
		write(node.prop);
		write(" of ");

		bool parenObj = (node.obj->type == kBinaryOpNode);
		if (parenObj) {
			write("(");
		}
		node.obj->accept(*this);
		if (parenObj) {
			write(")");
		}
	}
}
void CodeWriterVisitor::visit(const PlayCmdStmtNode &node) {
	auto &rawArgs = node.argList->getValue()->l;

	write("play");

	if (rawArgs.size() == 0) {
		write(" done");
		return;
	}

	auto &frame = rawArgs[0];
	if (rawArgs.size() == 1) {
		write(" frame ");
		frame->accept(*this);
		return;
	}

	auto &movie = rawArgs[1];
	if (!(frame->type == kLiteralNode && frame->getValue()->type == kDatumInt && frame->getValue()->i == 1)) {
		write(" frame ");
		frame->accept(*this);
		write(" of");
	}
	write(" movie ");
	movie->accept(*this);
}
void CodeWriterVisitor::visit(const ThePropExprNode &node) {
	write("the ");
	write(node.prop);
	write(" of ");

	bool parenObj = (node.obj->type == kBinaryOpNode);
	if (parenObj) {
		write("(");
	}
	node.obj->accept(*this); // TODO: we want the object to always be verbose
	if (parenObj) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const MenuPropExprNode &node) {
	write("the ");
	write(StandardNames::menuPropertyNames[node.prop]);
	write(" of menu ");

	bool parenMenuID = (node.menuID->type == kBinaryOpNode);
	if (parenMenuID) {
		write("(");
	}
	node.menuID->accept(*this);
	if (parenMenuID) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const SoundCmdStmtNode &node) {
	write("sound ");
	write(node.cmd);
	if (node.argList->getValue()->l.size() > 0) {
		write(" ");
		node.argList->accept(*this);
	}
}
void CodeWriterVisitor::visit(const SoundPropExprNode &node) {
	write("the ");
	write(StandardNames::soundPropertyNames[node.prop]);
	write(" of sound ");

	bool parenSoundID = (node.soundID->type == kBinaryOpNode);
	if (parenSoundID) {
		write("(");
	}
	node.soundID->accept(*this);
	if (parenSoundID) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const AssignmentStmtNode &node) {
	if (!_dot) { // TODO: forceVerboseÒ
		write("set ");
		node.variable->accept(*this); // TODO: we want the variable to always be verbose
		write(" to ");
		node.value->accept(*this);
	} else {
		node.variable->accept(*this);
		write(" = ");
		node.value->accept(*this);
	}
}
void CodeWriterVisitor::visit(const ExitRepeatStmtNode &) {
	write("exit repeat");
}
void CodeWriterVisitor::visit(const NextRepeatStmtNode &) {
	write("next repeat");
}
void CodeWriterVisitor::visit(const ObjBracketExprNode &node) {
	bool parenObj = node.obj->hasSpaces(_dot);
	if (parenObj) {
		write("(");
	}
	node.obj->accept(*this);
	if (parenObj) {
		write(")");
	}

	write("[");
	node.prop->accept(*this);
	write("]");
}
void CodeWriterVisitor::visit(const SpritePropExprNode &node) {
	write("the ");
	write(StandardNames::spritePropertyNames[node.prop]);
	write(" of sprite ");

	bool parenSpriteID = (node.spriteID->type == kBinaryOpNode);
	if (parenSpriteID) {
		write("(");
	}
	node.spriteID->accept(*this);
	if (parenSpriteID) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const ChunkDeleteStmtNode &node) {
	write("delete ");
	node.chunk->accept(*this);
}
void CodeWriterVisitor::visit(const ChunkHiliteStmtNode &node) {
	write("hilite ");
	node.chunk->accept(*this);
}
void CodeWriterVisitor::visit(const RepeatWhileStmtNode &node) {
	write("repeat while ");
	node.condition->accept(*this);
	if (!_sum) {
		writeLine();
		node.block->accept(*this);
		write("end repeat");
	}
}
void CodeWriterVisitor::visit(const MenuItemPropExprNode &node) {
	write("the ");
	write(StandardNames::menuItemPropertyNames[node.prop]);
	write(" of menuItem ");

	bool parenItemID = (node.itemID->type == kBinaryOpNode);
	if (parenItemID) {
		write("(");
	}
	node.itemID->accept(*this);
	if (parenItemID) {
		write(")");
	}

	write(" of menu ");

	bool parenMenuID = (node.menuID->type ==kBinaryOpNode);
	if (parenMenuID) {
		write("(");
	}
	node.menuID->accept(*this);
	if (parenMenuID) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const ObjPropIndexExprNode &node) {
	bool parenObj = node.obj->hasSpaces(_dot);
	if (parenObj) {
		write("(");
	}
	node.obj->accept(*this);
	if (parenObj) {
		write(")");
	}

	write(".");
	write(node.prop);
	write("[");
	node.index->accept(*this);
	if (node.index2) {
		write("..");
		node.index2->accept(*this);
	}
	write("]");
}
void CodeWriterVisitor::visit(const RepeatWithInStmtNode &node) {
	write("repeat with ");
	write(node.varName);
	write(" in ");
	node.list->accept(*this);
	if (!_sum) {
		writeLine();
		node.block->accept(*this);
		write("end repeat");
	}
}
void CodeWriterVisitor::visit(const RepeatWithToStmtNode &node) {
	write("repeat with ");
	write(node.varName);
	write(" = ");
	node.start->accept(*this);
	if (node.up) {
		write(" to ");
	} else {
		write(" down to ");
	}
	node.end->accept(*this);
	if (!_sum) {
		writeLine();
		node.block->accept(*this);
		write("end repeat");
	}
}
void CodeWriterVisitor::visit(const SpriteWithinExprNode &node) {
	write("sprite ");

	bool parenFirstSprite = (node.firstSprite->type == kBinaryOpNode);
	if (parenFirstSprite) {
		write("(");
	}
	node.firstSprite->accept(*this);
	if (parenFirstSprite) {
		write(")");
	}

	write(" within ");

	bool parenSecondSprite = (node.secondSprite->type == kBinaryOpNode);
	if (parenSecondSprite) {
		write("(");
	}
	node.secondSprite->accept(*this);
	if (parenSecondSprite) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const LastStringChunkExprNode &node) {
	write("the last ");
	write(StandardNames::chunkTypeNames[node.type]);
	write(" in ");

	bool parenObj = (node.obj->type == kBinaryOpNode);
	if (parenObj) {
		write("(");
	}
	node.obj->accept(*this); // TODO: we want the object to always be verbose
	if (parenObj) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const SpriteIntersectsExprNode &node) {
	write("sprite ");

	bool parenFirstSprite = (node.firstSprite->type == kBinaryOpNode);
	if (parenFirstSprite) {
		write("(");
	}
	node.firstSprite->accept(*this);
	if (parenFirstSprite) {
		write(")");
	}

	write(" intersects ");

	bool parenSecondSprite = (node.secondSprite->type == kBinaryOpNode);
	if (parenSecondSprite) {
		write("(");
	}
	node.secondSprite->accept(*this);
	if (parenSecondSprite) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const StringChunkCountExprNode &node) {
	write("the number of ");
	write(StandardNames::chunkTypeNames[node.type]); // we want the object to always be verbose
	write("s in ");

	bool parenObj = (node.obj->type == kBinaryOpNode);
	if (parenObj) {
		write("(");
	}
	node.obj->accept(*this); // TODO dot false?
	if (parenObj) {
		write(")");
	}
}
void CodeWriterVisitor::visit(const VarNode &node) {
	write(node.varName);
}
void CodeWriterVisitor::visit(const CallNode &node) {
	if (node.isExpression && node.argList->getValue()->l.size() == 0) {
		if (node.name == "pi") {
			write("PI");
			return;
		}
		if (node.name == "space") {
			write("SPACE");
			return;
		}
		if (node.name == "void") {
			write("VOID");
			return;
		}
	}

	if (!_dot && node.isMemberExpr()) {
		/**
		 * In some cases, member expressions such as `member 1 of castLib 1` compile
		 * to the function call `member(1, 1)`. However, this doesn't parse correctly
		 * in pre-dot-syntax versions of Director, and `put(member(1, 1))` does not
		 * compile. Therefore, we rewrite these expressions to the verbose syntax when
		 * in verbose mode.
		 */
		write(node.name);
		write(" ");

		auto memberID = node.argList->getValue()->l[0];
		bool parenMemberID = (memberID->type == kBinaryOpNode);
		if (parenMemberID) {
			write("(");
		}
		memberID->accept(*this);
		if (parenMemberID) {
			write(")");
		}

		if (node.argList->getValue()->l.size() == 2) {
			write(" of castLib ");

			auto castID = node.argList->getValue()->l[1];
			bool parenCastID = (castID->type == kBinaryOpNode);
			if (parenCastID) {
				write("(");
			}
			castID->accept(*this);
			if (parenCastID) {
				write(")");
			}
		}
		return;
	}

	write(node.name);
	if (node.noParens()) {
		write(" ");
		node.argList->accept(*this);
	} else {
		write("(");
		node.argList->accept(*this);
		write(")");
	}
}
void CodeWriterVisitor::visit(const BlockNode &node) {
	indent();
	for (const auto &child : node.children) {
		child->accept(*this);
		writeLine();
	}
	unindent();
}
void CodeWriterVisitor::visit(const NotOpNode &node) {
	write("not ");

	bool parenOperand = node.operand->hasSpaces(_dot);
	if (parenOperand) {
		write("(");
	}
	node.operand->accept(*this);
	if (parenOperand) {
		write(")");
	}
}

void CodeWriterVisitor::indent() {
	_indent++;
}

void CodeWriterVisitor::unindent() {
	if (_indent > 0)
		_indent--;
}

void CodeWriterVisitor::writeIndentation() {
	if (_indentWritten)
		return;

	for (int i = 0; i < _indent; i++) {
		_str += _indentation;
	}

	_indentWritten = true;
	_lineWidth = _indent * _indentation.size();
}

void CodeWriterVisitor::write(char c) {
	writeIndentation();
	_str += c;
	_lineWidth++;
}

void CodeWriterVisitor::write(const Common::String &s) {
	writeIndentation();
	_str += s;
	_lineWidth += s.size();
}

void CodeWriterVisitor::writeLine() {
	_str += _lineEnding;
	_lineWidth += _lineEnding.size();
	_indentWritten = false;
	_lineWidth = 0;
}

void CodeWriterVisitor::writeLine(const Common::String &s) {
	writeIndentation();
	_str += s;
	_lineWidth += s.size();
	_str += _lineEnding;
	_lineWidth += _lineEnding.size();
	_indentWritten = false;
	_lineWidth = 0;
}

void CodeWriterVisitor::write(Datum &datum) {
	switch (datum.type) {
	case kDatumVoid:
		write("VOID");
		return;
	case kDatumSymbol:
		write("#" + datum.s);
		return;
	case kDatumVarRef:
		write(datum.s);
		return;
	case kDatumString:
		if (datum.s.size() == 0) {
			write("EMPTY");
			return;
		}
		if (datum.s.size() == 1) {
			switch (datum.s[0]) {
			case '\x03':
				write("ENTER");
				return;
			case '\x08':
				write("BACKSPACE");
				return;
			case '\t':
				write("TAB");
				return;
			case '\r':
				write("RETURN");
				return;
			case '"':
				write("QUOTE");
				return;
			default:
				break;
			}
		}
		if (_sum) {
			write("\"" + Common::toPrintable(datum.s) + "\"");
			return;
		}
		write("\"" + datum.s + "\"");
		return;
	case kDatumInt:
		write(Common::String::format("%d", datum.i));
		return;
	case kDatumFloat:
		write(Common::String::format("%g", datum.f));
		return;
	case kDatumList:
	case kDatumArgList:
	case kDatumArgListNoRet: {
		if (datum.type == kDatumList)
			write("[");
		for (size_t ii = 0; ii < datum.l.size(); ii++) {
			if (ii > 0)
				write(", ");
			datum.l[ii]->accept(*this);
		}
		if (datum.type == kDatumList)
			write("]");
	}
		return;
	case kDatumPropList: {
		write("[");
		if (datum.l.size() == 0) {
			write(":");
		} else {
			for (size_t ii = 0; ii < datum.l.size(); ii += 2) {
				if (ii > 0)
					write(", ");
				datum.l[ii]->accept(*this);
				write(": ");
				datum.l[ii + 1]->accept(*this);
			}
		}
		write("]");
	}
		return;
	}
}

} // namespace LingoDec
