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

#include "harvester/dialogue/dialogue_render.h"
#include "harvester/dialogue/dialogue_session.h"

#include "common/util.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "harvester/cft_font.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"
#include "harvester/harvester.h"
#include "harvester/scene/room_presentation.h"

namespace Harvester {
namespace DialogueRendering {

int getDialogueTextLineHeight(const Graphics::Font &font) {
	return font.getFontHeight() + 2;
}

int resolveDialogueTextboxIndex(uint lineCount) {
	if (lineCount <= 1)
		return 0;
	if (lineCount < 3)
		return 1;
	if (lineCount < 4)
		return 2;
	if (lineCount < 6)
		return 3;
	if (lineCount < 7)
		return 4;
	if (lineCount < 9)
		return 5;
	if (lineCount < 11)
		return 6;
	return 7;
}

int resolveDialogueResponseTextboxIndex(uint responseRowCount) {
	if (responseRowCount == 0)
		return 0;
	if (responseRowCount < 3)
		return 3;
	if (responseRowCount < 4)
		return 4;
	if (responseRowCount < 6)
		return 5;
	return 6;
}

void drawDialogueTextLines(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::Array<Common::String> &lines, int x, int y, int width) {
	const int lineHeight = getDialogueTextLineHeight(font);
	for (uint i = 0; i < lines.size(); ++i)
		font.drawString(&screen, lines[i], x, y + (int)i * lineHeight, width, 0);
}

void wrapDialogueTextLikeNative(const Graphics::Font &font, bool usesCft,
		const Common::String &text, int width, Common::Array<Common::String> &lines) {
	lines.clear();
	if (text.empty())
		return;

	if (!usesCft || width <= 0) {
		font.wordWrapText(text, width, lines);
		if (lines.empty())
			lines.push_back(text);
		return;
	}

	wrapCftTextByCharacterCount(font, text, width, lines);
}

int getDialogueMenuItemAt(const Graphics::Font &font, uint itemCount, const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;
	if (mousePos.x < kDialogueTopicStartX || mousePos.x > kDialogueTopicEndX)
		return -1;
	if (mousePos.y < kDialogueKeywordTextY)
		return -1;

	const int lineHeight = getDialogueTextLineHeight(font);
	const int selection = (mousePos.y - kDialogueKeywordTextY) / MAX<int>(1, lineHeight);
	return selection >= 0 && selection < (int)itemCount ? selection : -1;
}

bool isDialogueOtherHit(const Common::Point &mousePos) {
	return mousePos.x >= kDialogueTopicStartX && mousePos.x <= kDialogueTopicEndX &&
		mousePos.y >= kDialogueOtherStartY && mousePos.y <= kDialogueOtherEndY;
}

} // End of namespace DialogueRendering

namespace {

static const int kDialogueOverlayX = 167;
static const int kDialogueOverlayY = 6;
static const int kDialogueLeftHeadX = 6;
static const int kDialogueRightHeadX = 474;
static const int kDialogueHeadY = 6;
static const int kDialogueSubtitleTextX = 178;
static const int kDialogueSubtitleTextY = 12;
static const int kDialogueKeywordTextX = 178;
static const int kDialogueKeywordTextWidth = 300;
static const int kDialogueOtherX = 171;
static const int kDialogueOtherY = 183;
static const int kDialogueTextEntryX = 208;
static const int kDialogueTextEntryY = 184;
static const int kDialogueOtherWidth = 120;
static const int kDialogueKeywordTitleYOffset = 4;
static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;

} // End of anonymous namespace

Graphics::Screen *RoomNpcDialogueSession::getActiveScreen() const {
	return _engine.getScreen();
}

void RoomNpcDialogueSession::drawFontString(const Graphics::Font &font, bool usesCft, const Common::String &text,
		int x, int y, int width, byte color) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	if (usesCft)
		font.drawString(activeScreen, text, x, y, width, 0);
	else
		RoomPresentation::drawShadowedString(*activeScreen, font, text, x, y, width, color);
}

