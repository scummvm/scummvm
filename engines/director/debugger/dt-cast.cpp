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

#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/text.h"
#include "director/castmember/shape.h"
#include "director/castmember/richtext.h"
#include "director/castmember/script.h"
#include "director/movie.h"
#include "director/types.h"
#include "director/window.h"

namespace Director {
namespace DT {

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
		"",                           // Empty
		ICON_MS_BACKGROUND_DOT_LARGE, // Bitmap
		ICON_MS_THEATERS,             // FilmLoop
		ICON_MS_MATCH_CASE,           // Text
		ICON_MS_PALETTE,              // Palette
		ICON_MS_IMAGESMODE,           // Picture
		ICON_MS_VOLUME_UP,            // Sound
		ICON_MS_SLAB_SERIF,           // Button
		ICON_MS_SHAPES,               // Shape
		ICON_MS_MOVIE,                // Movie
		ICON_MS_ANIMATED_IMAGES,      // DigitalVideo
		ICON_MS_FORMS_APPS_SCRIPT,    // Script
		ICON_MS_BRAND_FAMILY,         // RTE
		"?",                          // ???
		ICON_MS_TRANSITION_FADE};     // Transition
	if (castType < 0 || castType > kCastTransition)
		return "";
	return castTypes[(int)castType];
}

const char *toString(CastType castType) {
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
		"OLE",
		"Transition",
		"Xtra",
	};
	if (castType < 0 || castType > kCastXtra)
		return "???";
	return castTypes[(int)castType];
}

Common::String getDisplayName(CastMember *castMember) {
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

struct CastRowEntry {
	const Cast *cast = nullptr;
	CastMember *member = nullptr;
	int id = 0;
	Common::String name;
};

// Collects the cast's members that pass the filters, sorted by member number.
static void gatherCastMembers(const Cast *cast, Common::Array<CastRowEntry> &rows) {
	if (!cast || !cast->_loadedCast)
		return;

	Common::Array<int> ids;
	for (auto &it : *cast->_loadedCast)
		ids.push_back(it._key);
	Common::sort(ids.begin(), ids.end());

	for (int id : ids) {
		CastMember *member = cast->_loadedCast->getVal(id);
		member->load();

		Common::String name(getDisplayName(member));
		if (!_state->_cast._nameFilter.PassFilter(name.c_str()))
			continue;
		if ((member->_type != kCastTypeAny) &&
				!(_state->_cast._typeFilter & (1 << (int)member->_type)))
			continue;

		CastRowEntry entry;
		entry.cast = cast;
		entry.member = member;
		entry.id = id;
		entry.name = name;
		rows.push_back(entry);
	}
}

static ImGuiImage getThumbnail(CastMember *member) {
	switch (member->_type) {
	case kCastBitmap:
		return getImageID(member);
	case kCastText:
	case kCastRichText:
	case kCastButton:
		return getTextID(member);
	case kCastShape:
		return getShapeID(member);
	default:
		return {};
	}
}

static void drawCastRow(const CastRowEntry &entry) {
	// member numbers repeat across cast libs, so scope the row IDs
	ImGui::PushID(entry.cast->_castLibID);
	ImGui::PushID(entry.id);

	ImGui::TableNextRow();

	// Make the entire row selectable/clickable
	ImGui::TableSetColumnIndex(0);
	if (ImGui::Selectable("##row", false,
			ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
			ImVec2(0, 32.f) // match row height
	)) {
		_state->_castDetails._castMemberID = CastMemberID(entry.id, entry.cast->_castLibID);
		_state->_castDetails._window = _state->_castWindow;
		_state->_w.castDetails = true;
	}
	ImGui::SameLine();

	ImGui::Text("%s %s", toIcon(entry.member->_type), entry.name.c_str());

	ImGui::TableNextColumn();
	ImGui::Text("%d", entry.id);

	ImGui::TableNextColumn();
	if (entry.member->_type == CastType::kCastLingoScript) {
		ScriptCastMember *scriptMember = (ScriptCastMember *)entry.member;
		ImGui::Text("%s", toString(scriptMember->_scriptType));
	}
	ImGui::TableNextColumn();
	ImGui::Text("%s", toString(entry.member->_type));

	ImGui::TableNextColumn();
	float columnWidth = ImGui::GetColumnWidth();

	ImGuiImage imgID = getThumbnail(entry.member);
	if (imgID.id) {
		float offsetX = (columnWidth - 32.f) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
		showImage(imgID, entry.name.c_str(), 32.f);
	}

	ImGui::PopID();
	ImGui::PopID();
}

static void drawCastTile(const CastRowEntry &entry, float thumbnailSize) {
	// member numbers repeat across cast libs, so scope the tile IDs
	ImGui::PushID(entry.cast->_castLibID);
	ImGui::PushID(entry.id);

	// show the member number so tiles can be identified without clicking
	Common::String label = Common::String::format("%d: %s", entry.id, entry.name.c_str());

	ImGui::BeginGroup();
	const ImVec2 textSize = ImGui::CalcTextSize(entry.name.c_str());
	float textWidth = textSize.x;
	float textHeight = textSize.y;
	if (textWidth > thumbnailSize) {
		textWidth = thumbnailSize;
		textHeight *= (textSize.x / textWidth);
	}

	ImGuiImage imgID = getThumbnail(entry.member);
	if (imgID.id) {
		showImage(imgID, label.c_str(), thumbnailSize);
	} else {
		ImGui::InvisibleButton("##canvas", ImVec2(thumbnailSize, thumbnailSize));
		const ImVec2 p0 = ImGui::GetItemRectMin();
		const ImVec2 p1 = ImGui::GetItemRectMax();
		ImGui::PushClipRect(p0, p1, true);
		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRect(p0, p1, _state->theme->borderColor);
		const ImVec2 pos = p0 + ImVec2((thumbnailSize - textWidth) * 0.5f, (thumbnailSize - textHeight) * 0.5f);
		draw_list->AddText(nullptr, 0.f, pos, _state->theme->gridTextColor, entry.name.c_str(), 0, thumbnailSize);
		draw_list->AddText(nullptr, 0.f, p1 - ImVec2(16, 16), _state->theme->gridTextColor, toIcon(entry.member->_type));
		ImGui::PopClipRect();
	}
	ImGui::EndGroup();

	if (!imgID.id)
		ImGui::SetItemTooltip("%s", label.c_str());

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		// Cast Member Clicked
		_state->_castDetails._castMemberID = CastMemberID(entry.id, entry.cast->_castLibID);
		_state->_castDetails._window = _state->_castWindow;
		_state->_w.castDetails = true;
	}

	ImGui::PopID();
	ImGui::PopID();
}

