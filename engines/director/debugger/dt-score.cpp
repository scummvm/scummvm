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

enum { kModeMember, kModeBehavior, kModeLocation, kModeInk, kModeBlend, kModeExtended};
enum { kChTempo, kChPalette, kChTransition, kChSound1, kChSound2, kChScript };
const char *modes[] = { "Member", "Behavior", "Location", "Ink", "Blend", "Extended" };
const char *modes2[] = {
	ICON_MS_TIMER, "Tempo",					// timer
	ICON_MS_PALETTE, "Palette",				// palette
	ICON_MS_TRANSITION_FADE, "Transition",	// transition_fade
	ICON_MS_VOLUME_UP,"Sound 1",			// volume_up
	ICON_MS_VOLUME_DOWN,"Sound 2",			// volume_down
	ICON_MS_FORMS_APPS_SCRIPT, "Script",	// forms_apps_script
};

struct ScoreLayout {
	ImVec2 sidebar1Pos;
	ImVec2 mainChannelGridPos;
	ImVec2 modeSelectorPos;
	ImVec2 rulerPos;
	ImVec2 sidebar2Pos;
	ImVec2 gridPos;
	ImVec2 sliderPos;
	ImVec2 sliderYPos;
};

static ScoreLayout computeLayout(ImVec2 origin, const ImGuiState::ScoreConfig &cfg) {
	ScoreLayout l;
	l.sidebar1Pos = ImVec2(origin.x, origin.y);
	l.mainChannelGridPos = ImVec2(origin.x + cfg._sidebarWidth, origin.y);
	l.modeSelectorPos = ImVec2(origin.x, origin.y + cfg._sidebar1Height);
	l.rulerPos = ImVec2(origin.x + cfg._sidebarWidth, origin.y + cfg._sidebar1Height);
	l.sidebar2Pos = ImVec2(origin.x, origin.y + cfg._sidebar1Height + cfg._rulerHeight);
	l.gridPos = ImVec2(origin.x + cfg._sidebarWidth, origin.y + cfg._sidebar1Height + cfg._rulerHeight);
	l.sliderPos = ImVec2(origin.x + cfg._sidebarWidth, origin.y + cfg._sidebar1Height + cfg._rulerHeight + cfg._tableHeight + 8.0f);
	l.sliderYPos = ImVec2(origin.x + cfg._sidebarWidth + cfg._tableWidth + 8.0f, origin.y + cfg._sidebar1Height + cfg._rulerHeight);
	return l;
}

// helper to convert color for drawlist
static inline ImU32 U32(const ImVec4 &c) {
	return ImGui::GetColorU32(c);
}

// helper to draw thin rectangles for table grid
static void addThinRect(ImDrawList *dl, ImVec2 min, ImVec2 max, ImU32 col, float thickness = 0.1f) {
	dl->AddLine(ImVec2(min.x, min.y), ImVec2(max.x, min.y), col, thickness); // top
	dl->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, max.y), col, thickness); // right
	dl->AddLine(ImVec2(max.x, max.y), ImVec2(min.x, max.y), col, thickness); // bottom
	dl->AddLine(ImVec2(min.x, max.y), ImVec2(min.x, min.y), col, thickness); // left
}

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
					prevSprite->Width == sprite.Width,
					prevSprite->Height == sprite.Height,
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


static void drawSliderY(ImVec2 pos, int numChannels) {
	auto &cfg = _state->_scoreCfg;
	ImGui::SetCursorScreenPos(pos);
	ImGui::SetNextItemWidth(16.0f);
	int visibleChannels = (_state->_scoreMode == kModeExtended) ? (int)(cfg._tableHeight / cfg._cellHeightExtended) : cfg._visibleChannels;
	int maxScroll = MAX(1, numChannels - visibleChannels);
	ImGui::VSliderInt("##channelSlider", ImVec2(16.0f, cfg._tableHeight), &_state->_scoreState.channelScrollOffset, maxScroll, 1);
}

