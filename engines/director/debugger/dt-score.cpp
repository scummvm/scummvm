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
#include "director/director.h"
#include "director/debugger/dt-internal.h"

#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"

namespace Director {
namespace DT {

enum { kModeMember, kModeBehavior, kModeLocation, kModeInk, kModeBlend, kModeExtended,
		kChTempo, kChPalette, kChTransition, kChSound1, kChSound2, kChScript };
const char *modes[] = { "Member", "Behavior", "Location", "Ink", "Blend", "Extended" };
const char *modes2[] = {
	ICON_MS_TIMER, "Tempo",					// timer
	ICON_MS_PALETTE, "Palette",				// palette
	ICON_MS_TRANSITION_FADE, "Transition",	// transition_fade
	ICON_MS_VOLUME_UP,"Sound 1",			// volume_up
	ICON_MS_VOLUME_DOWN,"Sound 2",			// volume_up
	ICON_MS_FORMS_APPS_SCRIPT, "Script",	// forms_apps_script
};

// These are the score channel colors coming from the Authoring Tool
const uint32 scoreColors[6] = {
	0xceceff,
	0xffffce,
	0xceffce,
	0xceffff,
	0xffceff,
	0xffce9c,
};

static void buildContinuationData() {
	if (_state->_loadedContinuationData) {
		return;
	}

	Score *score = g_director->getCurrentMovie()->getScore();
	uint numFrames = score->_scoreCache.size();

	uint numChannels = score->_scoreCache[0]->_sprites.size();
	_state->_continuationData.resize(numChannels);

	for (int ch = 0; ch < (int)numChannels; ch++) {
		_state->_continuationData[ch].resize(numFrames);

		uint currentContinuation = 1;
		for (int f = 0; f < (int)numFrames; f++) {
			Frame &frame = *score->_scoreCache[f];
			Sprite &sprite = *frame._sprites[ch];

			Frame *prevFrame = (f == 0) ? nullptr : score->_scoreCache[f - 1];
			Sprite *prevSprite = (prevFrame) ? prevFrame->_sprites[ch] : nullptr;

			if (prevSprite) {
				if (!(*prevSprite == sprite)) {
					currentContinuation = f;
				}
			} else {
				currentContinuation = f;
			}
			_state->_continuationData[ch][f].first = currentContinuation;
		}

		currentContinuation = 1;
		for (int f = (int)numFrames - 1; f >= 0; f--) {
			Frame &frame = *score->_scoreCache[f];
			Sprite &sprite = *frame._sprites[ch];

			Frame *nextFrame = (f == (int)numFrames - 1) ? nullptr : score->_scoreCache[f + 1];
			Sprite *nextSprite = (nextFrame) ? nextFrame->_sprites[ch] : nullptr;

			if (nextSprite) {
				if (!(*nextSprite == sprite)) {
					currentContinuation = f;
				}
			} else {
				currentContinuation = f;
			}
			_state->_continuationData[ch][f].second = currentContinuation;
		}
	}

	_state->_loadedContinuationData = true;
}

static void displayScoreChannel(int ch, int mode, int modeSel) {
	Score *score = g_director->getCurrentMovie()->getScore();
	uint numFrames = score->_scoreCache.size();

	const uint currentFrameNum = score->getCurrentFrameNum();
	const ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.0f, 0.65f));

	ImGui::TableNextRow();

	ImGui::PushFont(_state->_tinyFont);

