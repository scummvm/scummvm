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

#include "common/system.h"
#include "sherlock/map.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/tattoo/tattoo_map.h"

namespace Sherlock {

Map *Map::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelMap(vm);
	else
		return new Tattoo::TattooMap(vm);
}

Map::Map(SherlockEngine *vm) : _vm(vm) {
	_charPoint = _oldCharPoint = 0;
	_active = _frameChangeFlag = false;
}

void Map::synchronize(Serializer &s) {
	s.syncAsSint32LE(_bigPos.x);
	s.syncAsSint32LE(_bigPos.y);
	s.syncAsSint32LE(_overPos.x);
	s.syncAsSint16LE(_overPos.y);
	s.syncAsSint16LE(_oldCharPoint);
}


} // End of namespace Sherlock
