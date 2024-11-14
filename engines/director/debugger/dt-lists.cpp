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
#include "director/archive.h"
#include "director/debugger.h"
#include "director/debugger/dt-internal.h"

#include "director/lingo/lingo-object.h"

namespace Director {
namespace DT {

void showCallStack() {
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

static void cacheVars() {
	// take a snapshot of the variables every 500 ms
	if ((g_director->getTotalPlayTime() - _state->_vars._lastTimeRefreshed) > 500) {
		_state->_vars._prevLocals = _state->_vars._locals;
		if (g_lingo->_state->localVars) {
			_state->_vars._locals = *g_lingo->_state->localVars;
		} else {
			_state->_vars._locals.clear();
		}

		_state->_vars._prevGlobals = _state->_vars._globals;
		_state->_vars._globals = g_lingo->_globalvars;
		_state->_vars._lastTimeRefreshed = g_director->getTotalPlayTime();
	}
}

void showVars() {
	if (!_state->_w.vars)
		return;

	cacheVars();

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
				bool changed = !_state->_vars._prevGlobals.contains(i) || !(_state->_vars._globals.getVal(i) == _state->_vars._prevGlobals.getVal(i));
				displayVariable(i, changed);
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
					bool changed = !_state->_vars._prevLocals.contains(i) || !(_state->_vars._locals.getVal(i) == _state->_vars._prevLocals.getVal(i));
					displayVariable(i, changed);
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
					displayVariable(i, false);
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

void showWatchedVars() {
	if (!_state->_w.watchedVars)
		return;

	cacheVars();

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Watched Vars", &_state->_w.watchedVars)) {
		for (auto &v : _state->_variables) {
			Datum name(v._key);
			name.type = VARREF;
			Datum val = g_lingo->varFetch(name, true);

			displayVariable(v._key, false);
			ImGui::SameLine();
			ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
		}

		if (_state->_variables.empty())
			ImGui::Text("(no watched variables)");
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

void showBreakpointList() {
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

void showArchive() {
	if (!_state->_w.archive)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size * 0.8f;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Archive", &_state->_w.archive)) {
		{ // Left pane
			ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

			for (auto &it : g_director->_allSeenResFiles) {
				Archive *archive = it._value;

				if (ImGui::TreeNode(archive->getPathName().toString().c_str())) {
					Common::Array<uint32> typeList = archive->getResourceTypeList();
					Common::sort(typeList.begin(), typeList.end());

					for (auto tag : typeList) {
						if (ImGui::TreeNode((void*)(intptr_t)tag, "%s", tag2str(tag))) {
							Common::Array<uint16> resList = archive->getResourceIDList(tag);
							Common::sort(resList.begin(), resList.end());

							for (auto id : resList) {
								if (ImGui::Selectable(Common::String::format("%d", id).c_str())) {
									_state->_archive.path = it._key;
									_state->_archive.resType = tag;
									_state->_archive.resId = id;

									free(_state->_archive.data);

									Common::SeekableReadStreamEndian *res = archive->getResource(tag, id);
									_state->_archive.data = (byte *)malloc(res->size());
									res->read(_state->_archive.data, res->size());
									_state->_archive.dataSize = res->size();

									delete res;
								}
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

			ImGui::Text("Resource %s %d (%d bytes)", tag2str(_state->_archive.resType), _state->_archive.resId, _state->_archive.dataSize);

			ImGui::Separator();

			if (!_state->_archive.path.empty())
				_state->_archive.memEdit.DrawContents(_state->_archive.data, _state->_archive.dataSize);

			ImGui::EndChild();
		}

	}
	ImGui::End();
}


} // namespace DT
} // namespace Director
