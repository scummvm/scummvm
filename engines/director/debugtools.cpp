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

#define IMGUI_DEFINE_MATH_OPERATORS

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/opengl/shader.h"
#include "image/png.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingodec/ast.h"
#include "director/lingo/lingodec/codewritervisitor.h"
#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/handler.h"
#include "director/lingo/lingodec/names.h"
#include "director/lingo/lingodec/resolver.h"
#include "director/lingo/lingodec/script.h"
#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/castmember.h"
#include "director/castmember/text.h"
#include "director/castmember/script.h"
#include "director/channel.h"
#include "director/debugtools.h"
#include "director/debugger.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/types.h"

namespace Director {

#define kMaxColumnsInTable 512

typedef struct ImGuiImage {
	ImTextureID id;
	int16 width;
	int16 height;
} ImGuiImage;

typedef struct ImGuiScriptCodeLine {
	uint32 pc;
	Common::String codeLine;
} ImGuiScriptCodeLine;

typedef struct ImGuiScript {
	bool score = false;
	CastMemberID id;
	ScriptType type;
	Common::String handlerId;
	Common::String handlerName;
	Common::String moviePath;

	bool isMethod = false;
	bool isGenericEvent = false;
	Common::StringArray argumentNames;
	Common::StringArray propertyNames;
	Common::StringArray globalNames;
	Common::SharedPtr<LingoDec::HandlerNode> root;

	bool operator==(const ImGuiScript &c) const {
		return moviePath == c.moviePath && score == c.score && id == c.id && handlerId == c.handlerId;
	}
	bool operator!=(const ImGuiScript &c) const {
		return !(*this == c);
	}
} ImGuiScript;

typedef struct ImGuiWindows {
	bool controlPanel = true;
	bool callStack = false;
	bool vars = false;
	bool channels = false;
	bool cast = false;
	bool funcList = false;
	bool score = false;
	bool bpList = false;
	bool settings = false;
	bool logger = false;
} ImGuiWindows;

static bool toggleButton(const char *label, bool *p_value, bool inverse = false) {
	int pop = 0;
	if (*p_value != inverse) {
		ImVec4 hovered = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_Button, hovered);
		pop = 1;
	}
	bool result = ImGui::Button(label);
	if (result) {
		*p_value = !*p_value;
	}
	ImGui::PopStyleColor(pop);
	return result;
}

struct ImGuiLogger {
	char _inputBuf[256];
	ImVector<char *> _items;
	ImVector<char *> _history;
	int _historyPos; // -1: new line, 0.._history.Size-1 browsing history.
	ImGuiTextFilter _filter;
	bool _autoScroll;
	bool _scrollToBottom;
	bool _showError = true;
	bool _showWarn = true;
	bool _showInfo = true;
	bool _showdebug = true;

	ImGuiLogger() {
		clear();
		memset(_inputBuf, 0, sizeof(_inputBuf));
		_historyPos = -1;
		_autoScroll = true;
		_scrollToBottom = false;
	}

	~ImGuiLogger() {
		clear();
		for (int i = 0; i < _history.Size; i++)
			free(_history[i]);
	}

	void clear() {
		for (int i = 0; i < _items.Size; i++)
			free(_items[i]);
		_items.clear();
	}

	void addLog(const char *fmt, ...) IM_FMTARGS(2) {
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		_items.push_back(Strdup(buf));
	}

