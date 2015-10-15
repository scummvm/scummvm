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

#include "mads/conversations.h"
#include "mads/mads.h"

namespace MADS {

GameConversation::GameConversation(MADSEngine *vm)
	: _vm(vm) {
	_running = _restoreRunning = 0;
}

GameConversation::~GameConversation() {
}

void GameConversation::get(int id) {
	warning("TODO GameConversation::get");
}

void GameConversation::run(int id) {
	warning("TODO GameConversation::run");
}

void GameConversation::stop() {
	warning("TODO GameConversation::stop");
}

void GameConversation::exportPointer(int *val) {
	warning("TODO GameConversation::exportPointer");
}

void GameConversation::exportValue(int val) {
	warning("TODO GameConversation::exportValue");
}

void GameConversation::setHeroTrigger(int val) {
	warning("TODO: GameConversation::setHeroTrigger");
}

void GameConversation::setInterlocutorTrigger(int val) {
	warning("TODO: GameConversation::setInterlocutorTrigger");
}

int* GameConversation::getVariable(int idx) {
	warning("TODO: GameConversation::getVariable");
	return nullptr;
}
} // End of namespace MADS
