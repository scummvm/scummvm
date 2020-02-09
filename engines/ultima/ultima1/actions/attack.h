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

#ifndef ULTIMA_ULTIMA1_ACTIONS_ATTACK_H
#define ULTIMA_ULTIMA1_ACTIONS_ATTACK_H

#include "ultima/ultima1/actions/action.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

using Shared::CCharacterInputMsg;
using Shared::CAttackMsg;
using Shared::CFireMsg;

/**
 * Common base class for attack and fire actions
 */
class AttackFire : public Action {
	DECLARE_MESSAGE_MAP;
	bool CharacterInputMsg(CCharacterInputMsg &msg);
protected:
	/**
	 * Do the attack in a given direction
	 */
	virtual void doAttack(Shared::Maps::Direction dir) = 0;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	AttackFire(TreeItem *parent) : Action(parent) {}

};

/**
 * Attack action
 */
class Attack : public AttackFire {
	DECLARE_MESSAGE_MAP;
	bool AttackMsg(CAttackMsg &msg);
protected:
	/**
	 * Do the attack in a given direction
	 */
	void doAttack(Shared::Maps::Direction dir) override;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Attack(TreeItem *parent) : AttackFire(parent) {}
};

/**
 * Fire action
 */
class Fire : public AttackFire {
	DECLARE_MESSAGE_MAP;
	bool FireMsg(CFireMsg &msg);
protected:
	/**
	 * Do the attack in a given direction
	 */
	void doAttack(Shared::Maps::Direction dir) override;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Fire(TreeItem *parent) : AttackFire(parent) {}
};

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
