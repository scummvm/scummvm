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
#include "director/castmember/text.h"
#include "director/castmember/richtext.h"
#include "director/castmember/shape.h"
#include "director/castmember/bitmap.h"
#include "director/sprite.h"
#include "director/castmember/filmloop.h"
#include "director/frame.h"
#include "director/score.h"

#include "director/types.h"

namespace Director {
namespace DT {

template<typename... Args>
void showProperty(const Common::String &title,
				  const ImVec4 *color,
				  const Common::String &fmt,
				  Args... args) {
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", title.c_str());

	ImGui::TableSetColumnIndex(1);

	if (color)
		ImGui::TextColored(*color, fmt.c_str(), args...);
	else
		ImGui::Text(fmt.c_str(), args...);
}

template<typename... Args>
void showProperty(const Common::String &title,
				  const Common::String &fmt,
				  Args... args) {
	showProperty(title, nullptr, fmt, args...);
}

void showPropertyBool(const Common::String &title, bool value) {
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", title.c_str());

	ImGui::TableSetColumnIndex(1);

	ImGui::Checkbox(("##" + title).c_str(), &value);
	ImGui::SameLine();
	ImGui::Text("%s", value ? "true" : "false");
}

void drawBitmapCMprops(BitmapCastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Bitmap")) {
		CastMemberInfo *info = member->getInfo();

		if (ImGui::CollapsingHeader("Media Properties")) {
			if (ImGui::BeginTable("##BitmapMediaProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("paletteRef", (member->_clut.castLib > 0 ? "Custom Palette" : paletteType2str((PaletteType)member->_clut.member).c_str()));

				showPropertyBool("centerRegPoint", (member->_flags1 & BitmapCastMember::kFlagCenterRegPoint) || (member->_flags1 & BitmapCastMember::kFlagCenterRegPointD4));

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Playback Properties")) {
			if (ImGui::BeginTable("##BitmapPlaybackProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				if (info == nullptr) {
					showProperty("autohilite", "No Info");
				} else {
					showPropertyBool("autoHilite", info->autoHilite);
				}

				showPropertyBool("dither", (member->_flags1 & BitmapCastMember::kFlagDither));

				showProperty("trimWhiteSpace", "TODO");

				showProperty("useAlpha", "TODO");

				showProperty("alphaThreshold", "%d", member->_alphaThreshold);

				showProperty("depth", "%d", member->_bitsPerPixel);

				showProperty("imageCompression", "TODO");

				if (info == nullptr) {
					showProperty("imageQuality", "No Info");
				} else {
					showProperty("imageQuality", "%d", info->imageQuality);
				}

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Media Access")) {
			if (ImGui::BeginTable("##BitmapMediaAccess", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("image", "%s", member->_picture == nullptr ? "empty" : "...");

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Outdated Properties")) {
			if (ImGui::BeginTable("##BitmapOutdatedProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("palette", "%d", member->_clut.member);

				PictureReference *pictureRef = member->getPicture();
				showProperty("picture", "%s", pictureRef == nullptr ? "empty" : "...");

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

void drawTextCMprops(TextCastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Text")) {

		if (ImGui::CollapsingHeader("Text Properties")) {
			if (ImGui::BeginTable("##TextProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				Common::String text = member->getText().encode();
				showProperty("text", "%s", text.c_str());

				showProperty("alignment", "%s", textAlignType2str(member->_textAlign).c_str());

				showProperty("font", "%s", member->getTextFont().c_str());

				showProperty("fontSize", "%d", member->getTextSize());

				showProperty("textStyle", "%s", member->getTextStyle().c_str());

				showProperty("textHeight", "%d", member->getTextHeight());

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Layout Properties")) {
			if (ImGui::BeginTable("##TextLayoutProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("border", "%d", member->_borderSize);

				showProperty("boxDropShadow", "%d", member->_boxShadow);

				showProperty("boxType", "%s", textType2str(member->_textType).c_str());

				showPropertyBool("editable", member->_editable);

				showProperty("lineCount", "%d", member->getLineCount());

				showProperty("scrollTop", "%d", member->_scroll);

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

void drawRichTextCMprops(RichTextCastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Rich Text")) {

		if (ImGui::CollapsingHeader("Rich Text Properties")) {
			if (ImGui::BeginTable("##RichTextProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				// Not using showProperty() here because we want to truncate
				// add a tooltip for long text values
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("text");
				ImGui::TableSetColumnIndex(1);
				Common::String text = member->getText();
				const int maxDisplayLength = 50;
				if (text.size() > maxDisplayLength) {
					Common::String truncated = text.substr(0, maxDisplayLength) + "...";
					ImGui::Text("%s", truncated.c_str());
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", text.c_str());
					}
				} else {
					ImGui::Text("%s", text.c_str());
				}

				// Not using showProperty() here because we want to display the color swatch and RGB values
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("foreColor");
				ImGui::TableSetColumnIndex(1);
				uint8 r, g, b, a;
				member->_pf32.colorToARGB(member->_foreColor, a, r, g, b);
				ImVec4 foreColorVec = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
				ImGui::ColorButton("##foreColorSwatch", foreColorVec, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker, ImVec2(40, 20));
				ImGui::SameLine();
				ImGui::Text("RGB(%d, %d, %d)", r, g, b);

				// Not using showProperty() here because we want to display the color swatch and RGB values
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("bgColor");
				ImGui::TableSetColumnIndex(1);
				member->_pf32.colorToARGB(member->_bgColor, a, r, g, b);
				ImVec4 bgColorVec = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
				ImGui::ColorButton("##bgColorSwatch", bgColorVec, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker, ImVec2(40, 20));
				ImGui::SameLine();
				ImGui::Text("RGB(%d, %d, %d)", r, g, b);

				showPropertyBool("antialiasFlag", (bool)member->_antialiasFlag);

				showProperty("antialiasFontSize", "%u", member->_antialiasFontSize);

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Layout Properties")) {
			if (ImGui::BeginTable("##RichTextLayoutProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("scrollPos", "%u", member->_scrollPos);

				showProperty("displayHeight", "%u", member->_displayHeight);

				// Not using showProperty() here because we want to show the binary representation of the cropFlags value
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("cropFlags");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%02X (", member->_cropFlags);
				ImGui::SameLine();
				for (int i = 7; i >= 0; i--) {
					bool bitSet = (member->_cropFlags & (1 << i)) != 0;
					if (bitSet) {
						ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "1");
					} else {
						ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "0");
					}
					if (i > 0) {
						ImGui::SameLine();
					}
				}
				ImGui::SameLine();
				ImGui::Text(")");

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Media Access")) {
			if (ImGui::BeginTable("##RichTextMediaAccess", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("picture", "%s", member->_picture == nullptr ? "empty" : "...");

				showProperty("pictureWithBg", "%s", member->_pictureWithBg == nullptr ? "empty" : "...");

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

void drawShapeCMprops(ShapeCastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Shape")) {

		if (ImGui::CollapsingHeader("Shape Properties")) {
			if (ImGui::BeginTable("##ShapeProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("shapeType", "%s", shapeType2str(member->_shapeType).c_str());

				showProperty("foreColor", "%d", member->getForeColor());

				showProperty("backColor", "%d", member->getBackColor());

				showProperty("pattern", "%d", member->_pattern);

				showPropertyBool("filled", (bool)member->_fillType);

				showProperty("lineSize", "%d", member->_lineThickness);

				showProperty("lineDirection", "%d", member->_lineDirection);

				showProperty("ink", "%d", member->_ink);

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

void drawBaseCMprops(CastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Member")) {
		Cast *cast = member->getCast();
		const CastMemberInfo *info = member->getInfo();

		if (ImGui::CollapsingHeader("Common Member Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("##CastCommonProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				if (info) {
					showProperty("name", "%s", info->name.c_str());
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("name", &grayColor, "No Info");
				}

				showProperty("number", "%d", member->getID());

				showProperty("castLibNum", "%d", cast->_castLibID);

				if (info && !info->fileName.empty()) {
					showProperty("fileName", "%s", info->fileName.c_str());
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("fileName", &grayColor, "...");
				}

				showProperty("type", "#%s", castType2str(member->_type));

				// Not using showProperty() here because we want to
				// show a tooltip with the full script text when hovering over the truncated version
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("scriptText");
				ImGui::TableSetColumnIndex(1);
				if (info && !info->script.empty()) {
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "...");
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", info->script.c_str());
					}
				} else {
					ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "...");
				}

				if (info && info->creationTime != 0) {
					showProperty("creationDate", "%u", info->creationTime);
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("creationDate", &grayColor, "...");
				}

				if (info && info->modifiedTime != 0) {
					showProperty("modifiedDate", "%u", info->modifiedTime);
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("modifiedDate", &grayColor, "...");
				}

				if (info && !info->modifiedBy.empty()) {
					showProperty("modifiedBy", "%s", info->modifiedBy.c_str());
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("modifiedBy", &grayColor, "...");
				}

				if (info && !info->comments.empty()) {
					showProperty("comments", "%s", info->comments.c_str());
				} else {
					ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					showProperty("comments", &grayColor, "...");
				}

				showProperty("purgePriority", "%d", member->_purgePriority);

				showPropertyBool("modified", member->isModified());

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Media Properties")) {
			if (ImGui::BeginTable("##CastMediaProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showPropertyBool("linked", info && !info->fileName.empty());

				showPropertyBool("loaded", member->isLoaded());

				ImGuiImage media = getImageID(member);

				ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
				showProperty("media", &grayColor, "%s", media.id == 0 ? "empty" : "...");

				// Not using showProperty() here because we want to show a
				// thumbnail of the media instead of text.
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("thumbnail");
				ImGui::TableSetColumnIndex(1);
				if (media.id) {
					showImage(media, "media", 16.f); // Show as small preview
				} else {
					ImGui::Text("empty");
				}

				uint32 size = member->_size;
				if (size < 1024) {
					showProperty("size", "%u bytes", size);
				} else if (size < 1024 * 1024) {
					showProperty("size", "%.2f KB", size / 1024.0f);
				} else {
					showProperty("size", "%.2f MB", size / (1024.0f * 1024.0f));
				}

				showPropertyBool("mediaReady", true);

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Graphic Properties")) {
			if (ImGui::BeginTable("##CastGraphicProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showPropertyBool("hilite", member->_hilite);

				Common::Point regPoint = member->getRegistrationOffset();
				showProperty("regPoint", "point(%d, %d)", regPoint.x, regPoint.y);

				showProperty("width", "%d", member->_initialRect.width());

				showProperty("height", "%d", member->_initialRect.height());

				showProperty("rect", "rect(%d, %d, %d, %d)",
					member->_initialRect.left,
					member->_initialRect.top,
					member->_initialRect.right,
					member->_initialRect.bottom);

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

void drawCastProps(Cast *cast) {
	assert(cast != nullptr);
	if (ImGui::BeginTabItem("Cast")) {

		if (ImGui::CollapsingHeader("Cast Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("##CastProperties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {

				showProperty("name", "%s", cast->getCastName().c_str());

				showProperty("number", "%d", cast->_castLibID);

				showProperty("fileName", "%s", cast->getMacName().c_str());

				showProperty("preLoadMode", "STUB");

				showProperty("selection", "STUB");

				ImGui::EndTable();
			}
		}

		ImGui::EndTabItem();
	}
}

// Per filmloop curent frame, keyed by cast member pointer
static Common::HashMap<FilmLoopCastMember *, int> _filmLoopCurrentFrame;

void drawFilmLoopCMprops(FilmLoopCastMember *member) {
	assert(member != nullptr);
	if (ImGui::BeginTabItem("Film Loop")) {
		if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("##FilmLoopProps", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
				showPropertyBool("looping", member->_looping);
				showPropertyBool("enableSound", member->_enableSound);
				showPropertyBool("crop", member->_crop);
				showPropertyBool("center", member->_center);
				if (member->_score)
					showProperty("frameCount", "%d", (int)member->_score->_scoreCache.size());
				ImGui::EndTable();
			}
		}

		Score *score = member->_score;
		if (!score || score->_scoreCache.empty()) {
			ImGui::TextDisabled("No frames loaded");
			ImGui::EndTabItem();
			return;
		}

		int numFrames = (int)score->_scoreCache.size();

		// find highest active channel
		int maxChannel = 0;
		for (int f = 0; f < numFrames; f++) {
			Frame *frame = score->_scoreCache[f];
			if (!frame) continue;
			for (int ch = 1; ch < (int)frame->_sprites.size(); ch++) {
				if (frame->_sprites[ch] && frame->_sprites[ch]->_castId.member != 0)
					maxChannel = MAX(maxChannel, ch);
			}
		}

		if (maxChannel == 0) {
			ImGui::TextDisabled("No sprite data");
			ImGui::EndTabItem();
			return;
		}

		// Initialize current frame for this member if needed
		if (!_filmLoopCurrentFrame.contains(member))
			_filmLoopCurrentFrame[member] = 0;
		int &currentFrame = _filmLoopCurrentFrame[member];

		const float cellW = 30.0f;
		const float cellH = 18.0f;
		const float labelW = 24.0f;
		const float rulerH = 16.0f;
		float gridW = labelW + numFrames * cellW;
		float gridH = rulerH + maxChannel * cellH;

		ImGui::Spacing();
		ImGui::Text("Score");
		ImGui::Separator();

		// Frame navigation controls
		if (ImGui::Button("|<")) currentFrame = 0;
		ImGui::SameLine();
		if (ImGui::Button("<") && currentFrame > 0) currentFrame--;
		ImGui::SameLine();
		ImGui::Text("Frame %d / %d", currentFrame + 1, numFrames);
		ImGui::SameLine();
		if (ImGui::Button(">") && currentFrame < numFrames - 1) currentFrame++;
		ImGui::SameLine();
		if (ImGui::Button(">|")) currentFrame = numFrames - 1;

		// Frame preview, show thumbnails of all active sprites in the current frame
		Frame *previewFrame = score->_scoreCache[currentFrame];
		Cast *cast = member->getCast();
		if (previewFrame && cast) {
			const float thumbSize = 48.0f;
			bool anySprite = false;
			for (int ch = 1; ch <= maxChannel; ch++) {
				if (ch >= (int)previewFrame->_sprites.size()) break;
				Sprite *sp = previewFrame->_sprites[ch];
				if (!sp || sp->_castId.member == 0) continue;

				CastMember *cm = cast->getCastMember(sp->_castId.member, true);
				if (!cm) continue;

				if (!anySprite) {
					ImGui::Separator();
					ImGui::Text("Frame preview");
					anySprite = true;
				}

				ImGuiImage imgID = getImageID(cm);
				ImGui::BeginGroup();
				if (imgID.id) {
					showImage(imgID, cm->getName().c_str(), thumbSize);
				} else {
					// No image -> draw a placeholder box
					ImVec2 pos = ImGui::GetCursorScreenPos();
					ImGui::GetWindowDrawList()->AddRect(pos, ImVec2(pos.x + thumbSize, pos.y + thumbSize), _state->theme->borderColor);
					ImGui::Dummy(ImVec2(thumbSize, thumbSize));
				}
				ImGui::Text("ch%d", ch);
				ImGui::EndGroup();
				ImGui::SameLine();
			}
			if (anySprite)
				ImGui::NewLine();
		}
		ImGui::Separator();

		// Scrollable child region
		float scrollbarH = ImGui::GetStyle().ScrollbarSize;
		ImGui::BeginChild("##FilmLoopScoreChild", ImVec2(0, MIN(gridH + scrollbarH + 4.0f, 200.0f)), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImDrawList *dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();

		// Ruler
		for (int f = 0; f < numFrames; f++) {
			float x = origin.x + labelW + f * cellW;
			float y = origin.y;
			ImVec2 rMin = ImVec2(x, y);
			ImVec2 rMax = ImVec2(x + cellW, y + rulerH);
			ImU32 rulerCol = ((f + 1) % 5 == 0) ? _state->theme->tableDarkColor : _state->theme->tableLightColor;
			dl->AddRectFilled(rMin, rMax, rulerCol);
			addThinRect(dl, rMin, rMax, _state->theme->borderColor);
			Common::String label = Common::String::format("%d", f + 1);
			ImVec2 textSz = ImGui::CalcTextSize(label.c_str());
			dl->AddText(ImVec2(x + (cellW - textSz.x) * 0.5f, y + (rulerH - textSz.y) * 0.5f), _state->theme->gridTextColor, label.c_str());
		}

		// Playhead
		{
			float px = origin.x + labelW + currentFrame * cellW;
			dl->AddLine(ImVec2(px, origin.y), ImVec2(px, origin.y + gridH), _state->theme->playhead_color, 2.0f);
			dl->AddTriangleFilled(
				ImVec2(px - 5.0f, origin.y),
				ImVec2(px + 5.0f, origin.y),
				ImVec2(px, origin.y + 8.0f),
				_state->theme->playhead_color);
		}

		// Channel rows
		for (int ch = 1; ch <= maxChannel; ch++) {
			float y = origin.y + rulerH + (ch - 1) * cellH;

			// label cells
			ImVec2 lblMin = ImVec2(origin.x, y);
			ImVec2 lblMax = ImVec2(origin.x + labelW, y + cellH);
			dl->AddRectFilled(lblMin, lblMax, _state->theme->tableDarkColor);
			addThinRect(dl, lblMin, lblMax, _state->theme->borderColor);
			Common::String chLabel = Common::String::format("%d", ch);
			ImVec2 chSz = ImGui::CalcTextSize(chLabel.c_str());
			dl->AddText(ImVec2(origin.x + (labelW - chSz.x) * 0.5f, y + (cellH - chSz.y) * 0.5f), _state->theme->gridTextColor, chLabel.c_str());

			// Pass 1: cell backgrounds
			for (int f = 0; f < numFrames; f++) {
				float x = origin.x + labelW + f * cellW;
				ImVec2 cMin = ImVec2(x, y);
				ImVec2 cMax = ImVec2(x + cellW, y + cellH);
				ImU32 col = ((f + 1) % 5 == 0) ? _state->theme->tableDarkColor : _state->theme->tableLightColor;
				dl->AddRectFilled(cMin, cMax, col);
				addThinRect(dl, cMin, cMax, _state->theme->borderColor);
			}

			// Pass 2: span bars
			int f = 0;
			while (f < numFrames) {
				Frame *frame = score->_scoreCache[f];
				if (!frame || ch >= (int)frame->_sprites.size() || !frame->_sprites[ch] || frame->_sprites[ch]->_castId.member == 0) {
					f++;
					continue;
				}

				// Find span end
				int spanStart = f;
				int memberNum = frame->_sprites[ch]->_castId.member;
				int spanEnd = f;
				while (spanEnd + 1 < numFrames) {
					Frame *nf = score->_scoreCache[spanEnd + 1];
					if (nf && ch < (int)nf->_sprites.size() && nf->_sprites[ch] && nf->_sprites[ch]->_castId.member == memberNum)
						spanEnd++;
					else
						break;
				}

				float x1 = origin.x + labelW + spanStart * cellW;
				float x2 = origin.x + labelW + (spanEnd + 1) * cellW;
				float cy = y + cellH * 0.5f;
				float pad = 1.0f;

				int colorIdx = memberNum % 6;
				ImU32 barColor = _state->theme->contColors[colorIdx];

				dl->AddRectFilled(ImVec2(x1, y + pad), ImVec2(x2 - 1.0f, y + cellH - pad), barColor);
				dl->AddLine(ImVec2(x1 + 6.0f, cy), ImVec2(x2 - 6.0f, cy), _state->theme->gridTextColor, 1.0f);
				dl->AddCircle(ImVec2(x1 + 4.0f, cy), 3.0f, _state->theme->gridTextColor, 0, 1.5f);
				dl->AddRect(ImVec2(x2 - 7.0f, cy - 3.0f), ImVec2(x2 - 1.0f, cy + 3.0f), _state->theme->gridTextColor, 0.0f, 0, 1.5f);

				float spanW = x2 - x1 - 8.0f;
				Common::String label = Common::String::format("%d", memberNum);
				if (spanW > 4.0f) {
					float textY = y + (cellH - ImGui::GetTextLineHeight()) * 0.5f;
					// clipText function is static in dt-score.cpp, so inline a simple clip
					Common::String clipped = label;
					if (ImGui::CalcTextSize(clipped.c_str()).x > spanW) clipped = "";
					if (!clipped.empty())
						dl->AddText(ImVec2(x1 + 4.0f, textY), _state->theme->gridTextColor, clipped.c_str());
				}

				// Tooltip
				ImGui::SetCursorScreenPos(ImVec2(x1, y));
				ImGui::InvisibleButton(Common::String::format("##fl_%d_%d", ch, spanStart).c_str(), ImVec2(x2 - x1, cellH));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("Channel %d | Frames %d-%d | Cast member %d", ch, spanStart + 1, spanEnd + 1, memberNum);
					ImGui::EndTooltip();
				}

				f = spanEnd + 1;
			}
		}

		// Advance the cursor
		ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + gridH));
		ImGui::Dummy(ImVec2(gridW, 0));

		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}

void drawCMTypeProps(CastMember *member) {
	assert(member != nullptr);
	switch (member->_type) {
	case kCastBitmap:
		drawBitmapCMprops(static_cast<BitmapCastMember *>(member));
		break;
	case kCastText:
		drawTextCMprops(static_cast<TextCastMember *>(member));
		break;
	case kCastRichText:
		drawRichTextCMprops(static_cast<RichTextCastMember *>(member));
		break;
	case kCastButton:
		// To add when ButtonCastMember is implemented
		break;
	case kCastShape:
		drawShapeCMprops(static_cast<ShapeCastMember *>(member));
		break;
	case kCastFilmLoop:
		drawFilmLoopCMprops(static_cast<FilmLoopCastMember *>(member));
		break;
	case kCastTypeAny:
	case kCastTypeNull:
	case kCastPalette:
	case kCastPicture:
	case kCastSound:
	case kCastMovie:
	case kCastDigitalVideo:
	case kCastLingoScript:
	case kCastTransition:
	case kCastXtra:
		// TODO
		break;
	}
}

int columnSizeForThumbnail(const ImGuiImage& imgID, float imageDrawSize, float padding) {
	if (imgID.width > imgID.height) {
		return imageDrawSize + padding;
	} else {
		return (imageDrawSize * imgID.width / imgID.height) + padding;
	}
}

void showCastDetails() {
	if (!_state->_w.castDetails)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 480), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast Details", &_state->_w.castDetails)) {
		CastMember *member = _state->_castDetails._castMember;
		assert(member != nullptr);

		CastType memberType = member->_type;

		// Columns to display details on the right of the bitmap
		if (ImGui::BeginTable("##CastDetailsHeaderTable", 2)) {
			ImGuiImage imgID = getImageID(member);


			ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthFixed, columnSizeForThumbnail(imgID, 64.f, 2.f));
			ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);

			// Show Bitmap
			showImageWrappedBorder(imgID, member->getName().c_str(), 64.f);

			// Move to the right of the Bitmap
			ImGui::TableSetColumnIndex(1);

			// Show Name of member
			ImGui::Text(getDisplayName(member).c_str());
			ImGui::Text(castType2str(memberType));

			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::BeginTabBar("#PropertyInspectors")) {

			drawBaseCMprops(member);

			drawCMTypeProps(member);

			drawCastProps(member->getCast());

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

} // namespace DT
} // namespace Director
