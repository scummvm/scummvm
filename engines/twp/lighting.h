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

#ifndef TWP_LIGHTING_H
#define TWP_LIGHTING_H

#include "twp/gfx.h"
#include "twp/spritesheet.h"

namespace Twp {

class Lighting: public Shader {
public:
	Lighting();
	virtual ~Lighting();

	void setSpriteSheetFrame(const SpriteSheetFrame &frame, const Texture &texture);

private:
	virtual void applyUniforms() final;

private:
	int32 _contentSizeLoc;
	int32 _spriteOffsetLoc;
	int32 _spritePosInSheetLoc;
	int32 _spriteSizeRelToSheetLoc;
	int32 _numberLightsLoc;
	int32 _ambientColorLoc;

	Math::Vector2d _contentSize;
	Math::Vector2d _spriteOffset;
	Math::Vector2d _spritePosInSheet;
	Math::Vector2d _spriteSizeRelToSheet;
};

} // namespace Twp

#endif
