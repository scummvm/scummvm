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

#include "zvision/graphics/effects/fog.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"

namespace ZVision {

FogFx::FogFx(ZVision *engine, uint32 key, Common::Rect region, bool ported, EffectMap *Map, const Common::String &clouds):
	GraphicsEffect(engine, key, region, ported) {

	_map = Map;

	_r = 0;
	_g = 0;
	_b = 0;

	_pos = 0;

	if (_engine->getSearchManager()->hasFile(clouds))
		_engine->getRenderManager()->readImageToSurface(clouds, _fog);
	else
		_engine->getRenderManager()->readImageToSurface("cloud.tga", _fog);

	_mp.resize(_fog.h);
	for (int16 i = 0; i < _fog.h; i++) {
		_mp[i].resize(_fog.w);
		for (int16 j = 0; j < _fog.w; j++)
			_mp[i][j] = true;
	}

	for (uint8 i = 0; i < 32; i++)
		_colorMap[i] = 0;
}

FogFx::~FogFx() {
	if (_map)
		delete _map;

	for (uint16 i = 0; i < _mp.size(); i++)
		_mp[i].clear();
	_mp.clear();
}

const Graphics::Surface *FogFx::draw(const Graphics::Surface &srcSubRect) {
	_surface.copyFrom(srcSubRect);
	EffectMap::iterator it = _map->begin();

	uint32 cnt = 0;

	for (uint16 j = 0; j < _surface.h; j++) {
		uint16 *lineBuf = (uint16 *)_surface.getBasePtr(0, j);

		for (uint16 i = 0; i < _surface.w; i++) {
			if (it->inEffect) {
				// Not 100% equivalent, but looks nice and not buggy
				uint8 sr, sg, sb;
				_engine->_resourcePixelFormat.colorToRGB(lineBuf[i], sr, sg, sb);
				uint16 fogColor = *(uint16 *)_fog.getBasePtr((i + _pos) % _fog.w, j);
				uint8 dr, dg, db;
				_engine->_resourcePixelFormat.colorToRGB(_colorMap[fogColor & 0x1F], dr, dg, db);
				uint16 fr = dr + sr;
				if (fr > 255)
					fr = 255;
				uint16 fg = dg + sg;
				if (fg > 255)
					fg = 255;
				uint16 fb = db + sb;
				if (fb > 255)
					fb = 255;
				lineBuf[i] = _engine->_resourcePixelFormat.RGBToColor(fr, fg, fb);
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

void FogFx::update() {
	_pos += _engine->getScriptManager()->getStateValue(StateKey_EF9_Speed);
	_pos %= _fog.w;

	uint8 dr = _engine->getScriptManager()->getStateValue(StateKey_EF9_R);
	uint8 dg = _engine->getScriptManager()->getStateValue(StateKey_EF9_G);
	uint8 db = _engine->getScriptManager()->getStateValue(StateKey_EF9_B);
	dr = CLIP((int)dr, 0, 31);
	dg = CLIP((int)dg, 0, 31);
	db = CLIP((int)db, 0, 31);

	if (dr != _r || dg != _g || db != _b) {
		if (_r > dr)
			_r--;
		else if (_r < dr)
			_r++;

		if (_g > dg)
			_g--;
		else if (_g < dg)
			_g++;

		if (_b > db)
			_b--;
		else if (_b < db)
			_b++;

		// Not 100% equivalent, but looks nice and not buggy

		_colorMap[31] = _engine->_resourcePixelFormat.RGBToColor(_r << 3, _g << 3, _b << 3);

		for (uint8 i = 0; i < 31; i++) {
			float perc = (float)i / 31.0;
			uint8 cr = (uint8)((float)_r * perc);
			uint8 cg = (uint8)((float)_g * perc);
			uint8 cb = (uint8)((float)_b * perc);
			_colorMap[i] = _engine->_resourcePixelFormat.RGBToColor(cr << 3, cg << 3, cb << 3);
		}
	}

	for (uint16 j = 0; j < _fog.h; j++) {
		uint16 *pix = (uint16 *)_fog.getBasePtr(0, j);

		for (uint16 i = 0; i < _fog.w; i++) {
			if (_mp[j][i]) {
				if ((pix[i] & 0x1F) == 0x1F) {
					pix[i]--;
					_mp[j][i] = false;
				} else
					pix[i]++;
			} else {
				if ((pix[i] & 0x1F) == 0) {
					pix[i]++;
					_mp[j][i] = true;
				} else
					pix[i]--;
			}
		}
	}
}

} // End of namespace ZVision
