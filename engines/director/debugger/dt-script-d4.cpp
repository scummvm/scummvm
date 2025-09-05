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

#include "director/director.h"
#include "director/movie.h"
#include "director/cast.h"
#include "director/debugger/dt-internal.h"

#include "director/debugger.h"

#include "director/lingo/lingo-object.h"

#include "director/lingo/lingodec/codewritervisitor.h"
#include "director/lingo/lingodec/names.h"
#include "director/lingo/lingodec/script.h"

namespace Director {
namespace DT {

class RenderScriptVisitor : public LingoDec::NodeVisitor {
public:
	RenderScriptVisitor(ImGuiScript &script, bool showByteCode, bool scrollTo) : _script(script), _showByteCode(showByteCode), _scrollTo(scrollTo) {
		Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
		if (!callstack.empty()) {
			CFrame *head = callstack[callstack.size() - 1];
			_isScriptInDebug = (head->sp.ctx->_id == script.id.member) && (*head->sp.name == script.handlerId);
		}
		_script.startOffsets.clear();
	}

	virtual void visit(const LingoDec::HandlerNode &node) override {
		if (_showByteCode) {
			byteCode(node);
			return;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2());
		lingoCode(node);
		ImGui::PopStyleVar();
	}

	virtual void visit(const LingoDec::CommentNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._comment_color), "-- %s", node.text.c_str());
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::LiteralNode &node) override {
		write(*node.value);
	}

