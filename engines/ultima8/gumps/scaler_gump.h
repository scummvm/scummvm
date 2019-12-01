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

#include "desktop_gump.h"
#include "ultima8/std/containers.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class RenderSurface;
struct Texture;

namespace Pentagram {
class Scaler;
};

class ScalerGump : public DesktopGump {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! ScalerGump constructor
	//! \param x x coordinate of our top-left corner in parent
	//! \param y y coordinate
	//! \param width width after scaling (usually screenwidth)
	//! \param height height after scaling (usually screenheight)
	//! \param swidth1 width before scaling (usually game-width)
	//! \param sheight1 height before scaling (usually game-height)
	//! \param scaler1 scaler to use
	ScalerGump(int32 x, int32 y, int32 width, int32 height);
	virtual ~ScalerGump(void);

	virtual void Paint(RenderSurface *surf, int32 lerp_factor, bool scaled);
	virtual void RenderSurfaceChanged();
	virtual void ParentToGump(int &px, int &py,
	                          PointRoundDir r = ROUND_TOPLEFT);
	virtual void GumpToParent(int &gx, int &gy,
	                          PointRoundDir r = ROUND_TOPLEFT);

	void GetScaledSize(int32 &sw, int32 &sh) const {
		sw = swidth1;
		sh = sheight1;
	}
	void ChangeScaler(std::string scalername, int scalex, int scaley);

protected:
	int                     swidth1;
	int                     sheight1;
	const Pentagram::Scaler *scaler1;
	RenderSurface           *buffer1;

	int                     swidth2;
	int                     sheight2;
	const Pentagram::Scaler *scaler2;
	RenderSurface           *buffer2;

	int32                   width;
	int32                   height;

private:
	void SetupScalers();

	void DoScalerBlit(Texture *src, int swidth, int sheight, RenderSurface *dest, int dwidth, int dheight, const Pentagram::Scaler *scaler);

	static void         ConCmd_changeScaler(const Console::ArgvType &argv);     //!< "GuiApp::changeScaler" console command
	static void         ConCmd_listScalers(const Console::ArgvType &argv);      //!< "GuiApp::changeScaler" console command

};

} // End of namespace Ultima8

#endif
