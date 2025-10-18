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

#include "common/memstream.h"

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

#define FRAME_PAGE_SIZE 100

static void buildContinuationData(Window *window) {
	if (_state->_loadedContinuationData == window->getCurrentMovie()->getMacName()) {
		return;
	}

	_state->_scorePageSlider = 0;
	_state->_continuationData.clear();

	Score *score = window->getCurrentMovie()->getScore();
	uint numFrames = score->_scoreCache.size();

	uint numChannels = score->_scoreCache[0]->_sprites.size();
	_state->_continuationData.resize(numChannels);

	for (int ch = 0; ch < (int)numChannels; ch++) {
		_state->_continuationData[ch].resize(numFrames);

		uint currentContinuation = 1;
		for (int f = 0; f < (int)numFrames; f++) {
			const Frame &frame = *score->_scoreCache[f];
			Sprite &sprite = *frame._sprites[ch];

			const Frame *prevFrame = (f == 0) ? nullptr : score->_scoreCache[f - 1];
			Sprite *prevSprite = (prevFrame) ? prevFrame->_sprites[ch] : nullptr;

			if (prevSprite) {
				if (!(*prevSprite == sprite)) {
					currentContinuation = f;
				}
			} else {
				currentContinuation = f;
			}

			_state->_continuationData[ch][f].first = currentContinuation;

#if 0
			if (ch == 3 && prevSprite && f >= 20 && f < 49) {
				warning("%02d: st: %d cid: %d sp: %d w: %d h: %d i: %d f: %d b: %d bl: %d in: %d t: %d",
					f,
					prevSprite->_spriteType == sprite._spriteType,
					prevSprite->_castId == sprite._castId,
					prevSprite->_startPoint == sprite._startPoint,
					prevSprite->_width == sprite._width,
					prevSprite->_height == sprite._height,
					prevSprite->_ink == sprite._ink,
					prevSprite->_foreColor == sprite._foreColor,
					prevSprite->_backColor == sprite._backColor,
					prevSprite->_blendAmount == sprite._blendAmount,
					prevSprite->_ink == sprite._ink,
					prevSprite->_thickness == sprite._thickness
				);
			}
#endif
		}

		currentContinuation = 1;
		for (int f = (int)numFrames - 1; f >= 0; f--) {
			const Frame &frame = *score->_scoreCache[f];
			Sprite &sprite = *frame._sprites[ch];

			const Frame *nextFrame = (f == (int)numFrames - 1) ? nullptr : score->_scoreCache[f + 1];
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

	_state->_loadedContinuationData = window->getCurrentMovie()->getMacName();
}

static void displayScoreChannel(int ch, int mode, int modeSel, Window *window) {
	Score *score = window->getCurrentMovie()->getScore();
	uint numFrames = score->_scoreCache.size();

	const uint currentFrameNum = score->getCurrentFrameNum();
	const ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.0f, 0.65f));

	ImGui::TableNextRow();

	ImGui::PushFont(_state->_tinyFont);

	if (modeSel == kModeExtended && mode == kModeExtended)
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));

	{ // Playback toggle
		ImGui::TableNextColumn();

		ImGui::PushID(ch + 20000 - mode);
		ImDrawList *dl = ImGui::GetWindowDrawList();
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		ImGui::SetItemTooltip("Playback toggle");

		if (ImGui::IsItemClicked(0)) {
			if (mode == kModeMember) {
				score->_channels[ch]->_visible = !score->_channels[ch]->_visible;

				window->render(true);
			}
		}

		if (mode != kModeMember || score->_channels[ch]->_visible)
			dl->AddCircleFilled(mid, 4.0f, ImColor(_state->_colors._channel_toggle));
		else
			dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._channel_toggle));

		ImGui::PopID();
	}

	{	// Channel name / number
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
		int rf = f + _state->_scoreFrameOffset - 1;
		Frame &frame = *score->_scoreCache[rf];
		Sprite &sprite = *frame._sprites[ch];

		_state->_colors._contColorIndex = frame._sprites[ch]->_colorcode & 0x07;
		if (_state->_colors._contColorIndex > 5)
			_state->_colors._contColorIndex = 0;

		ImGui::TableNextColumn();

		int startCont = _state->_continuationData[ch][rf].first;
		int endCont = _state->_continuationData[ch][rf].second;

		if (!(startCont == endCont) && (sprite._castId.member || sprite.isQDShape())) {
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
				if (mode == modeSel)
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, _state->_colors._contColors[_state->_colors._contColorIndex]);
				else
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, brightenColor(_state->_colors._contColors[_state->_colors._contColorIndex], 1.5));
			}
		}

		if (rf + 1 == (int)currentFrameNum)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);

		if (f == _state->_selectedScoreCast.frame + _state->_scoreFrameOffset - 1 &&
		  ch == _state->_selectedScoreCast.channel && mode <= kModeExtended)
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(1.0f, 0.3f, 0.3f, 0.6f)));

		int mode1 = mode;

		ImGui::PushID((ch * 20 + mode) * 10000 + f);

		// If the frame is not the start, then don't render any text
		if (mode == kModeMember) {
			if (rf != startCont || !(sprite._castId.member || sprite.isQDShape())) {
				if (rf == endCont && sprite._castId.member && mode == _state->_scoreMode) {
					ImGui::PushFont(ImGui::GetIO().FontDefault);
					ImGui::TextUnformatted("\uf819");
					ImGui::PopFont();
				} else {
					if (sprite._castId.member) {
						ImGui::Selectable("─");
					} else {
						ImGui::Selectable("  ");
					}
				}
				mode1 = -1; // Skip cell data rendering
			}
		}

		switch (mode1) {
		case -1:
			break;

		case kModeMember:
			if (sprite._castId.member)
				ImGui::Selectable(Common::String::format("%d", sprite._castId.member).c_str());
			else if (sprite.isQDShape())
				ImGui::Selectable("Q");
			else
				ImGui::Selectable("  ");
			break;

		case kModeInk:
			ImGui::Selectable(Common::String::format("%s", inkType2str(sprite._ink)).c_str());
			break;

		case kModeLocation:
			ImGui::Selectable(Common::String::format("%d, %d", sprite._startPoint.x, sprite._startPoint.y).c_str());
			break;

		case kModeBlend:
			ImGui::Selectable(Common::String::format("%d", sprite._blendAmount).c_str());
			break;

		case kModeBehavior:
			displayScriptRef(sprite._scriptId);
			break;

		case kChTempo:
			if (frame._mainChannels.tempo)
				ImGui::Selectable(Common::String::format("%d", frame._mainChannels.tempo).c_str());
			break;

		case kChPalette:
			if (frame._mainChannels.palette.paletteId.member)
				ImGui::Selectable(Common::String::format("%d", frame._mainChannels.palette.paletteId.member).c_str());
			break;

		case kChTransition:
			if (frame._mainChannels.transType)
				ImGui::Selectable(Common::String::format("%d", frame._mainChannels.transType).c_str());
			break;

		case kChSound1:
			if (frame._mainChannels.sound1.member)
				ImGui::Selectable(Common::String::format("%d", frame._mainChannels.sound1.member).c_str());
			break;

		case kChSound2:
			if (frame._mainChannels.sound2.member)
				ImGui::Selectable(Common::String::format("%d", frame._mainChannels.sound2.member).c_str());
			break;

		case kChScript:
			displayScriptRef(frame._mainChannels.actionId);
			break;

		case kModeExtended: // Render empty row
		default:
			ImGui::Selectable("  ");
		}

		ImGui::PopID();

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			_state->_selectedScoreCast.frame = f + _state->_scoreFrameOffset - 1;
			_state->_selectedScoreCast.channel = ch;

			if (f + _state->_scoreFrameOffset == (int)currentFrameNum) {
				if (_state->_selectedChannel == ch)
					_state->_selectedChannel = -1;
				else
					_state->_selectedChannel = ch;

				window->render(true);
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

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Score", &_state->_w.score)) {
		Window *selectedWindow = windowListCombo(&_state->_scoreWindow);

		buildContinuationData(selectedWindow);

		Score *score = selectedWindow->getCurrentMovie()->getScore();
		uint numFrames = score->_scoreCache.size();
		Cast *cast = selectedWindow->getCurrentMovie()->getCast();

		if (!numFrames) {
			ImGui::Text("No frames");
			ImGui::End();

			return;
		}

		if (_state->_selectedScoreCast.frame >= (int)numFrames)
			_state->_selectedScoreCast.frame = 0;

		if (!numFrames || _state->_selectedScoreCast.channel >= (int)score->_scoreCache[0]->_sprites.size())
			_state->_selectedScoreCast.channel = 0;

		if (_state->_scoreFrameOffset >= (int)numFrames)
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
				ImGui::TextUnformatted("\uf816"); ImGui::SameLine();	// line_start_circle
				// the continuation data is 0-indexed but the frames are 1-indexed
				ImGui::Text("%4d", _state->_continuationData[_state->_selectedScoreCast.channel][_state->_selectedScoreCast.frame].first + 1); ImGui::SameLine(50);
				ImGui::SetItemTooltip("Start Frame");
				ImGui::TextUnformatted("\uf819"); ImGui::SameLine();	// line_end_square
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

		uint numChannels = MIN<int>(score->_scoreCache[0]->_sprites.size(), score->_maxChannelsUsed + 10);
		uint tableColumns = MAX(numFrames + 5, 25U); // Set minimal table width to 25

		if (tableColumns > kMaxColumnsInTable - 3) // Current restriction of ImGui
			tableColumns = kMaxColumnsInTable - 3;

		ImGuiTableFlags addonFlags = _state->_scoreMode == kModeExtended ? 0 : ImGuiTableFlags_RowBg;

		ImGui::BeginChild("Score table", ImVec2(0, -20));

		if (ImGui::BeginTable("Score", tableColumns + 2,
					ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
					addonFlags)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed;

			ImGui::TableSetupScrollFreeze(2, 2);

			ImGui::PushFont(_state->_tinyFont);

			ImGui::TableSetupColumn("##disable", flags); // disable button

			ImGui::TableSetupColumn("##", flags);   // Number
			for (uint i = 0; i < tableColumns; i++) {
				Common::String label = Common::String::format("%-2d", i + _state->_scoreFrameOffset);
				label += Common::String::format("##l%d", i);

				ImGui::TableSetupColumn(label.c_str(), flags);
			}

			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			ImGui::TableNextRow(0);

			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(20);

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(0);

			ImGui::SetNextItemWidth(50);

			const char *selMode = modes[_state->_scoreMode];

			if (ImGui::BeginCombo("##mode", selMode)) {
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
				ImGui::TableSetColumnIndex(i + 2);
				const char *column_name = ImGui::TableGetColumnName(i + 2);

				ImGui::SetNextItemWidth(20);
				ImGui::TableHeader(column_name);
			}

			ImGui::TableNextRow();

			ImGui::TableNextColumn(); // Enable/Disable switch

			ImGui::TableNextColumn(); // Label column

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
				displayScoreChannel(0, kChTempo, 0, selectedWindow);
				displayScoreChannel(0, kChPalette, 0, selectedWindow);
				displayScoreChannel(0, kChTransition, 0, selectedWindow);
				displayScoreChannel(0, kChSound1, 0, selectedWindow);
				displayScoreChannel(0, kChSound2, 0, selectedWindow);
				displayScoreChannel(0, kChScript, 0, selectedWindow);
			}
			ImGui::TableNextRow();

			int mode = _state->_scoreMode;

			for (int ch = 0; ch < (int)numChannels - 1; ch++) {
				if (mode == kModeExtended) // This will render empty row
					displayScoreChannel(ch + 1, kModeExtended, _state->_scoreMode, selectedWindow);

				if (mode == kModeMember || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeMember, _state->_scoreMode, selectedWindow);

				if (mode == kModeBehavior || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBehavior, _state->_scoreMode, selectedWindow);

				if (mode == kModeInk || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeInk, _state->_scoreMode, selectedWindow);

				if (mode == kModeBlend || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeBlend, _state->_scoreMode, selectedWindow);

				if (mode == kModeLocation || mode == kModeExtended)
					displayScoreChannel(ch + 1, kModeLocation, _state->_scoreMode, selectedWindow);
			}
			ImGui::EndTable();
		}

		ImGui::EndChild();

		{  // Render pagination
			ImGui::BeginDisabled(numFrames <= FRAME_PAGE_SIZE);
			ImGui::Text("   Jump to frame:", numFrames);
			ImGui::SameLine();
			ImGui::SliderInt("##scorepage", &_state->_scorePageSlider, 0, numFrames / FRAME_PAGE_SIZE, "%d00");
			_state->_scoreFrameOffset = _state->_scorePageSlider * FRAME_PAGE_SIZE + 1;
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::Button(ICON_MS_ALIGN_JUSTIFY_CENTER, ImVec2(20, 20));
			ImGui::SetItemTooltip("Center View");
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
		Window *selectedWindow = windowListCombo(&_state->_scoreWindow);

		Score *score = selectedWindow->getCurrentMovie()->getScore();
		const Frame &frame = *score->_currentFrame;

		CastMemberID defaultPalette = selectedWindow->getCurrentMovie()->_defaultPalette;
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

		int columns = 22;

		if (score->_version >= kFileVer600) {
			columns += 1; // sprite name
		}

		if (ImGui::BeginTable("Channels", columns, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader;
			ImGui::TableSetupColumn("##toggle", flags);
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
			if (score->_version >= kFileVer600) {
				ImGui::TableSetupColumn("behaviors", flags);
			} else {
				ImGui::TableSetupColumn("script", flags);
			}
			if (score->_version >= kFileVer600)
				ImGui::TableSetupColumn("name", flags);
			ImGui::TableSetupColumn("colorcode", flags);
			ImGui::TableSetupColumn("blendAmount", flags);
			ImGui::TableSetupColumn("unk3", flags);
			ImGui::TableSetupColumn("constraint", flags);
			ImGui::TableSetupColumn("puppet", flags);
			ImGui::TableSetupColumn("moveable", flags);
			ImGui::TableSetupColumn("movieRate", flags);
			ImGui::TableSetupColumn("movieTime", flags);

			ImGui::TableAngledHeadersRow();

			for (int i = 0; i < MIN<int>(frame._numChannels, score->_maxChannelsUsed + 10); i++) {
				Channel &channel = *score->_channels[i + 1];
				Sprite &sprite = *channel._sprite;

				ImGui::TableNextRow();

				{ // Playback toggle
					ImGui::TableNextColumn();

					ImGui::PushID(i + 20000);
					ImDrawList *dl = ImGui::GetWindowDrawList();
					const ImVec2 pos1 = ImGui::GetCursorScreenPos();
					const ImVec2 mid(pos1.x + 7, pos1.y + 7);

					ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
					ImGui::SetItemTooltip("Playback toggle");

					if (ImGui::IsItemClicked(0)) {
						score->_channels[i]->_visible = !score->_channels[i]->_visible;

						selectedWindow->render(true);
					}

					if (score->_channels[i]->_visible)
						dl->AddCircleFilled(mid, 4.0f, ImColor(_state->_colors._channel_toggle));
					else
						dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._channel_toggle));

					ImGui::PopID();
				}

				ImGui::TableNextColumn();

				bool isSelected = (_state->_selectedChannel == i + 1);
				if (ImGui::Selectable(Common::String::format("%-3d", i + 1).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
					if (isSelected) {
						_state->_selectedChannel = -1;
					 } else {
						_state->_selectedChannel = i + 1;
					 }

					selectedWindow->render(true);
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

					if (score->_version >= kFileVer600) {
						if (sprite._behaviors.size() > 0) {
							for (uint j = 0; j < sprite._behaviors.size(); j++) {
								displayScriptRef(sprite._behaviors[j].memberID);
								ImGui::SameLine();
								if (sprite._behaviors[j].initializerIndex) {
									ImGui::Text("(%s)", sprite._behaviors[j].initializerParams.c_str());
								} else {
									ImGui::Text("(\"\")");
								}
							}
						} else {
							ImGui::PushID(i + 1);
							ImGui::TextUnformatted("  ");
							ImGui::PopID();
						}
					} else {
						// Check early for non integer script ids
						if (sprite._scriptId.member) {
							displayScriptRef(sprite._scriptId);
						} else {
							ImGui::PushID(i + 1);
							ImGui::TextUnformatted("  ");
							ImGui::PopID();
						}
					}

					if (score->_version >= kFileVer600) {
						ImGui::TableNextColumn();
						if (sprite._spriteListIdx) {
							Common::MemoryReadStreamEndian *stream = score->getSpriteDetailsStream(sprite._spriteListIdx + 2);
							Common::String name;
							if (stream)
								name = stream->readPascalString();
							ImGui::Text("%s", name.c_str());
						} else {
							ImGui::Text(" ");
						}
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
