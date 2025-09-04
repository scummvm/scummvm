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

		ImGui::PushID(pc);
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
		ImGui::PopID();
	}
}

static void renderScript(ImGuiScript &script, bool showByteCode, bool scrollTo) {
	if (script.oldAst) {
		renderOldScriptAST(script, showByteCode, scrollTo);
		return;
	}

	if (!script.root)
		return;

	renderScriptAST(script, showByteCode, scrollTo);
}

static void renderCallStack(uint pc) {
	Common::Array<CFrame *> callstack = g_lingo->_state->callstack;
	if (callstack.size() == 0) {
		ImGui::Text("End of execution\n");
		return;
	}

	const Movie *movie = g_director->getCurrentMovie();

	ImGui::Text("Call stack:\n");
	for (int i = 0; i < (int)callstack.size(); i++) {
		Common::String stackFrame;
		CFrame *frame = callstack[callstack.size() - i - 1];
		uint framePc = pc;
		if (i > 0)
			framePc = callstack[callstack.size() - i]->retPC;

		if (frame->sp.type != VOIDSYM) {
			stackFrame = Common::String::format("#%d ", i);
			stackFrame += Common::String::format("%d ", frame->sp.ctx->_scriptId);

			if (frame->sp.ctx && frame->sp.ctx->_id != -1) {
				stackFrame += Common::String::format("(%d): ", frame->sp.ctx->_id);
			} else if (frame->sp.ctx) {
				stackFrame += Common::String::format("(<p%d>): ", movie->getCast()->getCastIdByScriptId(frame->sp.ctx->_parentNumber));
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
			int castLibID = movie->getCast()->_castLibID;
			int castId = head->sp.ctx->_id;
			bool childScript = false;
			if (castId == -1) {
				castId = movie->getCast()->getCastIdByScriptId(head->sp.ctx->_parentNumber);
				childScript = true;
			}

			ImGuiScript script = toImGuiScript(scriptContext->_scriptType, CastMemberID(castId, castLibID), *head->sp.name);
			script.byteOffsets = head->sp.ctx->_functionByteOffsets[script.handlerId];
			script.moviePath = movie->getArchive()->getPathName().toString();
			script.handlerName = formatHandlerName(head->sp.ctx->_scriptId, castId, script.handlerName, head->sp.ctx->_scriptType, childScript);
			script.pc = framePc;
			setScriptToDisplay(script);
		}
	}
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

static void addToOpenHandlers(ImGuiScript handler) {
	_state->_openHandlers.erase(handler.id.member);
	_state->_openHandlers[handler.id.member] = handler;
}

static bool showHandler(ImGuiScript handler) {
	ScriptContext *ctx = getScriptContext(handler.id);
	Common::String wName;
	if (ctx) {
		wName = Common::String(ctx->asString());
	}

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 540), ImGuiCond_FirstUseEver);

	bool closed = true;

	if (ImGui::Begin(wName.c_str(), &closed)) {
		ImGuiEx::toggleButton(ICON_MS_PACKAGE_2, &_state->_showCompleteScript, true); // Lingo
		ImGui::SetItemTooltip("Show Handler");

		ImGui::SameLine();
		ImGuiEx::toggleButton(ICON_MS_STACKS, &_state->_showCompleteScript); // Bytecode
		ImGui::SetItemTooltip("Show Script Context");

		if (!ctx || ctx->_functionHandlers.size() <= 1 || !_state->_showCompleteScript) {
			renderScript(handler, false, true);
		} else {
			for (auto &functionHandler : ctx->_functionHandlers) {
				ImGuiScript script = toImGuiScript(ctx->_scriptType, handler.id, functionHandler._key);
				script.byteOffsets = ctx->_functionByteOffsets[script.handlerId];

				if (script == handler) {
					_state->_dbg._goToDefinition = true;
				}
				renderScript(script, false, script == handler);
				ImGui::NewLine();
			}
		}
	}
	ImGui::End();

	if (!closed)
		return false;

	return true;
}

/**
 * Display all open handlers
 */