	if (modeSel == kModeExtended && mode == kModeExtended)
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));

	{
		ImGui::TableNextColumn();

		float indentSize = 17.0f;

		if (mode < kChTempo && modeSel == kModeExtended)
			indentSize = 10.0f;

		if (modeSel == kModeExtended && mode == kModeExtended)
			indentSize = 0.1f;

		ImGui::Indent(indentSize);

		if (mode >= kChTempo) {
			ImGui::PushFont(ImGui::GetIO().FontDefault);

			ImGui::Text(modes2[(mode - kChTempo) * 2]);
			ImGui::SetItemTooltip(modes2[(mode - kChTempo) * 2 + 1]);

			ImGui::PopFont();
		} else if (modeSel != kModeExtended || mode == kModeExtended) {
			ImGui::Text("%3d", ch);
		} else {
			ImGui::Text(modes[mode]);
		}

		if (ch == _state->_selectedScoreCast.channel)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.6f)));
		else
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

		ImGui::Unindent(indentSize);
	}

	numFrames -= _state->_scoreFrameOffset - 1;
	numFrames = MIN<uint>(numFrames, kMaxColumnsInTable - 2);

	for (int f = 0; f < (int)numFrames; f++) {
		Frame &frame = *score->_scoreCache[f + _state->_scoreFrameOffset - 1];
		Sprite &sprite = *frame._sprites[ch];

		ImGui::TableNextColumn();

		int startCont = _state->_continuationData[ch][f].first;
		int endCont = _state->_continuationData[ch][f].second;
		ImGui::PushID(ch * 10000 + f);

		if (f + _state->_scoreFrameOffset == (int)currentFrameNum)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);

		if (!(startCont == endCont) && (sprite._castId.member || sprite.isQDShape())) {
			if (f == startCont) {
				_state->_colors._contColorIndex = (_state->_colors._contColorIndex + 1) % _state->_colors._contColorCount;
			}

			if (_state->_selectedScoreCast.frame + _state->_scoreFrameOffset - 1 >= startCont &&
				_state->_selectedScoreCast.frame + _state->_scoreFrameOffset - 1 <= endCont &&
				ch == _state->_selectedScoreCast.channel &&
				mode <= kModeExtended) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, _state->_colors._channel_selected_col);
			} else if (_state->_hoveredScoreCast.frame >= startCont &&
				_state->_hoveredScoreCast.frame <= endCont &&
				ch == _state->_hoveredScoreCast.channel) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, _state->_colors._channel_hovered_col);
			} else {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, _state->_colors._contColors[_state->_colors._contColorIndex]);
			}
		}

		float startX = ImGui::GetCursorScreenPos().x;
		float width = ImGui::GetColumnWidth(ImGui::TableGetColumnIndex()) * (endCont - startCont + 1);

		Common::String string = Common::String("");
		switch (mode) {
		case kModeMember:
			if (sprite._castId.member)
				string += Common::String::format("%d", sprite._castId.member).c_str();
			else if (sprite.isQDShape())
				string += "Q";
			else
				string += "  ";
			break;

		case kModeInk:
			string += Common::String::format("%s", inkType2str(sprite._ink)).c_str();
			break;

		case kModeLocation:
			string += Common::String::format("%d, %d", sprite._startPoint.x, sprite._startPoint.y);
			break;

		case kModeBlend:
			string += Common::String::format("%d", sprite._blendAmount);
			break;

		case kModeBehavior:
			displayScriptRef(sprite._scriptId);
			break;

		case kChTempo:
			if (frame._mainChannels.tempo)
				string += Common::String::format("%d", frame._mainChannels.tempo);
			break;

		case kChPalette:
			if (frame._mainChannels.palette.paletteId.member)
				string += Common::String::format("%d", frame._mainChannels.palette.paletteId.member);
			break;

		case kChTransition:
			if (frame._mainChannels.transType)
				string += Common::String::format("%d", frame._mainChannels.transType);
			break;

		case kChSound1:
			if (frame._mainChannels.sound1.member)
				string += Common::String::format("%d", frame._mainChannels.sound1.member);
			break;

		case kChSound2:
			if (frame._mainChannels.sound2.member)
				string += Common::String::format("%d", frame._mainChannels.sound2.member);
			break;

		case kChScript:
			displayScriptRef(frame._mainChannels.actionId);
			break;

		case kModeExtended: // Render empty row
		default:
			string += "  ";
		}

		const char *text = string.c_str();
		ImGui::Text("%s", text);

		// // Center text inside span
		// ImVec2 textSize = ImGui::CalcTextSize(text);
		// float cursorX = startX + (width - textSize.x) * 0.5f;
		// ImGui::SetCursorScreenPos(ImVec2(cursorX, ImGui::GetCursorScreenPos().y));

		// ImGui::TextUnformatted(text);
		ImGui::PopID();

		if (ImGui::IsItemClicked(0)) {
			_state->_selectedScoreCast.frame = f + _state->_scoreFrameOffset - 1;
			_state->_selectedScoreCast.channel = ch;

			if (f + _state->_scoreFrameOffset == (int)currentFrameNum) {
				if (_state->_selectedChannel == ch)
					_state->_selectedChannel = -1;
				else
					_state->_selectedChannel = ch;

				g_director->getCurrentWindow()->render(true);
			}
		}

		if (ImGui::IsItemHovered()) {
			_state->_hoveredScoreCast.frame = f;
			_state->_hoveredScoreCast.channel = ch;
		}
	}

	ImGui::PopFont();
}

