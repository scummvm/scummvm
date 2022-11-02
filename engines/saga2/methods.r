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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_METHODS_R
#define SAGA2_METHODS_R

namespace Saga2 {

// TODO FIXME. All of these have to be extracted from the disassembly

enum {
	Method_KnowledgePackage_evalResponse = 0,
	Method_KnowledgePackage_executeResponse = 1,

	Method_Actor_onTalkTo = 136,
	Method_Actor_onReceive = 137,
	Method_Actor_onDie = 138,
	Method_Actor_onDelobotomize = 139,
	Method_Actor_onLobotomize = 140,
	Method_Actor_onEndAssignment = 141,
	Method_Actor_onSpecialAttack = 142,
	Method_Actor_onAttacked = 143,

	Method_TileActivityInstance_onUse = 14,
	Method_TileActivityInstance_onTrigger = 15,
	Method_TileActivityInstance_onRelease = 16,
	Method_TileActivityInstance_onAcceptLockToggle = 17,
	Method_TileActivityInstance_onCanTrigger = 18,

	Method_GameObject_onUse = 21,
	Method_GameObject_onUseOn = 22,
	Method_GameObject_onUseOnTAI = 23,
	Method_GameObject_onTake = 24,
	Method_GameObject_onDrop = 25,
	Method_GameObject_onDropOn = 26,
	Method_GameObject_onOpen = 28,
	Method_GameObject_onClose = 29,
	Method_GameObject_onStrike = 30,
	Method_GameObject_onDamage = 31,
	Method_GameObject_onEat,
	Method_GameObject_onInsert,
	Method_GameObject_onRemove,
	Method_GameObject_onAcceptLockToggle,
	Method_GameObject_onAcceptMix,
	Method_GameObject_onAcceptDrop = 35,
	Method_GameObject_onAcceptDamage = 36,
	Method_GameObject_onAcceptStrike = 37,
	Method_GameObject_onAcceptInsertion = 40,
	Method_GameObject_onTrigger = 41,
	Method_GameObject_onTimerTick = 42,
	Method_GameObject_onSenseObject = 43,
	Method_GameObject_onSenseEvent = 44,
	Method_GameObject_onActivate = 45,
	Method_GameObject_onDeactivate = 46,
	Method_GameObject_onDeletion = 47
};

} // end of namespace Saga2

#endif
