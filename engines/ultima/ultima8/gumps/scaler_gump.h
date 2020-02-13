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

#ifndef ULTIMA8_GUMPS_ScalerGump_H
#define ULTIMA8_GUMPS_ScalerGump_H

#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderSurface;
struct Texture;
class Scaler;

class ScalerGump : public DesktopGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	//! ScalerGump constructor
	//! \param x x coordinate of our top-left corner in parent
	//! \param y y coordinate
	//! \param width width after scaling (usually screenwidth)
	//! \param height height after scaling (usually screenheight)
	//! \param swidth1 width before scaling (usually game-width)
	//! \param sheight1 height before scaling (usually game-height)
	//! \param scaler1 scaler to use
	ScalerGump(int32 x, int32 y, int32 width, int32 height);
	~ScalerGump(void) override;

	void Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	void RenderSurfaceChanged() override;
	void ParentToGump(int32 &px, int32 &py,
	                          PointRoundDir r = ROUND_TOPLEFT) override;
	void GumpToParent(int32 &gx, int32 &gy,
	                          PointRoundDir r = ROUND_TOPLEFT) override;

	void GetScaledSize(int32 &sw, int32 &sh) const {
		sw = swidth1;
		sh = sheight1;
	}

protected:
	int                     swidth1;
	int                     sheight1;
	const Scaler *scaler1;
	RenderSurface           *buffer1;

	int                     swidth2;
	int                     sheight2;
	const Scaler *scaler2;
	RenderSurface           *buffer2;

	int32                   width;
	int32                   height;

private:
	void setupScaling();

	void DoScalerBlit(Texture *src, int swidth, int sheight, RenderSurface *dest, int dwidth, int dheight, const Scaler *scaler);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
