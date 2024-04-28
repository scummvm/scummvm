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

#include "backends/imgui/imgui.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/debugtools.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

typedef struct ImGuiState {
	bool _showCallStack;
	bool _showVars;
	bool _showChannels;
} ImGuiState;

ImGuiState *_state = nullptr;

static void showCallStack() {
	if (!_state->_showCallStack)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("CallStack", &_state->_showCallStack)) {
		ImGui::Text("%s", lingo->formatCallStack(lingo->_state->pc).c_str());
	}
	ImGui::End();
}

static void showVars() {
	if (!_state->_showVars)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Vars", &_state->_showVars)) {
		ImGui::Text("%s", lingo->formatAllVars().c_str());
		ImGui::Separator();
		ImGuiIO &io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	}
	ImGui::End();
}

static void showChannels() {
	if (!_state->_showChannels)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Channels", &_state->_showChannels)) {
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
			ImGui::TableSetupColumn("CH", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("castId", ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("vis", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("inkData", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("ink", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("trails", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("stretch", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("line", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("dims", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("fg", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("bg", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("script", ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("colorcode", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("blendAmount", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("unk3", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("constraint", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("puppet", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("moveable", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("movieRate", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);
			ImGui::TableSetupColumn("movieTime", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader);

			ImGui::TableAngledHeadersRow();
			for (int i = 0; i < frame._numChannels; i++) {
				Channel &channel = *score->_channels[i + 1];
				Sprite &sprite = *channel._sprite;

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%-3d", i + 1);
				ImGui::TableNextColumn();

				if (sprite._castId.member) {
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
					ImGui::Text("%dx%d@%d,%d", channel._width, channel._height, channel._currentPoint.x, channel._currentPoint.y);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%s)", sprite._spriteType, spriteType2str(sprite._spriteType));
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._foreColor);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._backColor);
					ImGui::TableNextColumn();
					ImGui::Text("%s", sprite._scriptId.asString().c_str());
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
					ImGui::Text("%f", channel._movieRate);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%f)", channel._movieTime, (float)(channel._movieTime/60.0f));
				} else {
					ImGui::Text("000");
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void onImGuiInit() {
	_state = new ImGuiState();
	memset(_state, 0, sizeof(ImGuiState));
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("CallStack", NULL, &_state->_showCallStack);
			ImGui::MenuItem("Vars", NULL, &_state->_showVars);
			ImGui::MenuItem("Channels", NULL, &_state->_showChannels);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showVars();
	showCallStack();
	showChannels();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}
} // namespace Director