	void draw(const char *title, bool *p_open) {
		if (!*p_open)
			return;

		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title, p_open)) {
			ImGui::End();
			return;
		}

		// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
		// Here we create a context menu only available from the title bar.
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Close ImGuiLogger"))
				*p_open = false;
			ImGui::EndPopup();
		}

		// Clear
		if (ImGui::Button("\ue0b8")) {
			clear();
		}
		ImGui::SetItemTooltip("Clear");
		ImGui::SameLine();

		// Copy
		bool copy_to_clipboard = ImGui::Button("\ue14d");
		ImGui::SetItemTooltip("Copy to clipboard");
		ImGui::SameLine();

		// debug channels
		int numChannels = 0;
		auto channels = DebugMan.getDebugChannels();
		for (auto &channel : channels) {
			if (channel.name == "imgui")
				continue;
			bool enabled = DebugMan.isDebugChannelEnabled(channel.channel);
			if (enabled)
				numChannels++;
		}

		Common::String selChannels(Common::String::format("(%d channel%s)", numChannels, numChannels > 1 ? "s" : ""));
		ImGui::PushItemWidth(120);
		if (ImGui::BeginCombo("##Channels", selChannels.c_str())) {
			for (auto &channel : channels) {
				if (channel.name == "imgui")
					continue;
				bool enabled = DebugMan.isDebugChannelEnabled(channel.channel);
				if (ImGui::Checkbox(channel.name.c_str(), &enabled)) {
					if (enabled) {
						DebugMan.enableDebugChannel(channel.channel);
					} else {
						DebugMan.disableDebugChannel(channel.channel);
					}
				}
				ImGui::SetItemTooltip("%s", channel.description.c_str());
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();

		// Options menu
		if (ImGui::BeginPopup("Options")) {
			if (ImGui::InputInt("Debug Level", &gDebugLevel)) {
				if (gDebugLevel < 0)
					gDebugLevel = 0;
			}
			ImGui::Separator();
			ImGui::Checkbox("Auto-scroll", &_autoScroll);
			ImGui::EndPopup();
		}

		// Options, Filter
		if (ImGui::Button("\ue8b8"))
			ImGui::OpenPopup("Options");
		ImGui::SetItemTooltip("Options");
		ImGui::SameLine();

		ImGui::Spacing();
		ImGui::SameLine();

		// Error
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
		toggleButton("\ue160", &_showError);
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("Show Errors");
		ImGui::SameLine();

		// Warning
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
		toggleButton("\ue002", &_showWarn);
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("Show Warnings");
		ImGui::SameLine();

		// Info
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
		toggleButton("\ue88e", &_showInfo);
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("Show Info");
		ImGui::SameLine();

		// Debug
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.f));
		toggleButton("\ue868", &_showdebug);
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("Show Debug");
		ImGui::SameLine();

		_filter.Draw("Filter (\"incl,-excl\") (\"warn\")", 180);
		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::BeginPopupContextWindow()) {
			if (ImGui::Selectable("\ue0b8 Clear"))
				clear();
			if (ImGui::Selectable("\ue14d Copy"))
				copy_to_clipboard = true;
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		if (copy_to_clipboard)
			ImGui::LogToClipboard();
		for (int i = 0; i < _items.Size; i++) {
			const char *item = _items[i];
			bool isError = strstr(item, "[error]");
			if (!_showError && isError)
				continue;

			bool isWarn = strstr(item, "[warn]");
			if (!_showWarn && isWarn)
				continue;

			bool isDebug = strstr(item, "[debug]");
			if (!_showdebug && isDebug)
				continue;

			if (!_showInfo && !isError && !isWarn && !isDebug)
				continue;

			if (!_filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make _items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			if (isError) {
				item += 7;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				pop_color = true;
			} else if (isWarn) {
				item += 6;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
				pop_color = true;
			} else if (isDebug) {
				item += 7;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
				pop_color = true;
			} else if (strncmp(item, "> ", 2) == 0) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
				pop_color = true;
			}
			ImGui::TextUnformatted(item);
			if (pop_color)
				ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
			ImGui::LogFinish();

		if (_scrollToBottom || (_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		_scrollToBottom = false;

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}

private:
	static char *Strdup(const char *str) {
		size_t len = strlen(str) + 1;
		void *buf = malloc(len);
		IM_ASSERT(buf);
		return (char *)memcpy(buf, (const void *)str, len);
	}
};

typedef struct ImGuiState {
	struct {
		Common::HashMap<Graphics::Surface *, ImGuiImage> _textures;
		bool _listView = true;
		int _thumbnailSize = 64;
		ImGuiTextFilter _nameFilter;
		int _typeFilter = 0x7FFF;
	} _cast;
	struct {
		Common::Array<ImGuiScript> _scripts;
		uint _current = 0;
		ImGuiTextFilter _nameFilter;
		bool _showByteCode = false;
		bool _showScript = false;
	} _functions;

	struct {
		ImVec4 _bp_color_disabled = ImVec4(0.9f, 0.08f, 0.0f, 0.0f);
		ImVec4 _bp_color_enabled = ImVec4(0.9f, 0.08f, 0.0f, 1.0f);
		ImVec4 _bp_color_hover = ImVec4(0.42f, 0.17f, 0.13f, 1.0f);
		ImVec4 _line_color = ImVec4(0.44f, 0.44f, 0.44f, 1.0f);
		ImVec4 _call_color = ImColor(IM_COL32(0xFF, 0xC5, 0x5C, 0xFF));
		ImVec4 _builtin_color = ImColor(IM_COL32(0x60, 0x7C, 0xFF, 0xFF));
		ImVec4 _var_color = ImColor(IM_COL32(0x4B, 0xCD, 0x5E, 0xFF));
		ImVec4 _literal_color = ImColor(IM_COL32(0xFF, 0x9F, 0xDA, 0x9E));
		ImVec4 _comment_color = ImColor(IM_COL32(0xFF, 0xA5, 0x9D, 0x95));
		ImVec4 _type_color = ImColor(IM_COL32(0x13, 0xC5, 0xF9, 0xFF));
		ImVec4 _keyword_color = ImColor(IM_COL32(0xC1, 0xC1, 0xC1, 0xFF));
		ImVec4 _the_color = ImColor(IM_COL32(0xFF, 0x49, 0xEF, 0xFF));
	} _colors;

	struct {
		DatumHash _locals;
		DatumHash _globals;
		uint32 _lastTimeRefreshed = 0;
	} _vars;

	ImGuiWindows _w;
	ImGuiWindows _savedW;
	bool _wasHidden = false;

	Common::List<CastMemberID> _scriptCasts;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	int _prevFrame = -1;
	struct {
		int frame = -1;
		int channel = -1;
	} _selectedScoreCast;

	int _scoreMode = 0;
	int _scoreFrameOffset = 1;

	ImFont *_tinyFont = nullptr;

	ImGuiLogger _logger;
} ImGuiState;

ImGuiState *_state = nullptr;

const LingoDec::Handler *getHandler(CastMemberID id, const Common::String &handlerId) {
	Director::Movie *movie = g_director->getCurrentMovie();
	const Director::Cast *cast = movie->getCast(id);
	if (!cast->_lingodec)
		return nullptr;

	Common::SharedPtr<LingoDec::Node> node;
	for (auto it : cast->_lingodec->scripts) {
		for (const LingoDec::Handler &h : it.second->handlers) {
			if (h.name != handlerId)
				continue;
			return &h;
		}
	}
	return nullptr;
}

ImGuiScript toImGuiScript(CastMemberID id, const Common::String &handlerId) {
	ImGuiScript result;
	result.id = id;
	result.handlerId = handlerId;

	const LingoDec::Handler *handler = getHandler(id, handlerId);
	if (!handler)
		return result;

	result.root = handler->ast.root;
	result.isGenericEvent = handler->isGenericEvent;
	result.argumentNames = handler->argumentNames;
	result.propertyNames = handler->script->propertyNames;
	result.globalNames = handler->globalNames;

	LingoDec::Script *script = handler->script;
	if (!script)
		return result;

	result.isMethod = script->isFactory();
	return result;
}

static void setScriptToDisplay(const ImGuiScript &script);

static Director::Breakpoint *getBreakpoint(const Common::String &handlerName, int pc) {
	auto &bps = g_lingo->getBreakpoints();
	for (uint i = 0; i < bps.size(); i++) {
		if (bps[i].type == kBreakpointFunction && bps[i].funcName == handlerName && (int)bps[i].funcOffset == pc) {
			return &bps[i];
		}
	}
	return nullptr;
}

class RenderScriptVisitor : public LingoDec::NodeVisitor {
public:
	explicit RenderScriptVisitor(ImGuiScript &script, bool showByteCode) : _script(script), _showByteCode(showByteCode) {}

	virtual void visit(const LingoDec::HandlerNode &node) override {
		_handler = node.handler;

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
		int32 obj = 0;
		for (uint i = 0; i < _handler->bytecodeArray.size(); i++) {
			if (node._startOffset == _handler->bytecodeArray[i].pos) {
				obj = _handler->bytecodeArray[i].obj;
				break;
			}
		}

		// new line only if it's a statement
		if (node.isStatement) {
			renderLine(node._startOffset);
			renderIndentation();
		}

		const ImVec4 color = (ImVec4)ImColor(g_lingo->_builtinCmds.contains(node.name) ? _state->_colors._builtin_color : _state->_colors._call_color);
		ImGui::TextColored(color, "%s", node.name.c_str());
		if (!g_lingo->_builtinCmds.contains(node.name) && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
			ImGui::Text("Go to definition");
			ImGui::EndTooltip();
		}
		if (!g_lingo->_builtinCmds.contains(node.name) && ImGui::IsItemClicked()) {
			ImGuiScript script = toImGuiScript(CastMemberID(obj, _script.id.castLib), node.name);
			script.moviePath = _script.moviePath;
			script.handlerName = node.name;
			setScriptToDisplay(script);
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
		ImGui::TextColored(ImColor(_state->_colors._keyword_color), "end case");
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
			ImGui::Text("Click to add to watches.");
			ImGui::Text("= %s", val.asString(true).c_str());
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
				ImGui::TextColored(_state->_colors._var_color, "%s", _script.argumentNames[i].c_str());
				ImGui::SameLine();
			}
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
				ImGui::TextColored(_state->_colors._var_color, "%s", _script.globalNames[i].c_str());
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

	void byteCode(const LingoDec::HandlerNode &node) const {
		LingoDec::Handler *handler = node.handler;
		bool isMethod = handler->script->isFactory();

		if (!handler->isGenericEvent) {
			Common::String code;
			if (isMethod) {
				code += "method ";
			} else {
				code += "on ";
			}
			code += handler->name;
			if (handler->argumentNames.size() > 0) {
				code += " ";
				for (size_t i = 0; i < handler->argumentNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += handler->argumentNames[i];
				}
			}
			writeByteCode(0, code);
		}
		for (uint i = 0; i < handler->bytecodeArray.size(); i++) {
			LingoDec::CodeWriterVisitor code(_dot, true);
			code.indent();
			auto &bytecode = handler->bytecodeArray[i];
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
		if (!handler->isGenericEvent) {
			if (!isMethod) {
				writeByteCode(node._endOffset, "end");
			}
		}
	}

	void write(uint32 offset, const Common::String &code, ImVec4 color = ImVec4(1, 1, 1, 1)) const {
		renderLine(offset);
		renderIndentation();
		ImGui::TextColored(color, "%s", code.c_str());
	}

	void writeByteCode(uint32 offset, const Common::String &code) const {
		renderLine(offset);
		Common::String s;
		for (int i = 0; i < _indent; i++) {
			s += "  ";
		}
		ImGui::Text("%s", (s + code).c_str());
	}

	void renderLine(uint pc) const {
		ImDrawList *dl = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);

		ImVec4 color = _state->_colors._bp_color_disabled;

		Director::Breakpoint *bp = getBreakpoint(_script.handlerName, pc);
		if (bp)
			color = _state->_colors._bp_color_enabled;

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		if (ImGui::IsItemClicked(0)) {
			if (color == _state->_colors._bp_color_enabled) {
				g_lingo->delBreakpoint(bp->id);
				color = _state->_colors._bp_color_disabled;
			} else {
				Director::Breakpoint newBp;
				newBp.type = kBreakpointFunction;
				newBp.funcName = _script.handlerName;
				newBp.funcOffset = pc;
				g_lingo->addBreakpoint(newBp);
				color = _state->_colors._bp_color_enabled;
			}
		}

		if (color == _state->_colors._bp_color_disabled && ImGui::IsItemHovered()) {
			color = _state->_colors._bp_color_hover;
		}

		if (!bp || bp->enabled)
			dl->AddCircleFilled(mid, 4.0f, ImColor(color));
		else
			dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._line_color));
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), ImColor(_state->_colors._line_color));

		ImGui::SetItemTooltip("Click to add a breakpoint");

		ImGui::SameLine();
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
	LingoDec::Handler *_handler = nullptr;
};

