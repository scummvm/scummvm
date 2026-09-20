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

#include "frame_viewport.h"

#include "common/util.h"

#include "scooby/graphics/indexed_frame.h"

namespace Scooby {

FrameViewport::FrameViewport(float left, float top, float width, float height)
	: _left(left), _top(top), _width(width), _height(height) {
}

FrameViewport FrameViewport::fromCurrentWindow() {
	int screenWidth = 320; // GetScreenWidth();
	int screenHeight = 240; // GetScreenHeight();
	int widthLimitedScale = screenWidth * 3 / (IndexedFrame::Height * 4);
	int scale = MAX(1, MIN(widthLimitedScale, screenHeight / IndexedFrame::Height));
	float height = static_cast<float>(IndexedFrame::Height * scale);
	float width = height * 4.0f / 3.0f;
	return FrameViewport((static_cast<float>(screenWidth) - width) / 2.0f,
						 (static_cast<float>(screenHeight) - height) / 2.0f, width, height);
}

void FrameViewport::draw(/*Texture2D texture*/) const {
	// Rectangle source = {0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height)};
	// Rectangle destination = {_left, _top, _width, _height};
	// DrawTexturePro(texture, source, destination, Vector2{0, 0}, 0, WHITE);
}
} // namespace Scooby