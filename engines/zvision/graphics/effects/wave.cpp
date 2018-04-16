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

#include "common/scummsys.h"

#include "zvision/graphics/effects/wave.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"

namespace ZVision {

WaveFx::WaveFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, int16 frames, int16 centerX, int16 centerY, float ampl, float waveln, float spd):
	GraphicsEffect(engine, key, region, ported) {

	_frame = 0;
	_frameCount = frames;

	_ampls.resize(_frameCount);
	_halfWidth = _region.width() / 2;
	_halfHeight = _region.height() / 2;

	int32 frmsize = _halfWidth * _halfHeight;

	float phase = 0;

	int16 quarterWidth = _halfWidth / 2;
	int16 quarterHeight = _halfHeight / 2;

	for (int16 i = 0; i < _frameCount; i++) {
		_ampls[i].resize(frmsize);

		for (int16 y = 0; y < _halfHeight; y++)
			for (int16 x = 0; x < _halfWidth; x++) {
				int16 dx = (x - quarterWidth);
				int16 dy = (y - quarterHeight);

				_ampls[i][x + y * _halfWidth] = (int8)(ampl * sin(sqrt(dx * dx / (float)centerX + dy * dy / (float)centerY) / (-waveln * 3.1415926) + phase));
			}
		phase += spd;
	}
}

WaveFx::~WaveFx() {
	for (uint16 i = 0; i < _ampls.size(); i++)
		_ampls[i].clear();
	_ampls.clear();
}

const Graphics::Surface *WaveFx::draw(const Graphics::Surface &srcSubRect) {
	for (int16 y = 0; y < _halfHeight; y++) {
		uint16 *abc  = (uint16 *)_surface.getBasePtr(0, y);
		uint16 *abc2  = (uint16 *)_surface.getBasePtr(0, _halfHeight + y);
		uint16 *abc3  = (uint16 *)_surface.getBasePtr(_halfWidth, y);
		uint16 *abc4  = (uint16 *)_surface.getBasePtr(_halfWidth, _halfHeight + y);

		for (int16 x = 0; x < _halfWidth; x++) {
			int8 amnt = _ampls[_frame][x + _halfWidth * y];

			int16 nX = x + amnt;
			int16 nY = y + amnt;

			if (nX < 0)
				nX = 0;
			if (nX >= _region.width())
				nX = _region.width() - 1;
			if (nY < 0)
				nY = 0;
			if (nY >= _region.height())
				nY = _region.height() - 1;
			*abc = *(const uint16 *)srcSubRect.getBasePtr(nX, nY);

			nX = x + amnt + _halfWidth;
			nY = y + amnt;

			if (nX < 0)
				nX = 0;
			if (nX >= _region.width())
				nX = _region.width() - 1;
			if (nY < 0)
				nY = 0;
			if (nY >= _region.height())
				nY = _region.height() - 1;
			*abc3 = *(const uint16 *)srcSubRect.getBasePtr(nX, nY);

			nX = x + amnt;
			nY = y + amnt + _halfHeight;

			if (nX < 0)
				nX = 0;
			if (nX >= _region.width())
				nX = _region.width() - 1;
			if (nY < 0)
				nY = 0;
			if (nY >= _region.height())
				nY = _region.height() - 1;
			*abc2 = *(const uint16 *)srcSubRect.getBasePtr(nX, nY);

			nX = x + amnt + _halfWidth;
			nY = y + amnt + _halfHeight;

			if (nX < 0)
				nX = 0;
			if (nX >= _region.width())
				nX = _region.width() - 1;
			if (nY < 0)
				nY = 0;
			if (nY >= _region.height())
				nY = _region.height() - 1;
			*abc4 = *(const uint16 *)srcSubRect.getBasePtr(nX, nY);

			abc++;
			abc2++;
			abc3++;
			abc4++;
		}
	}

	return &_surface;
}

void WaveFx::update() {
	_frame = (_frame + 1) % _frameCount;
}

} // End of namespace ZVision
