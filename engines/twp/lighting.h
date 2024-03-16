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

#define MAX_LIGHTS 50

namespace Twp {

struct Lights;

class Lighting : public Shader {
public:
	Lighting();
	virtual ~Lighting();

	void setSpriteOffset(const Math::Vector2d &offset);
	void setSpriteSheetFrame(const SpriteSheetFrame &frame, const Texture &getNumTextures, bool flipX);

	void update(const Lights &lights);

private:
	virtual void applyUniforms() final;

public:
	Math::Vector2d _contentSize;
	Math::Vector2d _spriteOffset;
	Math::Vector2d _spritePosInSheet;
	Math::Vector2d _spriteSizeRelToSheet;

	bool _debug = false; // if true change the way we draw the lights to debug them easily
	Color _ambientLight; // Ambient light color
	int u_numberLights = 0;
	float u_lightPos[3 * MAX_LIGHTS];
	float u_coneDirection[2 * MAX_LIGHTS];
	float u_coneCosineHalfConeAngle[MAX_LIGHTS];
	float u_coneFalloff[MAX_LIGHTS];
	float u_lightColor[3 * MAX_LIGHTS];
	float u_brightness[MAX_LIGHTS];
	float u_cutoffRadius[MAX_LIGHTS];
	float u_halfRadius[MAX_LIGHTS];

private:
	bool _currentDebug = false;
};

} // namespace Twp

#endif
