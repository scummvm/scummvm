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

#ifndef ULTIMA_ULTIMA1_WIDGETS_DUNGEON_MONSTER_H
#define ULTIMA_ULTIMA1_WIDGETS_DUNGEON_MONSTER_H

#include "ultima/ultima1/widgets/dungeon_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Implements monsters within the dungeons
 */
class DungeonMonster : public Shared::Creature, public Shared::DungeonCreature {
private:
	DungeonWidgetId _monsterId;
protected:
	/**
	 * Handles moving creatures
	 */
	virtual void movement() override;
public:
	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	static Shared::MapWidget::CanMove canMoveTo(Shared::Map::MapBase *map, MapWidget *widget, const Point &destPos);
public:
	DECLARE_WIDGET(DungeonMonster)

	/**
	 * Constructor
	 */
	DungeonMonster(Shared::Game *game, Shared::Map::MapBase *map, DungeonWidgetId monsterId, int hitPoints,
		const Point &pt, Shared::Direction dir = Shared::DIR_NONE) :
		Shared::Creature(game, map, hitPoints, pt, dir), Shared::DungeonCreature(), _monsterId(monsterId) {}

	/**
	 * Constructor
	 */
	DungeonMonster(Shared::Game *game, Shared::Map::MapBase *map) :
		Shared::Creature(game, map), Shared::DungeonCreature(), _monsterId(MONSTER_NONE) {}

	/**
	 * Destructor
	 */
	virtual ~DungeonMonster() {}

	/**
	 * Returns the monster's type
	 */
	DungeonWidgetId id() const { return _monsterId; }

	/**
	 * Returns true if a monster blocks the background behind him
	 */
	virtual bool isBlockingView() const override;

	/**
	 * Draw a monster
	 */
	virtual void draw(Shared::DungeonSurface &s, uint distance) override;

	/**
	 * Called to update the widget at the end of a turn
	 * @param isPreUpdate		Update is called twice in succesion during the end of turn update.
	 *		Once with true for all widgets, then with it false
	 */
	virtual void update(bool isPreUpdate) override;

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	virtual CanMove canMoveTo(const Point &destPos) override;

	/**
	 * Handles attacks
	 */
	virtual void attack() override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
