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

#ifndef SCOOBY_FRAME_VIEWPORT_H
#define SCOOBY_FRAME_VIEWPORT_H

// Owns centered 4:3 correction and whole-row scaling for the fixed logical frame.
//
// The complete 320x224 logical texture is presented with point filtering.
// H32 centering occurs while the scene is composed into that texture; this
// host transform applies only the final 4:3 display correction.

namespace Scooby {

class FrameViewport {
public:
	// Builds the largest centered 4:3 viewport whose logical rows retain a whole-number scale.
	static FrameViewport fromCurrentWindow();

	// Draws the complete logical texture with deterministic point-filtered letterboxing.
	void draw(/*Texture2D texture*/) const;

private:
	FrameViewport(float left, float top, float width, float height);

	float _left;
	float _top;
	float _width;
	float _height;
};
} // namespace Scooby

#endif // SCOOBY_FRAME_VIEWPORT_H