void showScore() {
	if (!_state->_w.score)
		return;

	buildContinuationData();

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Score", &_state->_w.score)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		uint numFrames = score->_scoreCache.size();
		Cast *cast = g_director->getCurrentMovie()->getCast();

		if (!numFrames) {
			ImGui::Text("No frames");
			ImGui::End();

			return;
		}

		if (_state->_selectedScoreCast.frame >= (int)numFrames)
			_state->_selectedScoreCast.frame = 0;

		if (!numFrames || _state->_selectedScoreCast.channel >= (int)score->_scoreCache[0]->_sprites.size())
			_state->_selectedScoreCast.channel = 0;

		if (_state->_scoreFrameOffset >= (int) numFrames)
			_state->_scoreFrameOffset = 1;

		{ // Render sprite details
			Sprite *sprite = nullptr;
			CastMember *castMember = nullptr;
			bool shape = false;

			if (_state->_selectedScoreCast.frame != -1)
				sprite = score->_scoreCache[_state->_selectedScoreCast.frame]->_sprites[_state->_selectedScoreCast.channel];

			if (sprite) {
				castMember = cast->getCastMember(sprite->_castId.member, true);

				shape = sprite->isQDShape();
			}

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Image", ImVec2(200.0f, 70.0f));

			if (castMember || shape) {
				ImGuiImage imgID = {};

				if (castMember) {
					switch (castMember->_type) {
					case kCastBitmap:
						imgID = getImageID(castMember);
						break;
					case kCastShape:
						imgID = getShapeID(castMember);
						break;
					case kCastText:
					case kCastButton:
					case kCastRichText:
						imgID = getTextID(castMember);
						break;

					default:
						break;
					}
				}

				if (castMember && imgID.id) {
					Common::String name(getDisplayName(castMember));
					showImage(imgID, name.c_str(), 32.f);
				} else {
					ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
				}
				ImGui::SameLine();
				ImGui::Text("%s", sprite->_castId.asString().c_str());
				ImGui::Text("%s", spriteType2str(sprite->_spriteType));
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginChild("Details", ImVec2(500.0f, 70.0f));

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Ink", ImVec2(150.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Text("%s", inkType2str(sprite->_ink));
				ImGui::SameLine(70);
				ImGui::SetItemTooltip("Ink");
				ImGui::Text("|");
				ImGui::SameLine();
				ImGui::Text("%d", sprite->_blendAmount);
				ImGui::SameLine();
				ImGui::SetItemTooltip("Blend");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Range", ImVec2(100.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Text("\uf816"); ImGui::SameLine();	// line_start_circle
				// the continuation data is 0-indexed but the frames are 1-indexed
				ImGui::Text("%4d", _state->_continuationData[_state->_selectedScoreCast.channel][_state->_selectedScoreCast.frame].first + 1); ImGui::SameLine(50);
				ImGui::SetItemTooltip("Start Frame");
				ImGui::Text("\uf819"); ImGui::SameLine();	// line_end_square
				// the continuation data is 0-indexed but the frames are 1-indexed
				ImGui::Text("%4d", _state->_continuationData[_state->_selectedScoreCast.channel][_state->_selectedScoreCast.frame].second + 1); ImGui::SameLine();
				ImGui::SetItemTooltip("End Frame");
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Flags", ImVec2(200.0f, 20.0f));

			if (castMember || shape) {
				ImGui::Checkbox(ICON_MS_LOCK, &sprite->_enabled); ImGui::SameLine();
				ImGui::SetItemTooltip("enabled");
				ImGui::Checkbox(ICON_MS_EDIT_NOTE, &sprite->_editable); ImGui::SameLine();
				ImGui::SetItemTooltip("editable");
				ImGui::Checkbox(ICON_MS_MOVE_SELECTION_RIGHT, &sprite->_moveable); ImGui::SameLine();
				ImGui::SetItemTooltip("moveable");
				ImGui::Checkbox(ICON_MS_DYNAMIC_FEED, &sprite->_trails);
				ImGui::SetItemTooltip("trails");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Colors", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				ImVec4 fg = convertColor(sprite->_foreColor);

				ImGui::ColorButton("foreColor", fg);
				ImGui::SameLine();
				ImGui::Text("#%02x%02x%02x", (int)(fg.x * 255), (int)(fg.y * 255), (int)(fg.z * 255));
				ImGui::SetItemTooltip("Foreground Color");
				ImVec4 bg = convertColor(sprite->_backColor);
				ImGui::ColorButton("backColor", bg);
				ImGui::SameLine();
				ImGui::Text("#%02x%02x%02x", (int)(bg.x * 255), (int)(bg.y * 255), (int)(bg.z * 255));
				ImGui::SameLine();
				ImGui::SetItemTooltip("Background Color");
			}

			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Coordinates", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				ImGui::Text("X:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->_startPoint.x); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Reg Point Horizontal");
				ImGui::Text("W:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->getWidth());
				ImGui::SetItemTooltip("Width");

				ImGui::Text("Y:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->_startPoint.y); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Reg Point Vertical");
				ImGui::Text("H:"); ImGui::SameLine();
				ImGui::Text("%d", sprite->getHeight()); ImGui::SameLine();
				ImGui::SetItemTooltip("Height");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
			ImGui::BeginChild("Bbox", ImVec2(150.0f, 50.0f));

			if (castMember || shape) {
				const Common::Rect &box = sprite->getBbox(true);

				ImGui::Text("l:"); ImGui::SameLine();
				ImGui::Text("%d", box.left); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Left");
				ImGui::Text("r:"); ImGui::SameLine();
				ImGui::Text("%d", box.right);
				ImGui::SetItemTooltip("Right");

				ImGui::Text("t:"); ImGui::SameLine();
				ImGui::Text("%d", box.top); ImGui::SameLine(75);
				ImGui::SetItemTooltip("Top");
				ImGui::Text("b:"); ImGui::SameLine();
				ImGui::Text("%d", box.bottom);
				ImGui::SetItemTooltip("Bottom");
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();

			ImGui::EndChild();
		}

		uint numChannels = score->_scoreCache[0]->_sprites.size();
		uint tableColumns = MAX(numFrames + 5, 25U); // Set minimal table width to 25

		{  // Render pagination
			uint frame = 1;

			ImGui::Text("   Jump to frame: ");

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

			do {
				ImGui::SameLine(0, 20);

				if (ImGui::Selectable(Common::String::format("%d", frame).c_str(), (int) frame == _state->_scoreFrameOffset, 0, ImVec2(30, 10)))
					_state->_scoreFrameOffset = frame;

				frame += 300;
			} while (frame < numFrames);

			ImGui::PopStyleVar();

			ImGui::SameLine();
			ImGui::Text(" of %d", numFrames);
		}

		if (tableColumns > kMaxColumnsInTable - 2) // Current restriction of ImGui
			tableColumns = kMaxColumnsInTable - 2;

		ImGuiTableFlags addonFlags = _state->_scoreMode == kModeExtended ? 0 : ImGuiTableFlags_RowBg;

		if (ImGui::BeginTable("Score", tableColumns + 1,
					ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
					ImGuiTableFlags_SizingStretchProp | addonFlags)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed;

			ImGui::TableSetupScrollFreeze(1, 2);

			ImGui::PushFont(_state->_tinyFont);

			ImGui::TableSetupColumn("##", flags);
			for (uint i = 0; i < tableColumns; i++) {
				Common::String label = Common::String::format("%-2d", i + _state->_scoreFrameOffset);
				label += Common::String::format("##l%d", i);

				ImGui::TableSetupColumn(label.c_str(), flags);
			}

			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			ImGui::TableNextRow(0);

			ImGui::TableSetColumnIndex(0);
			ImGui::PushID(0);

			ImGui::SetNextItemWidth(50);

			const char *selMode = modes[_state->_scoreMode];

			if (ImGui::BeginCombo("##", selMode)) {
				for (int n = 0; n < ARRAYSIZE(modes); n++) {
					const bool selected = (_state->_scoreMode == n);
					if (ImGui::Selectable(modes[n], selected))
						_state->_scoreMode = n;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

				ImGui::TableHeader("##");
			}
			ImGui::PopID();

			for (uint i = 0; i < tableColumns; i++) {
				ImGui::TableSetColumnIndex(i + 1);
				const char *column_name = ImGui::TableGetColumnName(i + 1);

				ImGui::SetNextItemWidth(20);
				ImGui::TableHeader(column_name);
			}

			ImGui::TableNextRow();

			ImGui::TableNextColumn();

			float indentSize = 10.0;
			ImGui::Indent(indentSize);
			ImGui::Text("Labels");
			ImGui::Unindent(indentSize);

			ImGui::PopFont();

			if (score->_labels && score->_labels->size()) {
				auto labels = *score->_labels;
				auto it = labels.begin();

				for (uint f = 0; f < tableColumns; f++) {
					ImGui::TableNextColumn();

					while (it != labels.end() && (*it)->number < f + _state->_scoreFrameOffset)
						it++;

					if (it == labels.end())
						continue;

					if ((*it)->number == f + _state->_scoreFrameOffset) {
						ImGui::Text(ICON_MS_BEENHERE);
						ImGui::SetItemTooltip((*it)->name.c_str());
					}
				}
			}

			{
				displayScoreChannel(0, kChTempo, 0);
				displayScoreChannel(0, kChPalette, 0);
				displayScoreChannel(0, kChTransition, 0);
				displayScoreChannel(0, kChSound1, 0);
				displayScoreChannel(0, kChSound2, 0);
				displayScoreChannel(0, kChScript, 0);
			}
			ImGui::TableNextRow();

			int mode = _state->_scoreMode;

			// Reset the color index, so that each channel gets the same color each time
			_state->_colors._contColorIndex = 0;
			for (int ch = 0; ch < (int)numChannels - 1; ch++) {
				if (mode == kModeExtended) // This will render empty row
					displayScoreChannel(ch + 1, kModeExtended, _state->_scoreMode);

				if (mode == kModeMember || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeMember, _state->_scoreMode);

				if (mode == kModeBehavior || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBehavior, _state->_scoreMode);

				if (mode == kModeInk || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeInk, _state->_scoreMode);

				if (mode == kModeBlend || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBlend, _state->_scoreMode);

				if (mode == kModeLocation || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeLocation, _state->_scoreMode);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void showChannels() {
	if (!_state->_w.channels)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Channels", &_state->_w.channels)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		Frame &frame = *score->_currentFrame;

		CastMemberID defaultPalette = g_director->getCurrentMovie()->_defaultPalette;
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
		ImGui::Text("LSCR:   actionId: %s", frame._mainChannels.actionId.asString().c_str());

		if (ImGui::BeginTable("Channels", 21, ImGuiTableFlags_Borders)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader;
			ImGui::TableSetupColumn("CH", flags);
			ImGui::TableSetupColumn("castId", flags);
			ImGui::TableSetupColumn("vis", flags);
			ImGui::TableSetupColumn("inkData", flags);
			ImGui::TableSetupColumn("ink", flags);
			ImGui::TableSetupColumn("trails", flags);
			ImGui::TableSetupColumn("stretch", flags);
			ImGui::TableSetupColumn("line", flags);
			ImGui::TableSetupColumn("dims", flags);
			ImGui::TableSetupColumn("type", flags);
			ImGui::TableSetupColumn("fg", flags);
			ImGui::TableSetupColumn("bg", flags);
			ImGui::TableSetupColumn("script", flags);
			ImGui::TableSetupColumn("colorcode", flags);
			ImGui::TableSetupColumn("blendAmount", flags);
			ImGui::TableSetupColumn("unk3", flags);
			ImGui::TableSetupColumn("constraint", flags);
			ImGui::TableSetupColumn("puppet", flags);
			ImGui::TableSetupColumn("moveable", flags);
			ImGui::TableSetupColumn("movieRate", flags);
			ImGui::TableSetupColumn("movieTime", flags);

			ImGui::TableAngledHeadersRow();
			for (int i = 0; i < frame._numChannels; i++) {
				Channel &channel = *score->_channels[i + 1];
				Sprite &sprite = *channel._sprite;

				ImGui::TableNextRow();

				ImGui::TableNextColumn();

				bool isSelected = (_state->_selectedChannel == i + 1);
				if (ImGui::Selectable(Common::String::format("%-3d", i + 1).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
					if (isSelected) {
						_state->_selectedChannel = -1;
					 } else {
						_state->_selectedChannel = i + 1;
					 }

					g_director->getCurrentWindow()->render(true);
				}

				ImGui::TableNextColumn();

				if (sprite._castId.member) {
					Common::String chNum = Common::String::format("%d", i);
					Common::String colN;

					Common::Point position = channel.getPosition();
					ImGui::Text("%s", sprite._castId.asString().c_str());
					ImGui::TableNextColumn();
					colN = "##vis" + chNum;
					ImGui::Checkbox(colN.c_str(), &channel._visible);
					ImGui::TableNextColumn();
					ImGui::Text("0x%02x", sprite._inkData);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%s)", sprite._ink, inkType2str(sprite._ink));
					ImGui::TableNextColumn();
					colN = "##trails" + chNum;
					ImGui::Checkbox(colN.c_str(), &sprite._trails);
					ImGui::TableNextColumn();
					colN = "##stretch" + chNum;
					ImGui::Checkbox(colN.c_str(), &sprite._stretch);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._thickness);
					ImGui::TableNextColumn();
					ImGui::Text("%dx%d@%d,%d", channel.getWidth(), channel.getHeight(), position.x, position.y);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%s)", sprite._spriteType, spriteType2str(sprite._spriteType));
					ImGui::TableNextColumn();
					ImGui::PushID(i + 1);
					ImGui::Text("%3d", sprite._foreColor); ImGui::SameLine();
					ImGui::ColorButton("foreColor", convertColor(sprite._foreColor));
					ImGui::PopID();
					ImGui::TableNextColumn();
					ImGui::PushID(i + 1);
					ImGui::Text("%3d", sprite._backColor); ImGui::SameLine();
					ImGui::ColorButton("backColor", convertColor(sprite._backColor));
					ImGui::PopID();
					ImGui::TableNextColumn();
					// Check early for non integer script ids
					if (sprite._scriptId.member) {
						displayScriptRef(sprite._scriptId);
					} else {
						ImGui::PushID(i + 1);
						ImGui::Selectable("  ");
						ImGui::PopID();
					}
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._colorcode);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._blendAmount);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._unk3);
					ImGui::TableNextColumn();
					ImGui::Text("%d", channel._constraint);
					ImGui::TableNextColumn();
					colN = "##puppet" + chNum;
					ImGui::Checkbox(colN.c_str(), &sprite._puppet);
					ImGui::TableNextColumn();
					colN = "##moveable" + chNum;
					ImGui::Checkbox(colN.c_str(), &sprite._moveable);
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%f", channel._movieRate);
					else
						ImGui::Text("0");
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%d (%f)", channel._movieTime, (float)(channel._movieTime/60.0f));
					else
						ImGui::Text("0");
				} else {
					ImGui::Text("000");
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
