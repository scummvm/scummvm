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
#include "titanic/true_talk/tt_unnamed_script.h"

namespace Titanic {

TTUnnamedScriptBase::TTUnnamedScriptBase(int scriptId,
		const char *charClass, const char *charName,
		int v3, int v4, int v5, int v6, int v2, int v7) : _scriptId(scriptId),
		TTScriptBase(3, charClass, v2, charName, v3, v4, v5, v6, v7) {
}

/*------------------------------------------------------------------------*/

TTUnnamedScript::TTUnnamedScript(int scriptId) :
	TTUnnamedScriptBase(scriptId, "", "", 0, -1, -1, -1, 0, 0) {
}

void TTUnnamedScript::proc6() {
	warning("TODO");
}

void TTUnnamedScript::proc7() {
	warning("TODO");
}

void TTUnnamedScript::proc8() {
	warning("TODO");
}

void TTUnnamedScript::proc9() {
	warning("TODO");
}

void TTUnnamedScript::proc10() {
	warning("TODO");
}

void TTUnnamedScript::proc11() {
	warning("TODO");
}

} // End of namespace Titanic
