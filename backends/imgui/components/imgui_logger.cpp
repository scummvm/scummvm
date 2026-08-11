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
#include "common/debug-channels.h"
#include "common/debug.h"

#include "backends/imgui/components/imgui_logger.h"

namespace ImGuiEx {

ImGuiLogger::ImGuiLogger() {
	clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));
	_historyPos = -1;
	_autoScroll = true;
	_scrollToBottom = false;
}

ImGuiLogger::~ImGuiLogger() {
	clear();
	for (int i = 0; i < _history.Size; i++)
		free(_history[i]);
}

void ImGuiLogger::clear() {
	for (int i = 0; i < _items.Size; i++)
		free(_items[i]);
	_items.clear();
}

void ImGuiLogger::addLog(const char *fmt, ...) {
	// FIXME-OPT
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	_items.push_back(scumm_strdup(buf));
}

void ImGuiLogger::draw(const char *title, bool *p_open) {
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
	if (ImGui::Button(ICON_MS_CLEAR_ALL)) {
		clear();
	}
	ImGui::SetItemTooltip("Clear");
	ImGui::SameLine();

	// Copy
	bool copy_to_clipboard = ImGui::Button(ICON_MS_CONTENT_COPY);
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
	if (ImGui::Button(ICON_MS_SETTINGS))
		ImGui::OpenPopup("Options");
	ImGui::SetItemTooltip("Options");
	ImGui::SameLine();

	ImGui::Spacing();
	ImGui::SameLine();

	// Error
	ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_error_b);
	ImGuiEx::toggleButton("\ue160", &_showError);
	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Show Errors");
	ImGui::SameLine();

	// Warning
	ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_warning_b);
	ImGuiEx::toggleButton("\ue002", &_showWarn);
	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Show Warnings");
	ImGui::SameLine();

	// Info
	ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_info_b);
	ImGuiEx::toggleButton(ICON_MS_INFO, &_showInfo);
	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Show Info");
	ImGui::SameLine();

	// Debug
	ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_debug_b);
	ImGuiEx::toggleButton(ICON_MS_BUG_REPORT, &_showdebug);
	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Show Debug");
	ImGui::SameLine();

	_filter.Draw("Filter (\"incl,-excl\") (\"warn\")", 180);
	ImGui::Separator();

	ImGui::BeginChild("ScrollingRegion", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::Selectable(ICON_MS_CLEAR_ALL " Clear"))
			clear();
		if (ImGui::Selectable(ICON_MS_CONTENT_COPY " Copy"))
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
			ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_error);
			pop_color = true;
		} else if (isWarn) {
			item += 6;
			ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_warning);
			pop_color = true;
		} else if (isDebug) {
			item += 7;
			ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_debug);
			pop_color = true;
		} else if (strncmp(item, "> ", 2) == 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, _colors._logger_info);
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

void ImGuiLogger::drawColorOptions() {
	ImGui::SeparatorText("Logger");
	ImGui::ColorEdit4("Error", &_colors._logger_error.x);
	ImGui::ColorEdit4("Error Button", &_colors._logger_error_b.x);
	ImGui::ColorEdit4("Warning", &_colors._logger_warning.x);
	ImGui::ColorEdit4("Warning Button", &_colors._logger_warning_b.x);
	ImGui::ColorEdit4("Info", &_colors._logger_info.x);
	ImGui::ColorEdit4("Info Button", &_colors._logger_info_b.x);
	ImGui::ColorEdit4("Debug", &_colors._logger_debug.x);
	ImGui::ColorEdit4("Debug Button", &_colors._logger_debug_b.x);
}

} // namespace ImGuiEx
