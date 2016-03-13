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
#include "titanic/true_talk/deskbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

DeskbotScript::DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTNamedScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	CTrueTalkManager::setFlags(18, 0);
	CTrueTalkManager::setFlags(19, 0);
	CTrueTalkManager::setFlags(20, 0);
	CTrueTalkManager::setFlags(21, 0);
	CTrueTalkManager::setFlags(22, 0);

	randomizeFlags();
	_array[0] = 100;
	if (_field74 == 1)
		_field74 = 0;
}

int DeskbotScript::proc6() const {
	warning("TODO");
	return 2;
}

void DeskbotScript::proc7(int v1, int v2) {
	warning("TODO");
}

int DeskbotScript::proc10() const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc15() const {
	warning("TODO");
	return 0;
}

bool DeskbotScript::proc16() const {
	warning("TODO");
	return 0;
}

bool DeskbotScript::proc17() const {
	warning("TODO");
	return 0;
}

bool DeskbotScript::proc18() const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc21(int v) {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc22() const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc23() const {
	warning("TODO");
	return 0;
}

void DeskbotScript::proc24() {
	warning("TODO");
}

int DeskbotScript::proc25() const {
	warning("TODO");
	return 0;
}

void DeskbotScript::proc26() {
}

int DeskbotScript::proc36() const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc37() const {
	warning("TODO");
	return 0;
}

void DeskbotScript::proc38() {
	warning("TODO");
}

void DeskbotScript::proc39() {
	warning("TODO");
}

void DeskbotScript::proc40() {
	warning("TODO");
}

void DeskbotScript::proc41() {
	warning("TODO");
}

} // End of namespace Titanic
