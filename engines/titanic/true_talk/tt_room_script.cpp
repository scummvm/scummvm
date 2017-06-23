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
#include "titanic/true_talk/tt_room_script.h"

namespace Titanic {

TTroomScriptBase::TTroomScriptBase(int scriptId,
		const char *charClass, const char *charName,
		int v3, int v4, int v5, int v6, int v2, int v7) : _scriptId(scriptId),
		TTscriptBase(3, charClass, v2, charName, v3, v4, v5, v6, v7) {
}

/*------------------------------------------------------------------------*/

TTroomScript::TTroomScript(int scriptId) :
	TTroomScriptBase(scriptId, "", "", 0, -1, -1, -1, 0, 0), _field54(0) {
}

bool TTroomScript::proc8() const {
	return false;
}

void TTroomScript::proc9(int v) {
	if (v == 1)
		_field54 = 1;
}

ScriptChangedResult TTroomScript::scriptChanged(TTscriptBase *npcScript, int id) {
	if (id == 1)
		_field54 = 1;

	return SCR_1;
}

bool TTroomScript::proc11() const {
	return true;
}

} // End of namespace Titanic
