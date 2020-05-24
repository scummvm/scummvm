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
#include "ultima/ultima8/gumps/mini_stats_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/paperdoll_gump.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniStatsGump)

static const int gumpshape = 33;
static const int hpx = 6;
static const int manax = 13;
static const int bary = 19;
static const int barheight = 14;

static const uint32 hpcolour[] = { 0x980404, 0xBC0C0C, 0xD43030 };
static const uint32 manacolour[] = { 0x4050FC, 0x1C28FC, 0x0C0CCC };


MiniStatsGump::MiniStatsGump() : Gump() {

}

MiniStatsGump::MiniStatsGump(int x, int y, uint32 flags, int32 layer)
	: Gump(x, y, 5, 5, 0, flags, layer) {
}

MiniStatsGump::~MiniStatsGump() {
}

void MiniStatsGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(gumpshape);
	UpdateDimsFromShape();
}

void MiniStatsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	Actor *a = getMainActor();
	assert(a);

	int16 maxmana = a->getMaxMana();
	int16 mana = a->getMana();

	uint16 maxhp = a->getMaxHP();
	uint16 hp = a->getHP();

	int manaheight, hpheight;


	if (maxmana == 0)
		manaheight = 0;
	else
		manaheight = (mana * barheight) / maxmana;

	if (maxhp == 0)
		hpheight = 0;
	else
		hpheight = (hp * barheight) / maxhp;

	for (int i = 0; i < 3; ++i) {
		surf->Fill32(hpcolour[i], hpx + i, bary - hpheight + 1, 1, hpheight);
		surf->Fill32(manacolour[i], manax + i, bary - manaheight + 1, 1, manaheight);
	}
}

uint16 MiniStatsGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	if (PointOnGump(mx, my)) return getObjId();

	return 0;
}

Gump *MiniStatsGump::onMouseDown(int button, int32 mx, int32 my) {
	if (button == Shared::BUTTON_LEFT)
		return this;

	return nullptr;
}

void MiniStatsGump::onMouseDouble(int button, int32 mx, int32 my) {
	// check if there already is an open PaperdollGump
	MainActor *av = getMainActor();
	if (!av->getGump()) {
		av->callUsecodeEvent_use();
	}

	Close();
}

void MiniStatsGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool MiniStatsGump::loadData(Common::ReadStream *rs, uint32 version) {
	return Gump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
