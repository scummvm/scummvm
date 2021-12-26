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

#ifndef ZVISION_FOG_H
#define ZVISION_FOG_H

#include "zvision/graphics/graphics_effect.h"

namespace ZVision {

class ZVision;

// Used by Zork: Nemesis for the mixing chamber gas effect in the gas puzzle (location tt5e, when the blinds are down)
class FogFx : public GraphicsEffect {
public:

	FogFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, EffectMap *Map, const Common::String &clouds);
	~FogFx() override;

	const Graphics::Surface *draw(const Graphics::Surface &srcSubRect) override;

	void update() override;

private:
	EffectMap *_map;
	Graphics::Surface _fog;
	uint8 _r, _g, _b;
	int32 _pos;
	Common::Array< Common::Array< bool > > _mp;
	uint16 _colorMap[32];
};
} // End of namespace ZVision

#endif // ZVISION_FOG_H
