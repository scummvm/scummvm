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

#include "ultima/ultima1/widgets/dungeon_item.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

DungeonItem::DungeonItem(Shared::Game *game, Shared::Maps::MapBase *map, DungeonItemId itemId,
		const Point &pt) : DungeonWidget(game, map, (_itemId == DITEM_CHEST) ? MONSTER_MIMIC : UITEM_COFFIN, pt),
		_itemId(itemId) {
	_name = (_itemId == DITEM_CHEST) ? getGame()->_res->DUNGEON_ITEM_NAMES[0] : getGame()->_res->DUNGEON_ITEM_NAMES[1];
}

void DungeonItem::synchronize(Common::Serializer &s) {
	DungeonWidget::synchronize(s);
	s.syncAsUint16LE(_itemId);

	if (s.isLoading())
		_name = (_itemId == DITEM_CHEST) ? getGame()->_res->DUNGEON_ITEM_NAMES[0] : getGame()->_res->DUNGEON_ITEM_NAMES[1];
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
