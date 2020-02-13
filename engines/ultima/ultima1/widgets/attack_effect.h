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

#ifndef ULTIMA_ULTIMA1_WIDGETS_HIT_H
#define ULTIMA_ULTIMA1_WIDGETS_HIT_H

#include "ultima/shared/maps/map_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Common base class for both physical and spell attack effects
 */
class AttackEffect : public Shared::Maps::MapWidget {
protected:
	uint _tileId;
	Point _delta;
	uint _remainingDistance;
	uint _agility;
	uint _damage;
protected:
	/**
	 * Constructor
	 */
	AttackEffect(Shared::Game *game, Shared::Maps::MapBase *map, uint tileId) : Shared::Maps::MapWidget(game, map),
		_tileId(tileId), _remainingDistance(0), _agility(0), _damage(0) {}
public:
	/**
	 * Constructor
	 */
	AttackEffect(Shared::Game *game, Shared::Maps::MapBase *map) : Shared::Maps::MapWidget(game, map),
		_tileId(0), _remainingDistance(0), _agility(0), _damage(0) {}

	/**
	 * Handles loading and saving the widget's data
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Get the tile for the transport method
	 */
	uint getTileNum() const override { return _tileId; }

	/**
	 * Set the details for t
	 */
};

/**
 * Physical attack effects
 */
class PhysicalAttackEffect : public AttackEffect {
public:
	/**
	 * Constructor
	 */
	PhysicalAttackEffect(Shared::Game *game, Shared::Maps::MapBase *map) :
		AttackEffect(game, map, 50) {}
};

/**
 * Spell attack effects
 */
class SpellAttackEffect : public AttackEffect {
public:
	/**
	 * Constructor
	 */
	SpellAttackEffect(Shared::Game *game, Shared::Maps::MapBase *map) :
		AttackEffect(game, map, 51) {}
};


} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
