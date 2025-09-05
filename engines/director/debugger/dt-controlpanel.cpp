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
#include "director/movie.h"
#include "director/window.h"
#include "director/score.h"

namespace Director {
namespace DT {

static uint32 getLineFromPC() {
	ScriptData *scriptData = &_state->_functions._windowScriptData.getOrCreateVal(g_director->getCurrentWindow());

	const uint pc = g_lingo->_state->pc;
	if (scriptData->_scripts.empty())
		return 0;
	const Common::Array<uint> &offsets = scriptData->_scripts[scriptData->_current].startOffsets;
	for (uint i = 0; i < offsets.size(); i++) {
		if (pc <= offsets[i])
			return i;
	}
	return 0;
}

static bool stepOverShouldPauseDebugger() {
	const uint32 line = getLineFromPC();

	// we stop when we are :
	// - in the same callstack level and the statement line is different
	// - OR we go up in the callstack
	if (((g_lingo->_state->callstack.size() == _state->_dbg._callstackSize) && (line != _state->_dbg._lastLinePC)) ||
		 (g_lingo->_state->callstack.size() < _state->_dbg._callstackSize)) {
		_state->_dbg._lastLinePC = line;
		_state->_dbg._isScriptDirty = true;
		return true;
	}

	return false;
}

static bool stepInShouldPauseDebugger() {
	const uint32 line = getLineFromPC();

	// we stop when:
	// - the statement line is different
	// - OR when the callstack level change
	if ((g_lingo->_state->callstack.size() != _state->_dbg._callstackSize) || (_state->_dbg._lastLinePC != line)) {
		_state->_dbg._lastLinePC = line;
		_state->_dbg._isScriptDirty = true;
		return true;
	}

	return false;
}

static bool stepOutShouldPause() {
	const uint32 line = getLineFromPC();

	// we stop when:
	// - OR we go up in the callstack
	if (g_lingo->_state->callstack.size() < _state->_dbg._callstackSize) {
		_state->_dbg._lastLinePC = line;
		_state->_dbg._isScriptDirty = true;
		return true;
	}

	return false;
}

static void dgbStop() {
	g_lingo->_exec._state = kPause;
	g_lingo->_exec._shouldPause = nullptr;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepOver() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepOverShouldPauseDebugger;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepInto() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepInShouldPauseDebugger;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepOut() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepOutShouldPause;
	_state->_dbg._isScriptDirty = true;
}

void showControlPanel() {
	if (!_state->_w.controlPanel)
		return;

	ImVec2 vp(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImVec2(vp.x - 220.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 103), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Control Panel", &_state->_w.controlPanel, ImGuiWindowFlags_NoDocking)) {
		Movie *movie = g_director->getCurrentMovie();
		Score *score = movie->getScore();
		ImDrawList *dl = ImGui::GetWindowDrawList();

		ImU32 color = ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
		ImU32 color_red = ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
		ImU32 active_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
		ImU32 bgcolor = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 1.0f, 1.0f));
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImVec2 buttonSize(20, 14);
		float bgX1 = -4.0f, bgX2 = 21.0f;

		int frameNum = score->getCurrentFrameNum();

		if (_state->_prevFrame != -1 && _state->_prevFrame != frameNum) {
			score->_playState = kPlayPaused;
			_state->_prevFrame = -1;
		}

		{ // Rewind
			ImGui::InvisibleButton("Rewind", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				score->setCurrentFrame(1);
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 8, p.y), ImVec2(p.x + 8, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 8), ImVec2(p.x + 16, p.y), ImVec2(p.x + 16, p.y + 16), color);

			ImGui::SetItemTooltip("Rewind");
			ImGui::SameLine();
		}

		{ // Step Back
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Back", ImVec2(18, 16));

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum - 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 9, p.y), ImVec2(p.x + 9, p.y + 16), color);
			dl->AddRectFilled(ImVec2(p.x + 11, p.y), ImVec2(p.x + 17, p.y + 16), color);

			ImGui::SetItemTooltip("Step Back");
			ImGui::SameLine();
		}

		{ // Stop
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Stop", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayPaused;
				g_lingo->_exec._state = kPause;
				g_lingo->_exec._shouldPause = nullptr;
				_state->_dbg._isScriptDirty = true;

				g_system->displayMessageOnOSD(Common::U32String("Paused"));
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			ImU32 stopColor = (score->_playState == kPlayPaused || score->_playState == kPlayPausedAfterLoading) ? active_color : color;
			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 16, p.y + 16), stopColor);

			ImGui::SetItemTooltip("Stop");
			ImGui::SameLine();
		}

		{ // Step
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum + 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 6, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 2), ImVec2(p.x + 8, p.y + 14), ImVec2(p.x + 16, p.y + 8), color);

			ImGui::SetItemTooltip("Step");
			ImGui::SameLine();
		}

		{ // Play
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Play", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				if (score->_playState == kPlayPausedAfterLoading)
					score->_playState = kPlayLoaded;
				else
					score->_playState = kPlayStarted;

				g_lingo->_exec._state = kRunning;
				g_lingo->_exec._shouldPause = nullptr;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			if (score->_playState == kPlayStarted)
				color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 1.0f, 1.0f));

			dl->AddTriangleFilled(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 16), ImVec2(p.x + 14, p.y + 8), color);

			ImGui::SetItemTooltip("Play");
			ImGui::SameLine();
		}

		char buf[6];

		snprintf(buf, 6, "%d", score->getCurrentFrameNum());

		ImGui::SetNextItemWidth(35);
		ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
		ImGui::SetItemTooltip("Frame");

		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.5f, 1.0f), movie->getArchive()->getPathName().toString().c_str());
			ImGui::SetItemTooltip(movie->getArchive()->getPathName().toString().c_str());
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Text("Lingo:");
		ImGui::SameLine();
		{ // Step over
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Over", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepOver();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->PathArcToFast(ImVec2(p.x + 9, p.y + 15), 10.0f, 7, 11);
			dl->PathStroke(color_red, 0, 2);
			dl->AddLine(ImVec2(p.x + 18, p.y + 5), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 14, p.y + 10), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Over");
			ImGui::SameLine();
		}

		{ // Step into
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Into", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepInto();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Into");
			ImGui::SameLine();
		}

		{ // Step out
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Out", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepOut();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Out");
		}
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