void showCast() {
	if (!_state->_w.cast)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 480), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast", &_state->_w.cast)) {
		Window *selectedWindow = windowListCombo(&_state->_castWindow);

		// display a toolbar with: grid/list/filters buttons + name filter
		if (selectableViewButton(ICON_MS_LIST, _state->_cast._listView))
			_state->_cast._listView = true;
		ImGui::SetItemTooltip("List");
		ImGui::SameLine();
		if (selectableViewButton(ICON_MS_GRID_VIEW, !_state->_cast._listView))
			_state->_cast._listView = false;
		ImGui::SetItemTooltip("Grid");
		ImGui::SameLine();

		if (ImGui::Button(ICON_MS_FILTER_ALT)) {
			ImGui::OpenPopup("filters_popup");
		}
		ImGui::SameLine();

		if (ImGui::BeginPopup("filters_popup")) {
			ImGui::CheckboxFlags("All", &_state->_cast._typeFilter, 0xFFFF);
			ImGui::Separator();
			for (int i = 0; i <= (int)kCastXtra; i++) {
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
		Movie *movie = selectedWindow->getCurrentMovie();
		ImGui::BeginChild("##cast", ImVec2(childsize.x, childsize.y - sliderHeight));

		Common::Array<CastRowEntry> rows;
		for (auto it : *movie->getCasts())
			gatherCastMembers(it._value, rows);
		gatherCastMembers(movie->getSharedCast(), rows);

		if (_state->_cast._listView) {
			if (ImGui::BeginTable("Resources", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Name", 0, 120.f);
				ImGui::TableSetupColumn("#", 0, 20.f);
				ImGui::TableSetupColumn("Script", 0, 80.f);
				ImGui::TableSetupColumn("Type", 0, 80.f);
				ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 50.f);
				ImGui::TableHeadersRow();

				// only submit the visible rows, not the whole cast, each frame
				ImGuiListClipper clipper;
				clipper.Begin((int)rows.size());
				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						drawCastRow(rows[i]);
				}

				ImGui::EndTable();
			}
		} else {
			const float thumbnailSize = (float)_state->_cast._thumbnailSize;
			const float contentWidth = ImGui::GetContentRegionAvail().x;
			int columns = contentWidth / (thumbnailSize + 8.f);
			columns = columns < 1 ? 1 : columns;
			if (ImGui::BeginTable("Cast", columns)) {
				ImGuiListClipper clipper;
				clipper.Begin(((int)rows.size() + columns - 1) / columns);
				while (clipper.Step()) {
					for (int r = clipper.DisplayStart; r < clipper.DisplayEnd; r++) {
						ImGui::TableNextRow();
						for (int c = 0; c < columns; c++) {
							int index = r * columns + c;
							if (index >= (int)rows.size())
								break;
							ImGui::TableNextColumn();
							drawCastTile(rows[index], thumbnailSize);
						}
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

} // namespace DT
} // namespace Director
