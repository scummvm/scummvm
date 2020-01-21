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

#include "ultima/ultima1/widgets/dungeon_player.h"
#include "ultima/ultima1/widgets/dungeon_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

Shared::Maps::MapWidget::CanMove DungeonPlayer::canMoveTo(const Point &destPos) {
	Shared::Maps::MapWidget::CanMove result = Shared::Maps::DungeonWidget::canMoveTo(destPos);
	if (result != Shared::Maps::DungeonWidget::UNSET)
		return result;

	return Shared::Maps::DungeonWidget::UNSET;
	//return Shared::Maps::DungeonWidget::canMoveTo(_map, this, destPos);
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