static void drawSidebar1(ImDrawList *dl, ImVec2 startPos, Score *score) {
	float toggleColWidth = 20.0f;
	float labelColWidth = 40.0f;
	float totalWidth = toggleColWidth + labelColWidth;
	auto &cfg = _state->_scoreCfg;

	for (int ch = 1; ch <= 6; ch++) {
		float y = startPos.y + (ch - 1) * cfg._cellHeight;
		ImVec2 rowMin = ImVec2(startPos.x, y);
		ImVec2 rowMax = ImVec2(startPos.x + totalWidth, y + cfg._cellHeight);

		dl->AddRectFilled(rowMin, rowMax, cfg._tableDarkColor);
		addThinRect(dl, rowMin, rowMax, cfg._borderColor);

		float radius = 5.0f;
		float pad  = cfg._cellHeight * 0.12f; // inner padding

		ImVec2 center(rowMin.x + pad + radius, rowMax.y - pad - radius);

		dl->AddCircleFilled(center, radius, U32(_state->_colors._channel_toggle));

		// channel number centered in the right column
		ImFont *iconFont = ImGui::GetIO().FontDefault;
		const char *icon = modes2[(ch - 1) * 2];
		float textlen = ImGui::CalcTextSize(icon).x;
		float textX = startPos.x + toggleColWidth + (labelColWidth - textlen) / 2.0f;
		float textY = y + (cfg._cellHeight - ImGui::GetTextLineHeight()) / 2.0f;
		dl->AddText(iconFont, 0.0f, ImVec2(textX, textY), U32(_state->_colors._type_color), icon);

		// invisible button covering the row for interaction
		ImGui::SetCursorScreenPos(rowMin);
		ImGui::InvisibleButton(Common::String::format("##s1row%d", ch).c_str(), ImVec2(totalWidth, cfg._cellHeight));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", modes2[(ch - 1) * 2 + 1]);

	}
}

static void drawSidebar2(ImDrawList *dl, ImVec2 startPos, Score *score) {
	float toggleColWidth = 20.0f;
	float labelColWidth  = 40.0f;
	float totalWidth = toggleColWidth + labelColWidth;
	auto &cfg = _state->_scoreCfg;
	float cellH = (_state->_scoreMode == kModeExtended) ? cfg._cellHeightExtended : cfg._cellHeight; // using this only for cell height, as only cell height changes in extended mode
	int visibleChannels = (_state->_scoreMode == kModeExtended) ? (int)(cfg._tableHeight / cfg._cellHeightExtended) : cfg._visibleChannels;


	for (int i = 0; i < visibleChannels; i++) {
		int ch = i + _state->_scoreState.channelScrollOffset;
		float y = startPos.y + i * cellH;
		ImVec2 rowMin = ImVec2(startPos.x, y);
		ImVec2 rowMax = ImVec2(startPos.x + totalWidth, y + cellH);

		if (ch >= (int)score->_channels.size()) break;


		dl->AddRectFilled(rowMin, rowMax, cfg._tableDarkColor);
		addThinRect(dl, rowMin, rowMax, cfg._borderColor);

		// toggle circle
		// small square at bottom-left of the cell (size relative to cell)
		float radius = 5.0f;   // square size
		float pad  = cfg._cellHeight * 0.12f;   // inner padding

		ImVec2 center(rowMin.x + pad + radius, rowMax.y - pad - radius);

		if (score->_channels[ch]->_visible)
			dl->AddCircleFilled(center, radius, U32(_state->_colors._channel_toggle));
		else
			dl->AddCircle(center, radius, U32(_state->_colors._channel_toggle));

		ImGui::SetCursorScreenPos(rowMin);
		ImGui::InvisibleButton(Common::String::format("##s2toggle%d", ch).c_str(), ImVec2(toggleColWidth, cellH));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Playback toggle");
		if (ImGui::IsItemClicked()) { // determines what happens on toggle of the button
			score->_channels[ch]->_visible = !score->_channels[ch]->_visible;
		}

		// channel num and extra stuff if extended mode
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", ch);
		float textX  = startPos.x + toggleColWidth;

		if (_state->_scoreMode != kModeExtended) {
			float textY = y + (cellH - ImGui::GetTextLineHeight()) / 2.0f;
			dl->AddText(ImVec2(textX, textY), U32(_state->_colors._keyword_color), buf);
		} else { // draw channel number and labels
			dl->AddText(ImVec2(textX, y + 2.0f), U32(_state->_colors._keyword_color), buf);
			const char *subLabels[] = { "Member", "Behavior", "Ink", "Blend", "Location" };
			float lineH = ImGui::GetTextLineHeight();
			for (int s = 0; s < 5; s++) {
				float subX = textX - 17.0f;
				float subY = y + lineH + 2.0f + s * lineH; // offset below channel number
				dl->AddText(ImVec2(subX, subY), U32(_state->_colors._type_color), subLabels[s]);
			}
		}
	}
}

