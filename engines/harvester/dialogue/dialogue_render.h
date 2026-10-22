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

#ifndef HARVESTER_DIALOGUE_RENDER_H
#define HARVESTER_DIALOGUE_RENDER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Graphics {
class Font;
class Screen;
}

namespace Harvester {
namespace DialogueRendering {

static const int kDialogueSubtitleTextWidth = 316;
static const int kDialogueTextEntryMaxWidth = 550;
static const int kDialogueKeywordTextY = 34;
static const int kDialogueTopicStartX = 168;
static const int kDialogueTopicEndX = 482;
static const int kDialogueOtherStartY = 170;
static const int kDialogueOtherEndY = 193;

int getDialogueTextLineHeight(const Graphics::Font &font);
int resolveDialogueTextboxIndex(uint lineCount);
int resolveDialogueResponseTextboxIndex(uint responseRowCount);
void drawDialogueTextLines(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::Array<Common::String> &lines, int x, int y, int width);
void wrapDialogueTextLikeNative(const Graphics::Font &font, bool usesCft,
		const Common::String &text, int width, Common::Array<Common::String> &lines);
int getDialogueMenuItemAt(const Graphics::Font &font, uint itemCount, const Common::Point &mousePos);
bool isDialogueOtherHit(const Common::Point &mousePos);

} // End of namespace DialogueRendering
} // End of namespace Harvester

#endif // HARVESTER_DIALOGUE_RENDER_H