void showHandlers() {
	if (_state->_openHandlers.empty()) {
		return;
	}

	for (auto handler : _state->_openHandlers) {
		if (!showHandler(handler._value)) {
			_state->_openHandlers.erase(handler._value.id.member);
		}
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
	const Director::Movie *movie = g_director->getCurrentMovie();
	ScriptContext *scriptContext = head->sp.ctx;
	int castLibID = movie->getCast()->_castLibID;
	int castId = head->sp.ctx->_id;
	bool childScript = false;
	if (castId == -1) {
		castId = movie->getCast()->getCastIdByScriptId(head->sp.ctx->_parentNumber);
		childScript = true;
	}

	ImGuiScript script = toImGuiScript(scriptContext->_scriptType, CastMemberID(castId, castLibID), *head->sp.name);
	script.byteOffsets = scriptContext->_functionByteOffsets[script.handlerId];
	script.moviePath = movie->getArchive()->getPathName().toString();
	script.handlerName = formatHandlerName(head->sp.ctx->_scriptId, castId, script.handlerName, head->sp.ctx->_scriptType, childScript);
	script.pc = 0;
	setScriptToDisplay(script);
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
	ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_w.funcList)) {
		_state->_functions._nameFilter.Draw();
		ImGui::Separator();

		// Show a script context wise handlers
		ImGuiEx::toggleButton(ICON_MS_PACKAGE_2, &_state->_functions._showScriptContexts);
		ImGui::SetItemTooltip("Script Contexts");

		ImGui::SameLine();
		// Show a list of all handlers
		ImGuiEx::toggleButton(ICON_MS_STACKS, &_state->_functions._showScriptContexts, true);
		ImGui::SetItemTooltip("All Handlers");

		const ImVec2 childSize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##functions", ImVec2(childSize.x, childSize.y));

		const Movie *movie = g_director->getCurrentMovie();
		if (_state->_functions._showScriptContexts) {
			for (auto cast : *movie->getCasts()) {
				Common::String castName = Common::String::format("%d", cast._key);
				if (cast._value->getCastName().size()) {
					castName += Common::String::format(": %s ", cast._value->getCastName().c_str());
				}

				if (ImGui::TreeNodeEx(castName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
					for (auto context : cast._value->_lingoArchive->lctxContexts) {
						CastMemberInfo *cmi = cast._value->getCastMemberInfo(context._value->_id);
						Common::String contextName = Common::String::format("%d", context._value->_id);
						if (cmi && cmi->name.size()) {
							contextName += Common::String::format(": %s", cmi->name.c_str());
						}

						contextName += Common::String::format(": %s", scriptType2str(context._value->_scriptType));
						if (!context._value || !context._value->_functionHandlers.size() || !_state->_functions._nameFilter.PassFilter(contextName.c_str())) {
							continue;
						}

						ImGui::PushID(context._key);
						if (ImGui::TreeNode(contextName.c_str())) {
							if (ImGui::BeginTable("Functions", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
								ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch, 240.f);
								int castId = context._value->_id;
								bool childScript = false;
								if (castId == -1) {
									castId = movie->getCast()->getCastIdByScriptId(context._value->_parentNumber);
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
						CastMemberInfo *cmi = sharedCast->getCastMemberInfo(context._value->_id);
						Common::String contextName = Common::String::format("%d", context._value->_id);
						if (cmi && cmi->name.size()) {
							contextName += Common::String::format(": %s", cmi->name.c_str());
						}

						contextName += Common::String::format(": %s", scriptType2str(context._value->_scriptType));
						if (!context._value || !context._value->_functionHandlers.size() || !_state->_functions._nameFilter.PassFilter(contextName.c_str())) {
							continue;
						}

						ImGui::PushID(context._key);
						if (ImGui::TreeNode(contextName.c_str())) {
							if (ImGui::BeginTable("Functions", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
								ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch, 240.f);
								int castId = context._value->_id;
								bool childScript = false;
								if (castId == -1) {
									castId = movie->getCast()->getCastIdByScriptId(context._value->_parentNumber);
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
										script.handlerName = formatHandlerName(context._value->_scriptId, castId, script.handlerName, context._value->_scriptType, childScript);
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

	Director::Lingo *lingo = g_director->getLingo();
	const Movie *movie = g_director->getCurrentMovie();

	Window *currentWindow = g_director->getCurrentWindow();
	bool scriptsRendered = false;

	if (ImGui::Begin("Execution Context", &_state->_w.executionContext, ImGuiWindowFlags_AlwaysAutoResize)) {
		Window *stage = g_director->getStage();
		g_director->setCurrentWindow(stage);
		g_lingo->switchStateFromWindow();

		int windowID = 0;
		ImGui::PushID(windowID);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

		ImGui::BeginChild("Window##", ImVec2(500.0f, 700.0f));
		ImGui::Text("%s", stage->asString().c_str());
		ImGui::Text("%s", stage->getCurrentMovie()->getMacName().c_str());

		ImGui::SeparatorText("Backtrace");
		ImVec2 childSize = ImGui::GetContentRegionAvail();
		childSize.y /= 3;
		ImGui::BeginChild("##backtrace", childSize);
		renderCallStack(lingo->_state->pc);
		ImGui::EndChild();

		ImGui::SeparatorText("Scripts");

		ScriptData *scriptData = &_state->_functions._windowScriptData.getOrCreateVal(stage);
		updateCurrentScript();

		if (scriptData->_showScript) {
			ImGuiScript &current = scriptData->_scripts[scriptData->_current];

			// Get all the handlers from the script
			ScriptContext* context = getScriptContext(current.id);

			if (context) {
				ImGui::Text("%d:%s type:%s", context->_id, context->getName().c_str(), scriptType2str(context->_scriptType));
			}

			ImGui::BeginDisabled(scriptData->_scripts.empty() || scriptData->_current == 0);
			if (ImGui::Button(ICON_MS_ARROW_BACK)) {
				scriptData->_current--;
			}
			ImGui::EndDisabled();
			ImGui::SetItemTooltip("Backward");
			ImGui::SameLine();

			ImGui::BeginDisabled(scriptData->_current >= scriptData->_scripts.size() - 1);
			if (ImGui::Button(ICON_MS_ARROW_FORWARD)) {
				scriptData->_current++;
			}
			ImGui::EndDisabled();
			ImGui::SetItemTooltip("Forward");
			ImGui::SameLine();

			const char *currentScript = nullptr;

			if (scriptData->_current < scriptData->_scripts.size()) {
				currentScript = scriptData->_scripts[scriptData->_current].handlerName.c_str();
			}

			if (ImGui::BeginCombo("##handlers", currentScript)) {
				for (uint i = 0; i < scriptData->_scripts.size(); i++) {
					auto &script = scriptData->_scripts[i];
					bool selected = i == scriptData->_current;
					if (ImGui::Selectable(script.handlerName.c_str(), &selected)) {
						scriptData->_current = i;
					}
				}
				ImGui::EndCombo();
			}

			if (!scriptData->_scripts[scriptData->_current].oldAst) {
				ImGui::SameLine(0, 20);
				ImGuiEx::toggleButton(ICON_MS_PACKAGE_2, &scriptData->_showByteCode, true); // Lingo
				ImGui::SetItemTooltip("Lingo");
				ImGui::SameLine();

				ImGuiEx::toggleButton(ICON_MS_STACKS, &scriptData->_showByteCode); // Bytecode
				ImGui::SetItemTooltip("Bytecode");
			}

			ImGui::Separator();
			childSize = ImGui::GetContentRegionAvail();
			ImGui::BeginChild("##script", childSize);

			if (!context || context->_functionHandlers.size() == 1) {
				renderScript(current, scriptData->_showByteCode, true);
			} else {
				for (auto &functionHandler : context->_functionHandlers) {
					if (current.handlerId == functionHandler._key) {
						renderScript(current, scriptData->_showByteCode, true);
					} else {
						ImGuiScript script = toImGuiScript(context->_scriptType, current.id, functionHandler._key);
						script.byteOffsets = context->_functionByteOffsets[script.handlerId];
						script.moviePath = movie->getArchive()->getPathName().toString();
						script.handlerName = getHandlerName(functionHandler._value);
						// Need to pass by reference in case of the current handler
						renderScript(script, scriptData->_showByteCode, false);
					}
					ImGui::NewLine();
				}
			}
			scriptsRendered = true;

			ImGui::EndChild();
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopID();

		ImGui::SameLine();

		const Common::Array<Window *> *windowList = g_director->getWindowList();

		for (auto window : (*windowList)) {
			g_director->setCurrentWindow(window);
			g_lingo->switchStateFromWindow();

			windowID += 1;
			ImGui::PushID(windowID);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

			ImGui::BeginChild("Window##", ImVec2(500.0f, 700.0f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);

			ImGui::Text("%s", window->asString().c_str());
			ImGui::Text("%s", window->getCurrentMovie()->getMacName().c_str());

			ImGui::SeparatorText("Backtrace");
			childSize = ImGui::GetContentRegionAvail();
			childSize.y /= 3;
			ImGui::BeginChild("##backtrace", childSize);
			renderCallStack(lingo->_state->pc);
			ImGui::EndChild();

			ImGui::SeparatorText("Scripts");
			scriptData = &_state->_functions._windowScriptData.getOrCreateVal(window);
			updateCurrentScript();

			if (scriptData->_showScript) {
				ImGuiScript &current = scriptData->_scripts[scriptData->_current];

				// Get all the handlers from the script
				ScriptContext* context = getScriptContext(current.id);

				if (context) {
					int castId = context->_id;
					if (castId == -1) {
						castId = movie->getCast()->getCastIdByScriptId(context->_parentNumber);
					}
					Common::String scriptInfo = Common::String::format("%d:%s type:%s", castId, context->getName().c_str(), scriptType2str(context->_scriptType));
					ImGui::Text("%s", scriptInfo.c_str());
				}

				ImGui::BeginDisabled(scriptData->_scripts.empty() || scriptData->_current == 0);
				if (ImGui::Button(ICON_MS_ARROW_BACK)) {
					scriptData->_current--;
				}
				ImGui::EndDisabled();
				ImGui::SetItemTooltip("Backward");
				ImGui::SameLine();

				ImGui::BeginDisabled(scriptData->_current >= scriptData->_scripts.size() - 1);
				if (ImGui::Button(ICON_MS_ARROW_FORWARD)) {
					scriptData->_current++;
				}
				ImGui::EndDisabled();
				ImGui::SetItemTooltip("Forward");
				ImGui::SameLine();

				const char *currentScript = nullptr;

				if (scriptData->_current < scriptData->_scripts.size()) {
					currentScript = scriptData->_scripts[scriptData->_current].handlerName.c_str();
				}

				if (ImGui::BeginCombo("##handlers", currentScript)) {
					for (uint i = 0; i < scriptData->_scripts.size(); i++) {
						auto &script = scriptData->_scripts[i];
						bool selected = i == scriptData->_current;
						if (ImGui::Selectable(script.handlerName.c_str(), &selected)) {
							scriptData->_current = i;
						}
					}
					ImGui::EndCombo();
				}

				if (!scriptData->_scripts[scriptData->_current].oldAst) {
					ImGui::SameLine(0, 20);
					ImGuiEx::toggleButton(ICON_MS_PACKAGE_2, &scriptData->_showByteCode, true); // Lingo
					ImGui::SetItemTooltip("Lingo");
					ImGui::SameLine();

					ImGuiEx::toggleButton(ICON_MS_STACKS, &scriptData->_showByteCode); // Bytecode
					ImGui::SetItemTooltip("Bytecode");
				}

				ImGui::Separator();
				childSize = ImGui::GetContentRegionAvail();
				ImGui::BeginChild("##script", childSize);

				if (!context || context->_functionHandlers.size() == 1) {
					renderScript(current, scriptData->_showByteCode, true);
				} else {
					for (auto &functionHandler : context->_functionHandlers) {
						if (current.handlerId == functionHandler._key) {
							renderScript(current, scriptData->_showByteCode, true);
						} else {
							ImGuiScript script = toImGuiScript(context->_scriptType, current.id, functionHandler._key);
							script.byteOffsets = context->_functionByteOffsets[script.handlerId];
							script.moviePath = movie->getArchive()->getPathName().toString();
							script.handlerName = getHandlerName(functionHandler._value);
							// Need to pass by reference in case of the current handler
							renderScript(script, scriptData->_showByteCode, false);
						}
						ImGui::NewLine();
					}
				}

				scriptsRendered = true;
				ImGui::EndChild();
			}

			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::PopID();
		}

		// Mark the scripts not dirty after all the handlers have been rendered
		_state->_dbg._isScriptDirty = !scriptsRendered;

		g_director->setCurrentWindow(currentWindow);
		g_lingo->switchStateFromWindow();
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
