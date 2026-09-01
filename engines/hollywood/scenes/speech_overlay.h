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

#ifndef HOLLYWOOD_SCENES_SPEECH_OVERLAY_H
#define HOLLYWOOD_SCENES_SPEECH_OVERLAY_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodFont;

// Wrapped subtitle text and its resolved scene position.
struct SpeechOverlay {
	bool visible;
	byte colorIndex;
	uint16 centerX;
	uint16 topY;
	Common::Array<Common::String> lines;
};

void wrapSpeechOverlayText(const Common::String &text, int anchorX,
	Common::Array<Common::String> &lines);
void layoutSpeechOverlay(SpeechOverlay &overlay, const HollywoodFont *font,
	int centerX, int anchorBottomY, int viewportXOffset = 0);
void drawSpeechOverlayText(const SpeechOverlay &overlay, HollywoodFont *font,
	Graphics::Surface &surface, int viewportXOffset = 0);

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_SPEECH_OVERLAY_H
