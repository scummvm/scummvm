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
	case kCastTypeAny:
	case kCastTypeNull:
	case kCastFilmLoop:
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