static void showControlPanel() {
	if (!_state->_w.controlPanel)
		return;

	ImVec2 vp(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImVec2(vp.x - 220.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 103), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Control Panel", &_state->_w.controlPanel)) {
		Movie *movie = g_director->getCurrentMovie();
		Score *score = movie->getScore();
		ImDrawList *dl = ImGui::GetWindowDrawList();

		ImU32 color = ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
		ImU32 color_red = ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
		ImU32 active_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
		ImU32 bgcolor = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 1.0f, 1.0f));
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImVec2 buttonSize(20, 14);
		float bgX1 = -4.0f, bgX2 = 21.0f;

		int frameNum = score->getCurrentFrameNum();

		if (_state->_prevFrame != -1 && _state->_prevFrame != frameNum) {
			score->_playState = kPlayPaused;
			_state->_prevFrame = -1;
		}

		{ // Rewind
			ImGui::InvisibleButton("Rewind", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				score->setCurrentFrame(1);
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 8, p.y), ImVec2(p.x + 8, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 8), ImVec2(p.x + 16, p.y), ImVec2(p.x + 16, p.y + 16), color);

			ImGui::SetItemTooltip("Rewind");
			ImGui::SameLine();
		}

		{ // Step Back
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Back", ImVec2(18, 16));

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum - 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 9, p.y), ImVec2(p.x + 9, p.y + 16), color);
			dl->AddRectFilled(ImVec2(p.x + 11, p.y), ImVec2(p.x + 17, p.y + 16), color);

			ImGui::SetItemTooltip("Step Back");
			ImGui::SameLine();
		}

		{ // Stop
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Stop", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayPaused;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			ImU32 stopColor = (score->_playState == kPlayPaused) ? active_color : color;
			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 16, p.y + 16), stopColor);

			ImGui::SetItemTooltip("Stop");
			ImGui::SameLine();
		}

		{ // Step
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum + 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 6, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 2), ImVec2(p.x + 8, p.y + 14), ImVec2(p.x + 16, p.y + 8), color);

			ImGui::SetItemTooltip("Step");
			ImGui::SameLine();
		}

		{ // Play
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Play", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 16), ImVec2(p.x + 14, p.y + 8), color);

			ImGui::SetItemTooltip("Play");
			ImGui::SameLine();
		}

		char buf[6];

		snprintf(buf, 6, "%d", score->getCurrentFrameNum());

		ImGui::SetNextItemWidth(35);
		ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
		ImGui::SetItemTooltip("Frame");

		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.5f, 1.0f), movie->getArchive()->getPathName().toString().c_str());
			ImGui::SetItemTooltip(movie->getArchive()->getPathName().toString().c_str());
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Text("Lingo:");
		ImGui::SameLine();
		{ // Step over
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Over", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->PathArcToFast(ImVec2(p.x + 9, p.y + 15), 10.0f, 7, 11);
			dl->PathStroke(color_red, 0, 2);
			dl->AddLine(ImVec2(p.x + 18, p.y + 5), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 14, p.y + 10), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Over");
			ImGui::SameLine();
		}

		{ // Step into
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Into", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Into");
			ImGui::SameLine();
		}

		{ // Step out
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Out", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Out");
		}
	}
	ImGui::End();
}

static void showCallStack() {
	if (!_state->_w.callStack)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("CallStack", &_state->_w.callStack)) {
		ImGui::Text("%s", lingo->formatCallStack(lingo->_state->pc).c_str());
	}
	ImGui::End();
}

static GLuint loadTextureFromSurface(Graphics::Surface *surface, const byte *palette, int palCount) {

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
	Graphics::Surface *s = surface->convertTo(Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0), palette, palCount);
	glPixelStorei(GL_UNPACK_ALIGNMENT, s->format.bytesPerPixel);

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB, GL_UNSIGNED_BYTE, s->getPixels()));
	s->free();
	delete s;
	return image_texture;
}

static const char *toString(ScriptType scriptType) {
	static const char *scriptTypes[] = {
		"Score",
		"Cast",
		"Movie",
		"Event",
		"Test",
		"???",
		"???",
		"Parent",
	};
	if (scriptType < 0 || scriptType > kMaxScriptType)
		return "???";
	return scriptTypes[(int)scriptType];
}

static const char *toIcon(CastType castType) {
	static const char *castTypes[] = {
		"",        // Empty
		"\uf79e",  // Bitmap			// backround_dot_large
		"\ue8da",  // FilmLoop		// theaters
		"\uf6f1",  // Text			// match_case
		"\ue40a",  // Palette		// palette
		"\uefa2",  // Picture		// imagesmode
		"\ue050",  // Sound			// volume_up
		"\uf4ab",  // Button			// slab_serif
		"\ue602",  // Shape			// shapes
		"\ue02c",  // Movie			// movie
		"\uf49a",  // DigitalVideo	// animated_images
		"\uf0c8",  // Script			// forms_apps_script
		"\uf4f1",  // RTE			// brand_family
		"?",       // ???
		"\uf50c"}; // Transition		// transition_fade
	if (castType < 0 || castType > kCastTransition)
		return "";
	return castTypes[(int)castType];
}

static const char *toString(CastType castType) {
	static const char *castTypes[] = {
		"Empty",
		"Bitmap",
		"FilmLoop",
		"Text",
		"Palette",
		"Picture",
		"Sound",
		"Button",
		"Shape",
		"Movie",
		"DigitalVideo",
		"Script",
		"RTE",
		"???",
		"Transition"};
	if (castType < 0 || castType > kCastTransition)
		return "???";
	return castTypes[(int)castType];
}

static ImGuiImage getImageID(CastMember *castMember) {
	if (castMember->_type != CastType::kCastBitmap)
		return {};

	BitmapCastMember *bmpMember = (BitmapCastMember *)castMember;
	Common::Rect bbox(bmpMember->getBbox());
	Graphics::Surface *bmp = bmpMember->getMatte(bbox);
	if (!bmp)
		return {};

	if (_state->_cast._textures.contains(bmp))
		return _state->_cast._textures[bmp];

	Picture *pic = bmpMember->_picture;
	if (!pic)
		return {};

	ImTextureID textureID = (ImTextureID)(intptr_t)loadTextureFromSurface(&pic->_surface, pic->_palette, pic->_paletteColors);
	_state->_cast._textures[bmp] = {textureID, pic->_surface.w, pic->_surface.h};
	return _state->_cast._textures[bmp];
}

static void setToolTipImage(const ImGuiImage &image, const char *name) {
	if (ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
		ImGui::Text("%s", name);
		ImGui::Image(image.id, ImVec2(image.width, image.height), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		ImGui::EndTooltip();
	}
}

static void showImage(const ImGuiImage &image, const char *name, float thumbnailSize) {
	ImVec2 size;
	if (image.width > image.height) {
		size = {thumbnailSize - 2, (thumbnailSize - 2) * image.height / image.width};
	} else {
		size = {(thumbnailSize - 2) * image.width / image.height, thumbnailSize - 2};
	}
	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(thumbnailSize, thumbnailSize), 0xFFFFFFFF);
	ImVec2 pos = ImGui::GetCursorPos();
	ImVec2 imgPos = pos + ImVec2(1 + (thumbnailSize - 2 - size.x) * 0.5f, 1 + (thumbnailSize - 2 - size.y) * 0.5f);
	ImGui::SetCursorPos(imgPos);
	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	setToolTipImage(image, name);
}

static Common::String getDisplayName(CastMember *castMember) {
	const CastMemberInfo *castMemberInfo = castMember->getInfo();
	Common::String name(castMemberInfo ? castMemberInfo->name : "");
	if (!name.empty())
		return name;
	if (castMember->_type == kCastText) {
		TextCastMember *textCastMember = (TextCastMember *)castMember;
		return textCastMember->getText();
	}
	return Common::String::format("%u", castMember->getID());
}

