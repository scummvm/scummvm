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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/scaler_gump.h"

#include "ultima8/graphics/render_surface.h"
#include "ultima8/graphics/texture.h"
#include "ultima8/graphics/scaler.h"
#include "ultima8/graphics/scaler_manager.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/ultima8.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ScalerGump, DesktopGump)

ScalerGump::ScalerGump(int32 _x, int32 _y, int32 _width, int32 _height) :
	DesktopGump(_x, _y, _width, _height),
	swidth1(_width), sheight1(_height), scaler1(0), buffer1(0),
	swidth2(_width), sheight2(_height), scaler2(0), buffer2(0),
	width(_width), height(_height) {
	con.AddConsoleCommand("ScalerGump::changeScaler", ConCmd_changeScaler);
	con.AddConsoleCommand("ScalerGump::listScalers", ConCmd_listScalers);

	SetupScalers();
}

ScalerGump::~ScalerGump() {
	con.RemoveConsoleCommand(ConCmd_changeScaler);
	con.RemoveConsoleCommand(ConCmd_listScalers);

	FORGET_OBJECT(buffer1);
	FORGET_OBJECT(buffer2);
}

void ScalerGump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Skip the clipping rect/origin setting, since they will already be set
	// correctly by our parent.
	// (Or maybe I'm just to lazy to figure out the correct coordinates
	//  to use to compensate for the flipping... -wjp :-) )

	// Don't paint if hidden
	if (IsHidden()) return;

	// No scaling or filtering
	if (!buffer1) {
		PaintChildren(surf, lerp_factor, scaled);
		return;
	}

	// Render to texture
	buffer1->BeginPainting();
	PaintChildren(buffer1, lerp_factor, true);
	buffer1->EndPainting();

	if (!buffer2) {
		DoScalerBlit(buffer1->GetSurfaceAsTexture(), swidth1, sheight1, surf, width, height, scaler1);
	} else {
		buffer2->BeginPainting();
		DoScalerBlit(buffer1->GetSurfaceAsTexture(), swidth1, sheight1, buffer2, swidth2, sheight2, scaler1);
		buffer2->EndPainting();

		DoScalerBlit(buffer2->GetSurfaceAsTexture(), swidth2, sheight2, surf, width, height, scaler2);
	}

	int32 scalex = (width << 16) / swidth1;
	int32 scaley = (height << 16) / sheight1;

	// Iterate all children
	std::list<Gump *>::iterator it = children.end();
	std::list<Gump *>::iterator begin = children.begin();

	while (it != begin) {
		Gump *g = *it;
		// Paint if not closing
		if (!g->IsClosing())
			g->PaintCompositing(surf, lerp_factor, scalex, scaley);

		--it;
	}
}

