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

#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

namespace Director {
namespace DT {

class RenderOldScriptVisitor : public NodeVisitor {
private:
	ImGuiScript &_script;
	int _indent = 0;
	bool _isScriptInDebug = false;
	bool _currentStatementDisplayed = false;
	bool _scrollTo = false;
	bool _scrollDone = true;

public:
	explicit RenderOldScriptVisitor(ImGuiScript &script, bool scrollTo) : _script(script), _scrollTo(scrollTo) {
		Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
		if (!callstack.empty()) {
			CFrame *head = callstack[callstack.size() - 1];
			_isScriptInDebug = (head->sp.ctx->_id == script.id.member) && (*head->sp.name == script.handlerId);
		}
		_script.startOffsets.clear();
	}

	virtual bool visitHandlerNode(HandlerNode *node) {
		ImGui::Text("on ");
		ImGui::SameLine();
		ImGui::TextColored(_state->_colors._call_color, "%s", node->name->c_str());
		if (!node->args->empty()) {
			ImGui::SameLine();
			ImGui::Text(" ");
			ImGui::SameLine();
			for (uint i = 0; i < node->args->size(); i++) {
				Common::String *arg = (*node->args)[i];
				ImGui::Text("%s", arg->c_str());
				ImGui::SameLine();
				if (i != (node->args->size() - 1)) {
					ImGui::Text(", ");
					ImGui::SameLine();
				}
			}
			ImGui::NewLine();
		}
		if (_state->_dbg._goToDefinition && _scrollTo) {
			ImGui::SetScrollHereY(0.5f);
			_state->_dbg._goToDefinition = false;
		}

		indent();
		for (uint i = 0; i < node->stmts->size(); i++) {
			Node *stmt = (*node->stmts)[i];
			renderLine(stmt->startOffset);
			stmt->accept(this);
			ImGui::NewLine();
		}
		unindent();
		renderLine(node->endOffset);
		ImGui::TextColored(_state->_colors._keyword_color, "end");
		return true;
	}

	virtual bool visitScriptNode(ScriptNode *node) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2());
		for (Node *child : *node->children) {
			if (child->type == kHandlerNode && *((HandlerNode *)child)->name != _script.handlerId)
				continue;
			renderLine(child->startOffset);
			child->accept(this);
		}
		ImGui::PopStyleVar();
		return true;
	}

	virtual bool visitFactoryNode(FactoryNode *node) {
		ImGui::Text("factory %s", node->name->c_str());
		ImGui::NewLine();
		indent();
		for (uint i = 0; i < node->methods->size(); i++) {
			Node *method = (*node->methods)[i];
			renderLine(method->startOffset);
			method->accept(this);
			ImGui::NewLine();
		}
		unindent();
		return true;
	}

	virtual bool visitCmdNode(CmdNode *node) {
		ImGui::Text("%s ", node->name->c_str());
		ImGui::SameLine();
		if (*node->name == "go") {
			ImGui::TextColored(_state->_colors._keyword_color, "to ");
			ImGui::SameLine();
		}
		for (uint i = 0; i < node->args->size(); i++) {
			Node *arg = (*node->args)[i];
			if ((i != 0) || (node->args->size() < 2) || ((*node->args)[1]->type != kMovieNode)) {
				arg->accept(this);
				ImGui::SameLine();
				if (i != (node->args->size() - 1)) {
					ImGui::Text(", ");
					ImGui::SameLine();
				}
			}
		}
		return true;
	}

	virtual bool visitPutIntoNode(PutIntoNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "put ");
		ImGui::SameLine();
		node->val->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " into ");
		ImGui::SameLine();
		node->var->accept(this);
		return true;
	}

	virtual bool visitPutAfterNode(PutAfterNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "put ");
		ImGui::SameLine();
		node->val->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " after ");
		ImGui::SameLine();
		node->var->accept(this);
		return true;
	}

	virtual bool visitPutBeforeNode(PutBeforeNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "put ");
		ImGui::SameLine();
		node->val->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " before ");
		ImGui::SameLine();
		node->var->accept(this);
		return true;
	}

	virtual bool visitSetNode(SetNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "set ");
		ImGui::SameLine();
		node->val->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " to ");
		ImGui::SameLine();
		node->var->accept(this);
		return true;
	}

	void displayDefineVar(const char *name, IDList *names) {
		ImGui::Text("%s ", name);
		ImGui::SameLine();
		for (uint i = 0; i < names->size(); i++) {
			Common::String *arg = (*names)[i];
			ImGui::Text("%s", arg->c_str());
			ImGui::SameLine();
			if (i != (names->size() - 1)) {
				ImGui::Text(" ");
				ImGui::SameLine();
			}
		}
	}

	virtual bool visitGlobalNode(GlobalNode *node) {
		displayDefineVar("global", node->names);
		return true;
	}

	virtual bool visitPropertyNode(PropertyNode *node) {
		displayDefineVar("property", node->names);
		return true;
	}

	virtual bool visitInstanceNode(InstanceNode *node) {
		displayDefineVar("instance", node->names);
		return true;
	}

	virtual bool visitIfStmtNode(IfStmtNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "if ");
		ImGui::SameLine();
		node->cond->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " then ");
		if (node->stmts->size() == 1) {
			ImGui::SameLine();
			(*node->stmts)[0]->accept(this);
		} else {
			indent();
			for (uint i = 0; i < node->stmts->size(); i++) {
				Node *stmt = (*node->stmts)[i];
				renderLine(stmt->startOffset);
				stmt->accept(this);
				ImGui::NewLine();
			}
			unindent();
			renderLine(node->endOffset);
			ImGui::TextColored(_state->_colors._keyword_color, "endif");
			ImGui::SameLine();
		}
		return true;
	}

	virtual bool visitIfElseStmtNode(IfElseStmtNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "if ");
		ImGui::SameLine();
		node->cond->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " then ");
		if (node->stmts1->size() == 1) {
			ImGui::SameLine();
			(*node->stmts1)[0]->accept(this);
			ImGui::Text(" ");
			ImGui::SameLine();
		} else {
			uint offset = node->cond->endOffset;
			indent();
			for (uint i = 0; i < node->stmts1->size(); i++) {
				Node *stmt = (*node->stmts1)[i];
				renderLine(stmt->startOffset);
				stmt->accept(this);
				ImGui::NewLine();
				offset = stmt->endOffset;
			}
			unindent();
			renderLine(offset);
		}
		ImGui::TextColored(_state->_colors._keyword_color, "else ");
		if (node->stmts2->size() == 1) {
			ImGui::SameLine();
			(*node->stmts2)[0]->accept(this);
		} else {
			uint offset = node->cond->endOffset;
			indent();
			for (uint i = 0; i < node->stmts2->size(); i++) {
				Node *stmt = (*node->stmts2)[i];
				renderLine(stmt->startOffset);
				stmt->accept(this);
				ImGui::NewLine();
				offset = stmt->endOffset;
			}
			unindent();
			renderLine(offset);
			ImGui::TextColored(_state->_colors._keyword_color, "endif");
			ImGui::SameLine();
		}
		return true;
	}

	virtual bool visitRepeatWhileNode(RepeatWhileNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "repeat while ");
		ImGui::SameLine();
		node->cond->accept(this);
		ImGui::NewLine();
		indent();
		uint offset = node->cond->endOffset;
		for (uint i = 0; i < node->stmts->size(); i++) {
			Node *stmt = (*node->stmts)[i];
			renderLine(stmt->startOffset);
			stmt->accept(this);
			ImGui::NewLine();
			offset = stmt->endOffset;
		}
		unindent();
		renderLine(offset);
		ImGui::TextColored(_state->_colors._keyword_color, "endrepeat");
		return true;
	}

	virtual bool visitRepeatWithToNode(RepeatWithToNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "repeat with ");
		ImGui::SameLine();
		ImGui::Text("%s = ", node->var->c_str());
		ImGui::SameLine();
		node->start->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, " %s ", node->down ? "down to" : "to");
		node->end->accept(this);
		ImGui::NewLine();
		indent();
		for (uint i = 0; i < node->stmts->size(); i++) {
			Node *stmt = (*node->stmts)[i];
			renderLine(stmt->startOffset);
			stmt->accept(this);
			ImGui::NewLine();
		}
		unindent();
		renderLine(node->endOffset);
		ImGui::TextColored(_state->_colors._keyword_color, "endrepeat");
		return true;
	}

	virtual bool visitRepeatWithInNode(RepeatWithInNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "repeat with ");
		ImGui::SameLine();
		ImGui::Text("%s in ", node->var->c_str());
		ImGui::SameLine();
		node->list->accept(this);
		ImGui::NewLine();
		indent();
		for (uint i = 0; i < node->stmts->size(); i++) {
			Node *stmt = (*node->stmts)[i];
			renderLine(stmt->startOffset);
			stmt->accept(this);
			ImGui::NewLine();
		}
		unindent();
		renderLine(node->endOffset);
		ImGui::TextColored(_state->_colors._keyword_color, "endrepeat");
		return true;
	}

	virtual bool visitNextRepeatNode(NextRepeatNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "next repeat");
		return true;
	}

	virtual bool visitExitRepeatNode(ExitRepeatNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "exit repeat");
		return true;
	}

	virtual bool visitExitNode(ExitNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "exit");
		return true;
	}

	virtual bool visitReturnNode(ReturnNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "return");
		if (node->expr) {
			ImGui::Text(" ");
			ImGui::SameLine();
			node->expr->accept(this);
			ImGui::NewLine();
		}
		return true;
	}

	virtual bool visitTellNode(TellNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "tell ");
		node->target->accept(this);
		if (node->stmts->size() == 1) {
			ImGui::SameLine();
			ImGui::TextColored(_state->_colors._keyword_color, " to ");
			ImGui::SameLine();
			(*node->stmts)[0]->accept(this);
		} else {
			indent();
			for (uint i = 0; i < node->stmts->size(); i++) {
				Node *stmt = (*node->stmts)[i];
				renderLine(stmt->startOffset);
				stmt->accept(this);
				ImGui::NewLine();
			}
			unindent();
			renderLine(node->endOffset);
			ImGui::TextColored(_state->_colors._keyword_color, "endtell");
		}
		return true;
	}

	virtual bool visitWhenNode(WhenNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "when ");
		ImGui::SameLine();
		ImGui::Text("%s", node->event->c_str());
		ImGui::SameLine();
		ImGui::TextColored(_state->_colors._keyword_color, " then ");
		ImGui::SameLine();
		ImGui::Text("%s", node->code->c_str());
		ImGui::SameLine();
		return true;
	}

	virtual bool visitDeleteNode(DeleteNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "delete ");
		ImGui::SameLine();
		node->chunk->accept(this);
		return true;
	}

	virtual bool visitHiliteNode(HiliteNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "hilite ");
		ImGui::SameLine();
		node->chunk->accept(this);
		return true;
	}

	virtual bool visitAssertErrorNode(AssertErrorNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "scummvmAssertError ");
		ImGui::SameLine();
		node->stmt->accept(this);
		return true;
	}

	virtual bool visitIntNode(IntNode *node) {
		ImGui::TextColored(_state->_colors._literal_color, "%d", node->val);
		ImGui::SameLine();
		return true;
	}

	virtual bool visitFloatNode(FloatNode *node) {
		ImGui::TextColored(_state->_colors._literal_color, "%g", node->val);
		ImGui::SameLine();
		return true;
	}

	virtual bool visitSymbolNode(SymbolNode *node) {
		ImGui::TextColored(_state->_colors._literal_color, "%s", node->val->c_str());
		ImGui::SameLine();
		return true;
	}

	virtual bool visitStringNode(StringNode *node) {
		ImGui::TextColored(_state->_colors._literal_color, "\"%s\"", node->val->c_str());
		ImGui::SameLine();
		return true;
	}

	virtual bool visitListNode(ListNode *node) {
		ImGui::Text("[");
		ImGui::SameLine();
		for (uint i = 0; i < node->items->size(); i++) {
			Node *prop = (*node->items)[i];
			prop->accept(this);
			if (i != (node->items->size() - 1)) {
				ImGui::Text(",");
				ImGui::SameLine();
			}
		}
		ImGui::Text("]");
		ImGui::SameLine();
		return true;
	}

	virtual bool visitPropListNode(PropListNode *node) {
		ImGui::Text("[");
		ImGui::SameLine();
		if (node->items->empty()) {
			ImGui::Text(":");
			ImGui::SameLine();
		} else {
			for (uint i = 0; i < node->items->size(); i++) {
				Node *prop = (*node->items)[i];
				prop->accept(this);
				if (i != (node->items->size() - 1)) {
					ImGui::Text(",");
					ImGui::SameLine();
				}
			}
		}
		ImGui::Text("]");
		ImGui::SameLine();
		return true;
	}

	virtual bool visitPropPairNode(PropPairNode *node) {
		node->key->accept(this);
		ImGui::Text(":");
		ImGui::SameLine();
		node->val->accept(this);
		return true;
	}

	virtual bool visitFuncNode(FuncNode *node) {
		const bool isBuiltin = g_lingo->_builtinCmds.contains(*node->name);
		const ImVec4 color = (ImVec4)ImColor(isBuiltin ? _state->_colors._builtin_color : _state->_colors._call_color);
		ImGui::TextColored(color, "%s(", node->name->c_str());
		if (!isBuiltin && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
			ImGui::Text("Go to definition");
			ImGui::EndTooltip();
		}
		if (!isBuiltin && ImGui::IsItemClicked()) {
			int obj = 0;
			for (uint i = 0; i < _script.bytecodeArray.size(); i++) {
				if (node->startOffset == _script.bytecodeArray[i].pos) {
					obj = _script.bytecodeArray[i].obj;
					break;
				}
			}

			ScriptContext *context = getScriptContext(obj, _script.id, *node->name);
			if (context) {
				ImGuiScript script = toImGuiScript(_script.type, CastMemberID(context->_id, _script.id.castLib), *node->name);
				const Director::Movie *movie = g_director->getCurrentMovie();

				int castId = context->_id;
				bool childScript = false;
				if (castId == -1) {
					castId = movie->getCast()->getCastIdByScriptId(context->_parentNumber);
					childScript = true;
				}
				script.byteOffsets = context->_functionByteOffsets[script.handlerId];
				script.moviePath = _script.moviePath;

				script.handlerName = formatHandlerName(context->_scriptId, castId, script.handlerId, context->_scriptType, childScript);
				setScriptToDisplay(script);
				_state->_dbg._goToDefinition = true;
			}
		}
		ImGui::SameLine();
		for (uint i = 0; i < node->args->size(); i++) {
			Node *arg = (*node->args)[i];
			arg->accept(this);
			if (i != (node->args->size() - 1)) {
				ImGui::Text(",");
				ImGui::SameLine();
			}
		}
		ImGui::Text(")");
		ImGui::SameLine();
		return true;
	}

	virtual bool visitVarNode(VarNode *node) {
		ImGui::TextColored(_state->_colors._var_color, "%s", node->name->c_str());
		if (ImGui::IsItemHovered() && g_lingo->_globalvars.contains(*node->name)) {
			const Datum &val = g_lingo->_globalvars.getVal(*node->name);
			ImGui::BeginTooltip();
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
			_state->_variables[*node->name] = true;
		}
		ImGui::SameLine();
		return true;
	}

	virtual bool visitParensNode(ParensNode *node) {
		ImGui::Text("(");
		ImGui::SameLine();
		node->expr->accept(this);
		ImGui::Text(")");
		ImGui::SameLine();
		return true;
	}

	virtual bool visitUnaryOpNode(UnaryOpNode *node) {
		char op = '?';
		if (node->op == LC::c_negate) {
			op = '-';
		} else if (node->op == LC::c_not) {
			op = '!';
		}
		ImGui::Text("%c", op);
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitBinaryOpNode(BinaryOpNode *node) {
		node->a->accept(this);
		static struct {
			inst op;
			const char *name;
		} ops[] = {
			{LC::c_add, "+"},
			{LC::c_sub, "-"},
			{LC::c_mul, "*"},
			{LC::c_div, "/"},
			{LC::c_mod, "mod"},
			{LC::c_gt, ">"},
			{LC::c_lt, "<"},
			{LC::c_eq, "="},
			{LC::c_neq, "<>"},
			{LC::c_ge, ">="},
			{LC::c_le, "<="},
			{LC::c_and, "and"},
			{LC::c_or, "or"},
			{LC::c_ampersand, "&"},
			{LC::c_concat, "&&"},
			{LC::c_contains, "contains"},
			{LC::c_starts, "starts"},
		};
		for (auto &op : ops) {
			if (op.op == node->op) {
				ImGui::Text(" %s ", op.name);
				ImGui::SameLine();
				break;
			}
		}
		node->b->accept(this);
		return true;
	}

	virtual bool visitFrameNode(FrameNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "frame ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitMovieNode(MovieNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "movie ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitIntersectsNode(IntersectsNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "sprite ");
		ImGui::SameLine();
		node->sprite1->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, "intersects ");
		node->sprite2->accept(this);
		return true;
	}

	virtual bool visitWithinNode(WithinNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "sprite ");
		ImGui::SameLine();
		node->sprite1->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, "within ");
		node->sprite2->accept(this);
		return true;
	}

	virtual bool visitTheNode(TheNode *node) {
		ImGui::TextColored(_state->_colors._the_color, "the %s", node->prop->c_str());
		ImGui::SameLine();
		return true;
	}

	virtual bool visitTheOfNode(TheOfNode *node) {
		ImGui::TextColored(_state->_colors._the_color, "the %s of ", node->prop->c_str());
		ImGui::SameLine();
		node->obj->accept(this);
		return true;
	}

	virtual bool visitTheNumberOfNode(TheNumberOfNode *node) {
		ImGui::TextColored(_state->_colors._the_color, "the number of ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitTheLastNode(TheLastNode *node) {
		// TODO: change the node to know if it's 'in' or 'of'
		ImGui::TextColored(_state->_colors._the_color, "the last %s in/of ", toString(node->type).c_str());
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitTheDateTimeNode(TheDateTimeNode *node) {
		const char *key1 = "";
		switch (node->field) {
		case kTheAbbr:
			key1 = "abbreviated";
			break;
		case kTheLong:
			key1 = "long";
			break;
		case kTheShort:
			key1 = "short";
			break;
		}
		const char *key2 = node->entity == kTheDate ? "date" : "time";
		ImGui::TextColored(_state->_colors._the_color, "the %s %s", key1, key2);
		ImGui::SameLine();
		return true;
	}

	virtual bool visitMenuNode(MenuNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "menu ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitMenuItemNode(MenuItemNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "menuitem ");
		ImGui::SameLine();
		node->arg1->accept(this);
		ImGui::TextColored(_state->_colors._keyword_color, "of menu ");
		ImGui::SameLine();
		node->arg2->accept(this);
		return true;
	}

	virtual bool visitSoundNode(SoundNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "sound ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitSpriteNode(SpriteNode *node) {
		ImGui::TextColored(_state->_colors._keyword_color, "sprite ");
		ImGui::SameLine();
		node->arg->accept(this);
		return true;
	}

	virtual bool visitChunkExprNode(ChunkExprNode *node) {
		const char *key1 = "";
		switch (node->type) {
		case kChunkChar:
			key1 = "char";
			break;
		case kChunkWord:
			key1 = "word";
			break;
		case kChunkItem:
			key1 = "item";
			break;
		case kChunkLine:
			key1 = "line";
			break;
		}
		ImGui::Text("%s", key1);
		ImGui::SameLine();
		node->start->accept(this);
		if (node->end) {
			ImGui::TextColored(_state->_colors._keyword_color, " to ");
			ImGui::SameLine();
			node->end->accept(this);
		}
		ImGui::TextColored(_state->_colors._keyword_color, " of ");
		ImGui::SameLine();
		node->src->accept(this);
		return true;
	}

private:
	static Common::String toString(ChunkType chunkType) {
		// TODO: this method could be used in ChunkExprNode
		switch (chunkType) {
		case kChunkChar:
			return "char";
		case kChunkWord:
			return "word";
		case kChunkItem:
			return "item";
		case kChunkLine:
			return "line";
		}
		return "<unknown>";
	}

	void indent() {
		_indent++;
	}

	void unindent() {
		if (_indent > 0)
			_indent--;
	}

	void renderIndentation() const {
		for (int i = 0; i < _indent; i++) {
			ImGui::Text("  ");
			ImGui::SameLine();
		}
	}

	void renderLine(uint32 pc) {
		bool showCurrentStatement = false;
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

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));

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
			if (!_scrollDone && _scrollTo && g_lingo->_state->callstack.size() != _state->_dbg._callstackSize) {
				ImGui::SetScrollHereY(0.5f);
				_scrollDone = true;
			}
			dl->AddRectFilled(ImVec2(pos.x + 16.f, pos.y), ImVec2(pos.x + width, pos.y + 16.f), ImColor(IM_COL32(0xFF, 0xFF, 0x00, 0x20)), 0.4f);
		}
		// draw separator
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), ImColor(_state->_colors._line_color));

		ImGui::SetItemTooltip("Click to add a breakpoint");
		ImGui::SameLine();

		// draw offset
		ImGui::Text("[%5d] ", pc == 0xFFFFFFFF ? -1 : pc);
		ImGui::SameLine();
		renderIndentation();
	}
};

void renderOldScriptAST(ImGuiScript &script, bool showByteCode, bool scrollTo) {
    RenderOldScriptVisitor oldVisitor(script, scrollTo);
	script.oldAst->accept(&oldVisitor);
}

} // namespace DT

} // namespace Director