	virtual void visit(const LingoDec::NewObjNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), "new");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(_state->_colors._type_color), "%s", node.objType.c_str());
		ImGui::SameLine();
		ImGui::Text(" (");
		ImGui::SameLine();
		node.objArgs->accept(*this);
		ImGui::Text(")");
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::ObjCallV4Node &node) override {
		if (node.isStatement) {
			renderLine(node._startOffset);
			renderIndentation();
		}

		node.obj->accept(*this);
		ImGui::SameLine();
		ImGui::Text(" (");
		ImGui::SameLine();
		node.argList->accept(*this);
		ImGui::SameLine();
		ImGui::Text(")");
		if (!node.isStatement) {
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::CallNode &node) override {
		// new line only if it's a statement
		if (node.isStatement) {
			renderLine(node._startOffset);
			renderIndentation();
		}

		const ImVec4 color = (ImVec4)ImColor(g_lingo->_builtinCmds.contains(node.name) ? _state->_colors._builtin_color : _state->_colors._call_color);
		ImGui::TextColored(color, "%s", node.name.c_str());
		// TODO: we should test Director::builtins too (but inaccessible)
		if (!g_lingo->_builtinFuncs.contains(node.name) && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
			ImGui::Text("Go to definition");
			ImGui::EndTooltip();
		}
		if (!g_lingo->_builtinFuncs.contains(node.name) && ImGui::IsItemClicked()) {
			int32 obj = 0;
			for (uint i = 0; i < _script.bytecodeArray.size(); i++) {
				if (node._startOffset == _script.bytecodeArray[i].pos) {
					// This obj represents the index of the handler being called
					// The index may be local (lingo context-wide) or global (cast-wide)
					obj = _script.bytecodeArray[i].obj;
					break;
				}
			}
			ScriptContext *context = getScriptContext(obj, _script.id, node.name);

			if (context) {
				ImGuiScript script = toImGuiScript(_script.type, CastMemberID(context->_id, _script.id.castLib), node.name);
				const Director::Movie *movie = g_director->getCurrentMovie();

				script.byteOffsets = context->_functionByteOffsets[script.handlerId];
				script.moviePath = _script.moviePath;
				int castId = context->_id;
				bool childScript = false;
				if (castId == -1) {
					castId = movie->getCast()->getCastIdByScriptId(context->_parentNumber);
					childScript = true;
				}

				script.handlerName = formatHandlerName(context->_scriptId, castId, script.handlerId, context->_scriptType, childScript);
				setScriptToDisplay(script);
				_state->_dbg._goToDefinition = true;
			}
		}
		ImGui::SameLine();

		if (node.noParens()) {
			ImGui::Text(" ");
			ImGui::SameLine();
			node.argList->accept(*this);
		} else {
			ImGui::Text("(");
			ImGui::SameLine();
			node.argList->accept(*this);
			ImGui::Text(")");
			ImGui::SameLine();
		}

		if (node.isStatement) {
			ImGui::NewLine();
		}
	}

	virtual void visit(const LingoDec::BlockNode &node) override {
		indent();
		for (const auto &child : node.children) {
			child->accept(*this);
		}
		unindent();
	}

	virtual void visit(const LingoDec::PutStmtNode &node) override {
		write(node._startOffset, "put ", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.value->accept(*this);
		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), LingoDec::StandardNames::putTypeNames[node.type]);
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		node.variable->accept(*this);
		ImGui::NewLine();
	}

	virtual void visit(const LingoDec::TheExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the %s", node.prop.c_str());
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::ExitStmtNode &node) override {
		write(node._startOffset, "exit", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::WhenStmtNode &node) override {
		write(node._startOffset, "when ", _state->_colors._keyword_color);
		ImGui::SameLine();
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), LingoDec::StandardNames::whenEventNames[node.event]);
		ImGui::SameLine();
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " then ");
		ImGui::SameLine();
		ImGui::Text("%s", node.script.c_str());
	}

	virtual void visit(const LingoDec::RepeatWhileStmtNode &node) override {
		write(node._startOffset, "repeat while ", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.condition->accept(*this);
		ImGui::NewLine();
		node.block->accept(*this);
		write(node._endOffset, "end repeat", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::RepeatWithInStmtNode &node) override {
		write(node._startOffset, "repeat with ", _state->_colors._keyword_color);
		ImGui::SameLine();
		renderVar(node.varName);
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " in ");
		ImGui::SameLine();
		node.list->accept(*this);
		ImGui::NewLine();
		node.block->accept(*this);
		write(node._endOffset, "end repeat", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::RepeatWithToStmtNode &node) override {
		write(node._startOffset, "repeat with ", _state->_colors._keyword_color);
		ImGui::SameLine();
		renderVar(node.varName);
		ImGui::Text(" = ");
		ImGui::SameLine();
		node.start->accept(*this);
		if (node.up) {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " to ");
			ImGui::SameLine();
		} else {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " down to ");
			ImGui::SameLine();
		}
		node.end->accept(*this);
		ImGui::NewLine();
		node.block->accept(*this);
		write(node._endOffset, "end repeat", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::IfStmtNode &node) override {
		{
			write(node._startOffset, "if ", _state->_colors._keyword_color);
			ImGui::SameLine();
			node.condition->accept(*this);
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " then ");
		}
		node.block1->accept(*this);
		if (node.hasElse) {
			write(node.block2->_startOffset, "else ", _state->_colors._keyword_color);
			node.block2->accept(*this);
		}
		write(node._endOffset, "end if", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::TellStmtNode &node) override {
		write(node._startOffset, "tell ", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.window->accept(*this);
		ImGui::NewLine();
		node.block->accept(*this);
		write(node._endOffset, "end tell", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::EndCaseNode &node) override {
		write(node._endOffset, "end case", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::CaseLabelNode &node) override {
		renderLine(node._startOffset);
		renderIndentation();
		bool parenValue = node.value->hasSpaces(_dot);
		if (parenValue) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.value->accept(*this);
		if (parenValue) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		if (node.nextOr) {
			ImGui::Text(",");
			ImGui::SameLine();
			node.nextOr->accept(*this);
		} else {
			ImGui::Text(":");
			node.block->accept(*this);
		}
		if (node.nextLabel) {
			node.nextLabel->accept(*this);
		}
	}

	virtual void visit(const LingoDec::ChunkExprNode &node) override {
		ImGui::Text(LingoDec::StandardNames::chunkTypeNames[node.type]);
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		node.first->accept(*this);
		if (!(node.last->type == LingoDec::kLiteralNode && node.last->getValue()->type == LingoDec::kDatumInt && node.last->getValue()->i == 0)) {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " to ");
			ImGui::SameLine();
			node.last->accept(*this);
		}
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " of ");
		ImGui::SameLine();
		node.string->accept(*this);
	}

	virtual void visit(const LingoDec::InverseOpNode &node) override {
		ImGui::Text("-");
		ImGui::SameLine();

		bool parenOperand = node.operand->hasSpaces(_dot);
		if (parenOperand) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.operand->accept(*this);
		if (parenOperand) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::CaseStmtNode &node) override {
		write(node._startOffset, "case ", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.value->accept(*this);
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " of ");
		indent();
		if (node.firstLabel) {
			node.firstLabel->accept(*this);
		}
		if (node.otherwise) {
			node.otherwise->accept(*this);
		}
		unindent();
		write(node._endOffset, "end case", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::ObjCallNode &node) override {
		auto &rawArgs = node.argList->getValue()->l;

		auto &obj = rawArgs[0];
		bool parenObj = obj->hasSpaces(_dot);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::Text(node.name.c_str());
		ImGui::SameLine();
		ImGui::Text("(");
		ImGui::SameLine();
		for (size_t i = 1; i < rawArgs.size(); i++) {
			if (i > 1) {
				ImGui::Text(",");
				ImGui::SameLine();
			}
			rawArgs[i]->accept(*this);
		}
		ImGui::Text(")");
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::ObjPropExprNode &node) override {
		if (_dot) {
			bool parenObj = node.obj->hasSpaces(_dot);
			if (parenObj) {
				ImGui::Text("(");
				ImGui::SameLine();
			}
			node.obj->accept(*this);
			if (parenObj) {
				ImGui::Text(")");
				ImGui::SameLine();
			}

			ImGui::Text(".");
			ImGui::SameLine();
			ImGui::Text("%s", node.prop.c_str());
			ImGui::SameLine();
		} else {
			ImGui::TextColored(_state->_colors._the_color, "the %s", node.prop.c_str());
			ImGui::SameLine();
			ImGui::TextColored(_state->_colors._keyword_color, " of ");
			ImGui::SameLine();

			bool parenObj = (node.obj->type == LingoDec::kBinaryOpNode);
			if (parenObj) {
				ImGui::Text("(");
				ImGui::SameLine();
			}
			node.obj->accept(*this);
			if (parenObj) {
				ImGui::Text(")");
				ImGui::SameLine();
			}
		}
	}

	virtual void visit(const LingoDec::BinaryOpNode &node) override {
		unsigned int precedence = node.getPrecedence();
		bool parenLeft = false;
		bool parenRight = false;
		if (precedence) {
			if (node.left->type == LingoDec::kBinaryOpNode) {
				auto leftBinaryOpNode = static_cast<LingoDec::BinaryOpNode *>(node.left.get());
				parenLeft = (leftBinaryOpNode->getPrecedence() != precedence);
			}
			parenRight = (node.right->type == LingoDec::kBinaryOpNode);
		}

		if (parenLeft) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.left->accept(*this);
		if (parenLeft) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::Text(LingoDec::StandardNames::binaryOpNames[node.opcode]);
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();

		if (parenRight) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.right->accept(*this);
		if (parenRight) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::OtherwiseNode &node) override {
		write(node._startOffset, "otherwise:", _state->_colors._keyword_color);
		node.block->accept(*this);
	}

	virtual void visit(const LingoDec::MemberExprNode &node) override {
		bool hasCastID = node.castID && !(node.castID->type == LingoDec::kLiteralNode && node.castID->getValue()->type == LingoDec::kDatumInt && node.castID->getValue()->i == 0);
		ImGui::Text(node.type.c_str());
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		if (_dot) {
			ImGui::Text("(");
			ImGui::SameLine();
			node.memberID->accept(*this);
			if (hasCastID) {
				ImGui::Text(",");
				ImGui::SameLine();
				node.castID->accept(*this);
			}
			ImGui::Text(")");
			ImGui::SameLine();
		} else {
			bool parenMemberID = (node.memberID->type == LingoDec::kBinaryOpNode);
			if (parenMemberID) {
				ImGui::Text("(");
				ImGui::SameLine();
			}
			node.memberID->accept(*this);
			if (parenMemberID) {
				ImGui::Text(")");
				ImGui::SameLine();
			}

			if (hasCastID) {
				ImGui::TextColored(ImColor(_state->_colors._keyword_color), "of castLib");
				ImGui::SameLine();

				bool parenCastID = (node.castID->type == LingoDec::kBinaryOpNode);
				if (parenCastID) {
					ImGui::Text("(");
					ImGui::SameLine();
				}
				node.castID->accept(*this);
				if (parenCastID) {
					ImGui::Text(")");
					ImGui::SameLine();
				}
			}
		}
	}

	virtual void visit(const LingoDec::PlayCmdStmtNode &node) override {
		auto &rawArgs = node.argList->getValue()->l;

		write(node._startOffset, "play ", _state->_colors._keyword_color);
		ImGui::SameLine();

		if (rawArgs.size() == 0) {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " done");
			ImGui::SameLine();
			return;
		}

		auto &frame = rawArgs[0];
		if (rawArgs.size() == 1) {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " frame ");
			ImGui::SameLine();
			frame->accept(*this);
			return;
		}

		auto &movie = rawArgs[1];
		if (!(frame->type == LingoDec::kLiteralNode && frame->getValue()->type == LingoDec::kDatumInt && frame->getValue()->i == 1)) {
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " frame ");
			ImGui::SameLine();
			frame->accept(*this);
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " of ");
			ImGui::SameLine();
		}
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " movie ");
		ImGui::SameLine();
		movie->accept(*this);
	}

	virtual void visit(const LingoDec::ThePropExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), "the ");
		ImGui::SameLine();
		ImGui::Text(node.prop.c_str());
		ImGui::SameLine();
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " of ");
		ImGui::SameLine();

		bool parenObj = (node.obj->type == LingoDec::kBinaryOpNode);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::MenuPropExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the %s of menu ", LingoDec::StandardNames::menuPropertyNames[node.prop]);
		ImGui::SameLine();

		bool parenMenuID = (node.menuID->type == LingoDec::kBinaryOpNode);
		if (parenMenuID) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.menuID->accept(*this);
		if (parenMenuID) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::SoundCmdStmtNode &node) override {
		write(node._startOffset, "sound ", _state->_colors._keyword_color);
		ImGui::SameLine();
		ImGui::Text(node.cmd.c_str());
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		if (node.argList->getValue()->l.size() > 0) {
			node.argList->accept(*this);
		}
		ImGui::NewLine();
	}

	virtual void visit(const LingoDec::SoundPropExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the %s of sound ", LingoDec::StandardNames::soundPropertyNames[node.prop]);
		ImGui::SameLine();

		bool parenSoundID = (node.soundID->type == LingoDec::kBinaryOpNode);
		if (parenSoundID) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.soundID->accept(*this);
		if (parenSoundID) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::AssignmentStmtNode &node) override {
		if (!_dot) {
			write(node._startOffset, "set ", _state->_colors._keyword_color);
			ImGui::SameLine();
			node.variable->accept(*this);
			ImGui::TextColored(ImColor(_state->_colors._keyword_color), " to ");
			ImGui::SameLine();
			node.value->accept(*this);
		} else {
			node.variable->accept(*this);
			ImGui::Text(" = ");
			ImGui::SameLine();
			node.value->accept(*this);
		}
		ImGui::NewLine();
	}

	virtual void visit(const LingoDec::ExitRepeatStmtNode &node) override {
		write(node._startOffset, "exit repeat", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::NextRepeatStmtNode &node) override {
		write(node._startOffset, "next repeat", _state->_colors._keyword_color);
	}

	virtual void visit(const LingoDec::ObjBracketExprNode &node) override {
		bool parenObj = node.obj->hasSpaces(_dot);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::Text("[");
		ImGui::SameLine();
		node.prop->accept(*this);
		ImGui::Text("]");
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::SpritePropExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the %s of sprite ", LingoDec::StandardNames::spritePropertyNames[node.prop]);
		ImGui::SameLine();

		bool parenSpriteID = (node.spriteID->type == LingoDec::kBinaryOpNode);
		if (parenSpriteID) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.spriteID->accept(*this);
		if (parenSpriteID) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::ChunkDeleteStmtNode &node) override {
		write(node._startOffset, "delete", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.chunk->accept(*this);
	}

	virtual void visit(const LingoDec::ChunkHiliteStmtNode &node) override {
		write(node._startOffset, "hilite", _state->_colors._keyword_color);
		ImGui::SameLine();
		node.chunk->accept(*this);
	}

	virtual void visit(const LingoDec::MenuItemPropExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the %s of menuItem ", LingoDec::StandardNames::menuItemPropertyNames[node.prop]);
		ImGui::SameLine();

		bool parenItemID = (node.itemID->type == LingoDec::kBinaryOpNode);
		if (parenItemID) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.itemID->accept(*this);
		if (parenItemID) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " of menu ");
		ImGui::SameLine();

		bool parenMenuID = (node.menuID->type == LingoDec::kBinaryOpNode);
		if (parenMenuID) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.menuID->accept(*this);
		if (parenMenuID) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::ObjPropIndexExprNode &node) override {
		bool parenObj = node.obj->hasSpaces(_dot);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::Text(".");
		ImGui::SameLine();
		ImGui::Text(node.prop.c_str());
		ImGui::SameLine();
		ImGui::Text("[");
		ImGui::SameLine();
		node.index->accept(*this);
		if (node.index2) {
			ImGui::Text("..");
			ImGui::SameLine();
			node.index2->accept(*this);
		}
		ImGui::Text("]");
		ImGui::SameLine();
	}

	virtual void visit(const LingoDec::SpriteWithinExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), "sprite ");
		ImGui::SameLine();

		bool parenFirstSprite = (node.firstSprite->type == LingoDec::kBinaryOpNode);
		if (parenFirstSprite) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.firstSprite->accept(*this);
		if (parenFirstSprite) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " within ");
		ImGui::SameLine();

		bool parenSecondSprite = (node.secondSprite->type == LingoDec::kBinaryOpNode);
		if (parenSecondSprite) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.secondSprite->accept(*this);
		if (parenSecondSprite) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::LastStringChunkExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the last %s in ", LingoDec::StandardNames::chunkTypeNames[node.type]);
		ImGui::SameLine();

		bool parenObj = (node.obj->type == LingoDec::kBinaryOpNode);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::SpriteIntersectsExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), "sprite ");
		ImGui::SameLine();

		bool parenFirstSprite = (node.firstSprite->type == LingoDec::kBinaryOpNode);
		if (parenFirstSprite) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.firstSprite->accept(*this);
		if (parenFirstSprite) {
			ImGui::Text(")");
			ImGui::SameLine();
		}

		ImGui::TextColored(ImColor(_state->_colors._keyword_color), " intersects ");
		ImGui::SameLine();

		bool parenSecondSprite = (node.secondSprite->type == LingoDec::kBinaryOpNode);
		if (parenSecondSprite) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.secondSprite->accept(*this);
		if (parenSecondSprite) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::StringChunkCountExprNode &node) override {
		ImGui::TextColored(ImColor(_state->_colors._the_color), "the number of %ss in ", LingoDec::StandardNames::chunkTypeNames[node.type]);
		ImGui::SameLine();

		bool parenObj = (node.obj->type == LingoDec::kBinaryOpNode);
		if (parenObj) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.obj->accept(*this);
		if (parenObj) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void visit(const LingoDec::VarNode &node) override {
		renderVar(node.varName);
	}

	virtual void visit(const LingoDec::NotOpNode &node) override {
		ImGui::Text("not ");
		ImGui::SameLine();

		bool parenOperand = node.operand->hasSpaces(_dot);
		if (parenOperand) {
			ImGui::Text("(");
			ImGui::SameLine();
		}
		node.operand->accept(*this);
		if (parenOperand) {
			ImGui::Text(")");
			ImGui::SameLine();
		}
	}

	virtual void defaultVisit(const LingoDec::Node &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		node.accept(code);
		if (node.isStatement) {
			renderLine(node._startOffset);
			renderIndentation();
		}
		ImGui::Text("%s", code._str.c_str());
	}

private:
	void write(LingoDec::Datum &datum) {
		switch (datum.type) {
		case LingoDec::kDatumVoid:
			ImGui::TextColored(_state->_colors._keyword_color, "VOID");
			ImGui::SameLine();
			return;
		case LingoDec::kDatumSymbol:
			ImGui::Text("#%s", datum.s.c_str());
			ImGui::SameLine();
			return;
		case LingoDec::kDatumVarRef:
			ImGui::TextColored(_state->_colors._var_color, datum.s.c_str());
			ImGui::SameLine();
			return;
		case LingoDec::kDatumString:
			if (datum.s.empty()) {
				ImGui::TextColored(_state->_colors._keyword_color, "EMPTY");
				ImGui::SameLine();
				return;
			}
			if (datum.s.size() == 1) {
				switch (datum.s[0]) {
				case '\x03':
					ImGui::TextColored(_state->_colors._keyword_color, "ENTER");
					ImGui::SameLine();
					return;
				case '\x08':
					ImGui::TextColored(_state->_colors._keyword_color, "BACKSPACE");
					ImGui::SameLine();
					return;
				case '\t':
					ImGui::TextColored(_state->_colors._keyword_color, "TAB");
					ImGui::SameLine();
					return;
				case '\r':
					ImGui::TextColored(_state->_colors._keyword_color, "RETURN");
					ImGui::SameLine();
					return;
				case '"':
					ImGui::TextColored(_state->_colors._keyword_color, "QUOTE");
					ImGui::SameLine();
					return;
				default:
					break;
				}
			}
			ImGui::Text("\"%s\"", datum.s.c_str());
			ImGui::SameLine();
			return;
		case LingoDec::kDatumInt:
			ImGui::TextColored(_state->_colors._literal_color, "%d", datum.i);
			ImGui::SameLine();
			return;
		case LingoDec::kDatumFloat:
			ImGui::TextColored(_state->_colors._literal_color, "%g", datum.f);
			ImGui::SameLine();
			return;
		case LingoDec::kDatumList:
		case LingoDec::kDatumArgList:
		case LingoDec::kDatumArgListNoRet: {
			if (datum.type == LingoDec::kDatumList) {
				ImGui::Text("[");
				ImGui::SameLine();
			}
			for (size_t ii = 0; ii < datum.l.size(); ii++) {
				if (ii > 0) {
					ImGui::Text(", ");
					ImGui::SameLine();
				}
				datum.l[ii]->accept(*this);
			}
			if (datum.type == LingoDec::kDatumList) {
				ImGui::Text("]");
				ImGui::SameLine();
			}
		}
			return;
		case LingoDec::kDatumPropList: {
			ImGui::Text("[");
			ImGui::SameLine();
			if (datum.l.size() == 0) {
				ImGui::Text(":");
				ImGui::SameLine();
			} else {
				for (size_t ii = 0; ii < datum.l.size(); ii += 2) {
					if (ii > 0) {
						ImGui::Text(", ");
						ImGui::SameLine();
					}
					datum.l[ii]->accept(*this);
					ImGui::Text(": ");
					ImGui::SameLine();
					datum.l[ii + 1]->accept(*this);
				}
			}
			ImGui::Text("]");
			ImGui::SameLine();
		}
			return;
		}
	}

	void renderVar(const Common::String &varName) {
		ImGui::TextColored(_state->_colors._var_color, "%s", varName.c_str());
		if (ImGui::IsItemHovered() && g_lingo->_globalvars.contains(varName)) {
			const Datum &val = g_lingo->_globalvars.getVal(varName);
			ImGui::BeginTooltip();
			ImGui::Text("%s", varName.c_str());
			ImGui::Text("Click to add to watches.");
			Common::String s = val.asString(true);
			s.wordWrap(150);
			if (s.size() > 4000) {
				uint chop = s.size() - 4000;
				s.chop(s.size() - 4000);
				s += Common::String::format("... [chopped %d chars]", chop);
			}
			ImGui::Text("= %s", s.c_str());
			ImGui::EndTooltip();
		}
		if (ImGui::IsItemClicked()) {
			_state->_variables[varName] = true;
		}
		ImGui::SameLine();
	}

	void lingoCode(const LingoDec::HandlerNode &node) {
		if (_script.isGenericEvent) {
			node.block->accept(*this);
			return;
		}

		bool isMethod = _script.isMethod;
		write(node._startOffset, isMethod ? "method " : "on ", _state->_colors._keyword_color);
		ImGui::SameLine();
		ImGui::TextColored(_state->_colors._call_color, "%s", _script.handlerId.c_str());
		ImGui::SameLine();

		if (!_script.argumentNames.empty()) {
			ImGui::Text(" ");
			ImGui::SameLine();
			for (size_t i = 0; i < _script.argumentNames.size(); i++) {
				if (i > 0) {
					ImGui::Text(", ");
					ImGui::SameLine();
				}
				ImGui::Text("%s", _script.argumentNames[i].c_str());
				ImGui::SameLine();
			}
		}
		if (_state->_dbg._goToDefinition && _scrollTo) {
			ImGui::SetScrollHereY(0.5f);
			_state->_dbg._goToDefinition = false;
		}

		indent();

		if (isMethod && !_script.propertyNames.empty() && node.handler == &node.handler->script->handlers[0]) {
			ImGui::NewLine();
			write(node._startOffset, "instance ");
			ImGui::SameLine();
			for (size_t i = 0; i < _script.propertyNames.size(); i++) {
				if (i > 0)
					ImGui::Text(",");
				ImGui::SameLine();
				ImGui::TextColored(_state->_colors._var_color, "%s", _script.propertyNames[i].c_str());
				ImGui::SameLine();
			}
		}

		if (!_script.globalNames.empty()) {
			ImGui::NewLine();
			write(node._startOffset, "global ");
			ImGui::SameLine();
			for (size_t i = 0; i < _script.globalNames.size(); i++) {
				if (i > 0) {
					ImGui::Text(",");
					ImGui::SameLine();
				}
				renderVar(_script.globalNames[i]);
				ImGui::SameLine();
			}
		}

		ImGui::NewLine();
		unindent();
		node.block->accept(*this);

		if (!isMethod) {
			write(node._endOffset, "end", _state->_colors._keyword_color);
		}
	}

	void byteCode(const LingoDec::HandlerNode &node) {
		bool isMethod = _script.isMethod;
		if (!_script.isGenericEvent) {
			Common::String code;
			if (isMethod) {
				code += "method ";
			} else {
				code += "on ";
			}
			code += _script.handlerId;
			if (_script.argumentNames.size() > 0) {
				code += " ";
				for (size_t i = 0; i < _script.argumentNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += _script.argumentNames[i];
				}
			}
			writeByteCode(0, code);
		}
		for (uint i = 0; i < _script.bytecodeArray.size(); i++) {
			LingoDec::CodeWriterVisitor code(_dot, true);
			code.indent();
			auto &bytecode = _script.bytecodeArray[i];
			code.write(LingoDec::StandardNames::getOpcodeName(bytecode.opID));
			switch (bytecode.opcode) {
			case LingoDec::kOpJmp:
			case LingoDec::kOpJmpIfZ:
				code.write(" ");
				code.write(posToString(bytecode.pos + bytecode.obj));
				break;
			case LingoDec::kOpEndRepeat:
				code.write(" ");
				code.write(posToString(bytecode.pos - bytecode.obj));
				break;
			case LingoDec::kOpPushFloat32:
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
			writeByteCode(bytecode.pos, code._str);
		}
		if (!_script.isGenericEvent) {
			if (!isMethod) {
				writeByteCode(node._endOffset, "end");
			}
		}
	}

	void write(uint32 offset, const Common::String &code, ImVec4 color = ImVec4(1, 1, 1, 1)) {
		renderLine(offset);
		renderIndentation();
		ImGui::TextColored(color, "%s", code.c_str());
	}

	void writeByteCode(uint32 offset, const Common::String &code) {
		renderLine(offset);
		Common::String s;
		for (int i = 0; i < _indent; i++) {
			s += "  ";
		}
		ImGui::Text("%s", (s + code).c_str());
	}

	void renderLine(uint p) {
		bool showCurrentStatement = false;
		p = MIN(p, _script.byteOffsets.size() - 1);
		uint pc = _script.byteOffsets[p];
		_script.startOffsets.push_back(pc);

		if (_script.pc != 0 && pc >= _script.pc) {
			if (!_currentStatementDisplayed) {
				showCurrentStatement = true;
				_currentStatementDisplayed = true;
			}
		} else if (_isScriptInDebug && g_lingo->_exec._state == kPause) {
			// check current statement
			if (!_currentStatementDisplayed) {
				if (g_lingo->_state->pc <= pc) {
					showCurrentStatement = true;
					_currentStatementDisplayed = true;
				}
			}
		}

		ImDrawList *dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const float width = ImGui::GetContentRegionAvail().x;
		const ImVec2 mid(pos.x + 7, pos.y + 7);

		ImVec4 color = _state->_colors._bp_color_disabled;
		const Director::Breakpoint *bp = getBreakpoint(_script.handlerId, _script.id.member, pc);
		if (bp)
			color = _state->_colors._bp_color_enabled;

		// Need to give a new id for each button
		Common::String id = _script.handlerId + _renderLineID;
		ImGui::PushID(id.c_str());
		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		ImGui::PopID();
		_renderLineID++;

		// click on breakpoint column?
		if (ImGui::IsItemClicked(0)) {
			if (color == _state->_colors._bp_color_enabled) {
				g_lingo->delBreakpoint(bp->id);
				color = _state->_colors._bp_color_disabled;
			} else {
				Director::Breakpoint newBp;
				newBp.type = kBreakpointFunction;
				newBp.scriptId = _script.id.member;
				newBp.funcName = _script.handlerId;
				newBp.funcOffset = pc;
				g_lingo->addBreakpoint(newBp);
				color = _state->_colors._bp_color_enabled;
			}
		}

		if (color == _state->_colors._bp_color_disabled && ImGui::IsItemHovered()) {
			color = _state->_colors._bp_color_hover;
		}

		// draw breakpoint
		if (!bp || bp->enabled)
			dl->AddCircleFilled(mid, 4.0f, ImColor(color));
		else
			dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._line_color));

		// draw current statement
		if (showCurrentStatement) {
			dl->AddQuadFilled(ImVec2(pos.x, pos.y + 4.f), ImVec2(pos.x + 9.f, pos.y + 4.f), ImVec2(pos.x + 9.f, pos.y + 10.f), ImVec2(pos.x, pos.y + 10.f), ImColor(_state->_colors._current_statement));
			dl->AddTriangleFilled(ImVec2(pos.x + 8.f, pos.y), ImVec2(pos.x + 14.f, pos.y + 7.f), ImVec2(pos.x + 8.f, pos.y + 14.f), ImColor(_state->_colors._current_statement));
			if (_state->_dbg._scrollToPC && _scrollTo && g_lingo->_state->callstack.size() != _state->_dbg._callstackSize) {
				ImGui::SetScrollHereY(0.5f);
				_state->_dbg._scrollToPC = false;
			}
			dl->AddRectFilled(ImVec2(pos.x + 16.f, pos.y), ImVec2(pos.x + width, pos.y + 16.f), ImColor(IM_COL32(0xFF, 0xFF, 0x00, 0x20)), 0.4f);
		}
		// draw separator
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), ImColor(_state->_colors._line_color));

		ImGui::SetItemTooltip("Click to add a breakpoint");
		ImGui::SameLine();

		// draw offset
		ImGui::Text("[%5d] ", pc);
		ImGui::SameLine();
	}

	void renderIndentation(int indent) const {
		for (int i = 0; i < indent; i++) {
			ImGui::Text("  ");
			ImGui::SameLine();
		}
	}

	void renderIndentation() const {
		renderIndentation(_indent);
	}

	void indent() {
		_indent++;
	}

	void unindent() {
		if (_indent > 0)
			_indent--;
	}

	static Common::String posToString(int32 pos) {
		return Common::String::format("[%3d]", pos);
	}

private:
	ImGuiScript &_script;
	bool _showByteCode = false;
	bool _dot = false;
	int _indent = 0;
	bool _currentStatementDisplayed = false;
	bool _isScriptInDebug = false;
	int _renderLineID = 1;
	bool _scrollTo = false;
};

void renderScriptAST(ImGuiScript &script, bool showByteCode, bool scrollTo) {
	RenderScriptVisitor visitor(script, showByteCode, scrollTo);
	script.root->accept(visitor);
}


} // namespace DT
} // namespace Director