void ScalerGump::DoScalerBlit(Texture *src, int swidth, int sheight, RenderSurface *dest, int dwidth, int dheight, const Pentagram::Scaler *scaler) {
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
void ScalerGump::ParentToGump(int &px, int &py, PointRoundDir r) {
	px -= x;
	px *= dims.w;
	if (px < 0 && r == ROUND_TOPLEFT) px -= (width - 1);
	if (px > 0 && r == ROUND_BOTTOMRIGHT) px += (width - 1);
	px /= width;

	py -= y;
	py *= dims.h;
	if (py < 0 && r == ROUND_TOPLEFT) py -= (height - 1);
	if (py > 0 && r == ROUND_BOTTOMRIGHT) py += (height - 1);
	py /= height;
}

// Convert a gump point to parent relative point
void ScalerGump::GumpToParent(int &gx, int &gy, PointRoundDir r) {
	gx *= width;
	if (gx < 0 && r == ROUND_TOPLEFT) gx -= (dims.w - 1);
	if (gx > 0 && r == ROUND_BOTTOMRIGHT) gx += (dims.w - 1);
	gx /= dims.w;
	gx += x;

	gy *= height;
	if (gy < 0 && r == ROUND_TOPLEFT) gy -= (dims.h - 1);
	if (gy > 0 && r == ROUND_BOTTOMRIGHT) gy += (dims.h - 1);
	gy /= dims.h;
	gy += y;
}

void ScalerGump::RenderSurfaceChanged() {
	// Resize the gump to match the RenderSurface
	Pentagram::Rect new_dims;
	parent->GetDims(new_dims);

	width = new_dims.w;
	height = new_dims.h;

	SetupScalers();

	Gump::RenderSurfaceChanged();
}

void ScalerGump::ChangeScaler(std::string scalername, int scalex, int scaley) {
	SettingManager *settingman = SettingManager::get_instance();

	if (scalex != 0) settingman->set("scalex", scalex);
	if (scaley != 0) settingman->set("scaley", scaley);
	if (scalername != "") settingman->set("scaler", scalername);

	SetupScalers();

	Gump::RenderSurfaceChanged();
}

void ScalerGump::SetupScalers() {
	FORGET_OBJECT(buffer1);
	FORGET_OBJECT(buffer2);

	SettingManager *settingman = SettingManager::get_instance();

	settingman->setDefault("scalex", 320);
	settingman->setDefault("scaley", 200);
	settingman->setDefault("scaler", "point");
	settingman->setDefault("scalex2", 0);
	settingman->setDefault("scaley2", 0);
	settingman->setDefault("scaler2", "point");

	std::string scaler1name;
	std::string scaler2name;
	settingman->get("scalex", swidth1);
	settingman->get("scaley", sheight1);
	settingman->get("scaler", scaler1name);
	settingman->get("scaler2", scaler2name);
	settingman->get("scalex2", swidth2);
	settingman->get("scaley2", sheight2);

	const Pentagram::Scaler *point = ScalerManager::get_instance()->GetPointScaler();

	scaler1 = ScalerManager::get_instance()->GetScaler(scaler1name);
	if (!scaler1) scaler1 = point ;

	scaler2 = ScalerManager::get_instance()->GetScaler(scaler2name);
	if (!scaler2 || !scaler2->ScaleArbitrary()) scaler2 = point ;

	if (swidth1 < 0) swidth1 = -swidth1;
	else if (swidth1 == 0) swidth1 = width;
	else if (swidth1 < 100) swidth1 = width / swidth1;

	if (sheight1 < 0) sheight1 = -sheight1;
	else if (sheight1 == 0) sheight1 = height;
	else if (sheight1 < 100) sheight1 = height / sheight1;

	if (swidth2 < 0) swidth2 = -swidth2;
	else if (swidth2 != 0 && swidth2 < 100) swidth2 = width / swidth2;

	if (sheight2 < 0) sheight2 = -sheight2;
	else if (sheight2 != 0 && sheight2 < 100) sheight2 = height / sheight2;

	dims.w = swidth1;
	dims.h = sheight1;

	// We don't care, we are not going to support filters, at least not at the moment
	if (swidth1 == width && sheight1 == height) return;

	buffer1 = RenderSurface::CreateSecondaryRenderSurface(swidth1, sheight1);
	con.Printf(MM_INFO, "Using Scaler: %s. %s\n", scaler1->ScalerDesc(), scaler1->ScalerCopyright());

	// scaler2's factor isn't set so auto detect
	if (swidth2 == 0 || sheight2 == 0) {

		// scaler 1 is arbitrary so scaler2 not required
		if (scaler1->ScaleArbitrary()) return;

		swidth2 = swidth1 * 32;
		sheight2 = sheight1 * 32;
		for (int i = 31; i >= 0; i--) {
			if (scaler1->ScaleBits() & (1 << i)) {
				if (swidth2 > width || sheight2 > height) {
					swidth2 = swidth1 * i;
					sheight2 = sheight1 * i;
				}
			}
		}
	}

	// scaler2 is required
	if (swidth2 != width || sheight2 != height) {
		// Well almost, in this situation we code in DoScalerBlit to do this for us
		// scaler2 not required
		if (width == 640 && height == 480 &&
		        swidth2 == 640 && sheight2 == 400 &&
		        swidth1 == 320 && sheight2 == 200 &&
		        scaler2 == point) {
			return;
		}

		buffer2 = RenderSurface::CreateSecondaryRenderSurface(swidth2, sheight2);
		con.Printf(MM_INFO, "Using Secondary Scaler: %s. %s\n", scaler2->ScalerDesc(), scaler2->ScalerCopyright());
	}
}

void ScalerGump::ConCmd_changeScaler(const Console::ArgvType &argv) {
	if (argv.size() != 4 && argv.size() != 2) {
		pout << "Usage: ScalerGump::changeScaler scaler [scalex] [scaley]" << std::endl;
		return;
	}

	ScalerGump *scalerGump = static_cast<ScalerGump *>(Ultima8Engine::get_instance()->getDesktopGump()->FindGump<ScalerGump>());

	if (scalerGump) {
		if (argv.size() != 2)
			scalerGump->ChangeScaler(argv[1], strtol(argv[2].c_str(), 0, 0), strtol(argv[3].c_str(), 0, 0));
		else
			scalerGump->ChangeScaler(argv[1], 0, 0);
	}
}

void ScalerGump::ConCmd_listScalers(const Console::ArgvType &argv) {
	ScalerManager *scaleman = ScalerManager::get_instance();
	uint32 numScalers = scaleman->GetNumScalers();

	for (uint32 i = 0; i < numScalers; i++) {
		const Pentagram::Scaler *s = scaleman->GetScaler(i);

		pout << s->ScalerName() << ": " << s->ScalerDesc() << " -";

		if (s->ScaleArbitrary()) pout << " Arbitrary";
		else for (int b = 0; b < 32; b++) if (s->ScaleBits() & (1 << b)) pout << " " << b << "x";

		pout << std::endl;
	}
}

} // End of namespace Ultima8
