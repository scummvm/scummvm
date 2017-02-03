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

#include "bladerunner/actor_combat.h"

namespace BladeRunner {

ActorCombat::ActorCombat(BladeRunnerEngine *vm) {
	_vm = vm;
}

ActorCombat::~ActorCombat() {
}

void ActorCombat::hitAttempt() {
}

void ActorCombat::combatOn(int actorId, int a3, int a4, int otherActorId, int a6, int a7, int a8, int a9, int ammoDamage, int a11, int a12) {
}

void ActorCombat::combatOff() {
}

void ActorCombat::setup() {
}

} // End of namespace BladeRunner
