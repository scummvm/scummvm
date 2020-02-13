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

#ifndef WAVEFX_H_INCLUDED
#define WAVEFX_H_INCLUDED

#include "common/array.h"
#include "zvision/graphics/graphics_effect.h"

namespace ZVision {

class ZVision;

class WaveFx : public GraphicsEffect {
public:

	WaveFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, int16 frames, int16 centerX, int16 centerY, float ampl, float waveln, float spd);
	~WaveFx() override;

	const Graphics::Surface *draw(const Graphics::Surface &srcSubRect) override;

	void update() override;

private:
	int16 _frame;
	int16 _frameCount;
	int16 _halfWidth, _halfHeight;
	Common::Array< Common::Array< int8 > > _ampls;
};
} // End of namespace ZVision

#endif // WAVEFX_H_INCLUDED