static void showCast() {
	if (!_state->_w.cast)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(520, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast", &_state->_w.cast)) {
		// display a toolbar with: grid/list/filters buttons + name filter
		toggleButton("\ue896", &_state->_cast._listView); // list
		ImGui::SetItemTooltip("List");
		ImGui::SameLine();
		toggleButton("\ue9b0", &_state->_cast._listView, true); // grid_view
		ImGui::SetItemTooltip("Grid");
		ImGui::SameLine();

		if (ImGui::Button("\uef4f")) { // filter_alt
			ImGui::OpenPopup("filters_popup");
		}
		ImGui::SameLine();

		if (ImGui::BeginPopup("filters_popup")) {
			ImGui::CheckboxFlags("All", &_state->_cast._typeFilter, 0x7FFF);
			ImGui::Separator();
			for (int i = 0; i <= 14; i++) {
				ImGui::PushID(i);
				Common::String option(Common::String::format("%s %s", toIcon((CastType)i), toString((CastType)i)));
				ImGui::CheckboxFlags(option.c_str(), &_state->_cast._typeFilter, 1 << i);
				ImGui::PopID();
			}
			ImGui::EndPopup();
		}
		_state->_cast._nameFilter.Draw();
		ImGui::Separator();

		// display a list or a grid
		const float sliderHeight = _state->_cast._listView ? 0.f : 38.f;
		const ImVec2 childsize = ImGui::GetContentRegionAvail();
		Movie *movie = g_director->getCurrentMovie();
		ImGui::BeginChild("##cast", ImVec2(childsize.x, childsize.y - sliderHeight));
		if (_state->_cast._listView) {
			if (ImGui::BeginTable("Resources", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Name", 0, 120.f);
				ImGui::TableSetupColumn("#", 0, 20.f);
				ImGui::TableSetupColumn("Script", 0, 80.f);
				ImGui::TableSetupColumn("Type", 0, 80.f);
				ImGui::TableSetupColumn("Preview", 0, 32.f);
				ImGui::TableHeadersRow();

				for (auto it : *movie->getCasts()) {
					Cast *cast = it._value;
					if (!cast->_loadedCast)
						continue;

					for (auto castMember : *cast->_loadedCast) {
						if (!castMember._value->isLoaded())
							continue;

						Common::String name(getDisplayName(castMember._value));
						if (!_state->_cast._nameFilter.PassFilter(name.c_str()))
							continue;
						if ((castMember._value->_type != kCastTypeAny) && !(_state->_cast._typeFilter & (1 << (int)castMember._value->_type)))
							continue;

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s %s", toIcon(castMember._value->_type), name.c_str());

						ImGui::TableNextColumn();
						ImGui::Text("%d", castMember._key);

						ImGui::TableNextColumn();
						if (castMember._value->_type == CastType::kCastLingoScript) {
							ScriptCastMember *scriptMember = (ScriptCastMember *)castMember._value;
							ImGui::Text("%s", toString(scriptMember->_scriptType));
						}
						ImGui::TableNextColumn();
						ImGui::Text("%s", toString(castMember._value->_type));

						ImGui::TableNextColumn();
						ImGuiImage imgID = getImageID(castMember._value);
						if (imgID.id) {
							showImage(imgID, name.c_str(), 32.f);
						}
					}
				}

				ImGui::EndTable();
			}
		} else {
			const float thumbnailSize = (float)_state->_cast._thumbnailSize;
			const float contentWidth = ImGui::GetContentRegionAvail().x;
			int columns = contentWidth / (thumbnailSize + 8.f);
			columns = columns < 1 ? 1 : columns;
			if (ImGui::BeginTable("Cast", columns)) {
				for (auto it : *movie->getCasts()) {
					const Cast *cast = it._value;
					if (!cast->_loadedCast)
						continue;

					for (auto castMember : *cast->_loadedCast) {
						if (!castMember._value->isLoaded())
							continue;

						Common::String name(getDisplayName(castMember._value));
						if (!_state->_cast._nameFilter.PassFilter(name.c_str()))
							continue;
						if ((castMember._value->_type != kCastTypeAny) && !(_state->_cast._typeFilter & (1 << (int)castMember._value->_type)))
							continue;

						ImGui::TableNextColumn();

						ImGui::BeginGroup();
						const ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
						float textWidth = textSize.x;
						float textHeight = textSize.y;
						if (textWidth > thumbnailSize) {
							textWidth = thumbnailSize;
							textHeight *= (textSize.x / textWidth);
						}

						ImGuiImage imgID = getImageID(castMember._value);
						if (imgID.id) {
							showImage(imgID, name.c_str(), thumbnailSize);
						} else {
							ImGui::PushID(castMember._key);
							ImGui::InvisibleButton("##canvas", ImVec2(thumbnailSize, thumbnailSize));
							ImGui::PopID();
							const ImVec2 p0 = ImGui::GetItemRectMin();
							const ImVec2 p1 = ImGui::GetItemRectMax();
							ImGui::PushClipRect(p0, p1, true);
							ImDrawList *draw_list = ImGui::GetWindowDrawList();
							draw_list->AddRect(p0, p1, IM_COL32_WHITE);
							const ImVec2 pos = p0 + ImVec2((thumbnailSize - textWidth) * 0.5f, (thumbnailSize - textHeight) * 0.5f);
							draw_list->AddText(nullptr, 0.f, pos, IM_COL32_WHITE, name.c_str(), 0, thumbnailSize);
							draw_list->AddText(nullptr, 0.f, p1 - ImVec2(16, 16), IM_COL32_WHITE, toIcon(castMember._value->_type));
							ImGui::PopClipRect();
						}
						ImGui::EndGroup();
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();

		// in the footer display a slider for the grid view: thumbnail size
		if (!_state->_cast._listView) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::SliderInt("Thumbnail Size", &_state->_cast._thumbnailSize, 32, 256);
		}
	}
	ImGui::End();
}

static void displayVariable(Common::String &name) {
	const ImU32 var_color = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.0f, 1.0f));

	const ImU32 disp_color_disabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 0.0f));
	const ImU32 disp_color_enabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 1.0f));
	const ImU32 disp_color_hover = ImGui::GetColorU32(ImVec4(0.42f, 0.17f, 0.13f, 1.0f));
	ImU32 color;

	color = disp_color_disabled;

	if (_state->_variables.contains(name))
		color = disp_color_enabled;

	ImDrawList *dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 eyeSize = ImGui::CalcTextSize("\ue8f4 ");	// visibility
	ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

	ImGui::InvisibleButton("Line", ImVec2(textSize.x + eyeSize.x, textSize.y));
	if (ImGui::IsItemClicked(0)) {
		if (color == disp_color_enabled) {
			_state->_variables.erase(name);
			color = disp_color_disabled;
		} else {
			_state->_variables[name] = true;
			color = disp_color_enabled;
		}
	}

	if (color == disp_color_disabled && ImGui::IsItemHovered()) {
		color = disp_color_hover;
	}

	dl->AddText(pos, color, "\ue8f4 ");	// visibility
	dl->AddText(ImVec2(pos.x + eyeSize.x, pos.y), var_color, name.c_str());
}

static void showVars() {
	if (!_state->_w.vars)
		return;

	// take a snapshot of the variables every 500 ms
	if ((g_director->getTotalPlayTime() - _state->_vars._lastTimeRefreshed) > 500) {
		if (g_lingo->_state->localVars) {
			_state->_vars._locals = *g_lingo->_state->localVars;
		} else {
			_state->_vars._locals.clear();
		}

		_state->_vars._globals = g_lingo->_globalvars;
		_state->_vars._lastTimeRefreshed = g_director->getTotalPlayTime();
	}

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Vars", &_state->_w.vars)) {
		Common::Array<Common::String> keyBuffer;

		if (ImGui::CollapsingHeader("Global vars:", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto &it : _state->_vars._globals) {
				keyBuffer.push_back(it._key);
			}
			Common::sort(keyBuffer.begin(), keyBuffer.end());
			for (auto &i : keyBuffer) {
				Datum &val = _state->_vars._globals.getVal(i);
				displayVariable(i);
				ImGui::SameLine();
				ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
			}
			keyBuffer.clear();
		}
		if (ImGui::CollapsingHeader("Local vars:", ImGuiTreeNodeFlags_None)) {
			if (!_state->_vars._locals.empty()) {
				for (auto &it : _state->_vars._locals) {
					keyBuffer.push_back(it._key);
				}
				Common::sort(keyBuffer.begin(), keyBuffer.end());
				for (auto &i : keyBuffer) {
					Datum &val = _state->_vars._locals.getVal(i);
					displayVariable(i);
					ImGui::SameLine();
					ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
				}
				keyBuffer.clear();
			} else {
				ImGui::Text("  (no local vars)");
			}
		}
		if (ImGui::CollapsingHeader("Instance/property vars:", ImGuiTreeNodeFlags_None)) {
			if (lingo->_state->me.type == OBJECT && lingo->_state->me.u.obj->getObjType() & (kFactoryObj | kScriptObj)) {
				ScriptContext *script = static_cast<ScriptContext *>(lingo->_state->me.u.obj);
				for (uint32 i = 1; i <= script->getPropCount(); i++) {
					keyBuffer.push_back(script->getPropAt(i));
				}
				Common::sort(keyBuffer.begin(), keyBuffer.end());
				for (auto &i : keyBuffer) {
					Datum val = script->getProp(i);
					displayVariable(i);
					ImGui::SameLine();
					ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
				}
				keyBuffer.clear();
			} else {
				ImGui::Text("  (no instance or property)");
			}
		}
	}
	ImGui::End();
}

static ImVec4 convertColor(uint32 color) {
	if (g_director->_colorDepth <= 8) {
		float r = g_director->getPalette()[color * 3 + 0] * 1.0 / 255.0;
		float g = g_director->getPalette()[color * 3 + 1] * 1.0 / 255.0;
		float b = g_director->getPalette()[color * 3 + 2] * 1.0 / 255.0;

		return ImVec4(r, g, b, 1.0);
	}

	return ImGui::ColorConvertU32ToFloat4(color);
}

static void addScriptCastToDisplay(CastMemberID &id) {
	_state->_scriptCasts.remove(id);
	_state->_scriptCasts.push_back(id);
}

static void setScriptToDisplay(const ImGuiScript &script) {
	uint index = _state->_functions._scripts.size();
	_state->_functions._scripts.push_back(script);
	_state->_functions._current = index;
	_state->_functions._showScript = true;
}

