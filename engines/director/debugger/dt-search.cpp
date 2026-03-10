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
#include "director/debugger/dt-internal.h"

#include "director/cast.h"
#include "director/castmember/text.h"
#include "director/movie.h"

namespace Director {
namespace DT {

static const char *searchModeNames[] = {
	"All",
	"Handler Names",
	"Script Body",
};

static void searchCast(Cast *cast, int castLibID, const Common::String &query, Director::Lingo *lingo, Movie *movie, SearchMode mode) {
	if (!cast) return;
	if (!cast->_lingoArchive) return;

	for (int i = 0; i <= kMaxScriptType; i++) {
		if (cast->_lingoArchive->scriptContexts[i].empty())
			continue;

		for (auto &scriptContext : cast->_lingoArchive->scriptContexts[i]) {
			for (auto &functionHandler : scriptContext._value->_functionHandlers) {
				bool found = false;

				// Search handler name
				if (mode == kSearchAll || mode == kSearchHandlerNames) {
					Common::String handlerLower = functionHandler._key;
					handlerLower.toLowercase();
					if (handlerLower.contains(query.c_str()))
						found = true;

					// Search property and global names
					if (!found) {
						CastMemberID memberID(scriptContext._key, castLibID);
						ImGuiScript script = toImGuiScript(scriptContext._value->_scriptType, memberID, functionHandler._key);
						for (auto &name : script.propertyNames) {
							Common::String n = name;
							n.toLowercase();
							if (n.contains(query.c_str())) { found = true; break; }
						}
						if (!found) {
							for (auto &name : script.globalNames) {
								Common::String n = name;
								n.toLowercase();
								if (n.contains(query.c_str())) { found = true; break; }
							}
						}
					}
				}

				// Search script body via decodeInstruction
				if (!found && lingo && (mode == kSearchAll || mode == kSearchScriptBody)) {
					Symbol &sym = functionHandler._value;
					if (sym.type == HANDLER && sym.u.defn) {
						uint pc = 0;
						while (pc < sym.u.defn->size()) {
							Common::String line = lingo->decodeInstruction(sym.u.defn, pc, &pc);
							line.toLowercase();
							if (line.contains(query.c_str())) {
								found = true;
								break;
							}
						}
					}
				}

				if (found) {
					CastMemberID memberID(scriptContext._key, castLibID);
					const Common::String moviePath = movie->getArchive()->getPathName().toString();
					ImGuiScript script = buildImGuiHandlerScript(scriptContext._value, castLibID, functionHandler._key, moviePath);
					Common::String handlerName;
					if (functionHandler._value.ctx && functionHandler._value.ctx->_id)
						handlerName = Common::String::format("%d:", functionHandler._value.ctx->_id);
					handlerName += g_lingo->formatFunctionName(functionHandler._value);
					script.handlerName = handlerName;
					_state->_search.results.push_back(script);
				}
			}
		}
	}
}

static void runSearch(const Common::String &query, SearchMode mode) {
	_state->_search.results.clear();

	Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return;

	Director::Lingo *lingo = g_director->getLingo();

	for (auto it : *movie->getCasts())
		searchCast(it._value, it._key, query, lingo, movie, mode);
	searchCast(movie->getSharedCast(), SHARED_CAST_LIB, query, lingo, movie, mode);
}

void showSearchBar() {
	if (!_state->_w.search)
		return;

	Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return;

	auto &search = _state->_search;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Search", &_state->_w.search)) {
		_state->_dbg._highlightQuery = "";
		ImGui::End();
		return;
	}

	ImGui::SetNextItemWidth(120);
	if (ImGui::Combo("##searchmode", &search.mode, searchModeNames, ARRAYSIZE(searchModeNames)))
		search.dirty = true;

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-80);
	if (ImGui::InputText("##search", search.input, sizeof(search.input),
			ImGuiInputTextFlags_EnterReturnsTrue))
		search.dirty = true;

	ImGui::SameLine();
	if (ImGui::Button("Search"))
		search.dirty = true;

	if (search.dirty) {
		search.dirty = false;
		Common::String query(search.input);
		query.toLowercase();
		if (!query.empty())
			runSearch(query, (SearchMode)search.mode);
	}

	// display results
	ImGui::Separator();
	ImGui::Text("Results: %d", (int)search.results.size());
	ImGui::Separator();

	if (ImGui::BeginTable("##results", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Handler", 0, 240.f);
		ImGui::TableSetupColumn("Cast", ImGuiTableColumnFlags_WidthStretch, 240.f);
		ImGui::TableHeadersRow();

		for (int i = 0; i < (int)search.results.size(); i++) {
			auto &script = search.results[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(script.handlerName.c_str())) {
				Common::String q(search.input);
				q.toLowercase();
				_state->_dbg._highlightQuery = q;

				addToOpenHandlers(script);
				_state->_dbg._goToDefinition = true;
			}
			ImGui::TableNextColumn();
			ImGui::Text("%s", script.id.asString().c_str());
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	ImGui::End();
}


}
}
