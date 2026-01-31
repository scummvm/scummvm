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

#include "backends/imgui/IconsMaterialSymbols.h"
#include "backends/imgui/imgui_utils.h"

#include "director/cast.h"
#include "director/castmember/castmember.h"

namespace Director {
namespace DT {

void showCastDetails() {
	if (!_state->_w.castDetails)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 480), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast Details"), &_state->_w.castDetails) {
		CastMember *_member = _state->_castDetails._castMember;
		assert(_member != nullptr);

		CastType _type = _member->_type;
		CastMemberInfo *_info = _member->getInfo();

		// Columns to display details on the right of the bitmap
		ImGui::Columns(2, nullptr, false);
		ImGui::SetColumnWidth(0, 75.f);

		// Show Bitmap
		ImGuiImage imgID = getImageID(_member);
		showImage(imgID, _member->getName().c_str(), 64.f);

		// Move to the right of the Bitmap
		ImGui::NextColumn();

		// Show Name of _member
		ImGui::Text(getDisplayName(_member).c_str());
		ImGui::Text(castType2str(_type));

		if (_info == nullptr) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", "[No Cast Member Info]");
			ImGui::End();
			return;
		}

		ImGui::End();
	}
}

} // namespace DT
} // namespace Director
