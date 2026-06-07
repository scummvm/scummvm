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

#include "director/archive.h"
#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/movie.h"

namespace Director {
namespace DT {

static const char *searchModeNames[] = {
	"All",
	"Handlers",
	"Variables",
	"Body",
};

static bool containsQuery(const Common::String &s, const Common::String &query) {
	Common::String lower = s;
	lower.toLowercase();
	return lower.contains(query.c_str());
}

static void searchCast(Cast *cast, int castLibID, const Common::String &query, Director::Lingo *lingo, Movie *movie, SearchMode mode) {
	if (!cast || !cast->_lingoArchive)
		return;

	for (int i = 0; i <= kMaxScriptType; i++) {
		if (cast->_lingoArchive->scriptContexts[i].empty())
			continue;

		for (auto &scriptContext : cast->_lingoArchive->scriptContexts[i]) {
			// check property names once per script context, shared across all its handlers
			bool propertyMatch = false;
			if (mode == kSearchAll || mode == kSearchVariables) {
				for (const Common::String &name : scriptContext._value->getPropertyNames()) {
					if (containsQuery(name, query)) {
						propertyMatch = true;
						break;
					}
				}
			}

			// also search the cast member name itself
			bool castNameMatch = false;
			if (mode == kSearchAll || mode == kSearchHandlers) {
				CastMember *cm = cast->getCastMember(scriptContext._key, false);
				if (cm && containsQuery(cm->getName(), query))
					castNameMatch = true;
			}

			for (auto &functionHandler : scriptContext._value->_functionHandlers) {
				bool found = false;

				// handler name and cast member name
				if (mode == kSearchAll || mode == kSearchHandlers) {
					if (castNameMatch || containsQuery(functionHandler._key, query))
						found = true;
				}

				// variable names -> property (from ScriptContext), argument and global (from lingodec handler)
				if (!found && (mode == kSearchAll || mode == kSearchVariables)) {
					if (propertyMatch) {
						found = true;
					} else {
						CastMemberID memberID(scriptContext._key, castLibID);
						const LingoDec::Handler *ldHandler = getHandler(cast, memberID, functionHandler._key);
						if (ldHandler) {
							for (const Common::String &name : ldHandler->argumentNames) {
								if (containsQuery(name, query)) { found = true; break; }
							}
							if (!found) {
								for (const Common::String &name : ldHandler->globalNames) {
									if (containsQuery(name, query)) { found = true; break; }
								}
							}
						}
					}
				}

				// acript body via decoded bytecode
				if (!found && lingo && (mode == kSearchAll || mode == kSearchBody)) {
					Symbol &sym = functionHandler._value;
					if (sym.type == HANDLER && sym.u.defn) {
						uint pc = 0;
						while (pc < sym.u.defn->size()) {
							Common::String line = lingo->decodeInstruction(sym.u.defn, pc, &pc);
							if (containsQuery(line, query)) {
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
	ImGui::SetNextWindowSize(ImVec2(560, 320), ImGuiCond_FirstUseEver);

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
	// focus the input field when the window first opens
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();
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
		if (query.empty()) {
			// clear results immediately when the input is erased
			search.results.clear();
			_state->_dbg._highlightQuery = "";
		} else {
			runSearch(query, (SearchMode)search.mode);
		}
	}

	ImGui::Separator();
	ImGui::Text("Results: %d", (int)search.results.size());
	ImGui::Separator();

	ImVec2 tableSize = ImGui::GetContentRegionAvail();
	if (ImGui::BeginTable("##results", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_ScrollY, tableSize)) {
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Handler", 0, 200.f);
		ImGui::TableSetupColumn("Type", 0, 75.f);
		ImGui::TableSetupColumn("Cast Member", ImGuiTableColumnFlags_WidthStretch, 200.f);
		ImGui::TableHeadersRow();

		for (int i = 0; i < (int)search.results.size(); i++) {
			ImGuiScript &script = search.results[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (ImGui::Selectable(script.handlerName.c_str(), false,
					ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
				Common::String q(search.input);
				q.toLowercase();
				_state->_dbg._highlightQuery = q;
				addToOpenHandlers(script);
				_state->_dbg._goToDefinition = true;
			}

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(scriptType2str(script.type));

			ImGui::TableNextColumn();
			// show human-readable cast member name, falling back to the ID string
			Common::String castLabel = script.id.asString();
			Cast *resCast = nullptr;
			if (script.id.castLib == SHARED_CAST_LIB)
				resCast = movie->getSharedCast();
			else
				resCast = movie->getCasts()->getValOrDefault(script.id.castLib, nullptr);
			if (resCast) {
				CastMember *cm = resCast->getCastMember(script.id.member, false);
				if (cm) castLabel = getDisplayName(cm);
			}
			ImGui::TextUnformatted(castLabel.c_str());

			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	ImGui::End();
}


}
}
