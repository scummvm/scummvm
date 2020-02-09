/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef LIGHTFX_H_INCLUDED
#define LIGHTFX_H_INCLUDED

#include "zvision/graphics/graphics_effect.h"

namespace ZVision {

class ZVision;

class LightFx : public GraphicsEffect {
public:

	LightFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, EffectMap *Map, int8 delta, int8 minD = -127, int8 maxD = 127);
	~LightFx() override;

	const Graphics::Surface *draw(const Graphics::Surface &srcSubRect) override;

	void update() override;

private:
	EffectMap *_map;
	int32 _delta;
	bool _up;
	int32 _pos;

	int8 _minD;
	int8 _maxD;
};
} // End of namespace ZVision

#endif // LIGHTFX_H_INCLUDED