void RoomNpcDialogueSession::drawDialogueOverlay(const IndexedBitmap *overlayBitmap,
		const Common::Array<Common::String> *subtitleLines, const Common::Array<Common::String> *topics,
		int hoveredTopicIndex, bool hoverOther, const Common::String *textEntryValue,
		bool textEntryCursorVisible) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	setScaledPalette(*activeScreen, _palette, _paletteBrightness);
	RoomPresentation::blitBitmap(*activeScreen, _backdrop, 0, 0);
	if (_leftHeadVisible && _leftHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
	if (_rightHeadVisible && _rightHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
	if (overlayBitmap && overlayBitmap->isValid())
		blitTransparentBitmap(*activeScreen, *overlayBitmap, kDialogueOverlayX, kDialogueOverlayY);

	if (subtitleLines) {
		if (_subtitleFontUsesCft) {
			DialogueRendering::drawDialogueTextLines(*activeScreen, *_subtitleFont, *subtitleLines,
				kDialogueSubtitleTextX, kDialogueSubtitleTextY, DialogueRendering::kDialogueSubtitleTextWidth);
		} else {
			const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_subtitleFont);
			for (uint i = 0; i < subtitleLines->size(); ++i) {
				RoomPresentation::drawShadowedString(*activeScreen, *_subtitleFont, (*subtitleLines)[i],
					kDialogueSubtitleTextX,
					kDialogueSubtitleTextY + (int)i * lineHeight,
					DialogueRendering::kDialogueSubtitleTextWidth, kTextColorNormal);
			}
		}
	}

	if (topics) {
		if (_menuTextConfig.hasDialogueKeywordLabel()) {
			const Common::String &title = _menuTextConfig.dialogueKeywordLabel;
			const int titleWidth = _highlightFont->getStringWidth(title);
			const int titleX = kDialogueOverlayX +
				MAX<int>(0, ((int)_keywordBitmap.width - titleWidth) / 2);
			drawFontString(*_highlightFont, _highlightFontUsesCft, title, titleX,
				kDialogueOverlayY + kDialogueKeywordTitleYOffset, titleWidth,
				kTextColorNormal);
		}

		const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_menuFont);
		for (uint i = 0; i < topics->size(); ++i) {
			const bool highlighted = (int)i == hoveredTopicIndex;
			const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
			const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
			drawFontString(font, usesCft, (*topics)[i], kDialogueKeywordTextX,
				DialogueRendering::kDialogueKeywordTextY + (int)i * lineHeight, kDialogueKeywordTextWidth,
				highlighted ? kTextColorNormal : kTextColorHover);
		}

		const Graphics::Font &otherFont = hoverOther ? *_highlightFont : *_menuFont;
		const bool otherUsesCft = hoverOther ? _highlightFontUsesCft : _menuFontUsesCft;
		drawFontString(otherFont, otherUsesCft, _menuTextConfig.dialogueOtherLabel,
			kDialogueOtherX, kDialogueOtherY,
			kDialogueOtherWidth, hoverOther ? kTextColorNormal : kTextColorHover);
	}

	if (textEntryValue) {
		drawFontString(*_highlightFont, _highlightFontUsesCft,
			*textEntryValue + (textEntryCursorVisible ? "~" : ""),
			kDialogueTextEntryX, kDialogueTextEntryY, DialogueRendering::kDialogueTextEntryMaxWidth,
			kTextColorNormal);
	}

	if (_entityManager)
		_entityManager->drawCursor(*activeScreen);
	activeScreen->makeAllDirty();
	activeScreen->update();
}

void RoomNpcDialogueSession::drawDialogueResponseMenu(const IndexedBitmap *textboxBitmap,
		const Common::Array<DialogueResponseOptionLayout> &options, int hoveredOptionIndex) {
	Graphics::Screen *activeScreen = getActiveScreen();
	if (!activeScreen)
		return;

	setScaledPalette(*activeScreen, _palette, _paletteBrightness);
	RoomPresentation::blitBitmap(*activeScreen, _backdrop, 0, 0);
	if (_leftHeadVisible && _leftHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _leftHeadBitmap, kDialogueLeftHeadX, kDialogueHeadY);
	if (_rightHeadVisible && _rightHeadBitmap.isValid())
		blitTransparentBitmap(*activeScreen, _rightHeadBitmap, kDialogueRightHeadX, kDialogueHeadY);
	if (textboxBitmap && textboxBitmap->isValid())
		blitTransparentBitmap(*activeScreen, *textboxBitmap, kDialogueOverlayX, kDialogueOverlayY);

	const Common::String &title = _menuTextConfig.dialogueResponsesLabel;
	const Graphics::Font &titleFont = *_highlightFont;
	const bool titleUsesCft = _highlightFontUsesCft;
	const int titleWidth = titleFont.getStringWidth(title);
	const int titleX = kDialogueSubtitleTextX + MAX<int>(0, (DialogueRendering::kDialogueSubtitleTextWidth - titleWidth) / 2);
	drawFontString(titleFont, titleUsesCft, title, titleX, 11, titleWidth, kTextColorNormal);

	const int lineHeight = DialogueRendering::getDialogueTextLineHeight(*_menuFont);
	int drawY = DialogueRendering::kDialogueKeywordTextY;
	for (uint i = 0; i < options.size(); ++i) {
		const bool highlighted = (int)i == hoveredOptionIndex;
		const Graphics::Font &font = highlighted ? *_highlightFont : *_menuFont;
		const bool usesCft = highlighted ? _highlightFontUsesCft : _menuFontUsesCft;
		const byte color = highlighted ? kTextColorNormal : kTextColorHover;

		for (const Common::String &wrappedLine : options[i].wrappedLines) {
			drawFontString(font, usesCft, wrappedLine, kDialogueSubtitleTextX, drawY,
				DialogueRendering::kDialogueSubtitleTextWidth, color);
			drawY += lineHeight;
		}
	}

	if (_entityManager)
		_entityManager->drawCursor(*activeScreen);
	activeScreen->makeAllDirty();
	activeScreen->update();
}

} // End of namespace Harvester
