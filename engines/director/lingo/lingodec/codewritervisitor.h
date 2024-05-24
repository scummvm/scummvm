/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_CODEWRITERVISITOR_H
#define LINGODEC_CODEWRITERVISITOR_H

#include "./ast.h"

namespace LingoDec {

class CodeWriterVisitor: public LingoDec::NodeVisitor {
public:
	CodeWriterVisitor(bool dotSyntax, bool sum, const Common::String &lineEnding = "\n", const Common::String &indentation = "  ")
		: _dot(dotSyntax), _sum(sum), _lineEnding(lineEnding), _indentation(indentation) {}
	virtual ~CodeWriterVisitor() {}
	virtual void visit(const LingoDec::HandlerNode& node) override;
	virtual void visit(const LingoDec::ErrorNode& node) override;
	virtual void visit(const LingoDec::CommentNode& node) override;
	virtual void visit(const LingoDec::NewObjNode& node) override;
	virtual void visit(const LingoDec::LiteralNode& node) override;
	virtual void visit(const LingoDec::IfStmtNode& node) override;
	virtual void visit(const LingoDec::EndCaseNode& node) override;
	virtual void visit(const LingoDec::ObjCallNode& node) override;
	virtual void visit(const LingoDec::PutStmtNode& node) override;
	virtual void visit(const LingoDec::TheExprNode& node) override;
	virtual void visit(const LingoDec::BinaryOpNode& node) override;
	virtual void visit(const LingoDec::CaseStmtNode& node) override;
	virtual void visit(const LingoDec::ExitStmtNode& node) override;
	virtual void visit(const LingoDec::TellStmtNode& node) override;
	virtual void visit(const LingoDec::WhenStmtNode& node) override;
	virtual void visit(const LingoDec::CaseLabelNode& node) override;
	virtual void visit(const LingoDec::ChunkExprNode& node) override;
	virtual void visit(const LingoDec::InverseOpNode& node) override;
	virtual void visit(const LingoDec::ObjCallV4Node& node) override;
	virtual void visit(const LingoDec::OtherwiseNode& node) override;
	virtual void visit(const LingoDec::MemberExprNode& node) override;
	virtual void visit(const LingoDec::ObjPropExprNode& node) override;
	virtual void visit(const LingoDec::PlayCmdStmtNode& node) override;
	virtual void visit(const LingoDec::ThePropExprNode& node) override;
	virtual void visit(const LingoDec::MenuPropExprNode& node) override;
	virtual void visit(const LingoDec::SoundCmdStmtNode& node) override;
	virtual void visit(const LingoDec::SoundPropExprNode& node) override;
	virtual void visit(const LingoDec::AssignmentStmtNode& node) override;
	virtual void visit(const LingoDec::ExitRepeatStmtNode& node) override;
	virtual void visit(const LingoDec::NextRepeatStmtNode& node) override;
	virtual void visit(const LingoDec::ObjBracketExprNode& node) override;
	virtual void visit(const LingoDec::SpritePropExprNode& node) override;
	virtual void visit(const LingoDec::ChunkDeleteStmtNode& node) override;
	virtual void visit(const LingoDec::ChunkHiliteStmtNode& node) override;
	virtual void visit(const LingoDec::RepeatWhileStmtNode& node) override;
	virtual void visit(const LingoDec::MenuItemPropExprNode& node) override;
	virtual void visit(const LingoDec::ObjPropIndexExprNode& node) override;
	virtual void visit(const LingoDec::RepeatWithInStmtNode& node) override;
	virtual void visit(const LingoDec::RepeatWithToStmtNode& node) override;
	virtual void visit(const LingoDec::SpriteWithinExprNode& node) override;
	virtual void visit(const LingoDec::LastStringChunkExprNode& node) override;
	virtual void visit(const LingoDec::SpriteIntersectsExprNode& node) override;
	virtual void visit(const LingoDec::StringChunkCountExprNode& node) override;
	virtual void visit(const LingoDec::VarNode& node) override;
	virtual void visit(const LingoDec::CallNode& node) override;
	virtual void visit(const LingoDec::BlockNode& node) override;
	virtual void visit(const LingoDec::NotOpNode& node) override;

	size_t lineWidth() const { return _lineWidth; }
	void indent();
	void unindent();
	void writeIndentation();
	void write(char c);
	void write(const Common::String& s);
	void writeLine();
	void writeLine(const Common::String& s);
	void write(LingoDec::Datum& datum);

public:
	Common::String _str;

private:
	bool _dot = false;
	bool _sum = false;
	Common::String _lineEnding;
	Common::String _indentation = "  ";
	bool _indentWritten = false;
	int _indent = 0;
	size_t _lineWidth = 0;
};

} // namespace LingoDec

#endif // LINGODEC_CODEWRITERVISITOR_H