static void showChannels() {
	if (!_state->_w.channels)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Channels", &_state->_w.channels)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		Frame &frame = *score->_currentFrame;

		CastMemberID defaultPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
		ImGui::Text("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d, currentFPS: %d",
			frame._mainChannels.tempo, frame._mainChannels.skipFrameFlag, frame._mainChannels.blend, score->_currentFrameRate);
		if (!frame._mainChannels.palette.paletteId.isNull()) {
			ImGui::Text("PAL:    paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d, currentId: %s, defaultId: %s",
				frame._mainChannels.palette.paletteId.asString().c_str(), frame._mainChannels.palette.firstColor, frame._mainChannels.palette.lastColor, frame._mainChannels.palette.flags,
				frame._mainChannels.palette.cycleCount, frame._mainChannels.palette.speed, frame._mainChannels.palette.frameCount,
				frame._mainChannels.palette.fade, frame._mainChannels.palette.delay, frame._mainChannels.palette.style, g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
		} else {
			ImGui::Text("PAL:    paletteId: 000, currentId: %s, defaultId: %s\n", g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
		}
		ImGui::Text("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d",
			frame._mainChannels.transType, frame._mainChannels.transDuration, frame._mainChannels.transChunkSize);
		ImGui::Text("SND: 1  sound1: %d, soundType1: %d", frame._mainChannels.sound1.member, frame._mainChannels.soundType1);
		ImGui::Text("SND: 2  sound2: %d, soundType2: %d", frame._mainChannels.sound2.member, frame._mainChannels.soundType2);
		ImGui::Text("LSCR:   actionId: %d", frame._mainChannels.actionId.member);

		if (ImGui::BeginTable("Channels", 21, ImGuiTableFlags_Borders)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader;
			ImGui::TableSetupColumn("CH", flags);
			ImGui::TableSetupColumn("castId", flags);
			ImGui::TableSetupColumn("vis", flags);
			ImGui::TableSetupColumn("inkData", flags);
			ImGui::TableSetupColumn("ink", flags);
			ImGui::TableSetupColumn("trails", flags);
			ImGui::TableSetupColumn("stretch", flags);
			ImGui::TableSetupColumn("line", flags);
			ImGui::TableSetupColumn("dims", flags);
			ImGui::TableSetupColumn("type", flags);
			ImGui::TableSetupColumn("fg", flags);
			ImGui::TableSetupColumn("bg", flags);
			ImGui::TableSetupColumn("script", flags);
			ImGui::TableSetupColumn("colorcode", flags);
			ImGui::TableSetupColumn("blendAmount", flags);
			ImGui::TableSetupColumn("unk3", flags);
			ImGui::TableSetupColumn("constraint", flags);
			ImGui::TableSetupColumn("puppet", flags);
			ImGui::TableSetupColumn("moveable", flags);
			ImGui::TableSetupColumn("movieRate", flags);
			ImGui::TableSetupColumn("movieTime", flags);

			ImGui::TableAngledHeadersRow();
			for (int i = 0; i < frame._numChannels; i++) {
				Channel &channel = *score->_channels[i + 1];
				Sprite &sprite = *channel._sprite;

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%-3d", i + 1);
				ImGui::TableNextColumn();

				if (sprite._castId.member) {
					Common::Point position = channel.getPosition();
					ImGui::Text("%s", sprite._castId.asString().c_str());
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &channel._visible);
					ImGui::TableNextColumn();
					ImGui::Text("0x%02x", sprite._inkData);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._ink);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._trails);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._stretch);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._thickness);
					ImGui::TableNextColumn();
					ImGui::Text("%dx%d@%d,%d", channel.getWidth(), channel.getHeight(), position.x, position.y);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%s)", sprite._spriteType, spriteType2str(sprite._spriteType));
					ImGui::TableNextColumn();
					ImGui::Text("%3d", sprite._foreColor); ImGui::SameLine();
					ImGui::ColorButton("foreColor", convertColor(sprite._foreColor));
					ImGui::TableNextColumn();
					ImGui::Text("%3d", sprite._backColor); ImGui::SameLine();
					ImGui::ColorButton("backColor", convertColor(sprite._backColor));
					ImGui::TableNextColumn();
					if (sprite._scriptId.member) {
						ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "%s", sprite._scriptId.asString().c_str());

						if (ImGui::IsItemClicked(0))
							addScriptCastToDisplay(sprite._scriptId);
					}
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._colorcode);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._blendAmount);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._unk3);
					ImGui::TableNextColumn();
					ImGui::Text("%d", channel._constraint);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._puppet);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._moveable);
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%f", channel._movieRate);
					else
						ImGui::Text("0");
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%d (%f)", channel._movieTime, (float)(channel._movieTime/60.0f));
					else
						ImGui::Text("0");
				} else {
					ImGui::Text("000");
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void renderCastScript(Symbol &sym) {
	if (sym.type != HANDLER)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	Common::String handlerName;

	if (sym.ctx && sym.ctx->_id)
		handlerName = Common::String::format("%d:", sym.ctx->_id);

	handlerName += lingo->formatFunctionName(sym);

	ImGui::Text("%s", handlerName.c_str());

	ImDrawList *dl = ImGui::GetWindowDrawList();

	ImVec4 color;

	uint pc = 0;
	while (pc < sym.u.defn->size()) {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);
		Common::String bpName = Common::String::format("%s-%d", handlerName.c_str(), pc);

		color = _state->_colors._bp_color_disabled;

		Director::Breakpoint *bp = getBreakpoint(handlerName, pc);
		if (bp)
			color = _state->_colors._bp_color_enabled;

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		if (ImGui::IsItemClicked(0)) {
			if (bp) {
				g_lingo->delBreakpoint(bp->id);
				color = _state->_colors._bp_color_disabled;
			} else {
				Director::Breakpoint newBp;
				newBp.type = kBreakpointFunction;
				newBp.funcName = handlerName;
				newBp.funcOffset = pc;
				g_lingo->addBreakpoint(newBp);
				color = _state->_colors._bp_color_enabled;
			}
		}

		if (color == _state->_colors._bp_color_disabled && ImGui::IsItemHovered()) {
			color = _state->_colors._bp_color_hover;
		}

		dl->AddCircleFilled(mid, 4.0f, ImColor(color));
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), ImColor(_state->_colors._line_color));

		ImGui::SetItemTooltip("Click to add a breakpoint");

		ImGui::SameLine();
		ImGui::Text("[%5d] ", pc);
		ImGui::SameLine();
		ImGui::Text("%s", lingo->decodeInstruction(sym.u.defn, pc, &pc).c_str());
	}
}

static void renderScript(ImGuiScript &script, bool showByteCode) {
	if (!script.root) return;

	RenderScriptVisitor visitor(script, showByteCode);
	script.root->accept(visitor);
}

static bool showScriptCast(CastMemberID &id) {
	Common::String wName("Script ");
	wName += id.asString();

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	bool closed = true;

	if (ImGui::Begin(wName.c_str(), &closed)) {
		Cast *cast = g_director->getCurrentMovie()->getCasts()->getVal(id.castLib);
		ScriptContext *ctx = g_director->getCurrentMovie()->getScriptContext(kScoreScript, id);

		if (ctx) {
			for (auto &handler : ctx->_functionHandlers)
				renderCastScript(handler._value);
		} else if (cast->_lingoArchive->factoryContexts.contains(id.member)) {
			for (auto &it : *cast->_lingoArchive->factoryContexts.getVal(id.member)) {
				for (auto &handler : it._value->_functionHandlers)
					renderCastScript(handler._value);
			}
		} else {
			ImGui::Text("[Nothing]");
		}
	}
	ImGui::End();

	if (!closed)
		return false;

	return true;
}

/**
 * Display all open scripts
 */
static void displayScriptCasts() {
	if (_state->_scriptCasts.empty())
		return;

	for (Common::List<CastMemberID>::iterator scr = _state->_scriptCasts.begin(); scr != _state->_scriptCasts.end();) {
		if (!showScriptCast(*scr))
			scr = _state->_scriptCasts.erase(scr);
		else
			scr++;
	}
}