static void drawRuler(ImDrawList *dl, ImVec2 startPos) {
	auto &cfg = _state->_scoreCfg;
	int start = _state->_scoreState.xSliderValue;
	ImVec2 p1 = startPos;
	ImVec2 p2 = {p1.x + cfg._rulerWidth, p1.y + cfg._rulerHeight};

	dl->AddRectFilled(p1, p2, cfg._tableDarkColor);
	addThinRect(dl, p1, p2, cfg._borderColor);

	float bigTickLen = cfg._rulerHeight * 0.4f;
	float smallTickLen = cfg._rulerHeight * 0.3f;

	// i is the real frame number (1-indexed), rulerX is its pixel position
	for (int i = start; i < start + cfg._visibleFrames; i++) {
		float rulerX = p1.x + (i - start) * cfg._cellWidth + cfg._cellWidth / 2.0f;
		float len = smallTickLen;
		float thickness = 1.0f;

		if (i % 5 == 0) {
			len = bigTickLen;
			thickness = 1.5f;
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", i);
			float textlen = ImGui::CalcTextSize(buf).x;
			dl->AddText(ImVec2(rulerX - textlen / 2, p1.y + 4.0), U32(_state->_colors._type_color), buf);
		}

		dl->AddLine(ImVec2(rulerX, p2.y), ImVec2(rulerX, p2.y - len), U32(_state->_colors._line_color), thickness);
	}
}

