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

#include "zvision/graphics/effects/light.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"

namespace ZVision {

LightFx::LightFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, EffectMap *Map, int8 delta, int8 minD, int8 maxD):
	GraphicsEffect(engine, key, region, ported) {
	_map = Map;
	_delta = delta;
	_up = true;
	_pos = 0;

	_minD = minD;
	if (_minD < -delta)
		_minD = -delta;

	_maxD = maxD;
	if (_maxD > delta)
		_maxD = delta;
}

LightFx::~LightFx() {
	if (_map)
		delete _map;
}

const Graphics::Surface *LightFx::draw(const Graphics::Surface &srcSubRect) {
	_surface.copyFrom(srcSubRect);
	EffectMap::iterator it = _map->begin();
	uint32 cnt = 0;

	uint32 dcolor = 0;

	if (_pos < 0) {
		uint8 cc = ((-_pos) & 0x1F) << 3;
		dcolor = _engine->_resourcePixelFormat.RGBToColor(cc, cc, cc);
	} else {
		uint8 cc = (_pos & 0x1F) << 3;
		dcolor = _engine->_resourcePixelFormat.RGBToColor(cc, cc, cc);
	}

	for (uint16 j = 0; j < _surface.h; j++) {
		uint16 *lineBuf = (uint16 *)_surface.getBasePtr(0, j);

		for (uint16 i = 0; i < _surface.w; i++) {
			if (it->inEffect) {
				if (_pos < 0) {
					lineBuf[i] -= dcolor;
				} else {
					lineBuf[i] += dcolor;
				}
			}
			cnt++;
			if (cnt >= it->count) {
				it++;
				cnt = 0;
			}
			if (it == _map->end())
				break;
		}
		if (it == _map->end())
			break;
	}

	return &_surface;
}

void LightFx::update() {
	if (_up)
		_pos++;
	else
		_pos--;

	if (_pos <= _minD) {
		_up = !_up;
		_pos = _minD;
	} else if (_pos >= _maxD) {
		_up = !_up;
		_pos = _maxD;
	}
}

} // End of namespace ZVision
