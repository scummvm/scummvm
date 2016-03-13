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
#include "titanic/true_talk/maitred_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

MaitreDScript::MaitreDScript(int val1, const char *charClass, int v2,
	const char *charName, int v3, int val2) :
	TTNamedScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	CTrueTalkManager::setFlags(9, 1);
	CTrueTalkManager::setFlags(10, 0);
	CTrueTalkManager::setFlags(11, 0);
	CTrueTalkManager::setFlags(12, 0);
	CTrueTalkManager::setFlags(13, 0);
	CTrueTalkManager::setFlags(14, 0);
	CTrueTalkManager::setFlags(15, 0);
	CTrueTalkManager::setFlags(16, 0);
}

int MaitreDScript::proc6() const {
	warning("TODO");
	return 2;
}

void MaitreDScript::proc7(int v1, int v2) {
	warning("TODO");
}

int MaitreDScript::proc10() const {
	warning("TODO");
	return 0;
}

bool MaitreDScript::proc16() const {
	warning("TODO");
	return 0;
}

bool MaitreDScript::proc17() const {
	warning("TODO");
	return 0;
}

bool MaitreDScript::proc18() const {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc21(int v) {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc22() const {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc23() const {
	warning("TODO");
	return 0;
}

void MaitreDScript::proc24() {
	warning("TODO");
}

int MaitreDScript::proc25() const {
	warning("TODO");
	return 0;
}

void MaitreDScript::proc26() {
}

int MaitreDScript::proc37() const {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic
