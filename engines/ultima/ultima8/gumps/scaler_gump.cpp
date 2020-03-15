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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/scaler_gump.h"

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/scaler.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ScalerGump, DesktopGump)

ScalerGump::ScalerGump(int32 x, int32 y, int32 width, int32 height) :
		DesktopGump(x, y, width, height),
		_swidth1(width), _sheight1(height),
		_scaler1(nullptr), _buffer1(nullptr),
		_swidth2(width), _sheight2(height),
		_scaler2(nullptr), _buffer2(nullptr),
		_width(width), _height(height) {

	setupScaling();
	_buffer1->Fill32(0, 0, 0, _dims.w, _dims.h);
}

ScalerGump::~ScalerGump() {
	FORGET_OBJECT(_buffer1);
	FORGET_OBJECT(_buffer2);
}

void ScalerGump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Skip the clipping rect/origin setting, since they will already be set
	// correctly by our parent.
	// (Or maybe I'm just to lazy to figure out the correct coordinates
	//  to use to compensate for the flipping... -wjp :-) )

	// Don't paint if hidden
	if (IsHidden()) return;

	// No scaling or filtering
	if (!_buffer1) {
		PaintChildren(surf, lerp_factor, scaled);
		return;
	}

	// Render to texture
	_buffer1->BeginPainting();
	PaintChildren(_buffer1, lerp_factor, true);
	_buffer1->EndPainting();

	if (!_buffer2) {
		DoScalerBlit(_buffer1->GetSurfaceAsTexture(), _swidth1, _sheight1, surf, _width, _height, _scaler1);
	} else {
		_buffer2->BeginPainting();
		DoScalerBlit(_buffer1->GetSurfaceAsTexture(), _swidth1, _sheight1, _buffer2, _swidth2, _sheight2, _scaler1);
		_buffer2->EndPainting();

		DoScalerBlit(_buffer2->GetSurfaceAsTexture(), _swidth2, _sheight2, surf, _width, _height, _scaler2);
	}

	int32 scalex = (_width << 16) / _swidth1;
	int32 scaley = (_height << 16) / _sheight1;

	// Iterate all children
	Std::list<Gump *>::reverse_iterator it = _children.rbegin();
	Std::list<Gump *>::reverse_iterator end = _children.rend();

	while (it != end) {
		Gump *g = *it;
		// Paint if not closing
		if (!g->IsClosing())
			g->PaintCompositing(surf, lerp_factor, scalex, scaley);

		++it;
	}
}

void ScalerGump::DoScalerBlit(Texture *src, int swidth, int sheight, RenderSurface *dest, int dwidth, int dheight, const Scaler *scaler) {
	bool ok = true;

	// Cheap and nasty method to use a 2x scaler to do a 2.4x scale vertically
	if (dwidth == 640 && swidth == 320 && dheight == 480 && sheight == 200 && !scaler->ScaleArbitrary()) {
		ok = dest->ScalerBlit(src, 0, 0, swidth, 1, 0, 0, dwidth, 2, scaler);

		int d = 1, s = 0;
		while (d < 468 && ok) {
			ok = dest->ScalerBlit(src, 0, s, swidth, 3, 0, d, dwidth, 6, scaler);
			d += 5;
			s += 2;

			if (!ok) break;

			ok = dest->ScalerBlit(src, 0, s, swidth, 4, 0, d, dwidth, 8, scaler);
			d += 7;
			s += 3;
		}

		while (d < 478 && ok) {
			ok = dest->ScalerBlit(src, 0, s, swidth, 3, 0, d, dwidth, 6, scaler);
			d += 5;
			s += 2;
		}
	} else {
		ok = dest->ScalerBlit(src, 0, 0, swidth, sheight, 0, 0, dwidth, dheight, scaler);
	}

	if (!ok) {
		dest->StretchBlit(src, 0, 0, swidth, sheight, 0, 0, dwidth, dheight);
	}
}

// Convert a parent relative point to a gump point
void ScalerGump::ParentToGump(int32 &px, int32 &py, PointRoundDir r) {
	px -= _x;
	px *= _dims.w;
	if (px < 0 && r == ROUND_TOPLEFT) px -= (_width - 1);
	if (px > 0 && r == ROUND_BOTTOMRIGHT) px += (_width - 1);
	px /= _width;

	py -= _y;
	py *= _dims.h;
	if (py < 0 && r == ROUND_TOPLEFT) py -= (_height - 1);
	if (py > 0 && r == ROUND_BOTTOMRIGHT) py += (_height - 1);
	py /= _height;
}

// Convert a gump point to parent relative point
void ScalerGump::GumpToParent(int32 &gx, int32 &gy, PointRoundDir r) {
	gx *= _width;
	if (gx < 0 && r == ROUND_TOPLEFT) gx -= (_dims.w - 1);
	if (gx > 0 && r == ROUND_BOTTOMRIGHT) gx += (_dims.w - 1);
	gx /= _dims.w;
	gx += _x;

	gy *= _height;
	if (gy < 0 && r == ROUND_TOPLEFT) gy -= (_dims.h - 1);
	if (gy > 0 && r == ROUND_BOTTOMRIGHT) gy += (_dims.h - 1);
	gy /= _dims.h;
	gy += _y;
}

void ScalerGump::RenderSurfaceChanged() {
	// Resize the gump to match the RenderSurface
	Rect new_dims;
	_parent->GetDims(new_dims);

	_width = new_dims.w;
	_height = new_dims.h;

	setupScaling();

	Gump::RenderSurfaceChanged();
}

void ScalerGump::setupScaling() {
	FORGET_OBJECT(_buffer1);
	FORGET_OBJECT(_buffer2);

	_swidth1 = 320;
	_sheight1 = 200;
	_swidth2 = 0;
	_sheight2 = 0;
	const Scaler *point = &Ultima8Engine::get_instance()->point_scaler;
	_scaler1 = _scaler2 = point;

	if (_swidth1 < 0) _swidth1 = -_swidth1;
	else if (_swidth1 == 0) _swidth1 = _width;
	else if (_swidth1 < 100) _swidth1 = _width / _swidth1;

	if (_sheight1 < 0) _sheight1 = -_sheight1;
	else if (_sheight1 == 0) _sheight1 = _height;
	else if (_sheight1 < 100) _sheight1 = _height / _sheight1;

	if (_swidth2 < 0) _swidth2 = -_swidth2;
	else if (_swidth2 != 0 && _swidth2 < 100) _swidth2 = _width / _swidth2;

	if (_sheight2 < 0) _sheight2 = -_sheight2;
	else if (_sheight2 != 0 && _sheight2 < 100) _sheight2 = _height / _sheight2;

	_dims.w = _swidth1;
	_dims.h = _sheight1;

	// We don't care, we are not going to support filters, at least not at the moment
	if (_swidth1 == _width && _sheight1 == _height) return;

	_buffer1 = RenderSurface::CreateSecondaryRenderSurface(_swidth1, _sheight1);

	// _scaler2's factor isn't set so auto detect
	if (_swidth2 == 0 || _sheight2 == 0) {
		// scaler 1 is arbitrary so _scaler2 not required
		if (_scaler1->ScaleArbitrary()) return;

		_swidth2 = _swidth1 * 32;
		_sheight2 = _sheight1 * 32;
		for (int i = 31; i >= 0; i--) {
			if (_scaler1->ScaleBits() & (1 << i)) {
				if (_swidth2 > _width || _sheight2 > _height) {
					_swidth2 = _swidth1 * i;
					_sheight2 = _sheight1 * i;
				}
			}
		}
	}

	// _scaler2 is required
	if (_swidth2 != _width || _sheight2 != _height) {
		// Well almost, in this situation we code in DoScalerBlit to do this for us
		// _scaler2 not required
		if (_width == 640 && _height == 480 &&
		        _swidth2 == 640 && _sheight2 == 400 &&
		        _swidth1 == 320 && _sheight2 == 200) {
			return;
		}

		_buffer2 = RenderSurface::CreateSecondaryRenderSurface(_swidth2, _sheight2);
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