static void displayScripts() {
	if (!_state->_functions._showScript)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Script", &_state->_functions._showScript)) {
		ImGui::BeginDisabled(_state->_functions._scripts.empty() || _state->_functions._current == 0);
		if (ImGui::Button("\ue5c4")) { // Backward	// arrow_back
			_state->_functions._current--;
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Backward");
		ImGui::SameLine();

		ImGui::BeginDisabled(_state->_functions._current >= _state->_functions._scripts.size() - 1);
		if (ImGui::Button("\ue5c8")) { // Forward	// arrow_forward
			_state->_functions._current++;
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Forward");
		ImGui::SameLine();

		const char *currentScript = nullptr;
		if (_state->_functions._current < _state->_functions._scripts.size()) {
			currentScript = _state->_functions._scripts[_state->_functions._current].handlerName.c_str();
		}

		if (ImGui::BeginCombo("##handlers", currentScript)) {
			for (uint i = 0; i < _state->_functions._scripts.size(); i++) {
				auto &script = _state->_functions._scripts[i];
				bool selected = i == _state->_functions._current;
				if (ImGui::Selectable(script.handlerName.c_str(), &selected)) {
					_state->_functions._current = i;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine(0, 20);

		toggleButton("\uf569", &_state->_functions._showByteCode, true); // Lingo		// package_2
		ImGui::SetItemTooltip("Lingo");
		ImGui::SameLine();

		toggleButton("\uf500", &_state->_functions._showByteCode); // Bytecode	// stacks
		ImGui::SetItemTooltip("Bytecode");
		ImGui::Separator();

		const ImVec2 childsize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##script", childsize);
		ImGuiScript &script = _state->_functions._scripts[_state->_functions._current];
		renderScript(script, _state->_functions._showByteCode);
		ImGui::EndChild();
	}
	ImGui::End();
}

static Common::String getHandlerName(Symbol &sym) {
	Common::String handlerName;

	if (sym.ctx && sym.ctx->_id)
		handlerName = Common::String::format("%d:", sym.ctx->_id);
	handlerName += g_lingo->formatFunctionName(sym);

	return handlerName;
}

static void showFuncList() {
	if (!_state->_w.funcList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_w.funcList)) {
		Lingo *lingo = g_director->getLingo();
		Movie *movie = g_director->getCurrentMovie();
		ScriptContext *csc = lingo->_state->context;

		_state->_functions._nameFilter.Draw();
		ImGui::Separator();
		const ImVec2 childSize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##functions", ImVec2(childSize.x, childSize.y));

		if (ImGui::BeginTable("Functions", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Function", 0, 240.f);
			ImGui::TableSetupColumn("Movie", 0, 60.f);
			ImGui::TableSetupColumn("Cast", 0, 60.f);
			ImGui::TableSetupColumn("Type", 0, 80.f);
			ImGui::TableHeadersRow();

			if (csc) {
				Common::String scriptType(scriptType2str(csc->_scriptType));
				for (auto &functionHandler : csc->_functionHandlers) {
					Common::String function(g_lingo->formatFunctionName(functionHandler._value));
					if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
						continue;

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					if (ImGui::Selectable(function.c_str())) {
						// TODO:
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
					ImGui::TableNextColumn();
					ImGui::Text("-");
					ImGui::TableNextColumn();
					ImGui::Text("%s", scriptType.c_str());
				}
			}

			for (auto &cast : *movie->getCasts()) {
				for (int i = 0; i <= kMaxScriptType; i++) {
					if (cast._value->_lingoArchive->scriptContexts[i].empty())
						continue;
					Common::String scriptType(scriptType2str((ScriptType)i));
					for (auto &scriptContext : cast._value->_lingoArchive->scriptContexts[i]) {
						Common::String name = Common::String::format("%d", scriptContext._key);
						CastMemberInfo *cmi = cast._value->getCastMemberInfo(scriptContext._key);
						if (cmi && !cmi->name.empty()) {
							name += Common::String::format(" \"%s\"", cmi->name.c_str());
						}
						for (auto &functionHandler : scriptContext._value->_functionHandlers) {
							Common::String function = Common::String::format("%s: %s", name.c_str(), g_lingo->formatFunctionName(functionHandler._value).c_str());
							if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
								continue;

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							if (ImGui::Selectable(function.c_str())) {
								CastMemberID memberID(scriptContext._key, cast._key);
								ImGuiScript script = toImGuiScript(memberID, functionHandler._key);
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.handlerName = getHandlerName(functionHandler._value);
								setScriptToDisplay(script);
							}
							ImGui::TableNextColumn();
							ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
							ImGui::TableNextColumn();
							ImGui::Text("%d", cast._key);
							ImGui::TableNextColumn();
							ImGui::Text("%s", scriptType.c_str());
						}
					}
				}
			}

			Cast *sharedCast = movie->getSharedCast();
			if (sharedCast && sharedCast->_lingoArchive) {
				for (int i = 0; i <= kMaxScriptType; i++) {
					if (sharedCast->_lingoArchive->scriptContexts[i].empty())
						continue;
					Common::String scriptType(scriptType2str((ScriptType)i));
					for (auto &scriptContext : sharedCast->_lingoArchive->scriptContexts[i]) {
						Common::String name = Common::String::format("%d", scriptContext._key);
						CastMemberInfo *cmi = sharedCast->getCastMemberInfo(scriptContext._key);
						if (cmi && !cmi->name.empty()) {
							name += Common::String::format(" \"%s\"", cmi->name.c_str());
						}
						for (auto &functionHandler : scriptContext._value->_functionHandlers) {
							Common::String function = Common::String::format("%s: %s", name.c_str(), g_lingo->formatFunctionName(functionHandler._value).c_str());
							if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
								continue;

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							if (ImGui::Selectable(function.c_str())) {
								CastMemberID memberID(scriptContext._key, SHARED_CAST_LIB);
								ImGuiScript script = toImGuiScript(memberID, functionHandler._key);
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.handlerName = getHandlerName(functionHandler._value);
								setScriptToDisplay(script);
							}
							ImGui::TableNextColumn();
							ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
							ImGui::TableNextColumn();
							ImGui::Text("SHARED");
							ImGui::TableNextColumn();
							ImGui::Text("%s", scriptType.c_str());
						}
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

// Make the UI compact because there are so many fields
static void PushStyleCompact() {
	ImGuiStyle &style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact() {
	ImGui::PopStyleVar(2);
}

static void showSettings() {
	if (!_state->_w.settings)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Settings", &_state->_w.settings)) {
		ImGui::ColorEdit4("Breakpoint disabled", &_state->_colors._bp_color_disabled.x);
		ImGui::ColorEdit4("Breakpoint enabled", &_state->_colors._bp_color_enabled.x);
		ImGui::ColorEdit4("Breakpoint hover", &_state->_colors._bp_color_hover.x);
		ImGui::Separator();
		ImGui::ColorEdit4("Line", &_state->_colors._line_color.x);
		ImGui::ColorEdit4("Call", &_state->_colors._call_color.x);
		ImGui::ColorEdit4("Builtin", &_state->_colors._builtin_color.x);
		ImGui::ColorEdit4("Variable", &_state->_colors._var_color.x);
		ImGui::ColorEdit4("Literal", &_state->_colors._literal_color.x);
		ImGui::ColorEdit4("Comment", &_state->_colors._comment_color.x);
		ImGui::ColorEdit4("Type", &_state->_colors._type_color.x);
		ImGui::ColorEdit4("Keyword", &_state->_colors._keyword_color.x);
		ImGui::ColorEdit4("The entity", &_state->_colors._the_color.x);
	}
	ImGui::End();
}

static void showBreakpointList() {
	if (!_state->_w.bpList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Breakpoints", &_state->_w.bpList)) {
		auto &bps = g_lingo->getBreakpoints();
		if (ImGui::BeginTable("BreakpointsTable", 5, ImGuiTableFlags_SizingFixedFit)) {
			for (uint i = 0; i < 5; i++)
				ImGui::TableSetupColumn(NULL, i == 2 ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_NoHeaderWidth);

			for (uint i = 0; i < bps.size(); i++) {
				if (bps[i].type != kBreakpointFunction)
					continue;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImDrawList *dl = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				const ImVec2 mid(pos.x + 7, pos.y + 7);

				ImVec4 color = bps[i].enabled ? _state->_colors._bp_color_enabled : _state->_colors._bp_color_disabled;
				ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
				if (ImGui::IsItemClicked(0)) {
					if (bps[i].enabled) {
						bps[i].enabled = false;
						color = _state->_colors._bp_color_disabled;
					} else {
						bps[i].enabled = true;
						color = _state->_colors._bp_color_enabled;
					}
				}

				if (!bps[i].enabled && ImGui::IsItemHovered()) {
					color = _state->_colors._bp_color_hover;
				}

				if (bps[i].enabled)
					dl->AddCircleFilled(mid, 4.0f, ImColor(color));
				else
					dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._line_color));

				// enabled column
				ImGui::TableNextColumn();
				PushStyleCompact();
				ImGui::PushID(i);
				ImGui::Checkbox("", &bps[i].enabled);
				PopStyleCompact();

				// description
				ImGui::TableNextColumn();
				Common::String desc;
				if (bps[i].scriptId)
					desc = Common::String::format("%d: %s", bps[i].scriptId, bps[i].funcName.c_str());
				else
					desc = bps[i].funcName;
				ImGui::Text("%s", desc.c_str());

				// remove bp
				ImGui::TableNextColumn();
				pos = ImGui::GetCursorScreenPos();
				const bool del = ImGui::InvisibleButton("DelBp", ImVec2(16, ImGui::GetFontSize()));
				const bool hovered = ImGui::IsItemHovered();
				const float fontSize = ImGui::GetFontSize();
				const float cross_extent = ImGui::GetFontSize() * 0.5f * 0.7071f - 1.0f;
				const ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);
				const ImVec2 center = pos + ImVec2(0.5f + fontSize * 0.5f, 1.0f + fontSize * 0.5f);
				if (hovered)
					dl->AddCircleFilled(center, MAX(2.0f, fontSize * 0.5f + 1.0f), ImGui::GetColorU32(ImGuiCol_ButtonActive));
				dl->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
				dl->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), cross_col, 1.0f);

				// offset
				ImGui::TableNextColumn();
				ImGui::Text("%d", bps[i].funcOffset);
				ImGui::PopID();

				if (del) {
					g_lingo->delBreakpoint(bps[i].id);
					break;
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

enum { kModeMember, kModeBehavior, kModeLocation, kModeInk, kModeBlend, kModeExtended,
		kChTempo, kChPalette, kChTransition, kChSound1, kChSound2, kChScript };
const char *modes[] = { "Member", "Behavior", "Location", "Ink", "Blend", "Extended" };
const char *modes2[] = {
	"\ue425", "Tempo",		// timer
	"\ue40a", "Palette",	// palette
	"\uf50c", "Transition",	// transition_fade
	"\ue0501","Sound 1",	// volume_up
	"\ue0502","Sound 2",	// volume_up
	"\uf0c8", "Script",		// forms_apps_script
};

static void displayScoreChannel(int ch, int mode, int modeSel) {
	Score *score = g_director->getCurrentMovie()->getScore();
	uint numFrames = score->_scoreCache.size();

	const uint currentFrameNum = score->getCurrentFrameNum();
	const ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.0f, 0.65f));

	ImGui::TableNextRow();

	ImGui::PushFont(_state->_tinyFont);

	if (modeSel == kModeExtended && mode == kModeExtended)
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));

	{
		ImGui::TableNextColumn();

		float indentSize = 17.0;

		if (mode < kChTempo && modeSel == kModeExtended)
			indentSize = 10.0;

		if (modeSel == kModeExtended && mode == kModeExtended)
			indentSize = 0.1;

		ImGui::Indent(indentSize);

		if (mode >= kChTempo) {
			ImGui::PushFont(ImGui::GetIO().FontDefault);

			ImGui::Text(modes2[(mode - kChTempo) * 2]);
			ImGui::SetItemTooltip(modes2[(mode - kChTempo) * 2 + 1]);

			ImGui::PopFont();
		} else if (modeSel != kModeExtended || mode == kModeExtended) {
			ImGui::Text("%3d", ch);
		} else {
			ImGui::Text(modes[mode]);
		}

		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

		ImGui::Unindent(indentSize);
	}

	numFrames -= _state->_scoreFrameOffset;
	numFrames = MIN<uint>(numFrames, kMaxColumnsInTable - 2);

	for (int f = 0; f < (int)numFrames; f++) {
		Frame &frame = *score->_scoreCache[f + _state->_scoreFrameOffset];
		Sprite &sprite = *frame._sprites[ch];

		if (f + _state->_scoreFrameOffset == (int)currentFrameNum)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);

		ImGui::TableNextColumn();

		if (f == _state->_selectedScoreCast.frame + _state->_scoreFrameOffset &&
		  ch == _state->_selectedScoreCast.channel && mode <= kModeExtended)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.7f, 0.3f)));

		switch (mode) {
		case kModeMember:
			if (sprite._castId.member)
				ImGui::Selectable(Common::String::format("%d", sprite._castId.member).c_str());
			else if (sprite.isQDShape())
				ImGui::Selectable("Q");
			else
				ImGui::Selectable("  ");
			break;

		case kModeInk:
			ImGui::Selectable(Common::String::format("%s", inkType2str(sprite._ink)).c_str());
			break;

		case kModeLocation:
			ImGui::Selectable(Common::String::format("%d, %d", sprite._startPoint.x, sprite._startPoint.y).c_str());
			break;

		case kModeBlend:
			ImGui::Selectable(Common::String::format("%d", sprite._blendAmount).c_str());
			break;

		case kModeBehavior:
			if (sprite._scriptId.member) {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "%s", sprite._scriptId.asString().c_str());

				if (ImGui::IsItemClicked(0))
					addScriptCastToDisplay(sprite._scriptId);
			} else {
				ImGui::Selectable("  ");
			}
			break;

		case kChTempo:
			if (frame._mainChannels.tempo)
				ImGui::Text(Common::String::format("%d", frame._mainChannels.tempo).c_str());
			break;

		case kChPalette:
			if (frame._mainChannels.palette.paletteId.member)
				ImGui::Text(frame._mainChannels.palette.paletteId.asString().c_str());
			break;

		case kChTransition:
			if (frame._mainChannels.transType)
				ImGui::Text(Common::String::format("%d", frame._mainChannels.transType).c_str());
			break;

		case kChSound1:
			if (frame._mainChannels.sound1.member)
				ImGui::Text(Common::String::format("%d", frame._mainChannels.sound1.member).c_str());
			break;

		case kChSound2:
			if (frame._mainChannels.sound2.member)
				ImGui::Text(Common::String::format("%d", frame._mainChannels.sound2.member).c_str());
			break;

		case kChScript:
			if (frame._mainChannels.actionId.member)
				ImGui::Text(Common::String::format("%d", frame._mainChannels.actionId.member).c_str());
			break;

		case kModeExtended: // Render empty row
		default:
			ImGui::Selectable("  ");
		}

		if (ImGui::IsItemClicked(0)) {
			_state->_selectedScoreCast.frame = f + _state->_scoreFrameOffset;
			_state->_selectedScoreCast.channel = ch;
		}
	}

	ImGui::PopFont();
}

static void showScore() {
	if (!_state->_w.score)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Score", &_state->_w.score)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		uint numFrames = score->_scoreCache.size();
		Cast *cast = g_director->getCurrentMovie()->getCast();

		if (!numFrames) {
			ImGui::Text("No frames");
			ImGui::End();

			return;
		}

		if (_state->_selectedScoreCast.frame >= (int)numFrames)
			_state->_selectedScoreCast.frame = 0;

		if (!numFrames || _state->_selectedScoreCast.channel >= (int)score->_scoreCache[0]->_sprites.size())
			_state->_selectedScoreCast.channel = 0;

		if (_state->_scoreFrameOffset >= numFrames)
			_state->_scoreFrameOffset = 1;

		{ // Render sprite details
			Sprite *sprite = nullptr;
			CastMember *castMember = nullptr;
			bool shape = false;

			if (_state->_selectedScoreCast.frame != -1)
				sprite = score->_scoreCache[_state->_selectedScoreCast.frame]->_sprites[_state->_selectedScoreCast.channel];

			if (sprite) {
				castMember = cast->getCastMember(sprite->_castId.member, true);

				shape = sprite->isQDShape();
			}

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Image", ImVec2(200.0f, 70.0f));

			if (castMember || shape) {
				ImGuiImage imgID;

				if (castMember)
					imgID = getImageID(castMember);

				if (castMember && imgID.id) {
					Common::String name(getDisplayName(castMember));
					showImage(imgID, name.c_str(), 32.f);
				} else {
					ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
				}
				ImGui::SameLine();
				ImGui::Text("%s", sprite->_castId.asString().c_str());
				ImGui::Text("%s", spriteType2str(sprite->_spriteType));
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginChild("Details", ImVec2(500.0f, 70.0f));

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Ink", ImVec2(150.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Text("%s", inkType2str(sprite->_ink));
				ImGui::SameLine(70);
				ImGui::SetItemTooltip("Ink");
				ImGui::Text("|");
				ImGui::SameLine();
				ImGui::Text("%d", sprite->_blendAmount);
				ImGui::SameLine();
				ImGui::SetItemTooltip("Blend");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Range", ImVec2(100.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Text("\uf816"); ImGui::SameLine();	// line_start_circle
				ImGui::Text("  ?"); ImGui::SameLine(50);
				ImGui::SetItemTooltip("Start Frame");
				ImGui::Text("\uf819"); ImGui::SameLine();	// line_end_square
				ImGui::Text("  ?"); ImGui::SameLine();
				ImGui::SetItemTooltip("End Frame");
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Flags", ImVec2(200.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Checkbox("\ue897", &sprite->_enabled); ImGui::SameLine();	// lock
				ImGui::SetItemTooltip("enabled");
				ImGui::Checkbox("\ue745", &sprite->_editable); ImGui::SameLine();	// edit_note
				ImGui::SetItemTooltip("editable");
				ImGui::Checkbox("\uf712", &sprite->_moveable); ImGui::SameLine();	// move_selection_right
				ImGui::SetItemTooltip("moveable");
				ImGui::Checkbox("\uea14", &sprite->_trails);						// dynamic_feed
				ImGui::SetItemTooltip("trails");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Colors", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				ImVec4 fg = convertColor(sprite->_foreColor);

				ImGui::ColorButton("foreColor", fg);
				ImGui::SameLine();
				ImGui::Text("#%02x%02x%02x", (int)(fg.x * 255), (int)(fg.y * 255), (int)(fg.z * 255));
				ImGui::SetItemTooltip("Foreground Color");
				ImVec4 bg = convertColor(sprite->_backColor);
				ImGui::ColorButton("backColor", bg);
				ImGui::SameLine();
				ImGui::Text("#%02x%02x%02x", (int)(bg.x * 255), (int)(bg.y * 255), (int)(bg.z * 255));
				ImGui::SameLine();
				ImGui::SetItemTooltip("Background Color");
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Coordinates", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				ImGui::Text("X:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->_startPoint.x); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Reg Point Horizontal");
				ImGui::Text("W:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->getWidth());
				ImGui::SetItemTooltip("Width");

				ImGui::Text("Y:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->_startPoint.y); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Reg Point Vertical");
				ImGui::Text("H:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->getHeight()); ImGui::SameLine();
				ImGui::SetItemTooltip("Height");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Bbox", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				const Common::Rect &box = sprite->getBbox(true);

				ImGui::Text("l:"); ImGui::SameLine();
				ImGui::Text("%d", box.left); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Left");
				ImGui::Text("r:"); ImGui::SameLine();
				ImGui::Text("%d", box.right);
				ImGui::SetItemTooltip("Right");

				ImGui::Text("t:"); ImGui::SameLine();
				ImGui::Text("%d", box.top); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Top");
				ImGui::Text("b:"); ImGui::SameLine();
				ImGui::Text("%d", box.bottom);
				ImGui::SetItemTooltip("Bottom");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::EndChild();
		}

		uint numChannels = score->_scoreCache[0]->_sprites.size();
		uint tableColumns = MAX(numFrames + 5, 25U); // Set minimal table width to 25

		{  // Render pagination
			int frame = 1;

			ImGui::Text("   Jump to frame: ");

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

			do {
				ImGui::SameLine(0, 20);

				if (ImGui::Selectable(Common::String::format("%d", frame).c_str(), frame == _state->_scoreFrameOffset, 0, ImVec2(30, 10)))
					_state->_scoreFrameOffset = frame;

				frame += 300;
			} while (frame < numFrames);

			ImGui::PopStyleVar();

			ImGui::SameLine();
			ImGui::Text(" of %d", numFrames);
		}

		if (tableColumns > kMaxColumnsInTable - 2) // Current restriction of ImGui
			tableColumns = kMaxColumnsInTable - 2;

		ImGuiTableFlags addonFlags = _state->_scoreMode == kModeExtended ? 0 : ImGuiTableFlags_RowBg;

		if (ImGui::BeginTable("Score", tableColumns + 1,
							  ImGuiTableFlags_Borders | ImGuiTableFlags_HighlightHoveredColumn |
								  ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | addonFlags)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed;

			ImGui::TableSetupScrollFreeze(1, 2);

			ImGui::PushFont(_state->_tinyFont);

			ImGui::TableSetupColumn("##", flags);
			for (uint i = 0; i < tableColumns; i++)
				ImGui::TableSetupColumn(((i + _state->_scoreFrameOffset) % 5 ? " " : Common::String::format("%-2d", i + _state->_scoreFrameOffset).c_str()), flags);

			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			ImGui::TableNextRow(0);

			ImGui::TableSetColumnIndex(0);
			ImGui::PushID(0);

			ImGui::SetNextItemWidth(50);

			const char *selMode = modes[_state->_scoreMode];

			if (ImGui::BeginCombo("##", selMode)) {
				for (int n = 0; n < ARRAYSIZE(modes); n++) {
					const bool selected = (_state->_scoreMode == n);
					if (ImGui::Selectable(modes[n], selected))
						_state->_scoreMode = n;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

				ImGui::TableHeader("##");
			}
			ImGui::PopID();

			for (uint i = 0; i < tableColumns; i++) {
				ImGui::TableSetColumnIndex(i + 1);
				const char *column_name = ImGui::TableGetColumnName(i + 1);

				ImGui::SetNextItemWidth(20);
				ImGui::TableHeader(column_name);
			}

			ImGui::PopFont();

			{
				displayScoreChannel(0, kChTempo, 0);
				displayScoreChannel(0, kChPalette, 0);
				displayScoreChannel(0, kChTransition, 0);
				displayScoreChannel(0, kChSound1, 0);
				displayScoreChannel(0, kChSound2, 0);
				displayScoreChannel(0, kChScript, 0);
			}
			ImGui::TableNextRow();

			int mode = _state->_scoreMode;

			for (int ch = 0; ch < (int)numChannels - 1; ch++) {
				if (mode == kModeExtended) // This will render empty row
					displayScoreChannel(ch + 1, kModeExtended, _state->_scoreMode);

				if (mode == kModeMember || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeMember, _state->_scoreMode);

				if (mode == kModeBehavior || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBehavior, _state->_scoreMode);

				if (mode == kModeInk || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeInk, _state->_scoreMode);

				if (mode == kModeBlend || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBlend, _state->_scoreMode);

				if (mode == kModeLocation || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeLocation, _state->_scoreMode);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void onLog(LogMessageType::Type type, int level, uint32 debugChannels, const char *message) {
	switch (type) {
	case LogMessageType::kError:
		_state->_logger.addLog("[error]%s", message);
		break;
	case LogMessageType::kWarning:
		_state->_logger.addLog("[warn]%s", message);
		break;
	case LogMessageType::kInfo:
		_state->_logger.addLog("%s", message);
		break;
	case LogMessageType::kDebug:
		_state->_logger.addLog("[debug]%s", message);
		break;
	}
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = false;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;
	icons_config.GlyphOffset = {0, 4};

	static const ImWchar icons_ranges[] = {0xE000, 0xF8FF, 0};
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_state = new ImGuiState();

	_state->_tinyFont = ImGui::addTTFFontFromArchive("FreeSans.ttf", 10.0f, nullptr, nullptr);

	Common::setLogWatcher(onLog);
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::SeparatorText("Windows");

			if (ImGui::MenuItem("View All")) {
				if (_state->_wasHidden)
					_state->_w = _state->_savedW;

				_state->_wasHidden = false;
			}

			if (ImGui::MenuItem("Hide All")) {
				if (!_state->_wasHidden) {
					_state->_savedW = _state->_w;

					memset((void *)&_state->_w, 0, sizeof(_state->_w));
				}

				_state->_wasHidden = true;
			}

			ImGui::MenuItem("Control Panel", NULL, &_state->_w.controlPanel);
			ImGui::MenuItem("Score", NULL, &_state->_w.score);
			ImGui::MenuItem("Functions", NULL, &_state->_w.funcList);
			ImGui::MenuItem("Cast", NULL, &_state->_w.cast);
			ImGui::MenuItem("Channels", NULL, &_state->_w.channels);
			ImGui::MenuItem("CallStack", NULL, &_state->_w.callStack);
			ImGui::MenuItem("Breakpoints", NULL, &_state->_w.bpList);
			ImGui::MenuItem("Vars", NULL, &_state->_w.vars);
			ImGui::MenuItem("Settings", NULL, &_state->_w.settings);
			ImGui::MenuItem("Logger", NULL, &_state->_w.logger);

			ImGui::SeparatorText("Misc");
			if (ImGui::MenuItem("Save state")) {
			}
			if (ImGui::MenuItem("Load state")) {
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	displayScriptCasts();
	displayScripts();

	showControlPanel();
	showVars();
	showCallStack();
	showChannels();
	showCast();
	showFuncList();
	showScore();
	showBreakpointList();
	showSettings();
	_state->_logger.draw("Logger", &_state->_w.logger);
}

void onImGuiCleanup() {
	Common::setLogWatcher(nullptr);
	if (_state)
		delete _state->_tinyFont;

	delete _state;
	_state = nullptr;
}
} // namespace Director
