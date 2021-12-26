/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima8/gumps/cru_stat_gump.h"


namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruStatGump)

CruStatGump::CruStatGump() : TranslucentGump() {

}

CruStatGump::CruStatGump(Shape *shape, int x)
	: TranslucentGump(x, 0, 5, 5, 0, FLAG_DONT_SAVE) {
	_shape = shape;
}

CruStatGump::~CruStatGump() {
}

void CruStatGump::InitGump(Gump *newparent, bool take_focus) {
	TranslucentGump::InitGump(newparent, take_focus);

	UpdateDimsFromShape();
}

void CruStatGump::saveData(Common::WriteStream *ws) {
	TranslucentGump::saveData(ws);
}

bool CruStatGump::loadData(Common::ReadStream *rs, uint32 version) {
	return TranslucentGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
