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

#include "backends/imgui/IconsMaterialSymbols.h"
#include "director/director.h"
#include "director/debugger/dt-internal.h"

#include "director/archive.h"
#include "director/cast.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/castmember/castmember.h"
#include "director/lingo/lingo-object.h"


namespace Director {
namespace DT {

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

		Director::Breakpoint *bp = getBreakpoint(handlerName, sym.ctx->_id, pc);
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
	if (script.oldAst) {
		renderOldScriptAST(script, showByteCode);
		_state->_dbg._isScriptDirty = false;
		return;
	}

	if (!script.root)
		return;

	renderScriptAST(script, showByteCode);
	_state->_dbg._isScriptDirty = false;
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
void showScriptCasts() {
	if (_state->_scriptCasts.empty())
		return;

	for (Common::List<CastMemberID>::iterator scr = _state->_scriptCasts.begin(); scr != _state->_scriptCasts.end();) {
		if (!showScriptCast(*scr))
			scr = _state->_scriptCasts.erase(scr);
		else
			scr++;
	}
}

static void updateCurrentScript() {
	if ((g_lingo->_exec._state != kPause) || !_state->_dbg._isScriptDirty)
		return;

	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
	if (callstack.empty())
		return;

	// show current script of the current stack frame
	CFrame *head = callstack[callstack.size() - 1];
	Director::Movie *movie = g_director->getCurrentMovie();
	ScriptContext *scriptContext = head->sp.ctx;
	int castLibID = movie->getCast()->_castLibID;
	ImGuiScript script = toImGuiScript(scriptContext->_scriptType, CastMemberID(head->sp.ctx->_id, castLibID), *head->sp.name);
	script.byteOffsets = scriptContext->_functionByteOffsets[script.handlerId];
	script.moviePath = movie->getArchive()->getPathName().toString();
	script.handlerName = head->sp.ctx->_id ? Common::String::format("%d:%s", head->sp.ctx->_id, script.handlerId.c_str()) : script.handlerId;
	setScriptToDisplay(script);
}

void showScripts() {
	updateCurrentScript();

	if (!_state->_functions._showScript)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Script", &_state->_functions._showScript)) {
		ImGui::BeginDisabled(_state->_functions._scripts.empty() || _state->_functions._current == 0);
		if (ImGui::Button(ICON_MS_ARROW_BACK)) {
			_state->_functions._current--;
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Backward");
		ImGui::SameLine();

		ImGui::BeginDisabled(_state->_functions._current >= _state->_functions._scripts.size() - 1);
		if (ImGui::Button(ICON_MS_ARROW_FORWARD)) {
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

		if (!_state->_functions._scripts[_state->_functions._current].oldAst) {
			ImGui::SameLine(0, 20);
			toggleButton(ICON_MS_PACKAGE_2, &_state->_functions._showByteCode, true); // Lingo
			ImGui::SetItemTooltip("Lingo");
			ImGui::SameLine();

			toggleButton(ICON_MS_STACKS, &_state->_functions._showByteCode); // Bytecode
			ImGui::SetItemTooltip("Bytecode");
		}

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

void showFuncList() {
	if (!_state->_w.funcList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_w.funcList)) {
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

			Movie *movie = g_director->getCurrentMovie();
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
								ImGuiScript script = toImGuiScript(scriptContext._value->_scriptType, memberID, functionHandler._key);
								script.byteOffsets = scriptContext._value->_functionByteOffsets[script.handlerId];
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
								ImGuiScript script = toImGuiScript(scriptContext._value->_scriptType, memberID, functionHandler._key);
								script.byteOffsets = scriptContext._value->_functionByteOffsets[script.handlerId];
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

} // namespace DT
} // namespace Director
