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
#include "backends/imgui/imgui_utils.h"
#include "director/director.h"
#include "director/debugger/dt-internal.h"

#include "director/archive.h"
#include "director/window.h"
#include "director/cast.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/castmember/castmember.h"
#include "director/lingo/lingo-object.h"


namespace Director {
namespace DT {

static void renderScript(ImGuiScript &script, bool showByteCode, bool scrollTo) {
	if (script.oldAst) {
		renderOldScriptAST(script, showByteCode, scrollTo);
		return;
	}

	if (!script.root) {
		if (!script.rawText.empty())
			ImGui::TextUnformatted(script.rawText.c_str());
		return;
	}

	renderScriptAST(script, showByteCode, scrollTo);
}

static void renderCallStack(uint pc) {
	Common::Array<CFrame *> callstack = g_lingo->_state->callstack;
	if (callstack.size() == 0) {
		ImGui::Text("End of execution\n");
		return;
	}

	const Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return;

	ImGui::Text("Call stack:\n");
	for (int i = 0; i < (int)callstack.size(); i++) {
		Common::String stackFrame;
		CFrame *frame = callstack[callstack.size() - i - 1];
		uint framePc = pc;
		if (i > 0)
			framePc = callstack[callstack.size() - i]->retPC;

		if (frame->sp.type != VOIDSYM) {
			stackFrame = Common::String::format("#%d ", i);
			if (frame->sp.ctx)
				stackFrame += Common::String::format("%d ", frame->sp.ctx->_scriptId);

			if (frame->sp.ctx && frame->sp.ctx->_id != -1) {
				stackFrame += Common::String::format("(%d): ", frame->sp.ctx->_id);
			} else if (frame->sp.ctx) {
				// resolve in the cast lib that owns this context, not the default one
				int frameLibID = getCastLibIDForContext(frame->sp.ctx);
				const Cast *ownerCast = (frameLibID == SHARED_CAST_LIB) ? movie->getSharedCast() : movie->getCasts()->getValOrDefault(frameLibID, nullptr);
				int parentCastId = ownerCast ? ownerCast->getCastIdByScriptId(frame->sp.ctx->_parentNumber) : 0;
				stackFrame += Common::String::format("(<p%d>): ", parentCastId);
			}

			if (frame->sp.ctx && frame->sp.ctx->isFactory()) {
				stackFrame += Common::String::format("%s:", frame->sp.ctx->getName().c_str());
			}
			stackFrame += Common::String::format("%s at [%5d]\n",
				frame->sp.name->c_str(),
				framePc
			);
		} else {
			stackFrame = Common::String::format("#%d [unknown] at [%5d]\n",
				i,
				framePc
			);
		}

		if (ImGui::Selectable(stackFrame.c_str())) {
			CFrame *head = callstack[callstack.size() - i - 1];
			ScriptContext *scriptContext = head->sp.ctx;
			if (!scriptContext)
				continue;
			// the script can live in any cast lib, resolve it from the context
			int castLibID = getCastLibIDForContext(scriptContext);
			Common::String moviePath = movie->getArchive()->getPathName().toString();

			ImGuiScript script = buildImGuiHandlerScript(scriptContext, castLibID, *head->sp.name, moviePath);
			script.pc = framePc;
			setScriptToDisplay(script);
		}
	}
}

static Common::String getHandlerName(Symbol &sym) {
	Common::String handlerName;
	if (sym.ctx && sym.ctx->_id)
		handlerName = Common::String::format("%d:", sym.ctx->_id);
	handlerName += g_lingo->formatFunctionName(sym);
	return handlerName;
}

// Renders back/forward navigation, handler dropdown, and Lingo/Bytecode toggle for a ScriptData.
// Returns true when there is a script to render.
static bool renderScriptNavBar(ScriptData &data) {
	if (data._scripts.empty())
		return false;

	ImGui::BeginDisabled(data._current == 0);
	if (ImGui::Button(ICON_MS_ARROW_BACK)) {
		data._current--;
		data._scrollToCurrent = true;
	}
	ImGui::EndDisabled();
	ImGui::SetItemTooltip("Backward");
	ImGui::SameLine();

	ImGui::BeginDisabled(data._current >= data._scripts.size() - 1);
	if (ImGui::Button(ICON_MS_ARROW_FORWARD)) {
		data._current++;
		data._scrollToCurrent = true;
	}
	ImGui::EndDisabled();
	ImGui::SetItemTooltip("Forward");
	ImGui::SameLine();

	const char *currentName = data._scripts[data._current].handlerName.c_str();
	if (ImGui::BeginCombo("##handlers", currentName)) {
		for (uint i = 0; i < data._scripts.size(); i++) {
			bool selected = (i == data._current);
			ImGui::PushID(i);
			if (ImGui::Selectable(data._scripts[i].handlerName.c_str(), &selected)) {
				data._current = i;
				data._scrollToCurrent = true;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	if (!data._scripts[data._current].oldAst) {
		ImGui::SameLine(0, 20);
		if (selectableViewButton(ICON_MS_PACKAGE_2, !data._showByteCode))
			data._showByteCode = false;
		ImGui::SetItemTooltip("Lingo");
		ImGui::SameLine();
		if (selectableViewButton(ICON_MS_STACKS, data._showByteCode))
			data._showByteCode = true;
		ImGui::SetItemTooltip("Bytecode");
	}

	return true;
}

// Renders all handlers in the context, scrolling to the selected one.
static void renderScriptContext(ScriptData &data, const Movie *movie) {
	ImGuiScript &current = data._scripts[data._current];
	ScriptContext *context = getScriptContext(current.id);
	// per-store flag, so two script windows cannot steal each other's scroll
	bool scrollTo = data._scrollToCurrent;
	data._scrollToCurrent = false;

	if (!context || context->_functionHandlers.size() == 1) {
		renderScript(current, data._showByteCode, scrollTo);
		return;
	}

	for (auto &functionHandler : context->_functionHandlers) {
		if (current.handlerId == functionHandler._key) {
			renderScript(current, data._showByteCode, scrollTo);
		} else {
			ImGuiScript script = toImGuiScript(context->_scriptType, current.id, functionHandler._key);
			script.byteOffsets = context->_functionByteOffsets[script.handlerId];
			script.moviePath = movie->getArchive()->getPathName().toString();
			script.handlerName = getHandlerName(functionHandler._value);
			renderScript(script, data._showByteCode, false);
		}
		ImGui::NewLine();
	}
}

void showScriptsWindow() {
	ScriptData &data = _state->_openScripts;
	if (!_state->_w.scripts || !data._showScript || data._scripts.empty())
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 540), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scripts", &_state->_w.scripts)) {
		ScriptContext *ctx = getScriptContext(data._scripts[data._current].id);

		if (ctx)
			ImGui::Text("%s", ctx->asString().c_str());


		if (renderScriptNavBar(data)) {
			ImGui::Separator();
			ImVec2 childSize = ImGui::GetContentRegionAvail();
			ImGui::BeginChild("##script", childSize);
			renderScriptContext(data, g_director->getCurrentMovie());
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

static void updateCurrentScript() {
	if ((g_lingo->_exec._state != kPause) || !_state->_dbg._isScriptDirty)
		return;

	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
	if (callstack.empty())
		return;

	// show current script of the current stack frame
	CFrame *head = callstack[callstack.size() - 1];
	const Director::Movie *movie = g_director->getCurrentMovie();
	ScriptContext *scriptContext = head->sp.ctx;
	if (!scriptContext || !movie)
		return;
	// the script can live in any cast lib, resolve it from the context
	int castLibID = getCastLibIDForContext(scriptContext);
	Common::String moviePath = movie->getArchive()->getPathName().toString();

	ImGuiScript script = buildImGuiHandlerScript(scriptContext, castLibID, *head->sp.name, moviePath);
	// use the live pc for the current-statement marker
	script.pc = g_lingo->_state->pc;
	setScriptToDisplay(script);
}

void showFuncList() {
	if (!_state->_w.funcList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_w.funcList)) {
		Window *selectedWindow = windowListCombo(&_state->_functionsWindow);
		if (!selectedWindow->getCurrentMovie()) {
			ImGui::Text("No movie loaded");
			ImGui::End();
			return;
		}

		_state->_functions._nameFilter.Draw();
		ImGui::Separator();

		// Show a script context wise handlers
		if (selectableViewButton(ICON_MS_PACKAGE_2, _state->_functions._showScriptContexts))
			_state->_functions._showScriptContexts = true;
		ImGui::SetItemTooltip("Script Contexts");

		ImGui::SameLine();
		// Show a list of all handlers
		if (selectableViewButton(ICON_MS_STACKS, !_state->_functions._showScriptContexts))
			_state->_functions._showScriptContexts = false;
		ImGui::SetItemTooltip("All Handlers");

		const ImVec2 childSize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##functions", ImVec2(childSize.x, childSize.y));

		const Movie *movie = selectedWindow->getCurrentMovie();
		if (_state->_functions._showScriptContexts) {
			for (auto cast : *movie->getCasts()) {
				Common::String castName = Common::String::format("%d", cast._key);
				if (cast._value->getCastName().size()) {
					castName += Common::String::format(": %s ", cast._value->getCastName().c_str());
				}

				if (ImGui::TreeNodeEx(castName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
					for (auto context : cast._value->_lingoArchive->lctxContexts) {
						if (!context._value || !context._value->_functionHandlers.size()) {
							continue;
						}
						CastMemberInfo *cmi = cast._value->getCastMemberInfo(context._value->_id);
						Common::String contextName = Common::String::format("%d", context._value->_id);
						if (cmi && cmi->name.size()) {
							contextName += Common::String::format(": %s", cmi->name.c_str());
						}

						contextName += Common::String::format(": %s", scriptType2str(context._value->_scriptType));
						if (!_state->_functions._nameFilter.PassFilter(contextName.c_str())) {
							continue;
						}

						ImGui::PushID(context._key);
						if (ImGui::TreeNode(contextName.c_str())) {
							if (ImGui::BeginTable("Functions", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
								ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch, 240.f);
								int castId = context._value->_id;
								bool childScript = false;
								if (castId == -1) {
									// resolve the parent script in the cast that owns this context
									castId = cast._value->getCastIdByScriptId(context._value->_parentNumber);
									childScript = true;
								}

								for (auto &functionHandler : context._value->_functionHandlers) {
									Common::String function = Common::String::format("%s", g_lingo->formatFunctionName(functionHandler._value).c_str());

									ImGui::TableNextRow();
									ImGui::TableNextColumn();
									if (ImGui::Selectable(function.c_str())) {
										CastMemberID memberID(cast._value->getCastIdByScriptId(context._key), cast._key);
										ImGuiScript script = toImGuiScript(context._value->_scriptType, memberID, functionHandler._key);
										script.byteOffsets = context._value->_functionByteOffsets[script.handlerId];
										script.moviePath = movie->getArchive()->getPathName().toString();
										script.handlerName = formatHandlerName(context._value->_scriptId, castId, script.handlerId, context._value->_scriptType, childScript);
										addToOpenHandlers(script);
									}
								}
								ImGui::EndTable();
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}

			Cast *sharedCast = movie->getSharedCast();
			if (sharedCast && sharedCast->_lingoArchive) {
				Common::String castName = Common::String::format("%s", "SHARED");
				if (sharedCast->getCastName().size()) {
					castName += Common::String::format(": %s ", sharedCast->getCastName().c_str());
				}

				if (ImGui::TreeNode(castName.c_str())) {
					for (auto context : sharedCast->_lingoArchive->lctxContexts) {
						if (!context._value || !context._value->_functionHandlers.size()) {
							continue;
						}
						CastMemberInfo *cmi = sharedCast->getCastMemberInfo(context._value->_id);
						Common::String contextName = Common::String::format("%d", context._value->_id);
						if (cmi && cmi->name.size()) {
							contextName += Common::String::format(": %s", cmi->name.c_str());
						}

						contextName += Common::String::format(": %s", scriptType2str(context._value->_scriptType));
						if (!_state->_functions._nameFilter.PassFilter(contextName.c_str())) {
							continue;
						}

						ImGui::PushID(context._key);
						if (ImGui::TreeNode(contextName.c_str())) {
							if (ImGui::BeginTable("Functions", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
								ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch, 240.f);
								int castId = context._value->_id;
								bool childScript = false;
								if (castId == -1) {
									// resolve the parent script in the cast that owns this context
									castId = sharedCast->getCastIdByScriptId(context._value->_parentNumber);
									childScript = true;
								}

								for (auto &functionHandler : context._value->_functionHandlers) {
									Common::String function = Common::String::format("%s", g_lingo->formatFunctionName(functionHandler._value).c_str());

									ImGui::TableNextRow();
									ImGui::TableNextColumn();
									if (ImGui::Selectable(function.c_str())) {
										CastMemberID memberID(sharedCast->getCastIdByScriptId(context._key), SHARED_CAST_LIB);
										ImGuiScript script = toImGuiScript(context._value->_scriptType, memberID, functionHandler._key);
										script.byteOffsets = context._value->_functionByteOffsets[script.handlerId];
										script.moviePath = movie->getArchive()->getPathName().toString();
										script.handlerName = formatHandlerName(context._value->_scriptId, castId, script.handlerId, context._value->_scriptType, childScript);
										addToOpenHandlers(script);
									}
								}
								ImGui::EndTable();
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
		} else {
			if (ImGui::BeginTable("Functions", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Function", 0, 240.f);
				ImGui::TableSetupColumn("Cast Name", ImGuiTableColumnFlags_WidthStretch, 240.f);
				ImGui::TableHeadersRow();

				for (auto &cast : *movie->getCasts()) {
					for (int i = 0; i <= kMaxScriptType; i++) {
						if (cast._value->_lingoArchive->scriptContexts[i].empty())
							continue;
						Common::String scriptType(scriptType2str((ScriptType)i));
						for (auto &scriptContext : cast._value->_lingoArchive->scriptContexts[i]) {
							Common::String name = Common::String::format("%d", scriptContext._key);
							CastMemberInfo *cmi = cast._value->getCastMemberInfo(scriptContext._key);
							CastMember *castMember = cast._value->getCastMember(scriptContext._key);

							for (auto &functionHandler : scriptContext._value->_functionHandlers) {
								Common::String function = Common::String::format("%s-%s (%d lib %d)",
									castMember ? castType2str(castMember->_type) : "any", g_lingo->formatFunctionName(functionHandler._value).c_str(),
									scriptContext._key, cast._key
								);
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
									addToOpenHandlers(script);
								}

								ImGui::TableNextColumn();
								ImGui::Text("%s", (cmi && cmi->name.size()) ? cmi->name.c_str() : "unnamed");
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
							CastMember *castMember = sharedCast->getCastMember(scriptContext._key);

							for (auto &functionHandler : scriptContext._value->_functionHandlers) {
								Common::String function = Common::String::format("%s-%s (%d lib %d)",
									castMember ? castType2str(castMember->_type) : "any", g_lingo->formatFunctionName(functionHandler._value).c_str(),
									scriptContext._key, SHARED_CAST_LIB
								);
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
									addToOpenHandlers(script);
								}

								ImGui::TableNextColumn();
								ImGui::Text("%s", (cmi && cmi->name.size()) ? cmi->name.c_str() : "unnamed");
							}
						}
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void showExecutionContext() {
	if (!_state->_w.executionContext)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(500, 750), ImGuiCond_FirstUseEver);

	Window *currentWindow = g_director->getCurrentWindow();
	bool scriptsRendered = false;

	if (ImGui::Begin("Execution Context", &_state->_w.executionContext)) {
		// go-to-definition clicks should open in this window's script list
		_state->_dbg._hostExecutionContext = true;

		Window *selectedWindow = windowListCombo(&_state->_executionContextWindow);
		Movie *selectedMovie = selectedWindow->getCurrentMovie();

		if (!selectedMovie) {
			ImGui::Text("No movie loaded");
		} else {
			g_director->setCurrentWindow(selectedWindow);
			g_lingo->switchStateFromWindow();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Window##", ImGui::GetContentRegionAvail());
			ImGui::Text("%s", selectedWindow->asString().c_str());
			ImGui::Text("%s", selectedMovie->getMacName().c_str());

			ImGui::SeparatorText("Backtrace");
			ImVec2 childSize = ImGui::GetContentRegionAvail();
			childSize.y /= 3;
			ImGui::BeginChild("##backtrace", childSize);
			renderCallStack(g_lingo->_state->pc);
			ImGui::EndChild();

			ImGui::SeparatorText("Scripts");

			ScriptData *scriptData = &_state->_functions._windowScriptData.getOrCreateVal(selectedWindow);
			updateCurrentScript();

			if (scriptData->_showScript && !scriptData->_scripts.empty()) {
				bool oldSuppress = _state->_dbg._suppressHighlight;
				_state->_dbg._suppressHighlight = true;

				ScriptContext *context = getScriptContext(scriptData->_scripts[scriptData->_current].id);
				if (context)
					ImGui::Text("%d:%s type:%s", context->_id, context->getName().c_str(), scriptType2str(context->_scriptType));

				if (renderScriptNavBar(*scriptData)) {
					ImGui::Separator();
					childSize = ImGui::GetContentRegionAvail();
					ImGui::BeginChild("##script", childSize);
					renderScriptContext(*scriptData, selectedMovie);
					ImGui::EndChild();
					scriptsRendered = true;
				}

				_state->_dbg._suppressHighlight = oldSuppress;
			}

			ImGui::EndChild();
			ImGui::PopStyleColor();

			_state->_dbg._isScriptDirty = !scriptsRendered;

			g_director->setCurrentWindow(currentWindow);
			g_lingo->switchStateFromWindow();
		}

		_state->_dbg._hostExecutionContext = false;
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
