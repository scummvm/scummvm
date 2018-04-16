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

#ifndef GRAPHICS_EFFECT_H_INCLUDED
#define GRAPHICS_EFFECT_H_INCLUDED

#include "common/rect.h"
#include "common/list.h"
#include "graphics/surface.h"

#include "zvision/zvision.h"

namespace ZVision {

class ZVision;

class GraphicsEffect {
public:

	GraphicsEffect(ZVision *engine, uint32 key, Common::Rect region, bool ported) : _engine(engine), _key(key), _region(region), _ported(ported) {
		_surface.create(_region.width(), _region.height(), _engine->_resourcePixelFormat);
	}
	virtual ~GraphicsEffect() {}

	uint32 getKey() {
		return _key;
	}

	Common::Rect getRegion() {
		return _region;
	}

	bool isPort() {
		return _ported;
	}

	virtual const Graphics::Surface *draw(const Graphics::Surface &srcSubRect) {
		return &_surface;
	}

	virtual void update() {}

protected:
	ZVision *_engine;
	uint32 _key;
	Common::Rect _region;
	bool _ported;
	Graphics::Surface _surface;

// Static member functions
public:

};

struct EffectMapUnit {
	uint32 count;
	bool inEffect;
};

typedef Common::List<EffectMapUnit> EffectMap;

} // End of namespace ZVision

#endif // GRAPHICS_EFFECT_H_INCLUDED
