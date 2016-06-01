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

#include "common/textconsole.h"
#include "titanic/true_talk/doorbot_script.h"

namespace Titanic {

DoorbotScript::DoorbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	load("Responses/Doorbot");
}

int DoorbotScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	warning("TODO");
	return SS_2;
}

void DoorbotScript::proc7(int v1, int v2) {
	warning("TODO");
}

int DoorbotScript::proc10() const {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc15() const {
	warning("TODO");
	return 0;
}

bool DoorbotScript::proc16() const {
	warning("TODO");
	return 0;
}

bool DoorbotScript::proc17() const {
	warning("TODO");
	return 0;
}

bool DoorbotScript::proc18() const {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc22() const {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *DoorbotScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int DoorbotScript::proc25() const {
	warning("TODO");
	return 0;
}

void DoorbotScript::proc26() {
}

void DoorbotScript::proc32() {
	warning("TODO");
}

int DoorbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

uint DoorbotScript::translateId(uint id) const {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic
