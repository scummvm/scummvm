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

#include "hollywood/scenes/speech_overlay.h"

#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

enum {
	kSpeechLineHeight = 20,
	kSpeechMaximumLineCount = 10,
	kSpeechMaximumLineCharacters = 0x32,
	kSpeechMinimumLineCharacters = 0x18,
	kSpeechEdgeWidth = 0xa0
};

static uint speechLineWidth(const HollywoodFont *font, const Common::String &text) {
	return font != nullptr && font->isLoaded() ? font->getStringWidth(text) + 2 : 0;
}

static uint speechOverlayWidth(const SpeechOverlay &overlay, const HollywoodFont *font) {
	uint textWidth = 0;
	for (uint i = 0; i < overlay.lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, speechLineWidth(font, overlay.lines[i]));
	return textWidth;
}

void wrapSpeechOverlayText(const Common::String &text, int anchorX,
		Common::Array<Common::String> &lines, SpeechOverlayWrapStyle style) {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars;
	uint lineShrink;
	if (style == kSpeechOverlayFixedEdgeWrap) {
		maxChars = anchorX < kSpeechEdgeWidth ||
				HollywoodEngine::kScreenWidth - anchorX < kSpeechEdgeWidth ? 0x24 : 0x32;
		lineShrink = 2;
	} else {
		maxChars = kSpeechMaximumLineCharacters;
		if (anchorX < kSpeechEdgeWidth)
			maxChars = MAX<int>(0, anchorX) * kSpeechMaximumLineCharacters / kSpeechEdgeWidth;
		else if (HollywoodEngine::kScreenWidth - anchorX < kSpeechEdgeWidth)
			maxChars = MAX<int>(0, HollywoodEngine::kScreenWidth - anchorX) *
				kSpeechMaximumLineCharacters / kSpeechEdgeWidth;
		maxChars = MAX<uint>(maxChars, kSpeechMinimumLineCharacters);
		lineShrink = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
	}

	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < kSpeechMaximumLineCount) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}

		lines.push_back(Common::String(source + cursor, end - cursor));
		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;
		maxChars = maxChars > lineShrink ? maxChars - lineShrink : 1;
	}
}

void layoutSpeechOverlay(SpeechOverlay &overlay, const HollywoodFont *font,
		int centerX, int anchorBottomY, int viewportXOffset) {
	const uint textWidth = speechOverlayWidth(overlay, font);
	int adjustedCenterX = centerX - viewportXOffset;
	if (((adjustedCenterX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e)
		adjustedCenterX = (textWidth & 1) == 0 ? 0x27e - (textWidth >> 1) : 0x27d - (textWidth >> 1);
	if (adjustedCenterX - (int)(textWidth >> 1) < 1)
		adjustedCenterX = (textWidth >> 1) + 1;

	const int adjustedTopY = MAX<int>(1,
		anchorBottomY - (int)overlay.lines.size() * kSpeechLineHeight);
	overlay.centerX = (uint16)MAX<int>(viewportXOffset,
		MIN<int>(adjustedCenterX + viewportXOffset,
			viewportXOffset + HollywoodEngine::kScreenWidth - 1));
	overlay.topY = (uint16)MIN<int>(adjustedTopY, HollywoodEngine::kScreenHeight - 1);
}

void drawSpeechOverlayText(const SpeechOverlay &overlay, HollywoodFont *font,
		Graphics::Surface &surface, int viewportXOffset, int viewportYOffset) {
	if (!overlay.visible || font == nullptr || !font->isLoaded())
		return;

	font->setShadowColor(0);
	for (uint lineIndex = 0; lineIndex < overlay.lines.size(); ++lineIndex) {
		const Common::String &line = overlay.lines[lineIndex];
		const int lineWidth = speechLineWidth(font, line);
		int x = (int)overlay.centerX - (lineWidth >> 1) - viewportXOffset;
		x = CLIP<int>(x, 0, MAX<int>(0, HollywoodEngine::kScreenWidth - lineWidth));
		const int y = (int)overlay.topY + lineIndex * kSpeechLineHeight - viewportYOffset;
		font->drawString(&surface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

} // End of namespace Hollywood