static void drawModeSelector(ImVec2 startPos) {
	auto &cfg = _state->_scoreCfg;
	ImGui::SetNextItemWidth(cfg._sidebarWidth);
	ImGui::SetCursorScreenPos(startPos);
	if (ImGui::BeginCombo("##mode", modes[_state->_scoreMode])) {
		for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
			bool isSelected = (_state->_scoreMode == i);
			if (ImGui::Selectable(modes[i], isSelected))
				_state->_scoreMode = i;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

static void drawSpriteInspector(Score *score, Cast *cast, uint numFrames) {

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

}

static void drawSpriteGrid(ImDrawList *dl, ImVec2 startPos, Score *score, Cast *cast, Window *window) {
	int total_frames = (int)score->_scoreCache.size();

	auto &cfg = _state->_scoreCfg;
	float cellH = (_state->_scoreMode == kModeExtended) ? cfg._cellHeightExtended : cfg._cellHeight;
	int visibleChannels = (_state->_scoreMode == kModeExtended) ? (int)(cfg._tableHeight / cfg._cellHeightExtended) : cfg._visibleChannels;
	int startFrame = MAX(0, _state->_scoreState.xSliderValue - 1);
	int numChannels = MIN<int>(score->_scoreCache[0]->_sprites.size(), score->_maxChannelsUsed + 10);

	ImVec2 gridMin = startPos;
	ImVec2 gridMax = ImVec2(startPos.x + cfg._tableWidth, startPos.y + cfg._tableHeight);
	dl->PushClipRect(gridMin, gridMax, false);

	for (int i = 0; i < visibleChannels; i++) {
		int ch = i + _state->_scoreState.channelScrollOffset;
		if (ch >= numChannels) break;
		float y = startPos.y + i * cellH;

		// pass 1: draw cell backgrounds
		for (int f = 0; f < cfg._visibleFrames; f++) {
			int rf = startFrame + f;
			float x = startPos.x + f * cfg._cellWidth;
			ImVec2 cellMin = ImVec2(x, y);
			ImVec2 cellMax = ImVec2(x + cfg._cellWidth, y + cellH);
			ImU32 col = ((rf + 1) % 5 == 0) ? cfg._tableDarkColor : cfg._tableLightColor;
			dl->AddRectFilled(cellMin, cellMax, col);
			addThinRect(dl, cellMin, cellMax, cfg._borderColor);
		}

		// pass 2: draw sprite span bars on top of cells
		for (int f = 0; f < cfg._visibleFrames; f++) {
			int rf = startFrame + f;
			if (rf >= total_frames) break;
			Frame &frame = *score->_scoreCache[rf];
			Sprite &sprite = *frame._sprites[ch];

			if (!sprite._castId.member && !sprite.isQDShape()) continue; // empty cell
			if (ch >= (int)_state->_continuationData.size()) continue;
			if (rf >= (int)_state->_continuationData[ch].size()) break;

			int spanStart = _state->_continuationData[ch][rf].first;
			int spanEnd = _state->_continuationData[ch][rf].second;

			if (rf != spanStart && f != 0) continue;
			float x1 = startPos.x + MAX<float>(spanStart - startFrame, 0) * cfg._cellWidth;
			float x2 = MIN<float>(startPos.x + (spanEnd - startFrame + 1) * cfg._cellWidth, startPos.x + cfg._tableWidth);

			if (ch >= (int)_state->_continuationData.size()) continue;
			if (rf >= (int)_state->_continuationData[ch].size()) break;

			bool startVisible = spanStart >= startFrame;
			bool endVisible = spanEnd < startFrame + cfg._visibleFrames;

			// clamp x1 to grid left edge, x2 to right
			float cy  = y + cellH * ( (_state->_scoreMode == kModeExtended) ? 0.1 : 0.2);
			float pad = 0.0f; // vertical padding so bar doesnt touch cell borders

			// color selection based on if the sprite is selected or not
			ImU32 color;
			bool isSelected = (_state->_selectedScoreCast.channel == ch &&
							   _state->_selectedScoreCast.frame >= spanStart &&
							   _state->_selectedScoreCast.frame <= spanEnd);
			bool isHovered  = (_state->_hoveredScoreCast.channel == ch &&
							   _state->_hoveredScoreCast.frame >= spanStart &&
							   _state->_hoveredScoreCast.frame <= spanEnd);

			if (isSelected)
				color = U32(_state->_colors._channelSelectedCol);
			else if (isHovered)
				color = U32(_state->_colors._channelHoveredCol);
			else {
				int colorIdx = sprite._colorcode & 0x07;
				if (colorIdx > 5) colorIdx = 0;
				color = U32(_state->_colors._contColors[colorIdx]);
			}

			float rounding = 0.0f;
			dl->AddRectFilled(ImVec2(x1, y + pad), ImVec2(x2 - 1.0f, y + cellH - pad), color, rounding);
			// horizontal line through the middle, offset from x1 if circle is present
			dl->AddLine(ImVec2(x1 + (startVisible ? 6.0f : 0.0f), cy), ImVec2(x2 - 6.0f, cy), U32(_state->_colors._line_color), 1.0f);

			if (startVisible)
				dl->AddCircle(ImVec2(x1 + 4.0f, cy), 3.0f, U32(_state->_colors._line_color), 0, 1.5f);

			if (endVisible)
				dl->AddRect(ImVec2(x2 - 7.0f, cy - 3.0f), ImVec2(x2 - 1.0f, cy + 3.0f), U32(_state->_colors._line_color), 0.0f, 0, 1.5f);


			if (_state->_scoreMode == kModeExtended && startVisible && (sprite._castId.member || sprite.isQDShape())) {
				float lineH = ImGui::GetTextLineHeight();
				float textX = x1 + 4.0f;
				float baseY = y + 15.0f;
				CastMember *cm = cast->getCastMember(sprite._castId.member, true);
				// Member name
				char buf[64] = "";
				if (cm)
					snprintf(buf, sizeof(buf), "%s", getDisplayName(cm).c_str());
				else if (sprite.isQDShape())
					snprintf(buf, sizeof(buf), "Q");
				dl->AddText(ImVec2(textX, baseY), U32(_state->_colors._line_color), buf);

				// Behavior
				buf[0] = '\0';
				if (sprite._scriptId.member) {
					CastMember *sc = cast->getCastMember(sprite._scriptId.member, true);
					if (sc) snprintf(buf, sizeof(buf), "%s", getDisplayName(sc).c_str());
				}
				dl->AddText(ImVec2(textX, baseY + lineH), U32(_state->_colors._line_color), buf);

				// Ink
				dl->AddText(ImVec2(textX, baseY + lineH * 2), U32(_state->_colors._line_color), inkType2str(sprite._ink));

				// Blend
				snprintf(buf, sizeof(buf), "%d", sprite._blendAmount);
				dl->AddText(ImVec2(textX, baseY + lineH * 3), U32(_state->_colors._line_color), buf);

				// Location
				snprintf(buf, sizeof(buf), "%d,%d", sprite._startPoint.x, sprite._startPoint.y);
				dl->AddText(ImVec2(textX, baseY + lineH *4), U32(_state->_colors._line_color), buf);
			}

			if (startVisible && _state->_scoreMode != kModeExtended) {
				char label[64] = "";
				CastMember *cm = cast->getCastMember(sprite._castId.member, true);
				switch (_state->_scoreMode) {
				case kModeMember:
					if (cm)
						snprintf(label, sizeof(label), "%s", getDisplayName(cm).c_str());
					else if (sprite.isQDShape())
						snprintf(label, sizeof(label), "Q");
					break;
				case kModeBehavior:
					if (sprite._scriptId.member) {
						CastMember *script = cast->getCastMember(sprite._scriptId.member, true);
						if (script)
							snprintf(label, sizeof(label), "%s", getDisplayName(script).c_str());
					}
					break;
				case kModeInk:
					snprintf(label, sizeof(label), "%s", inkType2str(sprite._ink));
					break;
				case kModeLocation:
					snprintf(label, sizeof(label), "%d,%d", sprite._startPoint.x, sprite._startPoint.y);
					break;
				case kModeBlend:
					snprintf(label, sizeof(label), "%d", sprite._blendAmount);
					break;
				default:
					break;
				}
				if (label[0])
					dl->AddText(ImVec2(x1 + 4.0f, y + (cellH - ImGui::GetTextLineHeight()) / 2.0f), U32(_state->_colors._line_color), label);

			}

		}

		// pass 3, for clickable rects, add invisible buttosn
		for (int f = 0; f < cfg._visibleFrames; f++) {
			int rf = startFrame + f;
			if (rf >= total_frames) break;
			float x = startPos.x + f * cfg._cellWidth;
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			ImGui::InvisibleButton(Common::String::format("##cell_%d_%d", ch, f).c_str(), ImVec2(cfg._cellWidth, cellH));
			if (ImGui::IsItemClicked()) {
				_state->_selectedScoreCast.frame = rf;
				_state->_selectedScoreCast.channel = ch;
				_state->_selectedScoreCast.isMainChannel = false;
			}

			if (ImGui::IsItemHovered()) {
				_state->_hoveredScoreCast.frame = rf;
				_state->_hoveredScoreCast.channel = ch;
			}
		}
	}
	dl->PopClipRect();
}

static void drawSliderX(ImVec2 pos, Score *score) {
	auto &cfg = _state->_scoreCfg;
	ImGui::SetCursorScreenPos(pos);
	ImGui::SetNextItemWidth(cfg._rulerWidth);
	int totalFrames = (int)score->_scoreCache.size();
	int sliderMin = 1;
	int sliderMax = MAX(totalFrames - cfg._visibleFrames + 1, 1);
	ImGui::SliderInt("##frameSlider", &_state->_scoreState.xSliderValue, sliderMin, sliderMax);
}

static void drawMainChannelGrid(ImDrawList *dl, ImVec2 startPos, Score *score) {
	auto &cfg = _state->_scoreCfg;
	int startFrame = MAX(0, _state->_scoreState.xSliderValue - 1);
	int total_frames = (int)score->_scoreCache.size();

	for (int ch = kChTempo; ch <= kChScript; ch++) {
		float y = startPos.y + ch * cfg._cellHeight;

		// pass 1, backgrounds
		for (int f = 0; f <= cfg._visibleFrames; f++) {
			int rf = startFrame + f;
			float x = startPos.x + f * cfg._cellWidth;
			ImVec2 cellMin = ImVec2(x, y);
			ImVec2 cellMax = ImVec2(x + cfg._cellWidth, y + cfg._cellHeight);
			ImU32 col = ((rf + 1) % 5 == 0) ? cfg._tableDarkColor : cfg._tableLightColor;
			dl->AddRectFilled(cellMin, cellMax, col);
			addThinRect(dl, cellMin, cellMax, cfg._borderColor);
		}

		// pass 2, span bars
		int f = 0;
		while (f < cfg._visibleFrames) {
			int rf = startFrame + f;
			if (rf >= total_frames) break;

			// get label for this frame
			Frame &frame = *score->_scoreCache[rf];
			auto &mc = frame._mainChannels;
			Common::String label;
			switch (ch) {
			case kChTempo: if (mc.tempo) label = Common::String::format("%d", mc.tempo); break;
			case kChPalette: if (mc.palette.paletteId.member) label = Common::String::format("%d", mc.palette.paletteId.member); break;
			case kChTransition: if (mc.transType) label = Common::String::format("%d", mc.transType); break;
			case kChSound1: if (mc.sound1.member) label = Common::String::format("%d", mc.sound1.member); break;
			case kChSound2: if (mc.sound2.member) label = Common::String::format("%d", mc.sound2.member); break;
			case kChScript: if (mc.actionId.member) label = Common::String::format("%d", mc.actionId.member); break;
			}

			if (label.empty()) { f++; continue; }

			// find run end (same value)
			int runStart = rf, runEnd = rf;
			for (int nf = rf + 1; nf < total_frames && (nf - startFrame) < cfg._visibleFrames; nf++) {
				Common::String nextLabel;
				Frame &nframe = *score->_scoreCache[nf];
				auto &nmc = nframe._mainChannels;
				switch (ch) {
				case kChTempo: if (nmc.tempo) nextLabel = Common::String::format("%d", nmc.tempo); break;
				case kChPalette: if (nmc.palette.paletteId.member) nextLabel = Common::String::format("%d", nmc.palette.paletteId.member); break;
				case kChTransition: if (nmc.transType) nextLabel = Common::String::format("%d", nmc.transType); break;
				case kChSound1: if (nmc.sound1.member) nextLabel = Common::String::format("%d", nmc.sound1.member); break;
				case kChSound2: if (nmc.sound2.member) nextLabel = Common::String::format("%d", nmc.sound2.member); break;
				case kChScript: if (nmc.actionId.member) nextLabel = Common::String::format("%d", nmc.actionId.member); break;
				}
				if (label == nextLabel) runEnd = nf;
				else break;
			}

			bool startVisible = (runStart >= startFrame);
			bool endVisible = (runEnd < startFrame + cfg._visibleFrames);
			float x1 = startPos.x + MAX<float>(runStart - startFrame, 0) * cfg._cellWidth;
			float x2 = MIN<float>(startPos.x + (runEnd - startFrame + 1) * cfg._cellWidth, startPos.x + cfg._tableWidth);
			float cy = y + cfg._cellHeight * 0.2;
			float pad = 0.0f;

			dl->AddRectFilled(ImVec2(x1, y + pad), ImVec2(x2 - 1.0f, y + cfg._cellHeight - pad), U32(_state->_colors._contColors[ch % 6]), 0.0f);
			dl->AddLine(ImVec2(x1 + (startVisible ? 6.0f : 0.0f), cy), ImVec2(x2 - 6.0f, cy), U32(_state->_colors._line_color), 1.0f);
			if (startVisible)
				dl->AddCircle(ImVec2(x1 + 4.0f, cy), 3.0f,  U32(_state->_colors._line_color), 0, 1.5f);
			if (endVisible)
				dl->AddRect(ImVec2(x2 - 7.0f, cy - 3.0f), ImVec2(x2 - 1.0f, cy + 3.0f), U32(_state->_colors._line_color), 0.0f, 0, 1.5f);
			f = (runEnd - startFrame) + 1; // skip to after the run
		}

		// pass 3: invisible buttons for interaction
		for (f = 0; f < cfg._visibleFrames; f++) {
			int rf = startFrame + f;
			if (rf >= total_frames) break;
			float x = startPos.x + f * cfg._cellWidth;
			ImGui::SetCursorScreenPos(ImVec2(x, y));
			ImGui::InvisibleButton(
				Common::String::format("##maincell_%d_%d", ch, f).c_str(),
				ImVec2(cfg._cellWidth, cfg._cellHeight)
			);

			if (ImGui::IsItemClicked()) {
				_state->_selectedScoreCast.frame = rf;
				_state->_selectedScoreCast.channel = ch;
				_state->_selectedScoreCast.isMainChannel = true;
				Frame &clickedFrame = *score->_scoreCache[rf];
				auto &mc = clickedFrame._mainChannels;
				switch (ch) {
				case kChTempo:
					break;
				case kChPalette: // open cast window focused on palette member
					if (mc.palette.paletteId.member) {
						_state->_w.cast = true;
						// select it in the cast so showCast() highlights it
						for (auto &scriptCast : _state->_scriptCasts) {
							if (scriptCast == mc.palette.paletteId) {
								break;
							}
						}
					}
					break;
				case kChTransition:
					break;
				case kChSound1: // open cast window
					if (mc.sound1.member)
						_state->_w.cast = true;
					break;
				case kChSound2:
					if (mc.sound2.member)
						_state->_w.cast = true;
					break;
				case kChScript: // open script in script editor
					if (mc.actionId.member) {
						ImGuiScript script = toImGuiScript(kScoreScript, mc.actionId, "");
						setScriptToDisplay(script);
						_state->_w.funcList = true;
					}
					break;
				}
			}

			if (ImGui::IsItemHovered()) {
				Common::String label;
				Frame &frame = *score->_scoreCache[rf];
				auto &mc = frame._mainChannels;
				switch (ch) {
				case kChTempo: if (mc.tempo) label = Common::String::format("%d", mc.tempo); break;
				case kChPalette: if (mc.palette.paletteId.member) label = Common::String::format("%d", mc.palette.paletteId.member); break;
				case kChTransition: if (mc.transType) label = Common::String::format("%d", mc.transType); break;
				case kChSound1: if (mc.sound1.member) label = Common::String::format("%d", mc.sound1.member); break;
				case kChSound2: if (mc.sound2.member) label = Common::String::format("%d", mc.sound2.member); break;
				case kChScript: if (mc.actionId.member) label = Common::String::format("%d", mc.actionId.member); break;
				}
				if (!label.empty())
					ImGui::SetTooltip("%s: %s", modes2[(ch) * 2 + 1], label.c_str());
			}
		}
	}
}

static void drawPlayhead(ImDrawList *dl, ImVec2 rulerPos, ImVec2 mainChannelGridPos, ImVec2 gridPos, Score *score) {
	auto &cfg = _state->_scoreCfg;
	int start = _state->_scoreState.xSliderValue;
	uint currentFrameNum = score->getCurrentFrameNum();

	if ((int)currentFrameNum < start || (int)currentFrameNum >= start + cfg._visibleFrames)
		return;

	float px = rulerPos.x + (currentFrameNum - start) * cfg._cellWidth;
	float top = mainChannelGridPos.y; // top of main channel grid
	float bottom = gridPos.y + cfg._tableHeight; // bottom of sprite grid
	ImU32 RED = IM_COL32(200, 50, 0, 255);

	dl->AddLine(ImVec2(px, top), ImVec2(px, bottom), RED, 2.0f);

	// triangle marker in the ruler
	dl->AddTriangleFilled(
		ImVec2(px - 5.0f, rulerPos.y),
		ImVec2(px + 5.0f, rulerPos.y),
		ImVec2(px, rulerPos.y + 8.0f),
		RED
	);
}

void showScore() {
	if (!_state->_w.score)
		return;

	ImVec2 pos(20, 20);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size * 1.5f;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Score", &_state->_w.score, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
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

		drawSpriteInspector(score, cast, numFrames);
		ImDrawList *dl	= ImGui::GetWindowDrawList();
		ImVec2	 origin = ImGui::GetCursorScreenPos();
		ScoreLayout layout = computeLayout(origin, _state->_scoreCfg);
		int numChannels = MIN<int>(score->_scoreCache[0]->_sprites.size(), score->_maxChannelsUsed + 10);

		drawSidebar1(dl, layout.sidebar1Pos, score);
		drawMainChannelGrid(dl, layout.mainChannelGridPos, score);
		drawModeSelector(layout.modeSelectorPos);
		drawRuler(dl, layout.rulerPos);
		drawSidebar2(dl, layout.sidebar2Pos, score);
		drawSpriteGrid(dl, layout.gridPos, score, cast, selectedWindow);
		drawPlayhead(dl, layout.rulerPos, layout.mainChannelGridPos, layout.gridPos, score);
		drawSliderX(layout.sliderPos, score);
		drawSliderY(layout.sliderYPos, numChannels);

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
			ImGui::Text("PAL:	paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d, currentId: %s, defaultId: %s",
				frame._mainChannels.palette.paletteId.asString().c_str(), frame._mainChannels.palette.firstColor, frame._mainChannels.palette.lastColor, frame._mainChannels.palette.flags,
				frame._mainChannels.palette.cycleCount, frame._mainChannels.palette.speed, frame._mainChannels.palette.frameCount,
				frame._mainChannels.palette.fade, frame._mainChannels.palette.delay, frame._mainChannels.palette.style, g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
		} else {
			ImGui::Text("PAL:	paletteId: 000, currentId: %s, defaultId: %s\n", g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
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
						dl->AddCircleFilled(mid, 4.0f, U32(_state->_colors._channel_toggle));
					else
						dl->AddCircle(mid, 4.0f, U32(_state->_colors._channel_toggle));

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
